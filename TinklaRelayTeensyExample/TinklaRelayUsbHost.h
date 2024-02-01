#ifdef SERIAL_DEBUG
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
bool driver_active[CNT_DEVICES] = { false };

void UpdateActiveDeviceInfo() {
  for (uint8_t i = 0; i < CNT_DEVICES; i++) {
    if (*drivers[i] != driver_active[i]) {
      if (driver_active[i]) {
        driver_active[i] = false;
      } else {
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
}

void tinklaRelayusbHostSetup() {
  myusb.begin();
}

void tinklaRelayusbHostLoop() {
  myusb.Task();
  UpdateActiveDeviceInfo();
}