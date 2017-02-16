#ifndef MODE_ANGLE_H
#define MODE_ANGLE_H

#include <Utils.h>
#include "ModeCut.h"
#include "Inputs.h"

int prevAngle = 0, originalXAxisTo;
float angleRatio = 0;
bool finalizeAutoAngleCut = false;

int calculateAngle() {
	return round(atan(float(abs(state[yAxisTo]-state[yAxis]))/state[xAxisTo])*1800/3.14);
}

float getAxisRatio() {
	if (prevAngle != state[angle]) {
		prevAngle = state[angle];
		angleRatio = tan(3.14 * abs(float(state[angle])/10)/180);
	}
	return angleRatio;
}


void setVarioSpeed() {
	float refYSpeed, refXSpeed;
	int inputSpeed = readPotY();
	if (inputSpeed > STEPPER_MAX_SAFE_SPEED) {
		inputSpeed = STEPPER_MAX_SAFE_SPEED;
	}
	if (getAxisRatio() > 1) {
		refYSpeed = abs(state[angle]) < 895 ? inputSpeed : 0;
		refXSpeed = refYSpeed ? refYSpeed/getAxisRatio() : inputSpeed;
	} else {
		refXSpeed = abs(state[angle]) < 895 ? inputSpeed : 0;
		refYSpeed = refXSpeed ? refXSpeed*getAxisRatio() : inputSpeed;
	}
	//pp(refYSpeed, ' ', refXSpeed)
	updateXAxisSpeed(refXSpeed);
	updateYAxisSpeed(refYSpeed);
}
//noisy mind
void processModeAngleAuto(bool init, bool end) {
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
				programCorrectEnd();
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

void processModeAngleManual(bool init, bool end) {
	checkYSwitchAction.check();
  if (ySwitchState != OFF) {
		disablePotsCheck = true;

		xStepDirection = ySwitchState == INSIDE ? RIGHT : LEFT;
		if (state[angle] < 0) {
			xStepDirection = !xStepDirection;
		}
		yStepDirection = ySwitchState;
		setVarioSpeed();
		
		if (!halt) {
			xAxisStepper.runSpeed();
			yAxisStepper.runSpeed();
		}
  }	else {
  	disablePotsCheck = false;
  	processManualX();
  }
}

void processModeAngle(bool init, bool end) {
  if (state[angle]) {
  	if (state[xAxisTo] && state[yAxisTo]) {
      processModeAngleAuto(init, end);
    //} else if (state[yAxisTo]) {
    //	processModeAngleSemiAuto(init);
    } else {
      processModeAngleManual(init, end);
    }
  }
}

#endif




