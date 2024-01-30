class TinklaRelay: public USBDriver {
public:
    TinklaRelay(USBHost &host) : /* txtimer(this),*/  updatetimer(this) { init(); }
    void begin(const uint8_t key = 0);
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
    //USBDriverTimer txtimer;
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
};