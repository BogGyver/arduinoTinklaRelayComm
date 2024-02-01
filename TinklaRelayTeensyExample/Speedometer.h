
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

#include <U8g2lib.h>
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

#define MAX_DISPLAY_BRIGHTNESS_PERCENTAGE 100
#define VPOS  30
#define HPOS  64

void speedometerSetup() {
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub35_tn);
  u8g2.setFontPosCenter();
}

void speedometerLoop() {
  if (!tinklaRelay.rel_car_on) {
    u8g2.clearBuffer();
    u8g2.setContrast(0);
    u8g2.sendBuffer();
    return;
  }
  String spd = String((int)(tinklaRelay.rel_speed));
  u8g2.clearBuffer();
  uint8_t x = (uint8_t)(u8g2.getUTF8Width(spd.c_str())/2);
  u8g2.setContrast((uint8_t)(tinklaRelay.rel_brightness * 2.55 * MAX_DISPLAY_BRIGHTNESS_PERCENTAGE / 100));
  u8g2.drawStr(HPOS - x,VPOS, spd.c_str());
  u8g2.sendBuffer();
}


