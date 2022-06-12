// =====================================================
// HomeScreen.cpp

// Author: Richard Benear 3/20/21

#include "HomeScreen.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "../../../telescope/mount/Mount.h"
#include "../../../lib/tasks/OnTask.h"

#ifdef ODRIVE_MOTOR_PRESENT
  #include "../odriveExt/ODriveExt.h"
#endif

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

#define MOTOR_CURRENT_WARNING    2.0  // Warning when over 2 amps....coil heating occuring

// ------------ Page Drawing Support ----------------
// Modify the following strings to customize the Home Screen
// Column 1 Status strings
#define COL_1_NUM_ROWS 8

#define COL_1_ROW_1_S_STR "Time-----:"
#define COL_1_ROW_2_S_STR "LST------:"
#define COL_1_ROW_3_S_STR "Latitude-:"
#define COL_1_ROW_4_S_STR "Longitude:"
#define COL_1_ROW_5_S_STR "Temperat-:"
#define COL_1_ROW_6_S_STR "Humidity-:"
#define COL_1_ROW_7_S_STR "Dew Point:"
#define COL_1_ROW_8_S_STR "Altitude-:"

static const char colOneStatusStr[COL_1_NUM_ROWS][12] = {
  COL_1_ROW_1_S_STR, COL_1_ROW_2_S_STR, COL_1_ROW_3_S_STR, COL_1_ROW_4_S_STR,
  COL_1_ROW_5_S_STR, COL_1_ROW_6_S_STR, COL_1_ROW_7_S_STR, COL_1_ROW_8_S_STR};

// Column 2 Status strings
#define COL_2_NUM_ROWS 6

#define COL_2_ROW_1_S_STR "AZM enc deg:"
#define COL_2_ROW_2_S_STR "ALT enc deg:"
#define COL_2_ROW_3_S_STR "AZM Ibus---:"
#define COL_2_ROW_4_S_STR "ALT Ibus---:"
#define COL_2_ROW_5_S_STR "AZM MotTemp:"
#define COL_2_ROW_6_S_STR "ALT MotTemp:"

static const char colTwoStatusStr[COL_2_NUM_ROWS][14] = {
  COL_2_ROW_1_S_STR, COL_2_ROW_2_S_STR, COL_2_ROW_3_S_STR, COL_2_ROW_4_S_STR,
  COL_2_ROW_5_S_STR, COL_2_ROW_6_S_STR};

// Column 1 Command strings
#define COL_1_ROW_1_C_STR ":GL#"
#define COL_1_ROW_2_C_STR ":GS#"
#define COL_1_ROW_3_C_STR ":Gt#"
#define COL_1_ROW_4_C_STR ":Gg#"
#define COL_1_ROW_5_C_STR ":GX9A#"
#define COL_1_ROW_6_C_STR ":GX9C#"
#define COL_1_ROW_7_C_STR ":GX9E#"
#define COL_1_ROW_8_C_STR ":GX9D#"

static const char colOneCmdStr[COL_1_NUM_ROWS][8] = {
  COL_1_ROW_1_C_STR, COL_1_ROW_2_C_STR, COL_1_ROW_3_C_STR, COL_1_ROW_4_C_STR,
  COL_1_ROW_5_C_STR, COL_1_ROW_6_C_STR, COL_1_ROW_7_C_STR, COL_1_ROW_8_C_STR};


// ============================================
// ======= Draw Base content of HOME PAGE =====
// ============================================
void HomeScreen::draw() {
  currentScreen = HOME_SCREEN;
  setDayNight();
  tft.setTextSize(1);
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  drawMenuButtons();
  drawTitle(25, TITLE_TEXT_Y, "DIRECT-DRIVE SCOPE");
  tft.drawFastVLine(165, 155, 165,textColor);
  drawCommonStatusLabels();
  tft.setFont(&Inconsolata_Bold8pt7b);
  
  //========== Status Text ===========
  // Draw Status Labels for Real Time data only here, no data displayed
  int y_offset = 0;
  for (int i=0; i<COL_1_NUM_ROWS; i++) {
    tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
    tft.print(colOneStatusStr[i]);
  }

  y_offset = 0;
  for (int i=0; i<COL_2_NUM_ROWS; i++) {
    tft.setCursor(COL2_LABELS_X, COL2_LABELS_Y + y_offset);
    tft.print(colTwoStatusStr[i]);
  }
}

