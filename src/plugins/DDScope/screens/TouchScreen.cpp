// =====================================================
// TouchScreen.cpp
// Author: Richard Benear 2022

#include "TouchScreen.h"
#include "../../../lib/tasks/OnTask.h"
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
  #include "../screens/ODriveScreen.h"
#endif

void touchWrapper()  { touchScreen.touchScreenPoll(); }

void TouchScreen::init() {
// Start TouchScreen
  if (!ts.begin()) {
    VLF("MSG: TouchScreen, unable to start");
  } else {
    pinMode(TS_IRQ, INPUT_PULLUP); // XPT2046 library doesn't turn on pullup
    ts.setRotation(3); // touchscreen rotation
    VLF("MSG: TouchScreen, started");
  }

  // start touchscreen task
  VF("MSG: Setup, start TouchScreen polling task (rate 300 ms priority 7)... ");
  uint8_t TShandle = tasks.add(300, 0, true, 7, touchWrapper, "TouchScreen");
  if (TShandle)  { VLF("success"); } else { VLF("FAILED!"); }
  tasks.setTimingMode(TShandle, TM_MINIMUM);
}

// Poll the TouchScreen
void TouchScreen::touchScreenPoll() {
  if (ts.touched()) {
    if (screenTouched) return; // effectively a debounce, should be false if valid

    // tell other screens to process button states...they will clear this flag when done
    screenTouched = true;
    p = ts.getPoint();      

    // Scale from ~0->4000 to tft.width using the calibration #'s
    //VF("x="); V(p.x); VF(", y="); V(p.y); VF(", z="); VL(p.z); // for calibration
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    //VF("x="); V(p.x); VF(", y="); V(p.y); VF(", z="); VL(p.z); //for calibration

    // Check for touchscreen button action on the selected Screen
    switch (currentScreen) {
      case HOME_SCREEN:     homeScreen.touchPoll(p.x, p.y);       break;
      case GUIDE_SCREEN:    guideScreen.touchPoll(p.x, p.y);      break;
      case FOCUSER_SCREEN:  dCfocuserScreen.touchPoll(p.x, p.y);  break;
      case GOTO_SCREEN:     gotoScreen.touchPoll(p.x, p.y);       break;
      case MORE_SCREEN:     moreScreen.touchPoll(p.x, p.y);       break;
      case SETTINGS_SCREEN: settingsScreen.touchPoll(p.x, p.y);   break;
      case ALIGN_SCREEN:    alignScreen.touchPoll(p.x, p.y);      break;
      case CATALOG_SCREEN:  catalogScreen.touchPoll(p.x, p.y);    break;
      case PLANETS_SCREEN:  planetsScreen.touchPoll(p.x, p.y);    break;
      case XSTATUS_SCREEN:                                        break;

      #ifdef ODRIVE_MOTOR_PRESENT
        case ODRIVE_SCREEN:   oDriveScreen.touchPoll(p.x, p.y); break;
      #endif

      default:              homeScreen.touchPoll(p.x, p.y); break;
    }
    tasks.yield(15);
    
    // *************** MENU MAP ****************
    // Current Screen   |Cur |Col1|Col2|Col3|Col4|
    // Home-----------| Ho | Gu | Fo | GT | Mo |
    // Guide----------| Gu | Ho | Fo | Al | Mo |
    // Focuser--------| Fo | Ho | Gu | GT | Mo |
    // GoTo-----------| GT | Ho | Fo | Gu | Mo |

    //if ODRIVE_PRESENT then use this menu structure
    //  More & (CATs)--| Mo | GT | Se | Od | Al |
    //  ODrive---------| Od | Ho | Se | Al | Xs |
    //  Extended Status| Xs | Ho | Se | Al | Od |
    //  Settings-------| Se | Ho | Fo | Al | Od |
    //  Alignment------| Al | Ho | Fo | Gu | Od |
    //else if not ODRIVE_PRESENT use this menu structure
    //  More & (CATs)--| Mo | GT | Se | Gu | Al |
    //  Extended Status| Xs | Ho | Se | Al | Mo |
    //  Settings-------| Se | Ho | Xs | Al | Mo |
    //  Alignment------| Al | Ho | Fo | Gu | Mo |

    // Detect which Menu Screen is requested
    // skip checking these page menus since they don't have this menu setup
    if ((currentScreen == CATALOG_SCREEN) || 
        (currentScreen == PLANETS_SCREEN) ||  
        (currentScreen == CUST_CAT_SCREEN)) return; 
    
    firstDraw = true;
    
    // Check for any Menu buttons pressed
    // == LeftMost Menu Button ==
    if (p.y > MENU_Y && p.y < (MENU_Y + MENU_BOXSIZE_Y) && p.x > (MENU_X                   ) && p.x < (MENU_X                    + MENU_BOXSIZE_X)) {
      DD_CLICK;
      switch(currentScreen) {
        case HOME_SCREEN:     guideScreen.draw(); break;
        case GUIDE_SCREEN:    homeScreen.draw(); break;
        case FOCUSER_SCREEN:  homeScreen.draw(); break;
        case GOTO_SCREEN:     homeScreen.draw(); break;
        case MORE_SCREEN:     gotoScreen.draw(); break;
        case SETTINGS_SCREEN: homeScreen.draw(); break;
        case ALIGN_SCREEN:    homeScreen.draw(); break;

        #ifdef ODRIVE_MOTOR_PRESENT
          case ODRIVE_SCREEN:     moreScreen.draw(); break;
        #endif
        
        default:              homeScreen.draw(); break;
      }
    }
    // == Center Left Menu - Column 2 ==
    if (p.y > MENU_Y && p.y < (MENU_Y + MENU_BOXSIZE_Y) && p.x > (MENU_X +   MENU_X_SPACING) && p.x < (MENU_X +   MENU_X_SPACING + MENU_BOXSIZE_X)) {
      DD_CLICK;
      switch(currentScreen) {
        case HOME_SCREEN:     dCfocuserScreen.draw(); break;
        case GUIDE_SCREEN:    dCfocuserScreen.draw(); break;
        case FOCUSER_SCREEN:      guideScreen.draw(); break;
        case GOTO_SCREEN:     dCfocuserScreen.draw(); break;
        case MORE_SCREEN:      settingsScreen.draw(); break;
        case SETTINGS_SCREEN: extStatusScreen.draw(); break;
        case ALIGN_SCREEN:    dCfocuserScreen.draw(); break;

        #ifdef ODRIVE_MOTOR_PRESENT
          case ODRIVE_SCREEN:     moreScreen.draw(); break;
        #endif

        default:                 homeScreen.draw(); break;
      }
    }
    // == Center Right Menu - Column 3 ==
    if (p.y > MENU_Y && p.y < (MENU_Y + MENU_BOXSIZE_Y) && p.x > (MENU_X + 2*MENU_X_SPACING) && p.x < (MENU_X + 2*MENU_X_SPACING + MENU_BOXSIZE_X)) {
      DD_CLICK;
      switch(currentScreen) {
        case HOME_SCREEN:      gotoScreen.draw(); break;
        case GUIDE_SCREEN:    alignScreen.draw(); break;
        case FOCUSER_SCREEN:   gotoScreen.draw(); break;
        case GOTO_SCREEN:     guideScreen.draw(); break;
        case MORE_SCREEN:    oDriveScreen.draw(); break;
        case SETTINGS_SCREEN: alignScreen.draw(); break;
        case ALIGN_SCREEN:    guideScreen.draw(); break;

        #ifdef ODRIVE_MOTOR_PRESENT
          case ODRIVE_SCREEN:     moreScreen.draw(); break;
        #endif

        default:               homeScreen.draw(); break;
      }
    }
    // == Right Menu - Column 4 ==
    if (p.y > MENU_Y && p.y < (MENU_Y + MENU_BOXSIZE_Y) && p.x > (MENU_X + 3*MENU_X_SPACING) && p.x < (MENU_X + 3*MENU_X_SPACING + MENU_BOXSIZE_X)) {
      DD_CLICK;   
      switch(currentScreen) {
        case HOME_SCREEN:       moreScreen.draw(); break;
        case GUIDE_SCREEN:      moreScreen.draw(); break;
        case FOCUSER_SCREEN:    moreScreen.draw(); break;
        case GOTO_SCREEN:       moreScreen.draw(); break;
        case MORE_SCREEN:      alignScreen.draw(); break;
        case SETTINGS_SCREEN: oDriveScreen.draw(); break;
        case ALIGN_SCREEN:    oDriveScreen.draw(); break;

        #ifdef ODRIVE_MOTOR_PRESENT
          case ODRIVE_SCREEN:     moreScreen.draw(); break;
        #endif
        
        default:                homeScreen.draw(); break;
      }
    }
    tasks.yield(15);
  } 
}

TouchScreen touchScreen;

