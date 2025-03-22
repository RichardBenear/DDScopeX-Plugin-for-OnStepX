// =====================================================
// CustomCatScreen.cpp
// Custom User Catalog Screen
// This catalog can hold various saved objects and their coordinates from other catalogs
//
// Author: Richard Benear 6/22

#include "CustomCatScreen.h"
#include "MoreScreen.h"
#include "../catalog/Catalog.h"
#include "../catalog/CatalogTypes.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "src/telescope/mount/Mount.h"
#include "src/lib/tasks/OnTask.h"
#include "src/telescope/mount/goto/Goto.h"

#define CUS_X               1
#define CUS_Y               50
#define CUS_W               112
#define CUS_H               24
#define CUS_Y_SPACING       2

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

custom_t _cArray[MAX_CUSTOM_ROWS];

// Catalog Button object for default Arial font
Button customDefButton(0, 0, 0, 0, butOnBackground, butBackground, butOutline, defFontWidth, defFontHeight, "");

// Catalog Button object for custom font
Button customCatButton(0, 0 ,0, 0, butOnBackground, butBackground, butOutline, mainFontWidth, mainFontHeight, "");

// Canvas Print object default Arial 6x9 font
CanvasPrint canvCustomDefPrint(display.default_font);

// Canvas Print object, Inconsolata_Bold8pt7b font
CanvasPrint canvCustomInsPrint(&Inconsolata_Bold8pt7b);

// ========== Initialize and draw Custom Catalog ===============
void CustomCatScreen::init() { 
  returnToPage = display.currentScreen; // save page from where this function was called so we can return
  setCurrentScreen(CUSTOM_SCREEN);
  #ifdef ENABLE_TFT_CAPTURE
  tft.enableLogging(true);
  #endif
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  moreScreen.objectSelected = false;
  cCurrentPage = 0;
  cPrevPage = 0;
  cEndOfList = false;
  cAbsRow = 0; // initialize the absolute index into total array

  drawTitle(86, TITLE_TEXT_Y, "User Catalog");
  customCatalog = true;

  if (!loadCustomArray()) {
      canvCustomInsPrint.printRJ(STATUS_STR_X, STATUS_STR_Y, STATUS_STR_W, STATUS_STR_H, "ERR:Loading Custom", true); 
      moreScreen.draw();
      return;
  } else {
      parseCcatIntoArray();
  }
 
  // Draw the Trash can/Delete Icon bitmap; used to delete an entry in only the CUSTOM USER catalog
  uint8_t extern trash_icon[];
  tft.drawBitmap(270, 5, trash_icon, 28, 32, butBackground, ORANGE);

  drawCustomCat(); // draw first page of the selected catalog

  tft.setFont(&Inconsolata_Bold8pt7b);
  customCatButton.draw(BACK_X, BACK_Y, BACK_W, BACK_H, "BACK", BUT_OFF);
  customCatButton.draw(NEXT_X, NEXT_Y, BACK_W, BACK_H, "NEXT", BUT_OFF);
  customCatButton.draw(RETURN_X, RETURN_Y, RETURN_W, BACK_H, "RETURN", BUT_OFF);

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
    VLF("SD Cust read error");
    return false;
  }
  rdFile.close();
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
    _cArray[i].cRAhhmmss     = strtok(NULL, ";");            //VL(_cArray[i].cRAhhmmss);
    _cArray[i].cDECsddmmss   = strtok(NULL, "\n");           //VL(_cArray[i].cDECsddmmss);
  }
}

