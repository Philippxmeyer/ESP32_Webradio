#ifndef WEBRADIO_STREAMMETADATA_H
#define WEBRADIO_STREAMMETADATA_H
#include <stddef.h>
#include <stdint.h>

// Fixed-size, copyable snapshot for the queue between audio and UI tasks.
// Strings contain display glyphs (CP437), never pointers into the audio library.
struct StreamMetadata {
  static constexpr size_t MAX_DETAILS = 8;
  uint32_t station = 0;
  char sourceUrl[256] = {};
  char artist[192] = {};
  char title[256] = {};
  char stationName[128] = {};
  char description[256] = {};
  char url[192] = {};
  char details[MAX_DETAILS][160] = {};
  size_t detailCount = 0;
  char codec[16] = {};
  uint32_t bitrate = 0, sampleRate = 0;
  uint8_t channels = 0;

  void clear(uint32_t nextStation, const char* nextUrl = "");
  void streamTitle(const char* text);
  void id3(const char* text);
  static void copyText(char* dest, size_t capacity, const char* source);
};
#endif