// update multiple status items
void HomeScreen::updateThisStatus() {
  homeScreen.updateStatusCol1();
  homeScreen.updateStatusCol2();
  homeScreen.updateMountStatus();
  homeScreen.updateHomeButtons();
}

// =================================================
// ============ Update HOME Screen Status ============
// =================================================
// Column 1 poll updates
void HomeScreen::updateStatusCol1() {
  char xchReply[10]="";
  int y_offset = 0;

  for (int i=0; i<COL_1_NUM_ROWS; i++) {
    getLocalCmdTrim(colOneCmdStr[i], xchReply); 
    if (strcmp(curCol1[i], xchReply) !=0 || firstDraw) {
      canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, xchReply);
      strcpy(curCol1[i], xchReply);
    }
    y_offset +=COL1_LABEL_SPACING;
  }
}

// Column 2 poll updates
void HomeScreen::updateStatusCol2() {
  int bitmap_width_sub = 30;
  int y_offset =0;

  #ifdef ODRIVE_MOTOR_PRESENT
    // Show ODrive AZM encoder positions
    currentAZEncPos = oDriveExt.getEncoderPositionDeg(AZM_MOTOR);
  #elif
    currentAZEncPos = 0; // needs to be defined
  #endif
  if ((currentAZEncPos != lastAZEncPos) || firstDraw) {
    canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZEncPos);
    lastAZEncPos = currentAZEncPos;
  }
  
  // ALT encoder
  y_offset +=COL1_LABEL_SPACING;
  #ifdef ODRIVE_MOTOR_PRESENT
    // Show ODrive ALT encoder positions
    currentALTEncPos = oDriveExt.getEncoderPositionDeg(ALT_MOTOR);
  #elif
    currentALTEncPos = 0; // needs to be defined
  #endif
  if ((currentALTEncPos != lastALTEncPos) || firstDraw) {
    canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTEncPos);
    lastALTEncPos = currentALTEncPos;
  }

  // AZ current
  y_offset +=COL1_LABEL_SPACING;
  #ifdef ODRIVE_MOTOR_PRESENT
    // Show ODrive AZM motor current
    currentAZMotorCur = oDriveExt.getMotorCurrent(AZM_MOTOR);
  #elif
    currentAZMotorCur = 0; // needs to be defined
  #endif
  if ((currentAZMotorCur != lastAZMotorCur) || firstDraw) {
    if (lastAZMotorCur > MOTOR_CURRENT_WARNING) {
      canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZMotorCur);
    } else {
      canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZMotorCur);
    }
  lastAZMotorCur = currentAZMotorCur;
  }
  
  // ALT current
  y_offset +=COL1_LABEL_SPACING;
  #ifdef ODRIVE_MOTOR_PRESENT
    // Show ODrive ALT motor current
    currentALTMotorCur = oDriveExt.getMotorCurrent(ALT_MOTOR);
  #elif
    currentALTMotorCur = 0; // needs to be defined
  #endif
  if ((currentALTMotorCur != lastALTMotorCur) || firstDraw) {
    if (lastALTMotorCur > MOTOR_CURRENT_WARNING) {
      canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTMotorCur);
    } else {
      canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTMotorCur);
    }
  lastALTMotorCur = currentALTMotorCur;
  }

  // AZ Motor Temperature
  y_offset +=COL1_LABEL_SPACING;
  #ifdef ODRIVE_MOTOR_PRESENT
    currentAZMotorTemp = oDriveExt.getMotorTemp(AZM_MOTOR);
  #elif
    currentAZMotorTemp = 0; // needs to be defined
  #endif
  if ((currentAZMotorTemp != lastAZMotorTemp) || firstDraw) {
    if (currentAZMotorTemp >= 120) { // make box red
    canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZMotorTemp);
    } else {
      canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZMotorTemp);
    }
    lastAZMotorTemp = currentAZMotorTemp;
  }

  // ALT Motor Temperature
  y_offset +=COL1_LABEL_SPACING;
  #ifdef ODRIVE_MOTOR_PRESENT
    currentALTMotorTemp = oDriveExt.getMotorTemp(ALT_MOTOR);
    #elif
    currentALTMotorTemp = 0; // needs to be defined
  #endif
  if ((currentALTMotorTemp != lastALTMotorTemp) || firstDraw) {
    if (currentALTMotorTemp >= 120) { // make box red
      canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTMotorTemp);
    } else {
      canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTMotorTemp);
    }
    lastALTMotorTemp = currentALTMotorTemp;
  }
}

  // =================================================
  // ============== Update MOUNT Status ==============
  // =================================================
