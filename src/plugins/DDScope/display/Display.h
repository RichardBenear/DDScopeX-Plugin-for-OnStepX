// -------------------------------------------------------------------------------------------------
// Display.h

#ifndef DISPLAY_H
#define DISPLAY_H

#include <SD.h>
#include "src/Common.h"
#include <XPT2046_Touchscreen.h>
#include "../Adafruit_ILI9486_Teensy/Adafruit_ILI9486_Teensy.h"
#include "UIelements.h"

#define C_WIDTH  80
#define C_HEIGHT 14

#define BUT_ON  true
#define BUT_OFF false

// ODrive hardwired motor numbers
#define AZM_MOTOR 1
#define ALT_MOTOR 0 

#define TITLE_TEXT_Y       25

// Screen Selection buttons
#define MENU_X              3
#define MENU_Y             46
#define MENU_X_SPACING     80
#define MENU_Y_SPACING      0
#define MENU_BOXSIZE_X     72
#define MENU_BOXSIZE_Y     45
#define MENU_TEXT_X_OFFSET  8
#define MENU_TEXT_Y_OFFSET 28

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX           250 
#define TS_MINY           250
#define TS_MAXX          3900
#define TS_MAXY          3900
#define MINPRESSURE        80
#define MAXPRESSU        1000
#define PENRADIUS           3

// Color definitions
#define BLACK       0x0000      /*   0,   0,   0 */
#define NAVY        0x000F      /*   0,   0, 128 */
#define DARKGREEN   0x03E0      /*   0, 128,   0 */
#define DARKCYAN    0x03EF      /*   0, 128, 128 */
#define MAROON      0x7800      /* 128,   0,   0 */
#define PURPLE      0x780F      /* 128,   0, 128 */
#define OLIVE       0x7BE0      /* 128, 128,   0 */
#define LIGHTGREY   0xC618      /* 192, 192, 192 */
#define DARKGREY    0x7BEF      /* 128, 128, 128 */
#define BLUE        0x001F      /*   0,   0, 255 */
#define GREEN       0x07E0      /*   0, 255,   0 */
#define CYAN        0x07FF      /*   0, 255, 255 */
#define RED         0xF800      /* 255,   0,   0 */
#define MAGENTA     0xF81F      /* 255,   0, 255 */
#define YELLOW      0xFFE0      /* 255, 255,   0 */
#define WHITE       0xFFFF      /* 255, 255, 255 */
#define ORANGE      0xFD20      /* 255, 165,   0 */
#define GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define PINK        0xF81F		//
#define DEEP_MAROON 0x4800		// 

// recommended cutoff for LiPo battery is 19.2V but want some saftey margin
#define BATTERY_LOW_VOLTAGE   21  

// sound control of both duration and frequency
#define BEEP tone(STATUS_BUZZER_PIN, 1000UL, 40ULL); // both in milliseconds
#define ALERT tone(STATUS_BUZZER_PIN, 700UL, 80ULL); // both in milliseconds

enum Screen
{
  HOME_SCREEN,     // 0
  GUIDE_SCREEN,    // 1
  FOCUSER_SCREEN,  // 2
  GOTO_SCREEN,     // 3
  MORE_SCREEN,     // 4
  ODRIVE_SCREEN,   // 5 
  SETTINGS_SCREEN, // 6
  ALIGN_SCREEN,    // 7
  PLANETS_SCREEN,  // 8
  XSTATUS_SCREEN,  // 9
  TREASURE_SCREEN, // 10
  CUSTOM_SCREEN,   // 11
  SHC_CAT_SCREEN   // 12
}; 

enum SelectedCatalog
{
  STARS,
  MESSIER,
  CALDWELL,
  HERSCHEL,
  INDEX,
  PLANETS,
  TREASURE,
  CUSTOM,
};

//----------------------------------------------
// Structure storing Button object
//----------------------------------------------
typedef struct 
{
  bool                  initialized    = false;
  uint16_t              x              = 0;
  uint16_t              y              = 0;
  uint8_t               width          = 0;
  uint8_t               height         = 0;
  bool                  pressed        = false;
  uint16_t              colorActive    = 0xF800;
  uint16_t              colorNotActive = 0x0000;
  uint16_t              colorBorder    = 0xFFE0;
  const char*           label          = "";
  bool                  monitorOn      = false;
  int                   fontCharWidth  = 8;
  int                   fontCharHeight = 16;
} ButObject;

// Display objects
extern Adafruit_ILI9486_Teensy tft;

static XPT2046_Touchscreen ts(TS_CS, TS_IRQ); // Use Interrupts for touchscreen
static TS_Point p;

// =========================================
class Display {
  public:
    void init();
    void sdInit();
    void refreshButtons();
    void setCurrentScreen(Screen);
 
  // Local Command Channel support
    void setLocalCmd(char *command);
    void setLocalCmd(const char *command);
    void getLocalCmd(const char *command, char *reply);
    void getLocalCmdTrim(const char *command, char *reply);

  // Colors, Buttons, BitMap printing
    
    void drawButton(int x_start, int y_start, int w, int h, bool butOn, int text_x_offset, int text_y_offset, const char* label);
    
    void drawTitle(int text_x_offset, int text_y_offset, const char* label);

    void canvPrint(int x, int y, int y_off, int width, int height, const char* label);
    void canvPrint(int x, int y, int y_off, int width, int height, const char* label, uint16_t textColor, uint16_t butBackgnd);
    void canvPrint(int x, int y, int y_off, int width, int height, double label);
    void canvPrint(int x, int y, int y_off, int width, int height, double label, uint16_t textColor, uint16_t butBackgnd);
    void canvPrint(int x, int y, int y_off, int width, int height, int label);

    void drawMenuButtons();
    void drawCommonStatusLabels();
    void drawPic(File *StarMaps, uint16_t x, uint16_t y, uint16_t WW, uint16_t HH);

    // Status and updates
    void updateSpecificScreen();
    void updateCommonStatus();  
    void updateOnStepCmdStatus();

    #ifdef ODRIVE_MOTOR_PRESENT
      void updateODriveErrBar();
      void updateBatVoltage();
    #endif

    // Day or Night Modes
    void setNightMode(bool);
    bool getNightMode();

    // Color Theme
    uint16_t pgBackground = DEEP_MAROON; 
    uint16_t butBackground = BLACK;
    uint16_t butOnBackground = RED;
    uint16_t textColor = YELLOW; 
    uint16_t butOutline = YELLOW; 

    // Fonts
    const uint8_t  mainFontWidth = 8; // 8pt
    const uint8_t  mainFontHeight = 16; // 8pt
    const uint8_t  largeFontWidth = 11; // 11pt
    const uint8_t  largeFontHeight = 22; // 11pt
    const uint8_t  xlargeFontWidth = 17; // 12pt
    const uint8_t  xlargeFontHeight = 29; // 12pt

    

    // frequency and duration adjustable tone
    inline void soundFreq(int freq, int duration) { tone(STATUS_BUZZER_PIN, freq, duration); }

    static Screen currentScreen;
    static bool _nightMode;
    static bool _redrawBut;

  private:
    CommandError commandError = CE_NONE;
    bool firstGPS = true;
};

extern Display display;

#endif