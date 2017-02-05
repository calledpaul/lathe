#ifndef MODE_CUT_H
#define MODE_CUT_H

#include <Utils.h>
#include "Stepper.h"

bool xStepDirChange = false, yStepDirChange = false;
long xStepDirChangeStartTime, yStepDirChangeStartTime;
bool fullCutModeLastRound = false;
int prev_xSwitchState, xSwitchState;
int prev_ySwitchState, ySwitchState;


void checkXSwitch() {
  xSwitchState = getInputRegisterValue(INDEX_X_AXIS_SWITCH_LEFT) ? LEFT :
    (getInputRegisterValue(INDEX_X_AXIS_SWITCH_RIGHT) ? RIGHT : OFF);
  if (prev_xSwitchState != xSwitchState) {
    if (xSwitchState != OFF) {
      xStepDirection = xSwitchState; 
      updateXAxisSpeed();
    }
    prev_xSwitchState = xSwitchState;
    halt = false;
  }
}
//optimize by checking only every 50 msec
void checkYSwitch() {
  ySwitchState = FastGPIO::Pin<PIN_Y_AXIS_SWITCH_LEFT>::isInputHigh() ? LEFT :
    (FastGPIO::Pin<PIN_Y_AXIS_SWITCH_RIGHT>::isInputHigh() ? RIGHT : OFF);
  if (prev_ySwitchState != ySwitchState) {
    if (ySwitchState != OFF) {
      yStepDirection = ySwitchState;
      updateYAxisSpeed();
    }
    prev_ySwitchState = ySwitchState;
    halt = false;
  }
}
TimedAction checkYSwitchAction = TimedAction(50, checkYSwitch);

void processManualX() {
  checkXSwitch();
  if (xSwitchState != OFF && !halt) xAxisStepper.runSpeed(); 
}

void processManualY() {
  checkYSwitchAction.check();
  if (ySwitchState != OFF && !halt) yAxisStepper.runSpeed();
}

void processFullManualMode() {
  processManualX();
  //processManualY();
}

bool xReturnRule() {
  return (state[cut] == LEFT && xAxisStepper.currentPosition() != 0) || 
    (state[cut] == RIGHT && xAxisStepper.currentPosition() == 0);
}

int getModeCutXNextSpeed() {
  int safeSpeed = state[xAxisSpeed] <= STEPPER_MAX_SAFE_SPEED ? state[xAxisSpeed] : STEPPER_MAX_SAFE_SPEED;
  if (disablePotsCheck) {
    disablePotsCheck = false;
    forceXSpeedCheck = true;
  }
  if (xReturnRule()) {
    safeSpeed = STEPPER_MAX_SAFE_SPEED;
    disablePotsCheck = true;
  }
  return safeSpeed;
}

void initXStepperCut() {
  if (xAxisStepper.currentPosition() && !xAxisStepper.distanceToGo() && !xStepDirChange) {
    xAxisStepper.setCurrentPosition(0);
  }
  if (xAxisStepper.currentPosition() == 0) {      
    moveXAxisTo(state[xAxisTo], LEFT, getModeCutXNextSpeed());
    xStepDirChange = false;
  }
}

void processModeCutX(bool init) {
  if (init) {
    initXStepperCut();
  }
  if (xStepDirChange) {
    if (xReturnRule() || (millis() - xStepDirChangeStartTime >= autoModeDelay)) {
      changeXAxisDirection(getModeCutXNextSpeed());
      xStepDirChange = false;
    }
  } else if (xAxisStepper.distanceToGo() == 0) {
    xStepDirChange = true;
    xStepDirChangeStartTime = millis();
  } else { // go go go
    xAxisStepper.runSpeedToPosition();
  }
}


bool yReturnRule() {
  return (state[cut] == LEFT && prevYStepBy < 0) || // next is outside
      (state[cut] == RIGHT && prevYStepBy > 0); //next is inside
}

