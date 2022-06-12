
// =====================================================
// GuideScreen.cpp

// Author: Richard Benear
// 8/22/21 -- refactor 5/1/22

#include "GuideScreen.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include <Fonts/FreeSansBold9pt7b.h>

// Guide buttons
#define GUIDE_BOXSIZE_X          85
#define GUIDE_BOXSIZE_Y          65 
#define GUIDE_TEXT_X_OFFSET       8
#define GUIDE_TEXT_Y_OFFSET      38 
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
#define GUIDE_R_Y              162
#define GUIDE_R_BOXSIZE_X       74 
#define GUIDE_R_BOXSIZE_Y       28
#define GUIDE_R_SPACER           6 
#define GUIDE_R_TEXT_X_OFFSET    2 
#define GUIDE_R_TEXT_Y_OFFSET   17

// Stop guide button
#define STOP_X                    7
#define STOP_Y                  385 
#define STOP_BOXSIZE_X           86 
#define STOP_BOXSIZE_Y           40 
#define STOP_TEXT_X_OFFSET        4
#define STOP_TEXT_Y_OFFSET       25

// Spiral Search button
#define SPIRAL_X                220
#define SPIRAL_Y                385 
#define SPIRAL_BOXSIZE_X         86 
#define SPIRAL_BOXSIZE_Y         40 
#define SPIRAL_TEXT_X_OFFSET      2
#define SPIRAL_TEXT_Y_OFFSET     25

// Draw the GUIDE Page
void GuideScreen::draw() { 
  currentScreen = GUIDE_SCREEN;
  setDayNight();
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  drawMenuButtons();
  drawTitle(110, TITLE_TEXT_Y, "Guiding");
  drawCommonStatusLabels();
  tft.setFont(&Inconsolata_Bold8pt7b);
}

