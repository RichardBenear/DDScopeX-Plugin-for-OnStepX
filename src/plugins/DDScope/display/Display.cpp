// ==============================================
// ============ Display Support =================
// ==============================================
// Common Display functions
// 3.5" RPi Touchscreen and Display
// SPI Interface
// Author: Richard Benear 3/30/2021
// **************************************************

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <gfxfont.h>

#include "../../../telescope/mount/Mount.h"
//#include "src/lib/commands/CommandErrors.h"
#include "../../../lib/tasks/OnTask.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "../fonts/UbuntuMono_Bold8pt7b.h"
#include "../fonts/UbuntuMono_Bold11pt7b.h"
#include <Fonts/FreeSansBold12pt7b.h>

// DDScope specific
#include "../DDScope.h"
#include "Display.h"
#include "../catalog/Catalog.h"
#include "../screens/AlignScreen.h"
#include "../screens/CatalogScreen.h"
#include "../screens/DCFocuserScreen.h"
#include "../screens/GotoScreen.h"
#include "../screens/GuideScreen.h"
#include "../screens/HomeScreen.h"
#include "../screens/MoreScreen.h"
#include "../screens/PlanetsScreen.h"
#include "../screens/SettingsScreen.h"
#include "../screens/ExtStatusScreen.h"

#ifdef ODRIVE_MOTOR_PRESENT
  #include "../odriveExt/ODriveExt.h"
  #include "../screens/ODriveScreen.h"
#endif

#define TITLE_BOXSIZE_X         313
#define TITLE_BOXSIZE_Y          43
#define TITLE_BOX_X               3
#define TITLE_BOX_Y               1 

// Shared common Status 
#define COM_LABEL_Y_SPACE        17
#define COM_COL1_LABELS_X         8
#define COM_COL1_LABELS_Y       104
#define COM_COL1_DATA_X          72
#define COM_COL1_DATA_Y          COM_COL1_LABELS_Y
#define COM_COL2_LABELS_X       179
#define COM_COL2_DATA_X         250

Screen Display::currentScreen = HOME_SCREEN;
bool Display::_nightMode = false;
Adafruit_ILI9486_Teensy tft;

// =========================================
// ========= Initialize Display ============
// =========================================
void Display::init() {
  VLF("MSG: Display, started"); 
  tft.begin(); delay(1);
  tft.setRotation(0); // display rotation: Note it is different than touchscreen
  setNightMode(getNightMode()); 
  
  sdInit(); // initialize the SD card and draw start screen
  delay(1500); // let start screen show for 1.5 sec

  // set some defaults
  VLF("MSG: Setting up Limits, TZ, Site Name, Slew Speed");
  setLocalCmd(":SG+07:00#"); // Set Default Time Zone
  setLocalCmd(":Sh-01#"); //Set horizon limit -1 deg
  setLocalCmd(":So86#"); // Set overhead limit 86 deg
  setLocalCmd(":SMHome#"); // Set Site 0 name "Home"
  setLocalCmd(":SX93,1#"); // 2x slew speed
  //setLocalCmd(":SX93,2#"); // 1.5x slew speed
  //setLocalCmd(":SX93,3#"); // 1.0x slew speed
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

  tft.fillScreen(BLACK); 
  tft.setTextColor(YELLOW);
  drawPic(&StarMaps, 1, 0, 320, 480);  
  drawTitle(20, 30, "DIRECT-DRIVE SCOPE");
  tft.setCursor(60, 80);
  tft.setTextSize(2);
  tft.printf("Initializing");
  tft.setTextSize(1);
  tft.setCursor(120, 120);
  tft.printf("NGC 1566");
}

// screen selection
void Display::setCurrentScreen(Screen curScreen) {
currentScreen = curScreen;
};

