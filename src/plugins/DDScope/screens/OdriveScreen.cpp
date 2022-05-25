// =====================================================
// OdriveScreen.cpp
// Author: Richard Benear 2022

#include <ODriveArduino.h> // https://github.com/odriverobotics/ODrive/tree/master/Arduino/ODriveArduino
#include <ODriveEnums.h>
#include "ODriveScreen.h"
#include "../display/Display.h"
#include "../odriveExt/OdriveExt.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "../../../telescope/mount/Mount.h"

#define OD_ERR_OFFSET_X           4 
#define OD_ERR_OFFSET_Y         190 
#define OD_ERR_SPACING           16 
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
#define OD_ACT_Y_SPACING         4
#define OD_ACT_TEXT_X_OFFSET     10
#define OD_ACT_TEXT_Y_OFFSET     20

void demoWrapper() { oDriveExt.demoMode(true); }

//****** Draw Odrive Page ******
void ODriveScreen::draw() {
  display.updateColors();
  tft.setTextColor(display.textColor);
  tft.fillScreen(display.pgBackground);
  display.currentScreen = ODRIVE_SCREEN;
  display.drawMenuButtons();
  display.drawTitle(105, 30, "ODrive");
  display.drawCommonStatusLabels();
  display.updateOnStepCmdStatus();

  ODRIVE_SERIAL << "r hw_version_major\n"; 
  uint8_t hwMajor = oDriveArduino.readInt();
  ODRIVE_SERIAL << "r hw_version_minor\n"; 
  uint8_t hwMinor = oDriveArduino.readInt();
  ODRIVE_SERIAL << "r hw_version_variant\n"; 
  uint8_t hwVar = oDriveArduino.readInt();
  ODRIVE_SERIAL << "r fw_version_major\n"; 
  uint8_t fwMajor = oDriveArduino.readInt();
  ODRIVE_SERIAL << "r fw_version_minor\n"; 
  uint8_t fwMinor = oDriveArduino.readInt();
  ODRIVE_SERIAL << "r fw_version_revision\n"; 
  uint8_t fwRev = oDriveArduino.readInt();

  tft.setCursor(12, 165);
  tft.print("*HW Version:"); tft.print(hwMajor); tft.print("."); tft.print(hwMinor); tft.print("."); tft.print(hwVar);
  tft.setCursor(12, 177);
  tft.print("*FW Version:"); tft.print(fwMajor); tft.print("."); tft.print(fwMinor); tft.print("."); tft.print(fwRev);

  demoActive = false;
}

// ===== Decode all Odrive Errors =====
void ODriveScreen::decodeOdriveError(uint32_t errorCode) {
  if      (errorCode == ODRIVE_ERROR_NONE)                          tft.println("ODRIVE_ERROR_NONE");
  else if (errorCode == ODRIVE_ERROR_CONTROL_ITERATION_MISSED)      tft.println("ODRIVE_ERROR_CONTROL_ITERATION_MISSED");
  else if (errorCode == ODRIVE_ERROR_DC_BUS_UNDER_VOLTAGE)          tft.println("ODRIVE_ERROR_DC_BUS_UNDER_VOLTAGE");
  else if (errorCode == ODRIVE_ERROR_DC_BUS_OVER_VOLTAGE)           tft.println("ODRIVE_ERROR_DC_BUS_OVER_VOLTAGE");
  else if (errorCode == ODRIVE_ERROR_DC_BUS_OVER_REGEN_CURRENT)     tft.println("ODRIVE_ERROR_DC_BUS_OVER_REGEN_CURRENT");
  else if (errorCode == ODRIVE_ERROR_DC_BUS_OVER_CURRENT)           tft.println("ODRIVE_ERROR_DC_BUS_OVER_CURRENT");
  else if (errorCode == ODRIVE_ERROR_BRAKE_DEADTIME_VIOLATION)      tft.println("ODRIVE_ERROR_BRAKE_DEADTIME_VIOLATION");
  else if (errorCode == ODRIVE_ERROR_BRAKE_DUTY_CYCLE_NAN)          tft.println("ODRIVE_ERROR_BRAKE_DUTY_CYCLE_NAN");
  else if (errorCode == ODRIVE_ERROR_INVALID_BRAKE_RESISTANCE)      tft.println("ODRIVE_ERROR_INVALID_BRAKE_RESISTANCE");
}

