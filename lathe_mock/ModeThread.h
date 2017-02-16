#ifndef MODE_THREAD_H
#define MODE_THREAD_H

#include <Utils.h>
#include "ShaftEncoder.h"
#include "ModeCut.h"

long cutDepth = 0;
float precomputedStepRatio, shaftAgg = 0;
bool triggerTheadingStart = false;
int encoderTickDone = 0;

void followShaft(int delta) {
	encoderTickDone = 1;
	float togo = shaftAgg + precomputedStepRatio;
  int togoInt = int(togo);  
  shaftAgg = togo - togoInt;
  togoInt = abs(togoInt);
  while (togoInt--) {
    xAxisStepper.runStep(togo > 0);
    delayMicroseconds(10);
  }
}

long _calculateNextThreadYStep() {
  long fullDistance = abs(yAxisStepper.distanceToGo());
  //pp("fulldistance", "", fullDistance >> 5)
  if (!fullDistance) return 0;
  //different cut step gradient possible
  float step = round((cutDepth > fullDistance ? 0.05 : 0.1) * stepsToMM);
  return (fullDistance - step) > 0 ? step : fullDistance;
}

void stepThreadCutter() {
	int step = _calculateNextThreadYStep();
	cutDepth += step;
	//pp("in+", " ", step >> 5)
	pp("cut_depth", " ", cutDepth >> 5)
	yAxisStepper.stepContinuous(step);
}

void stepOutThreadCutter(bool clean = false) {	
	if (clean) {
		yStepDirection = !state[threadType];
		updateYAxisSpeed(STEPPER_MAX_SAFE_SPEED);
		yAxisStepper.stepContinuous(cutDepth-round(Y_AXIS_PLAY_MM*stepsToMM), false);
	} else {
		changeYAxisDirection(STEPPER_MAX_SAFE_SPEED);
		//pp("out", " ", cutDepth >> 5)
		yAxisStepper.stepContinuous(cutDepth);
	}
}

void stepInThreadCutter() {
	changeYAxisDirection(STEPPER_MAX_SAFE_SPEED);
	//pp("in", " ", cutDepth >> 5)
	yAxisStepper.stepContinuous(cutDepth); //+ 0.1
}

bool canThreadingStart() {
	return state[rpm] < STEPPER_MAX_SAFE_SPEED / threadDef[state[threadStep]].value;
}

void threadingStart() {
	if (canThreadingStart()) {
		precomputedStepRatio = (state[threadDir] == RIGHT ? -0.78125 : 0.78125) * threadDef[state[threadStep]].value;
		triggerTheadingStart = true;
		encoderTickDone = 0;
	}
}

void shaftZeroPointTick() {
	if (triggerTheadingStart && !shaftEncoder.tickCallbackEnabled) {
		shaftEncoder.tickCallbackEnabled = true;
		triggerTheadingStart = false;
	}
}

float getThreadDepth() {
	return (state[threadStep] < 14 ? 0.614 : 0.64037) * threadDef[state[threadStep]].value;
}

void initThreadCutDepth() {
	if (!cutDepth) {
		float depth = getThreadDepth();
		state[yAxisTo] = round(depth*100);
		//p(round(depth*signedDir[state[threadType]]*100))
    moveYAxisBy(round(depth*signedDir[state[threadType]]*100), STEPPER_MAX_SAFE_SPEED);
    stepThreadCutter();
		fullCutModeLastRound = false;
		threadingStart();
  }
}

void processModeThreadAuto(bool init) {
  if (init) {
  	xAxisStepper.setCurrentPosition(0);
    moveXAxisTo(state[xAxisTo], !state[threadDir], getModeCutXNextSpeed());
    initThreadCutDepth();
    disablePotsCheck = true;
  }
  if (yAxisStepper.distanceToGo() != 0 || fullCutModeLastRound) {
	  if (xAxisStepper.distanceToGo() == 0 && encoderTickDone) {
	  	shaftEncoder.tickCallbackEnabled = false;
	  	ignoreREDButton = true;
			stepOutThreadCutter(fullCutModeLastRound);
			if (fullCutModeLastRound) {
				fullCutModeLastRound = false;

				float rotationsOfShaftPerSecond = float(state[rpm])/60;
				int cleanSpd = round(16 * rotationsOfShaftPerSecond);

				changeXAxisDirection(cleanSpd);
				xAxisStepper.stepContinuousToDestination();
				programCorrectEnd();
			} else {				
				changeXAxisDirection(STEPPER_MAX_SAFE_SPEED);
				xAxisStepper.stepContinuousToDestination();
				changeXAxisDirection(STEPPER_MAX_SAFE_SPEED);
				stepInThreadCutter();
				stepThreadCutter();
				
				if (!yAxisStepper.distanceToGo()) {
					fullCutModeLastRound = true;
				}
				threadingStart();
			}
			ignoreREDButton = false;
	  }  
  }
}

void processModeThreadManual(bool init) {
	if (init) {
		xAxisStepper.setCurrentPosition(0);
		if (state[xAxisTo]) {
			moveXAxisTo(state[xAxisTo], !state[threadDir], getModeCutXNextSpeed());
		}
		disablePotX = true;
		threadingStart();
	}
	if (state[xAxisTo] && xAxisStepper.distanceToGo() == 0 && encoderTickDone) {
		programActive(false);
	}
}

void processModeThread(bool init, bool end) {
	if (end) {
		ignoreREDButton = true;
		disablePotsCheck = true;
		int cutterBuffer = cutDepth ? cutDepth : round(getThreadDepth()*stepsToMM);		
		if (encoderTickDone) {
	  	shaftEncoder.tickCallbackEnabled = false;
			yStepDirection = !state[threadType];
	  	updateYAxisSpeed(STEPPER_MAX_SAFE_SPEED);
			yAxisStepper.stepContinuous(cutterBuffer, false);
		}
		moveXAxisTo(0, state[threadDir], STEPPER_MAX_SAFE_SPEED);
		xAxisStepper.stepContinuousToDestination();

		if (encoderTickDone && !cutDepth) {
			yStepDirection = state[threadType];
	  	updateYAxisSpeed(STEPPER_MAX_SAFE_SPEED);
			yAxisStepper.stepContinuous(cutterBuffer, false);
		}
		cutDepth = 0;
		disablePotsCheck = false;
		ignoreREDButton = false;
	} else {
	  if (state[xAxisTo] && state[yAxisTo]) {
	    processModeThreadAuto(init);
	  } else {
	    processModeThreadManual(init);
	  }
	}
}

#endif




