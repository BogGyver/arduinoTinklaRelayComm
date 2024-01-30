#define DEBUG

/* USING SH1106/SSD1306 128x64 OLEDD DISPLAY WITH TEENSY 4.0 FOR DEBUG - With U8g2 library 
 *  - uncomment LCD_DEBUG definition below to use the LCD screen
 *  GND ==> GND
 *  VCC ==> 5V
 *  CLK ==> 13
 *  MOSI ==> 11
 *  RES ==> 23
 *  DC ==> 7
 *  CS ==> 12
 */

#define BOUD 115000

#ifdef LCD_DEBUG
#include <U8g2lib.h>
//#include <Arduino.h>
#ifdef U8X8_HAVE_HW_SPI
  #include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
  #include <Wire.h>
#endif
//U8G2_R2 is for rotation of 180 deg (U8G2_R0 - 0 deg, U8G2_R1 - 90 deg, U8G2_R2 - 180 deg, U8G2_R3 - 270 deg)
#ifndef u8g2
U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R2, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 12, /* dc=*/ 7, /* reset=*/ 23);  
#endif
#define LCD_LINES 4
#ifndef oldDbgTxt
String oldDbgTxt[4] = { "", "", "", ""};
#endif
#endif 


void showDebugTxt(String dbgTxt) {
  #ifdef LCD_DEBUG
  u8g2.clearBuffer();
  oldDbgTxt[LCD_LINES-1] = dbgTxt;
  for (int i = 0; i < LCD_LINES; i++) {
    u8g2.drawStr(10,12*(i + 1), oldDbgTxt[i].c_str());
    if (i < (LCD_LINES-1)) {
      oldDbgTxt[i] = oldDbgTxt[i+1];
    } else {
      oldDbgTxt[i] = "";
    }
  }
  u8g2.sendBuffer();
  #endif
  #ifdef SERIAL_DEBUG
  Serial.println(dbgTxt);
  #endif
}

void debug_setup() {
  Serial.begin(BOUD); // opens serial port, sets data rate to BOUD bps
  #ifdef LCD_DEBUG
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_samim_10_t_all);
  #endif
  showDebugTxt("Started...");  
}


