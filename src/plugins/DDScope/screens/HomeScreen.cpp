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
#define COL1_LABELS_Y            180
#define COL1_LABEL_SPACING       21
#define COL1_DATA_BOXSIZE_X      70
#define COL1_DATA_X              84
#define COL1_DATA_Y              COL1_LABELS_Y

// Column 2 Home Screen
#define COL2_LABELS_X            170
#define COL2_LABELS_Y            COL1_LABELS_Y
#define COL2_LABEL_SPACING       COL1_LABEL_SPACING
#define COL2_DATA_X              267
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
#define COL_1_NUM_ROWS 7
#define COL_1_ROW_1_S_STR "Time-----:"
#define COL_1_ROW_2_S_STR "LST------:"
#define COL_1_ROW_3_S_STR "Latitude-:"
#define COL_1_ROW_4_S_STR "Longitude:"
#define COL_1_ROW_5_S_STR "Temperat-:"
#define COL_1_ROW_6_S_STR "Humidity-:"
#define COL_1_ROW_7_S_STR "Dew Point:"
//#define COL_1_ROW_8_S_STR "Altitude-:"

// Column 2 Status strings
#define COL_2_NUM_ROWS 6
#define COL_2_ROW_1_S_STR "AZM enc deg:"
#define COL_2_ROW_2_S_STR "ALT enc deg:"
#define COL_2_ROW_3_S_STR "AZM Ibus---:"
#define COL_2_ROW_4_S_STR "ALT Ibus---:"
#define COL_2_ROW_5_S_STR "AZM MotTemp:"
#define COL_2_ROW_6_S_STR "ALT MotTemp:"

// Column 1 Command strings
#define COL_1_ROW_1_C_STR ":GL#"
#define COL_1_ROW_2_C_STR ":GS#"
#define COL_1_ROW_3_C_STR ":Gt#"
#define COL_1_ROW_4_C_STR ":Gg#"
#define COL_1_ROW_5_C_STR ":GX9A#" // temperature deg C
#define COL_1_ROW_6_C_STR ":GX9C#" // humidity
#define COL_1_ROW_7_C_STR ":GX9E#" // dew point deg C
//#define COL_1_ROW_8_C_STR ":GX9D#" // altitiude

 static const char colOneStatusStr[COL_1_NUM_ROWS][12] = {
  COL_1_ROW_1_S_STR, COL_1_ROW_2_S_STR, COL_1_ROW_3_S_STR, COL_1_ROW_4_S_STR,
  COL_1_ROW_5_S_STR, COL_1_ROW_6_S_STR, COL_1_ROW_7_S_STR};

  static const char colTwoStatusStr[COL_2_NUM_ROWS][14] = {
  COL_2_ROW_1_S_STR, COL_2_ROW_2_S_STR, COL_2_ROW_3_S_STR, COL_2_ROW_4_S_STR,
  COL_2_ROW_5_S_STR, COL_2_ROW_6_S_STR};

static const char colOneCmdStr[COL_1_NUM_ROWS][8] = {
  COL_1_ROW_1_C_STR, COL_1_ROW_2_C_STR, COL_1_ROW_3_C_STR, COL_1_ROW_4_C_STR,
  COL_1_ROW_5_C_STR, COL_1_ROW_6_C_STR, COL_1_ROW_7_C_STR};

//void updateHomeScreenWrapper() { display.updateOnStepCmdStatus(); }

