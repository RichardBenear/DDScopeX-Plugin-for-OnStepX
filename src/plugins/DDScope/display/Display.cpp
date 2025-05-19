// =====================================================
// Display.cpp
//
// Display Support - Common Display functions
// 3.5" RPi Touchscreen and Display
// SPI Interface
// Author: Richard Benear 3/30/2021 - refactor 6/22

// #include <Arduino.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>

// Fonts
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "../fonts/UbuntuMono_Bold8pt7b.h"
#include "../fonts/UbuntuMono_Bold11pt7b.h"
#include <Fonts/FreeSansBold12pt7b.h>

// DDScope specific
#include "Display.h"
#include "WifiDisplay.h"
#include "../catalog/Catalog.h"
#include "../screens/AlignScreen.h"
#include "../screens/TreasureCatScreen.h"
#include "../screens/CustomCatScreen.h"
#include "../screens/SHCCatScreen.h"
#include "../screens/DCFocuserScreen.h"
#include "../screens/GotoScreen.h"
#include "../screens/GuideScreen.h"
#include "../screens/HomeScreen.h"
#include "../screens/MoreScreen.h"
#include "../screens/PlanetsScreen.h"
#include "../screens/SettingsScreen.h"
#include "../screens/ExtStatusScreen.h"
#include "src/telescope/mount/limits/Limits.h"

#ifdef ODRIVE_MOTOR_PRESENT
  #include "../odriveExt/ODriveExt.h"
  #include "../screens/ODriveScreen.h"
#endif

#define TITLE_BOXSIZE_X         313
#define TITLE_BOXSIZE_Y          43
#define TITLE_BOX_X               3
#define TITLE_BOX_Y               2 

// Shared common Status 
#define COM_LABEL_Y_SPACE        17
#define COM_COL1_LABELS_X         8
#define COM_COL1_LABELS_Y       104
#define COM_COL1_DATA_X          74
#define COM_COL1_DATA_Y          COM_COL1_LABELS_Y
#define COM_COL2_LABELS_X       180
#define COM_COL2_DATA_X         245

#define L_CE_NONE                    "no errors"
#define L_CE_1                       "no error true"
#define L_CE_0                       "no error false/fail"
#define L_CE_CMD_UNKNOWN             "command unknown"
#define L_CE_REPLY_UNKNOWN           "invalid reply"
#define L_CE_PARAM_RANGE             "parameter out of range"
#define L_CE_PARAM_FORM              "bad parameter format"
#define L_CE_ALIGN_FAIL              "align failed"
#define L_CE_ALIGN_NOT_ACTIVE        "align not active"
#define L_CE_NOT_PARKED_OR_AT_HOME   "not parked or at home"
#define L_CE_PARKED                  "already parked"
#define L_CE_PARK_FAILED             "park failed"
#define L_CE_NOT_PARKED              "not parked"
#define L_CE_NO_PARK_POSITION_SET    "no park position set"
#define L_CE_SLEW_FAIL               "goto failed"
#define L_CE_LIBRARY_FULL            "library full"
#define L_CE_SLEW_ERR_BELOW_HORIZON  "goto below horizon"
#define L_CE_SLEW_ERR_ABOVE_OVERHEAD "goto above overhead"
#define L_CE_SLEW_ERR_IN_STANDBY     "slew in standby"
#define L_CE_SLEW_ERR_IN_PARK        "slew in park"
#define L_CE_SLEW_ERR_SLEW           "already in goto"
#define L_CE_SLEW_ERR_OUTSIDE_LIMITS "outside limits"
#define L_CE_SLEW_ERR_HARDWARE_FAULT "hardware fault"
#define L_CE_MOUNT_IN_MOTION         "mount in motion"
#define L_CE_SLEW_ERR_UNSPECIFIED    "other"
#define L_CE_UNK                     "unknown"

const char cmdErrStr[26][25] = {
L_CE_NONE, L_CE_1, L_CE_0, L_CE_CMD_UNKNOWN, L_CE_REPLY_UNKNOWN, L_CE_PARAM_RANGE,
L_CE_PARAM_FORM, L_CE_ALIGN_FAIL, L_CE_ALIGN_NOT_ACTIVE, L_CE_NOT_PARKED_OR_AT_HOME,
L_CE_PARKED, L_CE_PARK_FAILED, L_CE_NOT_PARKED, L_CE_NO_PARK_POSITION_SET, L_CE_SLEW_FAIL,
L_CE_LIBRARY_FULL, L_CE_SLEW_ERR_BELOW_HORIZON, L_CE_SLEW_ERR_ABOVE_OVERHEAD,
L_CE_SLEW_ERR_IN_STANDBY, L_CE_SLEW_ERR_IN_PARK, L_CE_SLEW_ERR_SLEW, L_CE_SLEW_ERR_OUTSIDE_LIMITS,
L_CE_SLEW_ERR_HARDWARE_FAULT, L_CE_MOUNT_IN_MOTION, L_CE_SLEW_ERR_UNSPECIFIED, L_CE_UNK
};

