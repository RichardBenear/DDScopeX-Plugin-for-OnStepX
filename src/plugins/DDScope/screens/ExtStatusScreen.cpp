// ==============================================
// ========== Extended Status Screen ============
// ==============================================
// Author: Richard Benear
// 8/30/2021

#include "ExtStatusScreen.h"
#include "src/telescope/mount/site/Site.h"

#define STATUS_BOXSIZE_X         53 
#define STATUS_BOXSIZE_Y         27 
#define STATUS_BOX_X             40 
#define STATUS_BOX_Y            150 
#define STATUS_X                  3 
#define STATUS_Y                101 
#define STATUS_SPACING           14 

// ========== Draw the Extended Status Screen ==========
void ExtStatusScreen::draw() {
  setCurrentScreen(XSTATUS_SCREEN);
  setNightMode(getNightMode());
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  drawMenuButtons();
  drawTitle(68, TITLE_TEXT_Y, "Extended Status");

  int y_offset = STATUS_Y;
  int y_spacer = STATUS_SPACING;
  char xchanReply[50];

  //tft.setCursor(STATUS_X, STATUS_Y); 
  //tft.fillRect(STATUS_X, STATUS_Y, 150, 200, pgBackground);
  
  // :GVD#      Get OnStepX Firmware Date
  //            Returns: MTH DD YYYY#
  // :GVM#      General Message
  //            Returns: s# (where s is a string up to 16 chars)
  // :GVN#      Get OnStepX Firmware Number
  //            Returns: M.mp#
  // :GVP#      Get OnStepX Product Name
  //            Returns: s#
  // :GVT#      Get OnStepX Firmware Time
  //            Returns: HH:MM:SS#
  tft.setCursor(STATUS_X, y_offset); 
  getLocalCmdTrim(":GVN#", xchanReply); // Get OnStep FW Version
  tft.print("OnStep FW Version: "); tft.print(xchanReply);
  
  // Begin parsing :GU# status data by getting the status string via local command channel
  // process the status string
  int i = 0;
  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset); 
  if (strstr(xchanReply,"n")) tft.print("Not Tracking"); else tft.print("Tracking   ");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  i++; 
  if (strstr(xchanReply,"N")) tft.print("Not Slewing"); else tft.print("Slewing    ");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  i++;
  if      (strstr(xchanReply,"p")) tft.print("Not Parked        ");
  else if (strstr(xchanReply,"I")) tft.print("Parking in process");
  else if (strstr(xchanReply,"P")) tft.print("Parked            ");
  else if (strstr(xchanReply,"F")) tft.print("Parking Failed    ");
  else                             tft.print("ERROR             ");
  
  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  i++;
  if (strstr(xchanReply,"H")) tft.print("Homed    "); else tft.print("Not Homed");

  #if TIME_LOCATION_PPS_SENSE != OFF
    y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  i++;
    if (strstr(xchanReply,"S")) tft.print("PPS Synched    "); else tft.print("PPS Not Synched");
  #endif

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  i++;
  if (strstr(xchanReply,"G")) tft.print("Pulse Guide Active  "); else tft.print("Pulse Guide Inactive");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  i++;
  if (strstr(xchanReply,"g")) tft.print("Guide Active    "); else tft.print("Guiding Inactive");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  i++;
  if (strstr(xchanReply,"r")) tft.print("Refraction Enabled Dual Axis");
  if (strstr(xchanReply,"rs")) tft.print("Refraction Enabled Single Axis");
  if (strstr(xchanReply,"t")) tft.print("OnTrack Enabled Dual Axis");
  if (strstr(xchanReply,"ts")) tft.print("OnTrack Enabled Single Axis");
  else tft.print("Rate Compensation None");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  i++;
  tft.print("Tracking Rate = "); 
  if (strstr(xchanReply,"(")) tft.print("Lunar");
  else if (strstr(xchanReply,"O")) tft.print("Solar");
  else if (strstr(xchanReply,"k")) tft.print("King");
  else tft.print("Sidereal");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  i++; 
  if (strstr(xchanReply,"w")) tft.print("Waiting At Home    "); else tft.print("Not Waiting At Home");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  i++;
  if (strstr(xchanReply,"u")) tft.print("Pause At Home Enabled      "); else tft.print("Pausing-At-Home Not Enabled");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  i++;
  if (strstr(xchanReply,"z")) tft.print("Buzzer Enabled "); else tft.print("Buzzer Disabled");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  i++;
  if (strstr(xchanReply,"a")) tft.print("Auto Meridian Flip: Enabled"); else tft.print("Auto Meridian Flip: Disabled");

  #if AXIS1_PEC == ON
    y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  i++;
    if (strstr(xchanReply,"R")) tft.print("PEC was recorded");
    if (transform.mountType != ALTAZM) {
      tft.print("PEC state = ");
      if      (strstr(xchanReply,"/")) tft.print("PEC Ignored");
      else if (strstr(xchanReply,",")) tft.print("PEC ready lay");
      else if (strstr(xchanReply,"~")) tft.print("PEC laying");
      else if (strstr(xchanReply,";")) tft.print("PEC ready record");
      else if (strstr(xchanReply,"^")) tft.print("PEC recording");
    }
  #endif

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  i++;
  if (strstr(xchanReply,"A")) tft.print("ALTAZM Mount"); 
  else if (strstr(xchanReply,"K")) tft.print("FORK Mount"); 
  else if (strstr(xchanReply,"E")) tft.print("GEM Mount"); 
  else tft.print("No Mount ");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  i++;
  if (strstr(xchanReply,"o")) tft.print("pier side NONE"); 
  else if (strstr(xchanReply,"T")) tft.print("pier side EAST"); 
  else if (strstr(xchanReply,"W")) tft.print("pier side WEST");
  else tft.print("pier side N/A") ;

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  i++;
  tft.print("Pulse Guide Rate = "); tft.print(xchanReply[i] - '0');

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  i++; 
  tft.print("Guide Rate = "); tft.print(xchanReply[i] - '0');

  int generalError = xchanReply[i] - '0';

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  tft.print("General Error = "); 
  if (generalError == 0)  tft.print("ERR_NONE         "); else
  if (generalError == 1)  tft.print("ERR_MOTOR_FAULT  "); else
  if (generalError == 2)  tft.print("ERR_ALT_MIN      "); else
  if (generalError == 3)  tft.print("ERR_LIMIT_SENSE  "); else
  if (generalError == 4)  tft.print("ERR_DEC          "); else
  if (generalError == 5)  tft.print("ERR_AZM          "); else
  if (generalError == 6)  tft.print("ERR_UNDER_POLE   "); else
  if (generalError == 7)  tft.print("ERR_MERIDIAN     "); else
  if (generalError == 8)  tft.print("ERR_SYNC         "); else
  if (generalError == 9)  tft.print("ERR_PARK         "); else
  if (generalError == 10) tft.print("ERR_GOTO_SYNC    "); else
  if (generalError == 11) tft.print("ERR_ALT_MAX      "); else
  if (generalError == 12) tft.print("ERR_UNSPECIFIED  "); else
  if (generalError == 13) tft.print("ERR_WEATHER_INIT "); else
  if (generalError == 14) tft.print("ERR_SITE_INIT    "); else
  if (generalError == 15) tft.print("ERR_NV_INIT      ");
  // end :GU# data string parsing

  // Other status information not from :GU# command
  // Get and show the Time and Location status
  char tempReply[10];

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  // show Local Time 24 Hr format
  getLocalCmdTrim(":GL#", tempReply); 
  tft.print("Local Time = "); tft.print(tempReply);

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  // show Current Date
  getLocalCmdTrim(":GC#", tempReply);
  tft.print("Local Time = "); tft.print(tempReply); 

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  // show TZ Offset
  getLocalCmdTrim(":GG#", tempReply);   
  tft.print("Time Zone = "); tft.print(tempReply);
  
  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  // show Latitude
  getLocalCmdTrim(":Gt#", tempReply); 
  tft.print("Time Zone = "); tft.print(tempReply);

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  // show Longitude
  getLocalCmdTrim(":Gg#", tempReply); 
  tft.print("Time Zone = "); tft.print(tempReply);
    
  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  getLocalCmdTrim(":GX80#", tempReply); 
  tft.print("UTC Time and Date = "); tft.print(tempReply);
  getLocalCmdTrim(":GX81#", tempReply); 
  tft.print(" : "); tft.print(tempReply);
}

ExtStatusScreen extStatusScreen;
