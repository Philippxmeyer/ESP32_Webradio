#include "StreamMetadata.h"
#include <string.h>

namespace {
uint8_t glyph(uint32_t codepoint) {
  if (codepoint >= 32 && codepoint < 127) return uint8_t(codepoint);
  // Unicode values of the upper half of the built-in Adafruit CP437 font.
  static const uint16_t cp437[] = {
    0x00c7,0x00fc,0x00e9,0x00e2,0x00e4,0x00e0,0x00e5,0x00e7,
    0x00ea,0x00eb,0x00e8,0x00ef,0x00ee,0x00ec,0x00c4,0x00c5,
    0x00c9,0x00e6,0x00c6,0x00f4,0x00f6,0x00f2,0x00fb,0x00f9,
    0x00ff,0x00d6,0x00dc,0x00a2,0x00a3,0x00a5,0x20a7,0x0192,
    0x00e1,0x00ed,0x00f3,0x00fa,0x00f1,0x00d1,0x00aa,0x00ba,
    0x00bf,0x2310,0x00ac,0x00bd,0x00bc,0x00a1,0x00ab,0x00bb,
    0x2591,0x2592,0x2593,0x2502,0x2524,0x2561,0x2562,0x2556,
    0x2555,0x2563,0x2551,0x2557,0x255d,0x255c,0x255b,0x2510,
    0x2514,0x2534,0x252c,0x251c,0x2500,0x253c,0x255e,0x255f,
    0x255a,0x2554,0x2569,0x2566,0x2560,0x2550,0x256c,0x2567,
    0x2568,0x2564,0x2565,0x2559,0x2558,0x2552,0x2553,0x256b,
    0x256a,0x2518,0x250c,0x2588,0x2584,0x258c,0x2590,0x2580,
    0x03b1,0x00df,0x0393,0x03c0,0x03a3,0x03c3,0x00b5,0x03c4,
    0x03a6,0x0398,0x03a9,0x03b4,0x221e,0x03c6,0x03b5,0x2229,
    0x2261,0x00b1,0x2265,0x2264,0x2320,0x2321,0x00f7,0x2248,
    0x00b0,0x2219,0x00b7,0x221a,0x207f,0x00b2,0x25a0,0x00a0
  };
  if (codepoint < 32 || codepoint == 0x00a0) return ' ';
  if (codepoint == 0x2013 || codepoint == 0x2014) return '-';
  if (codepoint == 0x2018 || codepoint == 0x2019) return '\'';
  if (codepoint == 0x201c || codepoint == 0x201d) return '"';
  for (size_t i = 0; i < 128; ++i)
    if (cp437[i] == codepoint) return uint8_t(128 + i);
  return '?';
}
void copyGlyphs(char* dest, size_t capacity, const char* source, size_t length) {
  if (length >= capacity) length = capacity - 1;
  memcpy(dest, source, length);
  dest[length] = 0;
  while (length && dest[length - 1] == ' ') dest[--length] = 0;
}
}

void StreamMetadata::copyText(char* dest, size_t capacity, const char* source) {
  if (!capacity) return;
  size_t out = 0;
  const unsigned char* p = reinterpret_cast<const unsigned char*>(source ? source : "");
  while (*p && out + 1 < capacity) {
    uint32_t codepoint = *p++;
    unsigned continuation = 0;
    uint32_t minimum = 0;
    if (codepoint >= 0xc2 && codepoint <= 0xdf) {
      continuation = 1; minimum = 0x80;
    } else if (codepoint >= 0xe0 && codepoint <= 0xef) {
      continuation = 2; minimum = 0x800;
    } else if (codepoint >= 0xf0 && codepoint <= 0xf4) {
      continuation = 3; minimum = 0x10000;
    }
    if (continuation) {
      uint32_t decoded = codepoint & ((1u << (6 - continuation)) - 1);
      unsigned valid = 0;
      while (valid < continuation && p[valid] && (p[valid] & 0xc0) == 0x80) {
        decoded = (decoded << 6) | (p[valid] & 0x3f);
        ++valid;
      }
      if (valid == continuation && decoded >= minimum && decoded <= 0x10ffff &&
          !(decoded >= 0xd800 && decoded <= 0xdfff)) {
        codepoint = decoded;
        p += continuation;
      }
      // Non-UTF-8 bytes are treated as Latin-1, as on older ICY servers.
    }
    const uint8_t c = glyph(codepoint);
    if (c == ' ' && (!out || dest[out - 1] == ' ')) continue;
    dest[out++] = char(c);
  }
  while (out && dest[out - 1] == ' ') --out;
  dest[out] = 0;
}

void StreamMetadata::clear(uint32_t nextStation, const char* nextUrl) {
  station = nextStation;
  // Clear in place: do not put a complete second snapshot on the audio stack.
  artist[0] = title[0] = stationName[0] = description[0] = url[0] = codec[0] = 0;
  memset(details, 0, sizeof(details));
  detailCount = 0;
  bitrate = sampleRate = channels = 0;
  const size_t length = nextUrl ? strlen(nextUrl) : 0;
  copyGlyphs(sourceUrl, sizeof(sourceUrl), nextUrl ? nextUrl : "", length);
}

void StreamMetadata::streamTitle(const char* text) {
  char normalized[sizeof(artist) + sizeof(title)];
  copyText(normalized, sizeof(normalized), text);
  char* separator = strstr(normalized, " - ");
  // ICY has one free-text field. Preserve the supplied order without labels.
  const char* nextTitle = separator ? separator + 3 : normalized;
  char nextArtist[sizeof(artist)] = {};
  if (separator) copyGlyphs(nextArtist, sizeof(nextArtist), normalized, separator - normalized);
  if (strcmp(nextArtist, artist) == 0 && strcmp(nextTitle, title) == 0) return;
  memcpy(artist, nextArtist, sizeof(artist));
  copyGlyphs(title, sizeof(title), nextTitle, strlen(nextTitle));
  // Album, year, etc. belong to a track, not to the whole station.
  memset(details, 0, sizeof(details));
  detailCount = 0;
}

void StreamMetadata::id3(const char* text) {
  char normalized[256];
  copyText(normalized, sizeof(normalized), text);
  if (!strncmp(normalized, "Artist: ", 8)) {
    copyGlyphs(artist, sizeof(artist), normalized + 8, strlen(normalized + 8));
    return;
  }
  if (!strncmp(normalized, "Title: ", 7)) {
    copyGlyphs(title, sizeof(title), normalized + 7, strlen(normalized + 7));
    return;
  }
  if (!normalized[0]) return;
  const char* colon = strchr(normalized, ':');
  const size_t keyLength = colon ? size_t(colon - normalized + 1) : strlen(normalized);
  size_t slot = detailCount;
  for (size_t i = 0; i < detailCount; ++i) {
    const char* oldColon = strchr(details[i], ':');
    const size_t oldKeyLength = oldColon ? size_t(oldColon - details[i] + 1) : strlen(details[i]);
    if (keyLength == oldKeyLength && !strncmp(details[i], normalized, keyLength)) {
      slot = i;
      break;
    }
  }
  if (slot == MAX_DETAILS) {
    memmove(details[0], details[1], (MAX_DETAILS - 1) * sizeof(details[0]));
    slot = MAX_DETAILS - 1;
  } else if (slot == detailCount) ++detailCount;
  copyGlyphs(details[slot], sizeof(details[slot]), normalized, strlen(normalized));
}
