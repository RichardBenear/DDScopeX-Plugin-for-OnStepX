// =====================================================
// AlignScreen.cpp

#include "AlignScreen.h"
#include "MoreScreen.h"
#include "SHCCatScreen.h"
#include "../catalog/Catalog.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "src/telescope/mount/Mount.h"
#include "src/telescope/mount/goto/Goto.h"

#define BIG_BOX_W           80
#define BIG_BOX_H           40
#define BIG_BOX_T_OFF_W     20
#define BIG_BOX_T_OFF_H     4
#define A_STATUS_X          115
#define A_STATUS_Y          245
#define A_STATUS_Y_SP       19
#define UPDATE_W            200
#define UPDATE_H            18

#define STATE_LABEL_X       105
#define STATE_LABEL_Y       218
#define STATUS_LABEL_X      STATE_LABEL_X
#define STATUS_LABEL_Y      195
#define ERROR_LABEL_X       STATE_LABEL_X
#define ERROR_LABEL_Y       STATUS_LABEL_Y+20
#define LABEL_SPACING_Y     2

// Go to Home position button
#define HOME_X              7
#define HOME_Y              180
#define HOME_BOXSIZE_W      BIG_BOX_W
#define HOME_BOXSIZE_H      BIG_BOX_H 
#define HOME_T_OFF_X        BIG_BOX_W/2-BIG_BOX_T_OFF_W
#define HOME_T_OFF_Y        BIG_BOX_H/2+BIG_BOX_T_OFF_H

// Align Num Stars Button(s)
#define NUM_S_X             1
#define NUM_S_Y             HOME_Y+HOME_BOXSIZE_H+5
#define NUM_S_BOXSIZE_W     30
#define NUM_S_BOXSIZE_H     35
#define NUM_S_SPACING_X     NUM_S_BOXSIZE_W+3 

// Go to Catalog Page Button
#define ACAT_X              HOME_X
#define ACAT_Y              NUM_S_Y+NUM_S_BOXSIZE_H+5
#define CAT_BOXSIZE_W       BIG_BOX_W
#define CAT_BOXSIZE_H       BIG_BOX_H 

// GO TO Target Button
#define GOTO_X              HOME_X
#define GOTO_Y              ACAT_Y+CAT_BOXSIZE_H+5
#define GOTO_BOXSIZE_W      BIG_BOX_W
#define GOTO_BOXSIZE_H      BIG_BOX_H 

// ABORT Button
#define ABORT_X             110
#define ABORT_Y             WRITE_ALIGN_Y

// ALIGN this Star Button
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
Button alignButton(
  0,0,0,0,
  display.butOnBackground, 
  display.butBackground, 
  display.butOutline, 
  display.mainFontWidth, 
  display.mainFontHeight, 
  "");

// ---- Draw Alignment Page ----
void AlignScreen::draw() { 
  setCurrentScreen(ALIGN_SCREEN);
  setNightMode(getNightMode());
  homeBut = false;
  catalogBut = false;
  gotoBut = false;
  abortBut = false;
  syncBut = false;
  saveAlignBut = false;
  startAlignBut = false;
  
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  drawMenuButtons();
  drawTitle(100, TITLE_TEXT_Y, "Alignment");
  tft.setFont(&Inconsolata_Bold8pt7b);
  updateAlignButtons(false);
  drawCommonStatusLabels();
  showCorrections();
  getAlignStatus();
}

// task update for this screen
void AlignScreen::updateAlignStatus() {
  updateCommonStatus();
  stateMachine();
}

