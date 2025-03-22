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
#define DDScopeFwVersionMinor       01    // minor version 00 to 99

#include <Arduino.h>
#include "DDScope.h"
#include "src/Common.h"
#include "screens/TouchScreen.h"
#include "screens/HomeScreen.h"
#include "src/lib/tasks/OnTask.h"
#include "src/libApp/commands/ProcessCmds.h"
#ifdef ODRIVE_MOTOR_PRESENT
  #include "odriveExt/ODriveExt.h"
#endif

void updateScreenWrapper() { display.updateSpecificScreen(); }
void refreshButtonsWrapper() { display.refreshButtons(); }
void getBatVoltageWrapper() { display.updateBatVoltage(1); }
// void commonStatusWrapper() { display.updateCommonStatus(); }

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
  VLF("MSG: ODrive, CAN channel init");
#endif

  // Initialize Touchscreen *NOTE: must occur before display.init() since SPI.begin() is done here
  VLF("MSG: TouchScreen, Initializing");
  touchScreen.init();

  // Initialize TFT Display
  VLF("MSG: Display, Initializing");
  display.init();

  VLF("MSG: Draw HomeScreen");
  homeScreen.draw();
  
  // update currently selected screen status
  VF("MSG: Setup, start Screen status update task (rate 1000 ms priority 6)... ");
  uint8_t us_handle = tasks.add(1000, 0, true, 3, updateScreenWrapper, "UpdateSpecificScreen");
  if (us_handle)  { VLF("success"); } else { VLF("FAILED!"); }

  // update Battery Voltage
  VF("MSG: Setup, start Battery Voltage status update task (rate 5000 ms priority 6)... ");
  uint8_t bat_handle = tasks.add(5000, 0, true, 6, getBatVoltageWrapper, "UpdateBatteryVoltage");
  if (bat_handle)  { VLF("success"); } else { VLF("FAILED!"); }

  // // refresh Buttons
  VF("MSG: Setup, refresh Buttons (rate 1000 ms priority 4)... ");
  uint8_t rs_handle = tasks.add(1000, 0, true, 6, refreshButtonsWrapper, "RefreshButtons");
  if (rs_handle) { VLF("success"); } else { VLF("FAILED!"); }

  // // check for General Errors
  // VF("MSG: Setup, General Error check (rate 1200 ms priority 7)... ");
  // uint8_t ge_handle = tasks.add(1000, 0, true, 7, generalErrorWrapper, "GeneralErrors");
  // if (ge_handle) { VLF("success"); } else { VLF("FAILED!"); }

  #ifdef ODRIVE_MOTOR_PRESENT
    VF("MSG: ODrive, ODRIVE_SWAP_AXES = "); if(ODRIVE_SWAP_AXES) VLF("ON"); else VLF("OFF");
    VF("MSG: ODrive, ODRIVE_COMM_MODE = "); if(ODRIVE_COMM_MODE == OD_UART) VLF("SERIAL"); else VLF("CAN bus");
  #endif

  // create/start a task to show the profiler at work
#if SHOW_TASKS_PROFILER_EVERY_SEC == ON
profilerHandle = tasks.add(250, 0, true, 2, profiler, "Profilr");
Serial.println("Profiler:  running every second");
Serial.println();
#endif
}

DDScope dDScope;