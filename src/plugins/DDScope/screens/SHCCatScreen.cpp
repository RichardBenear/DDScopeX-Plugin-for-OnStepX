// =====================================================
// SHCCatScreen.cpp
// SHC (Smart Hand Controller) Catalog Screen
// This is composed of multiple catalog choices derived from OnStep/SHC by Howard Dutton
//
// Author: Richard Benear 6/22

#include "SHCCatScreen.h"
#include "MoreScreen.h"
#include "AlignScreen.h"
#include "../catalog/Catalog.h"
#include "../catalog/CatalogTypes.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "src/telescope/mount/Mount.h"
#include "src/lib/tasks/OnTask.h"
#include "src/telescope/mount/goto/Goto.h"

// These defines are from the SmartHandController repository
#define L_CAT_PER_UNK "Period Unknown"
#define L_CAT_PER_IRR "Period Irregular"
#define L_CAT_PER "Period"
#define L_CAT_UNK "Unknown"
#define L_CAT_OC "Open Cluster"
#define L_CAT_GC "Globular Cluster"
#define L_CAT_PN "Planetary Nebula"
#define L_CAT_SG "Spiral Galaxy"
#define L_CAT_EG "Eliptical Galaxy"
#define L_CAT_IG "Irregular Galaxy"
#define L_CAT_KNT "Knot"
#define L_CAT_SNR "SuperNova Rmnnt"
#define L_CAT_GAL "Galaxy"
#define L_CAT_CN  "Cluster + Nebula"
#define L_CAT_STR "Star"
#define L_CAT_PLA "Planet"
#define L_CAT_CMT "Comet"
#define L_CAT_AST "Asteroid"

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

#define CAT_X               1
#define CAT_Y               50
#define CAT_W               112
#define CAT_H               21
#define CAT_Y_SPACING       1

#define WIDTH_OFF           40 // negative width offset
#define SUB_STR_X_OFF       2
#define FONT_Y_OFF          7

// Catalog Button object default Arial
Button shcCatDefButton(0, 0, 0, 0, display.butOnBackground, display.butBackground, display.butOutline, display.defFontWidth, display.defFontHeight, "");

// Catalog Button object custom font
Button shcCatButton(0, 0, 0, 0, display.butOnBackground, display.butBackground, display.butOutline, display.mainFontWidth, display.mainFontHeight, "");

// Canvas Print constructor
CanvasPrint shcCatDefPrint(0, 0, 0, 0, display.butOnBackground, display.butBackground, "");

// Canvas Print object, Inconsolata_Bold8pt7b font
CanvasPrint shcCatInsPrint(0, 0, 0, 0, display.butOnBackground, display.butBackground, "");

extern const char* Txt_Bayer[];

// Initialize the SHC Catalog Screen
void SHCCatScreen::init(uint8_t catSelected) {
  returnToPage = display.currentScreen; // save page from where this function was called so we can return
  setCurrentScreen(SHC_CAT_SCREEN);
  setNightMode(getNightMode());
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);

  moreScreen.objectSelected = false;
  _catSelected = catSelected; // save for others in this class

  // initialize paging array indexes
  for (int i=0; i<MAX_SHC_PAGES; i++) shcPagingArrayIndex[i] = 0; 

  // initialize which catalog is selected
  shcPrevRowIndex = cat_mgr.getIndex();
  cat_mgr.select(catSelected);
  cat_mgr.setIndex(0); // initialize row index for entire catalog array at zero
  strcpy(prefix, cat_mgr.catalogPrefix()); // prefix for catalog e.g. Star, M, N, I etc

  // Show Page Title
  strcpy(title, cat_mgr.catalogTitle());
  if (catSelected == HERSCHEL) strcpy(title, "Herschel"); else // shorten title
  if (catSelected == STARS) strcpy(title, "Stars"); // shorten title
  drawTitle(110, TITLE_TEXT_Y, title); 

  // Sub title
  tft.setFont(); // revert to basic Arial font
  tft.setCursor(9,25); 
  strcpy(title,cat_mgr.catalogSubMenu()); 
  tft.print(title); 

  // show number of catalog entries and active filter
  tft.fillRect(6, 9, 77, 32,  butBackground); // erase page numbers
  tft.setCursor(235, 25); tft.printf("Entries="); 
  if (moreScreen.activeFilter) tft.print("?Filt"); else tft.print(cat_mgr.getMaxIndex());
  tft.setCursor(235, 9); 
  tft.print(activeFilterStr[moreScreen.activeFilter]);

  shcCurrentPage = 0;
  drawShcCat(); // draw first page of the selected catalog

  tft.setFont(&Inconsolata_Bold8pt7b);
  shcCatButton.draw(BACK_X, BACK_Y, BACK_W, BACK_H, "BACK", BUT_OFF);
  shcCatButton.draw(NEXT_X, NEXT_Y, BACK_W, BACK_H, "NEXT", BUT_OFF);
  shcCatButton.draw(RETURN_X, RETURN_Y, RETURN_W, BACK_H, "RETURN", BUT_OFF);
  shcCatButton.draw(SAVE_LIB_X, SAVE_LIB_Y, SAVE_LIB_W, SAVE_LIB_H, "SAVE LIB", BUT_OFF);
}

