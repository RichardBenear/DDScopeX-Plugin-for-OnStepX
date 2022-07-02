// =====================================================
// MoreScreen.h

#ifndef MORE_S_H
#define MORE_S_H

#include <Arduino.h>
#include "../display/Display.h"

class MoreScreen : public Display {
  public:
    void draw();
    bool touchPoll(uint16_t px, uint16_t py);
    void updateMoreStatus();
    void updateMoreButtons(bool);
    bool moreButStateChange();

    bool objectSelected = false;
    unsigned int activeFilter;
    const char *activeFilterStr[3] = {"Filt: None", "Filt: Abv Hor", "Filt: All Sky"};
    
    char catSelectionStr1[26];
    char catSelectionStr2[11];
    char catSelectionStr3[11];
    char catSelectionStr4[16];
    char catSelectionStr5[15];

  private:
    bool catalogsActive = false;
    bool soundEnabled = true;
    bool goToButton = false;
    bool abortPgBut = false;
    bool clrCustom = false;
    bool sidereal = true;
    bool lunarRate = false;
    bool kingRate = false;
    bool incTrackRate = false;
    bool decTrackRate = false;
    bool rstTrackRate = false;
    bool filterBut = false;
    
    bool yesBut = false; 
    bool cancelBut =false;
    bool yesCancelActive = false;

    bool preSlewState = false;
    char preFilterState[10]="";
};

extern MoreScreen moreScreen;

#endif