// =====================================================
// ExtStatusScreen.h

#ifndef EXTSTATUS_S_H
#define EXTSTATUS_S_H

#include "../display/Display.h"

class ExtStatusScreen : public Display {
  public:
    void draw();
    void mountStatus();
    void tlsStatus();
    void limitsStatus();
    void updateExStatus();

  private: 
    char exReply[50];
    
};
    
extern ExtStatusScreen extStatusScreen;

#endif
