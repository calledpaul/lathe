#ifndef ShaftEncoder_h
#define ShaftEncoder_h 1

#include "Utils.h"
#include "PinChangeInt.h"

class ShaftEncoder {
  private:
    //for one interrupt
    //static int8_t lookup_table[] = {0, 0, 0, -1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, 0};
    //for two interrupts
    int8_t lookup_table[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
    uint8_t enc_val = 0;
    long shaftEncoderTicks = 0, prevShaftEncoderTicks = 0;
    unsigned long prev_rpm_time = 0;
    int realShaftEncoderTicksCount = 4096, oneRotationShaftEncoderTicksCount = 0, shaftDir;
    unsigned int shaftSpeed = 0;
    callbackFnInt _tickCallback;
    callbackFn _stopHandler, _zeroPointTickCallback;
    bool shaftIsStopped = true;
  public:
    bool tickCallbackEnabled = false;
    ShaftEncoder() {
      FastGPIO::Pin<PIN_SHAFT_ENCODER_ZERO>::setInput();
      FastGPIO::Pin<PIN_SHAFT_ENCODER_ZERO>::setInputPulledUp();
    }
    void attachCallback(callbackFnInt tickCallback) {
      _tickCallback = tickCallback;
    }
    void isr() {
      enc_val = enc_val << 2;
      enc_val |= ((PIND & 0b1100) >> 2);
      int d = lookup_table[enc_val & 0b1111];
      if (d) {
          shaftEncoderTicks += d;
          oneRotationShaftEncoderTicksCount += d;
          shaftDir = d;
          if (_tickCallback && tickCallbackEnabled) _tickCallback(d);
      }
    }
    void zeroPointISR() {
      if (prev_rpm_time != 0) {
          shaftSpeed = round((60 / (double(millis() - prev_rpm_time) / 1000)));
          if (abs(oneRotationShaftEncoderTicksCount) > realShaftEncoderTicksCount-40 
              && abs(oneRotationShaftEncoderTicksCount) <= realShaftEncoderTicksCount 
              && realShaftEncoderTicksCount != abs(oneRotationShaftEncoderTicksCount)) 
          {
              shaftEncoderTicks += (oneRotationShaftEncoderTicksCount > 0 ? 1 : -1)*(realShaftEncoderTicksCount - abs(oneRotationShaftEncoderTicksCount));
          }
      }
      oneRotationShaftEncoderTicksCount = 0;
      prev_rpm_time = millis();
      shaftIsStopped = false;
      if (_zeroPointTickCallback) _zeroPointTickCallback();
    }
    int speed() {
      return shaftSpeed;
    }
    int getShaftAngle() {      
      return oneRotationShaftEncoderTicksCount;
    }
    int shaftDirection() {
      return shaftDir;
    }
    bool shaftStatus() {
      bool ret = true;
      if (shaftIsStopped) {
        ret = false;
      } else if(shaftEncoderTicks == prevShaftEncoderTicks) {
        shaftSpeed = 0;
        shaftIsStopped = true;
        if (_stopHandler) {
          _stopHandler();
        }
        ret = false;
      }
      prevShaftEncoderTicks = shaftEncoderTicks;
      return ret;
    }
    long ticks() {
      return shaftEncoderTicks;
    }
    void setStopHandler(callbackFn stopFn) {
      _stopHandler = stopFn;
    }
    void setZeroPointTickCallback(callbackFn shaftZeroPointTick) {
      _zeroPointTickCallback = shaftZeroPointTick;
    }
};

ShaftEncoder shaftEncoder;

void shaftEncoderChangeISR() {
  shaftEncoder.isr();
}
void shaftEncoderZeroISR() {
  shaftEncoder.zeroPointISR();
}
void shaftStatus() {
  shaftEncoder.shaftStatus();
}

void initShaftEncoder(callbackFnInt tickCallback, callbackFn stopFn, callbackFn shaftZeroPointTick) {
  shaftEncoder.attachCallback(tickCallback);
  shaftEncoder.setStopHandler(stopFn);
  shaftEncoder.setZeroPointTickCallback(shaftZeroPointTick);
 
  attachInterrupt(INTERRUPT_SHAFT_ENCODER_A, shaftEncoderChangeISR, CHANGE);
  attachInterrupt(INTERRUPT_SHAFT_ENCODER_B, shaftEncoderChangeISR, CHANGE);

  pinMode(PIN_INTERRUPT_SHAFT_ENCODER_ZERO, INPUT);     //set the PIN_INTERRUPT_SHAFT_ENCODER_ZERO to input
  digitalWrite(PIN_INTERRUPT_SHAFT_ENCODER_ZERO, HIGH); //use the internal pullup resistor
  PCintPort::attachInterrupt(PIN_INTERRUPT_SHAFT_ENCODER_ZERO, shaftEncoderZeroISR, RISING); // at
}


#endif