void HomeScreen::updateMountStatus() {
  int x_offset = 10; // offset this to make easier to pick it up with the eye
  int y_offset = COL1_LABEL_SPACING*6+1;

  // Parking Status
  if (lCmountStatus.isParked()) {
    canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, " Parked    ");
  } else { 
    canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, " Not Parked");
  } 
  
  // Slewing
  y_offset +=COL1_LABEL_SPACING;
  if (lCmountStatus.isSlewing()) {
    canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, "  Slewing   ");
  } else {
    canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, " Not Slewing");
  }

  // Homing Status
  y_offset +=COL1_LABEL_SPACING;
  if (lCmountStatus.isHome()) {
    canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, "  Homed ");   
  } else { 
    canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, "Not Home");
  }
}

// ===============================================
// ============ Update Home Buttons ==============
// ===============================================
void HomeScreen::updateHomeButtons() {
  if (screenTouched || firstDraw || refreshScreen) {
    refreshScreen = false;
    if (screenTouched) refreshScreen = true;

    int x_offset = 0;
    int y_offset = 0;
    tft.setTextColor(textColor);
    
    // ============== Column 1 ===============
    // Enable / Disable Azimuth Motor
    if (!oDriveExt.odriveAzmPwr) {
      drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET, "  EN AZM   ");
    } else { //motor on
      drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,   "AZM Enabled");
    }
    // Enable / Disable Altitude Motor
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (!oDriveExt.odriveAltPwr) {
      drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET, "  EN ALT   ");
    } else { //motor on
      drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET,   "ALT Enabled");
    }
    // Stop all movement
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (stopButton) {
      drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,    "AllStopped");
      stopButton = false;
    } else { 
      drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET+5, ACTION_TEXT_Y_OFFSET, "  STOP!  ");
    }

    // ============== Column 2 ===============
    y_offset = 0;
    // Start / Stop Tracking
    if (!lCmountStatus.isTracking()) { 
      drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET, "Start Track");
    } else { 
      drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,     " Tracking  ");
    }
    
    // Night / Day Mode
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (!nightMode) {
      drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET, "Night Mode");   
    } else {
      drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET, " Day Mode");          
    }
    
    // Home Telescope
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (gotoHome) {
      drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,   "  Homing ");
      gotoHome = false;             
    } else {
      drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET, "  Go Home ");
    }  

    // ============== Column 3 ===============
    // Park / unPark Telescope
    y_offset = 0;
    if (lCmountStatus.isParked()) { 
      drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET, " Parked ");
    } else { 
      drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,   " Un Park ");
    }

    // Set Park Position
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (parkWasSet) {
      drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,     "Park Is Set");
      parkWasSet = false;
    } else {
      drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET-7, ACTION_TEXT_Y_OFFSET, "  Set Park ");
    }

    // Turn ON / OFF Fan
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (!fanOn) {
      drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET, "  Fan Off ");
    } else {
      drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET,   "  Fan On  ");
    }
    screenTouched = false;
  }
}