// ========== Update Guide Page Status ==========
void GuideScreen::updateThisStatus() {

    // Update for buttons only if the screen is touched
    if (screenTouched || firstDraw || refreshScreen) {
        refreshScreen = false;
        if (screenTouched) refreshScreen = true;
        
        // update current status of guide buttons
        //tft.setFont(&UbuntuMono_Bold11pt7b);
        tft.setFont(&FreeSansBold9pt7b);

        if (guidingEast || lCmountStatus.isSlewing()) { 
            drawButton(RIGHT_OFFSET_X, RIGHT_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUTTON_ON, GUIDE_TEXT_X_OFFSET+5, GUIDE_TEXT_Y_OFFSET, " EAST");
        } else {
            drawButton(RIGHT_OFFSET_X, RIGHT_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUTTON_OFF, GUIDE_TEXT_X_OFFSET+5, GUIDE_TEXT_Y_OFFSET, " EAST");
        }

        if (guidingWest || lCmountStatus.isSlewing()) { 
            drawButton(LEFT_OFFSET_X, LEFT_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUTTON_ON, GUIDE_TEXT_X_OFFSET+5, GUIDE_TEXT_Y_OFFSET, " WEST");
        } else {
            drawButton(LEFT_OFFSET_X, LEFT_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUTTON_OFF, GUIDE_TEXT_X_OFFSET+5, GUIDE_TEXT_Y_OFFSET, " WEST");
        }

        if (guidingNorth || lCmountStatus.isSlewing()) {
            drawButton(UP_OFFSET_X, UP_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUTTON_ON, GUIDE_TEXT_X_OFFSET+13, GUIDE_TEXT_Y_OFFSET, "  UP ");
        } else {
            drawButton(UP_OFFSET_X, UP_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUTTON_OFF, GUIDE_TEXT_X_OFFSET+13, GUIDE_TEXT_Y_OFFSET, "  UP ");
        }

        if (guidingSouth || lCmountStatus.isSlewing()) {
            drawButton(DOWN_OFFSET_X, DOWN_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUTTON_ON, GUIDE_TEXT_X_OFFSET, GUIDE_TEXT_Y_OFFSET, " DOWN");
        } else {
            drawButton(DOWN_OFFSET_X, DOWN_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUTTON_OFF, GUIDE_TEXT_X_OFFSET, GUIDE_TEXT_Y_OFFSET, " DOWN");
        }
        
        if (!syncOn) {
            drawButton(SYNC_OFFSET_X, SYNC_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUTTON_OFF, GUIDE_TEXT_X_OFFSET+5, GUIDE_TEXT_Y_OFFSET, " SYNC  ");
        } else {
            drawButton(SYNC_OFFSET_X, SYNC_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUTTON_ON, GUIDE_TEXT_X_OFFSET, GUIDE_TEXT_Y_OFFSET, "SYNCng");
            syncOn = false;
        }
        tft.setFont(&Inconsolata_Bold8pt7b); 

        // Draw Guide Rates Buttons
        int y_offset = 0;
        int x_offset = 0;
        int spacer = GUIDE_R_SPACER;
        if (oneXisOn) {   
            drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUTTON_ON, GUIDE_R_TEXT_X_OFFSET+4, GUIDE_R_TEXT_Y_OFFSET,     " Guide  ");
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUTTON_OFF, GUIDE_R_TEXT_X_OFFSET+2, GUIDE_R_TEXT_Y_OFFSET, " Center "); 
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUTTON_OFF, GUIDE_R_TEXT_X_OFFSET, GUIDE_R_TEXT_Y_OFFSET,   "  Find  ");
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUTTON_OFF, GUIDE_R_TEXT_X_OFFSET, GUIDE_R_TEXT_Y_OFFSET,   "Half Max"); 
        } 

        if (eightXisOn) {   
            x_offset = 0;
            drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUTTON_OFF, GUIDE_R_TEXT_X_OFFSET+4, GUIDE_R_TEXT_Y_OFFSET, " Guide  ");
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUTTON_ON, GUIDE_R_TEXT_X_OFFSET+2, GUIDE_R_TEXT_Y_OFFSET,     " Center "); 
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUTTON_OFF, GUIDE_R_TEXT_X_OFFSET, GUIDE_R_TEXT_Y_OFFSET,   "  Find  ");
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUTTON_OFF, GUIDE_R_TEXT_X_OFFSET, GUIDE_R_TEXT_Y_OFFSET,   "Half Max");
        }   

        if (twentyXisOn) {
            x_offset = 0;
            drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUTTON_OFF, GUIDE_R_TEXT_X_OFFSET+4, GUIDE_R_TEXT_Y_OFFSET, " Guide  ");
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUTTON_OFF, GUIDE_R_TEXT_X_OFFSET+2, GUIDE_R_TEXT_Y_OFFSET, " Center "); 
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUTTON_ON, GUIDE_R_TEXT_X_OFFSET, GUIDE_R_TEXT_Y_OFFSET,       "  Find  ");
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer; 
            drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUTTON_OFF, GUIDE_R_TEXT_X_OFFSET, GUIDE_R_TEXT_Y_OFFSET,   "Half Max");
        }

        if (HalfMaxisOn) {
            x_offset = 0;
            drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUTTON_OFF, GUIDE_R_TEXT_X_OFFSET+4, GUIDE_R_TEXT_Y_OFFSET, " Guide  ");
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUTTON_OFF, GUIDE_R_TEXT_X_OFFSET+2, GUIDE_R_TEXT_Y_OFFSET, " Center "); 
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUTTON_OFF, GUIDE_R_TEXT_X_OFFSET, GUIDE_R_TEXT_Y_OFFSET,   "  Find  ");
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer; 
            drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUTTON_ON, GUIDE_R_TEXT_X_OFFSET, GUIDE_R_TEXT_Y_OFFSET,       "Half Max");
        }        

        if (spiralOn) {  
            drawButton(SPIRAL_X, SPIRAL_Y, SPIRAL_BOXSIZE_X, SPIRAL_BOXSIZE_Y, BUTTON_ON, SPIRAL_TEXT_X_OFFSET+2, SPIRAL_TEXT_Y_OFFSET, "Spiral On");    
        } else {
            drawButton(SPIRAL_X, SPIRAL_Y, SPIRAL_BOXSIZE_X, SPIRAL_BOXSIZE_Y, BUTTON_OFF, SPIRAL_TEXT_X_OFFSET, SPIRAL_TEXT_Y_OFFSET, "Spiral Off");
        }    

        if (stopPressed) {  
            drawButton(STOP_X, STOP_Y, STOP_BOXSIZE_X, STOP_BOXSIZE_Y, BUTTON_ON, STOP_TEXT_X_OFFSET, STOP_TEXT_Y_OFFSET,   "Stopping"); 
            stopPressed = false;
        } else {
            drawButton(STOP_X, STOP_Y, STOP_BOXSIZE_X, STOP_BOXSIZE_Y, BUTTON_OFF, STOP_TEXT_X_OFFSET+4, STOP_TEXT_Y_OFFSET, "  STOP  ");
        } 
    screenTouched = false;
    }  
}