void ODriveScreen::decodeAxisError(int axis, uint32_t errorCode) {
  if      (errorCode == AXIS_ERROR_NONE)                            tft.println("AXIS_ERROR_NONE");
  else if (errorCode == AXIS_ERROR_INVALID_STATE)                   tft.println("AXIS_ERROR_INVALID_STATE");
  else if (errorCode == AXIS_ERROR_WATCHDOG_TIMER_EXPIRED)          tft.println("AXIS_ERROR_WATCHDOG_TIMER_EXPIRED");
  else if (errorCode == AXIS_ERROR_MIN_ENDSTOP_PRESSED)             tft.println("AXIS_ERROR_MIN_ENDSTOP_PRESSED");
  else if (errorCode == AXIS_ERROR_MAX_ENDSTOP_PRESSED)             tft.println("AXIS_ERROR_MAX_ENDSTOP_PRESSED");
  else if (errorCode == AXIS_ERROR_ESTOP_REQUESTED)                 tft.println("AXIS_ERROR_ESTOP_REQUESTED");
  else if (errorCode == AXIS_ERROR_HOMING_WITHOUT_ENDSTOP)          tft.println("AXIS_ERROR_HOMING_WITHOUT_ENDSTOP");
  else if (errorCode == AXIS_ERROR_OVER_TEMP)                       tft.println("AXIS_ERROR_OVER_TEMP");
  else if (errorCode == AXIS_ERROR_UNKNOWN_POSITION)                tft.println("AXIS_ERROR_UNKNOWN_POSITION");
}