void AlignScreen::getAlignStatus() {
  char _reply[4];
  int start_y = 176;
  int start_x = 2;
  getLocalCmdTrim(":GU#", _reply); 
  tft.setCursor(start_x, start_y); 
  tft.fillRect(start_x, start_y - 11, 315, 15, butBackground);
  if (_reply[0] == 'A') tft.printf("AltAzm |");
  if (_reply[0] == 'K') tft.printf(" Fork  |");
  if (_reply[0] == 'E') tft.printf(" GEM   |");
  tft.setCursor(70, start_y);
  if (_reply[1] == 'n') tft.printf("Not Tracking|"); else tft.printf("  Tracking  |");
  tft.setCursor(175, start_y);
  if (!goTo.alignDone()) tft.printf("  Needs aligned");
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
  int acorr_x = 115;
  int acorr_y = 310;
  char _reply[10];

  tft.drawRect(acorr_x, acorr_y, 200, 5*16, pgBackground); // clear background
  tft.setCursor(acorr_x, acorr_y); tft.print("Calculated Corrections");

  y_off += 16;
  getLocalCmdTrim(":GX00#", _reply); 
  sprintf(acorr,"ax1Cor=%s", _reply); // ax1Cor
  tft.setCursor(acorr_x+x_off, acorr_y+y_off); tft.print(acorr);

  y_off += 16;
  getLocalCmdTrim(":GX01#", _reply); 
  sprintf(acorr,"ax2Cor=%s", _reply); // ax2Cor
  tft.setCursor(acorr_x+x_off, acorr_y+y_off); tft.print(acorr);

  y_off += 16;
  getLocalCmdTrim(":GX02#", _reply); 
  sprintf(acorr,"altCor=%s", _reply); // altCor
  tft.setCursor(acorr_x+x_off, acorr_y+y_off); tft.print(acorr);

  y_off += 16;
  getLocalCmdTrim(":GX03#", _reply); 
  sprintf(acorr,"azmCor=%s", _reply); // azmCor
  tft.setCursor(acorr_x+x_off, acorr_y+y_off); tft.print(acorr);

  x_off = 100;
  y_off = 16;
  getLocalCmdTrim(":GX04#", _reply); 
  sprintf(acorr," doCor=%s", _reply);  // doCor
  tft.setCursor(acorr_x+x_off, acorr_y+y_off); tft.print(acorr);

  y_off += 16;
  getLocalCmdTrim(":GX05#", _reply); 
  sprintf(acorr," pdCor=%s", _reply);  // pdCor
  tft.setCursor(acorr_x+x_off, acorr_y+y_off); tft.print(acorr);

  y_off += 16;
  getLocalCmdTrim(":GX06#", _reply); 
  sprintf(acorr," ffCor=%s", _reply);  // ffCor
  tft.setCursor(acorr_x+x_off, acorr_y+y_off); tft.print(acorr);

  y_off += 16;
  getLocalCmdTrim(":GX07#", _reply); 
  sprintf(acorr," dfCor=%s", _reply);  // dfCor
  tft.setCursor(acorr_x+x_off, acorr_y+y_off); tft.print(acorr);
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
1) Home the Scope using [HOME] button
2) Select number of stars for Alignment, max=3
3) Press [CATALOG] button - Filter ALL_SKY_ALIGN automatically selected
4) On catalog page, select a suitable star
5) On catalog page, use [BACK] till return to this page
6) Press [GOTO] Button which slews to star
7) If star not centered, go to GUIDE page and center
8) Go back to ALIGN page if on GUIDE page
9) Press [ALIGN] button on the ALIGN page
10) If more stars are to be used, go to step 3, repeat
11) Press the [WRITE] button to save calculations to EEPROM
12) The [ABORT] button resets back to the Start 0) and shuts off motors
********************************************************/

