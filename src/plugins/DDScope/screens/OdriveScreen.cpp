// =====================================================
// ODriveScreen.cpp
//
// Author: Richard Benear 2022

#include <ODriveArduino.h> // https://github.com/odriverobotics/ODrive/tree/master/Arduino/ODriveArduino
#include "../odriveExt/ODriveExt.h"
#include "ODriveScreen.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "../../../telescope/mount/Mount.h"
#include "src/lib/tasks/OnTask.h"

#define OD_ERR_OFFSET_X           4 
#define OD_ERR_OFFSET_Y         188
#define OD_ERR_SPACING           14 
#define OD_BUTTONS_OFFSET        45 

// Buttons for actions that are not page selections
#define OD_ACT_BOXSIZE_X         100 
#define OD_ACT_BOXSIZE_Y         36 
#define OD_ACT_COL_1_X           3 
#define OD_ACT_COL_1_Y           324
#define OD_ACT_COL_2_X           OD_ACT_COL_1_X+OD_ACT_BOXSIZE_X+4
#define OD_ACT_COL_2_Y           OD_ACT_COL_1_Y
#define OD_ACT_COL_3_X           OD_ACT_COL_2_X+OD_ACT_BOXSIZE_X+4
#define OD_ACT_COL_3_Y           OD_ACT_COL_1_Y
#define OD_ACT_X_SPACING         7
#define OD_ACT_Y_SPACING         3

// Printing with stream operator helper functions
template<class T> inline Print& operator <<(Print& obj,     T arg) { obj.print(arg   ); return obj; }
template<>        inline Print& operator <<(Print& obj, float arg) { obj.print(arg, 4); return obj; }

// ODrive Screen Button object
Button odriveButton(
                OD_ACT_COL_1_X, OD_ACT_COL_1_Y, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y,
                butOnBackground, 
                butBackground, 
                butOutline, 
                mainFontWidth, 
                mainFontHeight, 
                "");

// Demo Mode Wrapper
void demoWrapper() { oDriveExt.demoMode(); }

typedef struct ODriveVersion {
    uint8_t hwMajor;
    uint8_t hwMinor;
    uint8_t hwVar;
    uint8_t fwMajor;
    uint8_t fwMinor;
    uint8_t fwRev;
    } ODriveVersion;

ODriveVersion oDversion;

//****** Draw ODrive Screen ******
void ODriveScreen::draw() {
  setCurrentScreen(ODRIVE_SCREEN);
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  
  drawMenuButtons();
  drawTitle(120, TITLE_TEXT_Y, "ODrive");
  tft.setFont(&Inconsolata_Bold8pt7b);

  drawCommonStatusLabels();
  updateOdriveButtons(false);
  showODriveErrors();
  showGains();

  #if ODRIVE_COMM_MODE == OD_UART
    ODRIVE_SERIAL << "r hw_version_major\n";
    oDversion.hwMajor = _oDriveDriver->readInt();

    ODRIVE_SERIAL << "r hw_version_minor\n";
    oDversion.hwMinor = _oDriveDriver->readInt();

    ODRIVE_SERIAL << "r hw_version_variant\n";
    oDversion.hwVar = _oDriveDriver->readInt();

    ODRIVE_SERIAL << "r fw_version_major\n";
    oDversion.fwMajor = _oDriveDriver->readInt();

    ODRIVE_SERIAL << "r fw_version_minor\n";
    oDversion.fwMinor = _oDriveDriver->readInt(); 

    ODRIVE_SERIAL << "r fw_version_revision\n";
    oDversion.fwRev = _oDriveDriver->readInt();
  #elif ODRIVE_COMM_MODE == OD_CAN
  // needs implemented
    oDversion.hwMajor = 3;
    oDversion.hwMinor = 6;
    oDversion.hwVar   = 24;
    oDversion.fwMajor = 0;
    oDversion.fwMinor = 5;
    oDversion.fwRev   = 4;
  #endif

  tft.setFont(); // default
  tft.setCursor(92, 164);
  tft.print("HW Version:"); tft.print(oDversion.hwMajor); tft.print("."); tft.print(oDversion.hwMinor); tft.print("."); tft.print(oDversion.hwVar);
  tft.setCursor(92, 176);
  tft.print("FW Version:"); tft.print(oDversion.fwMajor); tft.print("."); tft.print(oDversion.fwMinor); tft.print("."); tft.print(oDversion.fwRev);
}