// Manage Touching of Guiding Buttons
void GuideScreen::touchPoll(uint16_t px, uint16_t py) {
    // SYNC Button 
    if (py > SYNC_OFFSET_Y && py < (SYNC_OFFSET_Y + GUIDE_BOXSIZE_Y) && px > SYNC_OFFSET_X && px < (SYNC_OFFSET_X + GUIDE_BOXSIZE_X)) { 
      DD_CLICK;           
        setLocalCmd(":CS#"); // doesn't have reply
        syncOn = true;
        return;  
    }
                    
    // EAST / RIGHT button
    if (py > RIGHT_OFFSET_Y && py < (RIGHT_OFFSET_Y + GUIDE_BOXSIZE_Y) && px > RIGHT_OFFSET_X && px < (RIGHT_OFFSET_X + GUIDE_BOXSIZE_X)) {
      DD_CLICK;
        if (!guidingEast) {
            setLocalCmd(":Mw#"); // east west is swapped for DDScope
            guidingEast = true;
        } else {
            setLocalCmd(":Qw#");
            guidingEast = false;
        }
        return;
    }
                    
    // WEST / LEFT button
    if (py > LEFT_OFFSET_Y && py < (LEFT_OFFSET_Y + GUIDE_BOXSIZE_Y) && px > LEFT_OFFSET_X && px < (LEFT_OFFSET_X + GUIDE_BOXSIZE_X)) {
      DD_CLICK;
        if (!guidingWest) {
            setLocalCmd(":Me#"); // east west is swapped for DDScope
            guidingWest = true;
        } else {
            setLocalCmd(":Qe#");
            guidingWest = false;
        }
        return;
    }
                    
    // NORTH / UP button
    if (py > UP_OFFSET_Y && py < (UP_OFFSET_Y + GUIDE_BOXSIZE_Y) && px > UP_OFFSET_X && px < (UP_OFFSET_X + GUIDE_BOXSIZE_X)) {
      DD_CLICK;
        if (!guidingNorth) {
            setLocalCmd(":Mn#");
            guidingNorth = true;
        } else {
            setLocalCmd(":Qn#");
            guidingNorth = false;
        }
        return;
    }
                    
    // SOUTH / DOWN button
    if (py > DOWN_OFFSET_Y && py < (DOWN_OFFSET_Y + GUIDE_BOXSIZE_Y) && px > DOWN_OFFSET_X && px < (DOWN_OFFSET_X + GUIDE_BOXSIZE_X)) {
      DD_CLICK;
        if (!guidingSouth) {
            setLocalCmd(":Ms#");
            guidingSouth = true;
        } else {
            setLocalCmd(":Qs#");
            guidingSouth = false;
        }
        return;
    }

    // Select Guide Rates
    int y_offset = 0;
    int x_offset = 0;  
    int spacer = GUIDE_R_SPACER;
    
    // 1x Guide Rate 
    if (py > GUIDE_R_Y+y_offset && py < (GUIDE_R_Y+y_offset + GUIDE_R_BOXSIZE_Y) && px > GUIDE_R_X+x_offset && px < (GUIDE_R_X+x_offset + GUIDE_R_BOXSIZE_X)) {
      DD_CLICK;
        setLocalCmd(":RG#");
        oneXisOn = true;
        eightXisOn = false;
        twentyXisOn = false;
        HalfMaxisOn = false;
        return;
    }

    // 8x Rate for Centering
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    if (py > GUIDE_R_Y+y_offset && py < (GUIDE_R_Y+y_offset + GUIDE_R_BOXSIZE_Y) && px > GUIDE_R_X+x_offset && px < (GUIDE_R_X+x_offset + GUIDE_R_BOXSIZE_X)) {
      DD_CLICK;
        setLocalCmd(":RC#");
        oneXisOn = false;
        eightXisOn = true;
        twentyXisOn = false;
        HalfMaxisOn = false;
        return;
    }

    // 24x Rate for Moving
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    if (py > GUIDE_R_Y+y_offset && py < (GUIDE_R_Y+y_offset + GUIDE_R_BOXSIZE_Y) && px > GUIDE_R_X+x_offset && px < (GUIDE_R_X+x_offset + GUIDE_R_BOXSIZE_X)) {
      DD_CLICK;
        setLocalCmd(":RM#");
        oneXisOn = false;
        eightXisOn = false;
        twentyXisOn = true;
        HalfMaxisOn = false;
        return;
    }

    // Half Max Rate for Slewing
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    if (py > GUIDE_R_Y+y_offset && py < (GUIDE_R_Y+y_offset + GUIDE_R_BOXSIZE_Y) && px > GUIDE_R_X+x_offset && px < (GUIDE_R_X+x_offset + GUIDE_R_BOXSIZE_X)) {
      DD_CLICK;
        setLocalCmd(":RS#");
        oneXisOn = false;
        eightXisOn = false;
        twentyXisOn = false;
        HalfMaxisOn = true;
        return;
    }

    // Spiral Search
    if (py > SPIRAL_Y && py < (SPIRAL_Y + SPIRAL_BOXSIZE_Y) && px > SPIRAL_X && px < (SPIRAL_X + SPIRAL_BOXSIZE_X)) {
      DD_CLICK;
        if (!spiralOn) {
            setLocalCmd(":Mp#");
            spiralOn = true;
        } else {
            setLocalCmd(":Q#"); // stop moves
            spiralOn = false;
        }
        return;
    }
    
    // STOP moving
    if (py > STOP_Y && py < (STOP_Y + STOP_BOXSIZE_Y) && px > STOP_X && px < (STOP_X + STOP_BOXSIZE_X)) {
      DD_CLICK;
        setLocalCmd(":Q#");
        stopPressed = true;
        spiralOn = false;
        guidingEast = false;
        guidingWest = false;
        guidingNorth = false;
        guidingSouth = false;
        return;
    }
}

GuideScreen guideScreen;
