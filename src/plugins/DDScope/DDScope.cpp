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
#include "src/Common.h"
#include "display/Display.h"
#include "screens/TouchScreen.h"

void DDScope::init() {

  VLF("MSG: Plugins, starting: DDScope");

  // Initialize Touchscreen...must occur before display.init() since SPI.begin() is done here
  VLF("MSG: TouchScreen, Initializing");
  touchScreen.init();

  // Initialize TFT Display
  VLF("MSG: Display, Initializing");
  display.init();
}

DDScope dDScope;