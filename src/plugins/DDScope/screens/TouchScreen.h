// =====================================================
// TouchScreen.h

#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include <XPT2046_Touchscreen.h>
#include "../../../pinmaps/Pins.DDtPCB.h"

static XPT2046_Touchscreen ts(TS_CS, TS_IRQ); // Use Interrupts for touchscreen
static TS_Point p;

class TouchScreen {
  public:
    void init();
    void touchScreenPoll();

  private:
};

extern TouchScreen touchScreen;

#endif
