#include <SPI_anything.h>
#include "ManualEncoders.h"
#include "Screen.h"
#include "ModeCut.h"
#include "ModeAngle.h"
#include "ModeThread.h"
#include "Inputs.h"

/*
!!! your own identity!!!
Important
 - put on GitHub
REsearch
 - validate actual speed on both axis's - text 4x microstep
 - validate speed slowdown while both working
Test
 - validate angle modes

Hardware
 - tune up Y stepper power
 - fix Y pot glitch
 
Problems:
 - FIX final clear cut offset for THREADING auto
 - smooth speed, remove minimums
 - redo manula threading
 - pause/resume mode for CutAuto/Thread auto???
 - issues with restart AUTO CUT/THREAD/ANGLE

Screen features to add
 - proram output use RED LED
 - add program mode display -- think about this one
 - visual indication of program mode/ screen/led? 
*/

void encoderValueUpdate(int key, int val) {
  switch (key) {
    case actionMode:
      if (actionMode == MODE_CUT && calculateAngle()) state[angle] = calculateAngle();
      break;
    case yAxis:
      yAxisStepper.setCurrentPosition(long(val) << 5); // /2/100*stepsToMM
      yAxisStepper.referencePoint = long(val) << 5;
      updateYAxisSpeed();
      break;
    case xAxis:
      xAxisStepper.setCurrentPosition(0);
      break;
  }
  if (key == xAxis || key == xAxisTo || key == threadStep || key == threadType || key == threadDir) {
    cutDepth = 0;
  }
}

void shaftStopHandler() {
  shaftEncoder.tickCallbackEnabled = false;
  if (xSwitchState != OFF || ySwitchState != OFF || program) {
    program = false; //pause auto action
    halt = true; //stop manual runSpeed
    stopEngine(); 
  }
}

TimedAction shaftStatusAction = TimedAction(50, shaftStatus);
void msec200() {
  //long z = micros();
  if (!disablePotsCheck) checkPots();
  //p(micros()-z);
  updateStateVars();
  sendScreenUpdate();
}
TimedAction msec200Action = TimedAction(200, msec200);

bool continuousStepperEmergencyFunc() {
  //check for RedButton
  checkRedButton();
  //check for collision
  shaftStatusAction.check();
  return !program || halt;
}

long _prevPosX = 0, _prevPosY = 0;
long m = 0;
void speedCheck() {
  p(m);
  // pp(float(abs(_prevPosX-xAxisStepper.currentPosition()))/3200*60, ' ', float(abs(_prevPosY-yAxisStepper.currentPosition()))/3200*60)
  // _prevPosX = xAxisStepper.currentPosition();
  // _prevPosY = yAxisStepper.currentPosition();
}
TimedAction speedCheckAction = TimedAction(1000, speedCheck);

void setup() {
  Serial.begin(9600);
  initManualEncoders();
  spiInit();
  initShaftEncoder(followShaft, shaftStopHandler, shaftZeroPointTick);
  encoderValueUpdateFunc = encoderValueUpdate;

  FastGPIO::Pin<PIN_RED>::setInput();
  
  yAxisStepper.setMaxSpeed(maxspeedSteps);
  xAxisStepper.setMaxSpeed(maxspeedSteps);
  xAxisStepper.emergencyFunc = continuousStepperEmergencyFunc;
  yAxisStepper.emergencyFunc = continuousStepperEmergencyFunc;

  setUpFastPrescaler();

// state[xAxisTo] = 500;
// xAxisStepper.setCurrentPosition(500 << 5);
  // state[yAxisTo] = 800;
//yAxisStepper.setCurrentPosition(1000 << 5);
//yAxisStepper.referencePoint = 1000 << 5;

//state[actionMode] = MODE_THREAD;

  updateStateVars();
  sendScreenUpdate(true);

  updateYAxisSpeed(204);
  updateXAxisSpeed(204);
}

long bu = 0;

void loop() {
  speedCheckAction.check();
  bu = micros();
  //speedCheckAction.check();
  //could be slowed done when "isRunning"
  //readInputRegister();
  
  //checkRedButton();
  //shaftStatusAction.check();

  //msec200Action.check();

  if (program) {
    bool init = programChange && program;
    switch(state[actionMode]) {
      case MODE_CUT:
        processModeCut(init);
        
      break;
      case MODE_ANGLE:
        processModeAngle(init);
      break;
      case MODE_THREAD:
        processModeThread(init);
      break;
    }
  } else {
    if (ySwitchState == OFF && xSwitchState == OFF) {
      //manualEncoders.check();
    }
    processFullManualMode();
  }
  programChange = false;
  m += micros()-bu;
  m = m/2;
}