// copied from SWS MountStatus.h
// general (background) errors
#define L_GE_NONE "None"
#define L_GE_MOTOR_FAULT "Motor/driver fault"
#define L_GE_ALT_MIN "Below horizon limit" 
#define L_GE_LIMIT_SENSE "Limit sense"
#define L_GE_DEC "Dec limit exceeded"
#define L_GE_AZM "Azm limit exceeded"
#define L_GE_UNDER_POLE "Under pole limit exceeded"
#define L_GE_MERIDIAN "Meridian limit exceeded"
#define L_GE_SYNC "Sync safety limit exceeded"
#define L_GE_PARK "Park failed"
#define L_GE_GOTO_SYNC "Goto sync failed"
#define L_GE_UNSPECIFIED "Unknown error"
#define L_GE_ALT_MAX "Above overhead limit"
#define L_GE_WEATHER_INIT "Weather sensor init failed"
#define L_GE_SITE_INIT "Time or loc. not updated"
#define L_GE_NV_INIT "Init NV/EEPROM bad"
#define L_GE_OTHER "Unknown Error, code"

// Menu button object
Button menuButton(MENU_X, MENU_Y, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, butOnBackground, butBackground, butOutline, largeFontWidth, largeFontHeight, "");

// Canvas Print object Custom Font
CanvasPrint canvDisplayInsPrint(&Inconsolata_Bold8pt7b);
                
ScreenEnum Display::currentScreen = HOME_SCREEN;
bool Display::_nightMode = false;
float previousBatVoltage = 2.1;
char cmdErrGlobal[100] = "";
static CommandError latchedCmdErr = CE_NONE;
static unsigned long errorLatchStartTime = 0;
const unsigned long errorDisplayDuration = 5000;

TinyGPSPlus dgps;

uint16_t pgBackground = XDARK_MAROON;
uint16_t butBackground = BLACK;
uint16_t titleBackground = BLACK;
uint16_t butOnBackground = MAROON;
uint16_t textColor = DIM_YELLOW; 
uint16_t butOutline = ORANGE; 

// Local cmd channel object
CommandProcessor processor(9600, 'L');

Adafruit_ILI9486_Teensy tft; 
WifiDisplay wifiDisplay;

// =========================================
// ========= Initialize Display ============
// =========================================
void Display::init() {
  VLF("MSG: Display, started"); 
  tft.begin(); delay(1);
  sdInit(); // initialize the SD card and draw start screen

  tft.setRotation(0); // display rotation: Note it is different than touchscreen
  setNightMode(true); // always start up in Night Mode
  //delay(1500); // let start screen show for 1.5 sec

  // set some defaults
  // NOTE: change these for your own personal settings
  VLF("MSG: Setting up Limits, TZ, Site Name, Slew Speed");
  commandBool(":SG+06:00#"); // Set Default Time Zone
  commandBool(":Sh-02#"); //Set horizon limit -2 deg
  commandBool(":So87#"); // Set overhead limit 87 deg
  commandBool(":SMHome#"); // Set Site 0 name "Home"
}

// initialize the SD card and boot screen
void Display::sdInit() {
  if (!SD.begin(BUILTIN_SDCARD)) {
    VLF("MSG: SD Card, initialize failed");
  } else {
    VLF("MSG: SD Card, initialized");
  }

  // draw bootup screen
  File StarMaps;
  if((StarMaps = SD.open("NGC1566.bmp")) == 0) {
    VF("File Not Found");
    return;
  } 

  // Draw Start Page of NGC 1566 bitmap
  // tft.fillScreen(pgBackground); 
  // tft.setTextColor(textColor);
  // drawPic(&StarMaps, 1, 0, TFTWIDTH, TFT_HEIGHT);  
  // drawTitle(20, 30, "DIRECT-DRIVE SCOPE");
  // tft.setCursor(60, 80);
  // tft.setTextSize(2);
  // tft.print("Initializing");
  // tft.setTextSize(1);
  // tft.setCursor(120, 120);
  // tft.print("NGC 1566");
}

// Monitor any button that is waiting for a state change (other than being pressed)
// This does not include the Menu Buttons
void Display::refreshButtons() {
  //Serial.print(currentScreen);
  switch (currentScreen) {
    case HOME_SCREEN:      
      if (homeScreen.homeButStateChange()) 
        homeScreen.updateHomeButtons(); 
      break;       
    case GUIDE_SCREEN:   
      if (guideScreen.guideButStateChange()) 
        guideScreen.updateGuideButtons(); 
      break;        
    case FOCUSER_SCREEN:  
      if (dCfocuserScreen.focuserButStateChange()) 
        dCfocuserScreen.updateFocuserButtons(); 
      break; 
    case GOTO_SCREEN:     
      if (gotoScreen.gotoButStateChange()) 
        gotoScreen.updateGotoButtons(); 
      break;          
    case MORE_SCREEN:     
      if (moreScreen.moreButStateChange()) 
        moreScreen.updateMoreButtons(); 
      break;          
    case SETTINGS_SCREEN:  
      if (settingsScreen.settingsButStateChange()) 
        settingsScreen.updateSettingsButtons(); 
      break;
    case ALIGN_SCREEN:     
      if (moreScreen.moreButStateChange()) 
        alignScreen.updateAlignButtons(); 
      break;     
    case PLANETS_SCREEN:   
      if (planetsScreen.planetsButStateChange()) 
        planetsScreen.updatePlanetsButtons(); 
      break;
    case TREASURE_SCREEN:   
      if (treasureCatScreen.trCatalogButStateChange()) 
        treasureCatScreen.updateTreasureButtons(); 
      break; 
    case CUSTOM_SCREEN:   
      if (customCatScreen.cusCatalogButStateChange()) 
        customCatScreen.updateCustomButtons(); 
      break; 
    case SHC_CAT_SCREEN:   
      if (shcCatScreen.shCatalogButStateChange()) 
        shcCatScreen.updateShcButtons(); 
      break; 
    case XSTATUS_SCREEN:  
      // No buttons here
      break; 

    #ifdef ODRIVE_MOTOR_PRESENT
    case ODRIVE_SCREEN: 
      if (oDriveScreen.odriveButStateChange()) 
        oDriveScreen.updateOdriveButtons(); 
      break;
    #endif
  }
}

