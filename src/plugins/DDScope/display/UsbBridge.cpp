//#define USBHOST_PRINT_DEBUG
#include <Arduino.h>
#include "UsbBridge.h"
#include "src/lib/tasks/OnTask.h"

// Global objects
USBHost myusb;
USBSerial userSerial(myusb);  // THIS is the correct constructor

bool usbConnected = false;

void usbPollTask() {
  myusb.Task();

  //if (userSerial && userSerial.available()) {
    //Serial.println("✅ ESP32 USB device connected.");
    //usbConnected = true;
    // if (userSerial.available()) {
    //   char c = SERIAL_ESP.read();
    //   Serial.println(c);
    // }
  //}
}

void usbBegin(void) {
  pinMode(33, OUTPUT);    // A7: Enable USB Host power pin on Teensy
  digitalWrite(33, HIGH);    
  delay(50);

  myusb.begin();

  Serial.println("USB Host begin");

  Serial.printf("MSG: Starting usbPoll task (17ms, priority 6)");
  uint8_t usbTaskHandle = tasks.add(17, 0, true, 6, usbPollTask, "USB");
  if (usbTaskHandle) {
    Serial.println("USB task scheduled");
  } else {
    Serial.println("❌ Failed to schedule USB task");
  }
}
