// =====================================================
// MoreScreen.cpp
// ====== CATALOG and "MORE" Menus Page ============
// =================================================
// The Catalog-and-More page allows access to More sub menus and Catalogs
// Author: Richard Benear - Dec 2021
//
// Also uses routines from Smart Hand Controller (SHC) 
// Copyright (C) 2018 to 2021 Charles Lemaire, Howard Dutton, and Others
// Author: Charles Lemaire, https://pixelstelescopes.wordpress.com/teenastro/
// Author: Howard Dutton, http://www.stellarjourney.com, hjd1964@gmail.com

#include "MoreScreen.h"
#include "CatalogScreen.h"
#include "PlanetsScreen.h"
#include "HomeScreen.h"
#include "../catalog/Catalog.h"
#include "../display/Display.h"
#include "../odriveExt/ODriveExt.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include <Fonts/FreeSansBold9pt7b.h>
#include "../../../telescope/mount/Mount.h"

// Catalog Selection buttons
#define CAT_SEL_X               5
#define CAT_SEL_Y              175
#define CAT_SEL_BOXSIZE_X      110 
#define CAT_SEL_BOXSIZE_Y       28
#define CAT_SEL_SPACER          CAT_SEL_BOXSIZE_Y + 6
#define CAT_SEL_TEXT_X_OFFSET    7 
#define CAT_SEL_TEXT_Y_OFFSET   17

// Tracking rate buttons
#define TRACK_R_X              125
#define TRACK_R_Y              180
#define TRACK_R_BOXSIZE_X       70 
#define TRACK_R_BOXSIZE_Y       24
#define TRACK_R_SPACER           1 
#define TRACK_R_GROUP_SPACER     5 
#define TRACK_R_TEXT_X_OFFSET    4 
#define TRACK_R_TEXT_Y_OFFSET   16 

// Misc Buttons
#define MISC_X                 212
#define MISC_Y                 160
#define MISC_BOXSIZE_X         100 
#define MISC_BOXSIZE_Y          28
#define MISC_SPACER_Y            8 
#define MISC_TEXT_X_OFFSET       2 
#define MISC_TEXT_Y_OFFSET      18

// Filter Button
#define FM_X                   200
#define FM_Y                   157
#define FM_BOXSIZE_X           120 
#define FM_BOXSIZE_Y            28
#define FM_SPACER_Y              2 
#define FM_TEXT_X_OFFSET         0 
#define FM_TEXT_Y_OFFSET        18

#define GOTO_BUT_X             217
#define GOTO_BUT_Y             260
#define GOTO_TXT_OFF_X          15
#define GOTO_TXT_OFF_Y          25
#define GOTO_M_BOXSIZE_X        90
#define GOTO_M_BOXSIZE_Y        40

#define ABORT_M_BUT_X          218
#define ABORT_M_BUT_Y          310

extern const char *activeFilterStr[3];

// ============= Initialize the Catalog & More page ==================
void MoreScreen::draw() {
  display.currentScreen = MORE_SCREEN;
  display.setDayNight();
  tft.setTextColor(display.textColor);
  tft.fillScreen(display.pgBackground);

  tft.setCursor(TRACK_R_X+2, TRACK_R_Y-16);
  tft.print("Tracking");
  tft.setCursor(TRACK_R_X+5, TRACK_R_Y-4);
  tft.print(" Rates");

  display.drawMenuButtons();
  display.drawTitle(60, 30, "Catalogs & Misc");

  tft.setCursor(30, 170);
  tft.print("Catalogs");

  // Draw the HOME Icon bitmap
  uint8_t extern black_house_icon[];
  tft.drawBitmap(10, 5, black_house_icon, 39, 31,  display.butBackground, ORANGE);

  display.drawCommonStatusLabels();
  tft.setFont(&Inconsolata_Bold8pt7b);
}

