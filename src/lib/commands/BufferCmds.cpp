// -----------------------------------------------------------------------------------
// Command processing

#include <Arduino.h>
#include "../Constants.h"
#include "BufferCmds.h"

void Buffer::init(int mountType) {
  this->mountType = mountType;
}

bool Buffer::add(char c) {
  if (cbp > bufferSize - 1) {
    Serial.printf("[FATAL] Buffer overrun! cbp=%d\n", cbp);
    flush(); return false;
  }

  // (chr)6 is a special status command for the LX200 protocol
  if (c == (char)6 && cbp == 0) {
    cb[0] = ':'; 
    cb[1] = (char)6;
    if (mountType == 3) cb[2] = 'A'; else cb[2] = 'P';
    cb[3] = 0; 
    cbp = 3; 
    c = '#';
  }

  // ignore spaces/lf/cr
  if (c != (char)32 && c != (char)10 && c != (char)13 && c != (char)6) {
    //if (cbp > bufferSize-2) cbp = bufferSize - 2;
    cb[cbp] = c; 
    cbp++; 
    cb[cbp] = (char)0;
  }

  if (c == '#') {
    // validate the command frame, normal command
    if (!(cbp > 1) && ((cb[0] == ':') || (cb[0] == ';')) && (cb[cbp-1] == '#')) { 
      flush(); 
      return false; 
    }
  
    if (((cb[0] == ':') || (cb[0] == ';')) && (cb[1] == '#') && (cb[2] == 0)) { 
      flush(); 
      return false; 
    }
    checksum=(cb[0] == ';');
    if (checksum) {
      byte len=strlen(cb)-1;

      // Minimum length for a valid command is 5 ';CCS#'
      if (len < 5) {
        flush(); 
        cb[0]=':'; 
        cb[1]=(char)6; 
        cb[2]='0'; 
        cb[3]='#'; 
        cb[4]=0; 
        cbp=4; 
        return true; 
      }
      
      // checksum the data, for example ";111111CCS#".  I don't include the command frame in the checksum.  The error response is a checksumed string "CK_FAILS#" to request re-transmit.
      byte cks=0; for (int cksCount0=1; cksCount0 < len-3; cksCount0++) {  cks+=cb[cksCount0]; }
      char chkSum[3]; sprintf(chkSum,"%02X",cks);
      seq=cb[len-1];
      if (!((chkSum[0] == cb[len-3]) && (chkSum[1] == cb[len-2]))) { 
        flush(); 
        cb[0]=':'; 
        cb[1]=(char)6; 
        cb[2]='0'; 
        cb[3]='#'; 
        cb[4]=0; 
        cbp=4; 
        return true;
      }
      // remove the sequence char and checksum from string
      --len; --len; cb[--len]=0;
    }
    return true;
  } else {
    return false;
  }
}

char* Buffer::getCmd() {
  // the command is either one or two chars in length
  cmd[0] = 0;
  memmove(cmd, (char *)&cb[1], 2);
  cmd[2] = 0;
  if (cmd[1] == '#' && cmd[2] == 0) cmd[1] = 0;
  return cmd;
}

char* Buffer::getParameter() {
  // the remaining parameter
  pb[0] = 0;
  if (cbp > 4) memmove(pb, (char *)&cb[3], cbp-4);
  pb[cbp-4] = 0;
  return pb;
}

// char* Buffer::getParameter() {
//   pb[0] = 0;

//   if (cbp > 4) {
//     memmove(pb, (char *)&cb[3], cbp - 4);
//     pb[cbp - 4] = 0;
//   } else {
//     pb[0] = 0;  // empty parameter
//   }

//   return pb;
// }

char* Buffer::getSeq() {
  static char s[2] = " ";
  s[0] = seq;
  return s;
}

bool Buffer::ready() {
  if (!cbp) return false;
  if (cb[cbp-1] == '#' && cbp == 1) flush();
  return (cb[cbp-1] == '#');
}

bool Buffer::flush() {
  cbp = 0;
  cb[0] = (char)0;
  return true;
}
