// =====================================================
// HomeScreen.cpp

// Author: Richard Benear 3/20/21

#include "HomeScreen.h"
#include "../display/Display.h"
#include "../../../lib/tls/GPS.h"
#include "../../../telescope/mount/Mount.h"
#include "../odrive/Odrive.h"

// Column 1 Home Screen
#define COL1_LABELS_X            3
#define COL1_LABELS_Y            168
#define COL1_LABEL_SPACING       18
#define COL1_DATA_BOXSIZE_X      70
#define COL1_DATA_X              84
#define COL1_DATA_Y              COL1_LABELS_Y

// Column 2 Home Screen
#define COL2_LABELS_X            170
#define COL2_LABELS_Y            COL1_LABELS_Y
#define COL2_LABEL_SPACING       COL1_LABEL_SPACING
#define COL2_DATA_X              269
#define COL2_DATA_Y              COL1_DATA_Y
#define COL2_DATA_BOXSIZE_X      70
#define COL2_DATA_BOXSIZE_Y      COL1_LABEL_SPACING

// Buttons for actions that are not page selections
#define ACTION_BOXSIZE_X         100 
#define ACTION_BOXSIZE_Y         36 
#define ACTION_COL_1_X           3 
#define ACTION_COL_1_Y           324
#define ACTION_COL_2_X           ACTION_COL_1_X+ACTION_BOXSIZE_X+4
#define ACTION_COL_2_Y           ACTION_COL_1_Y
#define ACTION_COL_3_X           ACTION_COL_2_X+ACTION_BOXSIZE_X+4
#define ACTION_COL_3_Y           ACTION_COL_1_Y
#define ACTION_X_SPACING         7
#define ACTION_Y_SPACING         4
#define ACTION_TEXT_X_OFFSET     10
#define ACTION_TEXT_Y_OFFSET     20

#define MOTOR_CURRENT_WARNING  2.0  // Warning when over 2 amps....coil heating occuring


// ============================================
// ======= Draw Base content of HOME PAGE =====
// ============================================
void HomeScreen::draw() {
  display.currentScreen = HOME_SCREEN;
  tft.setTextSize(1);
  display.updateColors();
  tft.setTextColor(display.textColor);
  tft.fillScreen(display.pgBackground);
  display.drawMenuButtons();
  display.drawTitle(20, 30, "DIRECT-DRIVE SCOPE");
  tft.drawFastVLine(165, 155, 165,display.textColor);
  tft.setFont(&Inconsolata_Bold8pt7b); 
  display.drawCommonStatusLabels();
  display.updateOnStepCmdStatus();
  
  //========== Status Text ===========
  // Draw Status Labels for Real Time data only here, no data displayed
  int y_offset = 0;
  
  // Show Current Local Time
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("Time-----:");

  // Show LST
  y_offset +=COL1_LABEL_SPACING;
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("LST------:");
  
  // Display Latitude
  y_offset +=COL1_LABEL_SPACING;
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("Latitude-:");

  // Display Longitude
  y_offset +=COL1_LABEL_SPACING;
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("Longitude:");

  // Display ambient Temperature
  y_offset +=COL1_LABEL_SPACING;
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("Temperat-:");

  // Display ambient Humidity
  y_offset +=COL1_LABEL_SPACING;
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("Humidity-:");

  // Display Dew Point
  y_offset +=COL1_LABEL_SPACING;
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("Dew Point:");
  
  // Display Altitude
  y_offset +=COL1_LABEL_SPACING;
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("Altitude-:");

  // Battery Voltage
  y_offset +=COL1_LABEL_SPACING;
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("Battery V:");

//======= 2nd Column =======
// Motor encoder positions
  y_offset = 0;
  tft.setCursor(COL2_LABELS_X, COL2_LABELS_Y + y_offset);
  tft.print("AZM enc deg:");

  y_offset +=COL2_LABEL_SPACING;
  tft.setCursor(COL2_LABELS_X, COL2_LABELS_Y + y_offset);
  tft.print("ALT enc deg:");

  // Motor currents
  y_offset +=COL2_LABEL_SPACING;
  tft.setCursor(COL2_LABELS_X, COL2_LABELS_Y + y_offset);
  tft.print("AZM Ibus---:");

  y_offset +=COL2_LABEL_SPACING;
  tft.setCursor(COL2_LABELS_X, COL2_LABELS_Y + y_offset);
  tft.print("ALT Ibus---:");

  // ALT Motor Temperature
  y_offset +=COL2_LABEL_SPACING;
  tft.setCursor(COL2_LABELS_X, COL2_LABELS_Y + y_offset);
  tft.print("ALT MotTemp:");

   // AZ Motor Temperature
  y_offset +=COL2_LABEL_SPACING;
  tft.setCursor(COL2_LABELS_X, COL2_LABELS_Y + y_offset);
  tft.print("AZM MotTemp:");
}