//================== Update the Buttons ======================
void MoreScreen::updateThisStatus() {

  if (display.screenTouched || display.firstDraw || display.refreshScreen) { //reduce screen flicker 
    display.refreshScreen = false;
    if (display.screenTouched) display.refreshScreen = true;

    // Show any target object data selected from Catalog
    uint16_t x = 120; uint16_t y = 358; 
    tft.fillRect(x-2, y+3, 199, 5*16, display.butBackground);
    
    tft.setCursor(x,y+16  ); tft.print(catalogScreen.catSelectionStr1);
    tft.setCursor(x,y+16*2); tft.print(catalogScreen.catSelectionStr2);
    tft.setCursor(x,y+16*3); tft.print(catalogScreen.catSelectionStr3);
    tft.setCursor(x,y+16*4); tft.print(catalogScreen.catSelectionStr4); 
    tft.setCursor(x,y+16*5); tft.print(catalogScreen.catSelectionStr5); 

    int y_offset = 0;
    int x_offset = 0;
    // Manage Tracking Rate buttons
    if (sidereal) {
        display.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_ON, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET,   "Sidereal");
        y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
        display.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  Lunar "); 
        y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
        display.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  King  ");
    }
    if (lunarRate) {
        y_offset = 0;
        display.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET,   "Sidereal");
        y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
        display.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_ON, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  Lunar "); 
        y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
        display.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  King  ");
    }
    if (kingRate) {
        y_offset = 0;
        display.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET,   "Sidereal");
        y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
        display.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  Lunar "); 
        y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
        display.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_ON, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  King  ");
    }   
    // increment tracking rate by 0.02 Hz
    y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER+TRACK_R_GROUP_SPACER ; // space between tracking setting fields
    if (incTrackRate) {
        display.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_ON, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "IncTrack");
        incTrackRate = false;
    } else {
        display.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "IncTrack"); 
    }   
    // decrement tracking rate by 0.02 Hz
    y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER; 
    if (decTrackRate) {
        display.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_ON, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "DecTrack");
        decTrackRate = false;
    } else {
        display.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "DecTrack"); 
    }   

    // Reset Tracking Rate Sidereal
    y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER+TRACK_R_GROUP_SPACER ; 
    if (rstTrackRate) {
        display.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_ON, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "Reseting");
        rstTrackRate = false;
    } else {
        display.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "RstTrack"); 
    }   

    // Show current Tracking Rate
    // For ALT/AZ this is always shows the default rate
    char _sideRate[12];
    y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER+12;
    char sideRate[10];
    display.getLocalCmdTrim(":GT#", sideRate);
    sprintf(_sideRate, "TR=%s", sideRate);
    display.canvPrint(TRACK_R_X-6, TRACK_R_Y+y_offset, 0, 90, 16, _sideRate);

    x_offset = 0;
    y_offset = 0;

    // Filter Selection Button - circular selection of 3 values
    if (filterBut || display.firstDraw) {
      display.drawButton(FM_X + x_offset, FM_Y + y_offset, FM_BOXSIZE_X, FM_BOXSIZE_Y, BUTTON_OFF, FM_TEXT_X_OFFSET+8, FM_TEXT_Y_OFFSET,  activeFilterStr[activeFilter]);
      if (activeFilter == FM_ALIGN_ALL_SKY && !objectSelected) {
      display.canvPrint(120, 382, 0, 199, 16, "All Sky For STARS only");
      }
      filterBut = false;
    }

    // Clear Custom Catalog
     y_offset += FM_BOXSIZE_Y + FM_SPACER_Y;
    if (clrCustom) {
      if (!yesCancelActive) {
        yesCancelActive = true;
        display.drawButton(MISC_X + x_offset, MISC_Y + y_offset, 30, MISC_BOXSIZE_Y, BUTTON_ON, MISC_TEXT_X_OFFSET, MISC_TEXT_Y_OFFSET,   "Yes");
        display.drawButton(MISC_X + 40, MISC_Y + y_offset, 60, MISC_BOXSIZE_Y, BUTTON_ON, MISC_TEXT_X_OFFSET, MISC_TEXT_Y_OFFSET,   "Cancel");
        if (!objectSelected) display.canvPrint(120, 382, 0, 199, 16, "Delete Custom Catalog?!");
      }
      if (yesBut) { // go ahead and clear
        display.drawButton(MISC_X + x_offset, MISC_Y + y_offset, MISC_BOXSIZE_X, MISC_BOXSIZE_Y, BUTTON_ON, MISC_TEXT_X_OFFSET+8, MISC_TEXT_Y_OFFSET,   " Clearing ");
        
        File rmFile = SD.open("/custom.csv");
          if (rmFile) {
            SD.remove("/custom.csv");
          }
        rmFile.close(); 

        yesBut = false;
        clrCustom = false;
        yesCancelActive = false;
        display.refreshScreen = true; 
      }
      if (cancelBut) {
        cancelBut = false;
        clrCustom = false;
        yesCancelActive = false;
        display.refreshScreen = true; 
        display.drawButton(MISC_X + x_offset, MISC_Y + y_offset, MISC_BOXSIZE_X, MISC_BOXSIZE_Y, BUTTON_OFF, MISC_TEXT_X_OFFSET+5, MISC_TEXT_Y_OFFSET, "Clr Custom");
        if (objectSelected) tft.fillRect(x, y, 199, 5*16, display.pgBackground);
      }
    } else {
      display.drawButton(MISC_X + x_offset, MISC_Y + y_offset, MISC_BOXSIZE_X, MISC_BOXSIZE_Y, BUTTON_OFF, MISC_TEXT_X_OFFSET+5, MISC_TEXT_Y_OFFSET, "Clr Custom");
    }

    // Buzzer Enable Button
    y_offset += MISC_BOXSIZE_Y + MISC_SPACER_Y;
    if (soundEnabled) {
      display.drawButton(MISC_X + x_offset, MISC_Y + y_offset, MISC_BOXSIZE_X, MISC_BOXSIZE_Y, BUTTON_ON, MISC_TEXT_X_OFFSET+12, MISC_TEXT_Y_OFFSET, "Buzzer On ");
    } else { //off
      display.drawButton(MISC_X + x_offset, MISC_Y + y_offset, MISC_BOXSIZE_X, MISC_BOXSIZE_Y, BUTTON_OFF, MISC_TEXT_X_OFFSET+12, MISC_TEXT_Y_OFFSET, "Buzzer Off");
    }

    // Larger Button Text for GoTo and Abort
    tft.setFont(&FreeSansBold9pt7b); 

    // Go To Coordinates Button
    if (goToButton) {
      display.drawButton( GOTO_BUT_X, GOTO_BUT_Y,  GOTO_M_BOXSIZE_X, GOTO_M_BOXSIZE_Y, BUTTON_ON, GOTO_TXT_OFF_X-2, GOTO_TXT_OFF_Y, "Going");
      goToButton = false;
    } else {
      if (!lCmountStatus.isSlewing()) { 
        display.drawButton( GOTO_BUT_X, GOTO_BUT_Y,  GOTO_M_BOXSIZE_X, GOTO_M_BOXSIZE_Y, BUTTON_OFF, GOTO_TXT_OFF_X+5, GOTO_TXT_OFF_Y, "GoTo"); 
      } else {
        display.refreshScreen = true;
      }
    }
    
    // Abort GoTo Button
    if (abortPgBut) {
      display.drawButton(ABORT_M_BUT_X, ABORT_M_BUT_Y, GOTO_M_BOXSIZE_X, GOTO_M_BOXSIZE_Y, BUTTON_ON, GOTO_TXT_OFF_X-5, GOTO_TXT_OFF_Y, "Aborting"); 
      abortPgBut = false;
    } else {
      display.drawButton(ABORT_M_BUT_X, ABORT_M_BUT_Y, GOTO_M_BOXSIZE_X, GOTO_M_BOXSIZE_Y, BUTTON_OFF, GOTO_TXT_OFF_X, GOTO_TXT_OFF_Y, " Abort"); 
    }

    tft.setFont(&Inconsolata_Bold8pt7b); // Text back to default

    // Draw the Catalog Buttons
    char title[16]="";
    y_offset = 0;
    for (uint16_t i=1; i<=cat_mgr.numCatalogs(); i++) {
      cat_mgr.select(i-1);
      strcpy(title,cat_mgr.catalogTitle());
      display.drawButton(CAT_SEL_X, CAT_SEL_Y+y_offset, CAT_SEL_BOXSIZE_X, CAT_SEL_BOXSIZE_Y, BUTTON_OFF, CAT_SEL_TEXT_X_OFFSET, CAT_SEL_TEXT_Y_OFFSET, title);
      y_offset += CAT_SEL_SPACER;
    }

    // Planet Catalog Button
    display.drawButton(CAT_SEL_X, CAT_SEL_Y+y_offset, CAT_SEL_BOXSIZE_X, CAT_SEL_BOXSIZE_Y, BUTTON_OFF, CAT_SEL_TEXT_X_OFFSET, CAT_SEL_TEXT_Y_OFFSET, "Planets");

    y_offset += CAT_SEL_SPACER;
    // Treasure Catalog
    display.drawButton(CAT_SEL_X, CAT_SEL_Y+y_offset, CAT_SEL_BOXSIZE_X, CAT_SEL_BOXSIZE_Y, BUTTON_OFF, CAT_SEL_TEXT_X_OFFSET, CAT_SEL_TEXT_Y_OFFSET, "Treasure");

    y_offset += CAT_SEL_SPACER;
    // Custom User Catalog Button
    display.drawButton(CAT_SEL_X, CAT_SEL_Y+y_offset, CAT_SEL_BOXSIZE_X, CAT_SEL_BOXSIZE_Y, BUTTON_OFF, CAT_SEL_TEXT_X_OFFSET, CAT_SEL_TEXT_Y_OFFSET, "Custom Cat");
  }
  display.screenTouched = false;
}