// screen selection
void Display::setCurrentScreen(ScreenEnum curScreen) {
currentScreen = curScreen;
};

// select which screen to update at the Update task rate 
void Display::updateSpecificScreen() {
#ifdef ENABLE_TFT_MIRROR
  wifiDisplay.enableScreenCapture(true); 
#endif
  switch (currentScreen) {
    case HOME_SCREEN:       homeScreen.updateHomeStatus();            break;
    case GUIDE_SCREEN:      guideScreen.updateGuideStatus();          break;
    case FOCUSER_SCREEN:    dCfocuserScreen.updateFocuserStatus();    break;
    case GOTO_SCREEN:       gotoScreen.updateGotoStatus();            break;
    case MORE_SCREEN:       moreScreen.updateMoreStatus();            break;
    case SETTINGS_SCREEN:   settingsScreen.updateSettingsStatus();    break;
    case ALIGN_SCREEN:      alignScreen.updateAlignStatus();          break;
    case TREASURE_SCREEN:   treasureCatScreen.updateTreasureStatus(); break;
    case CUSTOM_SCREEN:     customCatScreen.updateCustomStatus();     break;
    case SHC_CAT_SCREEN:    shcCatScreen.updateShcStatus();           break;
    case PLANETS_SCREEN:    planetsScreen.updatePlanetsStatus();      break;
    case XSTATUS_SCREEN:    extStatusScreen.updateExStatus();         break;
    #ifdef ODRIVE_MOTOR_PRESENT
    case ODRIVE_SCREEN:     oDriveScreen.updateOdriveStatus();        break;
    #endif
    default:  break;
  }

  display.refreshButtons();

  // don't do the following updates on these screens
  if (currentScreen == CUSTOM_SCREEN || 
    currentScreen == SHC_CAT_SCREEN ||
    currentScreen == PLANETS_SCREEN ||
    currentScreen == XSTATUS_SCREEN ||
    currentScreen == TREASURE_SCREEN) {
    #ifdef ENABLE_TFT_MIRROR
      wifiDisplay.enableScreenCapture(false);
      wifiDisplay.sendFrameToEsp(FRAME_TYPE_DEF);
    #endif
    
    return;
  }

  display.updateCommonStatus();
  display.showOnStepGenErr(); 
  //display.showOnStepCmdErr();
  display.updateBatVoltage(1);
  
#ifdef ENABLE_TFT_MIRROR
  wifiDisplay.enableScreenCapture(false);
  wifiDisplay.sendFrameToEsp(FRAME_TYPE_DEF);
#endif
}

// ======= Local Command Channel Support ========
// Use for LX200 commands that only expect a bool returned
bool Display::commandBool(char *command) {
  //VL(command);
  char *serStatus = nullptr;
  char resp[80] = "";
  SERIAL_LOCAL.transmit(command);
  tasks.yield(10);
  serStatus = SERIAL_LOCAL.receive();
  strcpy(resp, serStatus);
  int l = strlen(resp) - 1; 
  if (l >= 0 && resp[l] == '#') resp[l] = 0; // removes the #
  //Serial.print(resp);
  return (resp);
}

bool Display::commandBool(const char *command) {
  return commandBool((char *)command);
}

