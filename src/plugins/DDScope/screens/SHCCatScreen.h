// =====================================================
// SHCCatScreen.h

#ifndef SHC_S_H
#define SHC_S_H

#include <Arduino.h>
#include "../display/Display.h"

#define NUM_CAT_ROWS_PER_SCREEN 16 //(370/CAT_H+CAT_Y_SPACING)
#define SD_CARD_LINE_LEN       110 // Length of line stored to SD card for Custom Catalog
#define MAX_SHC_PAGES           30 // used by paging indexer affay for SHC

//===============================
class SHCCatScreen : public Display {
  public:
    void init(uint8_t);
    void updateShcButtons(bool);
    bool touchPoll(uint16_t px, uint16_t py);
    bool catalogButStateChange();
    void updateShcStatus();
    
  private:
    void updateScreen();
    void drawShcCat();
    void saveSHC();
    void writeSHCTarget(uint16_t index);
    void showTargetCoords();

    bool delSelected = false;
    bool objSel = false;
    bool saveTouched = false;

    bool shcCatalog;
    bool shcEndOfList;
    const char *activeFilterStr[3] = {"Filt: None", "Filt: Abv Hor", "Filt: All Sky"};
    
    uint8_t  _catSelected;
    uint16_t catButDetected;
    uint16_t catButSelPos = 0;

    uint16_t shcCurrentPage;
    uint16_t returnToPage;
    uint16_t shcLastPage;

    uint16_t pre_shcIndex;
    uint16_t curSelSIndex;

    uint16_t shcPrevRowIndex;
    uint16_t shcLastRow;
    uint16_t shcRow;

    uint16_t shcPagingArrayIndex[MAX_SHC_PAGES];
    
    char herschObjName[7];
    char prefix[5];
    char title[14];
    char shcCustWrSD[SD_CARD_LINE_LEN];
    char truncObjType[5];

    // Smart Hand Controller (4) Catalogs
    char     shcObjName[NUM_CAT_ROWS_PER_SCREEN][20]; //19 + NULL
    char        shcCons[NUM_CAT_ROWS_PER_SCREEN][7];
    char          bayer[NUM_CAT_ROWS_PER_SCREEN][3];
    char       shcSubId[NUM_CAT_ROWS_PER_SCREEN][18];
    char     objTypeStr[NUM_CAT_ROWS_PER_SCREEN][15];
    float        shcMag[NUM_CAT_ROWS_PER_SCREEN];
    char  shcRACustLine[NUM_CAT_ROWS_PER_SCREEN][10];
    char shcDECCustLine[NUM_CAT_ROWS_PER_SCREEN][11];
    char     shcRaSrCmd[NUM_CAT_ROWS_PER_SCREEN][17]; // has ":Sr...#" cmd channel chars
    char    shcDecSrCmd[NUM_CAT_ROWS_PER_SCREEN][18]; // has ":Sd...#" cmd channel chars
    uint8_t    shcRaHrs[NUM_CAT_ROWS_PER_SCREEN][3]; 
    uint8_t    shcRaMin[NUM_CAT_ROWS_PER_SCREEN][3];
    uint8_t    shcRaSec[NUM_CAT_ROWS_PER_SCREEN][3];
    short     shcDecDeg[NUM_CAT_ROWS_PER_SCREEN][5]; 
    uint8_t   shcDecMin[NUM_CAT_ROWS_PER_SCREEN][3];
    uint8_t   shcDecSec[NUM_CAT_ROWS_PER_SCREEN][3];
    double       shcAlt[NUM_CAT_ROWS_PER_SCREEN];
    double       shcAzm[NUM_CAT_ROWS_PER_SCREEN];
};

extern SHCCatScreen shcCatScreen;

#endif

