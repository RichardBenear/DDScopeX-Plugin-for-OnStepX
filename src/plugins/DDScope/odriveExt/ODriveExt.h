// =====================================================
// ODriveExt.h

#if ODRIVE_COMM_MODE == OD_UART
  #include <ODriveArduino.h>

  
#endif

#include "../display/Display.h"

#ifndef ODRIVEEXT_H
#define ODRIVEEXT_H

enum Component
{
  COMP_FIRST,
  NO_COMP,
  AXIS,
  CONTROLLER,
  MOTOR,
  ENCODER,
  COMP_LAST
};

typedef struct ODriveVersion {
  uint8_t hwMajor;
  uint8_t hwMinor;
  uint8_t hwVar;
  uint8_t fwMajor;
  uint8_t fwMinor;
  uint8_t fwRev;
} ODriveVersion;


class ODriveExt : public Display {
  public:
    // getters
    int getMotorPositionCounts(int axis);
    uint8_t getODriveCurrentState(int axis);

    float getEncoderPositionDeg(int axis);
    float getMotorPositionTurns(int axis);
    float getMotorPositionDelta(int axis);
    float getMotorCurrent(int axis);
    float getMotorTemp(int axis);
    float getODriveVelGain(int axis);
    float getODriveVelIntGain(int axis);
    float getODrivePosGain(int axis);
    float getODriveBusVoltage();
    void getODriveVersion(ODriveVersion);

    uint32_t getODriveErrors(int axis, Component component);
    void demoMode();
    
    // other actions
    void setODriveVelGains(int axis, float level, float intLevel);
    void setODrivePosGain(int axis, float level);
    void updateODriveMotorPositions();
    void MotorEncoderDelta();
    void clearODriveErrors(int axis, int comp);
    void setHigherBaud();
    void clearAllODriveErrors();

    bool oDserialAvail = false;

    Component component = COMP_FIRST;
    
  private:
    bool batLowLED = false;
    bool oDriveRXoff = false;
};

extern ODriveExt oDriveExt;

#endif