// update multiple status items
void HomeScreen::updateStatusAll() {
  homeScreen.updateStatusCol1();
  tasks.yield(100);
  homeScreen.updateStatusCol2();
  tasks.yield(100);
  homeScreen.updateMountStatus();
  tasks.yield(100);
  homeScreen.updateHomeButtons();
}

// =================================================
// ============ Update HOME Screen Status ============
// =================================================
// Column 1 
void HomeScreen::updateStatusCol1() {
  char xchReply[10]="";
  int y_offset = 0; 

  //display.updateCommonStatus();

  // Show Local Time
  display.getLocalCmdTrim(":GL#", xchReply); 
  if (strcmp(curTime, xchReply) !=0 || display.firstDraw) {
    display.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, xchReply);
    strcpy(curTime, xchReply);
  }

  // Show LST
  y_offset +=COL1_LABEL_SPACING;
  display.getLocalCmdTrim(":GS#", xchReply); 
  if (strcmp(curLST, xchReply)!=0 || display.firstDraw) { 
    display.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, xchReply);
    strcpy(curLST, xchReply);
  }
   
  y_offset +=COL1_LABEL_SPACING;
  // Show Latitude
  display.getLocalCmdTrim(":Gt#", xchReply); 
  if (strcmp(curLatitude, xchReply)!=0 || display.firstDraw) {
    display.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, xchReply);
    strcpy(curLatitude, xchReply);
  }

  // Show Longitude
  y_offset +=COL1_LABEL_SPACING;
  display.getLocalCmdTrim(":Gg#", xchReply); 
  if (strcmp(curLongitude, xchReply)!=0 || display.firstDraw) {
    display.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, xchReply);
    strcpy(curLongitude, xchReply);
  }

  // Ambient Temperature
  y_offset +=COL1_LABEL_SPACING;
  display.getLocalCmdTrim(":GX9A#", xchReply); 
  double tempF = ((atof(xchReply)*9)/5) + 32;
  sprintf(xchReply, "%3.1f F", tempF); // convert back to string to right justify
  if (strcmp(curTemp, xchReply)!=0 || display.firstDraw) {
    display.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, xchReply);
    strcpy(curTemp, xchReply);
  }
   
  // Ambient Humidity
  y_offset +=COL1_LABEL_SPACING;
  display.getLocalCmdTrim(":GX9C#", xchReply); 
  if (strcmp(curHumidity, xchReply)!=0 || display.firstDraw) {
    display.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, xchReply);
    strcpy(curHumidity, xchReply);
  }
  
  // Ambient Dew Point
  y_offset +=COL1_LABEL_SPACING;
  display.getLocalCmdTrim(":GX9E#", xchReply); 
  if (strcmp(curDewpoint, xchReply)!=0 || display.firstDraw) {
    display.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, xchReply);
    strcpy(curDewpoint, xchReply);
  }  
    /*
  // Show Altitude
  y_offset +=COL1_LABEL_SPACING;
  //display.getLocalCmdTrim(":GX9D#", xchReply); 
  if (strcmp(curAlti, xchReply)!=0 || display.firstDraw) {
    display.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, xchReply);
    strcpy(curAlti, xchReply);
  }*/
