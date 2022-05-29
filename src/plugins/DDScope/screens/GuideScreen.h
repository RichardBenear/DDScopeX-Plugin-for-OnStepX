// =====================================================
// GuideScreen.h

#ifndef GUIDE_S_H
#define GUIDE_S_H

#include <Arduino.h>

class GuideScreen {
  public:
    void draw();
    void touchPoll(uint16_t px, uint16_t py);
    void updateThisStatus();
  
  private:
    bool guidingEast;
    bool guidingWest;
    bool guidingNorth;
    bool guidingSouth;
    bool oneXisOn = true;
    bool eightXisOn;
    bool twentyXisOn;
    bool HalfMaxisOn;
    bool syncOn;
    bool spiralOn;
    bool stopPressed;
};

extern GuideScreen guideScreen;

#endif