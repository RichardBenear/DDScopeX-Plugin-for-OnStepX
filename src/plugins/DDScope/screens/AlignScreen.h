// =====================================================
// Align Screen.h
#ifndef ALIGN_S_H
#define ALIGN_S_H

#include <Arduino.h>
#include "../display/Display.h"

//States of the Align State machine
typedef enum {
    Idle_State,
    Home_State,
    Num_Stars_State,
    Select_Catalog_State,
    Wait_Catalog_State,
    Goto_State,
    Wait_For_Slewing_State,
    Align_State,
    Write_State,
} AlignStates;

class AlignScreen : public Display {
  public:
    void draw();
    void touchPoll(uint16_t px, uint16_t py);
    void updateThisStatus();
    void stateMachine();
   
  private:
    void getAlignStatus();
    void showCorrections();

    uint8_t alignCurStar = 0; // current align star number
    uint8_t numAlignStars = 0; // number of "selected" align stars from buttons 

    char numStarsCmd[3];
    char acorr[10];
    char stateError[20];
    char alignStatus[5];
    char maxAlign[30];
    char curAlign[30];
    char lastAlign[30];

    bool homeBut;
    bool catalogBut;
    bool gotoBut;
    bool aborted;
    bool abortBut;
    bool alignBut;
    bool saveAlignBut;
    bool startAlignBut;
    bool firstLabel = false;
};

extern AlignScreen alignScreen;

#endif