/*
  // Update Battery Voltage
  y_offset +=COL1_LABEL_SPACING;
  currentBatVoltage = display.getBatteryVoltage();
  if ((currentBatVoltage != lastBatVoltage) || display.firstDraw) {
    if (currentBatVoltage < BATTERY_LOW_VOLTAGE) {
      display.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, currentBatVoltage);
    } else {
      display.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, currentBatVoltage);
    }
    lastBatVoltage = currentBatVoltage;
  }
  */
}

// =================================================
// ============ Update Column 2 Status =============
// =================================================
// Show ODrive encoder positions
// AZ encoder
void HomeScreen::updateStatusCol2() {
  int y_offset =0;
  int bitmap_width_sub = 30;
  currentAZEncPos = odrive.getEncoderPositionDeg(AZ);
  if ((currentAZEncPos != lastAZEncPos) || display.firstDraw) {
    display.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZEncPos);
    lastAZEncPos = currentAZEncPos;
  }
  
  // ALT encoder
  y_offset +=COL1_LABEL_SPACING;
  currentALTEncPos = odrive.getEncoderPositionDeg(ALT);
  if ((currentALTEncPos != lastALTEncPos) || display.firstDraw) {
    display.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTEncPos);
    lastALTEncPos = currentALTEncPos;
  }

  // Show ODrive motor currents
  // AZ current
  y_offset +=COL1_LABEL_SPACING;
  currentAZMotorCur = odrive.getMotorCurrent(AZ);
  if ((currentAZMotorCur != lastAZMotorCur) || display.firstDraw) {
    if (lastAZMotorCur > MOTOR_CURRENT_WARNING) {
      display.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZMotorCur);
    } else {
      display.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZMotorCur);
    }
  lastAZMotorCur = currentAZMotorCur;
  }
  
  // ALT current
  y_offset +=COL1_LABEL_SPACING;
  currentALTMotorCur = odrive.getMotorCurrent(ALT);
  if ((currentALTMotorCur != lastALTMotorCur) || display.firstDraw) {
    if (lastALTMotorCur > MOTOR_CURRENT_WARNING) {
      display.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTMotorCur);
    } else {
      display.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTMotorCur);
    }
  lastALTMotorCur = currentALTMotorCur;
  }
 
  // ALT Motor Temperature
  y_offset +=COL1_LABEL_SPACING;
  currentALTMotorTemp = odrive.getMotorTemp(ALT);
  if ((currentALTMotorTemp != lastALTMotorTemp) || display.firstDraw) {
    if (currentALTMotorTemp >= 120) { // make box red
      display.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTMotorTemp);
    } else {
      display.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTMotorTemp);
    }
    lastALTMotorTemp = currentALTMotorTemp;
  }

  // AZ Motor Temperature
  y_offset +=COL1_LABEL_SPACING;
  currentAZMotorTemp = odrive.getMotorTemp(AZ);
  if ((currentAZMotorTemp != lastAZMotorTemp) || display.firstDraw) {
    if (currentAZMotorTemp >= 120) { // make box red
    display.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZMotorTemp);
    } else {
      display.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZMotorTemp);
    }
    lastAZMotorTemp = currentAZMotorTemp;
  }
}

  // =================================================
  // ============== Update MOUNT Status ==============
  // =================================================
void HomeScreen::updateMountStatus() {
  int x_offset = 10; // offset this to make easier to pick it up with the eye
  int y_offset = COL1_LABEL_SPACING*6+1;

  // Parking Status
  if (mountStatus.isParked()) {
    display.canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, " Parked    ");
  } else { 
    display.canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, " Not Parked");
  } 
  
  // Slewing
  y_offset +=COL1_LABEL_SPACING;
  if (mountStatus.isSlewing()) {
    display.canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, "  Slewing   ");
  } else {
    display.canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, " Not Slewing");
  }

  // Homing Status
  y_offset +=COL1_LABEL_SPACING;
  if (mountStatus.isHome()) {
    display.canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, " Homed  ");   
  } else { 
    display.canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, "Not Home");
  }
}

