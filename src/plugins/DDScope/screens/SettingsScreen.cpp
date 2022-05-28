// =====================================================
// SettingsScreen.cpp

// Author: Richard Benear 2/13/22

#include "SettingsScreen.h"
#include "../display/Display.h"

#define PAD_BUTTON_X         2
#define PAD_BUTTON_Y         280
#define PAD_BUTTON_W         50
#define PAD_BUTTON_H         40
#define PAD_BUTTON_SPACING_X 1
#define PAD_BUTTON_SPACING_Y 1
#define PAD_T_OFF_X          PAD_BUTTON_W/2 - 10/2
#define PAD_T_OFF_Y          PAD_BUTTON_H/2 + 3

#define TXT_LABEL_X         5
#define TXT_LABEL_Y         171
#define TXT_FIELD_X         135
#define TXT_FIELD_Y         TXT_LABEL_Y
#define TXT_FIELD_WIDTH     65
#define TXT_FIELD_HEIGHT    26
#define TXT_SPACING_X       10
#define TXT_SPACING_Y       TXT_FIELD_HEIGHT

#define T_SELECT_X           205
#define T_SELECT_Y           155
#define T_CLEAR_X            265
#define T_CLEAR_Y            T_SELECT_Y
#define T_T_OFF_X            6
#define T_T_OFF_Y            15
#define CO_BOXSIZE_X         50
#define CO_BOXSIZE_Y         24

#define D_SELECT_X           T_SELECT_X
#define D_SELECT_Y           181
#define D_CLEAR_X            T_CLEAR_X
#define D_CLEAR_Y            D_SELECT_Y
#define D_T_OFF_X            T_T_OFF_X
#define D_T_OFF_Y            T_T_OFF_Y

#define U_SELECT_X           T_SELECT_X
#define U_SELECT_Y           207
#define U_CLEAR_X            T_CLEAR_X
#define U_CLEAR_Y            U_SELECT_Y
#define U_T_OFF_X            T_T_OFF_X
#define U_T_OFF_Y            T_T_OFF_Y

#define LA_SELECT_X          T_SELECT_X
#define LA_SELECT_Y          233
#define LA_CLEAR_X           T_CLEAR_X
#define LA_CLEAR_Y           LA_SELECT_Y
#define LA_T_OFF_X           T_T_OFF_X
#define LA_T_OFF_Y           T_T_OFF_Y

#define LO_SELECT_X          T_SELECT_X
#define LO_SELECT_Y          259
#define LO_CLEAR_X           T_CLEAR_X
#define LO_CLEAR_Y           LO_SELECT_Y
#define LO_T_OFF_X           T_T_OFF_X
#define LO_T_OFF_Y           T_T_OFF_Y

#define S_SEND_BUTTON_X      215
#define S_SEND_BUTTON_Y      285
#define S_SEND_T_OFF_X       23
#define S_SEND_T_OFF_Y       20
#define S_SEND_BOXSIZE_X     80
#define S_SEND_BOXSIZE_Y     30

#define SITE_BUTTON_X        210
#define SITE_BUTTON_Y        321
#define SITE_T_OFF_X         5
#define SITE_T_OFF_Y         13
#define SITE_BOXSIZE_X       90
#define SITE_BOXSIZE_Y       20

#define S_CMD_ERR_X          190
#define S_CMD_ERR_Y          300
#define S_CMD_ERR_W          159
#define S_CMD_ERR_H          19

#define TDU_DISP_X           156
#define TDU_DISP_Y           355
#define TDU_OFFSET_X         80
#define TDU_OFFSET_Y         20

#define CUSTOM_FONT_OFFSET  -15

