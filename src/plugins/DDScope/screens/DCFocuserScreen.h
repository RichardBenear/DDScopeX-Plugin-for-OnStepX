// =====================================================
// DCFocuserScreen.h

#ifndef DCFOCUSER_S_H
#define DCFOCUSER_S_H

#include <Arduino.h>
#include "../display/Display.h"

class DCFocuserScreen : public Display {
  public:
    void draw();
    bool touchPoll(uint16_t px, uint16_t py);
    void updateFocuserButtons(bool);
    void updateFocuserStatus();
    static bool focuserButStateChange();
    
  private:
    void focInit();
    void focChangeDirection();
    void focMove(int numPulses, int pulseWidth);
    void updateFocPosition();

    bool redrawBut = false;
    bool focMovingIn;
    bool gotoSetpoint;
    bool focGoToHalf ;
    bool setPoint ;
    bool decSpeed ;
    bool incSpeed ;
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

    int focPosition;
    int focTarget;
    int focDeltaMove;
    int focMaxPosition;
    int focMinPosition;
    int setPointTarget;
};

extern DCFocuserScreen dCfocuserScreen;

#endif
