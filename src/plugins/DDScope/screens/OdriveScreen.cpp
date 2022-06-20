// =====================================================
// ODriveScreen.cpp
// Author: Richard Benear 2022

#include <ODriveArduino.h> // https://github.com/odriverobotics/ODrive/tree/master/Arduino/ODriveArduino
#include <ODriveEnums.h>
#include "ODriveScreen.h"
#include "../odriveExt/ODriveExt.h"
#include "../fonts/Inconsolata_Bold8pt7b.h"
#include "../../../telescope/mount/Mount.h"
#include "src/lib/tasks/OnTask.h"
#include <HardwareSerial.h>

#define OD_ERR_OFFSET_X           4 
#define OD_ERR_OFFSET_Y         188
#define OD_ERR_SPACING           15 
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
#define OD_ACT_TEXT_X_OFFSET     10
#define OD_ACT_TEXT_Y_OFFSET     20

void demoWrapper() { oDriveExt.demoMode(true); }

ODriveVersion oDversion;

//****** Draw ODrive Screen ******
void ODriveScreen::draw() {
  setCurrentScreen(ODRIVE_SCREEN);
  setNightMode(getNightMode());
  tft.setTextColor(textColor);
  tft.fillScreen(pgBackground);
  
  drawMenuButtons();
  drawTitle(105, TITLE_TEXT_Y, "ODrive");
  tft.setFont(&Inconsolata_Bold8pt7b);

  drawCommonStatusLabels();
  updateOdriveButtons();
  showGains();
  showODriveErrors();

  oDriveExt.getODriveVersion(oDversion);

  tft.setCursor(42, 165);
  tft.print("*HW Version:"); tft.print(oDversion.hwMajor); tft.print("."); tft.print(oDversion.hwMinor); tft.print("."); tft.print(oDversion.hwVar);
  tft.setCursor(42, 177);
  tft.print("*FW Version:"); tft.print(oDversion.fwMajor); tft.print("."); tft.print(oDversion.fwMinor); tft.print("."); tft.print(oDversion.fwRev);

  demoActive = false;
}

// task update for this screen
void ODriveScreen::updateOdriveStatus() {
  updateCommonStatus();
}

// ====== Show the Gains ======
void ODriveScreen::showGains() {
  // Show AZM Velocity Gain - AZ is motor 1
  tft.setFont();
  float temp = oDriveExt.getODriveVelGain(AZM_MOTOR);
  tft.setCursor(210,280); tft.print("AZM Vel  Gain:");
  tft.fillRect(295,280, 39, 10, pgBackground); 
  tft.setCursor(295,280); tft.print(temp);
  
  // Show AZM Velocity Integrator Gain
  temp = oDriveExt.getODriveVelIntGain(AZM_MOTOR);
  tft.setCursor(210,290); tft.print("AZM VelI Gain:");
  tft.fillRect(295,290, 39, 10, pgBackground); 
  tft.setCursor(295,290); tft.print(temp);

  // Show ALT Velocity Gain - ALT is motor 0
  temp = oDriveExt.getODriveVelGain(ALT_MOTOR);
  tft.setCursor(210,300); tft.print("ALT Vel  Gain:");
  tft.fillRect(295,300, 39, 10, pgBackground); 
  tft.setCursor(295,300); tft.print(temp);

  // Show ALT Velocity Integrator Gain
  temp = oDriveExt.getODriveVelIntGain(ALT_MOTOR);
  tft.setCursor(210,310); tft.print("ALT VelI Gain:");
  tft.fillRect(295,310, 39, 10, pgBackground); 
  tft.setCursor(295,310); tft.print(temp);
  tft.setFont(&Inconsolata_Bold8pt7b);
}

