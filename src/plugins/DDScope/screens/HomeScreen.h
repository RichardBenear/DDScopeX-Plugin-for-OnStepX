// =====================================================
// HomeScreen.h

#ifndef HOME_S_H
#define HOME_S_H

#include "Arduino.h"
#include "../display/display.h"

class HomeScreen : public Display {
  public:
    void draw();
    void updateHomeStatus();
    void updateHomeButtons();
    bool touchPoll(int16_t px, int16_t py);

  private:
   
    // button states
    bool parkWasSet = false;
    bool stopButton = false;
    bool gotoHome =false;
    bool fanOn = false;
};

extern HomeScreen homeScreen;

#endif