// ============================================
// ======= Draw Base content of HOME PAGE =====
// ============================================
void HomeScreen::draw() {
  setCurrentScreen(HOME_SCREEN);
  tft.setTextSize(1);
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  drawMenuButtons();
  drawTitle(25, TITLE_TEXT_Y, "DIRECT-DRIVE SCOPE");
  tft.setFont(&Inconsolata_Bold8pt7b);
  tft.drawFastVLine(165, COL1_LABELS_Y-12, 160, textColor);

  // ======Draw Status Text ===========
  // Labels for Real Time data only here, no data displayed yet
  int y_offset = 0;
  for (int i=0; i<COL_1_NUM_ROWS; i++) {
    tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
    tft.print(colOneStatusStr[i]);
    y_offset +=COL1_LABEL_SPACING;
  }

  y_offset = 0;
  for (int i=0; i<COL_2_NUM_ROWS; i++) {
    tft.setCursor(COL2_LABELS_X, COL2_LABELS_Y + y_offset);
    tft.print(colTwoStatusStr[i]);
    y_offset +=COL1_LABEL_SPACING;
  }

  drawCommonStatusLabels();
  updateHomeStatus(); 
  updateCommonStatus(); 
  updateHomeButtons();
}

// =================================================
// ========== Update HOME Screen Status ============
// =================================================

void HomeScreen::updateHomeStatus() {
  float currentAZEncPos     = 00.0;
  float currentALTEncPos    = 00.0;
  float currentAZMotorCur   = 00.0;
  float currentALTMotorCur  = 00.0;
  float currentALTMotorTemp = 00.0;
  float currentAZMotorTemp  = 00.0;
  char curCol1[11][8];

  updateCommonStatus(); 

  char xchReply[10]="";
  int y_offset = 0;

  // Column 1 poll updates
  for (int i=0; i<COL_1_NUM_ROWS; i++) {
    getLocalCmdTrim(colOneCmdStr[i], xchReply); 
    if (strcmp(curCol1[i], xchReply) != 0) {
      canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, xchReply);
      strcpy(curCol1[i], xchReply);
    }
    y_offset +=COL1_LABEL_SPACING;
  }

  // Column 2 poll updates
  int bitmap_width_sub = 30;
  y_offset =0;

  #ifdef ODRIVE_MOTOR_PRESENT
    // Show ODrive AZM encoder positions
    currentAZEncPos = oDriveExt.getEncoderPositionDeg(AZM_MOTOR);
  #elif
    currentAZEncPos = 0; // needs to be defined
  #endif

  canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZEncPos);

  
  // ALT encoder
  y_offset +=COL1_LABEL_SPACING;
  #ifdef ODRIVE_MOTOR_PRESENT
    // Show ODrive ALT encoder positions
    currentALTEncPos = oDriveExt.getEncoderPositionDeg(ALT_MOTOR);
  #elif
    currentALTEncPos = 0; // needs to be defined
  #endif

  canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTEncPos);
 

  // AZ current
  y_offset +=COL1_LABEL_SPACING;
  #ifdef ODRIVE_MOTOR_PRESENT
    // Show ODrive AZM motor current
    currentAZMotorCur = oDriveExt.getMotorCurrent(AZM_MOTOR);
  #elif
    currentAZMotorCur = 0; // needs to be defined
  #endif
  
  if (currentAZMotorCur > MOTOR_CURRENT_WARNING) {
    canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZMotorCur);
  } else {
    canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZMotorCur);
  }

  
  // ALT current
  y_offset +=COL1_LABEL_SPACING;
  #ifdef ODRIVE_MOTOR_PRESENT
    // Show ODrive ALT motor current
    currentALTMotorCur = oDriveExt.getMotorCurrent(ALT_MOTOR);
  #elif
    currentALTMotorCur = 0; // needs to be defined
  #endif

  if (currentALTMotorCur > MOTOR_CURRENT_WARNING) {
    canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTMotorCur);
  } else {
    canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTMotorCur);
  }

  // AZ Motor Temperature
  y_offset +=COL1_LABEL_SPACING;
  #ifdef ODRIVE_MOTOR_PRESENT
    currentAZMotorTemp = oDriveExt.getMotorTemp(AZM_MOTOR);
  #elif
    currentAZMotorTemp = 0; // needs to be defined
  #endif
  
  if (currentAZMotorTemp >= 120) { // make box red
    canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZMotorTemp);
  } else {
    canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentAZMotorTemp);
  }

  // ALT Motor Temperature
  y_offset +=COL1_LABEL_SPACING;
  #ifdef ODRIVE_MOTOR_PRESENT
    currentALTMotorTemp = oDriveExt.getMotorTemp(ALT_MOTOR);
    #elif
    currentALTMotorTemp = 0; // needs to be defined
  #endif

  if (currentALTMotorTemp >= 120) { // make box red
    canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTMotorTemp);
  } else {
    canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, currentALTMotorTemp);
  }
}