// Draw the SETTINGS Page
void SettingsScreen::draw() {
  display.setDayNight();
  tft.setTextColor(display.textColor);
  tft.fillScreen(display.pgBackground);
  display.currentScreen = SETTINGS_SCREEN;
  display.drawTitle(100, 30, "Settings");
  display.drawMenuButtons();
  display.drawCommonStatusLabels(); // status common to many pages

  TtextIndex = 0; 
  DtextIndex = 0; 
  TztextIndex = 0; 
  LaTextIndex = 0; 
  LoTextIndex = 0; 

  // Draw Entry Field Labels
  tft.setCursor(TXT_LABEL_X, TXT_LABEL_Y);
  tft.print("Time 24h(hhmmss):");
  tft.setCursor(TXT_LABEL_X, TXT_LABEL_Y+TXT_SPACING_Y);
  tft.print(" Date (mm/dd/yy):");
  tft.setCursor(TXT_LABEL_X, TXT_LABEL_Y+TXT_SPACING_Y*2);
  tft.print(" Time Zone (sHH):");
  tft.setCursor(TXT_LABEL_X, TXT_LABEL_Y+TXT_SPACING_Y*3);
  tft.print("Latitude (sXX.X):");
  tft.setCursor(TXT_LABEL_X, TXT_LABEL_Y+TXT_SPACING_Y*4);
  tft.print("Longitud(sXXX.X):");

  tft.setCursor(TDU_DISP_X, TDU_DISP_Y);
  tft.print("     Time:");
  tft.setCursor(TDU_DISP_X, TDU_DISP_Y+TDU_OFFSET_Y);
  tft.print("     Date:");
  tft.setCursor(TDU_DISP_X, TDU_DISP_Y+TDU_OFFSET_Y*2);
  tft.print("       TZ:");
  tft.setCursor(TDU_DISP_X, TDU_DISP_Y+TDU_OFFSET_Y*3);
  tft.print(" Latitude:");
  tft.setCursor(TDU_DISP_X, TDU_DISP_Y+TDU_OFFSET_Y*4);
  tft.print("Longitude:");

// Draw Key Pad
  int z=0;
  for(int i=0; i<4; i++) { 
    for(int j=0; j<3; j++) {
      int row=i; int col=j; 
      display.drawButton(PAD_BUTTON_X+col*(PAD_BUTTON_W+PAD_BUTTON_SPACING_X), 
              PAD_BUTTON_Y+row*(PAD_BUTTON_H+PAD_BUTTON_SPACING_Y), 
              PAD_BUTTON_W, PAD_BUTTON_H, false, PAD_T_OFF_X, PAD_T_OFF_Y, sNumLabels[z]);
      z++;
    }
  }

// Initialize TIME, DATE, TZ, Latitude, Longitude Enter/Accept buttons
  display.drawButton(T_SELECT_X,   T_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, T_T_OFF_X, T_T_OFF_Y, "TmSel");
  display.drawButton( T_CLEAR_X,    T_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, T_T_OFF_X, T_T_OFF_Y, "TmClr"); 
  display.drawButton(D_SELECT_X,   D_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, D_T_OFF_X, D_T_OFF_Y, "DaSel");
  display.drawButton( D_CLEAR_X,    D_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, D_T_OFF_X, D_T_OFF_Y, "DaClr"); 
  display.drawButton(U_SELECT_X,   U_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, U_T_OFF_X, U_T_OFF_Y, "TzSel");
  display.drawButton( U_CLEAR_X,    U_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, U_T_OFF_X, U_T_OFF_Y, "TzClr"); 
  display.drawButton(LA_SELECT_X, LA_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, U_T_OFF_X, U_T_OFF_Y, "LaSel");
  display.drawButton( LA_CLEAR_X,  LA_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, U_T_OFF_X, U_T_OFF_Y, "LaClr"); 
  display.drawButton(LO_SELECT_X, LO_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, U_T_OFF_X, U_T_OFF_Y, "LoSel");
  display.drawButton( LO_CLEAR_X,  LO_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, U_T_OFF_X, U_T_OFF_Y, "LoClr"); 
  
  // Send data button
  display.drawButton(S_SEND_BUTTON_X, S_SEND_BUTTON_Y, S_SEND_BOXSIZE_X, S_SEND_BOXSIZE_Y, false, S_SEND_T_OFF_X, S_SEND_T_OFF_Y, "Send"); 
  
  // Initialize the background for the Text Entry fields
  tft.fillRect(TXT_FIELD_X, TXT_FIELD_Y+                CUSTOM_FONT_OFFSET, TXT_FIELD_WIDTH, TXT_FIELD_HEIGHT-8,  display.butBackground);
  tft.fillRect(TXT_FIELD_X, TXT_FIELD_Y+TXT_SPACING_Y+  CUSTOM_FONT_OFFSET, TXT_FIELD_WIDTH, TXT_FIELD_HEIGHT-8,  display.butBackground);
  tft.fillRect(TXT_FIELD_X, TXT_FIELD_Y+TXT_SPACING_Y*2+CUSTOM_FONT_OFFSET, TXT_FIELD_WIDTH, TXT_FIELD_HEIGHT-8,  display.butBackground);
  tft.fillRect(TXT_FIELD_X, TXT_FIELD_Y+TXT_SPACING_Y*3+CUSTOM_FONT_OFFSET, TXT_FIELD_WIDTH, TXT_FIELD_HEIGHT-8,  display.butBackground);
  tft.fillRect(TXT_FIELD_X, TXT_FIELD_Y+TXT_SPACING_Y*4+CUSTOM_FONT_OFFSET, TXT_FIELD_WIDTH, TXT_FIELD_HEIGHT-8,  display.butBackground);
} // end initialize

