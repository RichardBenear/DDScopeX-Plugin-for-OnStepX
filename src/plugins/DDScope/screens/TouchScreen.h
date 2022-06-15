// =====================================================
// TouchScreen.h

#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include "../display/Display.h"

class TouchScreen : public Display {
  public:
    void init();
    void touchScreenPoll(Screen);

  private:
    bool screenTouched = false;
    Screen tCurScreen = HOME_SCREEN;
};

extern TouchScreen touchScreen;

#endif