// *********** Update Align Buttons **************
void AlignScreen::updateAlignButtons(bool redrawBut) {
  _redrawBut = redrawBut;
  int x_offset = 0;
  
  // Go to Home Position
  if (homeBut) {
    if (mount.isHome()) {
      alignButton.draw(HOME_X, HOME_Y, HOME_BOXSIZE_W, HOME_BOXSIZE_H, "is Home", BUT_ON); 
    } else  if (mount.isSlewing()){
      alignButton.draw(HOME_X, HOME_Y, HOME_BOXSIZE_W, HOME_BOXSIZE_H, "is Slewing", BUT_ON);                  
    } else {
      alignButton.draw(HOME_X, HOME_Y, HOME_BOXSIZE_W, HOME_BOXSIZE_H, "not Home", BUT_OFF);
    }
  }
    
  // Number of Stars for Alignment Buttons
  // Alignment become active here
  if (numAlignStars == 1) {   
    alignButton.draw(NUM_S_X, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "1", BUT_ON);
  } else {
    alignButton.draw(NUM_S_X, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "1", BUT_OFF);
  } 
  x_offset += NUM_S_SPACING_X;
  if (numAlignStars == 1) {   
    alignButton.draw(NUM_S_X, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "2", BUT_ON);
  } else {
    alignButton.draw(NUM_S_X, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "2", BUT_OFF);
  } 
  x_offset += NUM_S_SPACING_X;
  if (numAlignStars == 1) {   
    alignButton.draw(NUM_S_X, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "3", BUT_ON);
  } else {
    alignButton.draw(NUM_S_X, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, "3", BUT_OFF);
  } 

  // go to the Star Catalog
  if (catalogBut) {
    alignButton.draw(ACAT_X, ACAT_Y, CAT_BOXSIZE_W, CAT_BOXSIZE_H, "CATALOG", BUT_ON);
  } else {
    alignButton.draw(ACAT_X, ACAT_Y, CAT_BOXSIZE_W, CAT_BOXSIZE_H, "CATALOG", BUT_OFF);         
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
    alignButton.draw(START_ALIGN_X, START_ALIGN_Y, ST_BOXSIZE_W, ST_BOXSIZE_H, "STARTed", BUT_ON);
  } else {
    alignButton.draw(START_ALIGN_X, START_ALIGN_Y, ST_BOXSIZE_W, ST_BOXSIZE_H, "START", BUT_OFF);
  }
  
  // Abort Alignment Button
  if (abortBut) {
    alignButton.draw(ABORT_X, ABORT_Y, GOTO_BOXSIZE_W, GOTO_BOXSIZE_H, "Abort'd", BUT_ON);
    abortBut = false;
  } else {
    alignButton.draw(ABORT_X, ABORT_Y, GOTO_BOXSIZE_W, GOTO_BOXSIZE_H, "ABORT", BUT_OFF);
  }
}