// Numpad handler, assign label to pressed button field array slot
void SettingsScreen::setProcessNumPadButton() {
  if (sNumDetected) {
    // Time Format: [HHMMSS]
    if (Tselect && TtextIndex < 6 && (sButtonPosition != 9 || sButtonPosition != 11)) {
      Ttext[TtextIndex] = sNumLabels[sButtonPosition][0];
      tft.fillRect(TXT_FIELD_X, TXT_FIELD_Y+CUSTOM_FONT_OFFSET, TXT_FIELD_WIDTH, TXT_FIELD_HEIGHT-8,  display.butBackground);
      tft.setCursor(TXT_FIELD_X, TXT_FIELD_Y);
      tft.print(Ttext); 
      TtextIndex++;
    }

    // Date Format: [DDMMYY] last two digits of year
    if (Dselect && DtextIndex < 6 && (sButtonPosition != 9 || sButtonPosition != 11)) {
      Dtext[DtextIndex] = sNumLabels[sButtonPosition][0]; 
      tft.fillRect(TXT_FIELD_X, TXT_FIELD_Y+TXT_SPACING_Y+CUSTOM_FONT_OFFSET, TXT_FIELD_WIDTH, TXT_FIELD_HEIGHT-8,  display.butBackground);
      tft.setCursor(TXT_FIELD_X, TXT_FIELD_Y+TXT_SPACING_Y);
      tft.print(Dtext);
      DtextIndex++;
    }

    // Time Zone Format: [sDD] first character must be a +/- and allow only 2 additional chars
    if (Tzselect && (((TztextIndex == 0 && (sButtonPosition == 9 || sButtonPosition == 11)) 
          || (TztextIndex>0 && (sButtonPosition!=9||sButtonPosition!=11)))) && TztextIndex < 3) { 
      Tztext[TztextIndex] = sNumLabels[sButtonPosition][0]; 
      tft.fillRect(TXT_FIELD_X, TXT_FIELD_Y+(TXT_SPACING_Y*2)+CUSTOM_FONT_OFFSET, TXT_FIELD_WIDTH, TXT_FIELD_HEIGHT-8,  display.butBackground);
      tft.setCursor(TXT_FIELD_X, TXT_FIELD_Y+(TXT_SPACING_Y*2));
      tft.print(Tztext);
      TztextIndex++;
    }

    //Latitude Format: [sDD.D] first character must be a +/- and require 3 additional chars. Automatically put decimal in after first 2
    if (LaSelect && (((LaTextIndex == 0 && (sButtonPosition == 9 || sButtonPosition == 11)) 
        || (LaTextIndex>0 && (sButtonPosition!=9||sButtonPosition!=11)))) && LaTextIndex < 4) { //
      LaText[LaTextIndex] = sNumLabels[sButtonPosition][0]; 

      tft.fillRect(TXT_FIELD_X, TXT_FIELD_Y+(TXT_SPACING_Y*3)+CUSTOM_FONT_OFFSET, TXT_FIELD_WIDTH, TXT_FIELD_HEIGHT-8,  display.butBackground);
      tft.setCursor(TXT_FIELD_X, TXT_FIELD_Y+(TXT_SPACING_Y*3));
      if (LaSelect && LaTextIndex == 3) {
        sprintf(LaText, "%c%c%c.%c", LaText[0], LaText[1], LaText[2], LaText[3]);
      }
      tft.print(LaText);
      LaTextIndex++;
      if (LaSelect && LaTextIndex == 3) tft.print(".");
    }

    //Longitude Format: [sDDD.D] first character must be a +/- and require 4 additional chars. Automatically put decimal in after first 3
    if (LoSelect && (((LoTextIndex == 0 && (sButtonPosition == 9 || sButtonPosition == 11)) 
      || (LoTextIndex>0 && (sButtonPosition!=9||sButtonPosition!=11)))) && LoTextIndex < 5) {
      LoText[LoTextIndex] = sNumLabels[sButtonPosition][0]; 
      
      tft.fillRect(TXT_FIELD_X, TXT_FIELD_Y+(TXT_SPACING_Y*4)+CUSTOM_FONT_OFFSET, TXT_FIELD_WIDTH, TXT_FIELD_HEIGHT-8,  display.butBackground);
      tft.setCursor(TXT_FIELD_X, TXT_FIELD_Y+(TXT_SPACING_Y*4));
      if (LoSelect && LoTextIndex == 4) {
        sprintf(LoText, "%c%c%c%c.%c", LoText[0], LoText[1], LoText[2], LoText[3], LoText[4]);
      }
      tft.print(LoText);
      LoTextIndex++;
      if (LoSelect && LoTextIndex == 4) tft.print(".");
    }
    sNumDetected = false;
  }
  //VF("sButtonPosition="); VL(sButtonPosition);
  //VF("Ttext="); VL(Ttext);
  //VF("Dtext="); VL(Dtext);
  //VF("Tztext="); VL(Tztext);
  //VF("TtextIndex="); VL(TtextIndex);
  //VF("DtextIndex="); VL(DtextIndex);
  //VF("TztextIndex="); VL(TztextIndex); VLF("");
}

