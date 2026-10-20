#ifndef WEBRADIO_RADIODISPLAY_H
#define WEBRADIO_RADIODISPLAY_H
#include <Adafruit_SH110X.h>
#include <Adafruit_SSD1306.h>
#include "Config.h"
#include "RadioAudio.h"
#include "StationMenu.h"
class RadioDisplay {
public:
  bool begin();
  void render(size_t station, const StationMenu& menu, uint8_t volume,
              bool selecting, uint32_t remaining, AudioStatus status,
              const StreamMetadata& metadata);
private:
  void line(const char* text, int16_t y, uint32_t elapsed);
  Adafruit_SH1106G sh1106_{128, 64, &Wire, -1};
  Adafruit_SSD1306 ssd1306_{128, 64, &Wire, -1};
  Adafruit_GFX* canvas_ = nullptr;
  size_t displayedStation_ = size_t(-1), detailPage_ = 0;
  bool wasSelecting_ = false;
  char artist_[192] = {}, title_[256] = {};
  char detailLines_[2][256] = {};
  uint32_t stationSince_ = 0, songSince_ = 0, detailSince_ = 0;
};
#endif