// ========== draw CUSTOM Screen of catalog data ========
void CustomCatScreen::drawCustomCat() {
  cRow = 0;
  pre_cAbsIndex=0;
  char catLine[47]=""; //hold the string that is displayed beside the button on each page

  cAbsRow = (cPagingArrayIndex[cCurrentPage]); // array of indexes for 1st row of each page
  cLastPage = (cusRowEntries / NUM_CUS_ROWS_PER_SCREEN)+1;
  VF("cusRowEntries="); VL(cusRowEntries);
  cNumRowsLastPage = (cusRowEntries % NUM_CUS_ROWS_PER_SCREEN) +1;
  if (cCurrentPage+1 == cLastPage) isLastPage = cNumRowsLastPage <= NUM_CUS_ROWS_PER_SCREEN; else isLastPage = false;

  // Show Page number and total Pages
  tft.fillRect(6, 9, 77, 32,  butBackground); // erase page numbers
  tft.fillRect(0,60,319,353, pgBackground); // clear lower screen
  tft.setFont(0); //revert to basic Arial font
  tft.setCursor(6, 9); 
  tft.printf("Page "); 
  tft.print(cCurrentPage+1);
  tft.printf(" of "); 
  if (moreScreen.activeFilter == FM_ABOVE_HORIZON) 
    tft.print("??"); 
  else 
    tft.print(cLastPage); 
  tft.setCursor(6, 25); 
  tft.print(activeFilterStr[moreScreen.activeFilter]);

  // TO DO: add code for case when filter enabled and screen contains fewer than NUM_CUS_ROWS_PER_SCREEN
  while ((cRow < NUM_CUS_ROWS_PER_SCREEN) && (cAbsRow != MAX_CUSTOM_ROWS)) { 
    // ======== process RA/DEC ===========
    double cRAdouble;
    double cDECdouble;

    // RA in Hrs:Min:Sec
    snprintf(cRaSrCmd[cAbsRow], 16, ":Sr%11s#", _cArray[cAbsRow].cRAhhmmss);
    snprintf(cDecSrCmd[cAbsRow], 17, ":Sd%12s#", _cArray[cAbsRow].cDECsddmmss);

    // to get Altitude, first convert RAh and DEC to double
    // Note: PM_HIGH is required for the format used here
    convert.hmsToDouble(&cRAdouble, _cArray[cAbsRow].cRAhhmmss, PM_HIGH);
    convert.dmsToDouble(&cDECdouble, _cArray[cAbsRow].cDECsddmmss, true, PM_HIGH);
    
    // NOTE: next 2 line were old version now replace by Coordinate transform cusTarget
    //double cHAdouble=haRange(LST()*15.0-cRAdouble);
    //cat_mgr.EquToHor(cRAdouble*15, cDECdouble, &dcAlt[cAbsRow], &dcAzm[cAbsRow]);

    // dcAlt[cAbsRow] is used by filter to check if above Horizon
    // Coordinate calculation using OnStep transforms
    Coordinate cusTarget;
    cusTarget.r = hrsToRad(cRAdouble);
    cusTarget.d = degToRad(cDECdouble);
    transform.rightAscensionToHourAngle(&cusTarget);
    transform.equToAlt(&cusTarget);
    dcAlt[cAbsRow] = radToDeg(cusTarget.a);

    //VF("activeFilter="); VL(activeFilter);
    if (((moreScreen.activeFilter == FM_ABOVE_HORIZON) && (dcAlt[cAbsRow] > 10.0)) || moreScreen.activeFilter == FM_NONE) { // filter out elements below 10 deg if filter enabled
      //VF("printing row="); VL(cAbsRow);
      // Erase text background
      tft.setCursor(CUS_X+CUS_W+2, CUS_Y+cRow*(CUS_H+CUS_Y_SPACING));
      tft.fillRect(CUS_X+CUS_W+2, CUS_Y+cRow*(CUS_H+CUS_Y_SPACING), 215, 17,  butBackground);

      // get object names and put them on the buttons
      customDefButton.drawLJ(CUS_X, CUS_Y+cRow*(CUS_H+CUS_Y_SPACING), CUS_W, CUS_H, _cArray[cAbsRow].cObjName, BUT_OFF);
                  
      // format and print the text field for this row next to the button
      // 7 - objName
      // 8 - RA
      // 7 - DEC
      // 4 - Cons
      // 9 - ObjType
      // 4 - Mag
      // 9 - Size ( Not used )
      // 18 - SubId
      // select some data fields to show beside the button
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
      if (cRow == 0) {canvCustomInsPrint.printRJ(STATUS_STR_X, STATUS_STR_Y, STATUS_STR_W, STATUS_STR_H, "None above 10 deg", true);}
      //VF("endOfList");
      return; 
    }
  }
  // cPagingArrayIndex holds index of first element of page to help with NEXT and BACK paging
  cPagingArrayIndex[cCurrentPage+1] = cAbsRow; 
  //VF("PagingArrayIndex+1="); VL(cPagingArrayIndex[cCurrentPage+1]);
}

// show status changes on tasks timer tick
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

// update buttons and rest of screen
void CustomCatScreen::updateCustomButtons(bool redrawBut) { 
  _redrawBut = redrawBut;  
  
  if (catButDetected) updateScreen();  
  tft.setFont(0); // basic Arial
}

