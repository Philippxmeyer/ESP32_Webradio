#include "RadioDisplay.h"
#include "StationStore.h"
#include "OledText.h"
#include <stdio.h>
#include <string.h>
bool RadioDisplay::begin() {
  Wire.begin(Config::I2C_SDA, Config::I2C_SCL);
  Wire.setClock(400000);
  bool ok;
  if (Config::OLED_SH1106) {
    ok = sh1106_.begin(Config::OLED_ADDRESS, true);
    canvas_ = &sh1106_;
  } else {
    ok = ssd1306_.begin(SSD1306_SWITCHCAPVCC, Config::OLED_ADDRESS, true, false);
    canvas_ = &ssd1306_;
  }
  if (!ok) { canvas_ = nullptr; Serial.println("OLED initialization failed"); }
  if (canvas_) canvas_->cp437(true);
  return ok;
}
namespace {
constexpr uint32_t SCROLL_PAUSE_MS = 1200, SCROLL_PIXELS_PER_SECOND = 30;
uint32_t travelTime(const char* text) {
  const size_t pixels = strlen(text) * 6;
  return pixels > 128 ? uint32_t((pixels - 128) * 1000 / SCROLL_PIXELS_PER_SECOND) : 0;
}
uint32_t pageTime(const char* text) {
  const uint32_t duration = travelTime(text) + 2 * SCROLL_PAUSE_MS;
  return duration < 4000 ? 4000 : duration;
}
const char* statusText(AudioStatus status) {
  switch (status) {
    case AudioStatus::WaitingWifi: return "WLAN verbinden...";
    case AudioStatus::Connecting: return "Stream verbinden...";
    case AudioStatus::Playing: return "Wiedergabe";
    case AudioStatus::Retry: return "Stream: neuer Versuch";
    case AudioStatus::Error: return "Audio-Startfehler";
  }
  return "";
}
}
void RadioDisplay::line(const char* text, int16_t y, uint32_t elapsed) {
  const uint32_t travel = travelTime(text);
  uint32_t offset = 0;
  if (travel) {
    const uint32_t phase = elapsed % (travel + 2 * SCROLL_PAUSE_MS);
    if (phase > SCROLL_PAUSE_MS) {
      const uint32_t moving = phase - SCROLL_PAUSE_MS;
      offset = moving >= travel ? strlen(text) * 6 - 128 : moving * SCROLL_PIXELS_PER_SECOND / 1000;
    }
  }
  canvas_->setCursor(-int16_t(offset), y);
  canvas_->print(text);
}
void RadioDisplay::render(size_t station, const StationMenu& menu, uint8_t volume,
                          bool selecting, uint32_t remaining, AudioStatus status,
                          const StreamMetadata& metadata) {
  if (!canvas_) return;
  const uint32_t now = millis();
  if (Config::OLED_SH1106) sh1106_.clearDisplay(); else ssd1306_.clearDisplay();
  canvas_->setTextColor(1);
  canvas_->cp437(true);
  canvas_->setTextSize(1);
  canvas_->setTextWrap(false);
  if (station != displayedStation_ || (wasSelecting_ && !selecting)) {
    displayedStation_ = station;
    stationSince_ = songSince_ = detailSince_ = now;
    detailPage_ = 0;
    artist_[0] = title_[0] = detailLines_[0][0] = detailLines_[1][0] = 0;
  }
  wasSelecting_ = selecting;
  if (selecting) {
    canvas_->setCursor(0, 0);
    printOledText(*canvas_, menu.title(), 21);
    canvas_->drawFastHLine(0, 10, 128, 1);
    const size_t count = menu.count();
    size_t first = count > 3 && menu.selected() > 0 ? menu.selected() - 1 : 0;
    if (count > 3 && first > count - 3) first = count - 3;
    for (size_t row = 0; row < 3 && first + row < count; ++row) {
      const size_t index = first + row;
      const int16_t y = 13 + row * 10;
      const bool selected = index == menu.selected();
      if (selected) canvas_->fillRect(0, y - 1, 128, 10, 1);
      canvas_->setTextColor(selected ? 0 : 1);
      canvas_->setCursor(0, y);
      const char* name = menu.name(index);
      const bool group = menu.entry(index).kind == StationMenu::Kind::Group;
      printOledText(*canvas_, name, group ? 20u : 21u);
      if (group) { canvas_->setCursor(122, y); canvas_->print(">"); }
    }
    canvas_->setTextColor(1);
    canvas_->setCursor(0, 43);
    canvas_->printf("%u/%u Abbruch in %us", unsigned(menu.selected() + 1),
                    unsigned(count), unsigned((remaining + 999) / 1000));
    line(statusText(status), 51, 0);
  } else {
    char stationName[StationStore::MAX_NAME_BYTES + 1];
    StreamMetadata::copyText(stationName, sizeof(stationName), stationStore.at(station).name.c_str());
    line(stationName, 0, now - stationSince_);
    canvas_->drawFastHLine(0, 10, 128, 1);
    // Never show metadata from the previous station while a command is pending.
    const bool current = metadata.station == stationStore.at(station).id &&
                         !strcmp(metadata.sourceUrl, stationStore.at(station).url.c_str());
    const char* artist = current ? metadata.artist : "";
    const char* title = current ? metadata.title : "";
    if (strcmp(artist_, artist) || strcmp(title_, title)) {
      strcpy(artist_, artist);
      strcpy(title_, title);
      songSince_ = detailSince_ = now;
      detailPage_ = 0;
    }
    if (!artist[0] && !title[0]) line("Keine Titelinfos", 14, 0);
    else {
      line(artist[0] ? artist : title, 14, now - songSince_);
      if (artist[0]) line(title, 25, now - songSince_);
    }
    char technical[96] = {};
    const char* details[StreamMetadata::MAX_DETAILS + 4];
    size_t count = 0;
    if (current) {
      for (size_t i = 0; i < metadata.detailCount; ++i)
        if (metadata.details[i][0]) details[count++] = metadata.details[i];
      if (metadata.description[0]) details[count++] = metadata.description;
      if (metadata.stationName[0] && strcmp(metadata.stationName, stationName))
        details[count++] = metadata.stationName;
      if (metadata.url[0]) details[count++] = metadata.url;
      if (metadata.codec[0] || metadata.bitrate || metadata.sampleRate) {
        size_t used = 0;
        if (metadata.codec[0]) used += snprintf(technical + used, sizeof(technical) - used, "%s", metadata.codec);
        if (metadata.bitrate) used += snprintf(technical + used, sizeof(technical) - used, "%s%lu kb/s", used ? " | " : "", (unsigned long)(metadata.bitrate / 1000));
        if (metadata.sampleRate) used += snprintf(technical + used, sizeof(technical) - used, "%s%lu.%lu kHz", used ? " | " : "", (unsigned long)(metadata.sampleRate / 1000), (unsigned long)(metadata.sampleRate % 1000 / 100));
        if (metadata.channels) snprintf(technical + used, sizeof(technical) - used, " | %u ch", metadata.channels);
        details[count++] = technical;
      }
    }
    const size_t rows = status == AudioStatus::Playing ? 2 : 1;
    const size_t pages = count ? (count + rows - 1) / rows : 1;
    if (detailPage_ >= pages) { detailPage_ = 0; detailSince_ = now; }
    uint32_t duration = pageTime(detailLines_[0]);
    if (rows == 2 && pageTime(detailLines_[1]) > duration) duration = pageTime(detailLines_[1]);
    if (uint32_t(now - detailSince_) >= duration) {
      detailPage_ = (detailPage_ + 1) % pages;
      detailSince_ = now;
    }
    for (size_t row = 0; row < rows; ++row) {
      const size_t index = detailPage_ * rows + row;
      const char* text = index < count ? details[index] : "";
      // Technical updates must not restart scrolling on every variable-bitrate frame.
      strncpy(detailLines_[row], text, sizeof(detailLines_[row]) - 1);
      detailLines_[row][sizeof(detailLines_[row]) - 1] = 0;
      line(detailLines_[row], 37 + row * 11, now - detailSince_);
    }
    if (status != AudioStatus::Playing) line(statusText(status), 48, 0);
  }
  // The only volume indication, at the very bottom in both views.
  canvas_->drawRect(0, 59, 128, 5, 1);
  canvas_->fillRect(1, 60, (126 * volume) / Config::MAX_VOLUME, 3, 1);
  if (Config::OLED_SH1106) sh1106_.display(); else ssd1306_.display();
}