// Local Cmd channel - use when the expected response is not bool
// Instead of using SERIAL_LOCAL this function uses parts of that code for quicker access
// There was "anomalous" behavior and hangs using SERIAL_LOCAL for reading data
void Display::commandWithReply(const char *command, char *reply) {
  char cmdReply[60] = "";
  char parameter[4] = "";
  bool supressFrame = false;
  bool numericReply = true;
  char cmd[5] = "";
  char mutableCommand[6]; 
  strcpy(mutableCommand, command);  // Copy command to a mutable buffer

  // Extract command (drop the ':')
  cmd[0] = mutableCommand[1];  // First command character
  if (mutableCommand[2] == '#') {  
    cmd[1] = '\0';  // 1-character command (e.g., ":Q#")
  } else {
    cmd[1] = mutableCommand[2];  // Second command character
    if (mutableCommand[3] == '#') {
      cmd[2] = '\0';  // 2-character command (e.g., ":GA#")
    } else {
      cmd[2] = '\0';  // Ensure cmd is always null-terminated
      // Extract parameter (if any)
      parameter[0] = mutableCommand[3];
      parameter[1] = mutableCommand[4];
      parameter[2] = '\0';  // Null-terminate parameter
    }
  }

    // V(cmd);
    // V(parameter);
    // V(" ");
  CommandError cmdErr = processor.command(cmdReply, cmd, parameter, &supressFrame, &numericReply);
  
  //VF(cmdErrStr[cmdErr]);
  // Latch new error and start timer
  if (cmdErr != CE_NONE && latchedCmdErr == CE_NONE) {
      latchedCmdErr = cmdErr;
      errorLatchStartTime = millis();
  }

  // Clear after duration
  if (latchedCmdErr != CE_NONE && millis() - errorLatchStartTime >= errorDisplayDuration) {
      latchedCmdErr = cmdErr;
  }
  if (currentScreen != XSTATUS_SCREEN) {
    snprintf(cmdErrGlobal, sizeof(cmdErrGlobal), "Cmd Error: %.88s", cmdErrStr[latchedCmdErr]);
    canvDisplayInsPrint.printLJ(3, 453, 314, C_HEIGHT + 2, cmdErrGlobal, false);
  }

  // handle numeric Replies
  if (numericReply) {
    if (cmdErr != CE_NONE && cmdErr != CE_1) strcpy(reply,"0"); else strcpy(reply,"1");
    supressFrame = true;
  } else {
    strcpy(reply, cmdReply); 
    // get rid of '#'
    if ((strlen(reply)>0) && (reply[strlen(reply)-1]=='#')) reply[strlen(reply)-1]=0;
  }
}

// Draw the Title block
void Display::drawTitle(int text_x, int text_y, const char* label) {
  tft.drawRect(1, 1, 319, 479, butOutline); // draw screen outline
  tft.setFont(&FreeSansBold12pt7b);
  tft.setTextColor(textColor);
  tft.fillRect(TITLE_BOX_X, TITLE_BOX_Y, TITLE_BOXSIZE_X, TITLE_BOXSIZE_Y, titleBackground);
  //tft.drawRect(TITLE_BOX_X, TITLE_BOX_Y, TITLE_BOXSIZE_X, TITLE_BOXSIZE_Y, butOutline);
  tft.setCursor(TITLE_BOX_X + text_x, TITLE_BOX_Y + text_y);
  tft.print(label);
  tft.setFont(&Inconsolata_Bold8pt7b);
}

// Color Themes (Day or Night)
void Display::setNightMode(bool nightMode) {
  _nightMode = nightMode;
  if (!nightMode) {
    // Day Color Theme
    pgBackground = XDARK_MAROON; 
    butBackground = BLACK;
    butOnBackground = MAROON;
    textColor = DIM_YELLOW; 
    butOutline = ORANGE; 
  } else {  
    // Night Color Theme
    pgBackground = BLACK; 
    butBackground = DARK_MAROON;
    butOnBackground = MAROON;
    textColor = ORANGE; 
    butOutline = ORANGE; 
  }
}

bool Display::getNightMode() {
  return _nightMode;
}

// Update Battery Voltage
void Display::updateBatVoltage(int axis) {
  float currentBatVoltage = oDriveExt.getODriveBusVoltage(axis);
  //VF("Bat Voltage:"); SERIAL_DEBUG.print(currentBatVoltage);
    char bvolts[12]="00.0 v";
    sprintf(bvolts, "%4.1f v", currentBatVoltage);
    //if (previousBatVoltage == currentBatVoltage) return;
    if (currentBatVoltage < BATTERY_LOW_VOLTAGE) { 
      tft.fillRect(135, 29, 50, 14, butOnBackground);
    } else {
      tft.fillRect(135, 29, 50, 14, butBackground);
    }
    tft.setFont(&Inconsolata_Bold8pt7b);
    tft.setCursor(135, 40);
    tft.print(bvolts);
  previousBatVoltage = currentBatVoltage;
}

// Define Hidden Motors OFF button
// Hidden button is GPS ICON area and will turn off current to both Motors
// This hidden area is on ALL screens for Saftey in case of mount collision
void Display::motorsOff(uint16_t px, uint16_t py) {
  if (py > ABORT_Y && py < (ABORT_Y + ABORT_BOXSIZE_Y) && px > ABORT_X && px < (ABORT_X + ABORT_BOXSIZE_X)) {
    ALERT;
    soundFreq(1500, 200);
    commandBool(":Q#"); // stops move
    axis1.enable(false); // turn off Motor1
    axis2.enable(false); // turn off Motor2
    commandBool(":Td#"); // Disable Tracking
  }
}