// status update for this screen
void ODriveScreen::updateOdriveStatus() {
  updateCommonStatus();
  getOnStepCmdErr(); // show error bar
}

// ====== Show the Gains ======
void ODriveScreen::showGains() {
  // Show AZM Velocity Gain - AZ is motor 1
  tft.setFont();
  float temp = oDriveExt.getODriveVelGain(AZM_MOTOR);
  tft.setCursor(203,282); tft.print("AZM Vel  Gain:");
  tft.fillRect(285,282, 39, 10, pgBackground); 
  tft.setCursor(285,282); tft.print(temp);
  
  // Show AZM Velocity Integrator Gain
  temp = oDriveExt.getODriveVelIntGain(AZM_MOTOR);
  tft.setCursor(203,292); tft.print("AZM VelI Gain:");
  tft.fillRect(285,292, 39, 10, pgBackground); 
  tft.setCursor(285,292); tft.print(temp);

  // Show ALT Velocity Gain - ALT is motor 0
  temp = oDriveExt.getODriveVelGain(ALT_MOTOR);
  tft.setCursor(203,302); tft.print("ALT Vel  Gain:");
  tft.fillRect(285,302, 39, 10, pgBackground); 
  tft.setCursor(285,302); tft.print(temp);

  // Show ALT Velocity Integrator Gain
  temp = oDriveExt.getODriveVelIntGain(ALT_MOTOR);
  tft.setCursor(203,312); tft.print("ALT VelI Gain:");
  tft.fillRect(285,312, 39, 10, pgBackground); 
  tft.setCursor(285,312); tft.print(temp);
  tft.setFont(&Inconsolata_Bold8pt7b);
}

