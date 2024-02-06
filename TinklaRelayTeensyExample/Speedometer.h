
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

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// If using software SPI (the default case):
#define OLED_MOSI   11
#define OLED_CLK   13
#define OLED_DC    7
#define OLED_CS    12
#define OLED_RESET 23
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

#define MAX_DISPLAY_BRIGHTNESS_PERCENTAGE 100
#define VPOS  10
#define HPOS  64

void speedometerSetup() {
  display.begin(SSD1306_SWITCHCAPVCC);
}

void drawCentreString(const char *buf)
{
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h); //calc width of new string
    display.setCursor(HPOS - w / 2, VPOS);
    display.print(buf);
}


void speedometerLoop() {
  if (tinklaRelay.tinklaRelayInitialized && (!tinklaRelay.rel_car_on)) {
    display.clearDisplay();
    return;
  }
  String spd = String((int)(tinklaRelay.rel_speed));
  display.clearDisplay();
  if (!tinklaRelay.tinklaRelayInitialized) {
    spd = "ERR";
    if (alternateON) {
      display.dim(true);
    } else {
      display.dim(false);
    }
  } else {
    if (tinklaRelay.rel_brightness <50) {
    display.dim(true);
  } else {
    display.dim(false);
  }
  }
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  drawCentreString(spd.c_str());
  display.display();
}


