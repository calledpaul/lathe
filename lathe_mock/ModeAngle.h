#ifndef MODE_ANGLE_H
#define MODE_ANGLE_H

#include <Utils.h>
#include "ModeCut.h"

int prevAngle = 0, originalXAxisTo;
float angleRatio = 0;
bool finalizeAutoAngleCut = false;

float calculateAngle() {

}

float getAxisRatio() {
	if (prevAngle != state[angle]) {
		prevAngle = state[angle];
		angleRatio = float(abs(state[angle]))/900;
	}
	return angleRatio;
}

int getAngleYAxisDirection() {
	int dir = xStepDirection == RIGHT ? INSIDE : OUTSIDE;
	if (state[angle] < 0) {
		dir = !dir;
	}
	return dir;
}

void setVarioSpeed() {
	int refXSpeed = state[xAxisSpeed], refYSpeed;
	if (refXSpeed > STEPPER_MAX_SAFE_SPEED) {
		refXSpeed = STEPPER_MAX_SAFE_SPEED;
	}
	refYSpeed = getAxisRatio()*refXSpeed;
	if (refYSpeed > STEPPER_MAX_SAFE_SPEED) {
		refYSpeed = STEPPER_MAX_SAFE_SPEED;
		refXSpeed = refYSpeed/getAxisRatio();	
	}

	if (state[xAxisSpeed] != refXSpeed) {
		updateXAxisSpeed(refXSpeed*signedDir[xStepDirection]);
	}
	if (state[yAxisSpeed] != refYSpeed) {
		updateYAxisSpeed(refYSpeed*signedDir[getAngleYAxisDirection()]);
	}
}
//noisy mind
void processModeAngleAuto(bool init) {
	if (init) {
		originalXAxisTo = state[xAxisTo];
		state[cut] = LEFT;
		finalizeAutoAngleCut = false;
		disablePotY = true;
	}
	if (finalizeAutoAngleCut) {
			setVarioSpeed();
			xAxisStepper.runSpeedToPosition();
			yAxisStepper.runSpeedToPosition();
			if (yAxisStepper.distanceToGo() == 0 || xAxisStepper.distanceToGo() == 0) {
				program = false;
    		stopEngine();
			}
	} else {
		processFullCutMode(init, true, getAxisRatio());

		//init finalize
		if (yAxisStepper.distanceToGo() == 0 && !fullCutModeLastRound) {
			state[xAxisTo] = originalXAxisTo;
			moveXAxisTo(state[xAxisTo], LEFT);
			changeYAxisDirection();
			finalizeAutoAngleCut = true;
		}
	}
}

void processModeAngleManual(bool init) {
	if (init) {
		disablePotY = true;		
	}
	checkYSwitchAction.check();
  if (ySwitchState != OFF) {  	

		xStepDirection = ySwitchState == INSIDE ? RIGHT : LEFT;
		if (state[angle] < 0) {
			xStepDirection != xStepDirection;
		}
		setVarioSpeed();
		if (!halt) {
			xAxisStepper.runSpeed();
			yAxisStepper.runSpeed();
		}
  }	else {
  	processManualX();
  }
}

void processModeAngle(bool init) {
  if (state[angle]) {
  	if (state[xAxisTo] && state[yAxisTo]) {
      processModeAngleAuto(init);
    //} else if (state[yAxisTo]) {
    //	processModeAngleSemiAuto(init);
    } else {
      processModeAngleManual(init);
    }
  }
}

#endif




