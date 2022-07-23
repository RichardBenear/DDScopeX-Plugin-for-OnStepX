// =====================================================
// GuideScreen.h

#ifndef GUIDE_S_H
#define GUIDE_S_H

#include <Arduino.h>
#include "../display/Display.h"

class GuideScreen : public Display {
  public:
    void draw();
    bool touchPoll(uint16_t px, uint16_t py);
    void updateGuideStatus();
    void updateGuideButtons(bool);
    bool guideButStateChange();
  
  private:
    bool guidingEast  = false;
    bool guidingWest  = false;
    bool guidingNorth = false;
    bool guidingSouth = false;
    bool halfXisOn    = false;
    bool oneXisOn     = false;
    bool eightXisOn   = false;
    bool twentyXisOn  = true;
    bool HalfMaxisOn  = false;
    bool syncOn       = false;
    bool spiralOn     = false;
    bool stopPressed  = false;
    bool preSlewState = false;
};

extern GuideScreen guideScreen;

#endif