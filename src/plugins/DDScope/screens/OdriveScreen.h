// -------------------------------------------------------------------------------------------------
// ODriveScreen.h

#ifndef ODRIVE_S_H
#define ODRIVE_S_H

#include <Arduino.h>
#include "../odriveExt/ODriveExt.h"
#include "../display/display.h"

class ODriveScreen : public Display {
  public:
    void draw();
    bool touchPoll(uint16_t px, uint16_t py);
    void updateOdriveStatus();
    void updateOdriveButtons();
    void decodeODriveErrors(int axis, Component, uint32_t errorCode);
    
  private:
    void showODriveErrors();
    void showGains();

    static const uint8_t box_height_adj = 10;
    bool clearODriveErr;
    bool resetODriveFlag;
    bool AZgainHigh;
    bool AZgainDefault;
    bool ALTgainHigh;
    bool ALTgainDefault;
    bool OdStopButton;
    bool demoActive;
    bool ODpositionUpdateEnabled;
    int demoHandle;
};

extern ODriveScreen oDriveScreen;

#endif