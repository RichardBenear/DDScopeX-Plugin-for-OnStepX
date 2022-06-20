// =====================================================
// GotoScreen.h

#ifndef GOTO_S_H
#define GOTO_S_H

#include <Arduino.h>
#include "../display/Display.h"

class GotoScreen : public Display {
  public:
    void draw();
    bool touchPoll(uint16_t px, uint16_t py);
    void updateGotoStatus();
    void updateGotoButtons();
    
  private:
    void processNumPadButton();
    void setTargPolaris();
    
    char RAtext[8] = "";
    char DECtext[8] = "";

    int buttonPosition = 0;
    uint8_t RAtextIndex = 0;
    uint8_t DECtextIndex = 0;

    bool RAselect = false;
    bool RAclear = false;
    bool DECselect = false;
    bool DECclear = false;
    bool sendOn = false;
    bool setPolOn = false;
    bool numDetected = false;
    bool goToButton = false;
    bool abortPgBut = false;
};

extern GotoScreen gotoScreen;

#endif