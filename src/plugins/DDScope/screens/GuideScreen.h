// =====================================================
// GuideScreen.h

#ifndef GUIDE_S_H
#define GUIDE_S_H

class GuideScreen {
  public:
    void draw();
    void touchPoll();
    void updateStatus();
    void updateStatusAll();
  
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