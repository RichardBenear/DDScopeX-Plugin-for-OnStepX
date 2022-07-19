
// =====================================================
// GuideScreen.cpp

// Author: Richard Benear
// initial 8/22/21 -- refactor 5/22

#include "GuideScreen.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "../fonts/UbuntuMono_Bold11pt7b.h"
#include "src/telescope/mount/Mount.h"
#include "src/telescope/mount/guide/Guide.h"

// Guide buttons
#define GUIDE_BOXSIZE_X          85
#define GUIDE_BOXSIZE_Y          65 
#define SYNC_OFFSET_X           113 
#define SYNC_OFFSET_Y           280 
#define LEFT_OFFSET_X             7 
#define LEFT_OFFSET_Y           SYNC_OFFSET_Y
#define RIGHT_OFFSET_X          219
#define RIGHT_OFFSET_Y          SYNC_OFFSET_Y
#define UP_OFFSET_X             SYNC_OFFSET_X
#define UP_OFFSET_Y             200 
#define DOWN_OFFSET_X           SYNC_OFFSET_X
#define DOWN_OFFSET_Y           360 

// Guide rates buttons
#define GUIDE_R_X                3
#define GUIDE_R_Y              165
#define GUIDE_R_BOXSIZE_X       74 
#define GUIDE_R_BOXSIZE_Y       28
#define GUIDE_R_SPACER           6 

// Stop guide button
#define STOP_X                    7
#define STOP_Y                  385 
#define STOP_BOXSIZE_X           86 
#define STOP_BOXSIZE_Y           40 

// Spiral Search button
#define SPIRAL_X                220
#define SPIRAL_Y                385 
#define SPIRAL_BOXSIZE_X         86 
#define SPIRAL_BOXSIZE_Y         40 

// Guide Screen Button object
Button guideButton(
                GUIDE_R_X, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y,
                butOnBackground, 
                butBackground, 
                butOutline, 
                mainFontWidth, 
                mainFontHeight, 
                "");
                
// Guide Screen Large Button object
Button guideLargeButton(
                GUIDE_R_X, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y,
                butOnBackground, 
                butBackground, 
                butOutline, 
                largeFontWidth, 
                largeFontHeight, 
                "");

// Draw the GUIDE Page
void GuideScreen::draw() { 
  setCurrentScreen(GUIDE_SCREEN);
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  drawMenuButtons();
  drawTitle(110, TITLE_TEXT_Y, "Guiding");
  tft.setFont(&Inconsolata_Bold8pt7b);

  drawCommonStatusLabels();
  updateGuideButtons(false);
}

// task update for this screen
void GuideScreen::updateGuideStatus() {
  updateCommonStatus(); // only this update for now
}

bool GuideScreen::guideButStateChange() {
  if (preSlewState != mount.isSlewing()) {
    preSlewState = mount.isSlewing(); 
    return true;
  } else if (display._redrawBut) {
    display._redrawBut = false;
    return true;
  } else { 
    return false;
  }
}

