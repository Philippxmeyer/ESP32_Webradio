#include "EncoderInput.h"
EncoderInput* EncoderInput::instance_ = nullptr;
void IRAM_ATTR EncoderInput::interrupt() { instance_->encoder_.readEncoder_ISR(); }
void EncoderInput::begin() {
  instance_ = this;
  encoder_.begin();
  encoder_.setup(interrupt);
  encoder_.disableAcceleration();
}
void EncoderInput::range(long maximum, long value, bool wrap) {
  encoder_.setBoundaries(0, maximum, wrap);
  encoder_.setEncoderValue(value);
  encoder_.encoderChanged();
}
bool EncoderInput::clicked() {
  // The library's click helper waits for release. Poll its raw state instead
  // and debounce without waiting, so long presses cannot freeze the UI.
  const uint32_t now = millis();
  const bool down = encoder_.isEncoderButtonDown();
  if (down != rawButton_) {
    rawButton_ = down;
    buttonChangedAt_ = now;
  }
  if (stableButton_ != rawButton_ && uint32_t(now - buttonChangedAt_) >= 30) {
    stableButton_ = rawButton_;
    return !stableButton_; // one click on debounced release
  }
  return false;
}
