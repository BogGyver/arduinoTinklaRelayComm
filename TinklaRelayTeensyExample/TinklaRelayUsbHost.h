#if defined(SERIAL_DEBUG) || defined(LCD_DEBUG)
  #include "Debug.h"
#endif

//USB Host stuff

#include "USBHost_t36.h"
#include "TinklaRelayDriver.h"

USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
TinklaRelay tinklaRelay(myusb);

USBDriver *drivers[] = { &tinklaRelay};
#define CNT_DEVICES (sizeof(drivers) / sizeof(drivers[0]))
#ifdef DEBUG
String driver_names[CNT_DEVICES] = { "TinklaRelay"};
unsigned long millisSinceLastError = 0;
#define TIME_BETWEEN_ERROR_MESSAGES 2000
unsigned int msg_numb = 0;
#endif
bool driver_active[CNT_DEVICES] = { false };

void UpdateActiveDeviceInfo() {
  for (uint8_t i = 0; i < CNT_DEVICES; i++) {
    if (*drivers[i] != driver_active[i]) {
      if (driver_active[i]) {
        #ifdef DEBUG
        showDebugTxt(driver_names[i] + " Disconnected ");
        #endif
        driver_active[i] = false;
      } else {
        #ifdef DEBUG
        showDebugTxt(driver_names[i] + " Connected");
        showDebugTxt(String(drivers[i]->idVendor(), HEX));
        showDebugTxt(String(drivers[i]->idProduct(), HEX));
        #endif
        driver_active[i] = true;
        #ifdef SERIAL_DEBUG
        const uint8_t *psz = drivers[i]->manufacturer();
        if (psz && *psz) Serial.printf("  manufacturer: %s\n", psz);
        psz = drivers[i]->product();
        if (psz && *psz) Serial.printf("  product: %s\n", psz);
        psz = drivers[i]->serialNumber();
        if (psz && *psz) Serial.printf("  Serial: %s\n", psz);
        #endif
      }
    }
  }
  #ifdef DEBUG
  if ((millis() - millisSinceLastError >= TIME_BETWEEN_ERROR_MESSAGES) && (!driver_active[0])) {
    showDebugTxt(String(msg_numb));
    showDebugTxt("NO TR CONNECTED...");
    millisSinceLastError = millis();
    msg_numb ++;
  }
  #endif
}

void tinklaRelayusbHostSetup() {
  #ifdef DEBUG
  debug_setup();
  #endif
  myusb.begin();
}

void tinklaRelayusbHostLoop() {
  myusb.Task();
  UpdateActiveDeviceInfo();
  
}