/* USING SH1106 OLEDD DISPLAY WITH TEENSY 4.0 FOR DEBUG - With U8g2 library 
 *  - uncomment LCD_DEBUG definition below to use the LCD screen
 *  GND ==> GND
 *  VCC ==> 5V
 *  CLK ==> 13
 *  MOSI ==> 11
 *  RES ==> 23
 *  DC ==> 7
 *  CS ==> 12
 */

//#define SERIAL_DEBUG
#define LCD_DEBUG

#if defined(SERIAL_DEBUG) || defined(LCD_DEBUG)
#define DEBUG
#endif

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
U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R2, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 12, /* dc=*/ 7, /* reset=*/ 23);  
#define LCD_LINES 4
String oldDbgTxt[4] = { "", "", "", ""};
#endif 

#include "USBHost_t36.h"
#include "TinklaRelayDriver.h"

void showDebugTxt(String dbgTxt) {
#ifdef DEBUG
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
#endif
}



//COMM VARIABLES
bool readingLine = false;
bool lineReceived = false;
bool dataRequested = false;
String readString; //will have the string here to process
const unsigned int endOfTransmissionCode = 0;
const bool ignoreCRandLF = false;
int outputBufferSize = 0;
#define BOUD 115200

//TIMERS
unsigned long millisMessageStarted;
unsigned long millisLastMessageEnded;
const unsigned int TIMEOUT_MSG = 10; // 0.01s to get the next character
const unsigned int TIMEOUT_NO_MSG = 3000; // 3s max between messages
const unsigned int TIME_BEFORE_NEXT_DATA_RELAY = 200; //0.2s before asking for next data packet

//BYTE PARSER
//First Byte After DATA
#define REL_OPTION1_ON 2
#define REL_OPTION2_ON 4
#define REL_OPTION3_ON 8
#define REL_OPTION4_ON 16
#define REL_CAR_ON 32
#define REL_GEAR_IN_REVERSE 64
#define REL_GEAR_IN_FORWARD 128
//Second Byte After DATA
#define REL_LEFT_TURN_SIGNAL 2
#define REL_RIGHT_TURN_SIGNAL 4
#define REL_BRAKE_PRESSED 8
#define REL_HIGHBEAMS_ON 16
#define REL_LIGHT_ON 32
#define REL_BELOW_20MPH 64
#define UNUSED1 128
//Third Byte After DATA
#define REL_LEFT_STEERING_ANGLE_ABOVE_45DEG 2
#define REL_RIGHT_STEERING_ANGLE_ABOVE_45DEG 4
#define REL_AP_ON 8
#define REL_CAR_CHARGING 16
#define REL_LEFT_SIDE_BSM 32
#define REL_RIGHT_SIDE_BSM 64
#define REL_TACC_ONLY_ACTIVE 128

//VALUES
bool rel_option1_on = false;
bool rel_option2_on = false;
bool rel_option3_on = false;
bool rel_option4_on = false;
bool rel_car_on = false;
bool rel_gear_in_reverse = false;
bool rel_gear_in_forward = false;

bool rel_left_turn_signal = false;
bool rel_right_turn_signal = false;
bool rel_brake_pressed = false;
bool rel_highbeams_on = false;
bool rel_light_on = false;
bool rel_below_20mph = false; //BELOW 20 MPH

bool rel_left_steering_above_45deg = false; //MORE THAN 45 DEG
bool rel_right_steering_above_45deg = false; //MORE THAN 45 DEG
bool rel_AP_on = false; //start with true if only Veh can is connected
bool rel_car_charging = false;
bool rel_left_side_bsm = false;
bool rel_right_side_bsm = false;
bool rel_tacc_only_active = false;

unsigned int rel_brightness = 100; // start brightness value


//USB Host stuff
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
TinklaRelay tinklaRelay(myusb);

USBDriver *drivers[] = { &tinklaRelay};

#define CNT_DEVICES (sizeof(drivers) / sizeof(drivers[0]))
const char *driver_names[CNT_DEVICES] = { "TinklaRelay"};
bool driver_active[CNT_DEVICES] = { false };

