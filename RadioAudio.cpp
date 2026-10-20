#include "RadioAudio.h"
#include <Audio.h>
#include <WiFi.h>
#include "Config.h"
#include "Stations.h"
#include <freertos/semphr.h>
#include <stdlib.h>
#include <string.h>
namespace {
// ICY/ID3 callbacks and decoder callbacks can run in different audio tasks.
StreamMetadata streamMetadata;
QueueHandle_t metadataQueue = nullptr;
SemaphoreHandle_t metadataMutex = nullptr;
class MetadataLock {
public:
  MetadataLock() { if (metadataMutex) xSemaphoreTake(metadataMutex, portMAX_DELAY); }
  ~MetadataLock() { if (metadataMutex) xSemaphoreGive(metadataMutex); }
  MetadataLock(const MetadataLock&) = delete;
  MetadataLock& operator=(const MetadataLock&) = delete;
};
void publishMetadata() {
  if (metadataQueue) xQueueOverwrite(metadataQueue, &streamMetadata);
}
void resetMetadata(uint32_t station, const char* url) {
  MetadataLock lock;
  streamMetadata.clear(station, url);
  publishMetadata();
}
}
void audio_showstreamtitle(const char* text) {
  MetadataLock lock;
  streamMetadata.streamTitle(text);
  publishMetadata();
}
void audio_id3data(const char* text) {
  MetadataLock lock;
  streamMetadata.id3(text);
  publishMetadata();
}
void audio_showstation(const char* text) {
  MetadataLock lock;
  StreamMetadata::copyText(streamMetadata.stationName, sizeof(streamMetadata.stationName), text);
  publishMetadata();
}
void audio_icydescription(const char* text) {
  MetadataLock lock;
  StreamMetadata::copyText(streamMetadata.description, sizeof(streamMetadata.description), text);
  publishMetadata();
}
void audio_icyurl(const char* text) {
  MetadataLock lock;
  StreamMetadata::copyText(streamMetadata.url, sizeof(streamMetadata.url), text);
  publishMetadata();
}
void audio_bitrate(const char* text) {
  MetadataLock lock;
  streamMetadata.bitrate = text ? strtoul(text, nullptr, 10) : 0;
  publishMetadata();
}
void audio_commercial(const char*) {
  MetadataLock lock;
  streamMetadata.streamTitle("Werbung");
  publishMetadata();
}
// ESP32-audioI2S 3.0.12 stores forceMono() but does not apply it.
// Its callback receives interleaved 16-bit stereo frames before I2S output.
void audio_process_i2s(int16_t* outBuff, uint16_t validSamples, uint8_t bitsPerSample,
                       uint8_t channels, bool* continueI2S) {
  *continueI2S = true;
  if (!Config::AUDIO_MONO || bitsPerSample != 16 || channels != 2) return;
  for (size_t frame = 0; frame < validSamples; ++frame) {
    const size_t i = frame * 2;
    const int16_t mono = (int32_t(outBuff[i]) + int32_t(outBuff[i + 1])) / 2;
    outBuff[i] = mono;
    outBuff[i + 1] = mono;
  }
}
bool RadioAudio::begin(uint32_t station, const char* url, uint8_t volume) {
  commands_ = xQueueCreate(1, sizeof(Command));
  if (!commands_) { status_ = AudioStatus::Error; return false; }
  metadata_ = xQueueCreate(1, sizeof(StreamMetadata));
  if (!metadata_) {
    vQueueDelete(commands_);
    commands_ = nullptr;
    status_ = AudioStatus::Error;
    return false;
  }
  metadataMutex = xSemaphoreCreateMutex();
  if (!metadataMutex) {
    vQueueDelete(commands_);
    vQueueDelete(metadata_);
    commands_ = metadata_ = nullptr;
    status_ = AudioStatus::Error;
    return false;
  }
  metadataQueue = metadata_;
  resetMetadata(station, url);
  request(station, url, volume);
  if (xTaskCreate(worker, "RadioAudio", 8192, this, 1, nullptr) != pdPASS) {
    vQueueDelete(commands_);
    vQueueDelete(metadata_);
    commands_ = nullptr;
    metadata_ = metadataQueue = nullptr;
    vSemaphoreDelete(metadataMutex);
    metadataMutex = nullptr;
    status_ = AudioStatus::Error;
    return false;
  }
  return true;
}
bool RadioAudio::readMetadata(StreamMetadata& snapshot) const {
  return metadata_ && xQueuePeek(metadata_, &snapshot, 0) == pdTRUE;
}
void RadioAudio::request(uint32_t station, const char* url, uint8_t volume) {
  if (!commands_ || !url || strlen(url) > StationStore::MAX_URL_BYTES) return;
  Command command{};
  command.station = station;
  strlcpy(command.url, url, sizeof(command.url));
  command.volume = volume;
  xQueueOverwrite(commands_, &command);
}
void RadioAudio::worker(void* context) {
  auto& self = *static_cast<RadioAudio*>(context);
  // Single owner of Audio; blocking stream connections do not block the UI.
  Audio audio;
  if (!audio.setPinout(Config::I2S_BCLK, Config::I2S_LRC, Config::I2S_DOUT)) {
    self.status_ = AudioStatus::Error;
    vTaskDelete(nullptr);
    return;
  }
  audio.forceMono(Config::AUDIO_MONO);
  audio.setConnectionTimeout(1500, 3000);
  Command current{};
  current.volume = Config::INITIAL_VOLUME;
  bool restart = true, wasConnected = false;
  uint32_t lastAttempt = 0;
  for (;;) {
    Command next;
    if (xQueueReceive(self.commands_, &next, 0) == pdTRUE) {
      restart = restart || next.station != current.station || strcmp(next.url, current.url) != 0;
      current = next;
      audio.setVolume(current.volume);
    }
    const bool connected = WiFi.status() == WL_CONNECTED;
    if (!connected) {
      if (wasConnected) {
        audio.stopSong();
        resetMetadata(current.station, current.url);
      }
      self.status_ = AudioStatus::WaitingWifi;
      restart = true;
    } else {
      const uint32_t now = millis();
      if (restart || (!audio.isRunning() && uint32_t(now - lastAttempt) >= Config::AUDIO_RETRY_MS)) {
        restart = false;
        audio.stopSong();
        resetMetadata(current.station, current.url);
        self.status_ = AudioStatus::Connecting;
        const bool ok = audio.connecttohost(current.url);
        lastAttempt = millis();
        self.status_ = ok ? AudioStatus::Connecting : AudioStatus::Retry;
      }
      audio.loop();
      if (audio.isRunning()) {
        self.status_ = AudioStatus::Playing;
        // Publish technical data at most once per second, outside the I2S callback.
        static uint32_t lastTechnicalUpdate = 0;
        if (uint32_t(now - lastTechnicalUpdate) >= 1000) {
          lastTechnicalUpdate = now;
          MetadataLock lock;
          streamMetadata.bitrate = audio.getBitRate();
          streamMetadata.sampleRate = audio.getSampleRate();
          streamMetadata.channels = audio.getChannels();
          StreamMetadata::copyText(streamMetadata.codec, sizeof(streamMetadata.codec), audio.getCodecname());
          publishMetadata();
        }
      }
      else if (self.status_ != AudioStatus::Connecting || uint32_t(millis() - lastAttempt) >= Config::AUDIO_RETRY_MS)
        self.status_ = AudioStatus::Retry;
    }
    wasConnected = connected;
    vTaskDelay(1);
  }
}
