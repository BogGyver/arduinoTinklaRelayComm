#define SERIAL_DEBUG

bool alternateON = false;
int counter = 0;

#include "TinklaRelayUsbHost.h"

//****** begin led globals and functions ******//

#include <OctoWS2811.h>
#include <cmath> // For exp()

#define RED    0xFF0000
#define GREEN  0x00FF00
#define BLUE   0x0000FF
#define AQUA   0x00FFFF
#define YELLOW 0xFFFF00
#define PINK   0xFF00FF
#define WHITE  0xFFFFFF
#define BLACK  0x000000
#define MAXIMUM_BRIGHTNESS  0.25
#define MINIMUM_BRIGHTNESS  0.01

const int max_led = 12;
const int num_pins = 1; 			    // Using only one pin
byte pin_list[num_pins] = {2}; 		// Change this if you use a different pin, based on your Teensy model

DMAMEM int display_memory[max_led*6];
int drawing_memory[max_led*6];
const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(max_led, display_memory, drawing_memory, config, num_pins, pin_list);

int edge_leds[max_led*6]; 			// RGBW needs 32 bytes, RGB needs only 24
int center_leds[max_led*6]; 		// RGBW needs 32 bytes, RGB needs only 24

int max_status = 200;
int four_step_status = max_status / 4;
int twelve_step_status = max_status / 12;
int tacc_status = 0;            // 0 = off, 1-... = animation, max_status = on
int AP_status = 0;              // 0 = off, 1-... = animation, max_status = on
int tacc_ready_status = 0;      // 0 = off, 1-... = animation, max_status = on
int AP_ready_status = 0;        // 0 = off, 1-... = animation, max_status = on
int left_bsm_status = 0;        // 0 = off, 1 = animation, 2 = on
int right_bsm_status = 0;       // 0 = off, 1 = animation, 2 = on
int charging_status = 0;        // 0 = off, 1-... = animation
int left_signal_status = 0;     // 0 = off, 1 = on
int right_signal_status = 0;    // 0 = off, 1 = on
bool animation = false;

int LED_AP_on(int AP_status) {
  if (AP_status >= max_status) {
    for (int i=2; i < leds.numPixels()-2; i++) {
      center_leds[i] = AQUA;
      animation = false;
    }
    return max_status;
  }
  else {
    int j = (AP_status / four_step_status) + 3;
    for (int i=2; i < j; i++) {
      center_leds[i] = AQUA;
      center_leds[leds.numPixels()-i-1] = AQUA;
      animation = true;
    }
    return AP_status + 1;
  }
}

int LED_tacc_on(int tacc_status) {
  if (tacc_status >= max_status) {
    for (int i=2; i < leds.numPixels()-2; i++) {
      center_leds[i] = GREEN;
      animation = false;
    }
    return max_status;
  }
  else {
    int j = (tacc_status / four_step_status) + 3;
    for (int i=2; i < j; i++) {
      center_leds[i] = GREEN;
      center_leds[leds.numPixels()-i-1] = GREEN;
      animation = true;
    }
    return tacc_status + 1;
  }
}

int LED_AP_ready(int AP_ready_status) {
  if (AP_ready_status >= max_status) {
    for (int i=2; i < leds.numPixels()-2; i++) {
      center_leds[i] = YELLOW;
      animation = false;
    }
    return max_status;
  }
  else {
    int j = (AP_ready_status / four_step_status) + 3;
    for (int i=2; i < j; i++) {
      center_leds[i] = YELLOW;
      center_leds[leds.numPixels()-i-1] = YELLOW;
      animation = true;
    }
    return AP_ready_status + 1;
  }
}

int LED_tacc_ready(int tacc_ready_status) {
  if (tacc_ready_status >= max_status) {
    for (int i=2; i < leds.numPixels()-2; i++) {
      center_leds[i] = YELLOW;
      animation = false;
    }
    return max_status;
  }
  else {
    int j = (tacc_ready_status / four_step_status) + 3;
    for (int i=2; i < j; i++) {
      center_leds[i] = YELLOW;
      center_leds[leds.numPixels()-i-1] = YELLOW;
      animation = true;
    }
    return tacc_ready_status + 1;
  }
}

