// =====================================================
// CustomCatScreen.cpp
// Custom User Catalog Screen
// This catalog can hold various saved objects and their coordinates from other catalogs
//
// Author: Richard Benear 6/22

#include "CustomCatScreen.h"
#include "MoreScreen.h"
//#include "../display/Display.h"
#include "../catalog/Catalog.h"
#include "../catalog/CatalogTypes.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "src/telescope/mount/Mount.h"

custom_t _cArray[MAX_CUSTOM_ROWS];

// Catalog Button object
Button customCatButton(
  0,0,0,0,
  display.butOnBackground, 
  display.butBackground, 
  display.butOutline, 
  display.mainFontWidth, 
  display.mainFontHeight, 
  "");

// have a separate set of values for Custom Catalog for better spacing/visibility since it will be used more frequently
#define CUS_X               1
#define CUS_Y               43
#define CUS_W               112
#define CUS_H               24
#define CUS_Y_SPACING       2
#define CUS_TEXT_X_OFF      3
#define CUS_TEXT_Y_OFF      12

#define BACK_X              5
#define BACK_Y              445
#define BACK_W              60
#define BACK_H              35

#define NEXT_X              255
#define NEXT_Y              BACK_Y

#define RETURN_X            165
#define RETURN_Y            BACK_Y
#define RETURN_W            80

#define SAVE_LIB_X          75
#define SAVE_LIB_Y          BACK_Y
#define SAVE_LIB_W          80
#define SAVE_LIB_H          BACK_H

#define STATUS_STR_X        3
#define STATUS_STR_Y        430
#define STATUS_STR_W        150
#define STATUS_STR_H        16

#define WIDTH_OFF           40 // negative width offset
#define SUB_STR_X_OFF       2
#define FONT_Y_OFF          7

void CustomCatScreen::init() { 
  returnToPage = display.currentScreen; // save page from where this function was called so we can return
  setCurrentScreen(CUSTOM_SCREEN);
  setNightMode(getNightMode());
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  moreScreen.objectSelected = false;
  cCurrentPage = 0;
  cPrevPage = 0;
  cEndOfList = false;
 
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);

  drawTitle(83, TITLE_TEXT_Y, "User Catalog");
  customCatalog = true;

  if (!loadCustomArray()) {
      canvPrint(STATUS_STR_X, STATUS_STR_Y, -6, STATUS_STR_W, STATUS_STR_H, "ERR:Loading Custom"); 
      moreScreen.draw();
      return;
  } else {
      parseCcatIntoArray();
  }
  cAbsRow = 0; // initialize the absolute index into total array
  
  // Draw the Trash can/Delete Icon bitmap; used to delete an entry in only the CUSTOM USER catalog
  uint8_t extern trash_icon[];
  tft.drawBitmap(270, 5, trash_icon, 28, 32, butBackground, ORANGE);

  drawCustomCat(); // draw first page of the selected catalog

  tft.setFont(&Inconsolata_Bold8pt7b);
  //updateCatalogButtons(false); // initial draw of buttons
  customCatButton.draw(BACK_X, BACK_Y, BACK_W, BACK_H, "BACK", BUT_OFF);
  customCatButton.draw(NEXT_X, NEXT_Y, BACK_W, BACK_H, "NEXT", BUT_OFF);
  customCatButton.draw(RETURN_X, RETURN_Y, RETURN_W, BACK_H, "RETURN", BUT_OFF);
  customCatButton.draw(SAVE_LIB_X, SAVE_LIB_Y, SAVE_LIB_W, SAVE_LIB_H, "SAVE LIB", BUT_OFF);
}

// The Custom catalog is a selection of User objects that have been saved on the SD card.
//      When using any of the catalogs, the SaveToCat button will store the objects info
//      in the Custom Catalog.
// Load the custom.csv file from SD into an RAM array of text lines
// success = 1, fail = 0
bool CustomCatScreen::loadCustomArray() {
  //============== Load from Custom File ==========================
  // Load RAM array from SD catalog file
  char rdChar;
  char rowString[SD_CARD_LINE_LEN]=""; // temp row buffer
  int rowNum=0;
  int charNum=0;
  File rdFile = SD.open("custom.csv");

  if (rdFile) {
    // load data from SD into array
    while (rdFile.available()) {
      rdChar = rdFile.read();
      rowString[charNum] = rdChar;
      charNum++;
      if (rdChar == '\n') {
          strcpy(Custom_Array[rowNum], rowString);
          strcpy(Copy_Custom_Array[rowNum], rowString);
          rowNum++;
          memset(&rowString, 0, SD_CARD_LINE_LEN);
          charNum = 0;
      }
    }
    cusRowEntries = rowNum-1; // actual number since rowNum was already incremented; start at 0
  } else {
    //VLF("SD Cust read error");
    return false;
  }
  rdFile.close();
  //VLF("SD Cust read done");
  return true;
}

