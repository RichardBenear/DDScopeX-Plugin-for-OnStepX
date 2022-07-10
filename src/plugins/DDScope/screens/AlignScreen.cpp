// =====================================================
// AlignScreen.cpp
//
// Author: Richard Benear 6/22

#include "AlignScreen.h"
#include "MoreScreen.h"
#include "SHCCatScreen.h"
#include "../catalog/Catalog.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "src/telescope/mount/Mount.h"
#include "src/telescope/mount/goto/Goto.h"
#include "src/telescope/mount/guide/Guide.h"
#include "src/lib/tasks/OnTask.h"

#define BIG_BOX_W           80
#define BIG_BOX_H           40
#define A_STATUS_X          115
#define A_STATUS_Y          245
#define A_STATUS_Y_SP       19
#define STATE_UPDATE_W      220
#define STATE_UPDATE_H      15

#define STATE_LABEL_X       2
#define STATE_LABEL_Y       470
#define STATUS_LABEL_X      95
#define STATUS_LABEL_Y      176
#define STATUS_LABEL_W      317
#define STATUS_LABEL_H      15

// Go to Home position button
#define HOME_X              7
#define HOME_Y              175
#define HOME_BOXSIZE_W      BIG_BOX_W
#define HOME_BOXSIZE_H      BIG_BOX_H 

// Align Num Stars Button(s)
#define NUM_S_X             1
#define NUM_S_Y             HOME_Y+HOME_BOXSIZE_H+5
#define NUM_S_BOXSIZE_W     30
#define NUM_S_BOXSIZE_H     35
#define NUM_S_SPACING_X     NUM_S_BOXSIZE_W+3 

// Go to Catalog Page Button
#define ALIGN_CAT_X         HOME_X
#define ALIGN_CAT_Y         NUM_S_Y+NUM_S_BOXSIZE_H+5
#define CAT_BOXSIZE_W       BIG_BOX_W
#define CAT_BOXSIZE_H       BIG_BOX_H 

// GO TO Target Button
#define GOTO_X              HOME_X
#define GOTO_Y              ALIGN_CAT_Y+CAT_BOXSIZE_H+5
#define GOTO_BOXSIZE_W      BIG_BOX_W
#define GOTO_BOXSIZE_H      BIG_BOX_H 

// ABORT Button
#define ABORT_X             115
#define ABORT_Y             WRITE_ALIGN_Y

// ALIGN/SYNC this Star Button
#define ALIGN_X             HOME_X
#define ALIGN_Y             GOTO_Y+GOTO_BOXSIZE_H+5
#define ALIGN_BOXSIZE_W     BIG_BOX_W
#define ALIGN_BOXSIZE_H     BIG_BOX_H 

// Write the Alignment Button
#define WRITE_ALIGN_X       HOME_X
#define WRITE_ALIGN_Y       ALIGN_Y+ALIGN_BOXSIZE_H+5
#define SA_BOXSIZE_W        BIG_BOX_W
#define SA_BOXSIZE_H        BIG_BOX_H 

// Start/Clear the Alignment Button
#define START_ALIGN_X       225
#define START_ALIGN_Y       WRITE_ALIGN_Y
#define ST_BOXSIZE_W        BIG_BOX_W
#define ST_BOXSIZE_H        BIG_BOX_H 

AlignStates Current_State = Idle_State;
AlignStates Next_State = Idle_State;

// Align Button object
Button alignButton(0,0,0,0,display.butOnBackground, display.butBackground, display.butOutline, display.mainFontWidth, display.mainFontHeight, "");

// Canvas Print object default Arial 6x9 font
CanvasPrint canvAlignInsPrint(0, 0, 0, 0, display.butOnBackground, display.butBackground, "");

