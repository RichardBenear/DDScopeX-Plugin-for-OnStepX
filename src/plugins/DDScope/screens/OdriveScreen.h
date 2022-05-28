// -------------------------------------------------------------------------------------------------
// ODriveScreen.h

#ifndef ODRIVE_S_H
#define ODRIVE_S_H

#include <Arduino.h>

class ODriveScreen {
  public:
    void draw();
    void touchPoll();
    void updateOdriveErrors();
    void updateThisStatus();
    
  private:
    void decodeOdriveError(uint32_t errorCode);
    void decodeAxisError(int axis, uint32_t errorCode);
    void decodeMotorError(int axis, uint32_t errorCode);
    void decodeControllerError(int axis, uint32_t errorCode);
    void decodeEncoderError(int axis, uint32_t errorCode);
    
    static const uint8_t box_height_adj = 10;
    bool clearOdriveErr;
    bool resetOdriveFlag;
    bool AZgainHigh;
    bool AZgainDefault;
    bool ALTgainHigh;
    bool ALTgainDefault;
    bool OdStopButton;
    bool demoActive;
    bool ODpositionUpdateEnabled;
    unsigned int lastOdriveErr;
    unsigned int lastALTErr;
    unsigned int lastALTCtrlErr;
    unsigned int lastALTMotorErr;
    unsigned int lastALTEncErr;
    unsigned int lastAZErr;
    unsigned int lastAZCtrlErr;
    unsigned int lastAZMotorErr;
    unsigned int lastAZEncErr;
    int current_AZ_ODerr;
    int current_ALT_ODerr;        
    int demoHandle;
};

extern ODriveScreen oDriveScreen;

#endif