// select which screen to update
void Display::updateSpecificScreen() {
  switch (currentScreen) {
    case HOME_SCREEN:       homeScreen.updateHomeStatus();         break;
    case GUIDE_SCREEN:      guideScreen.updateGuideStatus();       break;
    case FOCUSER_SCREEN:    dCfocuserScreen.updateFocuserStatus(); break;
    case GOTO_SCREEN:       gotoScreen.updateGotoStatus();         break;
    case MORE_SCREEN:       moreScreen.updateMoreStatus();         break;
    case SETTINGS_SCREEN:   settingsScreen.updateSettingsStatus(); break;
    case ALIGN_SCREEN:      alignScreen.updateAlignStatus();       break;
    case CATALOG_SCREEN:    catalogScreen.updateCatalogStatus();   break;
    case PLANETS_SCREEN:    planetsScreen.updatePlanetsStatus();   break;
  //case XSTATUS_SCREEN:                                           break;
    #ifdef ODRIVE_MOTOR_PRESENT
      case ODRIVE_SCREEN:   oDriveScreen.updateOdriveStatus();    break;
    #endif
    default: break;
  }
}

// ======= Use Local Command Channel ========
void Display::setLocalCmd(char *command) {
  SERIAL_LOCAL.transmit(command);
}

void Display::setLocalCmd(const char *command) {
  setLocalCmd((char *)command);
}

void Display::getLocalCmd(const char *command, char *reply) {
  SERIAL_LOCAL.transmit(command);
  tasks.yield(50);
  strcpy(reply, SERIAL_LOCAL.receive()); 
}

void Display::getLocalCmdTrim(const char *command, char *reply) {
  SERIAL_LOCAL.transmit(command); 
  tasks.yield(50);
  strcpy(reply, SERIAL_LOCAL.receive()); 
  if ((strlen(reply)>0) && (reply[strlen(reply)-1]=='#')) reply[strlen(reply)-1]=0;
}

// Draw a single button
void Display::drawButton(int x_start, int y_start, int w, int h, bool butOn, int text_x_offset, int text_y_offset, const char* label) {
  int buttonRadius = 7;
  if (butOn) {
    tft.fillRoundRect(x_start, y_start, w, h, buttonRadius, butOnBackground);
  } else {
    tft.fillRoundRect(x_start, y_start, w, h, buttonRadius, butBackground);
  }
  tft.drawRoundRect(x_start, y_start, w, h, buttonRadius, butOutline);
  tft.setCursor(x_start + text_x_offset, y_start + text_y_offset);
  tft.print(label);
}

// Draw the Title block
void Display::drawTitle(int text_x_offset, int text_y_offset, const char* label) {
  tft.setFont(&FreeSansBold12pt7b);
  tft.fillRect(TITLE_BOX_X, TITLE_BOX_Y, TITLE_BOXSIZE_X, TITLE_BOXSIZE_Y, butBackground);
  tft.drawRect(TITLE_BOX_X, TITLE_BOX_Y, TITLE_BOXSIZE_X, TITLE_BOXSIZE_Y, butOutline);
  tft.setCursor(TITLE_BOX_X + text_x_offset, TITLE_BOX_Y + text_y_offset);
  tft.print(label);
  tft.setFont(&Inconsolata_Bold8pt7b);
}

//---------------------------------------------------------
// Update a Data Field text using bitmap canvas
// const char* label 
void Display::canvPrint(int x, int y, int y_off, int width, int height, const char* label, uint16_t textColor, uint16_t butBackground) {
  char rjlabel[60];
  int y_box_offset = 10;
  GFXcanvas1 canvas(width, height);

  canvas.setFont(&Inconsolata_Bold8pt7b);  
  canvas.setCursor(0, (height-y_box_offset)/2 + y_box_offset); // offset from top left corner of canvas box
  sprintf(rjlabel, "%9s", label);
  canvas.print(rjlabel);
  tft.drawBitmap(x, y - y_box_offset + y_off, canvas.getBuffer(), width, height, textColor, butBackground);
}

// type const char* overload without colors
void Display::canvPrint(int x, int y, int y_off, int width, int height, const char* label) {
  canvPrint(x, y, y_off, width, height, label, textColor, butBackground);
}

