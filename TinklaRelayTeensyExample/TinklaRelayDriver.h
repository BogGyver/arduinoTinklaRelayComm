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

//CONTROL READ VALUES
#define GET_TINKLA_RELAY_SERIAL_NUMBER 0xD0

class TinklaRelay: public USBDriver {
public:
    TinklaRelay(USBHost &host) : /* txtimer(this),*/  updatetimer(this) { init(); }
    void begin(const uint8_t key = 0);
    //VALUES
    volatile bool rel_option1_on = false;
    volatile bool rel_option2_on = false;
    volatile bool rel_option3_on = false;
    volatile bool rel_option4_on = false;
    volatile bool rel_car_on = false;
    volatile bool rel_gear_in_reverse = false;
    volatile bool rel_gear_in_forward = false;
    volatile bool rel_left_turn_signal = false;
    volatile bool rel_right_turn_signal = false;
    volatile bool rel_brake_pressed = false;
    volatile bool rel_highbeams_on = false;
    volatile bool rel_light_on = false;
    volatile bool rel_below_20mph = false; //BELOW 20 MPH
    volatile bool rel_left_steering_above_45deg = false; //MORE THAN 45 DEG
    volatile bool rel_right_steering_above_45deg = false; //MORE THAN 45 DEG
    volatile bool rel_AP_on = false; //start with true if only Veh can is connected
    volatile bool rel_car_charging = false;
    volatile bool rel_left_side_bsm = false;
    volatile bool rel_right_side_bsm = false;
    volatile bool rel_tacc_only_active = false;
    volatile unsigned int rel_brightness = 100; // start brightness value
protected:
    virtual void Task();
    virtual bool claim(Device_t *device, int type, const uint8_t *descriptors, uint32_t len);
    virtual void disconnect();
private:
    static void rx_callback(const Transfer_t *transfer);
    static void tx_callback(const Transfer_t *transfer);
    void rx_data(const Transfer_t *transfer);
    void tx_data(const Transfer_t *transfer);
    void init();
    size_t write(const void *data, const size_t size);
    int read(void *data, const size_t size);
    
private:
    Pipe_t mypipes[2] __attribute__ ((aligned(32)));
    Transfer_t mytransfers[3] __attribute__ ((aligned(32)));
    strbuf_t mystring_bufs[1];
    USBDriverTimer updatetimer;
    Pipe_t *rxpipe;
    Pipe_t *txpipe;
    bool first_update;
    uint8_t txbuffer[240];
    uint8_t rxpacket[64];
    volatile uint16_t txhead;
    volatile uint16_t txtail;
    volatile bool     txready;
    volatile uint8_t  rxlen;
    volatile bool     do_polling;  
    setup_t setup;

    //COMM VARIABLES
    bool readingLine = false;
    bool lineReceived = false;
    bool dataRequested = false;
    uint8_t dataReqType = 0;
    String readString; //will have the string here to process
    const unsigned int endOfTransmissionCode = 0;
    const bool ignoreCRandLF = false;
    int outputBufferSize = 0;
    //TIMERS
    unsigned long millisMessageStarted;
    unsigned long millisLastMessageEnded;
    const unsigned int TIMEOUT_MSG = 100; // 0.1s to get the response
    const unsigned int TIMEOUT_NO_MSG = 3000; // 3s max between messages
    const unsigned int TIME_BEFORE_NEXT_DATA_RELAY = 200; //0.2s before asking for next data packet 
    void requestDataMessage(uint8_t dataReq);
    void processDataMessage(String dataMessage);
    void processBrightnessMessage(String dataMessage);
    void messageReceived();
    void messageStart();
    void resetComm();
    void resetFlags();
    void control(const Transfer_t *transfer);
};