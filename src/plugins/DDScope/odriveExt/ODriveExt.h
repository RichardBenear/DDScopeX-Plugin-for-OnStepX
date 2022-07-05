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

typedef struct ODriveVersion {
  uint8_t hwMajor;
  uint8_t hwMinor;
  uint8_t hwVar;
  uint8_t fwMajor;
  uint8_t fwMinor;
  uint8_t fwRev;
} ODriveVersion;
    
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

    void getODriveVersion(ODriveVersion oDversion);
    uint32_t getODriveErrors(int axis, Component component);
    void demoMode(bool onState);
    
    // other actions
    void setODriveVelGain(int axis, float level);
    void setODriveVelIntGain(int axis, float level);
    void setODrivePosGain(int axis, float level);
    void updateODriveMotorPositions();
    void MotorEncoderDelta();
    void clearODriveErrors(int axis, int comp);
    void setHigherBaud();
    void clearAllODriveErrors();
    
    bool odriveAzmPwr = false;
    bool odriveAltPwr = false;

    bool oDserialAvail = false;

    ODriveVersion oDversion = {0, 0, 0, 0, 0, 0};
    Component component = COMP_FIRST;
    
  private:
    bool batLowLED = false;
};

extern ODriveExt oDriveExt;

#endif
