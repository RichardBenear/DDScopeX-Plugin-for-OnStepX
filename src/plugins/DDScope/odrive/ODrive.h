// =====================================================
// ODrive.h

#ifndef ODRIVE_H
#define ODRIVE_H

enum Component
{
    ENCODER,
    MOTOR,
    CONTROLLER,
    AXIS
};
    
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
    
    static void demoModeOn();
    static void demoModeOff();
  
    // not currently used
    void clearAllOdriveErrors();
    int dumpOdriveErrors(int axis, int comp);
    int getOdriveRequestedState();

  private:
};

extern ODrive oDrive;

#endif