// Parse the custom.csv line data into individual fields in an array
// custom.csv file format = ObjName;Mag;Cons;ObjType;SubId;cRahhmmss;cDecsddmmss\n
void CustomCatScreen::parseCcatIntoArray() {
  for (int i=0; i<=cusRowEntries; i++) {
    _cArray[i].cObjName      = strtok(Custom_Array[i], ";"); //VL(_cArray[i].cObjName);
    _cArray[i].cMag          = strtok(NULL, ";");            //VL(_cArray[i].cMag);
    _cArray[i].cCons         = strtok(NULL, ";");            //VL(_cArray[i].cCons);
    _cArray[i].cObjType      = strtok(NULL, ";");            //VL(_cArray[i].cObjType);
    _cArray[i].cSubId        = strtok(NULL, ";");            //VL(_cArray[i].cSubId);
    _cArray[i].cRAhhmmss     = strtok(NULL, ";");            //VL(_cArray[i].cRAhhmmss)
    _cArray[i].cDECsddmmss   = strtok(NULL, "\n");           //VL(_cArray[i].cDECsddmmss);
  }
}

// ========== draw CUSTOM Screen of catalog data ========
void CustomCatScreen::drawCustomCat() {
  cRow = 0;
  pre_cAbsIndex=0;
  char catLine[47]=""; //hold the string that is displayed beside the button on each page

  // Show Page number and total Pages
  tft.fillRect(6, 9, 70, 12,  display.butBackground);
  tft.setFont(); // basic Arial font default
  tft.setTextSize(1);
  tft.setCursor(6, 9); 
  tft.printf("Page "); 
  tft.print(cCurrentPage+1);
  tft.printf(" of "); 
  if (moreScreen.activeFilter == FM_ABOVE_HORIZON) tft.print("??"); else tft.print(cLastPage); 
  tft.setCursor(6, 25); 
  tft.print(activeFilterStr[moreScreen.activeFilter]);
  tft.fillRect(0,60,319,358, pgBackground);

  cAbsRow = (cPagingArrayIndex[cCurrentPage]); // array of page 1st row indexes
  cLastPage = (cusRowEntries / NUM_CUS_ROWS_PER_SCREEN)+1;
  
  //VF("cusRowEntries="); VL(cusRowEntries);
  //VF("cLastPage="); VL(cLastPage);
  //VF("cCurPage="); VL(cCurrentPage);

  // Show Page number and total Pages
  tft.fillRect(6, 9, 70, 12, display.butBackground);
  tft.setCursor(6, 9); tft.printf("Page "); tft.print(cCurrentPage+1);
  tft.printf(" of "); if (moreScreen.activeFilter == FM_ABOVE_HORIZON) tft.print("??"); else tft.print(cLastPage+1);
  tft.setCursor(6, 25); tft.print(activeFilterStr[moreScreen.activeFilter]);

  while ((cRow < NUM_CUS_ROWS_PER_SCREEN) && (cAbsRow != MAX_CUSTOM_ROWS)) {  
    //VF("cAbsRow="); VL(cAbsRow);
    //VF("cRow="); VL(cRow);
    
    // ======== process RA/DEC ===========
    double cRAdouble;
    double cDECdouble;

    // RA in Hrs:Min:Sec
    snprintf(cRaSrCmd[cAbsRow], 16, ":Sr%11s#", _cArray[cAbsRow].cRAhhmmss);
    snprintf(cDecSrCmd[cAbsRow], 17, ":Sd%12s#", _cArray[cAbsRow].cDECsddmmss);

    // to get Altitude, first convert RAh and DEC to double
    convert.hmsToDouble(&cRAdouble, _cArray[cAbsRow].cRAhhmmss, PM_HIGH);
    convert.dmsToDouble(&cDECdouble, _cArray[cAbsRow].cDECsddmmss, true, PM_HIGH);
    
    //double cHAdouble=haRange(LST()*15.0-cRAdouble);
    cat_mgr.EquToHor(cRAdouble*15, cDECdouble, &dcAlt[cAbsRow], &dcAzm[cAbsRow]);

    //VF("activeFilter="); VL(activeFilter);
    if (((moreScreen.activeFilter == FM_ABOVE_HORIZON) && (dcAlt[cAbsRow] > 10.0)) || moreScreen.activeFilter == FM_NONE) { // filter out elements below 10 deg if filter enabled
      //VF("printing row="); VL(cAbsRow);
      // Erase text background
      tft.setCursor(CUS_X+CUS_W+2, CUS_Y+cRow*(CUS_H+CUS_Y_SPACING));
      tft.fillRect(CUS_X+CUS_W+2, CUS_Y+cRow*(CUS_H+CUS_Y_SPACING), 215, 17,  display.butBackground);

      // get object names and put them on the buttons
      customCatButton.draw(CUS_X, CUS_Y+cRow*(CUS_H+CUS_Y_SPACING), CUS_W, CUS_H, _cArray[cAbsRow].cObjName, BUT_OFF);
                  
      // format and print the text field for this row next to the button
      snprintf(catLine, 42, "%-4s |%-4s |%-9s |%-18s",  // 35 + 6 + NULL = 42
                                              _cArray[cAbsRow].cMag, 
                                              _cArray[cAbsRow].cCons, 
                                              _cArray[cAbsRow].cObjType, 
                                              _cArray[cAbsRow].cSubId);
      tft.setCursor(CUS_X+CUS_W+SUB_STR_X_OFF+2, CUS_Y+cRow*(CUS_H+CUS_Y_SPACING)+FONT_Y_OFF); 
      tft.print(catLine);
      cFiltArray[cRow] = cAbsRow;
      //VF("cFiltArray[cRow]="); VL(cFiltArray[cRow]);
      cRow++; // increments only through the number of lines displayed on screen per page
      //VF("ceRow="); VL(cRow);
    } 
    cPrevRowIndex = cAbsRow;
    cAbsRow++; // increments through all lines in the catalog

    // stop printing data if last row on the last page
    if (cAbsRow == cusRowEntries+1) {
      cEndOfList = true; 
      if (cRow == 0) {canvPrint(STATUS_STR_X, STATUS_STR_Y, 0, STATUS_STR_W, STATUS_STR_H, "None above 10 deg");}
      //VF("endOfList");
      return; 
    }
  }
  cPagingArrayIndex[cCurrentPage+1] = cAbsRow; // cPagingArrayIndex holds index of first element of page to help with NEXT and BACK paging
  //VF("PagingArrayIndex+1="); VL(cPagingArrayIndex[cCurrentPage+1]);
}

