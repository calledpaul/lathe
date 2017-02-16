#ifndef Stepper_h
#define Stepper_h

#include <Utils.h>
#include "AccelStepper.h"


int signedDir[2] = {-1, 1};

class AStepper : public AccelStepper {
  public:
    float play = 0.0;
    long referencePoint = 0;
    callbackFnRetBool emergencyFunc;
    AStepper(float playVal) : play(playVal), AccelStepper(AccelStepper::DRIVER, 0, 0) {}
    virtual void enableOutputs() {
      if (_enablePin != 0xff){
          pinMode(_enablePin, OUTPUT);
          digitalWrite(_enablePin, HIGH ^ _enableInverted);
      }
    }
    void removePlay(int direction) {
      unsigned long currentPos = currentPosition(), spd = speed(), target = targetPosition();
      setCurrentPosition(0);
      setSpeed(maxspeedSteps*signedDir[direction] / 1.5);
      while (currentPosition() != round(play * stepsToMM * signedDir[direction])) runSpeed();      
      setCurrentPosition(currentPos);
      moveTo(target);
      setSpeed(spd);
    }
    void setSpeedRemovePlay(float newSpeed) {
      int spd = speed();
      if ((speed() < 0 && newSpeed > 0) || (speed() > 0 && newSpeed < 0)){
        removePlay(newSpeed > 0 ? 1 : 0);
      }
      setSpeed(newSpeed);
    }
    void stepMM(float mm) {
      stepContinuous(round(mm * stepsToMM));
    }
    void stepContinuous(long steps, bool toPosition = true) {
      long snapshot = currentPosition();
      //pp(abs(snapshot - currentPosition()), " " , steps)
      //int entries = 0;
      while (abs(snapshot - currentPosition()) < steps) {
        //entries++;
        if (emergencyFunc && emergencyFunc()) break;
        if(toPosition)
          runSpeedToPosition();
        else
          runSpeed();
      }
      //p(entries);
      //p("  ")
    }
    void stepContinuousToDestination() {
      long snapshot = currentPosition();
      while (distanceToGo()) {
        if (emergencyFunc && emergencyFunc()) break;
        runSpeedToPosition();
      }
    }
};
class XStepper : public AStepper {
  public:
    XStepper() : AStepper(X_AXIS_PLAY_MM) {}
  protected:
  	virtual void setOutputPins(uint8_t mask) {
  		FastGPIO::Pin<PIN_X_AXIS_STEPPER_B>::setOutput((mask & (1 << 0)) ? (HIGH ^ _pinInverted[0]) : (LOW ^ _pinInverted[0]));
  		FastGPIO::Pin<PIN_X_AXIS_STEPPER_A>::setOutput((mask & (1 << 1)) ? (HIGH ^ _pinInverted[1]) : (LOW ^ _pinInverted[0]));
  	}
};

class YStepper : public AStepper {
  public:
    YStepper() : AStepper(Y_AXIS_PLAY_MM) {}
  protected:
  	virtual void setOutputPins(uint8_t mask) {
  		FastGPIO::Pin<PIN_Y_AXIS_STEPPER_B>::setOutput((mask & (1 << 0)) ? (HIGH ^ _pinInverted[0]) : (LOW ^ _pinInverted[0]));
  		FastGPIO::Pin<PIN_Y_AXIS_STEPPER_A>::setOutput((mask & (1 << 1)) ? (HIGH ^ _pinInverted[1]) : (LOW ^ _pinInverted[0]));
  	}
};

XStepper xAxisStepper;
YStepper yAxisStepper;

// some usefull functions

int xStepDirection, yStepDirection;
int prevYStepBy;

float getYSpeedAsMM() {
  return float(state[yAxisSpeed])/100;
}

void updateXAxisSpeed(float newSpeed = 0, bool pure = false) {
  float spd;
  if (!newSpeed) {
    spd = abs(xAxisStepper.speed());
  } else {
    spd = constrain(newSpeed, 0, STEPPER_MAX_SPEED);
    state[xAxisSpeed] = spd;
    if (!pure and spd <= 1) {
      spd /= 2; 
    }
  }
  xAxisStepper.setSpeedRemovePlay(speedRatio*spd*signedDir[xStepDirection]);
}

void updateYAxisSpeed(float newSpeed = 0, bool pure = false) {
  float spd;
  if (!newSpeed) {
    spd = abs(yAxisStepper.speed());
  } else {
    spd = constrain(newSpeed, 0, STEPPER_MAX_SPEED);
    state[yAxisSpeed] = spd;
    if (!pure and spd <= 1) {
      spd /= 2; 
    }
  }
  yAxisStepper.setSpeedRemovePlay(speedRatio*spd*signedDir[yStepDirection]);
}

void moveXAxisTo(long to, int dir = -1, int speed = 0) {
  if (dir != -1) xStepDirection = dir;
  xAxisStepper.moveTo((long(to) << 5)*signedDir[xStepDirection]); //round(float(to) * stepsToMM/100)
  updateXAxisSpeed(speed);
}
//
//void moveYAxisTo(long to, int dir = -1, int speed = 0) {
//  p(to)
//  if (dir != -1) yStepDirection = dir;
//  yAxisStepper.moveTo((to << 5)*signedDir[yStepDirection]); //round(float(to) * stepsToMM/100)
//  updateYAxisSpeed(speed);
//}

void moveYAxisBy(long by, int speed = 0) {
  prevYStepBy = by;
  yAxisStepper.moveTo(yAxisStepper.currentPosition()+(long(by) << 5));
  updateYAxisSpeed(speed);
}

void changeXAxisDirection(int speed = 0) {
  moveXAxisTo(xAxisStepper.currentPosition() ? 0 : state[xAxisTo], !xStepDirection, speed);
}

void changeYAxisDirection(int speed = 0) {
  yStepDirection = prevYStepBy*-1 > 0;
  moveYAxisBy(prevYStepBy*-1, speed);
}


#endif




