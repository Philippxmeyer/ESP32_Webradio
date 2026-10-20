#include "RadioApp.h"
#include "Stations.h"
void RadioApp::begin() {
  stationStore.begin();
  storageReady_ = preferences_.begin("webradio", false);
  // Preserve legacy position/volume settings; stationId survives list reordering.
  const uint32_t settings = storageReady_
    ? preferences_.getUInt("settings", Config::INITIAL_VOLUME)
    : Config::INITIAL_VOLUME;
  station_ = settings >> 8;
  volume_ = settings & 0xff;
  if (storageReady_ && preferences_.isKey("stationId")) {
    savedStationId_ = preferences_.getUInt("stationId", 0);
    station_ = stationStore.find(savedStationId_);
  }
  if (station_ >= stationStore.count()) station_ = 0;
  currentStationId_ = stationStore.at(station_).id;
  if (volume_ > Config::MAX_VOLUME) volume_ = Config::INITIAL_VOLUME;
  savedSettings_ = (uint32_t(station_) << 8) | volume_;
  encoder_.begin();
  volumeMode();
  display_.begin();
  network_.begin();
  audio_.begin(currentStationId_, stationStore.at(station_).url.c_str(), volume_);
  web_.begin([this] { stationsChanged(); });
}
void RadioApp::saveSettings(uint32_t now) {
  const uint32_t settings = (uint32_t(station_) << 8) | volume_;
  if (!storageReady_ || (settings == savedSettings_ && currentStationId_ == savedStationId_)) return;
  // Retry failed writes at most once per save interval.
  lastVolumeChange_ = now;
  if (preferences_.putUInt("stationId", currentStationId_) == sizeof(currentStationId_) &&
      preferences_.putUInt("settings", settings) == sizeof(settings)) {
    savedStationId_ = currentStationId_;
    savedSettings_ = settings;
  }
}
void RadioApp::stationsChanged() {
  station_ = stationStore.find(currentStationId_);
  if (station_ >= stationStore.count()) station_ = 0;
  currentStationId_ = stationStore.at(station_).id;
  menu_.reset();
  volumeMode();
  saveSettings(millis());
  audio_.request(currentStationId_, stationStore.at(station_).url.c_str(), volume_);
}
void RadioApp::volumeMode() {
  selecting_ = false;
  encoder_.range(Config::MAX_VOLUME, volume_, false);
  dirty_ = true;
}
void RadioApp::update() {
  network_.update(millis());
  web_.update();
  const uint32_t now = millis();
  // Rotation before click: simultaneous confirmation uses the latest selection.
  if (encoder_.changed()) {
    if (selecting_) menu_.select(size_t(encoder_.value()));
    else {
      volume_ = uint8_t(encoder_.value());
      lastVolumeChange_ = now;
      audio_.request(currentStationId_, stationStore.at(station_).url.c_str(), volume_);
    }
    lastActivity_ = now;
    dirty_ = true;
  }
  if (encoder_.clicked()) {
    if (selecting_) {
      const StationMenu::Action action = menu_.activate(station_);
      if (action == StationMenu::Action::Play) {
        currentStationId_ = stationStore.at(station_).id;
        saveSettings(now);
        audio_.request(currentStationId_, stationStore.at(station_).url.c_str(), volume_);
        volumeMode();
      } else if (action == StationMenu::Action::Close) {
        volumeMode();
      } else {
        encoder_.range(menu_.count() - 1, menu_.selected(), true);
        dirty_ = true;
      }
    } else {
      menu_.reset();
      selecting_ = true;
      encoder_.range(menu_.count() - 1, menu_.selected(), true);
      dirty_ = true;
    }
    lastActivity_ = now;
  }
  if (selecting_ && uint32_t(now - lastActivity_) >= Config::SELECTION_TIMEOUT_MS)
    volumeMode(); // discard unconfirmed selection
  if (uint32_t(now - lastVolumeChange_) >= Config::SETTINGS_SAVE_DELAY_MS)
    saveSettings(now);
  if ((dirty_ && uint32_t(now - lastFrame_) >= 50) || uint32_t(now - lastFrame_) >= 250) {
    const uint32_t remaining = selecting_ ? Config::SELECTION_TIMEOUT_MS - uint32_t(now - lastActivity_) : 0;
    const char* streamUrl = stationStore.at(station_).url.c_str();
    if (!audio_.readMetadata(metadata_) || metadata_.station != currentStationId_ ||
        strcmp(metadata_.sourceUrl, streamUrl)) metadata_.clear(currentStationId_, streamUrl);
    display_.render(station_, menu_, volume_, selecting_, remaining, audio_.status(), metadata_);
    lastFrame_ = now;
    dirty_ = false;
  }
}