//========= draw a Screen of SHC catalog data ===================
//=== can consist of many screens of objects, so separate function
void SHCCatScreen::drawShcCat() {
  shcRow = 0;
  pre_shcIndex=0;
  char catLine[47]=""; //hold the string that is displayed beside the button on each page

  // Show Page number and total Pages
  tft.fillRect(6, 9, 70, 12,  butBackground); // erase page numbers
  tft.fillRect(0,60,319,353, pgBackground); // clear lower screen
  tft.setFont(); // basic Arial default
  tft.setCursor(6, 9); 
  tft.printf("Page "); 
  tft.print(shcCurrentPage+1);
  tft.printf(" of "); 
  if (moreScreen.activeFilter == FM_ABOVE_HORIZON) 
    tft.print("??"); 
  else tft.print(shcLastPage); 
  tft.setCursor(6, 25); 
  tft.print(activeFilterStr[moreScreen.activeFilter]);

  cat_mgr.setIndex(shcPagingArrayIndex[shcCurrentPage]); // array of page 1st row indexes
  if (cat_mgr.hasActiveFilter()) {
    shcLastPage = shcPrevRowIndex >= cat_mgr.getIndex();
  } else {
    shcLastPage = ((cat_mgr.getMaxIndex() / NUM_CAT_ROWS_PER_SCREEN)+1);
  }
  shcLastRow = (cat_mgr.getMaxIndex() % NUM_CAT_ROWS_PER_SCREEN);

  while ((shcRow < NUM_CAT_ROWS_PER_SCREEN) || ((shcRow < shcLastRow) && shcLastPage)) { 
    // Page number and total Pages
    tft.fillRect(8, 9, 70, 12,  display.butBackground);
    tft.setCursor(8, 9); tft.printf("Page "); tft.print(shcCurrentPage+1);
    tft.printf(" of ");
    if (moreScreen.activeFilter) tft.print("??"); else tft.print((cat_mgr.getMaxIndex()/NUM_CAT_ROWS_PER_SCREEN)+1);

    // erase any previous data
    tft.setCursor(CAT_X+CAT_W+2, CAT_Y+shcRow*(CAT_H+CAT_Y_SPACING));
    tft.fillRect(CAT_X+CAT_W+2, CAT_Y+shcRow*(CAT_H+CAT_Y_SPACING), 215, 17, display.butBackground);

    //fill the button with some identifying text which varies depending on catalog chosen
    memset(shcObjName[shcRow], '\0', 20); //NULL out row first
    if (_catSelected == HERSCHEL || _catSelected == INDEX) { //use prefix and primaryId for these 2 catalogs
      snprintf(shcObjName[shcRow], 9, "%2s%4ld", prefix, cat_mgr.primaryId());
      //VF("priId="); VL(cat_mgr.primaryId());
    } else if (cat_mgr.objectName()!= -1) { // does it have a name
      strcpy(shcObjName[shcRow], cat_mgr.objectNameStr());
    } else if (cat_mgr.subId()!=-1) { // does it have a subId
      strcpy(shcObjName[shcRow], cat_mgr.subIdStr()); 
      //VF("subId="); VL(cat_mgr.subIdStr());
    } else {
      strcpy(shcObjName[shcRow], "Unknown");
    }
  
    shcCatDefButton.drawLJ(CAT_X, CAT_Y+shcRow*(CAT_H+CAT_Y_SPACING), CAT_W, CAT_H, shcObjName[shcRow], BUT_OFF);

    // Object type e.g. Star, Galaxy, etc
    memset(objTypeStr[shcRow], '\0', sizeof(objTypeStr[shcRow]));
    strcpy(objTypeStr[shcRow], cat_mgr.objectTypeStr());
    
    // Object SubId, e.g. N147, N7243
    memset(shcSubId[shcRow], '\0', sizeof(shcSubId[shcRow]));
    strcpy(shcSubId[shcRow], cat_mgr.subIdStr());
    
    // Constellation
    memset(shcCons[shcRow], '\0', sizeof(shcCons[shcRow]));
    strcpy(shcCons[shcRow], cat_mgr.constellationStr());
    
    // magnitude column
    shcMag[shcRow] = cat_mgr.magnitude();

    // bayer and flamsteen
    memset(bayer[shcRow], '\0', sizeof(bayer[shcRow]));
    if (!cat_mgr.isDsoCatalog()) { // stars catalog
      if (cat_mgr.bayerFlam() < 24) { // show greek letter names
          strcpy(bayer[shcRow], Txt_Bayer[cat_mgr.bayerFlam()]);
      } else { // just show Flamsteen number
          strcpy(bayer[shcRow], cat_mgr.bayerFlamStr());
      }
      snprintf(catLine, 34, "%4.1f |%3s%4s |%-18s", // 29 + 4 + NULL = 34
                                      shcMag[shcRow], 
                                      bayer[shcRow], 
                                      shcCons[shcRow], 
                                      objTypeStr[shcRow]);
    } else { // not a star 
        snprintf(catLine, 47, "%4.1f |%-4s|%-15s |%-18s", //41 + 5 + NULL = 47
                                        shcMag[shcRow], 
                                        shcCons[shcRow], 
                                        objTypeStr[shcRow], 
                                        shcSubId[shcRow]);
    }
    // print out a line of data to the right of the object's button
    tft.setCursor(CAT_X+CAT_W+SUB_STR_X_OFF, CAT_Y+shcRow*(CAT_H+CAT_Y_SPACING)+FONT_Y_OFF); 
    tft.print(catLine);

    // fill the RA array for this row on the current page
    // RA in Hrs:Min:Sec
    cat_mgr.raHMS(*shcRaHrs[shcRow], *shcRaMin[shcRow], *shcRaSec[shcRow]);
    // shcRACustLine is used by the "Save to custom" catalog feature
    snprintf(shcRACustLine[shcRow], 9, "%02u:%02u:%02u", (unsigned int)*shcRaHrs[shcRow], (unsigned int)*shcRaMin[shcRow], (unsigned int)*shcRaSec[shcRow]);
    snprintf(shcRaSrCmd[shcRow], 14, ":Sr%s#", shcRACustLine[shcRow]); // written to the controller for GoTo coordinates
    
    // fill the DEC array for this Row on the current page
    // DEC in Deg:Min:Sec
    cat_mgr.decDMS(*shcDecDeg[shcRow], *shcDecMin[shcRow], *shcDecSec[shcRow]);

    //save the Alt and Azm for use later
    cat_mgr.EquToHor(cat_mgr.ra(), cat_mgr.dec(), &shcAlt[shcRow], &shcAzm[shcRow]);

    // shcDECCustLine is used later by the "Save to custom catalog" feature
    snprintf(shcDECCustLine[shcRow], 10, "%+03d*%02u:%02u", (int)*shcDecDeg[shcRow], (unsigned int)*shcDecMin[shcRow], (unsigned int)*shcDecSec[shcRow]);
    snprintf(shcDecSrCmd[shcRow], 15, ":Sd%s#", shcDECCustLine[shcRow]); // written to the controller for GoTo coordinates
    
    shcPrevRowIndex = cat_mgr.getIndex();
    shcRow++; // increments through the number of lines on screen
    cat_mgr.incIndex(); // increment the index that keeps track of the entire catalog's contents
    
    // stop printing data if last field on the last page
    //VF("shcIndex="); VL(cat_mgr.getIndex());
    //VF("shcPrevRowIndex="); VL(shcPrevRowIndex);
    if ((shcPrevRowIndex >= cat_mgr.getIndex()) || (cat_mgr.getIndex() == cat_mgr.getMaxIndex())) {
      shcEndOfList = true; 
      return; 
    }
  }
  shcPagingArrayIndex[shcCurrentPage+1] = cat_mgr.getIndex(); // shcPagingArrayIndex holds index of first element of page to help with NEXT and BACK paging
  //VF("shcPagingArrayIndex+1="); VL(shcPagingArrayIndex[shcCurrentPage+1]);
}

