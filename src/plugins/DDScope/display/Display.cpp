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
#include "src/lib/commands/CommandErrors.h"
#include "src/libApp/commands/ProcessCmds.h"
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
#define TITLE_BOXSIZE_Y          40
#define TITLE_BOX_X               3
#define TITLE_BOX_Y               1 

// Shared common Status 
#define COM_LABEL_Y_SPACE        16
#define COM_COL1_LABELS_X         8
#define COM_COL1_LABELS_Y        98
#define COM_COL1_DATA_X          72
#define COM_COL1_DATA_Y          COM_COL1_LABELS_Y
#define COM_COL2_LABELS_X       179
#define COM_COL2_DATA_X         245

void updateScreenWrapper() { display.updateSpecificScreen(); }
void updateCommonWrapper() { display.updateCommonStatus(); }
void updateOnStepCmdWrapper() { display.updateOnStepCmdStatus(); }

#ifdef ODRIVE_MOTOR_PRESENT
  void updateODriveErrWrapper() { display.updateODriveErrBar(); }
  void updateBatVoltWrapper() { display.updateBatVoltage(); }
#endif

Adafruit_ILI9486_Teensy tft;

// =========================================
// ========= Initialize Display ============
// =========================================
void Display::init() {
  
  SerialLocal serialLocal; 

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

  VLF("MSG: Display, started"); 
  tft.begin(); delay(1);
  tft.setRotation(0); // display rotation: Note it is different than touchscreen
  
  sdInit(); // initialize the SD card and draw start screen
  delay(1500); // let start screen show for 1.5 sec

  // start common-most-screens status
  VF("MSG: Setup, start Common screen status polling task (rate 900 ms priority 7)... ");
  uint8_t CShandle = tasks.add(900, 0, true, 7, updateCommonWrapper, "UpdateCommonScreen");
  if (CShandle)  { VLF("success"); } else { VLF("FAILED!"); }

  // update the OnStep Cmd Error status 
  VF("MSG: Setup, start OnStep CMD status polling task (rate 1000 ms priority 7)... ");
  uint8_t CDhandle = tasks.add(1000, 0, true, 7, updateOnStepCmdWrapper, "UpdateOnStepCmdScreen");
  if (CDhandle) { VLF("success"); } else { VLF("FAILED!"); }

  // update this specific screen status
  VF("MSG: Setup, start Screen-specific status polling task (rate 2000 ms priority 7)... ");
  uint8_t SShandle = tasks.add(2000, 0, true, 7, updateScreenWrapper, "UpdateSpecificScreen");
  if (SShandle)  { VLF("success"); } else { VLF("FAILED!"); }

  #ifdef ODRIVE_MOTOR_PRESENT
    // update the ODrive Error status 
    VF("MSG: Setup, start ODrive Errors polling task (rate 1000 ms priority 7)... ");
    uint8_t ODhandle = tasks.add(1000, 0, true, 7, updateODriveErrWrapper, "UpdateODriveErrors");
    if (ODhandle) { VLF("success"); } else { VLF("FAILED!"); }

    // update battery voltage
    VF("MSG: Setup, start battery status polling task (rate 5000 ms priority 7)... ");
    uint8_t BVhandle = tasks.add(5000, 0, true, 7, updateBatVoltWrapper, "UpdateBatVolt");
    if (BVhandle)  { VLF("success"); } else { VLF("FAILED!"); }
  
    // make sure motor power starts OFF
    oDriveExt.odriveAzmPwr = false;
    oDriveExt.odriveAltPwr = false;
  #endif

  // draw Home screen
  tft.setFont(&Inconsolata_Bold8pt7b);
  homeScreen.draw();
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

// select which screen to update
void Display::updateSpecificScreen() {
  if (lastScreen != currentScreen) {
    firstDraw = true;
    lastScreen = currentScreen;
  }
  
  switch (currentScreen) {
    case HOME_SCREEN:       homeScreen.updateThisStatus();      break;
    case GUIDE_SCREEN:      guideScreen.updateThisStatus();     break;
    case FOCUSER_SCREEN:    dCfocuserScreen.updateThisStatus(); break;
    case GOTO_SCREEN:       gotoScreen.updateThisStatus();      break;
    case MORE_SCREEN:       moreScreen.updateThisStatus();      break;
    case SETTINGS_SCREEN:   settingsScreen.updateThisStatus();  break;
    case ALIGN_SCREEN:      alignScreen.updateThisStatus();     break;
    case CATALOG_SCREEN:    catalogScreen.updateThisStatus();   break;
    case PLANETS_SCREEN:    planetsScreen.updateThisStatus();   break;
    case CUST_CAT_SCREEN:   catalogScreen.updateThisStatus();   break;
    case XSTATUS_SCREEN:                                        break;
    #ifdef ODRIVE_MOTOR_PRESENT
      case ODRIVE_SCREEN:   oDriveScreen.updateThisStatus();    break;
    #endif
  }
  firstDraw = false;

  tasks.yield(100);
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
  tasks.yield(20);
  strcpy(reply, SERIAL_LOCAL.receive()); 
}

void Display::getLocalCmdTrim(const char *command, char *reply) {
  SERIAL_LOCAL.transmit(command); 
  tasks.yield(20);
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

// Update Data Field text using bitmap canvas
void Display::canvPrint(int x, int y, int y_off, int width, int height, const char* label) {
  char rjlabel[50];
  int y_box_offset = 10;
  GFXcanvas1 canvas(width, height);

  canvas.setFont(&Inconsolata_Bold8pt7b);  
  canvas.setCursor(0, (height-y_box_offset)/2 + y_box_offset); // offset from top left corner of canvas box
  sprintf(rjlabel, "%9s", label);
  canvas.print(rjlabel);
  tft.drawBitmap(x, y - y_box_offset + y_off, canvas.getBuffer(), width, height, textColor, butBackground);
}

void Display::canvPrint(int x, int y, int y_off, int width, int height, double label) {
  char rjlabel[50];
  int y_box_offset = 10;
  int x_fontSize = 10; // pixels width of a character
  GFXcanvas1 canvas(width, height);

  canvas.setFont(&Inconsolata_Bold8pt7b);
  canvas.setCursor(0, (height-y_box_offset)/2 + y_box_offset); // offset from top left corner of canvas box
  dtostrf(label, width/x_fontSize, 1, rjlabel); // right justify text in the bitmap
  canvas.print(rjlabel);
  tft.drawBitmap(x, y - y_box_offset + y_off, canvas.getBuffer(), width, height, textColor, butBackground);
}

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

// Color Themes (Day and Night)
void Display::setDayNight() {
  if (!nightMode) {
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

// ============ OnStep Command Errors ===============
void Display::updateOnStepCmdStatus() {
  if (currentScreen == CATALOG_SCREEN || 
    currentScreen == PLANETS_SCREEN ||
    currentScreen == CUST_CAT_SCREEN) return;
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
  if (currentScreen == CATALOG_SCREEN || 
    currentScreen == PLANETS_SCREEN ||
    currentScreen == CUST_CAT_SCREEN) return;
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

  // sound varying frequency alarm if Motor and Encoders positioins are too far apart
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

  
  switch(currentScreen) {
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
  currentBatVoltage = oDriveExt.getODriveBusVoltage();
  if ((currentBatVoltage != lastBatVoltage) || firstDraw) {
    if (currentBatVoltage < BATTERY_LOW_VOLTAGE) {
      tft.setCursor(140, 40);
      tft.print(currentBatVoltage);
    } else {
      tft.setCursor(140, 40);
      tft.print(currentBatVoltage);
    }
    lastBatVoltage = currentBatVoltage;
  }
}

// UpdateCommon Status - Real time data update for the particular labels printed above
// This Common Status is found at the top of most pages.
void Display::updateCommonStatus() { 
  if (currentScreen == CATALOG_SCREEN || 
      currentScreen == PLANETS_SCREEN ||
      currentScreen == CUST_CAT_SCREEN) return;

  // One Time update the SHC LST and Latitude if GPS locked
  if (tls.isReady() && firstGPS) {
    cat_mgr.setLstT0(shc.getLstT0()); 
    cat_mgr.setLat(shc.getLat());
    firstGPS = false;
  }

  int y_offset = 0;
  // Column 1
  // Current RA, Returns: HH:MM.T# or HH:MM:SS# (based on precision setting)
  getLocalCmdTrim(":GR#", ra_hms);
  if ((strcmp(currentRA, ra_hms)) || firstDraw) {
   canvPrint(COM_COL1_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH, C_HEIGHT, ra_hms);
    strcpy(currentRA, ra_hms);
  }

// Target RA, Returns: HH:MM.T# or HH:MM:SS (based on precision setting)
  y_offset +=COM_LABEL_Y_SPACE; 
  getLocalCmdTrim(":Gr#", tra_hms);
  if ((strcmp(currentTargRA, tra_hms)) || firstDraw) {
    canvPrint(COM_COL1_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH, C_HEIGHT, tra_hms);
    strcpy(currentTargRA, tra_hms);
  }

// Current DEC
   y_offset +=COM_LABEL_Y_SPACE; 
  getLocalCmdTrim(":GD#", dec_dms);
  if ((strcmp(currentDEC, dec_dms)) || firstDraw) {
    canvPrint(COM_COL1_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH, C_HEIGHT, dec_dms);
    strcpy(currentDEC, dec_dms);
  }

  // Target DEC
  y_offset +=COM_LABEL_Y_SPACE;  
  getLocalCmdTrim(":Gd#", tdec_dms); 
  if ((strcmp(currentTargDEC, tdec_dms)) || firstDraw) {
    canvPrint(COM_COL1_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH, C_HEIGHT, tdec_dms);
    strcpy(currentTargDEC, tdec_dms);
  }

  // ============ Column 2 ==============
  y_offset =0;

  // === Show both Target and Current ALT and AZM ===

  // Calculate Target AZM / ALT from Column 1 (:Gr#, :Gd#, target RA/DEC)
  // first, convert them to Double
  //shc.hmsToDouble(&currentTargRA_d, currentTargRA);
  //shc.dmsToDouble(&currentTargDEC_d, currentTargDEC, true, true);
 
  // convert to Horizon
 // currentTargRA_d *= 15;
  //cat_mgr.EquToHor(currentTargRA_d, currentTargDEC_d, &tAlt_d, &tAzm_d);

  // Get Current ALT and AZ and display them as Double
  getLocalCmdTrim(":GZ#", cAzmDMS); // DDD*MM'SS# 
  shc.dmsToDouble(&cAzm_d, cAzmDMS, false, true);

  getLocalCmdTrim(":GA#", cAltDMS);	// sDD*MM'SS#
  shc.dmsToDouble(&cAlt_d, cAltDMS, true, true);

  // Get Target ALT and AZ and display them as Double
  getLocalCmdTrim(":Gz#", tAzmDMS); // DDD*MM'SS# 
  shc.dmsToDouble(&tAzm_d, tAzmDMS, false, true);

  getLocalCmdTrim(":Gal#", tAltDMS);	// sDD*MM'SS#
  shc.dmsToDouble(&tAlt_d, tAltDMS, true, true);

  //--Current-- AZM float
  if ((current_cAzm != cAzm_d) || firstDraw) { 
    canvPrint(COM_COL2_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH-15, C_HEIGHT, cAzm_d);
    current_cAzm = cAzm_d;
  }

  // --Target-- AZM in degrees float
  y_offset +=COM_LABEL_Y_SPACE;
  if ((current_tAzm != tAzm_d) || firstDraw) {
    canvPrint(COM_COL2_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH-15, C_HEIGHT, tAzm_d);
    current_tAzm = tAzm_d;
  }

  // --Current-- ALT
  y_offset +=COM_LABEL_Y_SPACE;
  if ((current_cAlt != cAlt_d) || firstDraw) {
    canvPrint(COM_COL2_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH-15, C_HEIGHT, cAlt_d);
    current_cAlt = cAlt_d;
  }
  
  // --Target-- ALT in degrees
  y_offset +=COM_LABEL_Y_SPACE;
  if ((current_tAlt != tAlt_d) || firstDraw) {
    canvPrint(COM_COL2_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH-15, C_HEIGHT, tAlt_d);
    current_tAlt = tAlt_d;
  }
  firstDraw = false;
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

//================ SHC routines ====================
// Copied from SHC
// integer numeric conversion with error checking
bool SHC::atoi2(char *a, int *i) {
  char *conv_end;
  long l = strtol(a, &conv_end, 10);

  if (l < -32767 || l > 32768 || &a[0] == conv_end) return false;
  *i = l;
  return true;
}

// Copied this from Smart Hand Controller LX200.cpp and renamed it
// convert string in format HH:MM:SS to floating point
// (also handles)           HH:MM.M
bool SHC::hmsToDouble(double *f, char *hms) {
  char h[3],m[5],s[3];
  int  h1,m1,m2=0,s1=0;
  bool highPrecision=true;
  
  while (*hms==' ') hms++; // strip prefix white-space

  if (highPrecision) { if (strlen(hms)!= 8) return false; } else if (strlen(hms)!= 7) return false;

  h[0]=*hms++; h[1]=*hms++; h[2]=0; shc.atoi2(h,&h1);
  if (highPrecision) {
    if (*hms++!=':') return false; m[0]=*hms++; m[1]=*hms++; m[2]=0; shc.atoi2(m,&m1);
    if (*hms++!=':') return false; s[0]=*hms++; s[1]=*hms++; s[2]=0; shc.atoi2(s,&s1);
  } else {
    if (*hms++!=':') return false; m[0]=*hms++; m[1]=*hms++; m[2]=0; shc.atoi2(m,&m1);
    if (*hms++!='.') return false; m2=(*hms++)-'0';
  }
  if ((h1<0) || (h1>23) || (m1<0) || (m1>59) || (m2<0) || (m2>9) || (s1<0) || (s1>59)) return false;
  
  *f=h1+m1/60.0+m2/600.0+s1/3600.0;
  return true;
}

// DegreesMinutesSeconds to double
// Copied this from Smart Hand Controller LX200.cpp and renamed it
// dmsToDouble - convert string in format sDD:MM:SS to floating point
// (also handles)           DDD:MM:SS
//                          sDD:MM
//                          DDD:MM
//                          sDD*MM
//                          DDD*MM
bool SHC::dmsToDouble(double *f, char *dms, bool sign_present, bool highPrecision) {
  char d[4],m[5],s[3];
  int d1, m1, s1=0;
  int lowLimit=0, highLimit=360;
  int checkLen,checkLen1;
  double sign = 1.0;
  bool secondsOff = false;

  while (*dms==' ') dms++; // strip prefix white-space

  checkLen1=strlen(dms);

  // determine if the seconds field was used and accept it if so
  if (highPrecision) { 
    checkLen=9;
    if (checkLen1 != checkLen) return false;
  } else {
    checkLen=6;
    if (checkLen1 != checkLen) {
      if (checkLen1==9) { secondsOff=false; checkLen=9; } else return false;
    } else secondsOff = true;
  }

  // determine if the sign was used and accept it if so
  if (sign_present) {
    if (*dms=='-') sign=-1.0; else if (*dms=='+') sign=1.0; else return false; 
    dms++; d[0]=*dms++; d[1]=*dms++; d[2]=0; if (!shc.atoi2(d,&d1)) return false;
  } else {
    d[0]=*dms++; d[1]=*dms++; d[2]=*dms++; d[3]=0; if (!shc.atoi2(d,&d1)) return false;
  }

  // make sure the seperator is an allowed character
  if ((*dms!=':') && (*dms!='*') && (*dms!=char(223))) return false; else dms++;

  m[0]=*dms++; m[1]=*dms++; m[2]=0; if (!shc.atoi2(m,&m1)) return false;

  if ((highPrecision) && (!secondsOff)) {
    // make sure the seperator is an allowed character
    if (*dms++!=':') return false; 
    s[0]=*dms++; s[1]=*dms++; s[2]=0; shc.atoi2(s,&s1);
  }

  if (sign_present) { lowLimit=-90; highLimit=90; }
  if ((d1<lowLimit) || (d1>highLimit) || (m1<0) || (m1>59) || (s1<0) || (s1>59)) return false;
  
  *f=sign*(d1+m1/60.0+s1/3600.0);
  return true;
}

// modified this for xChan format from SHC
double SHC::getLstT0() {
  double f=0;
  display.getLocalCmdTrim(":GS#", locCmdReply);
  hmsToDouble(&f, locCmdReply);
  return f;
};

// modified this for xChan format from SHC
double SHC::getLat() {
  display.getLocalCmdTrim(":Gt#", locCmdReply);
  double f=-10000;
  dmsToDouble(&f, locCmdReply, true, false);
  return f;
};

// =============== Mount Status ===============
// Use local command channel to get mount status
bool LCmountStatus::isHome() {
  char reply[20];
  display.getLocalCmdTrim(":GU#", reply); 
  if (strstr(reply,"H")) return true; else return false;
}

bool LCmountStatus::isSlewing() {
  char reply[20];
  display.getLocalCmdTrim(":GU#", reply); 
  if (strstr(reply,"N")) return false; else return true;
}

bool LCmountStatus::isParked() {
  char reply[20];
  display.getLocalCmdTrim(":GU#", reply); 
  if (strstr(reply,"P")) return true; else return false;
}

bool LCmountStatus::isTracking() {
  char reply[20];
  display.getLocalCmdTrim(":GU#", reply);
  if (strstr(reply,"n")) return false; else return true;
}

Display display;
SHC shc;
LCmountStatus lCmountStatus;