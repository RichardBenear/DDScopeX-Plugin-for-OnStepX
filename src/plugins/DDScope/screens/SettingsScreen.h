// =====================================================
// SettingsScreen.h

#ifndef SETTINGS_S_H
#define SETTINGS_S_H

#include <Arduino.h>
#include "../display/Display.h"

class SettingsScreen : public Display {
  public:
    void draw();
    bool touchPoll(uint16_t px, uint16_t py);
    void updateSettingsButtons(bool);
    bool settingsButStateChange();
    void updateSettingsStatus();
  
  private:
    void setProcessNumPadButton();
  
    char sNumLabels[12][3] = {"9", "8", "7", "6", "5", "4", "3", "2", "1", "-", "0", "+"};
    char Ttext[8];
    char Dtext[8];
    char Tztext[8];
    char LaText[8];
    char LoText[8]; 
    char sCmd[12];

    int sButtonPosition;

    uint8_t TtextIndex;
    uint8_t DtextIndex;
    uint8_t TztextIndex;
    uint8_t LaTextIndex;
    uint8_t LoTextIndex;
    
    bool Tselect    = true;
    bool Tclear     = false;
    bool Dselect    = false;
    bool Dclear     = false;
    bool Tzselect   = false;
    bool Tzclear    = false;
    bool LaSelect   = false;
    bool LaClear    = false;
    bool LoSelect   = false;
    bool LoClear    = false;
    bool sSendOn    = false;
    bool siteOn     = false;
    bool sNumDetected = false;
};

extern SettingsScreen settingsScreen;

#endif