int LED_charging_on(int charging_status) {
  if (charging_status <= max_status) {
    int j = (charging_status / twelve_step_status);
    center_leds[leds.numPixels() - j] = AQUA;
  }
  else if (charging_status <= (max_status * 2)) {
    int j = (charging_status / twelve_step_status) - max_status;
    center_leds[j] = YELLOW;
  }
  else {
    charging_status = 0;
  }
  return charging_status + 1;
}

void LED_right_bsm_on() {
  edge_leds[0] = RED;
  edge_leds[1] = RED;
  edge_leds[2] = RED;
}

void LED_right_bsm_off() {
  edge_leds[0] = BLACK;
  edge_leds[1] = BLACK;
  edge_leds[2] = BLACK;
}

void LED_left_bsm_on() {
  edge_leds[max_led-1] = RED;
  edge_leds[max_led-2] = RED;
  edge_leds[max_led-3] = RED;
}

void LED_left_bsm_off() {
  edge_leds[max_led-1] = BLACK;
  edge_leds[max_led-2] = BLACK;
  edge_leds[max_led-3] = BLACK;
}

void LED_right_signal_on() {
  edge_leds[0] = GREEN;
  edge_leds[1] = GREEN;
}

void LED_right_signal_off() {
  edge_leds[0] = BLACK;
  edge_leds[1] = BLACK;
}

void LED_left_signal_on() {
  edge_leds[max_led-1] = GREEN;
  edge_leds[max_led-2] = GREEN;
}

void LED_left_signal_off() {
  edge_leds[max_led-1] = BLACK;
  edge_leds[max_led-2] = BLACK;
}

void LED_startup() {
  for (int i=0; i < leds.numPixels()-1; i++) {
    center_leds[i] = WHITE;
    animation = true;
  }
}

void LED_drive() {
  center_leds[2] = BLUE;
  center_leds[9] = BLUE;
}

void LED_reverse() {
  center_leds[2] = PINK;
  center_leds[9] = PINK;
}

void LED_show() {
  float my_brightness = set_brightness();

  // debug prints
  //for (int i = 0; i < leds.numPixels(); i = i + 1) { Serial.print(center_leds[i], HEX); Serial.print(", "); } Serial.println();
  //for (int i = 0; i < leds.numPixels(); i = i + 1) { Serial.print(edge_leds[i], HEX); Serial.print(", "); } Serial.println();

  // loop through and combine the edge and center arrays into a final output
  for (int i = 0; i < leds.numPixels(); i = i + 1) { 
    if (edge_leds[i] != BLACK) {   // animations overlay on top of the center patterns
      leds.setPixel(i, decrease_brightness(edge_leds[i], my_brightness));
    }
    else {
      float temp_brightness = my_brightness;
      if (!animation) {
        temp_brightness = my_brightness / 2;
      }
      leds.setPixel(i, decrease_brightness(center_leds[i], temp_brightness));
    }
  }
  leds.show(); 
}

float set_brightness() {
  int tinkla_bright = constrain(tinklaRelay.rel_brightness, 0, 100);
  //int tinkla_bright = 10;     // for manual debugging
  float normalized_bright = tinkla_bright / 100.0;

  //float curved_bright = pow(normalized_bright, 1.5);      // exponential brightness

  float k = 10; // Adjust this value to control the steepness of the curve
  float x = 0.5; // Midpoint of the sigmoid, assuming normalized_bright is in [0, 1]
  float curved_bright = 1.0f / (1.0f + exp(-k * (normalized_bright - x)));    // S-curve brightness

  float final_bright = ((MAXIMUM_BRIGHTNESS-MINIMUM_BRIGHTNESS) * curved_bright) + MINIMUM_BRIGHTNESS;
  return constrain(final_bright, MINIMUM_BRIGHTNESS, MAXIMUM_BRIGHTNESS);
}

uint32_t decrease_brightness(uint32_t color, float factor) {
  uint8_t red = (color >> 16) * factor;
  uint8_t green = ((color >> 8) & 0xFF) * factor;
  uint8_t blue = (color & 0xFF) * factor;
  return (red << 16) | (green << 8) | blue;
}

void LED_center_off() {
  for (int i=0; i < leds.numPixels(); i++) {
    center_leds[i] = BLACK;
  }
}

void LED_edge_off() {
  for (int i=0; i < leds.numPixels(); i++) {
    edge_leds[i] = BLACK;
  }
}