int getModeCutNextYSpeed() {
  int safeSpeed = state[yAxisSpeed] <= STEPPER_MAX_SAFE_SPEED ? state[yAxisSpeed] : STEPPER_MAX_SAFE_SPEED;
  if (disablePotsCheck) {
    disablePotsCheck = false;
    forceYSpeedCheck = true;
  }
  if (yReturnRule()) { 
    safeSpeed = STEPPER_MAX_SAFE_SPEED;
    disablePotsCheck = true;
  }
  return safeSpeed;
}

void initYStepperCut() {
  if (yAxisStepper.currentPosition() == yAxisStepper.referencePoint) {
    moveYAxisBy(abs(state[yAxisTo]-state[yAxis])*signedDir[state[yAxis] > state[yAxisTo] ? INSIDE : OUTSIDE], getModeCutNextYSpeed());
    yStepDirChange = false;
  }
}

void processModeCutY(bool init) {
  if (init) {
    initYStepperCut();
  }
  if (yStepDirChange) {
    if (yReturnRule() || (millis() - yStepDirChangeStartTime >= autoModeDelay)) {
      changeYAxisDirection(getModeCutNextYSpeed());
      yStepDirChange = false;
    }
  } else if (yAxisStepper.distanceToGo() == 0) {
    yStepDirChange = true;
    yStepDirChangeStartTime = millis();
  } else { // go go go
    yAxisStepper.runSpeedToPosition();
  }
}

long _calculateNextYStep() {
  long fullDistance = abs(yAxisStepper.distanceToGo());
  if (!fullDistance) return 0;
  int finalStep = round(0.1 * stepsToMM);
  float step = round(getYSpeedAsMM() * stepsToMM);
  long distanceNoLastRound = fullDistance - finalStep;
  return (distanceNoLastRound - step) > 0 ? step : 
    (distanceNoLastRound < finalStep ? fullDistance : distanceNoLastRound);
}

void processFullCutMode(bool init, bool skipFinalStep = false, float xToYDecrementRatio = 0) {
  if (init) {
    initXStepperCut();
    initYStepperCut();
    yAxisStepper.stepContinuous(_calculateNextYStep());
    fullCutModeLastRound = false;
  }
  if (yAxisStepper.distanceToGo() != 0 || fullCutModeLastRound) {
    if (xAxisStepper.distanceToGo() == 0) {
      int spd = getModeCutXNextSpeed();
      if (fullCutModeLastRound) {
        fullCutModeLastRound = false;
        disablePotsCheck = false;
      } else if ((state[cut] == LEFT && xAxisStepper.currentPosition() == 0) || 
        (state[cut] == RIGHT && xAxisStepper.currentPosition() != 0) || state[cut] == BOTH ) 
      {
        long nextYStep = _calculateNextYStep();
        if (xToYDecrementRatio && xAxisStepper.currentPosition() == 0) {
          state[xAxisTo] -= round(float(nextYStep)/xToYDecrementRatio);
          if (state[xAxisTo] < 0) state[xAxisTo] = 0;
        }
        yAxisStepper.stepContinuous(nextYStep);
        if (yAxisStepper.distanceToGo() == 0 && state[xAxisTo]) { //state[xAxisTo] rule needed for angle precutting
          fullCutModeLastRound = true;
          disablePotsCheck = true;

          float rotationsOfShaftPerSecond = float(state[rpm])/60;
          spd = round(8 * rotationsOfShaftPerSecond); // 0.08 cut
        }
        changeXAxisDirection(spd);
      } else {
        changeXAxisDirection(spd);
      }
    } else { // go go go
      xAxisStepper.runSpeedToPosition();
    }
  } else if (!skipFinalStep) {
    // just not needed
    //changeYAxisDirection(STEPPER_MAX_SAFE_SPEED);
    //yAxisStepper.stepMM(1);
    program = false;
    stopEngine();
  }
}

void processModeCut(bool init) {
  if (state[xAxisTo] && state[yAxisTo]) {
    processFullCutMode(init);
  } else if (state[xAxisTo]) {
    processModeCutX(init);
    processManualY();
  } else if (state[yAxisTo]) {
    processModeCutY(init);
    processManualX();
  }
}

#endif
