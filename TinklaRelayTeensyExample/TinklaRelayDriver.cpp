#define DEBUG_USB

#include "USBHost_t36.h" 
#include "TinklaRelayDriver.h"

#define TINKLA_RELAY_VID   0xbbaa
#define TINKLA_RELAY_PID   0xddcc

#define print   USBHost::print_
#define println USBHost::println_

void showDebugTxt(String dbgTxt);

uint8_t tinklaRelayData[] = {0,0,0,0,0,0,0,0,0,0};

static void pipe_set_addr(Pipe_t *pipe, uint32_t addr)
{
	pipe->qh.capabilities[0] = (pipe->qh.capabilities[0] & 0xFFFFFF80) | addr;
}

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
    showDebugTxt(String(dev->idVendor, HEX));
    showDebugTxt(String(dev->idProduct, HEX));
  #endif
  if (dev->idVendor != TINKLA_RELAY_VID) return false;
	if (dev->idProduct != TINKLA_RELAY_PID) return false;
  // only claim at interface level

	if (type != 1) {
    #ifdef DEBUG_USB
    showDebugTxt("NOT INTERF");
    #endif
    return false;
  }
	if (len < 9+7+7+7) { // Interface descriptor + 3 endpoint decriptors
    #ifdef DEBUG_USB
      showDebugTxt("WRONG SIZE");
    #endif
    return false;
  }

  //Few more TR specific checks
  uint32_t numendpoint = descriptors[4];
	if (numendpoint < 1) return false; 
	if (descriptors[5] != 255) return false; 
	if (descriptors[6] != 255) return false; 
	if (descriptors[7] != 255) return false; 

  bInterfaceNumber = descriptors[2];
  println("interfaceNumber=", bInterfaceNumber, HEX);

  uint8_t desc_index = 9;
	uint8_t in_index = 0xff, out_index = 0xff;

  println("numendpoint=", numendpoint, HEX);
	while (numendpoint--) {
		if ((descriptors[desc_index] != 7) || (descriptors[desc_index+1] != 5)) {
      #ifdef DEBUG_USB
      showDebugTxt("NOT ENDP");
      #endif
      return false; // not an end point
    }
    
    //if (descriptors[desc_index] != 4) return false; // not an end point
		if (descriptors[desc_index+3] == 2) {  // Bulk end point
			if (descriptors[desc_index+2] & 0x80)
				in_index = desc_index;
			else
				out_index = desc_index;
		}
		desc_index += 7;	// point to next one...
	}
	if ((in_index == 0xff) || (out_index == 0xff)) {	// did not find end point
    #ifdef DEBUG_USB
    showDebugTxt("NO ENDPs");
    #endif
    return false;
  } 
  uint32_t endpointIn = descriptors[in_index+2]; // bulk-in descriptor 1 81h
	uint32_t endpointOut = descriptors[out_index+2]; // bulk-out descriptor 2 02h

	println("endpointIn=", endpointIn, HEX);
	println("endpointOut=", endpointOut, HEX);

	uint32_t sizeIn = descriptors[in_index+4] | (descriptors[in_index+5] << 8);
	println("packet size in (USBDrive) = ", sizeIn);

	uint32_t sizeOut = descriptors[out_index+4] | (descriptors[out_index+5] << 8);
	println("packet size out (USBDrive) = ", sizeOut);

	uint32_t intervalIn = descriptors[in_index+6];
	uint32_t intervalOut = descriptors[out_index+6];

	println("polling intervalIn = ", intervalIn);
	println("polling intervalOut = ", intervalOut);
	rxpipe = new_Pipe(dev, 2, endpointIn & 0x0F, 1, sizeIn, intervalIn);
	txpipe = new_Pipe(dev, 2, endpointOut, 0, sizeOut, intervalOut);
	rxpipe->callback_function = rx_callback;
	txpipe->callback_function = tx_callback;
  millisLastMessageEnded = millis() + 5000; //wait 3 sconds before asking for data
  millisMessageStarted = millis() + 5000;
  tinklaRelayInitialized = true;
  device = dev;
  return true;
}

void TinklaRelay::disconnect()
{
	updatetimer.stop();
}

