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

//#include "../display/display.h"
#include "MoreScreen.h"
#include "CatalogScreen.h"
#include "PlanetsScreen.h"
#include "HomeScreen.h"
#include "../catalog/Catalog.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include <Fonts/FreeSansBold9pt7b.h>
#include "../../../telescope/mount/Mount.h"
#include "src/lib/tasks/OnTask.h"

// Catalog Selection buttons
#define CAT_SEL_X               5
#define CAT_SEL_Y              179
#define CAT_SEL_BOXSIZE_X      110 
#define CAT_SEL_BOXSIZE_Y       28
#define CAT_SEL_SPACER          CAT_SEL_BOXSIZE_Y + 5
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
#define MISC_Y                 162
#define MISC_BOXSIZE_X         100 
#define MISC_BOXSIZE_Y          28
#define MISC_SPACER_Y            8 
#define MISC_TEXT_X_OFFSET       2 
#define MISC_TEXT_Y_OFFSET      18

// Filter Button
#define FM_X                   200
#define FM_Y                   163
#define FM_BOXSIZE_X           120 
#define FM_BOXSIZE_Y            28
#define FM_SPACER_Y              2 
#define FM_TEXT_X_OFFSET         0 
#define FM_TEXT_Y_OFFSET        18

#define GOTO_BUT_X             217
#define GOTO_BUT_Y             264
#define GOTO_TXT_OFF_X          15
#define GOTO_TXT_OFF_Y          25
#define GOTO_M_BOXSIZE_X        90
#define GOTO_M_BOXSIZE_Y        40

#define ABORT_M_BUT_X          218
#define ABORT_M_BUT_Y          310

extern const char *activeFilterStr[3];

// ============= Initialize the Catalog & More page ==================
void MoreScreen::draw() {
  setCurrentScreen(MORE_SCREEN);
  setNightMode(getNightMode());
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);

  tft.setCursor(TRACK_R_X+12, TRACK_R_Y-7);
  tft.print("Rates");

   tft.setCursor(30, TRACK_R_Y-7);
  tft.print("Catalogs");

  drawMenuButtons();
  drawTitle(70, TITLE_TEXT_Y, "Catalogs & Misc");

  // Draw the HOME Icon bitmap
  uint8_t extern black_house_icon[];
  tft.drawBitmap(10, 5, black_house_icon, 39, 31,  butBackground, ORANGE);

  drawCommonStatusLabels();
  updateMoreButtons();
}

// task update for this screen
void MoreScreen::updateMoreStatus() {
  updateCommonStatus();
}