// show status changes on tasks timer tick
void SHCCatScreen::updateShcStatus() {
  // do nothing currently
}

// redraw screen to show state change
bool SHCCatScreen::catalogButStateChange() {
  if (display._redrawBut) {
    display._redrawBut = false;
    return true;
  } else { 
    return false;
  }
}

// =========  Update Screen buttons  ===========
void SHCCatScreen::updateShcButtons(bool redrawBut) { 
  _redrawBut = redrawBut;  

  if (catButDetected) updateScreen();

  tft.setFont(&Inconsolata_Bold8pt7b);
  if (saveTouched) {
    shcCatButton.draw(SAVE_LIB_X, SAVE_LIB_Y, SAVE_LIB_W, SAVE_LIB_H, "Saving", BUT_ON);
    saveTouched = false;
  } else { 
    shcCatButton.draw(SAVE_LIB_X, SAVE_LIB_Y, SAVE_LIB_W, SAVE_LIB_H, "SaveToCat", BUT_OFF);
  }
  tft.setFont();
}

// =============== Update buttons and text ================
void SHCCatScreen::updateScreen() {
  // Toggle off previous selected button and toggle on current selected button
  tft.setFont();
  shcCatDefButton.drawLJ(CAT_X, CAT_Y+pre_shcIndex*(CAT_H+CAT_Y_SPACING), CAT_W, CAT_H, shcObjName[pre_shcIndex], BUT_OFF);
  shcCatDefButton.drawLJ(CAT_X, CAT_Y+catButSelPos*(CAT_H+CAT_Y_SPACING), CAT_W, CAT_H, shcObjName[catButSelPos], BUT_ON);
  pre_shcIndex = catButSelPos;
  curSelSIndex = catButSelPos;
  catButDetected = false;
  
  // the following 5 lines are displayed on the Catalog/More page
  snprintf(moreScreen.catSelectionStr1, 26, "Name-:%-19s",  shcObjName[catButSelPos]);    //VF("shcObjName="); VL(shcObjName[catButSelPos]);
  snprintf(moreScreen.catSelectionStr2, 11, "Mag--:%-4.1f", shcMag[catButSelPos]);        //VF("shcMag=");     VL(shcMag[catButSelPos]);
  snprintf(moreScreen.catSelectionStr3, 11, "Const:%-4s",   shcCons[catButSelPos]);       //VF("shcCons=");    VL(shcCons[catButSelPos]);
  snprintf(moreScreen.catSelectionStr4, 16, "Type-:%-9s",   objTypeStr[catButDetected]);  //VF("objType=");    VL(objTypeStr[catButSelPos]);
  snprintf(moreScreen.catSelectionStr5, 15, "Id---:%-7s",   shcSubId[catButSelPos]);      //VF("shcSubId=");   VL(shcSubId[catButSelPos]);

  // show if we are above and below visible limits
  tft.setFont(&Inconsolata_Bold8pt7b);  
  if (shcAlt[catButSelPos] > 10.0) {      // minimum 10 degrees altitude
      shcCatInsPrint.PrintCus(STATUS_STR_X, STATUS_STR_Y, 0, STATUS_STR_W, STATUS_STR_H, "Above +10 deg");
  } else {
      shcCatInsPrint.PrintCus(STATUS_STR_X, STATUS_STR_Y, 0, STATUS_STR_W, STATUS_STR_H, "Below +10 deg");
  }

  writeSHCTarget(catButSelPos); // write RA and DEC as target for GoTo
  tasks.yield(70); 
  showTargetCoords(); // display the target coordinates that were just written            
}

