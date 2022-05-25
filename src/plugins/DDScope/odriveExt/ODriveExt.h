// =====================================================
// ODriveExt.h

#include <ODriveArduino.h>

#ifndef ODRIVEEXT_H
#define ODRIVEEXT_H

enum Component
{
    ENCODER,
    MOTOR,
    CONTROLLER,
    AXIS
};
    
// Printing with stream operator helper functions
template<class T> inline Print& operator <<(Print& obj, T arg) { obj.print(arg);    return obj; }
template<>        inline Print& operator <<(Print& obj, float arg) { obj.print(arg, 4); return obj; }

class ODriveExt
{
  public:
    // getters
    float getOdriveBusVoltage();
    float getEncoderPositionDeg(int axis);
    float getMotorPositionTurns(int axis);
    int   getMotorPositionCounts(int axis);
    float getMotorPositionDelta(int axis);
    float getMotorCurrent(int axis);
    float getMotorTemp(int axis);
    float getOdriveVelGain(int axis);
    float getOdriveVelIntGain(int axis);
    float getOdrivePosGain(int axis);
    
    // actions
    void setOdriveVelGain(int axis, float level);
    void setOdriveVelIntGain(int axis, float level);
    void setOdrivePosGain(int axis, float level);

    void updateOdriveMotorPositions();
    void clearOdriveErrors(int axis, int comp);
    
    static void demoMode(bool onState);
  
    // not currently used
    void clearAllOdriveErrors();
    int dumpOdriveErrors(int axis, int comp);
    int getOdriveRequestedState();

    bool odriveAZOff = true;
    bool odriveALTOff = true;

  private:
};

extern ODriveExt oDriveExt;

// Odrive Arduino functions
extern ODriveArduino oDriveArduino;

#endif