void TinklaRelay::control(const Transfer_t *transfer)
{
  if (!dataRequested) {
    //we are here but we did not request data
    return;
  } else {
    switch(dataReqType) {
      case GET_TINKLA_RELAY_DATA: //teensy data from TR ,GET_TINKLA_RELAY_DATA_SIZE
        {
          #ifdef DEBUG_USB
            showDebugTxt("TR DT: "+ String((char *)transfer->buffer));
          #endif
          break;
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
	if (!transfer->driver) return;
	((TinklaRelay *)(transfer->driver))->rx_data(transfer);
}

void TinklaRelay::tx_callback(const Transfer_t *transfer)
{
  if (!transfer->driver) return;
  ((TinklaRelay *)(transfer->driver))->tx_data(transfer);
}

void TinklaRelay::rx_data(const Transfer_t *transfer)
{
  rxlen = 0;
}

void TinklaRelay::tx_data(const Transfer_t *transfer)
{
  //txlen = 0;
}


size_t TinklaRelay::write(const void *data, const size_t size)
{
	return 0;
}

void TinklaRelay::requestDataMessage(uint8_t dataReq, uint8_t dataLen, void *buf) {
  if (millis() < 5000) {
    //do nothing the first 5 seconds
    return;
  }
  if ((millis() - millisLastMessageEnded >= TIME_BEFORE_NEXT_DATA_RELAY) && (!dataRequested)){
    mk_setup(setup, 0x80, dataReq, 0, 0, dataLen);
    dataRequested = true;
    dataReqType = dataReq;
    pipe_set_addr(device->control_pipe, device->address);
    queue_Control_Transfer(device, &setup, buf, this);
    messageStart();
  }
}


void TinklaRelay::Task() {
  if (millis() < 5000) {
    //do nothing the first 5 seconds
    return;
  }
  //check for message timeout
  if (millis() - millisLastMessageEnded >= TIMEOUT_NO_MSG) {
    #ifdef DEBUG_USB
      showDebugTxt("MSG TO");
    #endif
    resetComm();
    resetFlags();
    dataRequested = false;
  }

  //request data if not requested
  if (!dataRequested) {
    requestDataMessage(GET_TINKLA_RELAY_DATA,GET_TINKLA_RELAY_DATA_SIZE,&tinklaRelayData);
  }
}

//NOW THE COMM PART

void TinklaRelay::resetComm() {
  if (tinklaRelayInitialized) {
    millisMessageStarted = millis();
    millisLastMessageEnded = millis();
  }
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
  rel_use_imperial = false;

  rel_left_steering_above_45deg = false; 
  rel_right_steering_above_45deg = false;
  rel_AP_on = false;
  rel_car_charging = false;
  rel_left_side_bsm = false;
  rel_right_side_bsm = false;
  rel_tacc_only_active = false;

  rel_brightness = 100;
  rel_power_lvl = 0;
  rel_speed = 0;
}

void TinklaRelay::messageStart() {
  millisMessageStarted = millis();
}

void TinklaRelay::messageReceived() {
  dataRequested = false;
  dataReqType = 0;
  millisLastMessageEnded = millis();
  processDataMessage();
}

void TinklaRelay::processDataMessage() {
  rel_option1_on = ((tinklaRelayData[0] & REL_OPTION1_ON) > 0);
  rel_option2_on = ((tinklaRelayData[0] & REL_OPTION2_ON) > 0);
  rel_option3_on = ((tinklaRelayData[0] & REL_OPTION3_ON) > 0);
  rel_option4_on = ((tinklaRelayData[0] & REL_OPTION4_ON) > 0);
  rel_car_on = ((tinklaRelayData[0] & REL_CAR_ON) > 0);
  rel_gear_in_reverse = ((tinklaRelayData[0] & REL_GEAR_IN_REVERSE) > 0);
  rel_gear_in_forward = ((tinklaRelayData[0] & REL_GEAR_IN_FORWARD) > 0);

  rel_left_turn_signal = ((tinklaRelayData[1] & REL_LEFT_TURN_SIGNAL) > 0);
  rel_right_turn_signal = ((tinklaRelayData[1] & REL_RIGHT_TURN_SIGNAL) > 0);
  rel_brake_pressed = ((tinklaRelayData[1] & REL_BRAKE_PRESSED) > 0);
  rel_highbeams_on = ((tinklaRelayData[1] & REL_HIGHBEAMS_ON) > 0);
  rel_light_on = ((tinklaRelayData[1] & REL_LIGHT_ON) > 0);
  rel_below_20mph = ((tinklaRelayData[1] & REL_BELOW_20MPH) > 0); 

  rel_left_steering_above_45deg = ((tinklaRelayData[2] & REL_LEFT_STEERING_ANGLE_ABOVE_45DEG) > 0); 
  rel_right_steering_above_45deg = ((tinklaRelayData[2] & REL_RIGHT_STEERING_ANGLE_ABOVE_45DEG) > 0);
  rel_AP_on = ((tinklaRelayData[2] & REL_AP_ON) > 0);
  rel_car_charging = ((tinklaRelayData[2] & REL_CAR_CHARGING) > 0);
  rel_left_side_bsm = ((tinklaRelayData[2] & REL_LEFT_SIDE_BSM) > 0);
  rel_right_side_bsm = ((tinklaRelayData[2] & REL_RIGHT_SIDE_BSM) > 0);
  rel_tacc_only_active = ((tinklaRelayData[2] & REL_TACC_ONLY_ACTIVE) > 0);
  rel_brightness = tinklaRelayData[3];
  rel_speed = tinklaRelayData[4];
  rel_power_lvl = (int16_t)((tinklaRelayData[5] << 8) | tinklaRelayData[6]);
}