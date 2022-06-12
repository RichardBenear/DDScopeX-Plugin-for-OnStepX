// =====================================================
// DCFocuserScreen.h

#ifndef DCFOCUSER_S_H
#define DCFOCUSER_S_H

#include <Arduino.h>
#include "../display/Display.h"

class DCFocuserScreen : public Display {
  public:
    void draw();
    void touchPoll(uint16_t px, uint16_t py);
    void updateThisStatus();
    
  private:
    void focInit();
    void focChangeDirection();
    void focMove(int numPulses, int pulseWidth);
    void updateFocPosition();

    bool focMovingIn;
    bool gotoSetpoint;
    bool focGoToHalf ;
    bool setPoint ;
    bool decSpeed ;
    bool incSpeed ;
    bool stepPhasePressed ;
    bool incMoveCt ;
    bool decMoveCt ;
    bool setZero ;
    bool setMax ;
    bool revFocuser ;
    bool inwardCalState; // start with inward calibration
    bool focReset ;
    bool calibActive ;
    bool focGoToActive ;
    int focMoveSpeed; // pulse width in microsec
    int focMoveDistance; // probably need to start with 30 after powering up
    int moveDistance;
    int current_focMoveDistance;
    int current_focMoveSpeed;
    int current_focMaxPos;
    int current_focMinPos;
    int current_focPos;
    int current_focDeltaMove;
    int focPosition;
    int focTarget;
    int focDeltaMove;
    int focMaxPosition;
    int focMinPosition;
    int setPointTarget;
};

extern DCFocuserScreen dCfocuserScreen;

#endif
