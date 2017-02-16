#ifndef UTILS_H
#define UTILS_H
#include <Arduino.h>
#include "Def.h"
#include <math.h>
#include "TimedAction.h"
#include <FastGPIO.h>
#include <SPI.h>

extern "C" {
	typedef void (*callbackFn)(void);
	typedef void (*callbackFnInt)(int);
	typedef void (*callbackFnIntInt)(int, int);
	typedef void (*callbackFnIntBool)(int, bool);
  typedef bool (*callbackFnRetBool)(void);
}

byte inputRegister = 0;

void spiInit() {
  SPI.begin();
  FastGPIO::Pin<SS>::setOutputHigh();
  SPI.setClockDivider(SPI_CLOCK_DIV16); 
  pinMode(A0, OUTPUT);
  FastGPIO::Pin<A0>::setOutputHigh();
}

void readInputRegister() {
	FastGPIO::Pin<A0>::setOutputLow();
  inputRegister = SPI.transfer(0x00);
  FastGPIO::Pin<A0>::setOutputHigh();
}

bool getInputRegisterValue(int index) {
	return inputRegister & (1 << index);
}

void stopEngine() {
  FastGPIO::Pin<PIN_ENGINE_POWER_OFF>::setOutput(HIGH);
  //!!! this one could be optimized
  delay(50);
  FastGPIO::Pin<PIN_ENGINE_POWER_OFF>::setOutput(LOW);
  if (disablePotsCheck) {
    disablePotsCheck = false;
    disablePotY = false;
    disablePotX = false;
    forceYSpeedCheck = true;
    forceXSpeedCheck = true;
  }
}

void programActive(bool st) {
  if (state[program] != st) {
    programChange = true;
    state[program] = st ? 1 : 0;
    if (!st) {
      //safety precaution after turning auto mode off
      halt = true;
      if (disablePotsCheck) {
        disablePotsCheck = false;
        disablePotY = false;
        disablePotX = false;
        forceYSpeedCheck = true;
        forceXSpeedCheck = true;
      }
    } else {
      halt = false;
    }
  }
}
void programCorrectEnd() {
  programActive(false);
  programChange = false;
  stopEngine();
}

bool programState() {
  return state[program] == 1;
}

void setUpFastPrescaler() {
  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits
  //  ADCSRA |= bit (ADPS0);                               //   2  
  //  ADCSRA |= bit (ADPS1);                               //   4  
  //  ADCSRA |= bit (ADPS0) | bit (ADPS1);                 //   8  
  ADCSRA |= bit (ADPS2);                               //  16 
  //  ADCSRA |= bit (ADPS0) | bit (ADPS2);                 //  32 
  //  ADCSRA |= bit (ADPS1) | bit (ADPS2);                 //  64 
  //  ADCSRA |= bit (ADPS0) | bit (ADPS1) | bit (ADPS2);   // 128
}

#define pp(a, b, c) Serial.print(a); Serial.print(b); Serial.println(c);  
#define p(a) Serial.println(a);

#endif