// Show GPS Status ICON
void Display::showGpsStatus() {
  // not on these screens
  if (currentScreen == CUSTOM_SCREEN || 
    currentScreen == SHC_CAT_SCREEN ||
    currentScreen == PLANETS_SCREEN ||
    currentScreen == XSTATUS_SCREEN ||
    currentScreen == TREASURE_SCREEN) return;
  uint8_t extern gps_icon[];
  if (!tls.isReady()) {
    firstGPS = true; // turn on One-shot trigger
    if (!flash) {
      flash = true;
      tft.drawBitmap(278, 3, gps_icon, 37, 37, BLACK, RED);
    } else {
      flash = false;
      tft.drawBitmap(278, 3, gps_icon, 37, 37, BLACK, DIM_YELLOW);
    }

    if (firstRTC) { 
      // use the Teensy RTC until GPS is locked
      unsigned long TeensyTime = Teensy3Clock.get(); // get time from Teensy RTC
      setTime(TeensyTime);    // set system time
      firstRTC = false;
    }                       
  } else { // GPS is ready
    //if (firstGPS) {
      // If the GPS (or other TLS) is locked, then send LST and Latitude to the cat_mgr module
      double f=0;
      char reply[12];

      // Set LST for the cat_mgr 
      display.commandWithReply(":GS#", reply);
      convert.hmsToDouble(&f, reply);
      cat_mgr.setLstT0(f);

      // Set Latitude for cat_mgr
      display.commandWithReply(":Gt#", reply);
      convert.dmsToDouble(&f, reply, true);
      cat_mgr.setLat(f);
    
      // set the RTC in Teensy to the latest GPS reading
      // if (dgps.time.age() < 500) {
      //   setTime(dgps.time.hour(), dgps.time.minute(), dgps.time.second(), dgps.date.day(), dgps.date.month(), dgps.date.year());
      //   char tempReply[4]="0";
      //   //char error[100]="";
      //   commandWithReply(":GG#", tempReply); // get timezone
      //   adjustTime((atoi(tempReply)) * -1* SECS_PER_HOUR);
      //   unsigned long TeensyTime = now();              // get time in epoch
      //   Teensy3Clock.set(TeensyTime);                  // set Teensy time
      // }
    //firstGPS = false;
    tft.drawBitmap(278, 3, gps_icon, 37, 37, BLACK, DIM_YELLOW);
    //}
  }
}

bool Display::getGeneralErrorMessage(char message[], uint8_t error) {
  uint8_t lastError = error;
  enum GeneralErrors: uint8_t {
  ERR_NONE, ERR_MOTOR_FAULT, ERR_ALT_MIN, ERR_LIMIT_SENSE, ERR_DEC, ERR_AZM,
  ERR_UNDER_POLE, ERR_MERIDIAN, ERR_SYNC, ERR_PARK, ERR_GOTO_SYNC, ERR_UNSPECIFIED,
  ERR_ALT_MAX, ERR_WEATHER_INIT, ERR_SITE_INIT, ERR_NV_INIT};
  strcpy(message,"");

  if (lastError == ERR_NONE) strcpy(message, L_GE_NONE); else
  if (lastError == ERR_MOTOR_FAULT) strcpy(message, L_GE_MOTOR_FAULT); else
  if (lastError == ERR_ALT_MIN) strcpy(message, L_GE_ALT_MIN); else
  if (lastError == ERR_LIMIT_SENSE) strcpy(message, L_GE_LIMIT_SENSE); else
  if (lastError == ERR_DEC) strcpy(message, L_GE_DEC); else
  if (lastError == ERR_AZM) strcpy(message, L_GE_AZM); else
  if (lastError == ERR_UNDER_POLE) strcpy(message, L_GE_UNDER_POLE); else
  if (lastError == ERR_MERIDIAN) strcpy(message, L_GE_MERIDIAN); else
  if (lastError == ERR_SYNC) strcpy(message, L_GE_SYNC); else
  if (lastError == ERR_PARK) strcpy(message, L_GE_PARK); else
  if (lastError == ERR_GOTO_SYNC) strcpy(message, L_GE_GOTO_SYNC); else
  if (lastError == ERR_UNSPECIFIED) strcpy(message, L_GE_UNSPECIFIED); else
  if (lastError == ERR_ALT_MAX) strcpy(message, L_GE_ALT_MAX); else
  if (lastError == ERR_WEATHER_INIT) strcpy(message, L_GE_WEATHER_INIT); else
  if (lastError == ERR_SITE_INIT) strcpy(message, L_GE_SITE_INIT); else
  if (lastError == ERR_NV_INIT) strcpy(message, L_GE_NV_INIT); else
  sprintf(message, L_GE_OTHER " %d", (int)lastError);
  return message[0];
}

// ========== OnStep Command Errors =============
void Display::showOnStepCmdErr() {
  //VLF("getting Cmd Err");
  char cmdErr[60] = "";
  char temp[60] = "Command Error: ";
  // not on these screens
  if (currentScreen == CUSTOM_SCREEN || 
    currentScreen == SHC_CAT_SCREEN ||
    currentScreen == PLANETS_SCREEN ||
    currentScreen == XSTATUS_SCREEN ||
    currentScreen == TREASURE_SCREEN) return;

  commandWithReply(":GE#", cmdErr);
  strcat(temp, cmdErrStr[atoi(cmdErr)]);
  canvDisplayInsPrint.printLJ(3, 453, 314, C_HEIGHT+2, temp, false);
}