void ODriveScreen::decodeMotorError(int axis, uint32_t errorCode) { 
  if      (errorCode == MOTOR_ERROR_NONE)                           tft.println("MOTOR_ERROR_NONE");
  else if (errorCode == MOTOR_ERROR_PHASE_RESISTANCE_OUT_OF_RANGE)  tft.println("MOTOR_ERROR_PHASE_RESISTANCE_OUT_OF_RANGE");
  else if (errorCode == MOTOR_ERROR_PHASE_INDUCTANCE_OUT_OF_RANGE)  tft.println("MOTOR_ERROR_PHASE_INDUCTANCE_OUT_OF_RANGE");
  else if (errorCode == MOTOR_ERROR_DRV_FAULT)                      tft.println("MOTOR_ERROR_DRV_FAULT");
  else if (errorCode == MOTOR_ERROR_CONTROL_DEADLINE_MISSED)        tft.println("MOTOR_ERROR_CONTROL_DEADLINE_MISSED");
  else if (errorCode == MOTOR_ERROR_MODULATION_MAGNITUDE)           tft.println("MOTOR_ERROR_MODULATION_MAGNITUDE");
  else if (errorCode == MOTOR_ERROR_CURRENT_SENSE_SATURATION)       tft.println("MOTOR_ERROR_CURRENT_SENSE_SATURATION");
  else if (errorCode == MOTOR_ERROR_CURRENT_LIMIT_VIOLATION)        tft.println("MOTOR_ERROR_CURRENT_LIMIT_VIOLATION");
  else if (errorCode == MOTOR_ERROR_MODULATION_IS_NAN)              tft.println("MOTOR_ERROR_MODULATION_IS_NAN");
  else if (errorCode == MOTOR_ERROR_MOTOR_THERMISTOR_OVER_TEMP)     tft.println("MOTOR_ERROR_MOTOR_THERMISTOR_OVER_TEMP");
  else if (errorCode == MOTOR_ERROR_FET_THERMISTOR_OVER_TEMP)       tft.println("MOTOR_ERROR_FET_THERMISTOR_OVER_TEMP");
  else if (errorCode == MOTOR_ERROR_TIMER_UPDATE_MISSED)            tft.println("MOTOR_ERROR_TIMER_UPDATE_MISSED");
  else if (errorCode == MOTOR_ERROR_CURRENT_MEASUREMENT_UNAVAILABLE) tft.println("MOTOR_ERROR_CURRENT_MEASUREMENT_UNAVAIL");
  else if (errorCode == MOTOR_ERROR_CONTROLLER_FAILED)              tft.println("MOTOR_ERROR_CONTROLLER_FAILED");
  else if (errorCode == MOTOR_ERROR_I_BUS_OUT_OF_RANGE)             tft.println("MOTOR_ERROR_I_BUS_OUT_OF_RANGE");
  else if (errorCode == MOTOR_ERROR_BRAKE_RESISTOR_DISARMED)        tft.println("MOTOR_ERROR_BRAKE_RESISTOR_DISARMED"); 
  else if (errorCode == MOTOR_ERROR_SYSTEM_LEVEL)                   tft.println("MOTOR_ERROR_SYSTEM_LEVEL");
  else if (errorCode == MOTOR_ERROR_BAD_TIMING)                     tft.println("MOTOR_ERROR_BAD_TIMING");
  else if (errorCode == MOTOR_ERROR_UNKNOWN_PHASE_ESTIMATE)         tft.println("MOTOR_ERROR_UNKNOWN_PHASE_ESTIMATE");
  else if (errorCode == MOTOR_ERROR_UNKNOWN_PHASE_VEL)              tft.println("MOTOR_ERROR_UNKNOWN_PHASE_VEL");
  else if (errorCode == MOTOR_ERROR_UNKNOWN_TORQUE)                 tft.println("MOTOR_ERROR_UNKNOWN_TORQUE");  
  else if (errorCode == MOTOR_ERROR_UNKNOWN_CURRENT_COMMAND)        tft.println("MOTOR_ERROR_UNKNOWN_CURRENT_COMMAND");
  else if (errorCode == MOTOR_ERROR_UNKNOWN_CURRENT_MEASUREMENT)    tft.println("MOTOR_ERROR_UNKNOWN_CURRENT_MEASUREMENT");
  else if (errorCode == MOTOR_ERROR_UNKNOWN_VBUS_VOLTAGE)           tft.println("MOTOR_ERROR_UNKNOWN_VBUS_VOLTAGE");
  else if (errorCode == MOTOR_ERROR_UNKNOWN_VOLTAGE_COMMAND)        tft.println("MOTOR_ERROR_UNKNOWN_VOLTAGE_COMMAND"); 
  else if (errorCode == MOTOR_ERROR_UNKNOWN_GAINS)                  tft.println("MOTOR_ERROR_UNKNOWN_GAINS");  
  else if (errorCode == MOTOR_ERROR_CONTROLLER_INITIALIZING)        tft.println("MOTOR_ERROR_CONTROLLER_INITIALIZING"); 
  else if (errorCode == MOTOR_ERROR_UNBALANCED_PHASES)              tft.println("MOTOR_ERROR_UNBALANCED_PHASES"); 
}

void ODriveScreen::decodeControllerError(int axis, uint32_t errorCode) {
  if      (errorCode == CONTROLLER_ERROR_NONE)                      tft.println("CONTROLLER_ERROR_NONE");
  else if (errorCode == CONTROLLER_ERROR_OVERSPEED)                 tft.println("CONTROLLER_ERROR_OVERSPEED");
  else if (errorCode == CONTROLLER_ERROR_INVALID_INPUT_MODE)        tft.println("CONTROLLER_ERROR_INVALID_INPUT_MODE");
  else if (errorCode == CONTROLLER_ERROR_UNSTABLE_GAIN)             tft.println("CONTROLLER_ERROR_UNSTABLE_GAIN"); 
  else if (errorCode == CONTROLLER_ERROR_INVALID_MIRROR_AXIS)       tft.println("CONTROLLER_ERROR_INVALID_MIRROR_AXIS"); 
  else if (errorCode == CONTROLLER_ERROR_INVALID_LOAD_ENCODER)      tft.println("CONTROLLER_ERROR_INVALID_LOAD_ENCODER"); 
  else if (errorCode == CONTROLLER_ERROR_INVALID_ESTIMATE)          tft.println("CONTROLLER_ERROR_INVALID_ESTIMATE");
  else if (errorCode == CONTROLLER_ERROR_INVALID_CIRCULAR_RANGE)    tft.println("CONTROLLER_ERROR_INVALID_CIRCULAR_RANGE"); 
  else if (errorCode == CONTROLLER_ERROR_SPINOUT_DETECTED)          tft.println("CONTROLLER_ERROR_SPINOUT_DETECTED");  
}