// ----- Alignment State Machine -----
void AlignScreen::stateMachine() {
  // Display Alignment Status
  if (startAlignBut) { 
    canvPrint(A_STATUS_X, A_STATUS_Y, 0, 160, UPDATE_H, " -Align Active-   "); 
  } else if (Current_State == Write_State) { 
    canvPrint(A_STATUS_X, A_STATUS_Y, 0, 160, UPDATE_H, "  -Align Done-    "); 
  } else {             
    canvPrint(A_STATUS_X, A_STATUS_Y, 0, 160, UPDATE_H, "-Align Not Active-"); 
  }

  if (alignCurStar > numAlignStars) alignCurStar = numAlignStars;  

  sprintf(curAlign,  "  Current Star = %d", alignCurStar);
  sprintf(lastAlign, " Last Req Star = %d", numAlignStars);
  canvPrint(A_STATUS_X, A_STATUS_Y+  A_STATUS_Y_SP, 0, 160, 18, curAlign);
  canvPrint(A_STATUS_X, A_STATUS_Y+2*A_STATUS_Y_SP, 0, 160, 18, lastAlign);
  
  // =======================================================================
  // ==== Align State Machine .. updates at the update-timer-thread rate ===
  // =======================================================================
  /*
  switch(Current_State) {
    case 0:  canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H,  "State = Wait_For_Start");   break;
    case 1:  canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H,  "State = Home");             break;
    case 2:  canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H,  "State = Num_Stars");        break;
    case 3:  canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H,  "State = Select_Catalog");   break;
    case 4:  canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H,  "State = Wait_Catalog");     break;
    case 5:  canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H,  "State = GoTo");             break;
    case 6:  canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H,  "State = Wait_For_Slewing"); break;
    case 7:  canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H,  "State = Align");            break;
    case 8:  canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H,  "State = Write");            break;
    default: canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H,  "State = Wait_Start");       break;
  }
  */
  firstLabel=true;    // creates a one-time text update in a state to reduce screen flicker
                      // this is necessary in states that looping in their state and are also printing something
  
  // align state machine
  Current_State = Next_State;
  switch(Current_State) {
    case Idle_State: {
      if (startAlignBut) {
        canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, "Home Scope to Start");
        canvPrint(ERROR_LABEL_X, ERROR_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H,   "No Errors");
        Next_State = Home_State;
      } else {
        canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, "Press Start to Align");
        firstLabel = false;  
      }
      Next_State = Idle_State;
      break;
    } 

    case Home_State: {
      if (homeBut) {
        homeBut = false;
  
        if (!mount.isHome()) {
          setLocalCmd(":hC#"); // go HOME

          if (firstLabel) { // print only the 1st time, no flicker
            canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, "Slewing");
            firstLabel=false;  
          }
          Next_State = Home_State;
        } else {
          canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, "is Home");
          Next_State = Num_Stars_State;
        }
        Next_State = Home_State;
        break;
      }
    } 

    case Num_Stars_State: {
      if (numAlignStars>0) {
        char s[6]; sprintf (s, ":A%d#", numAlignStars); // set number of align stars
        setLocalCmd(s);
        Next_State = Select_Catalog_State; 
      } else {
        if (firstLabel) { 
          canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, "Select Number of Stars");
          firstLabel=false;
        } 
        Next_State = Num_Stars_State;
      }
      break;
    } 

    case Select_Catalog_State: {
      if (firstLabel) {
        canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, "Select Star in Catalog");
        firstLabel=false;
      }
      if (catalogBut) {
        catalogBut = false;
        Next_State = Wait_Catalog_State;
        moreScreen.activeFilter = FM_ALIGN_ALL_SKY;
        cat_mgr.filterAdd(moreScreen.activeFilter); 
        shcCatScreen.init(STARS);
        return;
      } else {
        Next_State = Select_Catalog_State;
      }
      break;
    }
        
    case Wait_Catalog_State: {
      if (display.currentScreen == ALIGN_SCREEN) { // doesn't change state until Catalog points back to this page
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
      if (firstLabel) {
        canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, "Press Go To");
        firstLabel=false;
      }
      if (gotoBut) {
        setLocalCmd(":MS#");
        gotoBut = false;
        Next_State = Wait_For_Slewing_State;
      } else { // wait for GoTo button press
        Next_State = Goto_State;
      }
      break;
    }

    case Wait_For_Slewing_State: {
      if (mount.isSlewing()) { //|| trackingSyncInProgress()) {
        if (firstLabel) {
          canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, "Slewing");
          firstLabel=false;
        }
        Next_State = Wait_For_Slewing_State;
      } else { // not slewing
        canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, "GoTo Completed");
        Next_State = Align_State;
      }
      break;
    }

    case Align_State: {
      if (!syncBut) {
        if (firstLabel) {
          canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, "Press Align");
          firstLabel=false;
        }
        Next_State = Align_State;
      } else {  // align this star
        setLocalCmd(":A+#"); // add star to alignment
        syncBut = false;
      
        if (alignCurStar <= numAlignStars) { // more stars to align? (alignCurStar was incremented by cmd A+)
          Next_State = Select_Catalog_State;
        } else {
          Next_State = Write_State;
        }
      } 
      break;
    }

    case Write_State: {
      if (saveAlignBut) {
        canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, "Writing Align Data");
        setLocalCmd(":AW#");

        saveAlignBut = false;
        Next_State = Idle_State;
      } else {
        if (firstLabel) {
          canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, "Waiting for Write");
          canvPrint(ERROR_LABEL_X, ERROR_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, "No Errors");
          showCorrections(); // show the corrections to determine if we want to Save or Abort
          firstLabel=false;
        }
        Next_State = Write_State;
      }
      getAlignStatus(); // show current align status
      break;
    }
    default: Next_State = Idle_State; 
  } // end switch current state
} // end State Machine

