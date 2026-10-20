#include "RadioApp.h"
RadioApp radio;
void setup() { Serial.begin(115200); radio.begin(); }
void loop() { radio.update(); delay(1); }