//================================================
// ===== TouchScreen Detect "MORE" page ==========
//================================================
void MoreScreen::touchPoll(uint16_t px, uint16_t py) {

  // Home Page ICON Button
  if (px > 10 && px < 50 && py > 5 && py < 37) {
    homeScreen.draw();
    return;
  }

  // Select Tracking Rates
  int x_offset = 0;
  int y_offset = 0;

  // Sidereal Rate 
  if (py > TRACK_R_Y+y_offset && py < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && px > TRACK_R_X && px < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
      display.setLocalCmd(":TQ#");
      sidereal = true;
      lunarRate = false;
      kingRate = false;
      return;
  }
  // Lunar Rate
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
  if (py > TRACK_R_Y+y_offset && py < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && px > TRACK_R_X && px < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
      display.setLocalCmd(":TL#");
      sidereal = false;
      lunarRate = true;
      kingRate = false;
      return;
  }
  // King Rate
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
  if (py > TRACK_R_Y+y_offset && py < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && px > TRACK_R_X && px < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
      display.setLocalCmd(":TK#");
      sidereal = false;
      lunarRate = false;
      kingRate = true;
      return;
  }
  // Increment tracking rate
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_GROUP_SPACER ;
  if (py > TRACK_R_Y+y_offset && py < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && px > TRACK_R_X && px < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
      display.setLocalCmd(":T+#");
      incTrackRate = true;
      decTrackRate = false;
      return;
  }
  // Decrement tracking rate
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
  if (py > TRACK_R_Y+y_offset && py < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && px > TRACK_R_X && px < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
      display.setLocalCmd(":T-#");
      incTrackRate = false;
      decTrackRate = true;
      return;
  }
  // Reset tracking rate
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_GROUP_SPACER ;
  if (py > TRACK_R_Y+y_offset && py < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && px > TRACK_R_X && px < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
      display.setLocalCmd(":TR#");
      rstTrackRate = true;
      return;
  }

  y_offset = 0;
  // Filter Select Button
  if (px > FM_X + x_offset && px < FM_X + x_offset + FM_BOXSIZE_X && py > FM_Y + y_offset && py <  FM_Y + y_offset + FM_BOXSIZE_Y) {
    filterBut = true;
    // circular selection
    if (activeFilter == FM_NONE) {
      activeFilter = FM_ABOVE_HORIZON; // filter disallows alt < 10 deg
      cat_mgr.filterAdd(activeFilter); 
    } else if (activeFilter == FM_ABOVE_HORIZON) {
      activeFilter = FM_ALIGN_ALL_SKY; // Used for stars only here: filter only allows Mag>=3; Alt>=10; Dec<=80
      cat_mgr.filterAdd(activeFilter); 
    } else if (activeFilter == FM_ALIGN_ALL_SKY) {
      activeFilter = FM_NONE;  // no filter
      cat_mgr.filtersClear();
    }
    return;
  }

  y_offset = 0;
  // Clear Custom Button
  y_offset += FM_BOXSIZE_Y + FM_SPACER_Y;
  if (!yesCancelActive && px > MISC_X + x_offset && px < MISC_X + x_offset + MISC_BOXSIZE_X && py > MISC_Y + y_offset && py <  MISC_Y + y_offset + MISC_BOXSIZE_Y) {
    clrCustom = true;
    return;
  }
  // Clearing Custom Catalog "Yes"
  if (px > MISC_X && px < MISC_X + 30 && py > MISC_Y + y_offset && py <  MISC_Y + y_offset + MISC_BOXSIZE_Y) {
    yesBut = true;
    clrCustom = true;
    return;
  }
  // Clearing Custom catalog "Cancel"
  if (px > MISC_X + 40 && px < MISC_X + 120 && py > MISC_Y + y_offset && py <  MISC_Y + y_offset + MISC_BOXSIZE_Y) {
    cancelBut = true;
    clrCustom = true;
    return;
  }

  // Buzzer Button
  y_offset += MISC_BOXSIZE_Y + MISC_SPACER_Y;
  if (px > MISC_X + x_offset && px < MISC_X + x_offset + MISC_BOXSIZE_X && py > MISC_Y + y_offset && py <  MISC_Y + y_offset + MISC_BOXSIZE_Y) {
    if (!soundEnabled) {
      soundEnabled = true; // turn on
    } else {
      soundEnabled = false; // toggle off
    }
    return;
  }

  // **** Go To Target Coordinates ****
  if (py > GOTO_BUT_Y && py < (GOTO_BUT_Y + GOTO_M_BOXSIZE_Y) && px > GOTO_BUT_X && px < (GOTO_BUT_X + GOTO_M_BOXSIZE_X)) {
    goToButton = true;
    display.setLocalCmd(":MS#"); // move to
    return;
  }

  // **** ABORT GOTO ****
  if (py > ABORT_M_BUT_Y && py < (ABORT_M_BUT_Y + GOTO_M_BOXSIZE_Y) && px > ABORT_M_BUT_X && px < (ABORT_M_BUT_X + GOTO_M_BOXSIZE_X)) {
    display.soundFreq(1500, 200);
    abortPgBut = true;
    display.setLocalCmd(":Q#"); // stops move
    motor1.power(false); // do this for safety reasons...mount may be colliding with something
    motor2.power(false);
    return;
  }

  // CATALOG Array Selection Buttons 
  y_offset = 0;
  for (uint16_t i=1; i<=cat_mgr.numCatalogs(); i++) {
    if (px > CAT_SEL_X && px < CAT_SEL_X + CAT_SEL_BOXSIZE_X && py > CAT_SEL_Y+y_offset  && py < CAT_SEL_Y+y_offset + CAT_SEL_BOXSIZE_Y) {
      
      // disable ALL_SKY filter if any DSO catalog...it's for STARS only
      if (i != 1 && activeFilter == FM_ALIGN_ALL_SKY) { // 1 is STARS
        cat_mgr.filtersClear();
        activeFilter = FM_NONE;
        return;
      } 
      catalogScreen.draw(i-1);
      return;
    }
    y_offset += CAT_SEL_SPACER;
  }

  // Planet Catalog Select Button
  if (px > CAT_SEL_X && px < CAT_SEL_X + CAT_SEL_BOXSIZE_X && py > CAT_SEL_Y+y_offset  && py < CAT_SEL_Y+y_offset + CAT_SEL_BOXSIZE_Y) {
    planetsScreen.draw();
    return;
  }

  // Treasure catalog select Button
  y_offset += CAT_SEL_SPACER;
  if (px > CAT_SEL_X && px < CAT_SEL_X + CAT_SEL_BOXSIZE_X && py > CAT_SEL_Y+y_offset  && py < CAT_SEL_Y+y_offset + CAT_SEL_BOXSIZE_Y) {
    catalogScreen.draw(cat_mgr.numCatalogs()+1);
    return;
  }

    // User custom catalog select Button
  y_offset += CAT_SEL_SPACER;
  if (px > CAT_SEL_X && px < CAT_SEL_X + CAT_SEL_BOXSIZE_X && py > CAT_SEL_Y+y_offset  && py < CAT_SEL_Y+y_offset + CAT_SEL_BOXSIZE_Y) {
    catalogScreen.draw(cat_mgr.numCatalogs()+2);
    return;
  }
}

MoreScreen moreScreen;
