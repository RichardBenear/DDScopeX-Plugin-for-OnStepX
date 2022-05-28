// =====================================================
// Align Screen.h
#ifndef ALIGN_S_H
#define ALIGN_S_H

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

class AlignScreen {
  public:
    void draw();
    void touchPoll();
    void updateThisStatus();
   
  private:
    void getAlignStatus();
    void showCorrections();
    
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
    bool firstLabel;
    bool dateWasSet;
    bool timeWasSet; 
};

extern AlignScreen alignScreen;

#endif