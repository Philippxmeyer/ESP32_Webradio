#ifndef WEBRADIO_RADIOAPP_H
#define WEBRADIO_RADIOAPP_H
#include "EncoderInput.h"
#include "RadioNetwork.h"
#include "RadioAudio.h"
#include "RadioDisplay.h"
#include "StationMenu.h"
#include "RadioWeb.h"
#include <Preferences.h>
class RadioApp {
public:
  void begin();
  void update();
private:
  void volumeMode();
  void stationsChanged();
  void saveSettings(uint32_t now);
  Preferences preferences_;
  bool storageReady_ = false;
  uint32_t savedSettings_ = 0, savedStationId_ = 0, currentStationId_ = 0, lastVolumeChange_ = 0;
  EncoderInput encoder_;
  RadioNetwork network_;
  RadioWeb web_;
  RadioAudio audio_;
  StreamMetadata metadata_;
  RadioDisplay display_;
  size_t station_ = 0;
  StationMenu menu_;
  uint8_t volume_ = Config::INITIAL_VOLUME;
  bool selecting_ = false, dirty_ = true;
  uint32_t lastActivity_ = 0, lastFrame_ = 0;
};


#endif

