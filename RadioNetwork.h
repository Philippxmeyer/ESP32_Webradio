#ifndef WEBRADIO_RADIONETWORK_H
#define WEBRADIO_RADIONETWORK_H
#include <WiFi.h>
#include "Config.h"
#include "StationStore.h"
class RadioNetwork {
public:
  void begin() {
    WiFi.setHostname(stationStore.hostname().c_str());
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);
    WiFi.begin(Config::WIFI_SSID, Config::WIFI_PASSWORD);
    lastAttempt_ = millis();
  }
  void update(uint32_t now) {
    if (!connected() && uint32_t(now - lastAttempt_) >= Config::WIFI_RETRY_MS) {
      lastAttempt_ = now;
      WiFi.reconnect();
    }
  }
  bool connected() const { return WiFi.status() == WL_CONNECTED; }
private:
  uint32_t lastAttempt_ = 0;
};


#endif

