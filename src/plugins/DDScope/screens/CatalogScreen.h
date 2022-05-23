// =====================================================
// CatalogScreen.h

#ifndef CATALOG_S_H
#define CATALOG_S_H

#include <Arduino.h>

#define NUM_CAT_ROWS_PER_SCREEN 16 //(370/CAT_H+CAT_Y_SPACING)
#define NUM_CUS_ROWS_PER_SCREEN 14 //(370/CUS_H+CUS_Y_SPACING)

#define MAX_TREASURE_PAGES       8 // more than needed with current settings
#define MAX_TREASURE_ROWS      129 // full number of rows in catlog; starts with 1, not 0
#define MAX_CUSTOM_PAGES        10 // 10 user pages should be enough
#define MAX_CUSTOM_ROWS         MAX_CUSTOM_PAGES*NUM_CUS_ROWS_PER_SCREEN // this is arbitrary selection..could be more if needed
#define MAX_SHC_PAGES           30 // used by paging indexer affay for SHC

// Catalog Selection buttons
#define SD_CARD_LINE_LEN       110 // Length of line stored to SD card for Custom Catalog
 
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
#define L_CAT_CN "Cluster + Nebula"
#define L_CAT_STR "Star"
#define L_CAT_PLA "Planet"
#define L_CAT_CMT "Comet"
#define L_CAT_AST "Asteroid"

// Treasure Catalog
typedef struct {
    char* tObjName;
    char* tRAhRAm;
    char* tsDECdDECm;
    char* tCons;
    char* tObjType;
    char* tMag;
    char* tSize;
    char* tSubId;
} treasure_t; 

// Custom User Catalog
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

//===============================
class CatalogScreen {
  public:
    void draw(int catSel);
    void touchPoll();
    void updateStatus();

    char catSelectionStr1[26];
    char catSelectionStr2[11];
    char catSelectionStr3[11];
    char catSelectionStr4[16];
    char catSelectionStr5[15];
   
  private:
    bool loadTreasureArray();
    bool loadCustomArray();
    void parseTcatIntoArray();
    void parseCcatIntoArray();
    void drawACatPage();
    void writeTarget(uint16_t index);
    void showTargetCoords();

    bool treasureCatalog;
    bool customCatalog;
    bool shcCatalog;
    bool objSel;
    bool backSel;
    bool nextSel;
    bool saveTouched;
    bool delSelected;

    bool tEndOfList;
    bool cEndOfList;
    bool shcEndOfList;
    bool customItemSelected;

    uint16_t catButSelPos;
    uint16_t catButDetected;

    uint16_t tCurrentPage;
    uint16_t cCurrentPage;
    uint16_t shcCurrentPage;

    uint16_t tPrevPage;
    uint16_t cPrevPage;
    uint16_t returnToPage;

    uint16_t pre_tAbsIndex;
    uint16_t pre_tRelIndex;
    uint16_t pre_cAbsIndex;
    uint16_t pre_cRelIndex;
    uint16_t pre_shcIndex;

    uint16_t curSelTIndex;
    uint16_t curSelCIndex;
    uint16_t curSelSIndex;

    uint16_t tPrevRowIndex;
    uint16_t cPrevRowIndex;
    uint16_t shcPrevRowIndex;

    uint16_t tAbsRow;
    uint16_t cAbsRow;

    uint16_t tLastPage;
    uint16_t cLastPage;

    uint16_t shcLastPage;
    uint16_t shcLastRow;

    uint16_t tRow;
    uint16_t cRow;
    uint16_t shcRow;

    uint16_t cusRowEntries;
    uint16_t treRowEntries;

    // ======== Arrays ==========
    char       tRaSrCmd[MAX_TREASURE_ROWS][13]; 
    char      tDecSrCmd[MAX_TREASURE_ROWS][14];
    char      tRAhhmmss[MAX_TREASURE_ROWS][9];
    char    tDECsddmmss[MAX_TREASURE_ROWS][10];
    char Treasure_Array[MAX_TREASURE_ROWS][SD_CARD_LINE_LEN];
    char   treaCustWrSD[SD_CARD_LINE_LEN];
    uint16_t tFiltArray[MAX_TREASURE_ROWS];

    char      Custom_Array[MAX_CUSTOM_ROWS][SD_CARD_LINE_LEN];
    char Copy_Custom_Array[MAX_CUSTOM_ROWS][SD_CARD_LINE_LEN]; // save a copy for row deletion purposes
    char          cRaSrCmd[MAX_CUSTOM_ROWS][18]; 
    char         cDecSrCmd[MAX_CUSTOM_ROWS][18];
    uint16_t    cFiltArray[MAX_CUSTOM_ROWS];

    // Other Arrays
    double dtAlt[MAX_TREASURE_ROWS];
    double dtAzm[MAX_TREASURE_ROWS];
    double dcAlt[MAX_CUSTOM_ROWS];
    double dcAzm[MAX_CUSTOM_ROWS];
    uint16_t tPagingArrayIndex[MAX_TREASURE_PAGES];
    uint16_t cPagingArrayIndex[MAX_CUSTOM_PAGES];
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
extern CatalogScreen catalogScreen;

#endif