// ===== Decode all ODrive Errors =====
void ODriveScreen::decodeODriveErrors(int axis, Component comp, uint32_t errorCode) {

  if (axis == -1) { //then ODrive top level decode, not axis specific
    if      (errorCode == ODRIVE_ERROR_NONE)                          tft.println("ERROR_NONE");
    else if (errorCode == ODRIVE_ERROR_CONTROL_ITERATION_MISSED)      tft.println("ERROR_CONTROL_ITERATION_MISSED");
    else if (errorCode == ODRIVE_ERROR_DC_BUS_UNDER_VOLTAGE)          tft.println("ERROR_DC_BUS_UNDER_VOLTAGE");
    else if (errorCode == ODRIVE_ERROR_DC_BUS_OVER_VOLTAGE)           tft.println("ERROR_DC_BUS_OVER_VOLTAGE");
    else if (errorCode == ODRIVE_ERROR_DC_BUS_OVER_REGEN_CURRENT)     tft.println("ERROR_DC_BUS_OVER_REGEN_CURRENT");
    else if (errorCode == ODRIVE_ERROR_DC_BUS_OVER_CURRENT)           tft.println("ERROR_DC_BUS_OVER_CURRENT");
    else if (errorCode == ODRIVE_ERROR_BRAKE_DEADTIME_VIOLATION)      tft.println("ERROR_BRAKE_DEADTIME_VIOLATION");
    else if (errorCode == ODRIVE_ERROR_BRAKE_DUTY_CYCLE_NAN)          tft.println("ERROR_BRAKE_DUTY_CYCLE_NAN");
    else if (errorCode == ODRIVE_ERROR_INVALID_BRAKE_RESISTANCE)      tft.println("ERROR_INVALID_BRAKE_RESISTANCE");
    else if (errorCode == 88)                                         tft.println("NOT_IMPLEMENTED");
    else                                                              tft.println("ERROR_UNKNOWN");
    return;
  }

  if (axis != -1 && comp == AXIS) {
    if      (errorCode == AXIS_ERROR_NONE)                            tft.println("AX_ERROR_NONE");
    else if (errorCode == AXIS_ERROR_INVALID_STATE)                   tft.println("AX_ERROR_INVALID_STATE");
    else if (errorCode == AXIS_ERROR_WATCHDOG_TIMER_EXPIRED)          tft.println("AX_ERROR_WATCHDOG_TIMER_EXPIRED");
    else if (errorCode == AXIS_ERROR_MIN_ENDSTOP_PRESSED)             tft.println("AX_ERROR_MIN_ENDSTOP_PRESSED");
    else if (errorCode == AXIS_ERROR_MAX_ENDSTOP_PRESSED)             tft.println("AX_ERROR_MAX_ENDSTOP_PRESSED");
    else if (errorCode == AXIS_ERROR_ESTOP_REQUESTED)                 tft.println("AX_ERROR_ESTOP_REQUESTED");
    else if (errorCode == AXIS_ERROR_HOMING_WITHOUT_ENDSTOP)          tft.println("AX_ERROR_HOMING_WITHOUT_ENDSTOP");
    else if (errorCode == AXIS_ERROR_OVER_TEMP)                       tft.println("AX_ERROR_OVER_TEMP");
    else if (errorCode == AXIS_ERROR_UNKNOWN_POSITION)                tft.println("AX_ERROR_UNKNOWN_POSITION");
    else                                                              tft.println("AX_ERROR_UNKNOWN");
    return;
  }

  if (axis != -1 && comp == MOTOR) {
    if      (errorCode == MOTOR_ERROR_NONE)                           tft.println("M_ERR_NONE");
    else if (errorCode == MOTOR_ERROR_PHASE_RESISTANCE_OUT_OF_RANGE)  tft.println("M_ERR_PHASE_RESISTANCE_OUT_OF_RANGE");
    else if (errorCode == MOTOR_ERROR_PHASE_INDUCTANCE_OUT_OF_RANGE)  tft.println("M_ERR_PHASE_INDUCTANCE_OUT_OF_RANGE");
    else if (errorCode == MOTOR_ERROR_DRV_FAULT)                      tft.println("M_ERR_DRV_FAULT");
    else if (errorCode == MOTOR_ERROR_CONTROL_DEADLINE_MISSED)        tft.println("M_ERR_CONTROL_DEADLINE_MISSED");
    else if (errorCode == MOTOR_ERROR_MODULATION_MAGNITUDE)           tft.println("M_ERR_MODULATION_MAGNITUDE");
    else if (errorCode == MOTOR_ERROR_CURRENT_SENSE_SATURATION)       tft.println("M_ERR_CURRENT_SENSE_SATURATION");
    else if (errorCode == MOTOR_ERROR_CURRENT_LIMIT_VIOLATION)        tft.println("M_ERR_CURRENT_LIMIT_VIOLATION");
    else if (errorCode == MOTOR_ERROR_MODULATION_IS_NAN)              tft.println("M_ERR_MODULATION_IS_NAN");
    else if (errorCode == MOTOR_ERROR_MOTOR_THERMISTOR_OVER_TEMP)     tft.println("M_ERR_MOTOR_THERMISTOR_OVER_TEMP");
    else if (errorCode == MOTOR_ERROR_FET_THERMISTOR_OVER_TEMP)       tft.println("M_ERR_FET_THERMISTOR_OVER_TEMP");
    else if (errorCode == MOTOR_ERROR_TIMER_UPDATE_MISSED)            tft.println("M_ERR_TIMER_UPDATE_MISSED");
    else if (errorCode == MOTOR_ERROR_CURRENT_MEASUREMENT_UNAVAILABLE) tft.println("M_ERR_CURRENT_MEASUREMENT_UNAVAIL");
    else if (errorCode == MOTOR_ERROR_CONTROLLER_FAILED)              tft.println("M_ERR_CONTROLLER_FAILED");
    else if (errorCode == MOTOR_ERROR_I_BUS_OUT_OF_RANGE)             tft.println("M_ERR_I_BUS_OUT_OF_RANGE");
    else if (errorCode == MOTOR_ERROR_BRAKE_RESISTOR_DISARMED)        tft.println("M_ERR_BRAKE_RESISTOR_DISARMED"); 
    else if (errorCode == MOTOR_ERROR_SYSTEM_LEVEL)                   tft.println("M_ERR_SYSTEM_LEVEL");
    else if (errorCode == MOTOR_ERROR_BAD_TIMING)                     tft.println("M_ERR_BAD_TIMING");
    else if (errorCode == MOTOR_ERROR_UNKNOWN_PHASE_ESTIMATE)         tft.println("M_ERR_UNKNOWN_PHASE_ESTIMATE");
    else if (errorCode == MOTOR_ERROR_UNKNOWN_PHASE_VEL)              tft.println("M_ERR_UNKNOWN_PHASE_VEL");
    else if (errorCode == MOTOR_ERROR_UNKNOWN_TORQUE)                 tft.println("M_ERR_UNKNOWN_TORQUE");  
    else if (errorCode == MOTOR_ERROR_UNKNOWN_CURRENT_COMMAND)        tft.println("M_ERR_UNKNOWN_CURRENT_COMMAND");
    else if (errorCode == MOTOR_ERROR_UNKNOWN_CURRENT_MEASUREMENT)    tft.println("M_ERR_UNKNOWN_CURRENT_MEASUREMENT");
    else if (errorCode == MOTOR_ERROR_UNKNOWN_VBUS_VOLTAGE)           tft.println("M_ERR_UNKNOWN_VBUS_VOLTAGE");
    else if (errorCode == MOTOR_ERROR_UNKNOWN_VOLTAGE_COMMAND)        tft.println("M_ERR_UNKNOWN_VOLTAGE_COMMAND"); 
    else if (errorCode == MOTOR_ERROR_UNKNOWN_GAINS)                  tft.println("M_ERR_UNKNOWN_GAINS");  
    else if (errorCode == MOTOR_ERROR_CONTROLLER_INITIALIZING)        tft.println("M_ERR_CONTROLLER_INITIALIZING"); 
    else if (errorCode == MOTOR_ERROR_UNBALANCED_PHASES)              tft.println("M_ERR_UNBALANCED_PHASES"); 
    else                                                              tft.println("M_ERR_UNKNOWN");
    return;
  }

  if (axis != -1 && comp == CONTROLLER) {
   #if ODRIVE_COMM_MODE == OD_UART
    if      (errorCode == CONTROLLER_ERROR_NONE)                      tft.println("C_ERR_NONE");
    else if (errorCode == CONTROLLER_ERROR_OVERSPEED)                 tft.println("C_ERR_OVERSPEED");
    else if (errorCode == CONTROLLER_ERROR_INVALID_INPUT_MODE)        tft.println("C_ERR_INVALID_INPUT_MODE");
    else if (errorCode == CONTROLLER_ERROR_UNSTABLE_GAIN)             tft.println("C_ERR_UNSTABLE_GAIN"); 
    else if (errorCode == CONTROLLER_ERROR_INVALID_MIRROR_AXIS)       tft.println("C_ERR_INVALID_MIRROR_AXIS"); 
    else if (errorCode == CONTROLLER_ERROR_INVALID_LOAD_ENCODER)      tft.println("C_ERR_INVALID_LOAD_ENCODER"); 
    else if (errorCode == CONTROLLER_ERROR_INVALID_ESTIMATE)          tft.println("C_ERR_INVALID_ESTIMATE");
    else if (errorCode == CONTROLLER_ERROR_INVALID_CIRCULAR_RANGE)    tft.println("C_ERR_INVALID_CIRCULAR_RANGE"); 
    else if (errorCode == CONTROLLER_ERROR_SPINOUT_DETECTED)          tft.println("C_ERR_SPINOUT_DETECTED"); 
    else                                                              tft.println("C_ERR_UNKNOWN"); 
    return;
   #elif ODRIVE_COMM_MODE == OD_CAN // then the code is are flags and not errors
    if (errorCode == 128) tft.println("Controller Trajectory Done"); return;
   #endif
  }

  if (axis != -1 && comp == ENCODER) {
    if      (errorCode == ENCODER_ERROR_NONE)                         tft.println("E_ERR_NONE"); 
    else if (errorCode == ENCODER_ERROR_UNSTABLE_GAIN)                tft.println("E_ERR_UNSTABLE_GAIN");
    else if (errorCode == ENCODER_ERROR_CPR_POLEPAIRS_MISMATCH)       tft.println("E_ERR_CPR_POLEPAIRS_MISMATCH"); 
    else if (errorCode == ENCODER_ERROR_NO_RESPONSE)                  tft.println("E_ERR_NO_RESPONSE");  
    else if (errorCode == ENCODER_ERROR_UNSUPPORTED_ENCODER_MODE)     tft.println("E_ERR_UNSUPPORTED_ENCODER_MODE");  
    else if (errorCode == ENCODER_ERROR_ILLEGAL_HALL_STATE)           tft.println("E_ERR_ILLEGAL_HALL_STATE");
    else if (errorCode == ENCODER_ERROR_INDEX_NOT_FOUND_YET)          tft.println("E_ERR_INDEX_NOT_FOUND_YET"); 
    else if (errorCode == ENCODER_ERROR_ABS_SPI_TIMEOUT)              tft.println("E_ERR_ABS_SPI_TIMEOUT");
    else if (errorCode == ENCODER_ERROR_ABS_SPI_COM_FAIL)             tft.println("E_ERR_ABS_SPI_COM_FAIL");
    else if (errorCode == ENCODER_ERROR_ABS_SPI_NOT_READY)            tft.println("E_ERR_ABS_SPI_NOT_READY");
    else if (errorCode == ENCODER_ERROR_HALL_NOT_CALIBRATED_YET)      tft.println("E_ERR_HALL_NOT_CALIBRATED_YET"); 
    else                                                              tft.println("E_ERR_UNKNOWN");
    return;
  }
}