void Display::canvPrint(int x, int y, int y_off, int width, int height, double label, uint16_t textColor, uint16_t butBackground) {
  char charlabel[60];
  int y_box_offset = 10;
  GFXcanvas1 canvas(width, height);

  canvas.setFont(&Inconsolata_Bold8pt7b);  
  canvas.setCursor(0, (height-y_box_offset)/2 + y_box_offset); // offset from top left corner of canvas box
  sprintf(charlabel, "%6.1f", label);
  canvas.print(charlabel);
  tft.drawBitmap(x, y - y_box_offset + y_off, canvas.getBuffer(), width, height, textColor, butBackground);
}

// type double label, overload without colors
void Display::canvPrint(int x, int y, int y_off, int width, int height, double label) {
  canvPrint(x, y, y_off, width, height, label, textColor, butBackground);
}

// type int label overload
void Display::canvPrint(int x, int y, int y_off, int width, int height, int label) {
  char rjlabel[50];
  int y_box_offset = 10;
  int x_fontSize = 10;
  GFXcanvas1 canvas(width, height);

  canvas.setFont(&Inconsolata_Bold8pt7b);
  canvas.setCursor(0, (height-y_box_offset)/2 + y_box_offset); // offset from top left corner of canvas box
  sprintf(rjlabel, "%*d", width/x_fontSize, label); // right justify text in the bitmap
  canvas.print(rjlabel);
  tft.drawBitmap(x, y - y_box_offset + y_off, canvas.getBuffer(), width, height, textColor, butBackground);
}
//------------------------------------------------------

// Color Themes (Day and Night)
void Display::setNightMode(bool nightMode) {
  _nightMode = nightMode;
  if (!_nightMode) {
    // Day Color Theme
    pgBackground = DEEP_MAROON; 
    butBackground = BLACK;
    butOnBackground = RED;
    textColor = YELLOW; 
    butOutline = YELLOW; 
  } else {  
    // Night Color Theme
    pgBackground = BLACK; 
    butBackground = DEEP_MAROON;
    butOnBackground = BLACK;
    textColor = ORANGE; 
    butOutline = ORANGE; 
  }
}

bool Display::getNightMode() {
  return _nightMode;
}

// ============ OnStep Command Errors ===============
void Display::updateOnStepCmdStatus() {
  if (Display::currentScreen == CATALOG_SCREEN || 
    Display::currentScreen == PLANETS_SCREEN ||
    Display::currentScreen == CUST_CAT_SCREEN) return;
  char cmd[40];
  //NEEDS WORK: sprintf(cmd, "OnStep Err: %s", commandErrorStr[commandError]);
  if (!tls.isReady()) {
    canvPrint(2, 454, 0, 319, C_HEIGHT, " Time and/or Date Not Set");
  } else {
    canvPrint(2, 454, 0, 319, C_HEIGHT, cmd);
  }
}

