#ifndef ROTARYENCODER_H
#define ROTARYENCODER_H

#include <Arduino.h>

class RotaryEncoder {
  unsigned int state, t;
  long change = 0, timestmp = 0;
  int mask0, mask1, ppc = 4, movement, tbl[16] = {
    0, +1, -1, 0,
    -1, 0, -2, +1,
    +1, +2, 0, -1,
    0, -1, +1, 0
  };
  unsigned int readState() {
    return (inputRegister & mask0 ? 1u : 0u) | (inputRegister & mask1 ? 2u : 0u);
  };
public:
  RotaryEncoder(int m0, int m1) : mask0(m0), mask1(m1), change(0), state(0) {}
  void init() {
    state = readState();
  };
  void poll() {
    t = readState();
    movement = tbl[(state << 2) | t];
    if (movement != 0) {
      change += movement;
      state = t;
    }
  };
  long getChange(int timeDependant = 0) {
    long r = 0;
    noInterrupts();
    if (change >= ppc - 1)
      r = (change + 1) / ppc;
    else if (change <= 1 - ppc)
      r = -((1 - change) / ppc);
    change -= (r * ppc);
    interrupts();
    if (r && timeDependant) {
      long m = millis();
      //Serial.println((m - timestmp));
      r = ((m - timestmp) > 200 ? 1 : (m - timestmp) > 50 ? 10 : 100) * r;
      timestmp = m;
    }
    return r;
  };
};

#endif




