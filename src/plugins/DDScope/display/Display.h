// -------------------------------------------------------------------------------------------------
// Display.h

#ifndef DISPLAY_H
#define DISPLAY_H

#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <XPT2046_Touchscreen.h>
#include "../Adafruit_ILI9486_Teensy/Adafruit_ILI9486_Teensy.h"

// OnStepX includes
#include "../../../Common.h"
#include "../../../lib/serial/Serial_Local.h"
#include "../../../lib/tasks/OnTask.h"
#include "../../../lib/sound/Sound.h"
#include "../../../libApp/commands/ProcessCmds.h"
#include "../../../telescope/mount/Mount.h"
#include "../../../telescope/mount/goto/GoTo.h"
#include "../../../lib/tls/GPS.h"
#include "../../../lib/serial/Serial_Local.h"

// Fonts
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "../fonts/UbuntuMono_Bold8pt7b.h"
#include "../fonts/UbuntuMono_Bold11pt7b.h"
#include "../fonts/UbuntuMono_Bold14pt7b.h"
#include "../fonts/UbuntuMono_Bold16pt7b.h"
#include "../fonts/ARCENA18pt7b.h"

// DDScope specific
#include "../../../pinmaps/Pins.DDT.h"
#include "../odriveExt/ODriveExt.h"
#include "../catalog/Catalog.h"
#include "../screens/AlignScreen.h"
#include "../screens/CatalogScreen.h"
#include "../screens/FocuserScreen.h"
#include "../screens/GotoScreen.h"
#include "../screens/GuideScreen.h"
#include "../screens/HomeScreen.h"
#include "../screens/MoreScreen.h"
#include "../screens/OdriveScreen.h"
#include "../screens/PlanetsScreen.h"
#include "../screens/SettingsScreen.h"

#define C_WIDTH  80
#define C_HEIGHT 14

#define BUTTON_ON    true
#define BUTTON_OFF   false

// ODrive hardwired motor numbers
#define AZM_MOTOR 1
#define ALT_MOTOR 0 

// recommended cutoff for LiPo battery is 19.2V but want some saftey margin
#define BATTERY_LOW_VOLTAGE    21  

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX        250 
#define TS_MINY        250
#define TS_MAXX       3900
#define TS_MAXY       3900
#define MINPRESSURE     80
#define MAXPRESSU     1000
#define PENRADIUS        3

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
  CATALOG_SCREEN,  // 8
  PLANETS_SCREEN,  // 9
  CUST_CAT_SCREEN  // 10
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

extern bool ODpositionUpdateEnabled;

// other Display related objects
static Sound ddTone;
static Adafruit_ILI9486_Teensy tft;
static XPT2046_Touchscreen ts(TS_CS, TS_IRQ); // Use Interrupts for touchscreen
static TS_Point p;

// =========================================
class Display {
  public:
    void init();
    void sdInit();
    void update();

  // Local Command Channel support
    void setLocalCmd(char *command);
    void setLocalCmd(const char *command);
    void getLocalCmd(const char *command, char *reply);
    void getLocalCmdTrim(const char *command, char *reply);

  // Colors, Buttons, BitMap printing
    void drawButton(int x_start, int y_start, int w, int h, bool butOn, int text_x_offset, int text_y_offset, const char* label);
    void drawTitle(int text_x_offset, int text_y_offset, const char* label);
    void canvPrint(int x, int y, int y_off, int width, int height, const char* label);
    void canvPrint(int x, int y, int y_off, int width, int height, double label);
    void canvPrint(int x, int y, int y_off, int width, int height, int label);
    void drawMenuButtons();
    void drawPic(File *StarMaps, uint16_t x, uint16_t y, uint16_t WW, uint16_t HH);

    // Status and updates
    Screen currentScreen = HOME_SCREEN;
    void updateScreenStatus();
    void touchScreenPoll();    
    void updateOnStepCmdStatus();
    void drawCommonStatusLabels();
    void updateCommonStatus();
    void updateColors();
    float getBatteryVoltage();

    // frequency based tone add here since not supported in OnStepX
    void soundFreq(int freq); 

    // Color Theme
    uint16_t pgBackground = DEEP_MAROON; 
    uint16_t butBackground = BLACK;
    uint16_t butOnBackground = RED;
    uint16_t textColor = YELLOW; 
    uint16_t butOutline = YELLOW; 

    bool firstDraw = false;
    bool refreshScreen;
    bool screenTouched;
    bool nightMode;

  private:
    int touchHandle;
    int updateScreenHandle;
    int focuserDcHandle;
  
    char ra_hms[10], dec_dms[11];
    char tra_hms[10], tdec_dms[11];
    char currentRA[10];
    char currentDEC[11]; 
    char currentTargRA[10];
    char currentTargDEC[11];
    double currentTargRA_d;
    double currentTargDEC_d;
    double azm_d;
    double alt_d;
    double tazm_d;
    double talt_d;
    double current_azm;
    double current_alt;
    double tra_d;
    double tra_dha;
    double tdec_d;
    double current_tazm;
    double current_talt;
    double altitudeFt;

    bool batLED = false;
    bool firstGPS = true;
};

extern Display display;

// Leveraged the following from SHC with some modifications
class SHC {
  public:
    bool atoi2(char *a, int *i);
    bool dmsToDouble(double *f, char *dms, bool sign_present, bool highPrecision);
    bool hmsToDouble(double *f, char *hms);
    double getLstT0();
    double getLat();
    void EquToHor(double RA, double Dec, double *Alt, double *Azm);

  private:
    char locCmdReply[16];
};

extern SHC shc;

// Local command channel mount status
class MountStatus {
  public:
    bool isSlewing();
    bool isParked();
    bool isHome();
    bool isTracking();

  private:
    char xchReply[10];
};

extern MountStatus mountStatus;

#endif