// ===============================================
// ============ Update Home Buttons ==============
// ===============================================
void HomeScreen::updateHomeButtons() {
  if (display.screenTouched || display.firstDraw || display.refreshScreen) {
    display.refreshScreen = false;
    if (display.screenTouched) display.refreshScreen = true;
    char xchReply[10]="";
    int x_offset = 0;
    int y_offset = 0;
    tft.setTextColor(display.textColor);
    
    // ============== Column 1 ===============
    // Enable / Disable Azimuth Motor
    if (odrive.odriveAZOff) {
      display.drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET, "  EN AZ   ");
    } else { //motor on
      display.drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,   "AZ Enabled");
    }
    // Enable / Disable Altitude Motor
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (odrive.odriveALTOff) {
      display.drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET, "  EN ALT   ");
    } else { //motor on
      display.drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET,   "ALT Enabled");
    }
    // Stop all movement
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (stopButton) {
      display.drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,    "AllStopped");
      stopButton = false;
    } else { 
      display.drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET+5, ACTION_TEXT_Y_OFFSET, "  STOP!  ");
    }

    // ============== Column 2 ===============
    y_offset = 0;
    // Start / Stop Tracking
    if (!mountStatus.isTracking()) { 
      display.drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET, "Start Track");
    } else { 
      display.drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,     " Tracking  ");
    }
    
    // Night / Day Mode
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (!display.nightMode) {
      display.drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET, "Night Mode");   
    } else {
      display.drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET, " Day Mode");          
    }
    // Home Telescope
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (gotoHome) {
      display.drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,   "  Homing ");
      gotoHome = false;             
    } else {
      display.drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET, "  Go Home ");
    }  

    // ============== Column 3 ===============
    // Park / unPark Telescope
    y_offset = 0;
    if (mountStatus.isParked()) { 
      display.drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET, " Parked ");
    } else { 
      display.drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,   "  Un Park ");
    }

    // Set Park Position
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (parkWasSet) {
      display.drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,     "Park WasSet");
      parkWasSet = false;
    } else {
      display.drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET-7, ACTION_TEXT_Y_OFFSET, "  Set Park ");
    }
    // Turn ON / OFF Fan
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (!fanOn) {
      display.drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET, "  Fan Off ");
    } else {
      display.drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET,   "  Fan On  ");
    }
    display.screenTouched = false;
  }
  display.firstDraw = false;
}

