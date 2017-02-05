#ifndef DEF_H
#define DEF_H

#define OFF 2
#define LEFT 0
#define RIGHT 1
#define BOTH 2

#define INSIDE 0
#define OUTSIDE 1

#define MODE_CUT 0
#define MODE_ANGLE 1
#define MODE_THREAD 2

#define STEPPER_MAX_PHYSICAL_SPEED 204
#define STEPPER_MAX_SPEED 204
#define STEPPER_MAX_SAFE_SPEED 120
#define STEPPER_CONTINUOUS_SAFE_SPEED 10
#define STEPS_PER_REVOLUTION 200 * 16

#define X_AXIS_PLAY_MM 0.35
#define Y_AXIS_PLAY_MM 0.05

//pots
#define PIN_X_AXIS_SPEED 7
#define PIN_Y_AXIS_SPEED 6
//switches
#define INDEX_X_AXIS_SWITCH_LEFT 6
#define INDEX_X_AXIS_SWITCH_RIGHT 7
#define PIN_Y_AXIS_SWITCH_LEFT A2
#define PIN_Y_AXIS_SWITCH_RIGHT A3
//stepper X
#define PIN_X_AXIS_STEPPER_POWER 6
#define PIN_X_AXIS_STEPPER_A 5
#define PIN_X_AXIS_STEPPER_B 4
//stepper Y
#define PIN_Y_AXIS_STEPPER_POWER 9
#define PIN_Y_AXIS_STEPPER_A 8
#define PIN_Y_AXIS_STEPPER_B 7
//shaft encoder
#define PIN_SHAFT_ENCODER_A 2
#define PIN_SHAFT_ENCODER_B 3
#define PIN_SHAFT_ENCODER_ZERO A5
//engine
#define PIN_ENGINE_POWER_OFF A1

#define PIN_RED A4

#define INTERRUPT_SHAFT_ENCODER_A 0
#define INTERRUPT_SHAFT_ENCODER_B 1
#define PIN_INTERRUPT_SHAFT_ENCODER_ZERO A5



#define modeEncoderValue 0
#define xAxis 1
#define xAxisTo 2
#define yAxis 3
#define yAxisTo 4
#define actionMode 5
#define cut 6
// from -90 to  90
#define angle 7 
#define threadDir 8
#define threadType 9
#define threadStep 10

#define rpm 11
#define xAxisSpeed 12
#define yAxisSpeed 13
#define shaftAngle 14


#define autoModeDelay 4000

bool program = false;
bool programChange = false;
//start with no action
bool halt = true;
bool disablePotsCheck = false;
bool disablePotY = false;
bool disablePotX = false;
bool forceXSpeedCheck = true;
bool forceYSpeedCheck = true;

unsigned int stepsToMM = STEPS_PER_REVOLUTION;
//unsigned int currentAccelSpeed = STEPPER_MAX_SAFE_SPEED;

float maxspeedSteps = 10880; // STEPPER_MAX_PHYSICAL_SPEED/60*STEPS_PER_REVOLUTION
float speedRatio = maxspeedSteps/STEPPER_MAX_SPEED;


int stateLimits[11][3] = {
	{1, 10},
	{0, 30000, 1},
	{0, 30000, 1},
	{0, 15000, 1},
	{0, 15000, 1}, 
	{0, 2}, //{MODE_CUT, MODE_ANGLE, MODE_THREAD}
	{0, 2}, //{LEFT, BOTH, RIGHT}
	{-900, 900, 1},
	{0, 1, 0}, //{RIGHT, LEFT}
	{0, 1, 0}, //{INSIDE, OUTSIDE}
	{0, 28} 
};

const int stateCount = 15;
volatile int oldState[stateCount] = {0}, state[stateCount] = { 0, 0, 0, 0, 0, MODE_CUT, LEFT, 0,  RIGHT, INSIDE, 5, 0, 0, 0, 0 };

typedef struct thread_temp {
	char label[10];
  float value;
} threadItem;

const int threadDefCount = 29;
threadItem threadDef[threadDefCount] = {
	{ "0.2mm ", 0.2 },
	{ "0.25mm", 0.25 },
	{ "0.35mm", 0.35 },
	{ "0.5mm ", 0.5 },
	{ "0.75mm", 0.75 },
	{ "1mm   ", 1 },
	{ "1.25mm", 1.25 },
	{ "1.5mm ", 1.5 },
	{ "1.75mm", 1.75 },
	{ "2mm   ", 2 },
	{ "2.25mm", 2.25 },
	{ "2.5mm ", 2.5 },
	{ "2.75mm", 2.75 },
	{ "3mm   ", 3 },
	{ "1/16\" ", 0.423 },
	{ "3/32\" ", 0.529 },
	{ " 1/8\" ", 0.635 },
	{ "5/32\" ", 0.793 },
	{ "3/16\" ",  1.05 },
	{ "7/32\" ", 1.058 },
	{ " 1/4\" ", 1.270 },
	{ "5/16\" ", 1.411 },
	{ " 3/8\" ", 1.588 },
	{ "7/16\" ", 1.814 },
	{ " 1/2\" ", 2.117 },
	{ " 5/8\" ", 2.309 },
	{ " 3/4\" ", 2.540 },
	{ " 7/8\" ", 2.822 },
	{ "   1\" ", 3.175 }
};

#endif

