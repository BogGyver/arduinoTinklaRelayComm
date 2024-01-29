//#define DEBUG

//COMM VARIABLES
bool readingLine = false;
bool lineReceived = false;
bool dataRequested = false;
String readString; //will have the string here to process
const unsigned int endOfTransmissionCode = 0;
const bool ignoreCRandLF = true;
int outputBufferSize = 0;

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

void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  outputBufferSize = Serial.availableForWrite();
  #ifdef DEBUG
  Serial.print("Output buffer length = ");
  Serial.println(outputBufferSize, DEC);
  #endif
  resetComm();
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
    Serial.println("DATA???");
    dataRequested = true;
    #ifdef DEBUG
    Serial.println("Requesting data from Tinkla Relay..");
    #endif
  }
}

void processDataMessage(String dataMessage) {
  int d1 = dataMessage.charAt(4);
  int d2 = dataMessage.charAt(5);
  int d3 = dataMessage.charAt(6);

  #ifdef DEBUG
    Serial.print("First data message byte = ");
    Serial.println(d1, DEC);
    Serial.print("Second data message byte = ");
    Serial.println(d2, DEC);
    Serial.print("Third data message byte = ");
    Serial.println(d3, DEC);
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
    Serial.print("Brightness message byte = ");
    Serial.println(d1, DEC);
  #endif
  //brightness comes as % (0-128%) and gets 100 added to it so 35% is 135
  if (d1 > 100) {
    rel_brightness = d1 - 100;
  }
}

void commLoop() {
  if (Serial.available()) {
    messageStart();
  }
  while (readingLine) {
    if (millis() - millisMessageStarted >= TIMEOUT_MSG) {
      #ifdef DEBUG
        if (readString.length() > 0) {
          Serial.println("Message did not end in the allocated time!");
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
      Serial.print("I received: ");
      Serial.println(readString);
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
      Serial.println("No message received in the allocated time!");
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