void LED_off() {
  for (int i=0; i < leds.numPixels(); i++) {
    leds.setPixel(i, BLACK);
    leds.show();
  }
}

void LED_debug_brightness() {
  int debug_brightness = 12 * tinklaRelay.rel_brightness / 100;
  for (int i=0; i < debug_brightness; i++) {
    center_leds[i] = YELLOW;
  }
}

//****** end led globals and functions ******//

void setup() {
  tinklaRelayusbHostSetup();

  //YOUR CODE STARTS HERE ***************************************
  Serial.begin(9600);
  //digitalWrite(13, HIGH);       // light the onboard LED to show power is on

  leds.begin();
  LED_center_off();
  LED_edge_off();
  LED_off();
  LED_startup();
  delay(1000);
  //leds.show();
  //YOUR CODE ENDS HERE ***************************************
}

void loop() {
  tinklaRelayusbHostLoop();
  //access all Tinkla Relay variables as tinklaRelay.rel_variable_you_want as defined in the TinklaRelayDriver.h
  if (!tinklaRelay.tinklaRelayInitialized) delay(100);
  counter = (counter + 1 ) % 10;
  if (counter == 0) alternateON = !alternateON;

  //YOUR CODE STARTS HERE ***************************************
  // translating relay variables to my led code variables

  // insert AP_ready_status and tacc_ready_status code here 
  // insert drive and reverse too

  if (tinklaRelay.rel_AP_on) {
    if (AP_status == 0) {
      AP_status = 1;
    }
  }
  else {
    if (AP_status > 0) {
      AP_status = 0;
    }
  }

  if (tinklaRelay.rel_tacc_only_active) {
    if (tacc_status == 0) {
      tacc_status = 1;
    }
  }
  else {
    if (tacc_status > 0) {
      tacc_status = 0;
    }
  }

  if (tinklaRelay.rel_car_charging) {
    if (charging_status == 0) {
      charging_status = 1;
    }
  }
  else {
    if (charging_status > 0) {
      charging_status = 0;
    }
  }

  if (tinklaRelay.rel_right_side_bsm) {
    if (right_bsm_status == 0) {
      right_bsm_status = 1;
    }
  }
  else {
    if (right_bsm_status > 0) {
      right_bsm_status = 0;
    }
  }

  if (tinklaRelay.rel_left_side_bsm) {
    if (left_bsm_status == 0) {
      left_bsm_status = 1;
    }
  }
  else {
    if (left_bsm_status > 0) {
      left_bsm_status = 0;
    }
  }

  if (tinklaRelay.rel_right_turn_signal) {
    if (right_signal_status == 0) {
      right_signal_status = 1;
    }
  }
  else {
    if (right_signal_status > 0) {
      right_signal_status = 0;
    }
  }

  if (tinklaRelay.rel_left_turn_signal) {
    if (left_signal_status == 0) {
      left_signal_status = 1;
    }
  }
  else {
    if (left_signal_status > 0) {
      left_signal_status = 0;
    }
  }

  // below is logic from my led code base
  if (AP_status > 0) {
    AP_status = LED_AP_on(AP_status);
  }
  else if (tacc_status > 0) {
    tacc_status = LED_tacc_on(tacc_status);
  }
  else if (AP_ready_status > 0) {
    AP_ready_status = LED_AP_ready(AP_ready_status);
  }
  else if (tacc_ready_status > 0) {
    tacc_ready_status = LED_tacc_ready(tacc_ready_status);
  }

  // insert drive and reverse code here

  else if (charging_status > 0) {
    charging_status = LED_charging_on(charging_status);
  }
  else {
    LED_center_off();
    //LED_debug_brightness();   // for debugging
  }

  if (right_bsm_status > 0) {
    LED_right_bsm_on();
  }
  else {
    LED_right_bsm_off();
  }
  if (right_signal_status > 0) {
    LED_right_signal_on();
  }
  else if (right_bsm_status == 0) {
    LED_right_signal_off();
  }

  if (left_bsm_status > 0) {
    LED_left_bsm_on();
  }
  else {
    LED_left_bsm_off();
  }
  if (left_signal_status > 0) {
    LED_left_signal_on();
  }
  else if (left_bsm_status == 0) {
    LED_left_signal_off();
  }

  LED_show();
  delay(5);     // for animation timing

  //YOUR CODE ENDS HERE ***************************************
}


