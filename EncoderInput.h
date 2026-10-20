#ifndef WEBRADIO_ENCODERINPUT_H
#define WEBRADIO_ENCODERINPUT_H
#include <AiEsp32RotaryEncoder.h>
#include "Config.h"
class EncoderInput {
public:
  void begin();
  bool clicked();
  bool changed() { return encoder_.encoderChanged() != 0; }
  long value() { return encoder_.readEncoder(); }
  void range(long maximum, long value, bool wrap);
private:
  static void IRAM_ATTR interrupt();
  static EncoderInput* instance_;
  bool rawButton_ = false;
  bool stableButton_ = false;
  uint32_t buttonChangedAt_ = 0;
  AiEsp32RotaryEncoder encoder_{Config::ENCODER_A, Config::ENCODER_B,
    Config::ENCODER_BUTTON, -1, Config::ENCODER_STEPS, false};
};

#endif