// ODrive AZ and ALT CONTROLLER (only) Error Status
void Display::updateODriveErrBar() {
if (Display::currentScreen == CATALOG_SCREEN || 
    Display::currentScreen == PLANETS_SCREEN ||
    Display::currentScreen == CUST_CAT_SCREEN) return;
  int x = 2;
  int y = 473;
  int label_x = 160;
  int data_x = 110;

  tft.setCursor(x, y);
  tft.print("AZ Ctrl err:");
  tft.setCursor(label_x, y);
  tft.print("ALT Ctrl err:");
  
  canvPrint(        data_x, y, 0, C_WIDTH-40, C_HEIGHT, (int)oDriveExt.getODriveErrors(AZM_MOTOR, AXIS));
  canvPrint(label_x+data_x, y, 0, C_WIDTH-40, C_HEIGHT, (int)oDriveExt.getODriveErrors(ALT_MOTOR, AXIS));

  // sound varying frequency alarm if Motor and Encoders positions are too far apart
  oDriveExt.MotorEncoderDelta();
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
  //  Settings-------| Se | Ho | Fo | Al | Od |
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
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "GUIDE");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "FOCUS");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "GO TO");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET-4, MENU_TEXT_Y_OFFSET, ".MORE.");
      break;

   case GUIDE_SCREEN:
      x_offset = 0;
      y_offset = 0;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET+3, MENU_TEXT_Y_OFFSET, "HOME");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "FOCUS");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "ALIGN");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET-4, MENU_TEXT_Y_OFFSET, ".MORE.");
      break;

   case FOCUSER_SCREEN:
      x_offset = 0;
      y_offset = 0;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET+3, MENU_TEXT_Y_OFFSET, "HOME");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "GUIDE");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "GO TO");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET-4, MENU_TEXT_Y_OFFSET, ".MORE.");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      break;
    
   case GOTO_SCREEN:
      x_offset = 0;
      y_offset = 0;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET+3, MENU_TEXT_Y_OFFSET, "HOME");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "FOCUS");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "GUIDE");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET-4, MENU_TEXT_Y_OFFSET, ".MORE.");
      break;
      
   case MORE_SCREEN:
      x_offset = 0;
      y_offset = 0;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "GO TO");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "SETng");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;

      #ifdef ODRIVE_MOTOR_PRESENT
        drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "ODRIV");
      #elif
        drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "GUIDE");
      #endif

      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "ALIGN");
      break;

    #ifdef ODRIVE_MOTOR_PRESENT
    case ODRIVE_SCREEN: 
      x_offset = 0;
      y_offset = 0;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET+3, MENU_TEXT_Y_OFFSET, "HOME");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "SETng");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "ALIGN");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "xSTAT");
      break;
    #endif
    
    case SETTINGS_SCREEN: 
      x_offset = 0;
      y_offset = 0;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET+3, MENU_TEXT_Y_OFFSET, "HOME");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "xSTAT");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "ALIGN");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      #ifdef ODRIVE_MOTOR_PRESENT
        drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "ODRIV");
      #elif
        drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET-4, MENU_TEXT_Y_OFFSET, ".MORE.");
      #endif  
      break;

    case ALIGN_SCREEN: 
      x_offset = 0;
      y_offset = 0;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET+3, MENU_TEXT_Y_OFFSET, "HOME");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "FOCUS");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "GUIDE");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      #ifdef ODRIVE_MOTOR_PRESENT
        drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "ODRIV");
      #elif
        drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET-4, MENU_TEXT_Y_OFFSET, ".MORE.");
      #endif  
      break;

   default: // HOME Screen
      x_offset = 0;
      y_offset = 0;
       drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "GUIDE");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET,  "FOCUS");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET,  "GO TO");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUTTON_OFF, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, ".MORE.");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      break;
  }  
  tft.setFont(&Inconsolata_Bold8pt7b);
}

// ==============================================
// ====== Draw multi-page status labels =========
// ==============================================
// These particular status labels are placed near the top of most Screens.
void Display::drawCommonStatusLabels() {
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
  
  tft.drawFastHLine(1, COM_COL1_LABELS_Y+y_offset+6, TFTWIDTH-1,textColor);
}

// Update Battery Voltage
void Display::updateBatVoltage() {
  float currentBatVoltage = oDriveExt.getODriveBusVoltage();
  if (oDriveExt.getODriveBusVoltage() < BATTERY_LOW_VOLTAGE) {
    canvPrint(130, 39, 0, 50, 12, currentBatVoltage, textColor, butOnBackground);
  } else {
    canvPrint(130, 39, 0, 50, 12, currentBatVoltage, textColor, butBackground);
  }
  //tft.setTextColor(textColor); // transparent background
}