// ===============================================
// ============ Update Home Buttons ==============
// ===============================================
void HomeScreen::updateHomeButtons() {
  int x_offset = 0;
  int y_offset = 0;
  tft.setTextColor(textColor);
  
  // ============== Column 1 ===============
  // Enable / Disable Azimuth Motor
  if (oDriveExt.odriveAzmPwr || oDriveExt.getODriveCurrentState(AZM_MOTOR) == AXIS_STATE_CLOSED_LOOP_CONTROL) {
     drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,   "AZM Enabled");
  } else { //motor on
    drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET, "  EN AZM   ");
  }
  // Enable / Disable Altitude Motor
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (oDriveExt.odriveAltPwr || oDriveExt.getODriveCurrentState(ALT_MOTOR) == AXIS_STATE_CLOSED_LOOP_CONTROL) {
    drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET, "ALT Enabled");
  } else { //motor on
    drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET, "  EN ALT   ");
  }
  // Stop all movement
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (stopButton) {
    drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET,  "All Stopped");
    stopButton = false;
  } else { 
    drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET+5, ACTION_TEXT_Y_OFFSET, "  STOP!  ");
  }

  // ============== Column 2 ===============
  y_offset = 0;
  // Start / Stop Tracking
  if (!isTracking()) { 
    drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET, "Start Track");
  } else { 
    drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,    " Tracking  ");
  }
  
  // Night / Day Mode
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (display.getNightMode()) {
    drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET, "Night Mode");   
  } else { // Day mode
    drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,  " Day Mode");          
  }
  
  // Home Telescope
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (mount.isSlewing()) {
    drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_ON, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,   " Slewing ");
  } else if (isHome()) {
    drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,  "  Homed  ");
    //gotoHome = false;             
  } else {
    drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUTTON_OFF, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,  " Go Home ");
  }  

  // ============== Column 3 ===============
  // Park / unPark Telescope
  y_offset = 0;
  if (isParked()) { 
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
}