void UpdateActiveDeviceInfo() {
  // First see if any high level devices
  bool new_device_detected = false;
  for (uint8_t i = 0; i < CNT_DEVICES; i++) {
    if (*drivers[i] != driver_active[i]) {
      if (driver_active[i]) {
        #ifdef DEBUG
        showDebugTxt(driver_names[i]);
        #endif
        driver_active[i] = false;
      } else {
        new_device_detected = true;
        #ifdef DEBUG
        showDebugTxt(driver_names[i]);
        showDebugTxt(drivers[i]->idVendor());
        showDebugTxt(drivers[i]->idProduct());
        #endif
        driver_active[i] = true;
        // const uint8_t *psz = drivers[i]->manufacturer();
        // if (psz && *psz) tft.printf("  manufacturer: %s\n", psz);
        // psz = drivers[i]->product();
        // if (psz && *psz) tft.printf("  product: %s\n", psz);
        // psz = drivers[i]->serialNumber();
        // if (psz && *psz) tft.printf("  Serial: %s\n", psz);
      }
    }
  }
  if (new_device_detected) {
    #ifdef DEBUG
        showDebugTxt("TinklaRelay found!");
    #endif
  }
}

void setup() {
  Serial.begin(BOUD); // opens serial port, sets data rate to BOUD bps
  outputBufferSize = Serial.availableForWrite();
  resetComm();
  #ifdef LCD_DEBUG
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_samim_10_t_all);
  #endif
  #ifdef DEBUG
  showDebugTxt("Started...");
  #endif
  myusb.begin();
}

void resetComm() {
  readingLine = false;
  lineReceived = false;
  readString = "";
  millisMessageStarted = 0;
  millisLastMessageEnded = millis();
}

void resetFlags() {
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

void messageStart() {
  readingLine = true;
  lineReceived = false;
  millisMessageStarted = millis();
}

void messageReceived() {
  readingLine = false;
  lineReceived = true;
  millisLastMessageEnded = millis();
}

void requestDataMessage() {
  if (millis() - millisLastMessageEnded >= TIME_BEFORE_NEXT_DATA_RELAY) {
    String dataRequest = "0123456";
    dataRequest[0] = 0x00;
    dataRequest[1] = 0x00;
    dataRequest[2] = 0xC0; //192 should be the node for IN
    dataRequest[3] = 0xD0; //command
    dataRequest[4] = 0x00;
    dataRequest[5] = 0x00;
    dataRequest[6] = 0x20; //length of return
    Serial.print(dataRequest);
    dataRequested = true;
    #ifdef DEBUG
    showDebugTxt("RQ Data");
    #endif
  }
}

void processDataMessage(String dataMessage) {
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

void processBrightnessMessage(String dataMessage) {
  int d1 = dataMessage.charAt(4);
  #ifdef DEBUG
  showDebugTxt("Got BRT");
  #endif
  //brightness comes as % (0-128%) and gets 100 added to it so 35% is 135
  if (d1 > 100) {
    rel_brightness = d1 - 100;
  }
}

void commLoop() {
  myusb.Task();
  UpdateActiveDeviceInfo();
  if (Serial.available()) {
    messageStart();
  }
  while (readingLine) {
    if (millis() - millisMessageStarted >= TIMEOUT_MSG) {
      #ifdef DEBUG
        if (readString.length() > 0) {
          showDebugTxt("CHR TO");
        }
      #endif
      resetComm();
    }
    if (Serial.available() >0) {
      char c = Serial.read();  //gets one byte from serial buffer
      if (c == endOfTransmissionCode) {
        messageReceived();
      } else {
        if (!(ignoreCRandLF && (( c == 10) || ( c == 13)))) {
          readString += c; //makes the string readString
        }
        millisMessageStarted = millis();
      }
    } 
  }
  
  // process data only when you receive data
  if (lineReceived) {
    #ifdef DEBUG
    showDebugTxt(readString.substring(0,7));
    #endif
    //is DATA message valid?
    if ((readString.length() == 7) && (readString.indexOf("DATA") == 0)) {
      processDataMessage(readString);
      dataRequested = false;
    }
    //is BRGT message valid?
    if ((readString.length() == 5) && (readString.indexOf("BRGT") == 0)) {
      processBrightnessMessage(readString);
    }
    //done with message
    resetComm();
  }
  
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
    requestDataMessage();
  }
}

void loop() {
  commLoop();
  //YOUR CODE STARTS HERE


  //YOUR CODE ENDS HERE
}
