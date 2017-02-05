#include <Adafruit_GFX.h>
#include <Adafruit_TFTLCD.h>
//#include <Fonts/FreeMono9pt7b.h>
#include "Colors.h"

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4


Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define lcdtext_def(x, y, text, color, size, active) tft.setCursor(x, y); tft.setTextColor( active ? BLACK : color, active ? WHITE : BLACK); tft.setTextSize(size); tft.print(text);
#define lcdtext(x, y, text, color, size) lcdtext_def(x, y, text, color, size, false)

void initLCD() {
	tft.reset();
  uint16_t identifier = tft.readID();
  tft.setRotation(3);
  tft.begin(identifier);
  //tft.setFont(&FreeMono9pt7b);
}