//==================================================
// =====  Update Screen buttons and text ===========
//==================================================
void CustomCatScreen::updateScreen() {
  tft.setFont(0);
  uint16_t cRelIndex = catButSelPos; // save the relative-to-this "screen/page" index of button pressed
  uint16_t cAbsIndex = cFiltArray[catButSelPos]; // this is absolute full array index

  if (cPrevPage == cCurrentPage) { //erase previous selection
    customDefButton.drawLJ(CUS_X, CUS_Y+pre_cRelIndex*(CUS_H+CUS_Y_SPACING), 
      CUS_W, CUS_H, _cArray[pre_cAbsIndex].cObjName, BUT_OFF); 
  }
  // highlight selected by settting background ON color 
  customDefButton.drawLJ(CUS_X, CUS_Y+cRelIndex*(CUS_H+CUS_Y_SPACING), 
    CUS_W, CUS_H, _cArray[cAbsIndex].cObjName, BUT_ON); 

  // the following 5 lines are displayed on the Catalog/More page
  snprintf(moreScreen.catSelectionStr1, 26, "Name-:%-19s", _cArray[cAbsIndex].cObjName);  //VF("c_objName="); //VL(_cArray[cAbsIndex].cObjName);
  snprintf(moreScreen.catSelectionStr2, 11, "Mag--:%-4s",  _cArray[cAbsIndex].cMag);      //VF("c_Mag=");     //VL(_cArray[cAbsIndex].cMag);
  snprintf(moreScreen.catSelectionStr3, 11, "Const:%-4s",  _cArray[cAbsIndex].cCons);     //VF("c_constel="); //VL(_cArray[cAbsIndex].cCons);
  snprintf(moreScreen.catSelectionStr4, 16, "Type-:%-9s",  _cArray[cAbsIndex].cObjType);  //VF("c_objType="); //VL(_cArray[cAbsIndex].cObjType);
  snprintf(moreScreen.catSelectionStr5, 15, "Id---:%-7s",  _cArray[cAbsIndex].cSubId);    //VF("c_subID=");   //VL(_cArray[cAbsIndex].cSubId);
  
  // show if we are above and below visible limits
  tft.setFont(&Inconsolata_Bold8pt7b); 
  if (dcAlt[cAbsIndex] > 10.0) {      // minimum 10 degrees altitude
      canvCustomInsPrint.printRJ(STATUS_STR_X, STATUS_STR_Y, STATUS_STR_W, STATUS_STR_H, "Above +10 deg", false);
  } else {
      canvCustomInsPrint.printRJ(STATUS_STR_X, STATUS_STR_Y, STATUS_STR_W, STATUS_STR_H, "Below +10 deg", true);
  }
  tft.setFont(0);

  writeCustomTarget(cAbsIndex); // write RA and DEC as target for GoTo
  tasks.yield(70);
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
      canvCustomInsPrint.printRJ(STATUS_STR_X, STATUS_STR_Y, STATUS_STR_W, STATUS_STR_H, "SD open ERROR", true);
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
} // end updateScreen