// combine all updates for this Screen
void SettingsScreen::updateStatusAll() {
  settingsScreen.updateStatus();
  display.updateCommonStatus();
  display.updateOnStepCmdStatus();
}

void SettingsScreen::updateStatus() {
    
    // process number pad buttons
    if (display.screenTouched || display.firstDraw || display.refreshScreen) {
      if (display.screenTouched) display.refreshScreen = true; else
      if (display.refreshScreen) display.refreshScreen = false; // cleans up state change for button presses
        
        // Get button and print label
        switch (sButtonPosition) {
        case 0:  setProcessNumPadButton(); break;
        case 1:  setProcessNumPadButton(); break;
        case 2:  setProcessNumPadButton(); break;
        case 3:  setProcessNumPadButton(); break;
        case 4:  setProcessNumPadButton(); break;
        case 5:  setProcessNumPadButton(); break;
        case 6:  setProcessNumPadButton(); break;
        case 7:  setProcessNumPadButton(); break;
        case 8:  setProcessNumPadButton(); break;
        case 9:  setProcessNumPadButton(); break;
        case 10: setProcessNumPadButton(); break;
        case 11: setProcessNumPadButton(); break;
        default: break;
        }

    // Time Select Button
    if (Tselect) {
      display.drawButton( T_SELECT_X,  T_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, true, T_T_OFF_X,  T_T_OFF_Y,  "TiSel");
    } else {
      display.drawButton( T_SELECT_X,  T_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, T_T_OFF_X,  T_T_OFF_Y,  "TiSel");
    }

    // Time Clear button
    if (Tclear) {
      display.drawButton(  T_CLEAR_X,   T_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, true, T_T_OFF_X,  T_T_OFF_Y,  "TiClr");
      tft.fillRect(TXT_FIELD_X, TXT_FIELD_Y+CUSTOM_FONT_OFFSET, TXT_FIELD_WIDTH, TXT_FIELD_HEIGHT-9,  display.butBackground);
      memset(Ttext,0,sizeof(Ttext)); // clear Time buffer
      tft.fillRect(S_CMD_ERR_X, S_CMD_ERR_Y+CUSTOM_FONT_OFFSET, S_CMD_ERR_W, S_CMD_ERR_H, display.pgBackground);
      TtextIndex = 0;
      sButtonPosition = 12;
      Tclear = false;
    } else {
      display.drawButton(T_CLEAR_X, T_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, T_T_OFF_X,  T_T_OFF_Y,  "TiClr");
    }
    
    // Date Select button
    if (Dselect) {
      display.drawButton(D_SELECT_X, D_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, true, D_T_OFF_X, D_T_OFF_Y, "DaSel");
    } else {
      display.drawButton(D_SELECT_X, D_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, D_T_OFF_X, D_T_OFF_Y, "DaSel"); 
    }

    // Date Clear Button
    if (Dclear) {
      display.drawButton( D_CLEAR_X,  D_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, true, D_T_OFF_X, D_T_OFF_Y, "DaClr"); 
      tft.fillRect(TXT_FIELD_X, TXT_FIELD_Y+CUSTOM_FONT_OFFSET+TXT_SPACING_Y, TXT_FIELD_WIDTH, TXT_FIELD_HEIGHT-9,  display.butBackground);
      memset(Dtext,0,sizeof(Dtext)); // clear DATE buffer
      tft.fillRect(S_CMD_ERR_X, S_CMD_ERR_Y+CUSTOM_FONT_OFFSET, S_CMD_ERR_W, S_CMD_ERR_H, display.pgBackground);
      DtextIndex = 0;
      sButtonPosition = 12;
      Dclear = false;
    } else {
      display.drawButton( D_CLEAR_X,  D_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, D_T_OFF_X, D_T_OFF_Y, "DaClr"); 
    }

    // Time Zone Select button
    if (Tzselect) {
      display.drawButton(U_SELECT_X, U_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, true, U_T_OFF_X, U_T_OFF_Y, "TzSel");
    } else {
      display.drawButton(U_SELECT_X, U_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, U_T_OFF_X, U_T_OFF_Y, "TzSel"); 
    }

    // Time Zone Clear Button update
    if (Tzclear) {
      display.drawButton( U_CLEAR_X,  U_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, true, U_T_OFF_X, U_T_OFF_Y, "TzClr"); 
      tft.fillRect(TXT_FIELD_X, TXT_FIELD_Y+CUSTOM_FONT_OFFSET+TXT_SPACING_Y*2, TXT_FIELD_WIDTH, TXT_FIELD_HEIGHT-9,  display.butBackground);
      memset(Tztext,0,sizeof(Tztext)); // clear TZ buffer
      tft.fillRect(S_CMD_ERR_X, S_CMD_ERR_Y+CUSTOM_FONT_OFFSET, S_CMD_ERR_W, S_CMD_ERR_H, display.pgBackground);
      TztextIndex = 0;
      sButtonPosition = 12;
      Tzclear = false;
    } else {
      display.drawButton( U_CLEAR_X,  U_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, U_T_OFF_X, U_T_OFF_Y, "TzClr"); 
    }

    // Latitude Select button
    if (LaSelect) {
      display.drawButton(LA_SELECT_X, LA_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, true, U_T_OFF_X, U_T_OFF_Y, "LaSel");
    } else {
      display.drawButton(LA_SELECT_X, LA_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, U_T_OFF_X, U_T_OFF_Y, "LaSel"); 
    }

    // Latitude Clear Button update
    if (LaClear) {
      display.drawButton( LA_CLEAR_X,  LA_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, true, U_T_OFF_X, U_T_OFF_Y, "LaClr"); 
      tft.fillRect(TXT_FIELD_X, TXT_FIELD_Y+CUSTOM_FONT_OFFSET+TXT_SPACING_Y*3, TXT_FIELD_WIDTH, TXT_FIELD_HEIGHT-9,  display.butBackground);
      memset(LaText,0,sizeof(LaText)); // clear latitude buffer
      tft.fillRect(S_CMD_ERR_X, S_CMD_ERR_Y+CUSTOM_FONT_OFFSET, S_CMD_ERR_W, S_CMD_ERR_H, display.pgBackground);
      LaTextIndex = 0;
      sButtonPosition = 12;
      LaClear = false;
    } else {
      display.drawButton( LA_CLEAR_X,  LA_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, U_T_OFF_X, U_T_OFF_Y, "LaClr"); 
    }

    // Longitude Select button
    if (LoSelect) {
      display.drawButton(LO_SELECT_X, LO_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, true, U_T_OFF_X, U_T_OFF_Y, "LoSel");
    } else {
      display.drawButton(LO_SELECT_X, LO_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, U_T_OFF_X, U_T_OFF_Y, "LoSel"); 
    }

    // Longitude Clear Button update
    if (LoClear) {
      display.drawButton( LO_CLEAR_X,  LO_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, true, U_T_OFF_X, U_T_OFF_Y, "LoClr"); 
      tft.fillRect(TXT_FIELD_X, TXT_FIELD_Y+CUSTOM_FONT_OFFSET+TXT_SPACING_Y*4, TXT_FIELD_WIDTH, TXT_FIELD_HEIGHT-9,  display.butBackground);
      memset(LoText,0,sizeof(LoText)); // clear longitude buffer
      tft.fillRect(S_CMD_ERR_X, S_CMD_ERR_Y+CUSTOM_FONT_OFFSET, S_CMD_ERR_W, S_CMD_ERR_H, display.pgBackground);
      LoTextIndex = 0;
      sButtonPosition = 12;
      LoClear = false;
    } else {
      display.drawButton( LO_CLEAR_X,  LO_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, false, U_T_OFF_X, U_T_OFF_Y, "LoClr"); 
    }
    
    // Send Data Button update
    if (sSendOn) {
      display.drawButton(S_SEND_BUTTON_X, S_SEND_BUTTON_Y, S_SEND_BOXSIZE_X, S_SEND_BOXSIZE_Y, true, S_SEND_T_OFF_X, S_SEND_T_OFF_Y, "Sent");
      sSendOn = false; 
    } else {
      display.drawButton(S_SEND_BUTTON_X, S_SEND_BUTTON_Y, S_SEND_BOXSIZE_X, S_SEND_BOXSIZE_Y, false, S_SEND_T_OFF_X, S_SEND_T_OFF_Y, "Send"); 
    }

    // Get and show the Time and Location status
    char tempReply[10];
    // show Local Time 24 Hr format
    display.getLocalCmdTrim(":GL#", tempReply); 
    tempReply[8] = 0; // clear # character
    display.canvPrint(TDU_DISP_X+TDU_OFFSET_X, TDU_DISP_Y, 0, 80, 16, tempReply);

    // show Current Date
    display.getLocalCmdTrim(":GC#", tempReply); 
    tempReply[8] = 0; // clear # character
    display.canvPrint(TDU_DISP_X+TDU_OFFSET_X, TDU_DISP_Y+TDU_OFFSET_Y, 0, 80, 16, tempReply);

    // show TZ Offset
    display.getLocalCmdTrim(":GG#", tempReply); 
    tempReply[3] = 0; // clear # character
    display.canvPrint(TDU_DISP_X+TDU_OFFSET_X, TDU_DISP_Y+TDU_OFFSET_Y*2, 0, 80, 16, tempReply);

    // show Latitude
    display.getLocalCmdTrim(":Gt#", tempReply); 
    tempReply[6] = 0; // clear # character
    display.canvPrint(TDU_DISP_X+TDU_OFFSET_X, TDU_DISP_Y+TDU_OFFSET_Y*3, 0, 80, 16, tempReply);

    // show Longitude
    display.getLocalCmdTrim(":Gg#", tempReply); 
    tempReply[7] = 0; // clear # character
    display.canvPrint(TDU_DISP_X+TDU_OFFSET_X, TDU_DISP_Y+TDU_OFFSET_Y*4, 0, 80, 16, tempReply);

    display.screenTouched = false;
  }
}

