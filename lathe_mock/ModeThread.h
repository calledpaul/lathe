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
	//pp("cut_depth", " ", cutDepth >> 5)
	yAxisStepper.stepContinuous(step);
}

void stepOutThreadCutter(bool clean = false) {	
	changeYAxisDirection(STEPPER_MAX_SAFE_SPEED);
	// pp("out", " ", (cutDepth + (clean ? 0 : 320)) >> 5)
	// yAxisStepper.stepContinuous(cutDepth + (clean ? 0 : 320)); //+ 0.1
	//pp("out", " ", cutDepth >> 5)
	yAxisStepper.stepContinuous(cutDepth); //+ 0.1
}

void stepInThreadCutter() {
	changeYAxisDirection(STEPPER_MAX_SAFE_SPEED);
	//pp("in", " ", cutDepth >> 5)
	yAxisStepper.stepContinuous(cutDepth); //+ 0.1
}

void threadingStart() {
	if (state[rpm] < STEPPER_MAX_SAFE_SPEED / threadDef[state[threadStep]].value) {
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

void initThreadCutDepth() {
	if (!cutDepth) {
		float depth = (state[threadStep] < 14 ? 0.614 : 0.64037) * threadDef[state[threadStep]].value;
		p(round(depth*signedDir[state[threadType]]*100))
    moveYAxisBy(round(depth*signedDir[state[threadType]]*100), STEPPER_MAX_SAFE_SPEED);
    cutDepth = 0;
    stepThreadCutter();
		fullCutModeLastRound = false;
		threadingStart();
  }
}

void processModeThreadAuto(bool init) {
  if (init) {
    initXStepperCut();
    initThreadCutDepth();
    disablePotsCheck = true;
  }
  if (yAxisStepper.distanceToGo() != 0 || fullCutModeLastRound) {
	  if (xAxisStepper.distanceToGo() == 0 && encoderTickDone) {
	  	shaftEncoder.tickCallbackEnabled = false;
			stepOutThreadCutter(fullCutModeLastRound);
			if (fullCutModeLastRound) {
				fullCutModeLastRound = false;

				float rotationsOfShaftPerSecond = float(state[rpm])/60;
				int cleanSpd = round(16 * rotationsOfShaftPerSecond);
				//pp("speed", " ", cleanSpd)
				changeXAxisDirection(cleanSpd);
				xAxisStepper.stepContinuousToDestination();
				//updateXAxisSpeed(STEPPER_MAX_SAFE_SPEED);
				//xAxisStepper.stepMM(20);
				stopEngine();
		    program = false;

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
	  }  
  }
}

void processModeThreadManual(bool init) {
	if (init) {
		threadingStart();
	}
}

void processModeThread(bool init) {
  if (state[xAxisTo]) {
    processModeThreadAuto(init);
  } else {
    processModeThreadManual(init);
  }
}

#endif