// ---- Draw Alignment Page ----
void AlignScreen::draw() {
  setCurrentScreen(ALIGN_SCREEN);
  setNightMode(getNightMode());
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  drawTitle(100, TITLE_TEXT_Y, "Alignment");
  drawMenuButtons();
  tft.setFont(&Inconsolata_Bold8pt7b);
  if (moreScreen.objectSelected) restoreAlignState();
         
  drawCommonStatusLabels();
  updateAlignButtons(false); // draw initial buttons
  canvAlignInsPrint.PrintLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Press Start to Align", false);

  canvAlignInsPrint.PrintLJ(250, 225, 65, 15, " G Rate ", false);
  char guideRateText[10];
  switch  (guide.settings.axis1RateSelect) {
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
  canvAlignInsPrint.PrintLJ(250, 240, 65, 15, guideRateText, false);
  showCorrections();
}

// task update for this screen
void AlignScreen::updateAlignStatus() {
  updateCommonStatus();
  stateMachine();
}

void AlignScreen::showAlignStatus() {
  char reply[15];
  int start_y = 176;
  int start_x = 2;
  getLocalCmdTrim(":GU#", reply); 
  tft.setCursor(start_x, start_y); 
}

// state change detection
bool AlignScreen::alignButStateChange() {
  if (preSlewState != mount.isSlewing()) {
    preSlewState = mount.isSlewing(); 
    return true;
  } else if (preHomeState != mount.isHome()) {
    preHomeState = mount.isHome(); 
    return true;
  } else if (display._redrawBut) {
    display._redrawBut = false;
    return true;
  } else {
    return false;
  }
}

/**************** Alignment Steps *********************
0) Press the [START] Button to begin Alignment
1) If not at home, Home the Scope using [HOME] button
2) Select number of stars for Alignment, max=3
3) Press [CATALOG] button - Filter ALL_SKY_ALIGN automatically selected
4) On catalog page, select a suitable star
5) On catalog page, use [RETURN] to this page
6) Press [GOTO] Button which slews to star
7) If star not centered, use guide buttons to center
8) Press [SYNC] button to register this star for correction
9) If more stars are to be used, go to step 3, repeat
10) Press the [WRITE] button to save calculations to EEPROM
11) The [ABORT] button resets everything ready for [START] again
********************************************************/

// *********** Update Align Buttons **************
void AlignScreen::updateAlignButtons(bool redrawBut) {
  _redrawBut = redrawBut;
  int x_offset = 0;
  
  // Go to Home Position
  if (mount.isHome()) {
    alignButton.draw(HOME_X, HOME_Y, HOME_BOXSIZE_W, HOME_BOXSIZE_H, "At Home", BUT_ON); 
  } else  if (mount.isSlewing()){
    alignButton.draw(HOME_X, HOME_Y, HOME_BOXSIZE_W, HOME_BOXSIZE_H, "Slewing", BUT_ON);                  
  } else {
    alignButton.draw(HOME_X, HOME_Y, HOME_BOXSIZE_W, HOME_BOXSIZE_H, "Not Home", BUT_OFF);
  }
    
  // Number of Stars for Alignment Buttons
  // Alignment become active here
  if (numAlignStars == 1) {   
    alignButton.draw(NUM_S_X, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "1", BUT_ON);
  } else {
    alignButton.draw(NUM_S_X, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "1", BUT_OFF);
  } 
  x_offset += NUM_S_SPACING_X;
  if (numAlignStars == 2) {   
    alignButton.draw(NUM_S_X+x_offset, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "2", BUT_ON);
  } else {
    alignButton.draw(NUM_S_X+x_offset, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "2", BUT_OFF);
  } 
  x_offset += NUM_S_SPACING_X;
  if (numAlignStars == 3) {   
    alignButton.draw(NUM_S_X+x_offset, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "3", BUT_ON);
  } else {
    alignButton.draw(NUM_S_X+x_offset, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "3", BUT_OFF);
  } 

  // go to the Star Catalog
  if (catalogBut) {
    alignButton.draw(ALIGN_CAT_X, ALIGN_CAT_Y, CAT_BOXSIZE_W, CAT_BOXSIZE_H, "CATALOG", BUT_ON);
  } else {
    alignButton.draw(ALIGN_CAT_X, ALIGN_CAT_Y, CAT_BOXSIZE_W, CAT_BOXSIZE_H, "CATALOG", BUT_OFF);         
  }

  // Go To Coordinates Button
  if (gotoBut || mount.isSlewing()) {
    alignButton.draw(GOTO_X, GOTO_Y,  GOTO_BOXSIZE_W, GOTO_BOXSIZE_H, "Going", BUT_ON);
  } else {
    alignButton.draw(GOTO_X, GOTO_Y,  GOTO_BOXSIZE_W, GOTO_BOXSIZE_H, "GOTO", BUT_OFF);
  }
  
  // SYNC button; calculate alignment parameters
  if (syncBut) {
    alignButton.draw(ALIGN_X, ALIGN_Y, ALIGN_BOXSIZE_W, ALIGN_BOXSIZE_H, "SYNC'd", BUT_ON);
  } else {
    alignButton.draw(ALIGN_X, ALIGN_Y, ALIGN_BOXSIZE_W, ALIGN_BOXSIZE_H, "SYNC", BUT_OFF);         
  }

  // save the alignment calculations to EEPROM
  if (saveAlignBut) {
    alignButton.draw(WRITE_ALIGN_X, WRITE_ALIGN_Y, SA_BOXSIZE_W, SA_BOXSIZE_H, "Saved", BUT_ON);
  } else {
    alignButton.draw(WRITE_ALIGN_X, WRITE_ALIGN_Y, SA_BOXSIZE_W, SA_BOXSIZE_H, "SAVE", BUT_OFF);
  }

  // start alignnment
  if (startAlignBut) {
    alignButton.draw(START_ALIGN_X, START_ALIGN_Y, ST_BOXSIZE_W, ST_BOXSIZE_H, "Running", BUT_ON);
  } else {
    alignButton.draw(START_ALIGN_X, START_ALIGN_Y, ST_BOXSIZE_W, ST_BOXSIZE_H, "START", BUT_OFF);
  }
  
  // Abort Alignment Button
  if (abortBut) {
    alignButton.draw(ABORT_X, ABORT_Y, GOTO_BOXSIZE_W, GOTO_BOXSIZE_H, "Abort'd", BUT_ON);
    abortBut = false;
    delay(500);
    alignButton.draw(ABORT_X, ABORT_Y, GOTO_BOXSIZE_W, GOTO_BOXSIZE_H, "ABORT", BUT_OFF);
  } else {
    alignButton.draw(ABORT_X, ABORT_Y, GOTO_BOXSIZE_W, GOTO_BOXSIZE_H, "ABORT", BUT_OFF);
  }
  updateGuideButtons(); // draw the guide buttons
}

// ===============================================================
// ==== Align State Machine .. updates at the status task rate ===
// ===============================================================
void AlignScreen::stateMachine() {
  switch(Current_State) {
    case 0:  canvAlignInsPrint.PrintLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State = Waiting For Start", false);           break;
    case 1:  canvAlignInsPrint.PrintLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State = Home Mount", false);                  break;
    case 2:  canvAlignInsPrint.PrintLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State = Select Number of Stars", false);      break;
    case 3:  canvAlignInsPrint.PrintLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State = Select Star in Catalog", false);      break;
    case 4:  canvAlignInsPrint.PrintLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State = Wait for Catalog return", false);     break;
    case 5:  canvAlignInsPrint.PrintLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State = Go To Star", false);                  break;
    case 6:  canvAlignInsPrint.PrintLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State = Wait For Slewing to finish", false);  break;
    case 7:  canvAlignInsPrint.PrintLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State = Center & Sync", false);               break;
    case 8:  canvAlignInsPrint.PrintLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State = Write Coords", false);                break;
    default: canvAlignInsPrint.PrintLJ(STATE_LABEL_X, STATE_LABEL_Y, STATUS_LABEL_W, STATUS_LABEL_H,  "State = Waiting for Start", false);           break;
  }
  
  if (startAlignBut) { 
    sprintf(curAlign,  "Current Star is: %d of %d", alignCurStar, numAlignStars);
    canvAlignInsPrint.PrintLJ(STATUS_LABEL_X, STATUS_LABEL_Y+20, 200, 15, curAlign, false);
  
    // align state machine
    Current_State = Next_State;
    switch(Current_State) {
      case Idle_State: {
        if (startAlignBut) {
          canvAlignInsPrint.PrintLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Home Scope to Start", false);
          Next_State = Home_State;
        } else {
          canvAlignInsPrint.PrintLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Press Start to Align", false);
          Next_State = Idle_State;
        }
        break;
      } 

      case Home_State: {
        if (!mount.isHome() && homeBut) {
          setLocalCmd(":hC#"); // go HOME
          if (mount.isSlewing()) { 
            canvAlignInsPrint.PrintLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Slewing", false);
          }
          Next_State = Home_State;
          homeBut = false;
        } else if (mount.isHome()) {
          canvAlignInsPrint.PrintLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Is Home", false);
          Next_State = Num_Stars_State;  
          homeBut = false; 
        } else {
          canvAlignInsPrint.PrintLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Not Home", false);
          Next_State = Home_State; 
        }
        break;
      } 

      case Num_Stars_State: {
        if (numAlignStars>0) {
          char s[6]; sprintf (s, ":A%d#", numAlignStars); // set number of align stars
          setLocalCmd(s);
          Next_State = Select_Catalog_State; 
        } else {
          canvAlignInsPrint.PrintLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Select Number of Stars", false);
          Next_State = Num_Stars_State;
        }
        break;
      } 

      case Select_Catalog_State: {
        if (catalogBut) {
          catalogBut = false;
          alignCurStar++;
          Next_State = Wait_Catalog_State;
          moreScreen.activeFilter = FM_ALIGN_ALL_SKY;
          cat_mgr.filterAdd(moreScreen.activeFilter); 
          saveAlignState();
          shcCatScreen.init(STARS); // draw the STARS SCREEN
          return;
        } else {
          canvAlignInsPrint.PrintLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Select Star in Catalog", false);
          Next_State = Select_Catalog_State;
        }
        break;
      }
          
      case Wait_Catalog_State: {
        if (display.currentScreen == ALIGN_SCREEN) { // doesn't change state until Catalog returns back to this page
          if (moreScreen.objectSelected) { // a star has been selected from the catalog
            Next_State = Goto_State;
          } else {
            Next_State = Select_Catalog_State;
          }
        } else {
          Next_State = Wait_Catalog_State;
        }
        break;
      }   

      case Goto_State: {
        if (gotoBut) {
          setLocalCmd(":MS#");
          gotoBut = false;
          Next_State = Wait_For_Slewing_State;
        } else { // wait for GoTo button press
          canvAlignInsPrint.PrintLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Press GoTo", false);
          Next_State = Goto_State;
        }
        break;
      }

      case Wait_For_Slewing_State: {
        if (mount.isSlewing()) { 
          canvAlignInsPrint.PrintLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Slewing", false);
          Next_State = Wait_For_Slewing_State;
        } else { // not slewing
          canvAlignInsPrint.PrintLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "GoTo Done", false);
          Next_State = Sync_State;
        }
        break;
      }

      case Sync_State: { 
        if (syncBut) {
          setLocalCmd(":A+#");
          syncBut = false;
          updateAlignButtons(false);
          
          if (alignCurStar < numAlignStars) { // more stars to align? 
            Next_State = Select_Catalog_State;
          } else {
            Next_State = Write_State;
          }
        } else { 
          canvAlignInsPrint.PrintLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Center Star then Press Sync", false);
          Next_State = Sync_State;
        } 
        break;
      }

      case Write_State: {
        if (saveAlignBut) {
          canvAlignInsPrint.PrintLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Writing Align Data", false);
          setLocalCmd(":AW#");
          saveAlignBut = false;
          Next_State = Status_State;
        } else {
          canvAlignInsPrint.PrintLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Waiting for Save", false);
          Next_State = Write_State;
        }
        break;
      }

      case Status_State: {
        canvAlignInsPrint.PrintLJ(STATUS_LABEL_X, STATUS_LABEL_Y, STATE_UPDATE_W, STATE_UPDATE_H, "Alignment Completed", false);
        showCorrections(); // show current align corrections
        startAlignBut = false;
        updateAlignButtons(false);
        Next_State = Idle_State;
        break;
      }

      default: Next_State = Idle_State; 
    } // end switch current state
  } // end start align button true
} // end State Machine