void ODriveScreen::decodeEncoderError(int axis, uint32_t errorCode) {
  if      (errorCode == ENCODER_ERROR_NONE)                         tft.println("ENCODER_ERROR_NONE"); 
  else if (errorCode == ENCODER_ERROR_UNSTABLE_GAIN)                tft.println("ENCODER_ERROR_UNSTABLE_GAIN");
  else if (errorCode == ENCODER_ERROR_CPR_POLEPAIRS_MISMATCH)       tft.println("ENCODER_ERROR_CPR_POLEPAIRS_MISMATCH"); 
  else if (errorCode == ENCODER_ERROR_NO_RESPONSE)                  tft.println("ENCODER_ERROR_NO_RESPONSE");  
  else if (errorCode == ENCODER_ERROR_UNSUPPORTED_ENCODER_MODE)     tft.println("ENCODER_ERROR_UNSUPPORTED_ENCODER_MODE");  
  else if (errorCode == ENCODER_ERROR_ILLEGAL_HALL_STATE)           tft.println("ENCODER_ERROR_ILLEGAL_HALL_STATE");
  else if (errorCode == ENCODER_ERROR_INDEX_NOT_FOUND_YET)          tft.println("ENCODER_ERROR_INDEX_NOT_FOUND_YET"); 
  else if (errorCode == ENCODER_ERROR_ABS_SPI_TIMEOUT)              tft.println("ENCODER_ERROR_ABS_SPI_TIMEOUT");
  else if (errorCode == ENCODER_ERROR_ABS_SPI_COM_FAIL)             tft.println("ENCODER_ERROR_ABS_SPI_COM_FAIL");
  else if (errorCode == ENCODER_ERROR_ABS_SPI_NOT_READY)            tft.println("ENCODER_ERROR_ABS_SPI_NOT_READY");
  else if (errorCode == ENCODER_ERROR_HALL_NOT_CALIBRATED_YET)      tft.println("ENCODER_ERROR_HALL_NOT_CALIBRATED_YET"); 
}

