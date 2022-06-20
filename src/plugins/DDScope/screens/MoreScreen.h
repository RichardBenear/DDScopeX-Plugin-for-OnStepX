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
    void updateMoreButtons();

    bool objectSelected = false;
    uint8_t catSelected = 0;
    uint16_t activeFilter = 0;
    
  private:
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
    bool filterBut = true;
    
    bool yesBut = false; 
    bool cancelBut =false;
    bool yesCancelActive = false;
};

extern MoreScreen moreScreen;

#endif