// ====== save data into user catalog =======
void SHCCatScreen::saveSHC() {
// "UNK",  "OC",  "GC",  "PN",  "DN",  "SG",  "EG",  "IG", "KNT", "SNR", "GAL",  "CN", "STR", "PLA", "CMT", "AST"
  if (strstr(objTypeStr[curSelSIndex],L_CAT_UNK)) strcpy(truncObjType,"UNK"); else
  if (strstr(objTypeStr[curSelSIndex],L_CAT_OC )) strcpy(truncObjType,"OC" ); else
  if (strstr(objTypeStr[curSelSIndex],L_CAT_GC )) strcpy(truncObjType,"GC" ); else
  if (strstr(objTypeStr[curSelSIndex],L_CAT_PN )) strcpy(truncObjType,"PN" ); else
  if (strstr(objTypeStr[curSelSIndex],L_CAT_SG )) strcpy(truncObjType,"SG" ); else
  if (strstr(objTypeStr[curSelSIndex],L_CAT_EG )) strcpy(truncObjType,"EG" ); else
  if (strstr(objTypeStr[curSelSIndex],L_CAT_IG )) strcpy(truncObjType,"IG" ); else
  if (strstr(objTypeStr[curSelSIndex],L_CAT_KNT)) strcpy(truncObjType,"KNT"); else
  if (strstr(objTypeStr[curSelSIndex],L_CAT_SNR)) strcpy(truncObjType,"SNR"); else
  if (strstr(objTypeStr[curSelSIndex],L_CAT_GAL)) strcpy(truncObjType,"GAL"); else
  if (strstr(objTypeStr[curSelSIndex],L_CAT_CN )) strcpy(truncObjType,"CN" ); else
  if (strstr(objTypeStr[curSelSIndex],L_CAT_STR)) strcpy(truncObjType,"STR"); else
  if (strstr(objTypeStr[curSelSIndex],L_CAT_PLA)) strcpy(truncObjType,"PLA"); else
  if (strstr(objTypeStr[curSelSIndex],L_CAT_CMT)) strcpy(truncObjType,"CMT"); else
  if (strstr(objTypeStr[curSelSIndex],L_CAT_AST)) strcpy(truncObjType,"AST");

  // ObjName, shcMag, Cons, ObjType, shcSubId, RA, DEC
  snprintf(shcCustWrSD, 81, "%-19s;%4.1f;%-4s;%-9s;%-18s;%8s;%9s\n", 
                                                  shcObjName[curSelSIndex], 
                                                  shcMag[curSelSIndex],
                                                  shcCons[curSelSIndex], 
                                                  truncObjType,
                                                  shcSubId[curSelSIndex],
                                                  shcRACustLine[curSelSIndex],
                                                  shcDECCustLine[curSelSIndex]);                                 
  // write string to the SD card
  File wrFile = SD.open("custom.csv", FILE_WRITE);
  if (wrFile) {
    wrFile.print(shcCustWrSD);
    }
    //VF("size="); VL(wrFile.size());
  wrFile.close();  
}

