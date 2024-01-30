//#define SERIAL_DEBUG
#define LCD_DEBUG

#include "TinklaRelayUsbHost.h"

void setup() {
  tinklaRelayusbHostSetup();
}

void loop() {
  tinklaRelayusbHostLoop();
  //access all Tinkla Relay variables as tinklaRelay.rel_variable_you_want as defined in the TinklaRelayDriver.h
  //YOUR CODE STARTS HERE


  //YOUR CODE ENDS HERE
}