// ========  Update Odrive Page Status ========
void ODriveScreen::updateStatus() {
  unsigned int errorCode = 0;
  int y_offset = 0;
  
  display.updateCommonStatus();
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y); 

   // Odrive Errors Status
  ODRIVE_SERIAL << "r odrive.error\n";
  errorCode = oDriveArduino.readInt();
  if ((lastOdriveErr != errorCode) || display.firstDraw) { decodeOdriveError(errorCode); lastOdriveErr = errorCode; }

  // ALT Error
  y_offset +=OD_ERR_SPACING + 6;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  tft.print("----ALT ERRORS----");

  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  ODRIVE_SERIAL << "r odrive.axis"<<ALT_MOTOR<<".error\n";
  errorCode = oDriveArduino.readInt();
  if ((lastALTErr != errorCode) || display.firstDraw) { decodeAxisError(ALT, errorCode); lastALTErr = errorCode; }

  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  ODRIVE_SERIAL << "r odrive.axis"<<ALT_MOTOR<<".controller.error\n";
  errorCode = oDriveArduino.readInt();
  if ((lastALTCtrlErr != errorCode) || display.firstDraw) { decodeControllerError(ALT, errorCode); lastALTCtrlErr = errorCode; }

  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  ODRIVE_SERIAL << "r odrive.axis"<<ALT_MOTOR<<".motor.error\n";
  errorCode = oDriveArduino.readInt();
  if ((lastALTMotorErr != errorCode) || display.firstDraw) { decodeMotorError(ALT, errorCode); lastALTMotorErr = errorCode; }

  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  ODRIVE_SERIAL << "r odrive.axis"<<ALT_MOTOR<<".encoder.error\n";
  errorCode = oDriveArduino.readInt();
  if ((lastALTEncErr != errorCode) || display.firstDraw) { decodeEncoderError(ALT, errorCode); lastALTEncErr = errorCode; }

  // AZ Errors
  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  tft.println("----AZ ERRORS----");

  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  ODRIVE_SERIAL << "r odrive.axis"<<AZM_MOTOR<<".error\n";
  errorCode = oDriveArduino.readInt();
  if ((lastAZErr != errorCode) || display.firstDraw) { decodeAxisError(ALT, errorCode); lastALTErr = errorCode; }

  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  ODRIVE_SERIAL << "r odrive.axis"<<AZM_MOTOR<<".controller.error\n";
  errorCode = oDriveArduino.readInt();
  if ((lastAZCtrlErr != errorCode) || display.firstDraw) { decodeControllerError(ALT, errorCode); lastALTCtrlErr = errorCode; }

  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  ODRIVE_SERIAL << "r odrive.axis"<<AZM_MOTOR<<".motor.error\n";
  errorCode = oDriveArduino.readInt();
  if ((lastAZMotorErr != errorCode) || display.firstDraw) { decodeMotorError(0, errorCode); lastALTMotorErr = errorCode; }

  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  ODRIVE_SERIAL << "r odrive.axis"<<ALT_MOTOR<<".encoder.error\n";
  errorCode = oDriveArduino.readInt();
  if ((lastAZEncErr != errorCode) || display.firstDraw) { decodeEncoderError(0, errorCode); lastALTEncErr = errorCode; }


  // ***** Button label updates *****
  if (display.screenTouched || display.firstDraw || display.refreshScreen) { // reduce screen flicker
    display.refreshScreen = false;
    if (display.screenTouched) display.refreshScreen = true;

    int x_offset = 0;
    y_offset = 0;
    y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;

    // =========== Column 1 ===========
    if (oDriveExt.odriveAZOff) {
      display.drawButton(OD_ACT_COL_1_X + x_offset, OD_ACT_COL_1_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, false, OD_ACT_TEXT_X_OFFSET, OD_ACT_TEXT_Y_OFFSET, "  EN AZ   ");
    } else { //motor on
      display.drawButton(OD_ACT_COL_1_X + x_offset, OD_ACT_COL_1_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, true, OD_ACT_TEXT_X_OFFSET, OD_ACT_TEXT_Y_OFFSET,   "AZ Enabled");
    }

    y_offset += OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
    if (oDriveExt.odriveALTOff) {
      display.drawButton(OD_ACT_COL_1_X + x_offset, OD_ACT_COL_1_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, false, OD_ACT_TEXT_X_OFFSET, OD_ACT_TEXT_Y_OFFSET, "  EN ALT   ");
    } else { //motor on
      display.drawButton(OD_ACT_COL_1_X + x_offset, OD_ACT_COL_1_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, true, OD_ACT_TEXT_X_OFFSET, OD_ACT_TEXT_Y_OFFSET,   "ALT Enabled");
    }

    // Second Column
    y_offset = 0;
    y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
    if (OdStopButton) {
      display.drawButton(OD_ACT_COL_2_X + x_offset, OD_ACT_COL_2_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, true, OD_ACT_TEXT_X_OFFSET, OD_ACT_TEXT_Y_OFFSET,    "AllStopped");
      OdStopButton = false;
    } else { 
      display.drawButton(OD_ACT_COL_2_X + x_offset, OD_ACT_COL_2_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, false, OD_ACT_TEXT_X_OFFSET+5, OD_ACT_TEXT_Y_OFFSET, "  STOP!  ");
    }

    y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
    // Clear Errors
    if (!clearOdriveErr) {
      display.drawButton(OD_ACT_COL_2_X + x_offset, OD_ACT_COL_2_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, false, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "Clear Errors");
    } else {
      display.drawButton(OD_ACT_COL_2_X + x_offset, OD_ACT_COL_2_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, true, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "Errs Cleared");
      clearOdriveErr = false;
    }

    // 3rd Column
    y_offset = -165;
    
    // AZ Gains High
    if (!AZgainHigh) {
      display.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, false, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "AZ Gain Hi");
    } else {
      display.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, true, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "AZ Gain Hi");
    }

    // AZ Gains Default
    y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
    if (!AZgainDefault) {
      display.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, false, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "AZ Gain Def");
    } else {
      display.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, true, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "AZ Gain Def");
    }

    // ALT Velocity Gain High
    y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
    if (!ALTgainHigh) {
      display.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, false, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "ALT Gain Hi");
    } else {
      display.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, true, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "ALT Gain Hi");
    }

    // ALT Velocity Gain Default
    y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
    if (!ALTgainDefault) {
      display.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, false, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "ALT Gain Def");
    } else {
      display.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, true, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "ALT Gain Def");
    }

    // ====== Show the Gains ======
    // Show AZM Velocity Gain - AZ is motor 1
    tft.setFont();
    float temp = oDriveExt.getOdriveVelGain(AZM_MOTOR);
    tft.setCursor(210,280); tft.print("AZM Vel  Gain:");
    tft.fillRect(295,280, 39, 10, display.pgBackground); 
    tft.setCursor(295,280); tft.print(temp);
    
    // Show AZM Velocity Integrator Gain
    temp = oDriveExt.getOdriveVelIntGain(AZM_MOTOR);
    tft.setCursor(210,290); tft.print("AZM VelI Gain:");
    tft.fillRect(295,290, 39, 10, display.pgBackground); 
    tft.setCursor(295,290); tft.print(temp);

    // Show ALT Velocity Gain - ALT is motor 0
    temp = oDriveExt.getOdriveVelGain(ALT_MOTOR);
    tft.setCursor(210,300); tft.print("ALT Vel  Gain:");
    tft.fillRect(295,300, 39, 10, display.pgBackground); 
    tft.setCursor(295,300); tft.print(temp);

    // Show ALT Velocity Integrator Gain
    temp = oDriveExt.getOdriveVelIntGain(ALT_MOTOR);
    tft.setCursor(210,310); tft.print("ALT VelI Gain:");
    tft.fillRect(295,310, 39, 10, display.pgBackground); 
    tft.setCursor(295,310); tft.print(temp);
    tft.setFont(&Inconsolata_Bold8pt7b);
    // ==================================

    y_offset = 0;
    // Demo Button
    if (!demoActive) {
      display.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, false, OD_ACT_TEXT_X_OFFSET-4, OD_ACT_TEXT_Y_OFFSET, "Demo ODrive");
    } else {
      display.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, true, OD_ACT_TEXT_X_OFFSET-4, OD_ACT_TEXT_Y_OFFSET, "Demo Active");
    }

    y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
    // Reset Odrive Button
    if (!resetOdriveFlag) {
      display.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, false, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "Reset ODrive");
    } else {
      display.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, true, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "  Resetting ");
      resetOdriveFlag = false;
    }

    y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
    // Enable or Disable the OD position update via UART
    if (ODpositionUpdateEnabled) {
      display.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, false, OD_ACT_TEXT_X_OFFSET-2, OD_ACT_TEXT_Y_OFFSET, "Dis UpDates");
    } else {
      display.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, true, OD_ACT_TEXT_X_OFFSET-2, OD_ACT_TEXT_Y_OFFSET,   "Ena UpDates");
    }
  }
  display.screenTouched = false;
}