// =============== check the Catalog Buttons if pressed ================
bool SHCCatScreen::touchPoll(uint16_t px, uint16_t py) {
  uint16_t i=0;
  for (i=0; i<(NUM_CAT_ROWS_PER_SCREEN); i++) {
    if (py > CAT_Y+(i*(CAT_H+CAT_Y_SPACING)) && py < (CAT_Y+(i*(CAT_H+CAT_Y_SPACING))) + CAT_H 
          && px > CAT_X && px < (CAT_X+CAT_W)) {
      BEEP;
      if (shcLastPage && i >= shcRow)  return true; 
      catButSelPos = i;
      catButDetected = true;
      return true;
    }
  }
 
  // BACK button
  if (py > BACK_Y && py < (BACK_Y + BACK_H) && px > BACK_X && px < (BACK_X + BACK_W)) {
    BEEP;
    if (shcCurrentPage > 0) {
    shcEndOfList = false;
    shcCurrentPage--;
    drawShcCat();
    }
    return false;
  }

  // NEXT page button - reuse BACK button box size
  if (py > NEXT_Y && py < (NEXT_Y + BACK_H) && px > NEXT_X && px < (NEXT_X + BACK_W)) {
    BEEP;
    if (!shcEndOfList) { 
    shcCurrentPage++;
    drawShcCat();
    }
    return false;
  }

  // RETURN page button - reuse BACK button box size
  if (py > RETURN_Y && py < (RETURN_Y + BACK_H) && px > RETURN_X && px < (RETURN_X + RETURN_W)) {
    BEEP;
    moreScreen.objectSelected = objSel; 
    if (returnToPage == ALIGN_SCREEN) {
      setCurrentScreen(ALIGN_SCREEN);
      alignScreen.draw();
      return false; // don't update this screen since returing to ALIGN
    } else if (returnToPage == MORE_SCREEN) {
      setCurrentScreen(MORE_SCREEN);
      moreScreen.draw();
      return false; // don't return this screen since returning to MORE
    }
  }

  // SAVE page to custom library button
  if (py > SAVE_LIB_Y && py < (SAVE_LIB_Y + SAVE_LIB_H) && px > SAVE_LIB_X && px < (SAVE_LIB_X + SAVE_LIB_W)) {
    BEEP;
    shcCatScreen.saveSHC();
    saveTouched = true;
    return true;
  }   
  return false;
}