// create enum Component operator ++
Component& operator ++ (Component& comp) {
  comp = Component(static_cast<std::underlying_type<Component>::type>(comp) + 1);
  return comp;
}

// ======== Show the ODRIVE errors ========
void ODriveScreen::showODriveErrors() {
  int y_offset = 0;
  tft.setFont();
  tft.fillRect(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y, 200, 12*OD_ERR_SPACING, pgBackground);
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y); 
  uint32_t err = 0;
  //char tempString[40] ="";

  // ODrive top level errors
  tft.print("-------Top Level Errors-------");
  err = oDriveExt.getODriveErrors(-1, Component::NO_COMP);
  //sprintf(tempString, "Top Level err=%08lX", err); VL(tempString);
  y_offset += OD_ERR_SPACING;
  
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  oDriveScreen.decodeODriveErrors(-1, Component::NO_COMP, err);
  
  // enum ordering: AXIS=2, CONTROLLER=3, MOTOR=4, ENCODER=5
  // AZM Errors
  y_offset += OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  tft.print("----------AZM Errors----------");

  for (Component comp = AXIS; comp != COMP_LAST; ++comp) {
    y_offset +=OD_ERR_SPACING;
    tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
    err = oDriveExt.getODriveErrors(AZM_MOTOR, comp);
    //sprintf(tempString, "AZM comp=%c err=%08lX", comp+48, err); VL(tempString);
    oDriveScreen.decodeODriveErrors(AZM_MOTOR, comp, err);
  }

  // ALT Errors
  y_offset += OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  tft.print("----------ALT Errors----------");

  for (Component comp = AXIS; comp != COMP_LAST; ++comp) {
    y_offset +=OD_ERR_SPACING;
    tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
    err = oDriveExt.getODriveErrors(ALT_MOTOR, comp);
    //sprintf(tempString, "ALT comp=%c err=%08lX", comp+48, err); VL(tempString);
    decodeODriveErrors(ALT_MOTOR, comp, err);
  }
}