// =========== Odrive touchscreen update ===========
void ODriveScreen::touchPoll() {
  int x_offset = 0;
  int y_offset = 0;
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;

  // ===== Column 1 - Leftmost ======
  // Enable Azimuth motor
  if (p.x > OD_ACT_COL_1_X + x_offset && p.x < OD_ACT_COL_1_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_1_Y + y_offset && p.y <  OD_ACT_COL_1_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    status.sound.click();
    if (oDriveExt.odriveAZOff) { // toggle ON
      digitalWrite(AZ_ENABLED_LED_PIN, LOW); // Turn On AZ LED
      oDriveExt.odriveAZOff = false; // false = NOT off
      motor1.power(true);
    } else { // since already ON, toggle OFF
      digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZ LED
      oDriveExt.odriveAZOff = true;
      motor1.power(false);
    }
  }
            
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Enable Altitude motor
  if (p.x > OD_ACT_COL_1_X + x_offset && p.x < OD_ACT_COL_1_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_1_Y + y_offset && p.y <  OD_ACT_COL_1_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    status.sound.click();
    if (oDriveExt.odriveALTOff) { // toggle ON
      digitalWrite(ALT_ENABLED_LED_PIN, LOW); // Turn On ALT LED
      oDriveExt.odriveALTOff = false; // false = NOT off
      motor2.power(true);
    } else { // toggle OFF
      digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn off ALT LED
      oDriveExt.odriveALTOff = true;
      motor2.power(false); // Idle the Odrive motor
    }
  }

  // Column 2
  // STOP everthing requested
  y_offset = 0;
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  if (p.x > OD_ACT_COL_2_X + x_offset && p.x < OD_ACT_COL_2_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_2_Y + y_offset && p.y <  OD_ACT_COL_2_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    if (!OdStopButton) {
      status.sound.click();
      display.setLocalCmd(":Q#"); // stops move
      motor1.power(false); // turn off the motors
      motor2.power(false);
      OdStopButton = true;
      oDriveExt.odriveAZOff = true;
      oDriveExt.odriveALTOff = true;
      digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZ LED
      digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn Off ALT LED
    }
  }
  
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Clear ODrive Errors
  if (p.x > OD_ACT_COL_2_X + x_offset && p.x < OD_ACT_COL_2_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_2_Y + y_offset && p.y <  OD_ACT_COL_2_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    VLF("MSG: Clearing ODrive Errors");
    oDriveExt.clearOdriveErrors(AZM_MOTOR, ENCODER);
    oDriveExt.clearOdriveErrors(AZM_MOTOR, CONTROLLER);
    oDriveExt.clearOdriveErrors(AZM_MOTOR, MOTOR);
    oDriveExt.clearOdriveErrors(ALT_MOTOR, ENCODER);
    oDriveExt.clearOdriveErrors(ALT_MOTOR, CONTROLLER);
    oDriveExt.clearOdriveErrors(ALT_MOTOR, MOTOR);
    status.sound.click();
    clearOdriveErr = true;
  }

  // Column 3
  y_offset = -165;
  // AZ Gain HIGH
  if (p.x > OD_ACT_COL_3_X + x_offset && p.x < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_3_Y + y_offset && p.y <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y-box_height_adj) {
    status.sound.click();
    AZgainHigh = true;
    AZgainDefault = false;
    oDriveExt.setOdriveVelGain(AZM_MOTOR, 1.8); // Set Velocity Gain
    delay(10);
    //setOdriveVelIntGain(AZ, 2.3); // Set Velocity Integrator Gain
  }

    // AZ Gain DEFault
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (p.x > OD_ACT_COL_3_X + x_offset && p.x < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_3_Y + y_offset && p.y <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y-box_height_adj) {
    status.sound.click();
    AZgainHigh = false;
    AZgainDefault = true;
    oDriveExt.setOdriveVelGain(AZM_MOTOR, 1.5);
    delay(10);
    //setOdriveVelIntGain(AZ, 2.0);
  }

    // ALT Gain HIGH
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (p.x > OD_ACT_COL_3_X + x_offset && p.x < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_3_Y + y_offset && p.y <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y-box_height_adj) {
    status.sound.click();
    ALTgainHigh = true;
    ALTgainDefault = false;
    oDriveExt.setOdriveVelGain(ALT_MOTOR, 0.5); // Set Velocity Gain
    delay(10);
    //setOdriveVelIntGain(ALT, 0.7); // Set Velocity Integrator Gain
  }

    // ALT Gain DEFault
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (p.x > OD_ACT_COL_3_X + x_offset && p.x < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_3_Y + y_offset && p.y <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y-box_height_adj) {
    status.sound.click();
    ALTgainHigh = false;
    ALTgainDefault = true;
    oDriveExt.setOdriveVelGain(ALT, 0.3);
    delay(10);
    //setOdriveVelIntGain(ALT, 0.4);
  }

  y_offset = 0;
  // Demo Mode for ODrive
  // Toggle on Demo Mode if button pressed, toggle off if pressed and already on
  // Demo mode relies on a pseudo-thread that fires off the change in positions
  if (p.x > OD_ACT_COL_3_X + x_offset && p.x < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_3_Y + y_offset && p.y <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    if (!demoActive) {
      VLF("MSG: Demo ODrive");
      status.sound.click();
      demoActive = true;
      //demoHandle = tasks.add(10000, 0, true, 7, oDriveExt.demoMode(true), "Demo On");
    } else {
      demoActive = false;
      VLF("MSG: Demo OFF ODrive");
      //tasks.remove(demoHandle);
      oDriveExt.demoMode(false);
      status.sound.click();
      //tasks.setDurationComplete(tasks.getHandleByName("Demo On"));
    }
  }

  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Reset ODRIVE
  if (p.x > OD_ACT_COL_3_X + x_offset && p.x < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_3_Y + y_offset && p.y <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    VLF("MSG: Reseting ODrive");
    digitalWrite(ODRIVE_RST, LOW);
    delay(1);
    digitalWrite(ODRIVE_RST, HIGH);
    status.sound.click();
    resetOdriveFlag = true;
    AZgainHigh = false;
    AZgainDefault = true;
    ALTgainHigh = false;
    ALTgainDefault = true;
  }

  // Disable / Enable ODrive motor position update - UART between Odrive and Teensy
  // Disable-position-updates so that they don't override ODrive
  // motor positions while tuning ODrive with ODrive USB channel
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  if (p.x > OD_ACT_COL_3_X + x_offset && p.x < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_3_Y + y_offset && p.y <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    status.sound.click();
    if (ODpositionUpdateEnabled) {
      ODpositionUpdateEnabled = false;
    } else {
      ODpositionUpdateEnabled = true;
    }
  }  
}    

