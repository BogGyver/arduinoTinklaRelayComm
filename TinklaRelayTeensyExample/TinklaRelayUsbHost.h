#if defined(SERIAL_DEBUG) || defined(LCD_DEBUG)
  #include "Debug.h"
#endif

//USB Host stuff

#include "USBHost_t36.h"
#include "TinklaRelayDriver.h"

USBHost myusb;
USBHub hub1(myusb);
TinklaRelay tinklaRelay(myusb);

USBDriver *drivers[] = { &tinklaRelay};
#define CNT_DEVICES (sizeof(drivers) / sizeof(drivers[0]))
#ifdef DEBUG
String driver_names[CNT_DEVICES] = { "TinklaRelay"};
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
}

void tinklaRelayusbHostSetup() {
  myusb.begin();
}

void tinklaRelayusbHostLoop() {
  myusb.Task();
  UpdateActiveDeviceInfo();
}