// ===== Check Align Buttons =====
// return true if touched
bool AlignScreen::touchPoll(uint16_t px, uint16_t py) {
  // Go to Home Telescope Requested
  if (px > HOME_X && px < HOME_X + HOME_BOXSIZE_W && py > HOME_Y  && py < HOME_Y + HOME_BOXSIZE_H) {
    BEEP;
    if (Current_State==Home_State) {
      homeBut = true;
    }
    return true;
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
  if (py > ACAT_Y && py < (ACAT_Y + CAT_BOXSIZE_H) && px > ACAT_X && px < (ACAT_X + CAT_BOXSIZE_W)) {
    BEEP;
    if (Current_State==Select_Catalog_State ) {
      catalogBut = true;
    }
    return true;
  }

  // Go To Target Coordinates
  if (py > GOTO_Y && py < (GOTO_Y + GOTO_BOXSIZE_H) && px > GOTO_X && px < (GOTO_X + GOTO_BOXSIZE_W)) {
    BEEP;
    if (Current_State==Goto_State) { 
      gotoBut = true;
    }
    return true;
  }

  // ==== ABORT GOTO ====
  if (py > ABORT_Y && py < (ABORT_Y + GOTO_BOXSIZE_H) && px > ABORT_X && px < (ABORT_X + GOTO_BOXSIZE_W)) {
    BEEP;
    abortBut = true;
   
    setLocalCmd(":Q#"); // stops move
    motor1.power(false); // do this for safety reasons...mount may be colliding with something
    motor2.power(false);

    numAlignStars = 0;
    alignCurStar = 0;

    // clear all button states
    homeBut = false;
    catalogBut = false;
    gotoBut = false;
    syncBut = false;
    saveAlignBut = false;
    startAlignBut = false;
    Next_State = Idle_State;
    return true;
  }

  // ALIGN / calculate alignment corrections Button
  if (py > ALIGN_Y && py < (ALIGN_Y + ALIGN_BOXSIZE_H) && px > ALIGN_X && px < (ALIGN_X + ALIGN_BOXSIZE_W)) {
    BEEP; 
    if (Current_State==Align_State) {
      syncBut = true;
    }
    return true;
  }

  // Write Alignment Button
  if (py > WRITE_ALIGN_Y && py < (WRITE_ALIGN_Y + SA_BOXSIZE_H) && px > WRITE_ALIGN_X && px < (WRITE_ALIGN_X + SA_BOXSIZE_W)) {
    BEEP; 
    if (Current_State==Write_State) {
      saveAlignBut = true;
    }
    return true;
  }  

  // START Alignment Button - clear the corrections, reset the state machine
  if (py > START_ALIGN_Y && py < (START_ALIGN_Y + ST_BOXSIZE_H) && px > START_ALIGN_X && px < (START_ALIGN_X + ST_BOXSIZE_W)) { 
    BEEP;
    startAlignBut = true;
    alignCurStar = 0;
    numAlignStars = 0; // number of selected align stars from buttons
    Current_State = Idle_State;
    Next_State = Idle_State;

    // Enable the Motors
    digitalWrite(AZ_ENABLED_LED_PIN, LOW); // Turn On AZ LED
    motor1.power(true); // AZ motor on
    
    digitalWrite(ALT_ENABLED_LED_PIN, LOW); // Turn On ALT LED
    motor2.power(true); // ALT motor on
    return true;
  } 
  return false;
}

AlignScreen alignScreen;
