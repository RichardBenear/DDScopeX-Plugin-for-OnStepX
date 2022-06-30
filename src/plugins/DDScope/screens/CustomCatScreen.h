// =====================================================
// CustomCatScreen.h

#ifndef CUSTOM_S_H
#define CUSTOM_S_H

#include <Arduino.h>
#include "../display/Display.h"

#define SD_CARD_LINE_LEN       110 // Length of line stored to SD card for Custom Catalog
#define NUM_CUS_ROWS_PER_SCREEN 14 //(370/CUS_H+CUS_Y_SPACING)
#define MAX_CUSTOM_PAGES        10 // 10 user pages should be enough
#define MAX_CUSTOM_ROWS         MAX_CUSTOM_PAGES*NUM_CUS_ROWS_PER_SCREEN // this is arbitrary selection..could be more if needed

typedef struct {
  char* cObjName;
  char* cRAhhmmss;
  char* cDECsddmmss;
  char* cCons;
  char* cObjType;
  char* cMag;
  char* cSize;
  char* cSubId;
} custom_t; 

class CustomCatScreen : public Display {
  public:
    void init();
    void updateCustomButtons(bool);
    bool touchPoll(uint16_t px, uint16_t py);
    bool catalogButStateChange();
    void updateCustomStatus();

  private:
    void updateScreen();
    void writeCustomTarget(uint16_t index);
    void showTargetCoords();
    bool loadCustomArray();
    void parseCcatIntoArray();
    void drawCustomCat();

    bool delSelected = false;
    bool objSel = false;
    bool saveTouched = false;

    bool customCatalog;
    bool cEndOfList;
    bool customItemSelected;

    uint16_t catButDetected;
    uint16_t catButSelPos = 0;
    uint16_t returnToPage;

    uint16_t pre_cAbsIndex;
    uint16_t pre_cRelIndex;
    uint16_t curSelCIndex;
    uint16_t cPrevRowIndex;

    uint16_t cCurrentPage;
    uint16_t cPrevPage;
    uint16_t cLastPage;

    uint16_t cAbsRow;
    uint16_t cRow;
    uint16_t cusRowEntries;

    char          Custom_Array[MAX_CUSTOM_ROWS][SD_CARD_LINE_LEN];
    char     Copy_Custom_Array[MAX_CUSTOM_ROWS][SD_CARD_LINE_LEN]; // save a copy for row deletion purposes
    char              cRaSrCmd[MAX_CUSTOM_ROWS][18]; 
    char             cDecSrCmd[MAX_CUSTOM_ROWS][18];
    uint16_t        cFiltArray[MAX_CUSTOM_ROWS];
    uint16_t cPagingArrayIndex[MAX_CUSTOM_PAGES];
   
    double dcAlt[MAX_CUSTOM_ROWS];
    double dcAzm[MAX_CUSTOM_ROWS];
    const char *activeFilterStr[3] = {"Filt: None", "Filt: Abv Hor", "Filt: All Sky"};
};

extern CustomCatScreen customCatScreen;

#endif