// ======= write Target Coordinates to controller =========
void SHCCatScreen::writeSHCTarget(uint16_t index) {
  //:Sr[HH:MM.T]# or :Sr[HH:MM:SS]# 
  setLocalCmd(shcRaSrCmd[index]);
    
  //:Sd[sDD*MM]# or :Sd[sDD*MM:SS]#
  setLocalCmd(shcDecSrCmd[index]);
  objSel = true;
}

// Show target coordinates RA/DEC and ALT/AZM
void SHCCatScreen::showTargetCoords() {
  // Get Target RA: Returns: HH:MM.T# or HH:MM:SS (based on precision setting)
  //getLocalCmdTrim(":Gr#", reply);
  //sprintf(_reply, "RA: %9s", reply);
  //shcCatDefPrint(radec_x, ra_y, width, height, _reply, false);
  
  // Get Target DEC: sDD*MM# or sDD*MM:SS# (based on precision setting)
  //getLocalCmdTrim(":Gd#", reply); 
  //sprintf(_reply, "DEC: %9s", reply);
  //shcCatDefPrint(radec_x, dec_y, width, height, _reply, false);

  // Get Target ALT and AZ and display them as Double
  //getLocalCmdTrim(":Gz#", reply); // DDD*MM'SS# 
  //convert.dmsToDouble(&tAzm_d, reply, false, PM_HIGH);

  //getLocalCmdTrim(":Gal#", reply);	// sDD*MM'SS#
  //convert.dmsToDouble(&tAlt_d, reply, true, PM_HIGH);

  //sprintf(_reply, "AZM: %6.1f", tAzm_d); 
  //shcCatDefPrint(altazm_x, ra_y, width-10, height, _reply, false);    

  //sprintf(_reply, "ALT: %6.1f", tAlt_d);
  //shcCatDefPrint(altazm_x, dec_y, width-10, height, _reply, false); 

    char _reply[15]   = "";
  uint16_t radec_x  = 155;
  uint16_t ra_y     = 405;
  uint16_t dec_y    = 418;
  uint16_t altazm_x = 250;
  uint16_t width    = 85;
  uint16_t height   = 12;

  Coordinate catTarget = goTo.getGotoTarget();
  transform.rightAscensionToHourAngle(&catTarget);
  transform.equToHor(&catTarget);

  sprintf(_reply, "RA: %6.1f", radToHrs(catTarget.r));
  shcCatDefPrint.PrintCus(radec_x, ra_y, width, height, _reply, false);
  sprintf(_reply, "DEC: %6.1f", radToDeg(catTarget.d));
  shcCatDefPrint.PrintCus(radec_x, dec_y, width, height, _reply, false);
  sprintf(_reply, "AZM: %6.1f", NormalizeAzimuth(radToDeg(catTarget.z))); 
  shcCatDefPrint.PrintCus(altazm_x, ra_y, width-10, height, _reply, false);    
  sprintf(_reply, "ALT: %6.1f", radToDeg(catTarget.a));
  shcCatDefPrint.PrintCus(altazm_x, dec_y, width-10, height, _reply, false); 
}

SHCCatScreen shcCatScreen;