// ===== Decode all ODrive Errors =====
void ODriveScreen::decodeODriveErrors(int axis, Component, uint32_t errorCode) {
  if (axis == -1) { //then ODrive top level decode
    if      (errorCode == ODRIVE_ERROR_NONE)                          tft.println("ODRIVE_ERROR_NONE");
    else if (errorCode == ODRIVE_ERROR_CONTROL_ITERATION_MISSED)      tft.println("ODRIVE_ERROR_CONTROL_ITERATION_MISSED");
    else if (errorCode == ODRIVE_ERROR_DC_BUS_UNDER_VOLTAGE)          tft.println("ODRIVE_ERROR_DC_BUS_UNDER_VOLTAGE");
    else if (errorCode == ODRIVE_ERROR_DC_BUS_OVER_VOLTAGE)           tft.println("ODRIVE_ERROR_DC_BUS_OVER_VOLTAGE");
    else if (errorCode == ODRIVE_ERROR_DC_BUS_OVER_REGEN_CURRENT)     tft.println("ODRIVE_ERROR_DC_BUS_OVER_REGEN_CURRENT");
    else if (errorCode == ODRIVE_ERROR_DC_BUS_OVER_CURRENT)           tft.println("ODRIVE_ERROR_DC_BUS_OVER_CURRENT");
    else if (errorCode == ODRIVE_ERROR_BRAKE_DEADTIME_VIOLATION)      tft.println("ODRIVE_ERROR_BRAKE_DEADTIME_VIOLATION");
    else if (errorCode == ODRIVE_ERROR_BRAKE_DUTY_CYCLE_NAN)          tft.println("ODRIVE_ERROR_BRAKE_DUTY_CYCLE_NAN");
    else if (errorCode == ODRIVE_ERROR_INVALID_BRAKE_RESISTANCE)      tft.println("ODRIVE_ERROR_INVALID_BRAKE_RESISTANCE");
    return;
  }

  if (axis != -1 && Component::AXIS) {
    if      (errorCode == AXIS_ERROR_NONE)                            tft.println("AXIS_ERROR_NONE");
    else if (errorCode == AXIS_ERROR_INVALID_STATE)                   tft.println("AXIS_ERROR_INVALID_STATE");
    else if (errorCode == AXIS_ERROR_WATCHDOG_TIMER_EXPIRED)          tft.println("AXIS_ERROR_WATCHDOG_TIMER_EXPIRED");
    else if (errorCode == AXIS_ERROR_MIN_ENDSTOP_PRESSED)             tft.println("AXIS_ERROR_MIN_ENDSTOP_PRESSED");
    else if (errorCode == AXIS_ERROR_MAX_ENDSTOP_PRESSED)             tft.println("AXIS_ERROR_MAX_ENDSTOP_PRESSED");
    else if (errorCode == AXIS_ERROR_ESTOP_REQUESTED)                 tft.println("AXIS_ERROR_ESTOP_REQUESTED");
    else if (errorCode == AXIS_ERROR_HOMING_WITHOUT_ENDSTOP)          tft.println("AXIS_ERROR_HOMING_WITHOUT_ENDSTOP");
    else if (errorCode == AXIS_ERROR_OVER_TEMP)                       tft.println("AXIS_ERROR_OVER_TEMP");
    else if (errorCode == AXIS_ERROR_UNKNOWN_POSITION)                tft.println("AXIS_ERROR_UNKNOWN_POSITION");
    return;
  }

  if (axis != -1 && Component::MOTOR) {
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
    return;
  }

  if (axis != -1 && Component::CONTROLLER) {
    if      (errorCode == CONTROLLER_ERROR_NONE)                      tft.println("CONTROLLER_ERROR_NONE");
    else if (errorCode == CONTROLLER_ERROR_OVERSPEED)                 tft.println("CONTROLLER_ERROR_OVERSPEED");
    else if (errorCode == CONTROLLER_ERROR_INVALID_INPUT_MODE)        tft.println("CONTROLLER_ERROR_INVALID_INPUT_MODE");
    else if (errorCode == CONTROLLER_ERROR_UNSTABLE_GAIN)             tft.println("CONTROLLER_ERROR_UNSTABLE_GAIN"); 
    else if (errorCode == CONTROLLER_ERROR_INVALID_MIRROR_AXIS)       tft.println("CONTROLLER_ERROR_INVALID_MIRROR_AXIS"); 
    else if (errorCode == CONTROLLER_ERROR_INVALID_LOAD_ENCODER)      tft.println("CONTROLLER_ERROR_INVALID_LOAD_ENCODER"); 
    else if (errorCode == CONTROLLER_ERROR_INVALID_ESTIMATE)          tft.println("CONTROLLER_ERROR_INVALID_ESTIMATE");
    else if (errorCode == CONTROLLER_ERROR_INVALID_CIRCULAR_RANGE)    tft.println("CONTROLLER_ERROR_INVALID_CIRCULAR_RANGE"); 
    else if (errorCode == CONTROLLER_ERROR_SPINOUT_DETECTED)          tft.println("CONTROLLER_ERROR_SPINOUT_DETECTED");  
    return;
  }

  if (axis != -1 && Component::ENCODER) {
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
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y); 

  // ODrive top level errors
  uint32_t err = oDriveExt.getODriveErrors(-1, Component::NONE);
  oDriveScreen.decodeODriveErrors(-1, Component::NONE, err);
  
  // AZM Errors
  y_offset +=OD_ERR_SPACING + 6;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  tft.print("----AZM ERRORS----");

  for (Component comp = Component::AXIS; comp != Component::COMP_LAST; ++comp) {
    y_offset +=OD_ERR_SPACING;
    tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
    uint32_t err = oDriveExt.getODriveErrors(AZM_MOTOR, comp);
    oDriveScreen.decodeODriveErrors(AZM_MOTOR, comp, err);
  }

  // ALT Errors
  y_offset +=OD_ERR_SPACING + 6;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  tft.print("----ALT ERRORS----");

  for (Component comp = Component::AXIS; comp != Component::COMP_LAST; ++comp) {
    y_offset +=OD_ERR_SPACING;
    tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
    uint32_t err = oDriveExt.getODriveErrors(ALT_MOTOR, comp);
    decodeODriveErrors(ALT_MOTOR, comp, err);
  }
}

