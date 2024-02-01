#define SERIAL_DEBUG

#include "TinklaRelayUsbHost.h"
#include "NeoPixelAction.h"
#include "Speedometer.h"

void setup() {
  tinklaRelayusbHostSetup();
  //YOUR CODE STARTS HERE
  speedometerSetup();
  neoPixelSetup();

  //YOUR CODE ENDS HERE
}

void loop() {
  tinklaRelayusbHostLoop();
  //access all Tinkla Relay variables as tinklaRelay.rel_variable_you_want as defined in the TinklaRelayDriver.h
  //YOUR CODE STARTS HERE
  neoPixelLoop();
  speedometerLoop();
  //YOUR CODE ENDS HERE
}
