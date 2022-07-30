// =====================================================
// ODriveExt.h

#include <ODriveArduino.h>
#include "../display/Display.h"

#ifndef ODRIVEEXT_H
#define ODRIVEEXT_H

enum Component
{
  COMP_FIRST,
  NONE,
  AXIS,
  CONTROLLER,
  MOTOR,
  ENCODER,
  COMP_LAST
};

// Printing with stream operator helper functions
template<class T> inline Print& operator <<(Print& obj, T arg) { obj.print(arg);    return obj; }
template<>        inline Print& operator <<(Print& obj, float arg) { obj.print(arg, 4); return obj; }

class ODriveExt : public Display {
  public:
    // getters
    int   getMotorPositionCounts(int axis);
    int   getODriveCurrentState(int axis);

    float getEncoderPositionDeg(int axis);
    float getMotorPositionTurns(int axis);
    float getMotorPositionDelta(int axis);
    float getMotorCurrent(int axis);
    float getMotorTemp(int axis);
    float getODriveVelGain(int axis);
    float getODriveVelIntGain(int axis);
    float getODrivePosGain(int axis);
    float getODriveBusVoltage();

    uint32_t getODriveErrors(int axis, Component component);
    void demoMode();
    
    // other actions
    void setODriveVelGain(int axis, float level);
    void setODriveVelIntGain(int axis, float level);
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
