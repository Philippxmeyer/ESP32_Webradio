#ifndef WEBRADIO_CONFIG_H
#define WEBRADIO_CONFIG_H
#include <Arduino.h>
namespace Config {
constexpr char WIFI_SSID[] = "DEIN_WLAN";
constexpr char WIFI_PASSWORD[] = "DEIN_PASSWORT";
constexpr int I2C_SDA = 21, I2C_SCL = 22;
constexpr uint8_t OLED_ADDRESS = 0x3C;
constexpr bool OLED_SH1106 = true; // false: SSD1306 (both 128x64)
constexpr int ENCODER_A = 32, ENCODER_B = 33, ENCODER_BUTTON = 27;
constexpr uint8_t ENCODER_STEPS = 4;
constexpr int I2S_BCLK = 26, I2S_LRC = 25, I2S_DOUT = 23;
constexpr bool AUDIO_MONO = true; // true: one amp (L+R mix), false: two amps (left/right)
constexpr uint8_t MAX_VOLUME = 21, INITIAL_VOLUME = 8;
constexpr uint32_t SELECTION_TIMEOUT_MS = 5000;
constexpr uint32_t SETTINGS_SAVE_DELAY_MS = 2000;
constexpr uint32_t WIFI_RETRY_MS = 15000, AUDIO_RETRY_MS = 10000;
}

#endif

