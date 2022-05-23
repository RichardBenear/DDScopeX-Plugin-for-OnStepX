// =====================================================
// ODrive.h

#ifndef ODRIVE_H
#define ODRIVE_H

#include "../../../telescope/mount/Mount.h"

#define AZ      1 // Odrive Motor 1
#define ALT     0 // Odrive Motor 0

enum Component
{
    ENCODER,
    MOTOR,
    CONTROLLER,
    AXIS
};

#define odALT 0 // ODrive Motor 0 - Do not change
#define odAZM 1 // ODrive Motor 1 - Do not change

//#define ODRIVE_SLEW_MODE_STEPPER //when defined, slewing is at the Onstep step rate
//                                instead of the ODrive trapezoidal max velocity rate
    
class ODrive 
{
  public:
    void init();

    bool idleOdriveMotor(int axis);
    void stopMotors();
    bool turnOnOdriveMotor(int axis);

    float getOdriveBusVoltage();
    float getEncoderPositionDeg(int axis);
    float getMotorPositionTurns(int axis);
    int   getMotorPositionCounts(int axis);
    float getMotorCurrent(int axis);
    float getMotorTemp(int motor);
    float getOdriveVelGain(int axis);
    float getOdriveVelIntGain(int axis);
    float getOdrivePosGain(int axis);
    float getMotorPositionDelta(int axis);
    
    void setOdriveVelGain(int axis, float level);
    void setOdriveVelIntGain(int axis, float level);
    void setOdrivePosGain(int axis, float level);

    void updateOdriveMotorPositions();
    void clearOdriveErrors(int axis, int comp);
    
    static void demoModeOn();
    static void demoModeOff();
  
    // not currently used
    void clearAllOdriveErrors();
    int dumpOdriveErrors(int axis, int comp);
    int getOdriveRequestedState();

    bool axis1Enabled;
    bool axis2Enabled;
    bool odriveAZOff;
    bool odriveALTOff;
    //Mount mount;
  private:
};
extern ODrive odrive;

#endif
