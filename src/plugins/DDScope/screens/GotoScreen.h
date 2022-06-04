// =====================================================
// GotoScreen.h

#ifndef GOTO_S_H
#define GOTO_S_H

#include <Arduino.h>

class GotoScreen {
  public:
    void draw();
    void touchPoll(uint16_t px, uint16_t py);
    void updateThisStatus();
    
  private:
    void processNumPadButton();
    void setTargPolaris();
    
    char RAtext[8];
    char DECtext[8];
    char cmd[10];

    int buttonPosition;
    uint8_t RAtextIndex;
    uint8_t DECtextIndex;

    bool RAselect;
    bool RAclear;
    bool DECselect;
    bool DECclear;
    bool sendOn;
    bool setPolOn;
    bool timeOn;
    bool numDetected;
    bool goToButton;
    bool abortPgBut;
};

extern GotoScreen gotoScreen;

#endif