// ========  Update ODrive Page Buttons ========
void ODriveScreen::updateOdriveButtons() {
  tft.setFont(&Inconsolata_Bold8pt7b);

  int x_offset = 0;
  int y_offset = 0;
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;

  // ----- Column 1 -----
  if (oDriveExt.odriveAzmPwr || oDriveExt.getODriveCurrentState(AZM_MOTOR) == AXIS_STATE_CLOSED_LOOP_CONTROL) {
    drawButton(OD_ACT_COL_1_X + x_offset, OD_ACT_COL_1_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUTTON_ON, OD_ACT_TEXT_X_OFFSET, OD_ACT_TEXT_Y_OFFSET,   "AZ Enabled");
  } else { //motor off
    drawButton(OD_ACT_COL_1_X + x_offset, OD_ACT_COL_1_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUTTON_OFF, OD_ACT_TEXT_X_OFFSET, OD_ACT_TEXT_Y_OFFSET, "  EN AZ   ");
  }

  y_offset += OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  if (oDriveExt.odriveAltPwr || oDriveExt.getODriveCurrentState(ALT_MOTOR) == AXIS_STATE_CLOSED_LOOP_CONTROL) {
    drawButton(OD_ACT_COL_1_X + x_offset, OD_ACT_COL_1_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUTTON_ON, OD_ACT_TEXT_X_OFFSET, OD_ACT_TEXT_Y_OFFSET,   "ALT Enabled");
  } else { //motor off
    drawButton(OD_ACT_COL_1_X + x_offset, OD_ACT_COL_1_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUTTON_OFF, OD_ACT_TEXT_X_OFFSET, OD_ACT_TEXT_Y_OFFSET, "  EN ALT   ");
  }

  // ----- Column 2 -----
  y_offset = 0;
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  if (OdStopButton) {
    drawButton(OD_ACT_COL_2_X + x_offset, OD_ACT_COL_2_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUTTON_ON, OD_ACT_TEXT_X_OFFSET, OD_ACT_TEXT_Y_OFFSET,    "AllStopped");
    OdStopButton = false;
  } else { 
    drawButton(OD_ACT_COL_2_X + x_offset, OD_ACT_COL_2_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUTTON_OFF, OD_ACT_TEXT_X_OFFSET+5, OD_ACT_TEXT_Y_OFFSET, "  STOP!  ");
  }

  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Clear Errors
  if (!clearODriveErr) {
    drawButton(OD_ACT_COL_2_X + x_offset, OD_ACT_COL_2_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUTTON_OFF, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "Clear Errors");
  } else {
    drawButton(OD_ACT_COL_2_X + x_offset, OD_ACT_COL_2_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUTTON_ON, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "Errs Cleared");
    clearODriveErr = false;
  }

  // ----- 3rd Column -----
  y_offset = -160;
  
  // AZ Gains High
  if (!AZgainHigh) {
    drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, false, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "AZ Gain Hi");
  } else {
    drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, true, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "AZ Gain Hi");
  }

  // AZ Gains Default
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (!AZgainDefault) {
    drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, false, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "AZ Gain Def");
  } else {
    drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, true, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "AZ Gain Def");
  }

  // ALT Velocity Gain High
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (!ALTgainHigh) {
    drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, false, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "ALT Gain Hi");
  } else {
    drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, true, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "ALT Gain Hi");
  }

  // ALT Velocity Gain Default
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (!ALTgainDefault) {
    drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, false, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "ALT Gain Def");
  } else {
    drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, true, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "ALT Gain Def");
  }

  y_offset = 0;
  // Demo Button
  if (!demoActive) {
    drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUTTON_OFF, OD_ACT_TEXT_X_OFFSET-4, OD_ACT_TEXT_Y_OFFSET, "Demo ODrive");
  } else {
    drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUTTON_ON, OD_ACT_TEXT_X_OFFSET-4, OD_ACT_TEXT_Y_OFFSET, "Demo Active");
  }

  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Reset ODrive Button
  if (!resetODriveFlag) {
    drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUTTON_OFF, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "Reset ODrive");
  } else {
    drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUTTON_ON, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "  Resetting ");
    resetODriveFlag = false;
  }

  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Enable or Disable the ODrive position update via UART
  if (ODpositionUpdateEnabled) {
    drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUTTON_OFF, OD_ACT_TEXT_X_OFFSET-2, OD_ACT_TEXT_Y_OFFSET, "Dis UpDates");
  } else {
    drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUTTON_ON, OD_ACT_TEXT_X_OFFSET-2, OD_ACT_TEXT_Y_OFFSET,   "Ena UpDates");
  }
}

