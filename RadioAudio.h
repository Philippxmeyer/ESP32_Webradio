#ifndef WEBRADIO_RADIOAUDIO_H
#define WEBRADIO_RADIOAUDIO_H
#include <Arduino.h>
#include "StationStore.h"
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include "StreamMetadata.h"
enum class AudioStatus { WaitingWifi, Connecting, Playing, Retry, Error };
class RadioAudio {
public:
  bool begin(uint32_t station, const char* url, uint8_t volume);
  void request(uint32_t station, const char* url, uint8_t volume);
  AudioStatus status() const { return status_.load(); }
  bool readMetadata(StreamMetadata& snapshot) const;
private:
  struct Command { uint32_t station; char url[StationStore::MAX_URL_BYTES + 1]; uint8_t volume; };
  static void worker(void* context);
  QueueHandle_t commands_ = nullptr;
  QueueHandle_t metadata_ = nullptr;
  std::atomic<AudioStatus> status_{AudioStatus::WaitingWifi};
};
#endif
