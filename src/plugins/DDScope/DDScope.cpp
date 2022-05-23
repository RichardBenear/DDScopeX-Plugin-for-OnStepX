//
// Title:  DDScopeX (Plugin for OnStep)
// Author: Richard Benear
//
// Description:
// Direct Drive Telescope plugin for OnStep
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

#include <Arduino.h>
#include "../../Common.h"
#include "DDScope.h"
#include "display/Display.h"
#include "../../lib/axis/motor/oDrive/ODrive.h"
#include "screens/HomeScreen.h"
#include "../../telescope/mount/site/Site.h"

void updateWrapper() { dDScope.update(); }
void touchWrapper() { display.touchScreenPoll(); }

void DDScope::init() {

  VLF("MSG: Plugins, starting: DDScope");
  pinMode(ALT_THERMISTOR_PIN, INPUT); // Analog input
  pinMode(AZ_THERMISTOR_PIN, INPUT); // Analog input
  
  pinMode(ODRIVE_RST, OUTPUT);
  digitalWrite(ODRIVE_RST, LOW); // start ODrive in Reset

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

  SerialLocal serialLocal;

  // Initialize TFT Display and Touchscreen
  VLF("MSG: Initializing Display");
  display.init();

  // Initialize the SD card and startup screen
  VLF("MSG: Initializing SD Card");
  display.sdInit();

  VLF("MSG: Initializing ODrive");
  odriveMotor.init();

  // task parameters are:
  // handle  = tasks.add(period_ms, duration_ms, repeat_true_or_false, priority_0to7, callback_function);
  // success = tasks.requestHardwareTimer(handle, hardware_timer_number_1to4, hardware_timer_priority_0to255);
  VF("MSG: Setup, start screen update status polling task (rate 2000 ms priority 7)... ");
  if (tasks.add(2000, 0, true, 7, updateWrapper, "UpdateStatus"))  { VLF("success"); } else { VLF("FAILED!"); }

  VF("MSG: Setup, start input touch screen polling task (rate 300 ms priority 6)... ");
  if (tasks.add(300, 0, true, 6, touchWrapper, "TouchScreen"))  { VLF("success"); } else { VLF("FAILED!"); }
}

void DDScope::update() {

  switch (display.currentScreen) {
    case HOME_SCREEN:     homeScreen.updateStatusAll();
    case GUIDE_SCREEN:    guideScreen.updateStatus();
    case FOCUSER_SCREEN:  focuserScreen.updateStatus();
    case GOTO_SCREEN:     gotoScreen.updateStatus();
    case MORE_SCREEN:     moreScreen.updateStatus();
    case ODRIVE_SCREEN:   odriveScreen.updateStatus();
    case SETTINGS_SCREEN: settingsScreen.updateStatus();
    case ALIGN_SCREEN:    alignScreen.updateStatus();
    case CATALOG_SCREEN:  catalogScreen.updateStatus();
    case PLANETS_SCREEN:  planetsScreen.updateStatus();
    case CUST_CAT_SCREEN: catalogScreen.updateStatus();
  }

  tasks.yield(100);
}

DDScope dDScope;