// ================= Check Align Buttons ===================
// -- return true if touched --
bool AlignScreen::touchPoll(uint16_t px, uint16_t py) { 
  // ==== ABORT GOTO  Button ====
  if (py > ABORT_Y && py < (ABORT_Y + GOTO_BOXSIZE_H) && px > ABORT_X && px < (ABORT_X + GOTO_BOXSIZE_W)) {
    BEEP;
    setLocalCmd(":Q#"); // stops move

    // clear all button states since not sure which was active
    numAlignStars = 0;
    alignCurStar = 0;
    abortBut = true;
    homeBut = false;
    catalogBut = false;
    gotoBut = false;
    syncBut = false;
    saveAlignBut = false;
    startAlignBut = false;
    Next_State = Idle_State;
    return true;
  }

  // Go to Home Telescope Requested
  if (px > HOME_X && px < HOME_X + HOME_BOXSIZE_W && py > HOME_Y  && py < HOME_Y + HOME_BOXSIZE_H) {
    if (Current_State==Home_State) {
      BEEP;
      homeBut = true;
      return true;
    }
  }
  
  // Number of Stars for alignment
  int x_offset = 0;
  if (Current_State==Num_Stars_State) {
    if (py > NUM_S_Y && py < (NUM_S_Y + NUM_S_BOXSIZE_H) && px > NUM_S_X+x_offset && px < (NUM_S_X+x_offset + NUM_S_BOXSIZE_W)) {
      BEEP;
      numAlignStars = 1;
      return true;
    }
    x_offset += NUM_S_SPACING_X;
    if (py > NUM_S_Y && py < (NUM_S_Y + NUM_S_BOXSIZE_H) && px > NUM_S_X+x_offset && px < (NUM_S_X+x_offset + NUM_S_BOXSIZE_W)) {
      BEEP;
      numAlignStars = 2;
      return true;
    }
    x_offset += NUM_S_SPACING_X;
    if (py > NUM_S_Y && py < (NUM_S_Y + NUM_S_BOXSIZE_H) && px > NUM_S_X+x_offset && px < (NUM_S_X+x_offset + NUM_S_BOXSIZE_W)) {
      BEEP;
      numAlignStars = 3;
      return true;
    }
  }

  // Call up the Catalog Button
  if (py > ALIGN_CAT_Y && py < (ALIGN_CAT_Y + CAT_BOXSIZE_H) && px > ALIGN_CAT_X && px < (ALIGN_CAT_X + CAT_BOXSIZE_W)) {
    if (Current_State==Select_Catalog_State ) {
      BEEP;
      catalogBut = true;
      return true;
    }
  }

  // Go To Target Coordinates
  if (py > GOTO_Y && py < (GOTO_Y + GOTO_BOXSIZE_H) && px > GOTO_X && px < (GOTO_X + GOTO_BOXSIZE_W)) {
    if (Current_State==Goto_State) { 
      BEEP;
      gotoBut = true;
      return true;
    }
  }

  // SYNC - calculate alignment corrections Button
  if (py > ALIGN_Y && py < (ALIGN_Y + ALIGN_BOXSIZE_H) && px > ALIGN_X && px < (ALIGN_X + ALIGN_BOXSIZE_W)) { 
    if (Current_State==Sync_State) {
      BEEP;
      syncBut = true;
      return true;
    }
  }

  // Write Alignment Button
  if (py > WRITE_ALIGN_Y && py < (WRITE_ALIGN_Y + SA_BOXSIZE_H) && px > WRITE_ALIGN_X && px < (WRITE_ALIGN_X + SA_BOXSIZE_W)) {
    if (Current_State==Write_State) {
      BEEP;
      saveAlignBut = true;
      return true;
    }
  }  

  // START Alignment Button - clear the corrections, reset the state machine
  if (py > START_ALIGN_Y && py < (START_ALIGN_Y + ST_BOXSIZE_H) && px > START_ALIGN_X && px < (START_ALIGN_X + ST_BOXSIZE_W)) { 
    BEEP;
    startAlignBut = true;
    alignCurStar = 0;
    numAlignStars = 0; // number of selected align stars from buttons
    Current_State = Idle_State;
    //Next_State = Idle_State;

    // Enable the Motors
    digitalWrite(AZ_ENABLED_LED_PIN, LOW); // Turn On AZ LED
    motor1.power(true); // AZ motor on
    
    digitalWrite(ALT_ENABLED_LED_PIN, LOW); // Turn On ALT LED
    motor2.power(true); // ALT motor on
    return true;
  } 

  // EAST / RIGHT button
  if (py > r_y && py < (r_y + box_h) && px > r_x && px < (r_x + box_w)) {
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
  if (py > l_y && py < (l_y + box_h) && px > l_x && px < (l_x + box_w)) {
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
  if (py > u_y && py < (u_y + box_h) && px > u_x && px < (u_x + box_w)) {
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
  if (py > d_y && py < (d_y + box_h) && px > d_x && px < (d_x + box_w)) {
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
  return false;
}

// Align Screen state managment...needed for returning from STARS to ALIGN SCREEN
static uint8_t _numAlignStars = 0;
static uint8_t _alignCurStar = 0;
static bool _homeBut = false;
static bool _catalogBut = false;
static bool _gotoBut = false;
static bool _syncBut = false;
static bool _saveAlignBut = false;
static bool _startAlignBut = false;
static AlignStates _Next_State = Idle_State;
static AlignStates _Current_State = Idle_State;

void AlignScreen::restoreAlignState() {
  numAlignStars = _numAlignStars;
  alignCurStar = _alignCurStar;
  homeBut = _homeBut;
  catalogBut = _catalogBut;
  gotoBut = _gotoBut;
  syncBut = _syncBut;
  saveAlignBut = _saveAlignBut;
  startAlignBut = _startAlignBut;
  Next_State = _Next_State;
  Current_State = _Current_State;
}

void AlignScreen::saveAlignState() {
  _numAlignStars = numAlignStars;
  _alignCurStar = alignCurStar;
  _homeBut = homeBut;
  _catalogBut = catalogBut;
  _gotoBut = gotoBut;
  _syncBut = syncBut;
  _saveAlignBut = saveAlignBut;
  _startAlignBut = startAlignBut;
  _Next_State = Next_State;
  _Current_State = Current_State;
}

// ========== Update Guide Page Buttons ==========
void AlignScreen::updateGuideButtons() {
  if (guidingEast && mount.isSlewing()) { 
    alignButton.draw(r_x, r_y, box_w, box_h, "EAST", BUT_ON);
  } else {
    alignButton.draw(r_x, r_y, box_w, box_h, "EAST", BUT_OFF);
  }

  if (guidingWest && mount.isSlewing()) { 
    alignButton.draw(l_x, l_y, box_w, box_h, "WEST", BUT_ON);
  } else {
    alignButton.draw(l_x, l_y, box_w, box_h, "WEST", BUT_OFF);
  }

 if (guidingNorth && mount.isSlewing()) { 
    alignButton.draw(u_x, u_y, box_w, box_h, "NORTH", BUT_ON);
  } else {
    alignButton.draw(u_x, u_y, box_w, box_h, "NORTH", BUT_OFF);
  }

  if (guidingSouth && mount.isSlewing()) { 
    alignButton.draw(d_x, d_y, box_w, box_h, "SOUTH", BUT_ON);
  } else {
    alignButton.draw(d_x, d_y, box_w, box_h, "SOUTH", BUT_OFF);
  }
}

// ***** Show Calculated Corrections ******
// ax1Cor: align internal index for Axis1
// ax2Cor: align internal index for Axis2
// altCor: for geometric coordinate correction/align, - is below the pole, + above
// azmCor: - is right of the pole, + is left
// doCor: declination/optics orthogonal correction
// pdCor: declination/polar orthogonal correction
// dfCor: fork or declination axis flex
// tfCor: tube flex
void AlignScreen::showCorrections() {
  int x_off = 0;
  int y_off = 0;
  int acorr_x = 100;
  int acorr_y = 330;
  int acorr_w = 100;
  int acorr_h = 15;
  char _reply[10];

  tft.drawRect(acorr_x, acorr_y, acorr_w, 5*acorr_h, pgBackground); // clear background
  tft.setCursor(acorr_x+20, acorr_y); tft.print("Calculated Corrections");

  y_off += 14;
  getLocalCmdTrim(":GX00#", _reply); 
  sprintf(acorr,"ax1Cor=%s", _reply); // ax1Cor
  canvAlignInsPrint.PrintLJ(acorr_x+x_off, acorr_y+y_off, acorr_w, acorr_h, acorr, false);

  y_off += 14;
  getLocalCmdTrim(":GX01#", _reply); 
  sprintf(acorr,"ax2Cor=%s", _reply); // ax2Cor
  canvAlignInsPrint.PrintLJ(acorr_x+x_off, acorr_y+y_off, acorr_w, acorr_h, acorr, false);

  y_off += 14;
  getLocalCmdTrim(":GX02#", _reply); 
  sprintf(acorr,"altCor=%s", _reply); // altCor
  canvAlignInsPrint.PrintLJ(acorr_x+x_off, acorr_y+y_off, acorr_w, acorr_h, acorr, false);

  y_off += 14;
  getLocalCmdTrim(":GX03#", _reply); 
  sprintf(acorr,"azmCor=%s", _reply); // azmCor
  canvAlignInsPrint.PrintLJ(acorr_x+x_off, acorr_y+y_off, acorr_w, acorr_h, acorr, false);

  x_off = 110;
  y_off = 14;
  getLocalCmdTrim(":GX04#", _reply); 
  sprintf(acorr," doCor=%s", _reply);  // doCor
  canvAlignInsPrint.PrintLJ(acorr_x+x_off, acorr_y+y_off, acorr_w, acorr_h, acorr, false);

  y_off += 14;
  getLocalCmdTrim(":GX05#", _reply); 
  sprintf(acorr," pdCor=%s", _reply);  // pdCor
  canvAlignInsPrint.PrintLJ(acorr_x+x_off, acorr_y+y_off, acorr_w, acorr_h, acorr, false);

  y_off += 14;
  getLocalCmdTrim(":GX06#", _reply); 
  sprintf(acorr," ffCor=%s", _reply);  // ffCor
  canvAlignInsPrint.PrintLJ(acorr_x+x_off, acorr_y+y_off, acorr_w, acorr_h, acorr, false);

  y_off += 14;
  getLocalCmdTrim(":GX07#", _reply); 
  sprintf(acorr," dfCor=%s", _reply);  // dfCor
  canvAlignInsPrint.PrintLJ(acorr_x+x_off, acorr_y+y_off, acorr_w, acorr_h, acorr, false);
}

AlignScreen alignScreen;