// =================================================
// ============== Update Touchscreen ===============
// =================================================
void HomeScreen::touchPoll() {
  int x_offset = 0;
  int y_offset = 0;
  char xchReply[12]="";
  
  // ======= Column 1 - Leftmost =======
  // Enable Azimuth motor
  if (p.x > ACTION_COL_1_X + x_offset && p.x < ACTION_COL_1_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_1_Y + y_offset && p.y <  ACTION_COL_1_Y + y_offset + ACTION_BOXSIZE_Y) {
    ddTone.click();
    if (odrive.odriveAZOff) { // toggle ON
      odrive.odriveAZOff = false; // false = NOT off
      digitalWrite(AZ_ENABLED_LED_PIN, LOW); // Turn On AZ LED
      odrive.turnOnOdriveMotor(AZ);
    } else { // since already ON, toggle OFF
      odrive.odriveAZOff = true;
      digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZ LED
      odriveMotor.power(AZ);
    }
  }
            
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  // Enable Altitude motor
  if (p.x > ACTION_COL_1_X + x_offset && p.x < ACTION_COL_1_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_1_Y + y_offset && p.y <  ACTION_COL_1_Y + y_offset + ACTION_BOXSIZE_Y) {
    ddTone.click();
    if (odrive.odriveALTOff) { // toggle ON
      odrive.odriveALTOff = false; // false = NOT off
      digitalWrite(ALT_ENABLED_LED_PIN, LOW); // Turn On ALT LED
      odrive.turnOnOdriveMotor(ALT);
    } else { // toggle OFF
      odriveMotor.power(ALT); // Idle the Odrive channel
      odrive.odriveALTOff = true;
      digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn off ALT LED
    }
  }

  // STOP everthing requested
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (p.x > ACTION_COL_1_X + x_offset && p.x < ACTION_COL_1_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_1_Y + y_offset && p.y <  ACTION_COL_1_Y + y_offset + ACTION_BOXSIZE_Y) {
    if (!stopButton) {
      ddTone.click();
      stopButton = true;
      odrive.stopMotors();
    }
  }
  // ======= COLUMN 2 of Buttons - Middle =========
  // Start/Stop Tracking
  y_offset = 0;
  if (p.x > ACTION_COL_2_X + x_offset && p.x < ACTION_COL_2_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_2_Y + y_offset && p.y <  ACTION_COL_2_Y + y_offset + ACTION_BOXSIZE_Y) {
    ddTone.click();
    if (!mountStatus.isTracking()) {
      display.setLocalCmd(":Te#"); // Enable Tracking
    } else {
      display.setLocalCmd(":Td#"); // Disable Tracking
    } 
  }

  // Set Night or Day Mode
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (p.x > ACTION_COL_2_X + x_offset && p.x < ACTION_COL_2_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_2_Y + y_offset && p.y <  ACTION_COL_2_Y + y_offset + ACTION_BOXSIZE_Y) {
    ddTone.click();
    if (!display.nightMode) {
    display.nightMode = true; // toggle on
    } else {
    display.nightMode = false; // toggle off
    }
    display.updateColors();
    display.firstDraw = true;
    homeScreen.draw(); return;
  }
  
  // Go to Home Telescope 
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (p.x > ACTION_COL_2_X + x_offset && p.x < ACTION_COL_2_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_2_Y + y_offset && p.y <  ACTION_COL_2_Y + y_offset + ACTION_BOXSIZE_Y) {
    ddTone.click();
    display.setLocalCmd(":hC#"); // go HOME
    gotoHome = true;
  }
  
  // ======== COLUMN 3 of Buttons - Leftmost ========
  // Park and UnPark Telescope
  y_offset = 0;
  if (p.x > ACTION_COL_3_X + x_offset && p.x < ACTION_COL_3_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_3_Y + y_offset && p.y <  ACTION_COL_3_Y + y_offset + ACTION_BOXSIZE_Y) {
    ddTone.click();
    if (!mountStatus.isParked()) { 
      display.setLocalCmd(":hP#"); // go Park
    } else { // already parked
      display.setLocalCmd(":hR#"); // Un park position
    }
  }

  // Set Park Position to Current
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (p.x > ACTION_COL_3_X + x_offset && p.x < ACTION_COL_3_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_3_Y + y_offset && p.y <  ACTION_COL_3_Y + y_offset + ACTION_BOXSIZE_Y) {
    ddTone.click();
    display.setLocalCmd(":hQ#"); // Set Park Position
    parkWasSet = true;
  }

  // Fan Control Action Button
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (p.x > ACTION_COL_3_X + x_offset && p.x < ACTION_COL_3_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_3_Y + y_offset && p.y <  ACTION_COL_3_Y + y_offset + ACTION_BOXSIZE_Y) {
    ddTone.click();
    if (!fanOn) {
    digitalWrite(FAN_ON_PIN, HIGH);
    fanOn = true;
    } else {
    digitalWrite(FAN_ON_PIN, LOW);
    fanOn = false;
    }
  }
}

HomeScreen homeScreen;