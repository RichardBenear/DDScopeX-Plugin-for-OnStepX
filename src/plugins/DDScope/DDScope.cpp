// =====================================================
// DDScope.cpp
//
// Title:  DDScope (Plugin for OnStepX)
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
#define PluginFWVersionMajor       1
#define PluginFWVersionMinor       01    // minor version 00 to 99

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

#ifdef ODRIVE_MOTOR_PRESENT
  void updateBatVoltWrapper() { display.updateBatVoltage(); }
#endif

void DDScope::init() {

  VF("MSG: Plugins, starting:"); VLF(PluginName);

  // Initilize custom pins...may want to move some of these to Features in future
  pinMode(ALT_THERMISTOR_PIN, INPUT); // Analog input
  pinMode(AZ_THERMISTOR_PIN, INPUT); // Analog input

  pinMode(AZ_ENABLED_LED_PIN, OUTPUT);
  digitalWrite(AZ_ENABLED_LED_PIN,HIGH); // LED OFF, active low 
  pinMode(ALT_ENABLED_LED_PIN, OUTPUT);
  digitalWrite(ALT_ENABLED_LED_PIN,HIGH); // LED OFF, active low

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

  // Initialize Touchscreen *NOTE: must occur before display.init() since SPI.begin() is done here
  VLF("MSG: TouchScreen, Initializing");
  touchScreen.init();

  // Initialize TFT Display
  VLF("MSG: Display, Initializing");
  display.init();

  VLF("MSG: Draw HomeScreen");
  homeScreen.draw();

  // update currently selected screen status
  VF("MSG: Setup, start Screen status polling task (rate 1500 ms priority 6)... ");
  uint8_t us_handle = tasks.add(1500, 0, true, 6, updateScreenWrapper, "UpdateSpecificScreen");
  if (us_handle)  { VLF("success"); } else { VLF("FAILED!"); }

  // refresh Buttons
  VF("MSG: Setup, refresh Buttons (rate 1500 ms priority 6)... ");
  uint8_t rs_handle = tasks.add(1500, 0, true, 6, refreshButtonsWrapper, "RefreshButtons");
  if (rs_handle) { VLF("success"); } else { VLF("FAILED!"); }

  #ifdef ODRIVE_MOTOR_PRESENT
    // update battery voltage
    VF("MSG: Setup, start Battery voltage polling task (rate 10000 ms priority 6)... ");
    uint8_t bv_handle = tasks.add(10000, 0, true, 6, updateBatVoltWrapper, "UpdateBatVolt");
    if (bv_handle)  { VLF("success"); } else { VLF("FAILED!"); }
  
    VF("MSG: ODrive, ODRIVE_SWAP_AXES = "); if(ODRIVE_SWAP_AXES) VLF("ON"); else VLF("OFF");
  #endif
}

DDScope dDScope;