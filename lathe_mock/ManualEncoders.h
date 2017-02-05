#ifndef MANUAL_ENCODERS_H
#define MANUAL_ENCODERS_H

#include <Utils.h>
#include "RotaryEncoder.h"


RotaryEncoder modeEncoder(B00100000, B00010000);
RotaryEncoder valueEncoder(B00000010, B00000001);

callbackFnIntInt encoderValueUpdateFunc;

void initManualEncoders() {
  modeEncoder.init();
  valueEncoder.init();
}

void checkManualEncoders() {
  int change, tempVal;
  modeEncoder.poll();
  change = modeEncoder.getChange();
  if (change) {
    tempVal = state[modeEncoderValue] + change;
    if (tempVal >= stateLimits[modeEncoderValue][0] && (
        (state[actionMode] == MODE_CUT && tempVal <= 6) ||
        (state[actionMode] == MODE_ANGLE && tempVal <= 6) ||
        (state[actionMode] == MODE_THREAD && tempVal <= 8) 
      )) {
        state[modeEncoderValue] = tempVal;
    }
  }
  int valIndex = state[modeEncoderValue];
  if (state[actionMode] == MODE_ANGLE && valIndex == 6) {
    valIndex = angle;
  } else if (state[actionMode] == MODE_THREAD && valIndex > 5) {
    valIndex += 2;
  }
  if (getInputRegisterValue(3)) {
    state[valIndex] = 0;
    if (encoderValueUpdateFunc) encoderValueUpdateFunc(valIndex, state[valIndex]);
  }
  valueEncoder.poll();
  change = valueEncoder.getChange(stateLimits[valIndex][2]);
  if (change) {
    tempVal = state[valIndex] + change;
    if (tempVal >= stateLimits[valIndex][0] && tempVal <= stateLimits[valIndex][1]) {
      state[valIndex] = tempVal;
      if (encoderValueUpdateFunc) encoderValueUpdateFunc(valIndex, state[valIndex]);
    }
  }
}

TimedAction manualEncoders = TimedAction(2, checkManualEncoders);

#endif