// =========== ODrive touchscreen update ===========
bool ODriveScreen::touchPoll(uint16_t px, uint16_t py) {
  int x_offset = 0;
  int y_offset = 0;
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;

  // ===== Column 1 - Leftmost ======
  // Enable Azimuth motor
  if (px > OD_ACT_COL_1_X + x_offset && px < OD_ACT_COL_1_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_1_Y + y_offset && py <  OD_ACT_COL_1_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    BEEP;
    if (!oDriveExt.odriveAzmPwr) { // if not On, toggle ON
      digitalWrite(AZ_ENABLED_LED_PIN, LOW); // Turn On AZ LED
      oDriveExt.odriveAzmPwr = true;
      motor1.power(true);
    } else { // since already ON, toggle OFF
      digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZ LED
      oDriveExt.odriveAzmPwr = false;
      motor1.power(false);
    }
    return true;
  }
            
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Enable Altitude motor
  if (px > OD_ACT_COL_1_X + x_offset && px < OD_ACT_COL_1_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_1_Y + y_offset && py <  OD_ACT_COL_1_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    BEEP;
    if (!oDriveExt.odriveAltPwr) { // toggle ON
      digitalWrite(ALT_ENABLED_LED_PIN, LOW); // Turn On ALT LED
      oDriveExt.odriveAltPwr = true; 
      motor2.power(true);
    } else { // toggle OFF
      digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn off ALT LED
      oDriveExt.odriveAltPwr = false;
      motor2.power(false); // Idle the ODrive motor
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
      motor2.power(false);
      OdStopButton = true;
      oDriveExt.odriveAzmPwr = false;
      oDriveExt.odriveAltPwr = false;
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
    oDriveExt.clearODriveErrors(AZM_MOTOR, ENCODER);
    oDriveExt.clearODriveErrors(AZM_MOTOR, CONTROLLER);
    oDriveExt.clearODriveErrors(AZM_MOTOR, MOTOR);
    oDriveExt.clearODriveErrors(ALT_MOTOR, ENCODER);
    oDriveExt.clearODriveErrors(ALT_MOTOR, CONTROLLER);
    oDriveExt.clearODriveErrors(ALT_MOTOR, MOTOR);
    clearODriveErr = true;
    return true;
  }

  // Column 3
  y_offset = -165;
  // AZ Gain HIGH
  if (px > OD_ACT_COL_3_X + x_offset && px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_3_Y + y_offset && py <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y-box_height_adj) {
    BEEP;
    AZgainHigh = true;
    AZgainDefault = false;
    oDriveExt.setODriveVelGain(AZM_MOTOR, 1.8); // Set Velocity Gain
    delay(1);
    oDriveExt.setODriveVelIntGain(AZM_MOTOR, 2.3); // Set Velocity Integrator Gain
    return true;
  }

    // AZ Gain DEFault
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (px > OD_ACT_COL_3_X + x_offset && px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_3_Y + y_offset && py <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y-box_height_adj) {
    BEEP;
    AZgainHigh = false;
    AZgainDefault = true;
    oDriveExt.setODriveVelGain(AZM_MOTOR, 1.5);
    delay(1);
    oDriveExt.setODriveVelIntGain(AZM_MOTOR, 2.0);
    return true;
  }

    // ALT Gain HIGH
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (px > OD_ACT_COL_3_X + x_offset && px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_3_Y + y_offset && py <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y-box_height_adj) {
    BEEP;
    ALTgainHigh = true;
    ALTgainDefault = false;
    oDriveExt.setODriveVelGain(ALT_MOTOR, 0.5); // Set Velocity Gain
    delay(1);
    oDriveExt.setODriveVelIntGain(ALT_MOTOR, 0.7); // Set Velocity Integrator Gain
    return true;
  }

    // ALT Gain DEFault
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (px > OD_ACT_COL_3_X + x_offset && px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_3_Y + y_offset && py <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y-box_height_adj) {
    BEEP;
    ALTgainHigh = false;
    ALTgainDefault = true;
    oDriveExt.setODriveVelGain(ALT_MOTOR, 0.3);
    delay(1);
    oDriveExt.setODriveVelIntGain(ALT_MOTOR, 0.4);
    return true;
  }

  y_offset = 0;
  // Demo Mode for ODrive
  // Toggle on Demo Mode if button pressed, toggle off if pressed and already on
  // Demo mode relies on a pseudo-thread that fires off the change in positions
  if (px > OD_ACT_COL_3_X + x_offset && px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_3_Y + y_offset && py <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    BEEP;
    if (!demoActive) {
      VLF("MSG: Demo ODrive");
      demoActive = true;
      //demoHandle = tasks.add(10000, 0, true, 6, oDriveExt.demoMode(true), "Demo On");
    } else {
      demoActive = false;
      VLF("MSG: Demo OFF ODrive");
      //tasks.remove(demoHandle);
      oDriveExt.demoMode(false);
      //tasks.setDurationComplete(tasks.getHandleByName("Demo On"));
    }
    return true;
  }

  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Reset ODRIVE
  if (px > OD_ACT_COL_3_X + x_offset && px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_3_Y + y_offset && py <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    BEEP;
    VLF("MSG: Reseting ODrive");
    tasks.setDurationComplete(tasks.getHandleByName("Target_0")); // not sure about the name
    tasks.setDurationComplete(tasks.getHandleByName("Target_1")); // not sure about the name
    ODRIVE_SERIAL.end();
    delay(5);
    digitalWrite(ODRIVE_RST, LOW);
    delay(1);
    digitalWrite(ODRIVE_RST, HIGH);
    delay(500);
    axis1.init(&motor1); // start motor timers and serial
    axis2.init(&motor2);
    resetODriveFlag = true;
    AZgainHigh = false;
    AZgainDefault = true;
    ALTgainHigh = false;
    ALTgainDefault = true;
    return true;
  }

  // Disable / Enable ODrive motor position update - UART between ODrive and Teensy
  // Disable-position-updates so that they don't override ODrive
  // motor positions while tuning ODrive with ODrive USB channel
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  if (px > OD_ACT_COL_3_X + x_offset && px < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && py > OD_ACT_COL_3_Y + y_offset && py <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    BEEP;
    if (ODpositionUpdateEnabled) {
      ODpositionUpdateEnabled = false;
    } else {
      ODpositionUpdateEnabled = true;
    }
    return true;
  }  
  return false;
}    

ODriveScreen oDriveScreen;