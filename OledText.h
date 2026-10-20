#ifndef WEBRADIO_OLEDTEXT_H
#define WEBRADIO_OLEDTEXT_H
#include <Adafruit_GFX.h>
// Stored/web names use UTF-8; the OLED font uses CP437.
inline void printOledText(Adafruit_GFX& canvas, const char* text, size_t columns) {
  size_t written = 0;
  const auto* p = reinterpret_cast<const uint8_t*>(text);
  while (*p && written++ < columns) {
    uint32_t cp = *p++;
    if (cp >= 0x80) {
      unsigned more = cp < 0xe0 ? 1 : cp < 0xf0 ? 2 : 3;
      cp &= more == 1 ? 0x1f : more == 2 ? 0x0f : 7;
      while (more-- && *p) cp = (cp << 6) | (*p++ & 0x3f);
    }
    uint8_t glyph = cp < 0x80 ? cp : '?';
    switch (cp) {
      case 0xe4: glyph = 0x84; break; case 0xf6: glyph = 0x94; break;
      case 0xfc: glyph = 0x81; break; case 0xc4: glyph = 0x8e; break;
      case 0xd6: glyph = 0x99; break; case 0xdc: glyph = 0x9a; break;
      case 0xdf: glyph = 0xe1; break;
    }
    canvas.write(glyph);
  }
}
#endif
