// =====================================================
// MoreScreen.h

#ifndef MORE_S_H
#define MORE_S_H

#include <Arduino.h>

class MoreScreen {
  public:
    void draw();
    void touchPoll();
    void updateThisStatus();

    bool objectSelected;
    uint8_t catSelected;
    uint16_t activeFilter;
    
  private:
    bool soundEnabled;
    bool goToButton;
    bool abortPgBut;
    bool catSelBut1;
    bool catSelBut2;
    bool catSelBut3;
    bool catSelBut4;
    bool catSelBut5;
    bool clrCustom;
    bool sideRate;
    bool lunarRate;
    bool kingRate;
    bool incTrackRate;
    bool decTrackRate;
    bool rstTrackRate;
    bool filterBut;
    bool yesBut; 
    bool cancelBut;
    bool yesCancelActive;
    double catMgrLst;
};

extern MoreScreen moreScreen;

#endif