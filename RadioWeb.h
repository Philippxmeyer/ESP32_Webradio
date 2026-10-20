#ifndef WEBRADIO_RADIOWEB_H
#define WEBRADIO_RADIOWEB_H
#include <WebServer.h>
#include <functional>
class RadioWeb {
public:
  void begin(std::function<void()> changed);
  void update();
private:
  void readList();
  void saveList();
  void receiveBody();
  void error(int status, const String& message);
  void headers();
  WebServer server_{80};
  std::function<void()> changed_;
  String token_, body_, activeHostname_;
  bool restartPending_ = false;
  uint32_t restartSince_ = 0;
  uint32_t revision_ = 1;
  bool oversized_ = false, complete_ = false, wasConnected_ = false, mdns_ = false;
};
#endif