// show status changes on tasks timer
void CustomCatScreen::updateCustomStatus() {
  // do nothing currently
}

// redraw screen to show state change
bool CustomCatScreen::catalogButStateChange() {
  if (display._redrawBut) {
    display._redrawBut = false;
    return true;
  } else { 
    return false;
  }
}

//======================================================
// =====  Update Screen buttons and text ===========
//======================================================
void CustomCatScreen::updateCustomButtons(bool redrawBut) { 
  _redrawBut = redrawBut;  
  tft.setFont(); // basic Arial

  if (catButDetected) updateScreen();  

  if (saveTouched) {
    customCatButton.draw(SAVE_LIB_X, SAVE_LIB_Y, SAVE_LIB_W, SAVE_LIB_H, " Saving", BUT_ON);
  } else { 
    customCatButton.draw(SAVE_LIB_X, SAVE_LIB_Y, SAVE_LIB_W, SAVE_LIB_H, "SaveToCat", BUT_OFF);
  }
}

// ====== CUSTOM USER catalog ... uses different row spacing =====
void CustomCatScreen::updateScreen() {
  tft.setFont(&Inconsolata_Bold8pt7b);
  uint16_t cRelIndex = catButSelPos; // save the "screen/page" index of button pressed
  uint16_t cAbsIndex = cFiltArray[catButSelPos];

  if (cPrevPage == cCurrentPage) { //erase previous selection
    customCatButton.draw(CUS_X, CUS_Y+pre_cRelIndex*(CUS_H+CUS_Y_SPACING), CUS_W, CUS_H, _cArray[pre_cAbsIndex].cObjName, BUT_OFF); 
  }
  // highlight selected by settting background ON color 
  customCatButton.draw(CUS_X, CUS_Y+cRelIndex*(CUS_H+CUS_Y_SPACING), CUS_W, CUS_H, _cArray[cAbsIndex].cObjName, BUT_ON); 

  // the following 5 lines are displayed on the Catalog/More page
  snprintf(moreScreen.catSelectionStr1, 26, "Name-:%-19s", _cArray[cAbsIndex].cObjName);  //VF("c_objName="); //VL(_cArray[cAbsIndex].cObjName);
  snprintf(moreScreen.catSelectionStr2, 11, "Mag--:%-4s",  _cArray[cAbsIndex].cMag);      //VF("c_Mag=");     //VL(_cArray[cAbsIndex].cMag);
  snprintf(moreScreen.catSelectionStr3, 11, "Const:%-4s",  _cArray[cAbsIndex].cCons);     //VF("c_constel="); //VL(_cArray[cAbsIndex].cCons);
  snprintf(moreScreen.catSelectionStr4, 16, "Type-:%-9s",  _cArray[cAbsIndex].cObjType);  //VF("c_objType="); //VL(_cArray[cAbsIndex].cObjType);
  snprintf(moreScreen.catSelectionStr5, 15, "Id---:%-7s",  _cArray[cAbsIndex].cSubId);    //VF("c_subID=");   //VL(_cArray[cAbsIndex].cSubId);
  
  // show if we are above and below visible limits
  if (dcAlt[cAbsIndex] > 10.0) {      // minimum 10 degrees altitude
      canvPrint(STATUS_STR_X, STATUS_STR_Y, 0, STATUS_STR_W, STATUS_STR_H, "Above +10 deg");
  } else {
      canvPrint(STATUS_STR_X, STATUS_STR_Y, 0, STATUS_STR_W, STATUS_STR_H, "Below +10 deg");
  }
  writeCustomTarget(cAbsIndex); // write RA and DEC as target for GoTo
  showTargetCoords(); // display the target coordinates that were just written

  // Support for deleting a object row from the Custom library screen
  pre_cRelIndex = cRelIndex;
  pre_cAbsIndex = cAbsIndex;
  curSelCIndex = cAbsIndex;
  cPrevPage = cCurrentPage;
  catButDetected = false;
  customItemSelected = true;

  // DELETE CUSTOM USER library ROW check
  // Delete the row and shift other rows up, write back to storage media
  if (delSelected && customItemSelected) {
    delSelected = false;

    // delIndex is Row to delete in full array
    cAbsIndex = cFiltArray[catButSelPos];
    uint16_t delIndex = cAbsIndex;
    if (cusRowEntries == 0) { // check if delete of only one entry in catalog, if so, just delete catalog
      File rmFile = SD.open("/custom.csv");
        if (rmFile) {
            SD.remove("/custom.csv");
        }
      rmFile.close(); 
      return;
    } else { // copy rows after the one to be deleted over rows -1
      //VF("cAbsIndex="); VL(cAbsIndex);
      while (delIndex <= cusRowEntries-1) {
        strcpy(Copy_Custom_Array[delIndex], Copy_Custom_Array[delIndex+1]);
        delIndex++;
      }
      memset(Copy_Custom_Array[cusRowEntries], '\0', SD_CARD_LINE_LEN); // null terminate row left over at the end
      //for (int i = 0; i<=cusRowEntries-1; i++) { VL(Copy_Custom_Array[i]); }
    }

    // delete old SD file
    File rmFile = SD.open("/custom.csv");
      if (rmFile) {
          SD.remove("/custom.csv");
      }
    rmFile.close(); 

    // write new array into SD File
    File cWrFile;
    if ((cWrFile = SD.open("custom.csv", FILE_WRITE)) == 0) {
      canvPrint(STATUS_STR_X, STATUS_STR_Y, 0, STATUS_STR_W, STATUS_STR_H, "SD open ERROR");
      return;
    } else {
      if (cWrFile) {
        for(uint16_t i=0; i<=cusRowEntries-1; i++) {
            cWrFile.print(Copy_Custom_Array[i]);
            delay(5);
        }
        //VF("cWrsize="); VL(cWrFile.size());
        cWrFile.close();
      }
    }
  } // end Custom row delete
}