// =================================================
// ============== Update Touchscreen ===============
// =================================================
void HomeScreen::touchPoll(int16_t px, int16_t py) {
  int x_offset = 0;
  int y_offset = 0;
  
  // ======= Column 1 - Leftmost =======
  // Enable Azimuth motor
  if (px > ACTION_COL_1_X + x_offset && px < ACTION_COL_1_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_1_Y + y_offset && py <  ACTION_COL_1_Y + y_offset + ACTION_BOXSIZE_Y) {
    DD_CLICK;
    if (!oDriveExt.odriveAzmPwr) { // if not On, toggle ON
      digitalWrite(AZ_ENABLED_LED_PIN, LOW); // Turn On AZ LED
      oDriveExt.odriveAzmPwr = true;
      motor1.power(true);
    } else { // since already ON, toggle OFF
      digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZ LED
      oDriveExt.odriveAzmPwr = false;
      motor1.power(false);
    }
    return;
  }
            
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  // Enable Altitude motor
  if (px > ACTION_COL_1_X + x_offset && px < ACTION_COL_1_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_1_Y + y_offset && py <  ACTION_COL_1_Y + y_offset + ACTION_BOXSIZE_Y) {
    DD_CLICK;
    if (!oDriveExt.odriveAltPwr) { // toggle ON
      digitalWrite(ALT_ENABLED_LED_PIN, LOW); // Turn On ALT LED
      oDriveExt.odriveAltPwr = true; 
      motor2.power(true);
    } else { // toggle OFF
      digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn off ALT LED
      oDriveExt.odriveAltPwr = false;
      motor2.power(false); // Idle the ODrive motor
    }
    return;
  }

  // STOP everthing requested
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (px > ACTION_COL_1_X + x_offset && px < ACTION_COL_1_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_1_Y + y_offset && py <  ACTION_COL_1_Y + y_offset + ACTION_BOXSIZE_Y) {
    DD_CLICK;
    if (!stopButton) {
      stopButton = true;

      digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZM LED
      oDriveExt.odriveAzmPwr = false; 
      motor1.power(false);
      digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn Off ALT LED
      oDriveExt.odriveAltPwr = false;
      motor2.power(false);
    }
    return;
  }

  // ======= COLUMN 2 of Buttons - Middle =========
  // Start/Stop Tracking
  y_offset = 0;
  if (px > ACTION_COL_2_X + x_offset && px < ACTION_COL_2_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_2_Y + y_offset && py <  ACTION_COL_2_Y + y_offset + ACTION_BOXSIZE_Y) {
    DD_CLICK;
    if (!lCmountStatus.isTracking()) {
      setLocalCmd(":Te#"); // Enable Tracking
      oDriveExt.odriveAltPwr = true;
      oDriveExt.odriveAzmPwr = true;
    } else {
      setLocalCmd(":Td#"); // Disable Tracking
    }
    return; 
  }

  // Set Night or Day Mode
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (px > ACTION_COL_2_X + x_offset && px < ACTION_COL_2_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_2_Y + y_offset && py <  ACTION_COL_2_Y + y_offset + ACTION_BOXSIZE_Y) {
    DD_CLICK;
    if (!nightMode) {
      nightMode = true; // toggle on
    } else {
      nightMode = false; // toggle off
    }
    setDayNight();
    firstDraw = true;
    homeScreen.draw(); 
    return;
  }
  
  // Go to Home Telescope 
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (px > ACTION_COL_2_X + x_offset && px < ACTION_COL_2_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_2_Y + y_offset && py <  ACTION_COL_2_Y + y_offset + ACTION_BOXSIZE_Y) {
    DD_CLICK;
    //setLocalCmd(":hF#"); // reset Home position
    //tasks.yield(4);
    _oDriveDriver->SetPosition(0, 0.0);
    _oDriveDriver->SetPosition(1, 0.0);
    //setLocalCmd(":hC#"); // go HOME
    gotoHome = true;
    return;
  }
  
  // ======== COLUMN 3 of Buttons - Leftmost ========
  // Park and UnPark Telescope
  y_offset = 0;
  if (px > ACTION_COL_3_X + x_offset && px < ACTION_COL_3_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_3_Y + y_offset && py <  ACTION_COL_3_Y + y_offset + ACTION_BOXSIZE_Y) {
    DD_CLICK;
    if (!lCmountStatus.isParked()) { 
      setLocalCmd(":hP#"); // go Park
    } else { // already parked
      setLocalCmd(":hR#"); // Un park position
    }
    return;
  }

  // Set Park Position to Current
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (px > ACTION_COL_3_X + x_offset && px < ACTION_COL_3_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_3_Y + y_offset && py <  ACTION_COL_3_Y + y_offset + ACTION_BOXSIZE_Y) {
    DD_CLICK;
    setLocalCmd(":hQ#"); // Set Park Position
    parkWasSet = true;
    return;
  }

  // Fan Control Action Button
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (px > ACTION_COL_3_X + x_offset && px < ACTION_COL_3_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_3_Y + y_offset && py <  ACTION_COL_3_Y + y_offset + ACTION_BOXSIZE_Y) {
    DD_CLICK;
    if (!fanOn) {
      digitalWrite(FAN_ON_PIN, HIGH);
      fanOn = true;
    } else {
      digitalWrite(FAN_ON_PIN, LOW);
      fanOn = false;
    }
    return;
  }
}

HomeScreen homeScreen;