// ========== Update Guide Page Buttons ==========
void GuideScreen::updateGuideButtons(bool redrawBut) {
  _redrawBut = redrawBut;
   tft.setFont(&UbuntuMono_Bold11pt7b); 

  if (guidingEast && mount.isSlewing()) { 
    guideLargeButton.draw(RIGHT_OFFSET_X, RIGHT_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, "EAST", BUT_ON);
  } else {
    guideLargeButton.draw(RIGHT_OFFSET_X, RIGHT_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, "EAST", BUT_OFF);
  }

  if (guidingWest && mount.isSlewing()) { 
    guideLargeButton.draw(LEFT_OFFSET_X, LEFT_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, "WEST", BUT_ON);
  } else {
    guideLargeButton.draw(LEFT_OFFSET_X, LEFT_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, "WEST", BUT_OFF);
  }

 if (guidingNorth && mount.isSlewing()) { 
    guideLargeButton.draw(UP_OFFSET_X, UP_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, "NORTH", BUT_ON);
  } else {
    guideLargeButton.draw(UP_OFFSET_X, UP_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, "NORTH", BUT_OFF);
  }

  if (guidingSouth && mount.isSlewing()) { 
    guideLargeButton.draw(DOWN_OFFSET_X, DOWN_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, "SOUTH", BUT_ON);
  } else {
    guideLargeButton.draw(DOWN_OFFSET_X, DOWN_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, "SOUTH", BUT_OFF);
  }
  
  if (!syncOn) {
    guideLargeButton.draw(SYNC_OFFSET_X, SYNC_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, "SYNC", BUT_OFF);
  } else {
    guideLargeButton.draw(SYNC_OFFSET_X, SYNC_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, "SYNCng", BUT_ON);
    syncOn = false;
  }
  tft.setFont(&Inconsolata_Bold8pt7b); 

  // Draw Guide Rates Buttons
  // :RG#       Set guide rate: Guiding        1X
  // :RC#       Set guide rate: Centering      8X
  // :RM#       Set guide rate: Find          20X
  // :RF#       Set guide rate: Fast          48X
  // :RS#       Set guide rate: Slew           ?X (1/2 of current goto rate)
  // :Rn#       Set guide rate to n, where n = 0..9
  char guideRateText[10];
  switch (guide.settings.axis1RateSelect) {
    case 0:  strcpy(guideRateText, " Quarter"); break;
    case 1:  strcpy(guideRateText, "  Half  "); break;
    case 2:  strcpy(guideRateText, "    1X  "); break;
    case 3:  strcpy(guideRateText, "    2X  "); break;
    case 4:  strcpy(guideRateText, "    4X  "); break;
    case 5:  strcpy(guideRateText, "    8X  "); break;
    case 6:  strcpy(guideRateText, "   20X  "); break;
    case 7:  strcpy(guideRateText, "   48X  "); break;
    case 8:  strcpy(guideRateText, "Half Max"); break;
    case 9:  strcpy(guideRateText, "   Max  "); break;
    case 10: strcpy(guideRateText, " Custom "); break;
    default: strcpy(guideRateText, "  Error "); break;
  }
  int x_offset = 0;
  int spacer = GUIDE_R_SPACER;
  if (oneXisOn) {   
    guideButton.draw(GUIDE_R_X+x_offset, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, "Guide", BUT_ON);
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    guideButton.draw(GUIDE_R_X+x_offset, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, "Find", BUT_OFF);
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    guideButton.draw(GUIDE_R_X+x_offset, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, "Fast", BUT_OFF);
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    guideButton.draw(GUIDE_R_X+x_offset, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, "Half Max", BUT_OFF); 
  } 

  if (eightXisOn) {   
      guideButton.draw(GUIDE_R_X+x_offset, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, "Guide", BUT_OFF);
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    guideButton.draw(GUIDE_R_X+x_offset, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, "Find", BUT_ON);
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    guideButton.draw(GUIDE_R_X+x_offset, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, "Fast", BUT_OFF);
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    guideButton.draw(GUIDE_R_X+x_offset, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, "Half Max", BUT_OFF); 
  }   

  if (twentyXisOn) {
      guideButton.draw(GUIDE_R_X+x_offset, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, "Guide", BUT_OFF);
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    guideButton.draw(GUIDE_R_X+x_offset, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, "Find", BUT_OFF);
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    guideButton.draw(GUIDE_R_X+x_offset, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, "Fast", BUT_ON);
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    guideButton.draw(GUIDE_R_X+x_offset, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, "Half Max", BUT_OFF); 
  }

  if (HalfMaxisOn) {
    guideButton.draw(GUIDE_R_X+x_offset, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, "Guide", BUT_OFF);
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    guideButton.draw(GUIDE_R_X+x_offset, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, "Find", BUT_OFF);
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    guideButton.draw(GUIDE_R_X+x_offset, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, "Fast", BUT_OFF);
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    guideButton.draw(GUIDE_R_X+x_offset, GUIDE_R_Y, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, "Half Max", BUT_ON); 
  }        

  if (spiralOn) {  
    guideButton.draw(SPIRAL_X, SPIRAL_Y, SPIRAL_BOXSIZE_X, SPIRAL_BOXSIZE_Y, "Spiral On", BUT_ON);    
  } else {
    guideButton.draw(SPIRAL_X, SPIRAL_Y, SPIRAL_BOXSIZE_X, SPIRAL_BOXSIZE_Y, "Spiral Off", BUT_OFF); 
  }    

  if (stopPressed) {  
    guideButton.draw(STOP_X, STOP_Y, STOP_BOXSIZE_X, STOP_BOXSIZE_Y, "Stoppped", BUT_ON); 
    stopPressed = false;
  } else {
    guideButton.draw(STOP_X, STOP_Y, STOP_BOXSIZE_X, STOP_BOXSIZE_Y, "STOP", BUT_OFF);
  } 
}