//================== Update the Buttons ======================
void MoreScreen::updateMoreButtons() {
  
  // Show any target object data selected from Catalog
  uint16_t x = 120; uint16_t y = 358; 
  tft.fillRect(x-2, y+3, 199, 5*16, butBackground);
  
  tft.setCursor(x,y+16  ); tft.print(catalogScreen.catSelectionStr1);
  tft.setCursor(x,y+16*2); tft.print(catalogScreen.catSelectionStr2);
  tft.setCursor(x,y+16*3); tft.print(catalogScreen.catSelectionStr3);
  tft.setCursor(x,y+16*4); tft.print(catalogScreen.catSelectionStr4); 
  tft.setCursor(x,y+16*5); tft.print(catalogScreen.catSelectionStr5); 

  int y_offset = 0;
  int x_offset = 0;
  // Manage Tracking Rate buttons
  if (sidereal) {
      drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_ON, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET,   "Sidereal");
      y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
      drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  Lunar "); 
      y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
      drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  King  ");
  }
  if (lunarRate) {
      y_offset = 0;
      drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET,   "Sidereal");
      y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
      drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_ON, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  Lunar "); 
      y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
      drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  King  ");
  }
  if (kingRate) {
      y_offset = 0;
      drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET,   "Sidereal");
      y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
      drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  Lunar "); 
      y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
      drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_ON, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  King  ");
  }   
  // increment tracking rate by 0.02 Hz
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER+TRACK_R_GROUP_SPACER ; // space between tracking setting fields
  if (incTrackRate) {
      drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_ON, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "IncTrack");
      incTrackRate = false;
  } else {
      drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "IncTrack"); 
  }   
  // decrement tracking rate by 0.02 Hz
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER; 
  if (decTrackRate) {
      drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_ON, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "DecTrack");
      decTrackRate = false;
  } else {
      drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "DecTrack"); 
  }   

  // Reset Tracking Rate Sidereal
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER+TRACK_R_GROUP_SPACER ; 
  if (rstTrackRate) {
      drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_ON, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "Reseting");
      rstTrackRate = false;
  } else {
      drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUTTON_OFF, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "RstTrack"); 
  }   

  // Show current Tracking Rate
  // For ALT/AZ this always shows the default rate
  char _sideRate[12];
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER+12;
  char sideRate[10];
  getLocalCmdTrim(":GT#", sideRate);
  sprintf(_sideRate, "TR=%s", sideRate);
  canvPrint(TRACK_R_X-6, TRACK_R_Y+y_offset, 0, 90, 16, _sideRate);

  x_offset = 0;
  y_offset = 0;

  // Filter Selection Button - circular selection of 3 values
  if (filterBut) {
    drawButton(FM_X + x_offset, FM_Y + y_offset, FM_BOXSIZE_X, FM_BOXSIZE_Y, BUTTON_OFF, FM_TEXT_X_OFFSET+8, FM_TEXT_Y_OFFSET,  activeFilterStr[activeFilter]);
    if (activeFilter == FM_ALIGN_ALL_SKY && !objectSelected) {
    canvPrint(120, 382, 0, 199, 16, "All Sky For STARS only");
    }
    filterBut = false;
  }

  // Clear Custom Catalog
    y_offset += FM_BOXSIZE_Y + FM_SPACER_Y;
  if (clrCustom) {
    if (!yesCancelActive) {
      yesCancelActive = true;
      drawButton(MISC_X + x_offset, MISC_Y + y_offset, 30, MISC_BOXSIZE_Y, BUTTON_ON, MISC_TEXT_X_OFFSET, MISC_TEXT_Y_OFFSET,   "Yes");
      drawButton(MISC_X + 40, MISC_Y + y_offset, 60, MISC_BOXSIZE_Y, BUTTON_ON, MISC_TEXT_X_OFFSET, MISC_TEXT_Y_OFFSET,   "Cancel");
      if (!objectSelected) canvPrint(120, 382, 0, 199, 16, "Delete Custom Catalog?!");
    }
    if (yesBut) { // go ahead and clear
      drawButton(MISC_X + x_offset, MISC_Y + y_offset, MISC_BOXSIZE_X, MISC_BOXSIZE_Y, BUTTON_ON, MISC_TEXT_X_OFFSET+8, MISC_TEXT_Y_OFFSET,   " Clearing ");
      
      File rmFile = SD.open("/custom.csv");
        if (rmFile) {
          SD.remove("/custom.csv");
        }
      rmFile.close(); 

      yesBut = false;
      clrCustom = false;
      yesCancelActive = false;
      //tasks.immediate(us_handle); 
    }
    if (cancelBut) {
      cancelBut = false;
      clrCustom = false;
      yesCancelActive = false;
      //tasks.immediate(us_handle); 
      drawButton(MISC_X + x_offset, MISC_Y + y_offset, MISC_BOXSIZE_X, MISC_BOXSIZE_Y, BUTTON_OFF, MISC_TEXT_X_OFFSET+5, MISC_TEXT_Y_OFFSET, "Clr Custom");
      if (objectSelected) tft.fillRect(x, y, 199, 5*16, pgBackground);
    }
  } else {
    drawButton(MISC_X + x_offset, MISC_Y + y_offset, MISC_BOXSIZE_X, MISC_BOXSIZE_Y, BUTTON_OFF, MISC_TEXT_X_OFFSET+5, MISC_TEXT_Y_OFFSET, "Clr Custom");
  }

  // Buzzer Enable Button
  y_offset += MISC_BOXSIZE_Y + MISC_SPACER_Y;
  if (soundEnabled) {
    drawButton(MISC_X + x_offset, MISC_Y + y_offset, MISC_BOXSIZE_X, MISC_BOXSIZE_Y, BUTTON_ON, MISC_TEXT_X_OFFSET+12, MISC_TEXT_Y_OFFSET, "Buzzer On ");
  } else { //off
    drawButton(MISC_X + x_offset, MISC_Y + y_offset, MISC_BOXSIZE_X, MISC_BOXSIZE_Y, BUTTON_OFF, MISC_TEXT_X_OFFSET+12, MISC_TEXT_Y_OFFSET, "Buzzer Off");
  }

  // Larger Button Text for GoTo and Abort
  tft.setFont(&FreeSansBold9pt7b); 

  // Go To Coordinates Button
  if (goToButton) {
    drawButton( GOTO_BUT_X, GOTO_BUT_Y,  GOTO_M_BOXSIZE_X, GOTO_M_BOXSIZE_Y, BUTTON_ON, GOTO_TXT_OFF_X-2, GOTO_TXT_OFF_Y, "Going");
    goToButton = false;
  } else {
    if (!mount.isSlewing()) { 
      drawButton( GOTO_BUT_X, GOTO_BUT_Y,  GOTO_M_BOXSIZE_X, GOTO_M_BOXSIZE_Y, BUTTON_OFF, GOTO_TXT_OFF_X+5, GOTO_TXT_OFF_Y, "GoTo"); 
    } else {
      //tasks.immediate(us_handle); 
    }
  }
  
  // Abort GoTo Button
  if (abortPgBut) {
    drawButton(ABORT_M_BUT_X, ABORT_M_BUT_Y, GOTO_M_BOXSIZE_X, GOTO_M_BOXSIZE_Y, BUTTON_ON, GOTO_TXT_OFF_X-5, GOTO_TXT_OFF_Y, "Aborting"); 
    abortPgBut = false;
  } else {
    drawButton(ABORT_M_BUT_X, ABORT_M_BUT_Y, GOTO_M_BOXSIZE_X, GOTO_M_BOXSIZE_Y, BUTTON_OFF, GOTO_TXT_OFF_X, GOTO_TXT_OFF_Y, " Abort"); 
  }

  tft.setFont(&Inconsolata_Bold8pt7b); // Text back to default

  // Draw the Catalog Buttons
  char title[16]="";
  y_offset = 0;
  for (uint16_t i=1; i<=cat_mgr.numCatalogs(); i++) {
    cat_mgr.select(i-1);
    strcpy(title,cat_mgr.catalogTitle());
    drawButton(CAT_SEL_X, CAT_SEL_Y+y_offset, CAT_SEL_BOXSIZE_X, CAT_SEL_BOXSIZE_Y, BUTTON_OFF, CAT_SEL_TEXT_X_OFFSET, CAT_SEL_TEXT_Y_OFFSET, title);
    y_offset += CAT_SEL_SPACER;
  }

  // Planet Catalog Button
  drawButton(CAT_SEL_X, CAT_SEL_Y+y_offset, CAT_SEL_BOXSIZE_X, CAT_SEL_BOXSIZE_Y, BUTTON_OFF, CAT_SEL_TEXT_X_OFFSET, CAT_SEL_TEXT_Y_OFFSET, "Planets");

  y_offset += CAT_SEL_SPACER;
  // Treasure Catalog
  drawButton(CAT_SEL_X, CAT_SEL_Y+y_offset, CAT_SEL_BOXSIZE_X, CAT_SEL_BOXSIZE_Y, BUTTON_OFF, CAT_SEL_TEXT_X_OFFSET, CAT_SEL_TEXT_Y_OFFSET, "Treasure");

  y_offset += CAT_SEL_SPACER;
  // Custom User Catalog Button
  drawButton(CAT_SEL_X, CAT_SEL_Y+y_offset, CAT_SEL_BOXSIZE_X, CAT_SEL_BOXSIZE_Y, BUTTON_OFF, CAT_SEL_TEXT_X_OFFSET, CAT_SEL_TEXT_Y_OFFSET, "Custom Cat");
}