//=====================================================
// **** Handle any buttons that have been pressed *****
//=====================================================
bool CustomCatScreen::touchPoll(uint16_t px, uint16_t py) {
  // TO DO: add code for case when filter enabled and screen contains fewer than NUM_CUS_ROWS_PER_SCREEN
  if (isLastPage) cRowsPerPage = cNumRowsLastPage; else cRowsPerPage = NUM_CUS_ROWS_PER_SCREEN;
   VF("islastpage="); VL(isLastPage);
   VF("cRowsPerpage="); VL(cRowsPerPage);
    VF("cNumRowLastPage="); VL(cNumRowsLastPage);
  for (int i=0; i < cRowsPerPage; i++) {
    if (py > CUS_Y+(i*(CUS_H+CUS_Y_SPACING)) && py < (CUS_Y+(i*(CUS_H+CUS_Y_SPACING))) + CUS_H 
          && px > CUS_X && px < (CUS_X+CUS_W)) {
      BEEP;
      if (cAbsRow <= 1 || (cAbsRow > cusRowEntries+1)) return false; 
      catButSelPos = i;
      catButDetected = true;
      updateScreen();
      return false; // update screen by redrawing buttons
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
    return false; // skip update since redrawing full screen again
  }

  // NEXT page button - reuse BACK button box size
  if (py > NEXT_Y && py < (NEXT_Y + BACK_H) && px > NEXT_X && px < (NEXT_X + BACK_W)) {
    BEEP;
    if (!cEndOfList) {
      cPrevPage = cCurrentPage;
      cCurrentPage++;
      drawCustomCat();
    }
    return false; // skip update since redrawing full screen again
  }

  // RETURN page button - reuse BACK button box size
  if (py > RETURN_Y && py < (RETURN_Y + BACK_H) && px > RETURN_X && px < (RETURN_X + RETURN_W)) {
    BEEP;
    #ifdef ENABLE_TFT_CAPTURE
      tft.enableLogging(false);
      tft.saveBufferToSD("CustCat");
    #endif
    moreScreen.objectSelected = objSel; 
    moreScreen.draw();
    return false; // don't update this screen since returning to MORE
  }

  // Trash Can pressed, Delete custom library item that is selected 
  if (py > 3 && py < 42 && px > 282 && px < 317) {
    BEEP;
    delSelected = true;
    drawCustomCat(); 
    return false; // no need to redraw, skip
  }  

  // Check emergeyncy ABORT button area
  display.motorsOff(px, py);
  
  return false; 
}

// ======= write Target Coordinates to controller =========
void CustomCatScreen::writeCustomTarget(uint16_t index) {
  //:Sr[HH:MM.T]# or :Sr[HH:MM:SS]# 
  setLocalCmd(cRaSrCmd[index]);
      
  //:Sd[sDD*MM]# or :Sd[sDD*MM:SS]#
  setLocalCmd(cDecSrCmd[index]);
  objSel = true;
}

// Show target coordinates RA/DEC and ALT/AZM
void CustomCatScreen::showTargetCoords() {
  char _reply[15]   = "";
  uint16_t radec_x  = 155;
  uint16_t ra_y     = 405;
  uint16_t dec_y    = 418;
  uint16_t altazm_x = 246;
  uint16_t width    = 85;
  uint16_t height   = 12;

  // The following used the local command channel
  // Decided to change to direct access to transfroms method since
  //    getting target ALT and AZM didn't work as expected and transform is faster
  //double cAzm_d     = 0.0;
  //double cAlt_d     = 0.0;
  //char reply[15]    = "";
  
  // Get Target RA: Returns: HH:MM.T# or HH:MM:SS (based on precision setting)
  //getLocalCmdTrim(":Gr#", reply);

  // Get Target DEC: sDD*MM# or sDD*MM:SS# (based on precision setting)
  //getLocalCmdTrim(":Gd#", reply); 

  // Get Target ALT and AZ and display them as Double
  //getLocalCmdTrim(":Gz#", reply); // DDD*MM'SS# 
  //convert.dmsToDouble(&tAzm_d, reply, false, PM_LOW);

  //getLocalCmdTrim(":Gal#", reply);	// sDD*MM'SS#
  //convert.dmsToDouble(&tAlt_d, reply, true, PM_LOW);

  //sprintf(_reply, "AZM: %6.1f", cAzm_d); 
  //canvCustomDefPrint(altazm_x, ra_y, width-10, height, _reply, false);    

  //sprintf(_reply, "ALT: %6.1f", cAlt_d);
  //canvCustomDefPrint(altazm_x, dec_y, width-10, height, _reply, false); 

  Coordinate catTarget = goTo.getGotoTarget();
  transform.rightAscensionToHourAngle(&catTarget);
  transform.equToHor(&catTarget);

  sprintf(_reply, "RA: %6.1f", radToHrs(catTarget.r));
  canvCustomDefPrint.printRJ(radec_x, ra_y, width, height, _reply, false);
  sprintf(_reply, "DEC: %6.1f", radToDeg(catTarget.d));
  canvCustomDefPrint.printRJ(radec_x, dec_y, width, height, _reply, false);
  sprintf(_reply, "AZM: %6.1f", (double)NormalizeAzimuth(radToDeg(catTarget.z))); 
  canvCustomDefPrint.printRJ(altazm_x, ra_y, width-10, height, _reply, false);    
  sprintf(_reply, "ALT: %6.1f", radToDeg(catTarget.a));
  canvCustomDefPrint.printRJ(altazm_x, dec_y, width-10, height, _reply, false); 
}

CustomCatScreen customCatScreen;