// Manage Touching of Guiding Buttons
bool GuideScreen::touchPoll(uint16_t px, uint16_t py) {
    // SYNC Button 
    if (py > SYNC_OFFSET_Y && py < (SYNC_OFFSET_Y + GUIDE_BOXSIZE_Y) && px > SYNC_OFFSET_X && px < (SYNC_OFFSET_X + GUIDE_BOXSIZE_X)) { 
      BEEP;           
        setLocalCmd(":CS#"); // doesn't have reply
        syncOn = true;
        return true;  
    }
                    
    // EAST / RIGHT button
    if (py > RIGHT_OFFSET_Y && py < (RIGHT_OFFSET_Y + GUIDE_BOXSIZE_Y) && px > RIGHT_OFFSET_X && px < (RIGHT_OFFSET_X + GUIDE_BOXSIZE_X)) {
      BEEP;
        if (!guidingEast) {
            setLocalCmd(":Mw#"); // east west is swapped for DDScope
            guidingEast = true;
            guidingWest = false;
            guidingNorth = false;
            guidingSouth = false;
        } else {
            setLocalCmd(":Qw#");
            guidingEast = false;
        }
        return true;
    }
                    
    // WEST / LEFT button
    if (py > LEFT_OFFSET_Y && py < (LEFT_OFFSET_Y + GUIDE_BOXSIZE_Y) && px > LEFT_OFFSET_X && px < (LEFT_OFFSET_X + GUIDE_BOXSIZE_X)) {
      BEEP;
        if (!guidingWest) {
            setLocalCmd(":Me#"); // east west is swapped for DDScope
            guidingEast = false;
            guidingWest = true;
            guidingNorth = false;
            guidingSouth = false;
        } else {
            setLocalCmd(":Qe#");
            guidingWest = false;
        }
        return true;
    }
                    
    // NORTH / UP button
    if (py > UP_OFFSET_Y && py < (UP_OFFSET_Y + GUIDE_BOXSIZE_Y) && px > UP_OFFSET_X && px < (UP_OFFSET_X + GUIDE_BOXSIZE_X)) {
      BEEP;
        if (!guidingNorth) {
            setLocalCmd(":Mn#");
            guidingEast = false;
            guidingWest = false;
            guidingNorth = true;
            guidingSouth = false;
        } else {
            setLocalCmd(":Qn#");
            guidingNorth = false;
        }
        return true;
    }
                    
    // SOUTH / DOWN button
    if (py > DOWN_OFFSET_Y && py < (DOWN_OFFSET_Y + GUIDE_BOXSIZE_Y) && px > DOWN_OFFSET_X && px < (DOWN_OFFSET_X + GUIDE_BOXSIZE_X)) {
      BEEP;
        if (!guidingSouth) {
            setLocalCmd(":Ms#");
            guidingEast = false;
            guidingWest = false;
            guidingNorth = false;
            guidingSouth = true;
        } else {
            setLocalCmd(":Qs#");
            guidingSouth = false;
        }
        return true;
    }

    // :RG#       Set guide rate: Guiding        1X
    // :RC#       Set guide rate: Centering      8X
    // :RM#       Set guide rate: Find          20X
    // :RF#       Set guide rate: Fast          48X
    // :RS#       Set guide rate: Slew           ?X (1/2 of current goto rate)
    // :Rn#       Set guide rate to n, where n = 0..9
    // Select Guide Rates
    int y_offset = 0;
    int x_offset = 0;  
    int spacer = GUIDE_R_SPACER;
    
    // 1x Guide Rate 
    if (py > GUIDE_R_Y+y_offset && py < (GUIDE_R_Y+y_offset + GUIDE_R_BOXSIZE_Y) && px > GUIDE_R_X+x_offset && px < (GUIDE_R_X+x_offset + GUIDE_R_BOXSIZE_X)) {
      BEEP;
        setLocalCmd(":RG#");
        oneXisOn = true;
        eightXisOn = false;
        twentyXisOn = false;
        HalfMaxisOn = false;
        return true;
    }

    // 20x Rate
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    if (py > GUIDE_R_Y+y_offset && py < (GUIDE_R_Y+y_offset + GUIDE_R_BOXSIZE_Y) && px > GUIDE_R_X+x_offset && px < (GUIDE_R_X+x_offset + GUIDE_R_BOXSIZE_X)) {
      BEEP;
        setLocalCmd(":RM#");
        oneXisOn = false;
        eightXisOn = true;
        twentyXisOn = false;
        HalfMaxisOn = false;
        return true;
    }

    // 48x Rate for Moving
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    if (py > GUIDE_R_Y+y_offset && py < (GUIDE_R_Y+y_offset + GUIDE_R_BOXSIZE_Y) && px > GUIDE_R_X+x_offset && px < (GUIDE_R_X+x_offset + GUIDE_R_BOXSIZE_X)) {
      BEEP;
        setLocalCmd(":RF#");
        oneXisOn = false;
        eightXisOn = false;
        twentyXisOn = true;
        HalfMaxisOn = false;
        return true;
    }

    // Half Max Rate for Slewing
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    if (py > GUIDE_R_Y+y_offset && py < (GUIDE_R_Y+y_offset + GUIDE_R_BOXSIZE_Y) && px > GUIDE_R_X+x_offset && px < (GUIDE_R_X+x_offset + GUIDE_R_BOXSIZE_X)) {
      BEEP;
        setLocalCmd(":RS#");
        oneXisOn = false;
        eightXisOn = false;
        twentyXisOn = false;
        HalfMaxisOn = true;
        return true;
    }

    // Spiral Search
    if (py > SPIRAL_Y && py < (SPIRAL_Y + SPIRAL_BOXSIZE_Y) && px > SPIRAL_X && px < (SPIRAL_X + SPIRAL_BOXSIZE_X)) {
      BEEP;
        if (!spiralOn) {
            setLocalCmd(":Mp#");
            spiralOn = true;
        } else {
            setLocalCmd(":Q#"); // stop moves
            spiralOn = false;
        }
        return true;
    }
    
    // STOP moving
    if (py > STOP_Y && py < (STOP_Y + STOP_BOXSIZE_Y) && px > STOP_X && px < (STOP_X + STOP_BOXSIZE_X)) {
      BEEP;
        setLocalCmd(":Q#");
        stopPressed = true;
        spiralOn = false;
        guidingEast = false;
        guidingWest = false;
        guidingNorth = false;
        guidingSouth = false;
        return true;
    }
    return false;
}

GuideScreen guideScreen;
