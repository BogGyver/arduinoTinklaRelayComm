#include <Adafruit_NeoPixel.h>

#define PIN 6
#define MAX_BRIGHTNESS_PERCENTAGE 60 //scale car display brightness by this percentage

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

//This is an example on how to use the 12x wheel to show data from Tinkla Relay
#define CONTROLLED_PIXELS 12
uint32_t pixelColors[12];
#define PWR_ORANGE        strip.Color(252, 140, 3)
#define PWR_GREEN         strip.Color(0, 255, 0)
#define AP_TOURQUISE      strip.Color(3, 215, 252)
#define BSM_RED           strip.Color(255, 0, 0)
#define LED_OFF           strip.Color(10,10,10) 
#define LED_WHITE         strip.Color(127, 127, 127)
//this is just to be able to rotate them later if we want to
//positive clockwise, negative counter clockwise, allows one to rotate easily the design. #3 is at 9 o'clock, #0 is at 6 o'clock
#define PIXEL_SHIFT -5
const uint8_t posToPixelNo[] = {7,8,9,10,11,0,1,2,3,4,5,6};

void neoPixelSetup() {
  strip.begin();
  if (tinklaRelay.rel_car_on) {
    strip.setBrightness((uint8_t)(tinklaRelay.rel_brightness * MAX_BRIGHTNESS_PERCENTAGE / 100));
  } else {
    strip.setBrightness(0);
  }
  strip.show(); // Initialize all pixels to 'off'
}

void setPixelColor(uint8_t pos, uint32_t pixelColor) {
  pixelColors[(CONTROLLED_PIXELS + pos + PIXEL_SHIFT) % CONTROLLED_PIXELS] = pixelColor;
}

void neoPixelComputeValues() {
  //first we turn them all off
  for (uint8_t i=0; i< 12;i++ ) {
    setPixelColor(i, LED_OFF);
  }
  //now we show the power used 
  //from 1 to 3 would be regen, from 3 to 11 would be power consumed
  if (tinklaRelay.rel_power_lvl < 0) {
    uint8_t m = 3 - (uint8_t)(-tinklaRelay.rel_power_lvl * 2 / 55);
    for (uint8_t i=3; i>=m;i-- ) {
      setPixelColor(i, PWR_GREEN);
    }
  } else {
    if (tinklaRelay.rel_power_lvl > 0) {
      uint8_t m = 3 +(uint8_t)(tinklaRelay.rel_power_lvl * 8 / 300);
      for (uint8_t i=3; i<=m;i++ ) {
        setPixelColor(i, PWR_ORANGE);
      }
    }  else {
      //pwr is exact 0, so most likely car stopped
      setPixelColor(3, LED_WHITE);
    }
  }
  //now let's do BSM over 3 and 9
  if (tinklaRelay.rel_left_side_bsm) {
    setPixelColor(3, BSM_RED);
  }
  if (tinklaRelay.rel_right_side_bsm) {
    setPixelColor(9, BSM_RED);
  }
  //if AP is enabled lets turn 0 blue
  if (tinklaRelay.rel_AP_on) {
    setPixelColor(0, AP_TOURQUISE);
  }
}

void neoPixelLoop() {
  if (tinklaRelay.rel_car_on) {
    neoPixelComputeValues();
    strip.setBrightness((uint8_t)(tinklaRelay.rel_brightness * MAX_BRIGHTNESS_PERCENTAGE / 100));
  } else {
    if (!tinklaRelay.tinklaRelayInitialized) {
      for (uint8_t i=0; i< 12;i++ ) {
        if (i % 2 == 0) {
          setPixelColor(i, alternateON ? LED_OFF : AP_TOURQUISE );
        } else {
          setPixelColor(i, alternateON ? AP_TOURQUISE : LED_OFF);
        }
      }
      setPixelColor(3, BSM_RED);
      strip.setBrightness((uint8_t)(tinklaRelay.rel_brightness * MAX_BRIGHTNESS_PERCENTAGE / 100));
    } else {
      strip.setBrightness(0);
      strip.show();
      return;
    }
  }
  for(uint16_t i=0; i<CONTROLLED_PIXELS; i++) {
    strip.setPixelColor(i, pixelColors[i]);
  }
  strip.show();
}