// UpdateCommon Status - Real time data update for the particular labels printed above
// This Common Status is found at the top of most pages.
void Display::updateCommonStatus() { 
  if (Display::currentScreen == CATALOG_SCREEN || 
    Display::currentScreen == PLANETS_SCREEN ||
    Display::currentScreen == CUST_CAT_SCREEN) return;

  char ra_hms[10]   = ""; 
  char dec_dms[11]  = "";
  char tra_hms[10]  = "";
  char tdec_dms[11] = "";
  char cAzmDMS[10]  = "";
  char cAltDMS[11]  = "";
  char tAzmDMS[10]  = "";
  char tAltDMS[11]  = "";
  double cAzm_d = 0.0;
  double cAlt_d = 0.0;
  double tAzm_d = 0.0;
  double tAlt_d = 0.0;

  int y_offset = 0;
  // ----- Column 1 -----
  // Current RA, Returns: HH:MM.T# or HH:MM:SS# (based on precision setting)
  getLocalCmdTrim(":GR#", ra_hms);
  canvPrint(COM_COL1_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH, C_HEIGHT, ra_hms);

// Target RA, Returns: HH:MM.T# or HH:MM:SS (based on precision setting)
  y_offset +=COM_LABEL_Y_SPACE; 
  getLocalCmdTrim(":Gr#", tra_hms);
  canvPrint(COM_COL1_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH, C_HEIGHT, tra_hms);

// Current DEC
   y_offset +=COM_LABEL_Y_SPACE; 
  getLocalCmdTrim(":GD#", dec_dms);
  canvPrint(COM_COL1_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH, C_HEIGHT, dec_dms);

  // Target DEC
  y_offset +=COM_LABEL_Y_SPACE;  
  getLocalCmdTrim(":Gd#", tdec_dms); 
  canvPrint(COM_COL1_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH, C_HEIGHT, tdec_dms);
  
  // ----- Column 2 -----
  y_offset =0;

  // Get Current ALT and AZ and display them as Double
  getLocalCmdTrim(":GZ#", cAzmDMS); // DDD*MM'SS# 
  convert.dmsToDouble(&cAzm_d, cAzmDMS, false, PM_HIGH);
  canvPrint(COM_COL2_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH-15, C_HEIGHT, cAzm_d);

  y_offset +=COM_LABEL_Y_SPACE;  
  getLocalCmdTrim(":GA#", cAltDMS);	// sDD*MM'SS#
  convert.dmsToDouble(&cAlt_d, cAltDMS, true, PM_HIGH);
  canvPrint(COM_COL2_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH-15, C_HEIGHT, tAzm_d);

  // Get Target ALT and AZ and display them as Double
  y_offset +=COM_LABEL_Y_SPACE;  
  getLocalCmdTrim(":Gz#", tAzmDMS); // DDD*MM'SS# 
  convert.dmsToDouble(&tAzm_d, tAzmDMS, false, PM_HIGH);
  canvPrint(COM_COL2_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH-15, C_HEIGHT, cAlt_d);

  y_offset +=COM_LABEL_Y_SPACE;  
  getLocalCmdTrim(":Gal#", tAltDMS);	// sDD*MM'SS#
  convert.dmsToDouble(&tAlt_d, tAltDMS, true, PM_HIGH);
  canvPrint(COM_COL2_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH-15, C_HEIGHT, tAlt_d);
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

//=============== Mount Status ===============
// Use local command channel to get mount status
double Display::getLstT0() {
  double f=0;
  char reply[12];
  display.getLocalCmdTrim(":GS#", reply);
  convert.hmsToDouble(&f, reply, PM_HIGH);
  return f;
}

// modified this for xChan format from SHC
double Display::getLat() {
  char reply[12];
  display.getLocalCmdTrim(":Gt#", reply);
  double f=-10000;
  convert.dmsToDouble(&f, reply, true, PM_HIGH);
  return f;
}

bool Display::isHome() {
  char reply[12];
  display.getLocalCmdTrim(":GU#", reply); 
  if (strstr(reply,"H")) return true; else return false;
}

bool Display::isParked() {
  char reply[12];
  display.getLocalCmdTrim(":GU#", reply); 
  if (strstr(reply,"P")) return true; else return false;
}

bool Display::isTracking() {
  char reply[12];
  display.getLocalCmdTrim(":GU#", reply);
  if (strstr(reply,"n")) return false; else return true;
}

Display display;