// ========== OnStep General Errors =============
void Display::showOnStepGenErr() {
  // not on these screens
  if (currentScreen == CUSTOM_SCREEN || 
    currentScreen == SHC_CAT_SCREEN ||
    currentScreen == PLANETS_SCREEN ||
    currentScreen == XSTATUS_SCREEN ||
    currentScreen == TREASURE_SCREEN) return;

  char temp[80] = "";
  char temp1[80] = "General Error: ";
  
  //commandWithReply(":GU#", genErr);
  //error = (genErr[strlen(genErr) - 1] - '0');
  getGeneralErrorMessage(temp, limits.errorCode());
  strcat(temp1, temp);
  canvDisplayInsPrint.printLJ(3, 470, 314, C_HEIGHT+2, temp1, false);
}

// Draw the Menu buttons
void Display::drawMenuButtons() {
  int y_offset = 0;
  int x_offset = 0;

  tft.setTextColor(textColor);
  tft.setFont(&UbuntuMono_Bold11pt7b); 
  
  // *************** MENU MAP ****************
  // Current Screen   |Cur |Col1|Col2|Col3|Col4|
  // Home-----------| Ho | Gu | Fo | GT | Mo |
  // Guide----------| Gu | Ho | Fo | Al | Mo |
  // Focuser--------| Fo | Ho | Gu | GT | Mo |
  // GoTo-----------| GT | Ho | Fo | Gu | Mo |

  // if ODRIVE_PRESENT then use this menu structure
  //  More & (CATs)--| Mo | GT | Se | Od | Al |
  //  ODrive---------| Od | Ho | Se | Al | Xs |
  //  Extended Status| Xs | Ho | Se | Al | Od |
  //  Settings-------| Se | Ho | Xs | Al | Od |
  //  Alignment------| Al | Ho | Fo | Gu | Od |
  // else if not ODRIVE_PRESENT use this menu structure
  //  More & (CATs)--| Mo | GT | Se | Gu | Al |
  //  Extended Status| Xs | Ho | Se | Al | Mo |
  //  Settings-------| Se | Ho | Xs | Al | Mo |
  //  Alignment------| Al | Ho | Fo | Gu | Mo |

  switch(Display::currentScreen) {
    case HOME_SCREEN: 
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GUIDE", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "FOCUS", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GO TO", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "CTLGS", BUT_OFF);
      break;

   case GUIDE_SCREEN:
      x_offset = 0;
      y_offset = 0;
       menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "HOME", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "FOCUS", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ALIGN", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "CATLGS", BUT_OFF);
      break;

   case FOCUSER_SCREEN:
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "HOME", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GUIDE", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GO TO", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "CATLGS", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      break;
    
   case GOTO_SCREEN:
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "HOME", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "FOCUS", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GUIDE", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "CATLGS", BUT_OFF);
      break;
      
   case MORE_SCREEN:
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GO TO", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "SETng", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;

      #ifdef ODRIVE_MOTOR_PRESENT
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ODRIV", BUT_OFF);
      #elif
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GUIDE", BUT_OFF);
      #endif

      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ALIGN", BUT_OFF);
      break;

    #ifdef ODRIVE_MOTOR_PRESENT
    case ODRIVE_SCREEN: 
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "HOME", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "SETng", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ALIGN", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "xSTAT", BUT_OFF);
      break;
    #endif
    
    case SETTINGS_SCREEN: 
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "HOME", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "XSTAT", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ALIGN", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
       
      #ifdef ODRIVE_MOTOR_PRESENT
        menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ODRIV", BUT_OFF);
      #elif
        menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "MORE..", BUT_OFF);
      #endif  
      break;

      case XSTATUS_SCREEN: 
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "HOME", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "SETng", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ALIGN", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
       
      #ifdef ODRIVE_MOTOR_PRESENT
        menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ODRIV", BUT_OFF);
      #elif
        menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "CATLGS", BUT_OFF);
      #endif  
      break;

    case ALIGN_SCREEN: 
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "HOME", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "FOCUS", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GUIDE", BUT_OFF);
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      #ifdef ODRIVE_MOTOR_PRESENT
        menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "ODRIV", BUT_OFF);
      #elif
        menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "CATLGS", BUT_OFF);
      #endif  
      break;

   default: // HOME Screen
      x_offset = 0;
      y_offset = 0;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GUIDE", BUT_OFF);
       
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "FOCUS", BUT_OFF);
    
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "GO TO", BUT_OFF);
     
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      menuButton.draw(MENU_X + x_offset, MENU_Y + y_offset, "CATLGS", BUT_OFF);
     
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      break;
  }  
  tft.setFont(&Inconsolata_Bold8pt7b);
}

