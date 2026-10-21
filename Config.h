#ifndef WEBRADIO_CONFIG_H
#define WEBRADIO_CONFIG_H

#include <Arduino.h>

namespace Config {

// ------------------------------------------------------------
// WLAN
// ------------------------------------------------------------

constexpr char WIFI_SSID[] = "DEIN_WLAN";
constexpr char WIFI_PASSWORD[] = "DEIN_PASSWORT";


// ------------------------------------------------------------
// Display / I2C
// ------------------------------------------------------------
//
// ESP32-S3 SuperMini
//
// SDA = GPIO8
// SCL = GPIO9
//
// Beide Pins sind normale GPIOs und auf diesem Board gut geeignet
// für I2C.
//

constexpr int I2C_SDA = 8;
constexpr int I2C_SCL = 9;

constexpr uint8_t OLED_ADDRESS = 0x3C;

// true  = SH1106
// false = SSD1306
//
// Beide Displays: 128 x 64 Pixel
//
constexpr bool OLED_SH1106 = true;


// ------------------------------------------------------------
// Drehencoder
// ------------------------------------------------------------
//
// Encoderkontakte gegen GND.
// Interne Pull-ups werden von der Encoderbibliothek verwendet.
//
// CLK / A = GPIO4
// DT  / B = GPIO5
// SW      = GPIO6
//

constexpr int ENCODER_A = 4;
constexpr int ENCODER_B = 5;
constexpr int ENCODER_BUTTON = 6;

// Typisch:
// 4 = viele mechanische Encoder
// 2 = manche Encoder / Module
//
constexpr uint8_t ENCODER_STEPS = 4;


// ------------------------------------------------------------
// I2S Audio / MAX98357A
// ------------------------------------------------------------
//
// BCLK = GPIO12
// LRC  = GPIO13
// DIN  = GPIO11
//
// Diese drei Pins werden ausschließlich für den digitalen
// Audiobus verwendet.
//
// MAX98357A:
// BCLK -> GPIO12
// LRC  -> GPIO13
// DIN  -> GPIO11
//

constexpr int I2S_BCLK = 12;
constexpr int I2S_LRC  = 13;
constexpr int I2S_DOUT = 11;


// ------------------------------------------------------------
// Audio
// ------------------------------------------------------------
//
// true:
//   Linker und rechter Kanal werden softwareseitig zu Mono
//   zusammengemischt.
//   Ein MAX98357A.
//
// false:
//   Stereoausgabe.
//   Zwei MAX98357A parallel an BCLK/LRC/DIN.
//   Kanalwahl über SD/MODE der Verstärker.
//

constexpr bool AUDIO_MONO = true;


// ------------------------------------------------------------
// Lautstärke
// ------------------------------------------------------------

constexpr uint8_t MAX_VOLUME = 21;
constexpr uint8_t INITIAL_VOLUME = 8;


// ------------------------------------------------------------
// Bedienung
// ------------------------------------------------------------

// Menü nach 5 Sekunden ohne Bedienung verlassen.
constexpr uint32_t SELECTION_TIMEOUT_MS = 5000;

// Lautstärke erst nach 2 Sekunden Ruhe in NVS speichern.
constexpr uint32_t SETTINGS_SAVE_DELAY_MS = 2000;


// ------------------------------------------------------------
// Netzwerk / Audio Retry
// ------------------------------------------------------------

constexpr uint32_t WIFI_RETRY_MS  = 15000;
constexpr uint32_t AUDIO_RETRY_MS = 10000;

} // namespace Config

#endif