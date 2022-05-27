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
#include "DDScope.h"
#include "../../pinmaps/Pins.DDT.h" // this must precede Display.h
#include "display/Display.h"
#include "../../lib/tasks/OnTask.h"
#include "../../lib/serial/Serial_Local.h"
#include "../../telescope/mount/Mount.h"

void updateWrapper() { dDScope.update(); }
void touchWrapper() { display.touchScreenPoll(); }

void DDScope::init() {

  VLF("MSG: Plugins, starting: DDScope");

  // note: put these pins in an init somewhere
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
  VLF("MSG: Display, Initializing");
  display.init();
  homeScreen.draw();

  // Initialize the SD card and startup screen
  display.sdInit();

  // task parameters are:
  // handle  = tasks.add(period_ms, duration_ms, repeat_true_or_false, priority_0to7, callback_function);
  // success = tasks.requestHardwareTimer(handle, hardware_timer_number_1to4, hardware_timer_priority_0to255);
  VF("MSG: Setup, start screen update status polling task (rate 2000 ms priority 7)... ");
  if (tasks.add(5000, 0, true, 7, updateWrapper, "UpdateScreen"))  { VLF("success"); } else { VLF("FAILED!"); }

  VF("MSG: Setup, start input touch screen polling task (rate 300 ms priority 4)... ");
  if (tasks.add(300, 0, true, 4, touchWrapper, "TouchScreen"))  { VLF("success"); } else { VLF("FAILED!"); }
}

void DDScope::update() {
  if (display.lastScreen != display.currentScreen) {
    display.firstDraw = true;
    display.lastScreen = display.currentScreen;
  }
  
  switch (display.currentScreen) {
    case HOME_SCREEN:     homeScreen.updateStatusAll();  break;
    case GUIDE_SCREEN:    guideScreen.updateStatus();    break;
    case FOCUSER_SCREEN:  focuserScreen.updateStatus();  break;
    case GOTO_SCREEN:     gotoScreen.updateStatus();     break;
    case MORE_SCREEN:     moreScreen.updateStatus();     break;
    case ODRIVE_SCREEN:   odriveScreen.updateStatus();   break;
    case SETTINGS_SCREEN: settingsScreen.updateStatus(); break;
    case ALIGN_SCREEN:    alignScreen.updateStatus();    break;
    case CATALOG_SCREEN:  catalogScreen.updateStatus();  break;
    case PLANETS_SCREEN:  planetsScreen.updateStatus();  break;
    case CUST_CAT_SCREEN: catalogScreen.updateStatus();  break;
  }
  display.firstDraw = false;
  
  tasks.yield();
}

DDScope dDScope;