bool ODriveScreen::odriveButStateChange() {
  if (preAzmState != axis1.isEnabled()) {
    preAzmState = axis1.isEnabled(); 
    return true; 
  } else if (preAltState != axis2.isEnabled()) {
    preAltState = axis2.isEnabled(); 
    return true; 
  } else if (display._redrawBut) {
    display._redrawBut = false;
    return true;
  } else {
    return false;
  }
}

// ========  Update ODrive Page Buttons ========
void ODriveScreen::updateOdriveButtons(bool redrawBut) {
   // redrawBut when true forces a refresh of all buttons once more..used for a toggle effect on some buttons
  
  _redrawBut = redrawBut;
  tft.setFont(&Inconsolata_Bold8pt7b);

  int x_offset = 0;
  int y_offset = 0;
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;

 // ----- Column 1 -----
  // Enable / Disable Azimuth Motor
  if (axis1.isEnabled()) {
    odriveButton.draw(OD_ACT_COL_1_X, OD_ACT_COL_1_Y + y_offset, "AZM Enabled", BUT_ON);
  } else { //motor off
    odriveButton.draw(OD_ACT_COL_1_X, OD_ACT_COL_1_Y + y_offset, "EN AZM", BUT_OFF);
  }

  // Enable / Disable Altitude Motor
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  if (axis2.isEnabled()) {
    odriveButton.draw(OD_ACT_COL_1_X, OD_ACT_COL_1_Y + y_offset, "ALT Enabled", BUT_ON);
  } else { //motor off
    odriveButton.draw(OD_ACT_COL_1_X, OD_ACT_COL_1_Y + y_offset, "EN ALT", BUT_OFF);
  }

// ----- Column 2 -----
  // Stop all movement  
 y_offset = 0;
 y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  if (OdStopButton) {
    odriveButton.draw(OD_ACT_COL_2_X, OD_ACT_COL_2_Y + y_offset, "All Stop'd", BUT_ON);
    OdStopButton = false;
  } else { 
    odriveButton.draw(OD_ACT_COL_2_X, OD_ACT_COL_2_Y + y_offset, "STOP!", BUT_OFF);
  }

  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Clear Errors
  if (!clearODriveErr) {
    odriveButton.draw(OD_ACT_COL_2_X, OD_ACT_COL_2_Y + y_offset, "Clr Errors", BUT_OFF);
  } else {
    odriveButton.draw(OD_ACT_COL_2_X, OD_ACT_COL_2_Y + y_offset, "Errs Cleared", BUT_ON);
    clearODriveErr = false;
  }

  // ----- 3rd Column -----
  y_offset = -160;
  
  // AZ Gains High
  if (!oDriveExt.AZgainHigh) {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, "AZ Gain Hi", BUT_OFF);
  } else {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, "AZ Gain Hi", BUT_ON);
  }

  // AZ Gains Default
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (!oDriveExt.AZgainDefault) {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj,"AZ Gain Def", BUT_OFF);
  } else {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, "AZ Gain Def", BUT_ON);
  }

  // ALT Velocity Gain High
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (!oDriveExt.ALTgainHigh) {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, "ALT Gain Hi", BUT_OFF);
  } else {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, "ALT Gain Hi", BUT_ON);
  }

  // ALT Velocity Gain Default
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (!oDriveExt.ALTgainDefault) {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, "ALT Gain Def", BUT_OFF);
  } else {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, "ALT Gain Def", BUT_ON);
  }

  //----------------------------------------
  y_offset = 0;
  // Demo Button
  if (!demoActive) {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, "Demo ODrive", BUT_OFF);
  } else {
    odriveButton.draw(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, "Demo Active", BUT_ON);
  }

  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Reset ODrive Button
  if (!resetODriveFlag) {
    odriveButton.draw(OD_ACT_COL_3_X, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, "Rst ODrive", BUT_OFF);
  } else {
    odriveButton.draw(OD_ACT_COL_3_X, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, "Resetting", BUT_ON);
    resetODriveFlag = false;
  }

  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Enable or Disable the ODrive position update via SERIAL
  if (ODpositionUpdateEnabled) {
    odriveButton.draw(OD_ACT_COL_3_X, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, "Updates On", BUT_ON);
  } else {
    odriveButton.draw(OD_ACT_COL_3_X, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, "UpDates Off", BUT_OFF);
  }
}

