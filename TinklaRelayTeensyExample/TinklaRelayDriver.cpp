/* USB EHCI Host for Teensy 3.6
 * Copyright 2017 Michael McElligott
 * Copyright 2017 Paul Stoffregen (paul@pjrc.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#define DEBUG_USB

#include <Arduino.h>
#include "USBHost_t36.h"  // Read this header first for key info
#include "TinklaRelayDriver.h"

#define TINKLA_RELAY_VID   0xbbaa
#define TINKLA_RELAY_PID   0xddcc

void showDebugTxt(String dbgTxt);

void TinklaRelay::init()
{
	contribute_Pipes(mypipes, sizeof(mypipes)/sizeof(Pipe_t));
	contribute_Transfers(mytransfers, sizeof(mytransfers)/sizeof(Transfer_t));
	contribute_String_Buffers(mystring_bufs, sizeof(mystring_bufs)/sizeof(strbuf_t));
	driver_ready_for_device(this);
}

bool TinklaRelay::claim(Device_t *dev, int type, const uint8_t *descriptors, uint32_t len)
{
  #ifdef DEBUG_USB
    showDebugTxt("Trying to claim TR");
    showDebugTxt(String(dev->idVendor));
    showDebugTxt(String(dev->idProduct));
  #endif
  if (dev->idVendor != TINKLA_RELAY_VID) return false;
	if (dev->idProduct != TINKLA_RELAY_PID) return false;
	rxpipe = txpipe = NULL;
  return true;
}

void TinklaRelay::disconnect()
{
	updatetimer.stop();
	//txtimer.stop();
}

void TinklaRelay::control(const Transfer_t *transfer)
{
  if (!dataRequested) {
    //we are here but we did not request data
    #ifdef DEBUG_USB
      showDebugTxt("In CTRL w/o req");
    #endif
    return;
  } else {
    switch(dataReqType) {
      case GET_TINKLA_RELAY_SERIAL_NUMBER:
        {
          #ifdef DEBUG_USB
            char *relay_serial_number = (char *)transfer->buffer;
            showDebugTxt("RX SN:" + String(relay_serial_number).substring(0,5));
          #endif
        }
      default:
        {
          #ifdef DEBUG_USB
            showDebugTxt("UNKNOWN DATA");
          #endif
        }
    }
  }
  messageReceived();
}


void TinklaRelay::rx_callback(const Transfer_t *transfer)
{
  #ifdef DEBUG_USB
  showDebugTxt("In rx_callback!");
  #endif
	if (!transfer->driver) return;
	((TinklaRelay *)(transfer->driver))->rx_data(transfer);
}

void TinklaRelay::tx_callback(const Transfer_t *transfer)
{
  #ifdef DEBUG_USB
  showDebugTxt("In rx_callback!");
  #endif
  if (!transfer->driver) return;
  ((TinklaRelay *)(transfer->driver))->tx_data(transfer);
}

void TinklaRelay::rx_data(const Transfer_t *transfer)
{
  
}

void TinklaRelay::tx_data(const Transfer_t *transfer)
{

}


size_t TinklaRelay::write(const void *data, const size_t size)
{
	return 0;
}

void TinklaRelay::requestDataMessage(uint8_t dataReq) {
  if ((millis() - millisLastMessageEnded >= TIME_BEFORE_NEXT_DATA_RELAY) && (!dataRequested)){
    //dest, request_type, requst, value, index, length
    mk_setup(setup, 192, dataReq, 0, 0, 0x20);
    queue_Control_Transfer(device, &setup, NULL, this);
    dataRequested = true;
    dataReqType = dataReq;
    #ifdef DEBUG
    showDebugTxt("RQ Data " + String(dataReq));
    #endif
    messageStart();
  }
}


void TinklaRelay::Task()
{
	
  // if (Serial.available()) {
  //   messageStart();
  // }
  // while (readingLine) {
  //   if (millis() - millisMessageStarted >= TIMEOUT_MSG) {
  //     #ifdef DEBUG
  //       if (readString.length() > 0) {
  //         showDebugTxt("CHR TO");
  //       }
  //     #endif
  //     resetComm();
  //   }
  //   if (Serial.available() >0) {
  //     char c = Serial.read();  //gets one byte from serial buffer
  //     if (c == endOfTransmissionCode) {
  //       messageReceived();
  //     } else {
  //       if (!(ignoreCRandLF && (( c == 10) || ( c == 13)))) {
  //         readString += c; //makes the string readString
  //       }
  //       millisMessageStarted = millis();
  //     }
  //   } 
  // }
  
  // // process data only when you receive data
  // if (lineReceived) {
  //   #ifdef DEBUG
  //   showDebugTxt(readString.substring(0,7));
  //   #endif
  //   //is DATA message valid?
  //   if ((readString.length() == 7) && (readString.indexOf("DATA") == 0)) {
  //     processDataMessage(readString);
  //     dataRequested = false;
  //   }
  //   //is BRGT message valid?
  //   if ((readString.length() == 5) && (readString.indexOf("BRGT") == 0)) {
  //     processBrightnessMessage(readString);
  //   }
  //   //done with message
  //   resetComm();
  // }
  
  //check for message timeout
  if (millis() - millisLastMessageEnded >= TIMEOUT_NO_MSG) {
    #ifdef DEBUG
      showDebugTxt("MSG TO");
    #endif
    resetComm();
    resetFlags();
    dataRequested = false;
  }

  //request data if not requested
  if (!dataRequested) {
    requestDataMessage(GET_TINKLA_RELAY_SERIAL_NUMBER);
  }
}

//NOW THE COMM PART

void TinklaRelay::resetComm() {
  millisMessageStarted = 0;
  millisLastMessageEnded = millis();
  dataRequested = false;
  dataReqType = 0;
}

void TinklaRelay::resetFlags() {
  rel_option1_on = false;
  rel_option2_on = false;
  rel_option3_on = false;
  rel_option4_on = false;
  rel_car_on = false;
  rel_gear_in_reverse = false;
  rel_gear_in_forward = false;

  rel_left_turn_signal = false;
  rel_right_turn_signal = false;
  rel_brake_pressed = false;
  rel_highbeams_on = false;
  rel_light_on = false;
  rel_below_20mph = false; 

  rel_left_steering_above_45deg = false; 
  rel_right_steering_above_45deg = false;
  rel_AP_on = false;
  rel_car_charging = false;
  rel_left_side_bsm = false;
  rel_right_side_bsm = false;
  rel_tacc_only_active = false;
}

void TinklaRelay::messageStart() {
  millisMessageStarted = millis();
}

void TinklaRelay::messageReceived() {
  dataRequested = false;
  dataReqType = 0;
  millisLastMessageEnded = millis();
}



void TinklaRelay::processDataMessage(String dataMessage) {
  int d1 = dataMessage.charAt(4);
  int d2 = dataMessage.charAt(5);
  int d3 = dataMessage.charAt(6);

  #ifdef DEBUG
  showDebugTxt("Got DAT");
  #endif

  rel_option1_on = ((d1 & REL_OPTION1_ON) > 0);
  rel_option2_on = ((d1 & REL_OPTION2_ON) > 0);
  rel_option3_on = ((d1 & REL_OPTION3_ON) > 0);
  rel_option4_on = ((d1 & REL_OPTION4_ON) > 0);
  rel_car_on = ((d1 & REL_CAR_ON) > 0);
  rel_gear_in_reverse = ((d1 & REL_GEAR_IN_REVERSE) > 0);
  rel_gear_in_forward = ((d1 & REL_GEAR_IN_FORWARD) > 0);

  rel_left_turn_signal = ((d2 & REL_LEFT_TURN_SIGNAL) > 0);
  rel_right_turn_signal = ((d2 & REL_RIGHT_TURN_SIGNAL) > 0);
  rel_brake_pressed = ((d2 & REL_BRAKE_PRESSED) > 0);
  rel_highbeams_on = ((d2 & REL_HIGHBEAMS_ON) > 0);
  rel_light_on = ((d2 & REL_LIGHT_ON) > 0);
  rel_below_20mph = ((d2 & REL_BELOW_20MPH) > 0); 

  rel_left_steering_above_45deg = ((d3 & REL_LEFT_STEERING_ANGLE_ABOVE_45DEG) > 0); 
  rel_right_steering_above_45deg = ((d3 & REL_RIGHT_STEERING_ANGLE_ABOVE_45DEG) > 0);
  rel_AP_on = ((d3 & REL_AP_ON) > 0);
  rel_car_charging = ((d3 & REL_CAR_CHARGING) > 0);
  rel_left_side_bsm = ((d3 & REL_LEFT_SIDE_BSM) > 0);
  rel_right_side_bsm = ((d3 & REL_RIGHT_SIDE_BSM) > 0);
  rel_tacc_only_active = ((d3 & REL_TACC_ONLY_ACTIVE) > 0);
}

void TinklaRelay::processBrightnessMessage(String dataMessage) {
  int d1 = dataMessage.charAt(4);
  #ifdef DEBUG
  showDebugTxt("Got BRT");
  #endif
  //brightness comes as % (0-128%) and gets 100 added to it so 35% is 135
  if (d1 > 100) {
    rel_brightness = d1 - 100;
  }
}