// ======= write Target Coordinates to controller =========
void CustomCatScreen::writeCustomTarget(uint16_t index) {
  //:Sr[HH:MM.T]# or :Sr[HH:MM:SS]# 
  setLocalCmd(cRaSrCmd[index]);
      
  //:Sd[sDD*MM]# or :Sd[sDD*MM:SS]#
  setLocalCmd(cDecSrCmd[index]);
  objSel = true;
}

//=====================================================
// **** Handle any buttons that have been selected ****
//=====================================================
bool CustomCatScreen::touchPoll(uint16_t px, uint16_t py) {
  for (int i=0; i<=cusRowEntries || i<NUM_CUS_ROWS_PER_SCREEN; i++) {
    if (cAbsRow == cusRowEntries+2) return true;
    if (py > CUS_Y+(i*(CUS_H+CUS_Y_SPACING)) && py < (CUS_Y+(i*(CUS_H+CUS_Y_SPACING))) + CUS_H 
          && px > CUS_X && px < (CUS_X+CUS_W)) {
      BEEP;
      if (cLastPage==0 && i >= cRow) return true; // take care of only one entry on the page
      catButSelPos = i;
      catButDetected = true;
      return true;
    }
  }
 
  // BACK button
  if (py > BACK_Y && py < (BACK_Y + BACK_H) && px > BACK_X && px < (BACK_X + BACK_W)) {
    BEEP;
    if (cCurrentPage > 0) {
      cPrevPage = cCurrentPage;
      cEndOfList = false;
      cCurrentPage--;
      drawCustomCat();
    }
    return true;
  }

  // NEXT page button - reuse BACK button box size
  if (py > NEXT_Y && py < (NEXT_Y + BACK_H) && px > NEXT_X && px < (NEXT_X + BACK_W)) {
    BEEP;
    if (!cEndOfList) {
      cPrevPage = cCurrentPage;
      cCurrentPage++;
      drawCustomCat();
    }
    return true;
  }

  // RETURN page button - reuse BACK button box size
  if (py > RETURN_Y && py < (RETURN_Y + BACK_H) && px > RETURN_X && px < (RETURN_X + RETURN_W)) {
    BEEP;
    moreScreen.objectSelected = objSel; 
    moreScreen.draw();
    return true;
  }

  // SAVE page to custom library button
  if (py > SAVE_LIB_Y && py < (SAVE_LIB_Y + SAVE_LIB_H) && px > SAVE_LIB_X && px < (SAVE_LIB_X + SAVE_LIB_W)) {
    BEEP;
    canvPrint(STATUS_STR_X, STATUS_STR_Y, 0, STATUS_STR_W, STATUS_STR_H, "Can't Save Custom");
    return true;
    saveTouched = true;
  }   

  // Delete custom library item that is selected 
  if (py > 3 && py < 42 && px > 282 && px < 317) {
    BEEP;
    delSelected = true;
    return true;
  }  
  return false; 
}

