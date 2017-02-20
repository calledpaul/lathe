#include <SPI_anything.h>
#include "ManualEncoders.h"
#include "Screen.h"
#include "ModeCut.h"
#include "ModeAngle.h"
#include "ModeThread.h"
#include "Inputs.h"

/*
Minor Bugs
  - some crazy bug while too often program clicks during threaing auto
REsearch
  - validate actual speed on both axis's - test 4x microstep
  - validate speed slowdown while both working
*/

void triggerCalculateAngle() {
  if (state[actionMode] == MODE_ANGLE) state[angle] = calculateAngle();
}

void encoderValueUpdate(int key, int val) {
  switch (key) {
    case actionMode:
      triggerCalculateAngle();
      break;
    case yAxis:
      yAxisStepper.setCurrentPosition(long(val) << 5); // /2/100*stepsToMM
      yAxisStepper.referencePoint = yAxisStepper.currentPosition();
      triggerCalculateAngle();
      updateYAxisSpeed();
      break;
    case yAxisTo:
      yAxisStepper.referencePoint = yAxisStepper.currentPosition();
      triggerCalculateAngle();
      break;
    case xAxis:
      xAxisStepper.setCurrentPosition(0);
      triggerCalculateAngle();
      break;
    case xAxisTo:
      triggerCalculateAngle();
      break;
    case threadStep:
      state[yAxisTo] = state[yAxis] + (state[threadDir] == RIGHT ? -1 : 1)*round(getThreadDepth()*100);
      break;
  }
  if (key == xAxis || key == xAxisTo || key == threadStep || key == threadType || key == threadDir) {
    cutDepth = 0;
  }
}

void updateStateVars() {
  //with a bug in 0.01mm
  state[xAxis] = abs(xAxisStepper.currentPosition() >> 5);
  state[yAxis] = abs(yAxisStepper.currentPosition() >> 5);
  if (abs(state[rpm] - shaftEncoder.speed()) > 2) {
    state[rpm] = shaftEncoder.speed();
  }
  state[shaftAngle] = state[rpm] < 30 ? shaftEncoder.getShaftAngle() : -1;
}

void shaftStopHandler() {
  shaftEncoder.tickCallbackEnabled = false;
  if (xSwitchState != OFF || ySwitchState != OFF || programState()) {
    programActive(false);
    //safety precaution after turning auto mode off
    halt = true;
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
  return !ignoreREDButton && (!programState() || halt);
}

// long _prevPosX = 0, _prevPosY = 0;
// long m = 0;
// void speedCheck() {
//   p(m);
//   // pp(float(abs(_prevPosX-xAxisStepper.currentPosition()))/3200*60, ' ', float(abs(_prevPosY-yAxisStepper.currentPosition()))/3200*60)
//   // _prevPosX = xAxisStepper.currentPosition();
//   // _prevPosY = yAxisStepper.currentPosition();
// }
// TimedAction speedCheckAction = TimedAction(1000, speedCheck);

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

  updateStateVars();
  sendScreenUpdate(true);
}

long bu = 0;

void executePrograms() {
  bool init = programChange && programState(), end = programChange && !programState();
  programChange = false;
  switch(state[actionMode]) {
    case MODE_CUT:
      processModeCut(init, end);
    break;
    case MODE_ANGLE:
      processModeAngle(init, end);
    break;
    case MODE_THREAD:
      processModeThread(init, end);
    break;
  }
}

void loop() {
  //speedCheckAction.check();  
  //could be slowed done when "isRunning"
  readInputRegister();
  
  checkRedButton();
  shaftStatusAction.check();

  msec200Action.check();

  if (programState() || programChange) {
    executePrograms();
    // triggered inside of one of programs, run to end correctly
    if (programChange) {
      executePrograms();
    }
  } else {
    if (ySwitchState == OFF && xSwitchState == OFF) {
      manualEncoders.check();
    }
    processFullManualMode();
  }
  programChange = false;
}



