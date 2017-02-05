#ifndef Steppers_h
#define Steppers_h 1

volatile int tries = 0, state = 0, lastPlay = 0;
volatile long stepsDone = 0;

float shaftAgg = 0;

3200/4096

void followShaftLean(int delta) {
  float togo = delta ? shaftAgg + 0.78125 : shaftAgg - 0.78125
  //float togo = shaftAgg + float(delta) * STEPS_PER_REVOLUTION / realShaftEncoderTicksCount;
  int togoInt = int(togo);  
  shaftAgg = togo - togoInt;
  togoInt = abs(togoInt);
  //check if we need while here
  while (togoInt--) {
    xAxisStepper.runStep(shaftDir);
  }
}








void followShaft(int delta) {
  float togo = shaftAgg + float(delta) * STEPS_PER_REVOLUTION / realShaftEncoderTicksCount;
  int togoInt = int(togo);
  

// cool part about removing play on shaft change, but IT's NOT nEeDED

  if (prevShaftDir != shaftDir) {
    tries = shaftDir*10;
    state = shaftDir;
    prevShaftDir = shaftDir;
  } else if(state) {
    if (state == shaftDir && !tries) {
      if (lastPlay != shaftDir) {
        removePlay(shaftDir > 0 ? 1 : -1);
        lastPlay = shaftDir;
      }
      state = 0;
    }
    tries -= delta;
  }

///

  shaftAgg = togo - togoInt;
  togoInt = abs(togoInt);
  while (togoInt--) {
    xAxisStepper.runStep(shaftDir);
    stepsDone -= shaftDir;
  }
  prevShaftEncoderTicks = shaftEncoderTicks;
}



  void stepperPower(bool on, bool force = false) {
    if (stepperPowerOn != on || force) {
      digitalWrite(stepperPowerPin, on ? LOW : HIGH);
      stepperPowerOn = on;
    }
  }


  void accel() {
    if (switchState != 0 && speed >= maxWorkSpeed && currentAccelSpeed < speed) {
      currentAccelSpeed += 10;
      updateStepSpeed(currentAccelSpeed);
    }
  }

#endif