// **** TouchScreen was touched, determine which button *****
void SettingsScreen::touchPoll() {
  
  //were number Pad buttons pressed?
  for(int i=0; i<4; i++) { 
    for(int j=0; j<3; j++) {
      int row=i; int col=j; 
      if (p.y >   PAD_BUTTON_Y+row*(PAD_BUTTON_H+PAD_BUTTON_SPACING_Y) 
       && p.y <  (PAD_BUTTON_Y+row*(PAD_BUTTON_H+PAD_BUTTON_SPACING_Y)) + PAD_BUTTON_H 
       && p.x >   PAD_BUTTON_X+col*(PAD_BUTTON_W+PAD_BUTTON_SPACING_X) 
       && p.x <  (PAD_BUTTON_X+col*(PAD_BUTTON_W+PAD_BUTTON_SPACING_X) + PAD_BUTTON_W)) {
        status.sound.beep();
        sButtonPosition=row*3+col;
        //VF("sButtonPosition="); VL(sButtonPosition);
        sNumDetected = true;
      }
    }
  }

  // Select Time field
  if (p.y > T_SELECT_Y && p.y < (T_SELECT_Y + CO_BOXSIZE_Y) && p.x > T_SELECT_X && p.x < (T_SELECT_X + CO_BOXSIZE_X)) {
    status.sound.beep();
    Tselect = true;
    Dselect = false;
    Tzselect = false;
    LaSelect = false;
    LoSelect = false;
  }

  // Clear Time field
  if (p.y > T_CLEAR_Y && p.y < (T_CLEAR_Y + CO_BOXSIZE_Y) && p.x > T_CLEAR_X && p.x < (T_CLEAR_X + CO_BOXSIZE_X)) {
    status.sound.beep();
    Tclear = true; 
    TtextIndex = 0;
    sButtonPosition = 12; 
  }

  // Select Date field
  if (p.y > D_SELECT_Y && p.y < (D_SELECT_Y + CO_BOXSIZE_Y) && p.x > D_SELECT_X && p.x < (D_SELECT_X + CO_BOXSIZE_X)) {
    status.sound.beep();
    Tselect = false;
    Dselect = true;
    Tzselect = false;
    LaSelect = false;
    LoSelect = false;
  }

  // Clear DEC field
  if (p.y > D_CLEAR_Y && p.y < (D_CLEAR_Y + CO_BOXSIZE_Y) && p.x > D_CLEAR_X && p.x < (D_CLEAR_X + CO_BOXSIZE_X)) {
    status.sound.beep();
    Dclear = true; 
    DtextIndex = 0;
    sButtonPosition = 12; 
  }

  // Select TZ field
  if (p.y > U_SELECT_Y && p.y < (U_SELECT_Y + CO_BOXSIZE_Y) && p.x > U_SELECT_X && p.x < (U_SELECT_X + CO_BOXSIZE_X)) {
    status.sound.beep();
    Tselect = false;
    Dselect = false;
    Tzselect = true;
    LaSelect = false;
    LoSelect = false;
  }

  // Clear TZ field
  if (p.y > U_CLEAR_Y && p.y < (U_CLEAR_Y + CO_BOXSIZE_Y) && p.x > U_CLEAR_X && p.x < (U_CLEAR_X + CO_BOXSIZE_X)) {
    status.sound.beep();
    Tzclear = true; 
    TztextIndex = 0;
    sButtonPosition = 12; 
  }

  // Select Latitude field
  if (p.y > LA_SELECT_Y && p.y < (LA_SELECT_Y + CO_BOXSIZE_Y) && p.x > LA_SELECT_X && p.x < (LA_SELECT_X + CO_BOXSIZE_X)) {
    status.sound.beep();
    Tselect = false;
    Dselect = false;
    Tzselect = false;
    LaSelect = true;
    LoSelect = false;
  }

  // Clear Latitude field
  if (p.y > LA_CLEAR_Y && p.y < (LA_CLEAR_Y + CO_BOXSIZE_Y) && p.x > LA_CLEAR_X && p.x < (LA_CLEAR_X + CO_BOXSIZE_X)) {
    status.sound.beep();
    LaClear = true; 
    LaTextIndex = 0;
    sButtonPosition = 12; 
  }

// Select Longitude field
  if (p.y > LO_SELECT_Y && p.y < (LO_SELECT_Y + CO_BOXSIZE_Y) && p.x > LO_SELECT_X && p.x < (LO_SELECT_X + CO_BOXSIZE_X)) {
    status.sound.beep();
    Tselect = false;
    Dselect = false;
    Tzselect = false;
    LaSelect = false;
    LoSelect = true;
  }

  // Clear Longitude field
  if (p.y > LO_CLEAR_Y && p.y < (LO_CLEAR_Y + CO_BOXSIZE_Y) && p.x > LO_CLEAR_X && p.x < (LO_CLEAR_X + CO_BOXSIZE_X)) {
    status.sound.beep();
    LoClear = true; 
    LoTextIndex = 0;
    sButtonPosition = 12; 
  }

  // SEND Data
  if (p.y > S_SEND_BUTTON_Y && p.y < (S_SEND_BUTTON_Y + S_SEND_BOXSIZE_Y) && p.x > S_SEND_BUTTON_X && p.x < (S_SEND_BUTTON_X + S_SEND_BOXSIZE_X)) {
    status.sound.beep();
    sSendOn = true; 
    TtextIndex  = 0;
    DtextIndex  = 0;
    TztextIndex  = 0;
    LaTextIndex = 0;
    LoTextIndex = 0;
    sButtonPosition = 12; 
    static char sLaMin[3] = "";
    static char sLoMin[3] = "";
    
    if (Tselect) {
      // Set Local Time :SL[HH:MM:SS]# 24Hr format
      sprintf(sCmd, ":SL%c%c:%c%c:%c%c#", Ttext[0], Ttext[1], Ttext[2], Ttext[3], Ttext[4], Ttext[5]);
      display.setLocalCmd(sCmd);
      //if (commandError == 1) {
      //    tft.setCursor(S_CMD_ERR_X, S_CMD_ERR_Y);
      //    tft.fillRect(S_CMD_ERR_X, S_CMD_ERR_Y-16, 150, 20, pgBackground);
      //    tft.print("Time String Accepted");
      //} else {
        cat_mgr.setLstT0(shc.getLstT0());
     // }
    } else if (Dselect) { // :SC[MM/DD/YY]# 
      sprintf(sCmd, ":SC%c%c/%c%c/%c%c#", Dtext[0], Dtext[1], Dtext[2], Dtext[3], Dtext[4], Dtext[5]);
      display.setLocalCmd(sCmd);
      //if (commandError == 1) {
     //     tft.setCursor(S_CMD_ERR_X, S_CMD_ERR_Y);
     //     tft.fillRect(S_CMD_ERR_X, S_CMD_ERR_Y-16, 150, 20, pgBackground);
      //    tft.print("Date String Accepted");
      //} else {
      cat_mgr.setLstT0(shc.getLstT0());
     // }
    } else if (Tzselect) { // :SG[sHH]#
      sprintf(sCmd, ":SG%c%c%c#", Tztext[0], Tztext[1], Tztext[2]);
      display.setLocalCmd(sCmd);
     // if (commandError == 1) {
     //     tft.setCursor(S_CMD_ERR_X, S_CMD_ERR_Y);
      //    tft.fillRect(S_CMD_ERR_X, S_CMD_ERR_Y-16, 150, 20, pgBackground);
       //   tft.print("TZ String Accepted ");
      //} else {
      cat_mgr.setLstT0(shc.getLstT0());
      //}
    } else if (LaSelect) { // :St[sDD*MM]#
      uint8_t laMin = LaText[4] - '0';  //digit after decimal point
      sprintf(sLaMin, "%02d\n", laMin * 6); // convert fractional degrees to string minutes
      sprintf(sCmd, ":St%c%c%c*%2s#", LaText[0], LaText[1], LaText[2], sLaMin);
      display.setLocalCmd(sCmd);
     // if (commandError == 1) {
     //     tft.setCursor(S_CMD_ERR_X, S_CMD_ERR_Y);
      //    tft.fillRect(S_CMD_ERR_X, S_CMD_ERR_Y-16, 150, 20, pgBackground);
       //   tft.print("Lat String Accepted ");
    //  } else {
        cat_mgr.setLat(shc.getLat());
        cat_mgr.setLstT0(shc.getLstT0());
     // }
    } else if (LoSelect) { // :Sg[(s)DDD*MM]#
      uint8_t loMin = LoText[5] - '0'; //digit after decimal point
      sprintf(sLoMin, "%02d\n", loMin * 6); // convert fractional degrees to string minutes
      sprintf(sCmd, ":Sg%c%c%c%c*%2s#", LoText[0], LoText[1], LoText[2], LoText[3], sLoMin);
      display.setLocalCmd(sCmd);
    //  if (commandError == 1) {
    //      tft.setCursor(S_CMD_ERR_X, S_CMD_ERR_Y);
    //      tft.fillRect(S_CMD_ERR_X, S_CMD_ERR_Y-16, 150, 20, pgBackground);
    //      tft.print("Long String Accepted ");
    //  } else {
      cat_mgr.setLstT0(shc.getLstT0());
    //  }
    }
  }
}

SettingsScreen settingsScreen;
