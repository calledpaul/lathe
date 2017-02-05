#include <Utils.h>
#include <SPI_anything.h>
#include "DrawFuncs.h"

typedef struct tag_name {   
   callbackFnIntBool renderer;
   bool active;
} drawItem;

const int componentsSize = 15;
drawItem components[componentsSize] = {
  {dummy},
  {inputXpos, true},
  {inputXDefPos},
  {inputYpos},
  {inputYDefPos},
  {inputMode},
  {inputCut},
  {inputAngle},
  {inputThreadDir},
  {inputThreadType},
  {inputThreadStep},
  {outputShaftSpeed},
  {outputXSpeed},
  {outputYSpeed},
  {outputShaftAngle}
};
volatile int stateChange[componentsSize];

void render(int comp, bool reg = true) {
  int snapshot = state[comp];
  int val = state[comp];

  if ((comp == xAxisSpeed) && state[rpm]) {
    float rotationsOfShaftPerSecond = float(state[rpm])/60;
    val = round(float(val)*STEPPER_MAX_PHYSICAL_SPEED/STEPPER_MAX_SPEED / rotationsOfShaftPerSecond);
  }

  components[comp].renderer(val, components[comp].active);
  if (snapshot == state[comp]) {
    stateChange[comp] = 0;
  }
}

void renderSelection(int value, bool active) {
  int n, activate;
  modeSelectCache = 0;
  activate = state[modeEncoderValue];
  // first deactivate all (except needed one, if it's for some reason active)
  for(n=1; n < componentsSize; n++) {
    if (components[n].active && n != activate) {
      components[n].active = false;
      render(n);
    }
  }
  //then activate needed one
  if (!components[activate].active) {
    components[activate].active = true;
    render(activate);
  }
  if (activate == state[modeEncoderValue]) {
    stateChange[modeEncoderValue] = 0;
  }
}

void draw() {
  int i;
  //long bu = millis();
  for(i=0; i < componentsSize; i++) {
    if (stateChange[i]) {
      render(i);
      if (i == actionMode) {
        switch (state[i]) {
          case MODE_CUT:
            drawModeLabelCut();
            render(cut);
            break;
          case MODE_ANGLE:
            drawModeLabelAngle();
            render(angle);
            break;
          case MODE_THREAD:
            drawModeLabelThread();
            render(threadDir);
            render(threadType);
            render(threadStep);
            break;
        }
      }
    }
  }
  //Serial.println(millis()-bu);
}

TimedAction drawAction = TimedAction(100, draw);

void setup(void) {
  Serial.begin(9600);
  initLCD();
  tft.fillScreen(BLACK);
  //lcdtext(100, 80, "Happy", GREEN, 3)
  //lcdtext(100, 140, "machining ;)", GREEN, 3)
  //delay(3000);
  drawBackground();
  components[0].renderer = renderSelection;
  drawModeLabelCut();
  for(int i=1; i < componentsSize; i++) if (i<7 || i > 10) render(i);
    
  SPCR |= bit (SPE);
  pinMode(MISO, OUTPUT);
  SPI.attachInterrupt();
}

void loop () {
  drawAction.check();
}

ISR (SPI_STC_vect) {
  SPI_readAnything_ISR (message);
  state[message.mixed] = message.data;
  stateChange[message.mixed] = 1;
}