// Show target coordinates RA/DEC and ALT/AZM
void CustomCatScreen::showTargetCoords() {
  char raTargReply[15];
  char decTargReply[15];
  //double _catAzm, _catAlt;
  uint16_t radec_x = 160;
  uint16_t ra_y = 420;
  uint16_t dec_y = 433;
  uint16_t altazm_x = 240;
  char tAzmDMS[10] = "";
  char tAltDMS[11] = "";
  double tAzm_d = 0.0;
  double tAlt_d = 0.0;
  tft.setFont();

  // Get Target RA: Returns: HH:MM.T# or HH:MM:SS (based on precision setting)
  getLocalCmdTrim(":Gr#", raTargReply);  
  raTargReply[8] = 0; // clear # character
  
  tft.fillRect(radec_x, ra_y, 75, 11, pgBackground);
  tft.setCursor(radec_x, ra_y); tft.print(" RA:");
  tft.setCursor(radec_x+24, ra_y); tft.print(raTargReply);    
  
  // Get Target DEC: sDD*MM# or sDD*MM:SS# (based on precision setting)
  getLocalCmdTrim(":Gd#", decTargReply); 
  decTargReply[9] = 0; // clear # character

  tft.fillRect(radec_x, dec_y, 75, 11, pgBackground);
  tft.setCursor(radec_x, dec_y); tft.print("DEC:");
  tft.setCursor(radec_x+24, dec_y); tft.print(decTargReply);

  // === Show ALT and AZM ===

  // Get Target ALT and AZ and display them as Double
  getLocalCmdTrim(":Gz#", tAzmDMS); // DDD*MM'SS# 
  convert.dmsToDouble(&tAzm_d, tAzmDMS, false, PM_HIGH);

  getLocalCmdTrim(":Gal#", tAltDMS);	// sDD*MM'SS#
  convert.dmsToDouble(&tAlt_d, tAltDMS, true, PM_HIGH);

  tft.fillRect(altazm_x, ra_y, 70, 11, pgBackground);
  tft.setCursor(altazm_x, ra_y); tft.print("| AZ:");
  tft.setCursor(altazm_x+29, ra_y); tft.print(tAzm_d);      

  tft.fillRect(altazm_x, dec_y, 70, 11, pgBackground);
  tft.setCursor(altazm_x, dec_y); tft.print("| AL:");
  tft.setCursor(altazm_x+29, dec_y); tft.print(tAlt_d);
}

CustomCatScreen customCatScreen;
