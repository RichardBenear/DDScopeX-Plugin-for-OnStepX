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
#include "display/Display.h"
#include "../../lib/tasks/OnTask.h"
#include "../../lib/serial/Serial_Local.h"

void updateScreenWrapper() { dDScope.specificScreenUpdate(); }
void updateCommonWrapper() { display.updateCommonStatus(); }
void updateOnStepCmdWrapper() { display.updateOnStepCmdStatus(); }
void touchWrapper()  { display.touchScreenPoll(); }

void DDScope::init() {

  VLF("MSG: Plugins, starting: DDScope");

  SerialLocal serialLocal;

  // Initialize TFT Display and Touchscreen
  VLF("MSG: Display, Initializing");
  display.init();
  
  // touchscreen task
  VF("MSG: Setup, start input touch screen polling task (rate 300 ms priority 2)... ");
  if (tasks.add(300, 0, true, 2, touchWrapper, "TouchScreen"))  { VLF("success"); } else { VLF("FAILED!"); }

  // update this common-among-screens status
  VF("MSG: Setup, start screen update Common status polling task (rate 900 ms priority 7)... ");
  uint8_t CShandle = tasks.add(900, 0, true, 7, updateCommonWrapper, "UpdateCommonScreen");
  if (CShandle)  { VLF("success"); } else { VLF("FAILED!"); }
  tasks.setTimingMode(CShandle, TM_MINIMUM);

  // update the OnStep Cmd Error status display
  VF("MSG: Setup, start screen update OnStep CMD status polling task (rate 1100 ms priority 7)... ");
  uint8_t CDhandle = tasks.add(1000, 0, true, 7, updateOnStepCmdWrapper, "UpdateOnStepCmdScreen");
  if (CDhandle) { VLF("success"); } else { VLF("FAILED!"); }
  tasks.setTimingMode(CDhandle, TM_MINIMUM);

  // update this specific screen status
  VF("MSG: Setup, start screen update This screen status polling task (rate 3000 ms priority 7)... ");
  uint8_t SShandle = tasks.add(3000, 0, true, 7, updateScreenWrapper, "UpdateSpecificScreen");
  if (SShandle)  { VLF("success"); } else { VLF("FAILED!"); }
  tasks.setTimingMode(SShandle, TM_MINIMUM);
}

// select which screen to update
void DDScope::specificScreenUpdate() {
  if (display.lastScreen != display.currentScreen) {
    display.firstDraw = true;
    display.lastScreen = display.currentScreen;
  }
  
  switch (display.currentScreen) {
    case HOME_SCREEN:     homeScreen.updateThisStatus();     break;
    case GUIDE_SCREEN:    guideScreen.updateThisStatus();    break;
    case FOCUSER_SCREEN:  focuserScreen.updateThisStatus();  break;
    case GOTO_SCREEN:     gotoScreen.updateThisStatus();     break;
    case MORE_SCREEN:     moreScreen.updateThisStatus();     break;
    case ODRIVE_SCREEN:   oDriveScreen.updateThisStatus();   break;
    case SETTINGS_SCREEN: settingsScreen.updateThisStatus(); break;
    case ALIGN_SCREEN:    alignScreen.updateThisStatus();    break;
    case CATALOG_SCREEN:  catalogScreen.updateThisStatus();  break;
    case PLANETS_SCREEN:  planetsScreen.updateThisStatus();  break;
    case CUST_CAT_SCREEN: catalogScreen.updateThisStatus();  break;
  }
  display.firstDraw = false;

  tasks.yield();
}

DDScope dDScope;
