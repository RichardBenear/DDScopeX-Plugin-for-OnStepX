// -------------------------------------------------------------------------------------------------
// ODriveScreen.h

#ifndef ODRIVE_S_H
#define ODRIVE_S_H

#include <Arduino.h>
#include "../odriveExt/ODriveExt.h"
//#include "../display/display.h"

class ODriveScreen : public Display {
  public:
    void draw();
    bool touchPoll(uint16_t px, uint16_t py);
    void updateOdriveStatus();
    void updateOdriveButtons(bool);
    bool odriveButStateChange();
    void decodeODriveErrors(int axis, Component, uint32_t errorCode);
    
  private:
    void showODriveErrors();
    void showGains();

    static const uint8_t box_height_adj = 10;
    bool clearODriveErr   = false;
    bool resetODriveFlag  = false;
    bool AZgainHigh       = false;
    bool AZgainDefault    = true;
    bool ALTgainHigh      = false;
    bool ALTgainDefault   = true;
    bool OdStopButton     = false;
    bool demoActive       = false;
    bool ODpositionUpdateEnabled = true;
    int preAzmState       = 0;
    int preAltState       = 0;
    int demoHandle;
};

extern ODriveScreen oDriveScreen;

#endif