// =================================================
// =========== Check for Button Press ==============
// =================================================
bool HomeScreen::touchPoll(int16_t px, int16_t py) {
  int x_offset = 0;
  int y_offset = 0;
  
  // ======= Column 1 - Leftmost =======
  // Enable Azimuth motor
  if (px > ACTION_COL_1_X + x_offset && px < ACTION_COL_1_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_1_Y + y_offset && py <  ACTION_COL_1_Y + y_offset + ACTION_BOXSIZE_Y) {
    BEEP;
    if (!oDriveExt.odriveAzmPwr) { // if not On, toggle ON
      digitalWrite(AZ_ENABLED_LED_PIN, LOW); // Turn On AZ LED
      oDriveExt.odriveAzmPwr = true;
      motor1.power(true);
    } else { // since already ON, toggle OFF
      digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZ LED
      oDriveExt.odriveAzmPwr = false;
      motor1.power(false);
    }
    return true;
  }
            
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  // Enable Altitude motor
  if (px > ACTION_COL_1_X + x_offset && px < ACTION_COL_1_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_1_Y + y_offset && py <  ACTION_COL_1_Y + y_offset + ACTION_BOXSIZE_Y) {
    BEEP;
    if (!oDriveExt.odriveAltPwr) { // toggle ON
      digitalWrite(ALT_ENABLED_LED_PIN, LOW); // Turn On ALT LED
      oDriveExt.odriveAltPwr = true; 
      motor2.power(true);
    } else { // toggle OFF
      digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn off ALT LED
      oDriveExt.odriveAltPwr = false;
      motor2.power(false); // Idle the ODrive motor
    }
    return true;
  }

  // STOP everthing requested
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (px > ACTION_COL_1_X + x_offset && px < ACTION_COL_1_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_1_Y + y_offset && py <  ACTION_COL_1_Y + y_offset + ACTION_BOXSIZE_Y) {
    BEEP;
    if (!stopButton) {
      stopButton = true;

      digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZM LED
      oDriveExt.odriveAzmPwr = false; 
      motor1.power(false);
      digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn Off ALT LED
      oDriveExt.odriveAltPwr = false;
      motor2.power(false);
    }
    return true;
  }

  // ======= COLUMN 2 of Buttons - Middle =========
  // Start/Stop Tracking
  y_offset = 0;
  if (px > ACTION_COL_2_X + x_offset && px < ACTION_COL_2_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_2_Y + y_offset && py <  ACTION_COL_2_Y + y_offset + ACTION_BOXSIZE_Y) {
    BEEP;
    if (!isTracking()) {
      setLocalCmd(":Te#"); // Enable Tracking
      // set motor flags to true since tracking command enables motor power
      oDriveExt.odriveAltPwr = true;
      oDriveExt.odriveAzmPwr = true;
    } else {
      setLocalCmd(":Td#"); // Disable Tracking
    }
    return true; 
  }

  // Set Night or Day Mode
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (px > ACTION_COL_2_X + x_offset && px < ACTION_COL_2_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_2_Y + y_offset && py <  ACTION_COL_2_Y + y_offset + ACTION_BOXSIZE_Y) {
    BEEP;
    if (!getNightMode()) {
      setNightMode(true); // toggle on
    } else {
      setNightMode(false); // toggle off
    }
    draw(); // redraw new screen colors
    return true;
  }
  
  // Go to Home Telescope 
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (px > ACTION_COL_2_X + x_offset && px < ACTION_COL_2_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_2_Y + y_offset && py <  ACTION_COL_2_Y + y_offset + ACTION_BOXSIZE_Y) {
    BEEP;
    //setLocalCmd(":hF#"); // reset Home position
    _oDriveDriver->SetPosition(0, 0.0);
    _oDriveDriver->SetPosition(1, 0.0);
    setLocalCmd(":hC#"); // go HOME
    gotoHome = true;
    return true;
  }
  
  // ======== COLUMN 3 of Buttons - Leftmost ========
  // Park and UnPark Telescope
  y_offset = 0;
  if (px > ACTION_COL_3_X + x_offset && px < ACTION_COL_3_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_3_Y + y_offset && py <  ACTION_COL_3_Y + y_offset + ACTION_BOXSIZE_Y) {
    BEEP;
    if (!isParked()) { 
      setLocalCmd(":hP#"); // go Park
    } else { // already parked
      setLocalCmd(":hR#"); // Un park position
    }
    return true;
  }

  // Set Park Position to Current
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (px > ACTION_COL_3_X + x_offset && px < ACTION_COL_3_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_3_Y + y_offset && py <  ACTION_COL_3_Y + y_offset + ACTION_BOXSIZE_Y) {
    BEEP;
    setLocalCmd(":hQ#"); // Set Park Position
    parkWasSet = true;
    return true;
  }

  // Fan Control Action Button
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (px > ACTION_COL_3_X + x_offset && px < ACTION_COL_3_X + x_offset + ACTION_BOXSIZE_X && py > ACTION_COL_3_Y + y_offset && py <  ACTION_COL_3_Y + y_offset + ACTION_BOXSIZE_Y) {
    BEEP;
    if (!fanOn) {
      digitalWrite(FAN_ON_PIN, HIGH);
      fanOn = true;
    } else {
      digitalWrite(FAN_ON_PIN, LOW);
      fanOn = false;
    }
    return true;
  }
  return false;
}

HomeScreen homeScreen;