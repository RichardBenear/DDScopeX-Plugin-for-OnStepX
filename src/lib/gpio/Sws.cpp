// -----------------------------------------------------------------------------------
// Smart Web Server GPIO support (optional when SWS encoders are not used)

#include "Sws.h"

#if defined(GPIO_DEVICE) && GPIO_DEVICE == SWS

#include "../tasks/OnTask.h"
#include "../../libApp/commands/ProcessCmds.h"

// check for SWS device
bool SwsGpio::init() {
  static bool initialized = false;
  if (initialized) return found;

  found = true;

  return found;
}

// no command processing
bool SwsGpio::command(char *reply, char *command, char *parameter, bool *supressFrame, bool *numericReply, CommandError *commandError) {
  char *conv_end;

  if (command[0] == 'G') {
    if (command[1] == 'X' && parameter[2] == 0)  {

      // :GXGO#     Get Gpio Output mode or State
      //            Returns: Value
      if (parameter[0] == 'G' && parameter[1] == 'O') {
        for (int i = 0; i < 8; i++) {
          char newMode = 'X';
          if (mode[i] == INPUT) newMode = 'I'; else
          if (mode[i] == INPUT_PULLUP) newMode = 'U'; else
          if (mode[i] == OUTPUT) newMode = 'O';

          if (virtualMode[i] != newMode) {
            reply[i] = newMode;
            virtualMode[i] = newMode;
          } else {
            if (virtualMode[i] == 'O') { reply[i] = (virtualWrite[i]) ? '1' : '0'; } else reply[i] = 'X';
          }

          reply[i + 1] = 0;
        }
        *numericReply = false;
      } else return false;
    } else return false;
  } else

  if (command[0] == 'S') {
    if (command[1] == 'X') {
      if (parameter[2] != ',') { *commandError = CE_PARAM_FORM; return true; }

      // :SXG[n],[v]#   Set Gpio input [n]umber to [v]alue
      //              Return: 0 on failure
      //                      1 on success
      if (parameter[0] == 'G' && parameter[1] >= '0' && parameter[1] <= '7') {
        int i = parameter[1] - '0';
        int v = atol(&parameter[3]);
        if (v == 0 || v == 1) {
          virtualRead[i] = v;
        } else *commandError = CE_PARAM_RANGE;
      } else return false;
    } else return false;
  } else return false;

  return true;
}

// set GPIO pin (0 to 7) mode for INPUT, INPUT_PULLUP, or OUTPUT
void SwsGpio::pinMode(int pin, int mode) {
  if (found && pin >= 0 && pin <= 7) {
    #ifdef INPUT_PULLDOWN
      if (mode == INPUT_PULLDOWN) mode = INPUT;
    #endif
    if (mode == INPUT_PULLUP) mode = INPUT;
    this->mode[pin] = mode;
  }
}

// one four channel SWS GPIO is supported, this gets the last set value
int SwsGpio::digitalRead(int pin) {
  if (found && pin >= 0 && pin <= 7) {
    if (mode[pin] == INPUT || mode[pin] == INPUT_PULLUP) {
      return virtualRead[pin];
    } else return state[pin]; 
  } else return 0;
}

// one four channel SWS GPIO is supported, this sets each output on or off
void SwsGpio::digitalWrite(int pin, bool value) {
  if (found && pin >= 0 && pin <= 7) {
    state[pin] = value;
    virtualWrite[pin] = value;
  } else return;
}

SwsGpio gpio;

#endif
