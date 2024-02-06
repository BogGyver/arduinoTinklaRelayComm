#define SERIAL_DEBUG

bool alternateON = false;
int counter = 0;

#include "TinklaRelayUsbHost.h"
#include "NeoPixelAction.h"
#include "Speedometer.h"

void setup() {
  tinklaRelayusbHostSetup();
  //YOUR CODE STARTS HERE
  Serial.begin(9600);
  speedometerSetup();
  neoPixelSetup();
  //YOUR CODE ENDS HERE
}

void loop() {
  tinklaRelayusbHostLoop();
  //access all Tinkla Relay variables as tinklaRelay.rel_variable_you_want as defined in the TinklaRelayDriver.h
  //YOUR CODE STARTS HERE
  if (!tinklaRelay.tinklaRelayInitialized) delay(100);
  counter = (counter + 1 ) % 10;
  if (counter == 0) alternateON = !alternateON;
  neoPixelLoop();
  speedometerLoop();
  //YOUR CODE ENDS HERE
}