// =========== ODrive button update ===========
bool ODriveScreen::touchPoll(uint16_t px, uint16_t py) {
  int x_offset = 0;
  int y_offset = 0;
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;

  // ===== Column 1 - Leftmost ======
  // Enable Azimuth motor
  if (px > OD_ACT_COL_1_X + x_offset && px < OD_ACT_COL_1_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_1_Y + y_offset && py <  OD_ACT_COL_1_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    BEEP;
    if (!axis1.isEnabled()) { // if not On, toggle ON
      digitalWrite(AZ_ENABLED_LED_PIN, LOW); // Turn On AZ LED
      motor1.power(true);
    } else { // since already ON, toggle OFF
      digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZ LED
      motor1.power(false);
    }
    return true;
  }
            
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Enable Altitude motor
  if (px > OD_ACT_COL_1_X + x_offset && px < OD_ACT_COL_1_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_1_Y + y_offset && py <  OD_ACT_COL_1_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    BEEP;
    if (axis2.isEnabled()) { // toggle ON
      digitalWrite(ALT_ENABLED_LED_PIN, LOW); // Turn On ALT LED
      motor2.power(true);
    } else { // toggle OFF
      digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn off ALT LED
      motor2.power(false); // turn off ODrive motor
    }
    return true;
  }

  // ----- Column 2 -----
  // STOP everthing requested
  y_offset = 0;
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  if (px > OD_ACT_COL_2_X + x_offset && px < OD_ACT_COL_2_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_2_Y + y_offset && py <  OD_ACT_COL_2_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    BEEP;
    if (!OdStopButton) {
      setLocalCmd(":Q#"); // stops move
      motor1.power(false); // turn off the motors
      axis1.enable(false);
      motor2.power(false);
      axis2.enable(false);
      OdStopButton = true;
      digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZ LED
      digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn Off ALT LED
    }
    return true;
  }
  
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Clear ODrive Errors
  if (px > OD_ACT_COL_2_X + x_offset && px < OD_ACT_COL_2_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_2_Y + y_offset && py <  OD_ACT_COL_2_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    BEEP;
    VLF("MSG: Clearing ODrive Errors");
    oDriveExt.clearAllODriveErrors();
    return true;
  }

  // Column 3
  y_offset = -165;
  // AZ Gain HIGH
  if (px > OD_ACT_COL_3_X + x_offset && px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_3_Y + y_offset && py <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y-box_height_adj) {
    BEEP;
    oDriveExt.AZgainHigh = true;
    oDriveExt.AZgainDefault = false;
    oDriveExt.setODriveVelGains(AZM_MOTOR, AZM_VEL_GAIN_HI, AZM_VEL_INT_GAIN_HI); // Set Velocity Gain
    delay(1);
    return true;
  }

    // AZ Gain DEFault
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (px > OD_ACT_COL_3_X + x_offset && px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_3_Y + y_offset && py <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y-box_height_adj) {
    BEEP;
    oDriveExt.AZgainHigh = false;
    oDriveExt.AZgainDefault = true;
    oDriveExt.setODriveVelGains(AZM_MOTOR, AZM_VEL_GAIN_DEF, AZM_VEL_INT_GAIN_DEF);
    delay(1);
    return true;
  }

    // ALT Gain HIGH
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (px > OD_ACT_COL_3_X + x_offset && px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_3_Y + y_offset && py <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y-box_height_adj) {
    BEEP;
    oDriveExt.ALTgainHigh = true;
    oDriveExt.ALTgainDefault = false;
    oDriveExt.setODriveVelGains(ALT_MOTOR, ALT_VEL_GAIN_HI, ALT_VEL_INT_GAIN_HI); // Set Velocity Gain, Integrator gain
    delay(1);
    return true;
  }

    // ALT Gain DEFault
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (px > OD_ACT_COL_3_X + x_offset && px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_3_Y + y_offset && py <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y-box_height_adj) {
    BEEP;
    oDriveExt.ALTgainHigh = false;
    oDriveExt.ALTgainDefault = true;
    oDriveExt.setODriveVelGains(ALT_MOTOR, ALT_VEL_GAIN_DEF, ALT_VEL_INT_GAIN_DEF);
    delay(1);
    return true;
  }

  y_offset = 0;
  // Demo Mode for ODrive
  // Toggle on Demo Mode if button pressed, toggle off if pressed and already on
  // Demo Mode bypasses OnStep control to repeat a sequence of moves
  if (px > OD_ACT_COL_3_X + x_offset && px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_3_Y + y_offset && py <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    BEEP;
    if (!demoActive) {
      setLocalCmd(":Q#"); // does not turn off Motor power
      demoActive = true;
      
      // Start demo task
      VF("MSG: Setup, Demo Mode (rate 10 sec priority 6)... ");
      uint8_t demoHandle = tasks.add(10000, 0, true, 6, demoWrapper, "Demo");
      if (demoHandle) { VLF("success"); } else { VLF("FAILED!"); }
    } else {
      demoActive = false;
      VLF("MSG: Demo OFF ODrive");
      tasks.setDurationComplete(demoHandle);
    }
    return true;
  }

  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Reset ODRIVE
  if (px > OD_ACT_COL_3_X + x_offset && px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_3_Y + y_offset && py <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    BEEP;
    VLF("MSG: Resetting ODrive");
    delay(5);
    digitalWrite(ODRIVE_RST, LOW);
    delay(2);
    digitalWrite(ODRIVE_RST, HIGH);
    delay(1000);
    //oDriveScreen.draw();
    //ODRIVE_SERIAL.flush();
    //oDriveExt.clearAllODriveErrors();
    resetODriveFlag = true;
    oDriveExt.AZgainHigh = false;
    oDriveExt.AZgainDefault = true;
    oDriveExt.ALTgainHigh = false;
    oDriveExt.ALTgainDefault = true;
    return true;
  }

  // Disable / Enable ODrive motor position update - SERIAL between ODrive and Teensy
  // Disable-position-updates so that they don't override the ODrive
  // motor positions while tuning ODrive with ODrive USB channel
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  if (px > OD_ACT_COL_3_X + x_offset && px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_3_Y + y_offset && py <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    BEEP;
    if (ODpositionUpdateEnabled) { // on, then toggle off
      axis1.enable(false);
      axis2.enable(false);
      ODpositionUpdateEnabled = false;
    } else { // off, then toggle on
      axis1.enable(true);
      axis2.enable(true);
      ODpositionUpdateEnabled = true;
    }
    return false;
  }  
  return true;
}    

ODriveScreen oDriveScreen;