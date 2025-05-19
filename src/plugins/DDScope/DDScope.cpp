// =====================================================
// DDScope.cpp
//
// Title:  DDScope (Direct Drive Plugin for OnStepX)
// Author: Richard Benear
//
// Description:
// Direct Drive Telescope plugin for OnStepX.
// Refer to Readme.md for more information.
//
// Copyright (C) 2022 Richard Benear
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

// Firmware version -------------------------------------------------------------------------
#define PluginName                "DDScope"
#define DDScopeFwVersionMajor       1
#define DDScopeFwVersionMinor       03    // minor version 00 to 99

#include <Arduino.h>
#include "display/Display.h"
#include "DDScope.h"
#include "src/Common.h"
#include "screens/TouchScreen.h"
#include "screens/HomeScreen.h"
#include "src/lib/tasks/OnTask.h"
#include "src/libApp/commands/ProcessCmds.h"
#include "src/plugins/DDScope/display/UsbBridge.h"
#include "src/plugins/DDScope/display/WifiDisplay.h"

#ifdef ODRIVE_MOTOR_PRESENT
  #include "odriveExt/ODriveExt.h"
#endif

void touchWrapper() { touchScreen.touchScreenPoll(display.currentScreen); }
void updateScreenWrapper() { display.updateSpecificScreen(); }
void espWrapper() { wifiDisplay.espPoll(); }

void DDScope::init() {

  VF("MSG: Plugins, starting:"); VLF(PluginName);

  // Initilize custom pins...may want to move some of these to Features in future
  pinModeEx(ODRIVE_RST_PIN, OUTPUT);

  pinMode(ALT_THERMISTOR_PIN, INPUT); // Analog input
  pinMode(AZ_THERMISTOR_PIN, INPUT); // Analog input

  pinMode(AZ_ENABLED_LED_PIN, OUTPUT);
  digitalWrite(AZ_ENABLED_LED_PIN,HIGH); // LED OFF, active low 
  pinMode(ALT_ENABLED_LED_PIN, OUTPUT);
  digitalWrite(ALT_ENABLED_LED_PIN,HIGH); // LED OFF, active low

  pinMode(STATUS_TRACK_LED_PIN, OUTPUT);
  digitalWrite(STATUS_TRACK_LED_PIN,HIGH); // LED OFF, active low

  pinMode(BATTERY_LOW_LED_PIN, OUTPUT); 
  digitalWrite(BATTERY_LOW_LED_PIN,HIGH); // LED OFF, active low

  pinMode(FAN_ON_PIN, OUTPUT); 
  digitalWrite(FAN_ON_PIN,LOW); // Fan is on active high

  pinMode(FOCUSER_EN_PIN, OUTPUT); 
  digitalWrite(FOCUSER_EN_PIN,HIGH); // Focuser enable is active low
  pinMode(FOCUSER_STEP_PIN, OUTPUT); 
  digitalWrite(FOCUSER_STEP_PIN,LOW); // Focuser Step is active high
  pinMode(FOCUSER_DIR_PIN, OUTPUT); 
  digitalWrite(FOCUSER_DIR_PIN,LOW); // Focuser Direction
  pinMode(FOCUSER_SLEEP_PIN, OUTPUT); 
  digitalWrite(FOCUSER_SLEEP_PIN,HIGH); // Focuser motor driver not sleeping

#if ODRIVE_COMM_MODE == OD_UART
  ODRIVE_SERIAL.begin(ODRIVE_SERIAL_BAUD);
  VLF("MSG: ODrive, SERIAL channel init");
#elif ODRIVE_COMM_MODE == OD_CAN
  // .begin is done by the constructor
  // in ODriveTeensyCAN.cpp
  VLF("MSG: ODrive, CAN channel init");
#endif

  // Initialize Touchscreen *NOTE: must occur before display.init() since SPI.begin() is done here
  VLF("MSG: TouchScreen, Initializing");
  touchScreen.init();

  // Initialize TFT Display
  VLF("MSG: Display, Initializing");
  display.init();

#ifdef ENABLE_TFT_MIRROR
  // Communication channel between Teensy and ESP32-S3 for WiFi Screen Mirror
  usbBegin();
   
  // Task to poll the ESP32-S3 communication state
  VF("MSG: Starting espPoll task (10ms, priority 3)... ");
  if (tasks.add(10, 0, true, 3, espWrapper, "espPoll")) {
    VLF("success");
  } else {
    VLF("FAILED to start espPoll task!");
  }
#endif

// start touchscreen task
  VF("MSG: Setup, start TouchScreen polling task (rate 333 ms priority 6)... ");
  uint8_t TShandle = tasks.add(250, 0, true, 3, touchWrapper, "TouchScreen");
  if (TShandle) {
    VLF("success");
  } else {
    VLF("FAILED!");
  }
  tasks.setTimingMode(TShandle, TM_MINIMUM);

  // Update currently selected screen status
  //   NOTE: this task MUST be a lower priority than the TouchScreen task to prevent
  //   race conditions that result in the WiFi uncompressedBuffer being overwritten
  VF("MSG: Setup, start Screen status update task (rate 1000 ms priority 6)... ");
  uint8_t us_handle = tasks.add(1000, 0, true, 5, updateScreenWrapper, "UpdateSpecificScreen");
  if (us_handle)  { VLF("success"); } else { VLF("FAILED!"); }

#ifdef ODRIVE_MOTOR_PRESENT
  VF("MSG: ODrive, ODRIVE_SWAP_AXES = "); if(ODRIVE_SWAP_AXES) VLF("ON"); else VLF("OFF");
  VF("MSG: ODrive, ODRIVE_COMM_MODE = "); if(ODRIVE_COMM_MODE == OD_UART) VLF("SERIAL"); else VLF("CAN bus");
#endif

  VLF("MSG: Draw HomeScreen");
  homeScreen.draw();

  // create/start a task to show the profiler at work
#if SHOW_TASKS_PROFILER_EVERY_SEC == ON
profilerHandle = tasks.add(250, 0, true, 2, profiler, "Profilr");
SERIAL_DEBUG.println("Profiler:  running every second");
SERIAL_DEBUG.println();
#endif
}

DDScope dDScope;