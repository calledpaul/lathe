#include "LCD.h"

void _drawThread(int x, int y, int color) {
  int i, t, w = 5, h = 10;
  for(i=0; i<7; i++) {
    t = i*10;
    tft.drawLine(x+t+(i?0:w/2), y-(i?0:h/2), x+w+t, y-h, color);
    tft.drawLine(x+w+t, y-h, x+w*2+t, y, color);
  }
  t = i*10;
  tft.drawLine(x+t, y, x+w/2+t, y-h/2, color);
}

void drawRect(int x, int y, bool active = false) {
  tft.fillRect(x, y, 78, 26, active ? WHITE : BLACK);
  int color = active ? BLACK : WHITE;
  tft.drawRect(x+2, y+2, 30, 22, color);
  tft.drawRect(x+32, y+7, 40, 12, color);
}

void drawAngle(int x, int y, bool active = false) {
  tft.fillRect(x, y, 84, 26, active ? WHITE : BLACK);
  tft.drawTriangle(x+5, y+2, x+5, y+22, x+80, y+12, active ? BLACK : WHITE);
}

void drawThread(int x, int y, bool active = false) {
  tft.fillRect(x, y-18, 80, 25, active ? WHITE : BLACK);
  x += 2;
  int color = active ? BLACK : WHITE;
  _drawThread(x, y, color);
  _drawThread(x, y+1, color);
  _drawThread(x+1, y, color);
}


void drawBackground() {
  tft.drawRect(0, 0, 320, 240, WHITE);
  tft.drawRect(205, 0, 320, 40, WHITE);
  tft.drawLine(0, 80, 320, 80, WHITE);
  tft.drawLine(0, 160, 320, 160, WHITE);
  lcdtext(215, 15, "     RPM", WHITE, 2)
  tft.drawLine(0, 189, 320, 189, WHITE);  
  tft.drawLine(235, 160, 235, 189, WHITE);
  tft.drawLine(147, 160, 147, 189, WHITE);

  lcdtext(10, 52, "X", WHITE, 3)
  lcdtext(10, 132, "Y", WHITE, 3)
  lcdtext(170, 25, "mm", WHITE, 2)
  lcdtext(170, 57, "mm", WHITE, 2)
  //Diameter signs
  tft.drawCircle(180, 93, 6, WHITE);
  tft.drawLine(173, 100, 187, 87, WHITE);

  tft.drawCircle(180, 128, 6, WHITE);
  tft.drawLine(173, 135, 187, 122, WHITE);
  
  lcdtext(290, 45, "mm", WHITE, 2)
  lcdtext(170, 105, "mm", WHITE, 2)
  lcdtext(170, 137, "mm", WHITE, 2)
  lcdtext(305, 85, "o", WHITE, 1)
}


void dummy(int  v, bool active) { }

void inputXpos(int  v, bool active) { char buff[10]; dtostrf(float(v)/100, 6, 2, buff); lcdtext_def(20, 10, buff, GREEN, 4, active);}
void inputXDefPos(int  v, bool active) { char buff[10]; dtostrf(float(v)/100, 6, 2, buff); lcdtext_def(55, 50, buff, GREEN, 3, active);}
void inputYpos(int  v, bool active) { char buff[10]; dtostrf(float(v)/50, 6, 2, buff); lcdtext_def(20, 90, buff, YELLOW, 4, active);}
void inputYDefPos(int  v, bool active) { char buff[10]; dtostrf(float(v)/50, 6, 2, buff); lcdtext_def(55, 130, buff, YELLOW, 3, active); }

void inputCut(int v, bool active) {
  char strs[3][16] = { 
    " LEFT-INSIDE   ",
    " RIGHT-OUTSIDE ",
    " <> BOTH        "
  };
  lcdtext_def(125, 205, strs[v], CYAN, 2, active);
}
void inputAngle(int  v, bool active) { char buff[10]; dtostrf(float(v)/10, 6, 2, buff); lcdtext_def(80, 205, buff, CYAN, 2, active); }

bool forceSpeedRedraw = false;
void outputShaftSpeed(int v, bool active) {
  if (state[actionMode] == MODE_THREAD) {    
    int color = (state[rpm] < (STEPPER_MAX_SAFE_SPEED / threadDef[state[threadStep]].value)) ? BLACK : RED;
    tft.fillRect(207, 2, 60, 36, color);    
  } else if(forceSpeedRedraw) {
    tft.fillRect(207, 2, 60, 36, BLACK);
    forceSpeedRedraw = false;
  }
  char buff[6]; sprintf(buff, "%4d", v); lcdtext(215, 15, buff, CYAN, 2)
}

void inputThreadDir(int v, bool active) { lcdtext_def(60, 205, v ? "R" : "L", CYAN, 2, active); }
void inputThreadType(int v, bool active) { lcdtext_def(140, 205, v ? "IN " : "OUT", CYAN, 2, active); }
void inputThreadStep(int  v, bool active) { 
  lcdtext_def(245, 205, threadDef[v].label, CYAN, 2, active); 
  forceSpeedRedraw = true;
  outputShaftSpeed(state[rpm], false);
}

int modeSelectCache = 0;
int prevModeValueCache = -1;
void inputMode(int  v, bool active) { 
  //if (!modeSelectCache) {
    tft.fillRect(2, 162, 65, 27, active ? WHITE : BLACK);
    lcdtext(8, 168, "MODE:", active ? BLACK : WHITE, 2);
    tft.drawLine(67, 160, 67, 189, WHITE);
    modeSelectCache = 1;
  //}
  //if (prevModeValueCache == -1 || prevModeValueCache == MODE_CUT || v == MODE_CUT) {
    drawRect(69, 162, v == MODE_CUT);
  //}
  //if (prevModeValueCache == -1 || prevModeValueCache == MODE_ANGLE || v == MODE_ANGLE) {
    drawAngle(150, 162, v == MODE_ANGLE);
  //}
  //if (prevModeValueCache == -1 || prevModeValueCache == MODE_THREAD || v == MODE_THREAD) {
    drawThread(237, 180, v == MODE_THREAD);
  //}
  //prevModeValueCache = v;
  forceSpeedRedraw = true;
  outputShaftSpeed(state[rpm], false);
}


void outputShaftAngle(int v, bool active) {char buff[6]; sprintf(buff, "%3d", abs(round(v*0.087890625))); lcdtext(265, 87, buff, CYAN, 2) };
void outputXSpeed(int v, bool active) {char buff[6]; dtostrf(float(v)/100, 4, 2, buff); lcdtext(205, 50, buff, GREEN, 3) };
void outputYSpeed(int v, bool active) {char buff[6]; dtostrf(float(v)/100, 4, 2, buff); lcdtext(205, 130, buff, YELLOW, 3) };

void drawModeLabelBack() { tft.fillRect(2, 192, 316, 47, BLACK); }
void drawModeLabelCut() { drawModeLabelBack(); lcdtext(10, 205, "Direction:", WHITE, 2) }
void drawModeLabelAngle() { drawModeLabelBack(); lcdtext(10, 205, "Angle:", WHITE, 2) }
void drawModeLabelThread() { drawModeLabelBack(); lcdtext(10, 205, "Dir:", WHITE, 2) lcdtext(80, 205, "Type:", WHITE, 2) lcdtext(185, 205, "Step:", WHITE, 2) }
