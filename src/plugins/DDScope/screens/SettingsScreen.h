// =====================================================
// SettingsScreen.h

#ifndef SETTINGS_S_H
#define SETTINGS_S_H

#include <Arduino.h>
#include "../display/display.h"

class SettingsScreen : public Display {
  public:
    void draw();
    bool touchPoll(uint16_t px, uint16_t py);
    void updateSettingsButtons();
    void updateSettingsStatus();
    
  private:
    void setProcessNumPadButton();
  
    char sNumLabels[12][3] = {"9", "8", "7", "6", "5", "4", "3", "2", "1", "-", "0", "+"};
    char Ttext[8];
    char Dtext[8];
    char Tztext[8];
    char LaText[8];
    char LoText[8];
    int sButtonPosition;
    uint8_t TtextIndex;
    uint8_t DtextIndex;
    uint8_t TztextIndex;
    uint8_t LaTextIndex;
    uint8_t LoTextIndex;
    char sCmd[12];
    bool Tselect;
    bool Tclear;
    bool Dselect;
    bool Dclear;
    bool Tzselect;
    bool Tzclear;
    bool LaSelect;
    bool LaClear;
    bool LoSelect;
    bool LoClear;
    bool sSendOn;
    bool siteOn;
    bool sNumDetected;
};

extern SettingsScreen settingsScreen;

#endif