//==============================================
// ===== TouchScreen Poll "MORE" page ==========
//==============================================
bool MoreScreen::touchPoll(uint16_t px, uint16_t py) {
  char reply[5];

  // Home Page ICON Button
  if (px > 10 && px < 50 && py > 5 && py < 37) {
    BEEP;
    setCurrentScreen(HOME_SCREEN); homeScreen.draw();
    return true;
  }

  // Select Tracking Rates
  int x_offset = 0;
  int y_offset = 0;

  // Sidereal Rate 
  if (py > TRACK_R_Y+y_offset && py < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && px > TRACK_R_X && px < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
    BEEP;
      setLocalCmd(":TQ#");
      sidereal = true;
      lunarRate = false;
      kingRate = false;
      return true;
  }
  // Lunar Rate
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
  if (py > TRACK_R_Y+y_offset && py < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && px > TRACK_R_X && px < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
    BEEP;
      setLocalCmd(":TL#");
      sidereal = false;
      lunarRate = true;
      kingRate = false;
      return true;
  }
  // King Rate
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
  if (py > TRACK_R_Y+y_offset && py < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && px > TRACK_R_X && px < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
    BEEP;
      setLocalCmd(":TK#");
      sidereal = false;
      lunarRate = false;
      kingRate = true;
      return true;
  }
  // Increment tracking rate
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_GROUP_SPACER ;
  if (py > TRACK_R_Y+y_offset && py < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && px > TRACK_R_X && px < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
    BEEP;
      setLocalCmd(":T+#");
      incTrackRate = true;
      decTrackRate = false;
      return true;
  }
  // Decrement tracking rate
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
  if (py > TRACK_R_Y+y_offset && py < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && px > TRACK_R_X && px < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
    BEEP;
      setLocalCmd(":T-#");
      incTrackRate = false;
      decTrackRate = true;
      return true;
  }
  // Reset tracking rate
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_GROUP_SPACER ;
  if (py > TRACK_R_Y+y_offset && py < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && px > TRACK_R_X && px < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
    BEEP;
      setLocalCmd(":TR#");
      rstTrackRate = true;
      return true;
  }

  y_offset = 0;
  // Filter Select Button
  if (px > FM_X + x_offset && px < FM_X + x_offset + FM_BOXSIZE_X && py > FM_Y + y_offset && py <  FM_Y + y_offset + FM_BOXSIZE_Y) {
    BEEP;
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
    return true;
  }

  y_offset = 0;
  // Clear Custom Button
  y_offset += FM_BOXSIZE_Y + FM_SPACER_Y;
  if (!yesCancelActive && px > MISC_X + x_offset && px < MISC_X + x_offset + MISC_BOXSIZE_X && py > MISC_Y + y_offset && py <  MISC_Y + y_offset + MISC_BOXSIZE_Y) {
    BEEP;
    clrCustom = true;
    return true;
  }
  // Clearing Custom Catalog "Yes"
  if (px > MISC_X && px < MISC_X + 30 && py > MISC_Y + y_offset && py <  MISC_Y + y_offset + MISC_BOXSIZE_Y) {
    BEEP;
    yesBut = true;
    clrCustom = true;
    return true;
  }
  // Clearing Custom catalog "Cancel"
  if (px > MISC_X + 40 && px < MISC_X + 120 && py > MISC_Y + y_offset && py <  MISC_Y + y_offset + MISC_BOXSIZE_Y) {
    BEEP;
    cancelBut = true;
    clrCustom = true;
    return true;
  }

  // Buzzer Button
  y_offset += MISC_BOXSIZE_Y + MISC_SPACER_Y;
  if (px > MISC_X + x_offset && px < MISC_X + x_offset + MISC_BOXSIZE_X && py > MISC_Y + y_offset && py <  MISC_Y + y_offset + MISC_BOXSIZE_Y) {
    BEEP;
    if (!soundEnabled) {
      soundEnabled = true; // turn on
      setLocalCmd(":SX97,1#");
    } else {
      soundEnabled = false; // toggle off
      setLocalCmd(":SX97,0#");
    }
    return true;
  }
  
  // **** Go To Target Coordinates ****
  if (py > GOTO_BUT_Y && py < (GOTO_BUT_Y + GOTO_M_BOXSIZE_Y) && px > GOTO_BUT_X && px < (GOTO_BUT_X + GOTO_M_BOXSIZE_X)) {
    BEEP;
    goToButton = true;
    getLocalCmdTrim(":MS#", reply);
    VF("reply="); VL(reply);
  return true;
  }

  // **** ABORT GOTO ****
  if (py > ABORT_M_BUT_Y && py < (ABORT_M_BUT_Y + GOTO_M_BOXSIZE_Y) && px > ABORT_M_BUT_X && px < (ABORT_M_BUT_X + GOTO_M_BOXSIZE_X)) {
    BEEP;
    soundFreq(1500, 200);
    abortPgBut = true;
    setLocalCmd(":Q#"); // stops move
    motor1.power(false); // do this for safety reasons...mount may be colliding with something
    motor2.power(false);
    return true;
  }

  // CATALOG Array Selection Buttons 
  y_offset = 0;
  for (uint16_t i=1; i<=cat_mgr.numCatalogs(); i++) {
    if (px > CAT_SEL_X && px < CAT_SEL_X + CAT_SEL_BOXSIZE_X && py > CAT_SEL_Y+y_offset  && py < CAT_SEL_Y+y_offset + CAT_SEL_BOXSIZE_Y) {
      BEEP;
      
      // disable ALL_SKY filter if any DSO catalog...it's for STARS only
      if (i != 1 && activeFilter == FM_ALIGN_ALL_SKY) { // 1 is STARS
        cat_mgr.filtersClear();
        activeFilter = FM_NONE;
        return true;
      } 
      catalogScreen.draw(i-1);
      return true;
    }
    y_offset += CAT_SEL_SPACER;
  }

  // Planet Catalog Select Button
  if (px > CAT_SEL_X && px < CAT_SEL_X + CAT_SEL_BOXSIZE_X && py > CAT_SEL_Y+y_offset  && py < CAT_SEL_Y+y_offset + CAT_SEL_BOXSIZE_Y) {
    BEEP;
    planetsScreen.draw();
    return true;
  }

  // Treasure catalog select Button
  y_offset += CAT_SEL_SPACER;
  if (px > CAT_SEL_X && px < CAT_SEL_X + CAT_SEL_BOXSIZE_X && py > CAT_SEL_Y+y_offset  && py < CAT_SEL_Y+y_offset + CAT_SEL_BOXSIZE_Y) {
    BEEP;
    catalogScreen.draw(cat_mgr.numCatalogs()+1);
    return true;
  }

    // User custom catalog select Button
  y_offset += CAT_SEL_SPACER;
  if (px > CAT_SEL_X && px < CAT_SEL_X + CAT_SEL_BOXSIZE_X && py > CAT_SEL_Y+y_offset  && py < CAT_SEL_Y+y_offset + CAT_SEL_BOXSIZE_Y) {
    BEEP;
    catalogScreen.draw(cat_mgr.numCatalogs()+2);
    return true;
  }
  return false;
}

MoreScreen moreScreen;