// ==============================================
// ====== Draw multi-screen status labels =========
// ==============================================
// These particular status labels are placed near the top of most Screens.
void Display::drawCommonStatusLabels() {
  tft.setFont(&Inconsolata_Bold8pt7b);
  int y_offset = 0;

  // Column 1
  // Display RA Current
  tft.setCursor(COM_COL1_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("RA-----:");

  // Display RA Target
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL1_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("RA tgt-:");

  // Display DEC Current
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL1_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("DEC----:");

  // Display DEC Target
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL1_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("DEC tgt:");

  // Column 2
  // Display Current Azimuth
  y_offset =0;
  tft.setCursor(COM_COL2_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("AZ-----:");

  // Display Target Azimuth
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL2_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("AZ  tgt:");
  
  // Display Current ALT
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL2_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("ALT----:"); 

  // Display Target ALT
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL2_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("ALT tgt:"); 
  
  tft.drawFastHLine(1, COM_COL1_LABELS_Y+y_offset+6, TFTWIDTH-1, textColor);
  tft.drawFastVLine(TFTWIDTH/2, 96, 60, textColor);
}

// UpdateCommon Status - Real time data update for the particular labels printed above
// This Common Status is found at the top of most pages.
void Display::updateCommonStatus() { 
  //VLF("updating common status");
  showGpsStatus();

  // // If the GPS (or other TLS) is locked, then send LST and Latitude to the cat_mgr module
  // if (tls.isReady() && firstGPS) {
  //   double f=0;
  //   char reply[12];

  //   // Set LST for the cat_mgr 
  //   display.commandWithReply(":GS#", reply);
  //   convert.hmsToDouble(&f, reply);
  //   cat_mgr.setLstT0(f);

  //   // Set Latitude for cat_mgr
  //   display.commandWithReply(":Gt#", reply);
  //   convert.dmsToDouble(&f, reply, true);
  //   cat_mgr.setLat(f);
  //   firstGPS = false;
  // }

  // Flash tracking LED if mount is tracking
  if (mount.isTracking()) {
    if (trackLedOn) {
      digitalWrite(STATUS_TRACK_LED_PIN, HIGH); // LED OFF, active low
      tft.setFont(&Inconsolata_Bold8pt7b);
      tft.fillRect(50, 28, 72, 14, BLACK); 
      tft.setCursor(50, 38);
      tft.print("        ");
      trackLedOn = false;
    } else {
      digitalWrite(STATUS_TRACK_LED_PIN, LOW); // LED ON
      tft.setFont(&Inconsolata_Bold8pt7b);
      tft.setCursor(50, 38);
      tft.print("Tracking");
      trackLedOn = true;
    }
  #ifdef ODRIVE_MOTOR_PRESENT
  // frequency varying alarm if Motor and Encoders positions are too far apart indicating unbalanced loading or hitting obstruction
    oDriveExt.MotorEncoderDelta();
  #endif
  } else { // not tracking 
    digitalWrite(STATUS_TRACK_LED_PIN, HIGH); // LED OFF
    tft.setFont(&Inconsolata_Bold8pt7b);
    tft.fillRect(50, 28, 72, 14, BLACK); 
    tft.setCursor(50, 38);
    tft.print("        ");
    trackLedOn = false;
  }

  if (currentScreen == CUSTOM_SCREEN || 
      currentScreen == SHC_CAT_SCREEN ||
      currentScreen == PLANETS_SCREEN ||
      currentScreen == TREASURE_SCREEN) return;

  char ra_hms[10]   = ""; 
  char dec_dms[11]  = "";
  char tra_hms[10]  = "";
  char tdec_dms[11] = "";
  
  int y_offset = 0;
  // ----- Column 1 -----
  // Current RA, Returns: HH:MM.T# or HH:MM:SS# (based on precision setting)
  commandWithReply(":GR#", ra_hms);
  canvDisplayInsPrint.printRJ(COM_COL1_DATA_X, COM_COL1_DATA_Y, C_WIDTH, C_HEIGHT, ra_hms, false);

  // Target RA, Returns: HH:MM.T# or HH:MM:SS (based on precision setting)
  y_offset +=COM_LABEL_Y_SPACE; 
  commandWithReply(":Gr#", tra_hms);
  canvDisplayInsPrint.printRJ(COM_COL1_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH, C_HEIGHT, tra_hms, false);

  // Current DEC
   y_offset +=COM_LABEL_Y_SPACE; 
  commandWithReply(":GD#", dec_dms);
  canvDisplayInsPrint.printRJ(COM_COL1_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH, C_HEIGHT, dec_dms, false);

  // Target DEC
  y_offset +=COM_LABEL_Y_SPACE;  
  commandWithReply(":Gd#", tdec_dms); 
  canvDisplayInsPrint.printRJ(COM_COL1_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH, C_HEIGHT, tdec_dms, false);
  //VLF("common column 1 check point complete");
  // ----- Column 2 -----
  y_offset =0;

  Coordinate dispTarget = goTo.getGotoTarget();
  transform.rightAscensionToHourAngle(&dispTarget);
  transform.equToHor(&dispTarget);

  // Get CURRENT AZM
  //commandWithReply(":GZ#", cAzmDMS); // DDD*MM'SS# 
  //convert.dmsToDouble(&cAzm_d, cAzmDMS, false, PM_LOW);
  double temp = NormalizeAzimuth(radToDeg(mount.getPosition(CR_MOUNT_HOR).z));
  canvDisplayInsPrint.printRJ(COM_COL2_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH-20, C_HEIGHT, temp, false);

  // Get TARGET AZM
  y_offset +=COM_LABEL_Y_SPACE;  
  //commandWithReply(":Gz#", tAzmDMS); // DDD*MM'SS# 
  //convert.dmsToDouble(&tAzm_d, tAzmDMS, false, PM_LOW);
  temp = NormalizeAzimuth(radToDeg(dispTarget.z));
  canvDisplayInsPrint.printRJ(COM_COL2_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH-20, C_HEIGHT, temp, false);

  // Get CURRENT ALT
  y_offset +=COM_LABEL_Y_SPACE;  
  //commandWithReply(":GA#", cAltDMS);	// sDD*MM'SS#
  //convert.dmsToDouble(&cAlt_d, cAltDMS, true, PM_LOW);
  temp = radToDeg(mount.getPosition(CR_MOUNT_ALT).a);
  canvDisplayInsPrint.printRJ(COM_COL2_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH-20, C_HEIGHT, temp, false);
  
  // Get TARGET ALT
  y_offset +=COM_LABEL_Y_SPACE;  
  //commandWithReply(":Gal#", tAltDMS);	// sDD*MM'SS#
  //convert.dmsToDouble(&tAlt_d, tAltDMS, true, PM_LOW);
  canvDisplayInsPrint.printRJ(COM_COL2_DATA_X, COM_COL1_DATA_Y+y_offset, C_WIDTH-20, C_HEIGHT, radToDeg(dispTarget.a), false);
  //VLF("column 2 complete");
}

// draw a picture -This member function is a copy from rDUINOScope but with 
//    pushColors() changed to drawPixel() with a loop
// rDUINOScope - Arduino based telescope control system (GOTO).
//    Copyright (C) 2016 Dessislav Gouzgounov (Desso)
//    PROJECT Website: http://rduinoscope.byethost24.com
void Display::drawPic(File *StarMaps, uint16_t x, uint16_t y, uint16_t WW, uint16_t HH){
  uint8_t header[14 + 124]; // maximum length of bmp file header
  uint16_t color[320];  
  uint16_t num;   
  uint8_t color_l, color_h;
  uint32_t i,j,k;
  uint32_t width;
  uint32_t height;
  uint16_t bits;
  uint32_t compression;
  uint32_t alpha_mask = 0;
  uint32_t pic_offset;
  char temp[20]="";

  /** read header of the bmp file */
  i=0;
  while (StarMaps->available()) {
    header[i] = StarMaps->read();
    i++;
    if(i==14){
      break;
    }
  }

  pic_offset = (((uint32_t)header[0x0A+3])<<24) + (((uint32_t)header[0x0A+2])<<16) + (((uint32_t)header[0x0A+1])<<8)+(uint32_t)header[0x0A];
  while (StarMaps->available()) {
    header[i] = StarMaps->read();
    i++;
    if(i==pic_offset){
      break;
    }
  }
 
  /** calculate picture width ,length and bit numbers of color */
  width = (((uint32_t)header[0x12+3])<<24) + (((uint32_t)header[0x12+2])<<16) + (((uint32_t)header[0x12+1])<<8)+(uint32_t)header[0x12];
  height = (((uint32_t)header[0x16+3])<<24) + (((uint32_t)header[0x16+2])<<16) + (((uint32_t)header[0x16+1])<<8)+(uint32_t)header[0x16];
  compression = (((uint32_t)header[0x1E + 3])<<24) + (((uint32_t)header[0x1E + 2])<<16) + (((uint32_t)header[0x1E + 1])<<8)+(uint32_t)header[0x1E];
  bits = (((uint16_t)header[0x1C+1])<<8) + (uint16_t)header[0x1C];
  if(pic_offset>0x42){
    alpha_mask = (((uint32_t)header[0x42 + 3])<<24) + (((uint32_t)header[0x42 + 2])<<16) + (((uint32_t)header[0x42 + 1])<<8)+(uint32_t)header[0x42];
  }
  sprintf(temp, "%lu", pic_offset);  //VF("pic_offset=");  VL(temp);
  sprintf(temp, "%lu", width);       //VF("width=");       VL(temp);
  sprintf(temp, "%lu", height);      //VF("height=");      VL(temp);
  sprintf(temp, "%lu", compression); //VF("compression="); VL(temp);
  sprintf(temp, "%d",  bits);        //VF("bits=");        VL(temp);
  sprintf(temp, "%lu", alpha_mask);  //VF("alpha_mask=");  VL(temp);

  /** set position to pixel table */
  StarMaps->seek(pic_offset);
  /** check picture format */
  if(pic_offset == 138 && alpha_mask == 0){
    /** 565 format */
    tft.setRotation(0);
    /** read from SD card, write to TFT LCD */
    for(j=0; j<HH; j++){ // read all lines
      for(k=0; k<WW; k++){ // read two bytes and pack them in int16, continue for a row
          color_l = StarMaps->read();
          color_h = StarMaps->read();
          color[k]=0;
          color[k] += color_h;
          color[k] <<= 8;
          color[k] += color_l;
      }
      num = 0;
    
      while (num < x + width - 1){  //implementation for DDScope
      //if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;
        //setAddrWindow(x, y, x + 1, y + 1);
        //pushColor(uint16_t color)

        //while (num < x+width-1){
        tft.drawPixel(x+num, y+j, color[num]); //implementation for DDScope
        num++;
      }
      // dummy read twice to align for 4 
      if(width%2){
        StarMaps->read();StarMaps->read();
      }
    }
  }
}

Display display;