#ifndef SPI_ANYTHING_H
#define SPI_ANYTHING_H

#include <Arduino.h>
#include "Def.h"
#include <FastGPIO.h>
#include <SPI.h>

typedef struct dataStruct {
  byte mixed;
  int data;
};
volatile dataStruct message;

template <typename T> unsigned int SPI_writeAnything (const T& value) {
  const byte * p = (const byte*) &value;
  unsigned int i;
  for (i = 0; i < sizeof value; i++)
      SPI.transfer(*p++);
  return i;
}

template <typename T> unsigned int SPI_readAnything(T& value) {
  byte * p = (byte*) &value;
  unsigned int i;
  for (i = 0; i < sizeof value; i++)
      *p++ = SPI.transfer (0);
  return i;
}

template <typename T> unsigned int SPI_readAnything_ISR(T& value) {
  byte * p = (byte*) &value;
  unsigned int i;
  *p++ = SPDR;  // get first byte
  for (i = 1; i < sizeof value; i++)
    *p++ = SPI.transfer (0);
  return i;
}

void SPI_sendMessage(byte key, int value) {
  message.mixed = key;
  message.data = value;
//  Serial.print(key);
//  Serial.print(' ');
//  Serial.println(value);
  
  FastGPIO::Pin<SS>::setOutputLow();
  SPI_writeAnything (message);
  FastGPIO::Pin<SS>::setOutputHigh();
}

#endif