void ODriveScreen::updateOdriveErrors() {
  int x = 2;
  int y = 473;
  int label_x = 160;
  int data_x = 110;
  tft.setFont(&Inconsolata_Bold8pt7b);
 
// ODrive AZ and ALT CONTROLLER (only) Error Status
  if (display.firstDraw) {
    tft.setCursor(x, y);
    tft.print("AZ Ctrl err:");
    tft.setCursor(label_x, y);
    tft.print("ALT Ctrl err:");
    current_AZ_ODerr = oDriveExt.dumpOdriveErrors(AZM_MOTOR, CONTROLLER);
    current_ALT_ODerr = oDriveExt.dumpOdriveErrors(ALT_MOTOR, CONTROLLER);
    display.canvPrint(        data_x, y, 0, C_WIDTH-40, C_HEIGHT, current_AZ_ODerr);
    display.canvPrint(label_x+data_x, y, 0, C_WIDTH-40, C_HEIGHT, current_ALT_ODerr);
  }
  int AZ_ODerr = oDriveExt.dumpOdriveErrors(AZM_MOTOR, CONTROLLER);
  if (current_AZ_ODerr != AZ_ODerr) {
      display.canvPrint(        data_x, y, 0, C_WIDTH-40, C_HEIGHT, AZ_ODerr);
      current_AZ_ODerr = AZ_ODerr;
  }
  int ALT_ODerr = oDriveExt.dumpOdriveErrors(AZM_MOTOR, CONTROLLER);
  if (current_ALT_ODerr != ALT_ODerr) {
      display.canvPrint(label_x+data_x, y, 0, C_WIDTH-40, C_HEIGHT, ALT_ODerr);
      current_ALT_ODerr = ALT_ODerr;
  }
}

ODriveScreen odriveScreen;