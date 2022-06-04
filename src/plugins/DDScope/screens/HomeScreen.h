// =====================================================
// HomeScreen.h

#ifndef HOME_S_H
#define HOME_S_H

#include "Arduino.h"

class HomeScreen {
  public:
    void draw();
    void updateThisStatus();
    void updateStatusCol1();
    void updateStatusCol2();
    void updateHomeButtons();
    void updateMountStatus();
    void touchPoll(int16_t px, int16_t py);

  private:
    float currentAZEncPos;
    float lastAZEncPos;
    float currentALTEncPos;
    float lastALTEncPos;
    float currentAZMotorCur;
    float lastAZMotorCur;
    float currentALTMotorCur;
    float lastALTMotorCur;
    float currentALTMotorTemp;
    float lastALTMotorTemp;
    float currentAZMotorTemp;
    float lastAZMotorTemp;
    char curLatitude[10];
    char curLongitude[10];
    char curTime[10];
    char curLST[10];
    char curTemp[10];
    char curHumidity[10];
    char curDewpoint[10];
    char curAlti[10];
    bool parkWasSet;
    bool stopButton;
    bool gotoHome;
    bool fanOn;
};

extern HomeScreen homeScreen;

#endif