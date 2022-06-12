// =====================================================
// MoreScreen.h

#ifndef MORE_S_H
#define MORE_S_H

#include <Arduino.h>
#include "../display/Display.h"

class MoreScreen : public Display {
  public:
    void draw();
    void touchPoll(uint16_t px, uint16_t py);
    void updateThisStatus();

    bool objectSelected;
    uint8_t catSelected;
    uint16_t activeFilter;
    
  private:
    bool soundEnabled = true;
    bool goToButton = false;
    bool abortPgBut = false;
    bool clrCustom;
    bool sidereal = true;
    bool lunarRate = false;
    bool kingRate = false;
    bool incTrackRate;
    bool decTrackRate;
    bool rstTrackRate;
    bool filterBut;
    bool yesBut; 
    bool cancelBut;
    bool yesCancelActive;
};

extern MoreScreen moreScreen;

#endif