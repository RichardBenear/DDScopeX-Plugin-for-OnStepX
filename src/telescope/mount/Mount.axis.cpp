//--------------------------------------------------------------------------------------------------
// telescope mount control, axis instances

#include "Mount.h"

#ifdef MOUNT_PRESENT

#ifdef AXIS1_ODRIVE_PRESENT
  const ODriveDriverSettings ODriveSettingsAxis1 = {AXIS1_DRIVER_MODEL, AXIS1_DRIVER_STATUS};
  ODriveMotor motor1(1, &ODriveSettingsAxis1);
#endif

#ifdef AXIS1_SERVO_PRESENT
  ServoControl servoControlAxis1;

  #if AXIS1_SERVO_ENCODER == ENC_AB
    Encoder encAxis1(AXIS1_SERVO_ENC1_PIN, AXIS1_SERVO_ENC2_PIN);
  #else
    Encoder encAxis1(AXIS1_SERVO_ENC1_PIN, AXIS1_SERVO_ENC2_PIN, AXIS1_SERVO_ENCODER, AXIS1_SERVO_ENCODER_TRIGGER, &servoControlAxis1.directionHint);
  #endif

  #if AXIS1_SERVO_FEEDBACK == FB_PID
    Pid pidAxis1(AXIS1_SERVO_P, AXIS1_SERVO_I, AXIS1_SERVO_D, AXIS1_SERVO_P_GOTO, AXIS1_SERVO_I_GOTO, AXIS1_SERVO_D_GOTO);
  #endif

  #if defined(AXIS1_SERVO_DC)
    const ServoDcPins ServoPinsAxis1 = {AXIS1_SERVO_PH1_PIN, AXIS1_SERVO_PH1_STATE, AXIS1_SERVO_PH2_PIN, AXIS1_SERVO_PH2_STATE, AXIS1_ENABLE_PIN, AXIS1_ENABLE_STATE, AXIS1_FAULT_PIN};
    const ServoDcSettings ServoSettingsAxis1 = {AXIS1_DRIVER_MODEL, AXIS1_DRIVER_STATUS};
    ServoDc driver1(1, &ServoPinsAxis1, &ServoSettingsAxis1);
  #elif defined(AXIS1_SERVO_TMC2209)
    const ServoTmcPins ServoPinsAxis1 = {AXIS1_STEP_PIN, AXIS1_DIR_PIN, AXIS1_ENABLE_PIN, AXIS1_ENABLE_STATE, AXIS1_M0_PIN, AXIS1_M1_PIN, AXIS1_FAULT_PIN};
    const ServoTmcSettings ServoSettingsAxis1 = {AXIS1_DRIVER_MODEL, AXIS1_DRIVER_STATUS, AXIS1_DRIVER_MICROSTEPS, AXIS1_DRIVER_IRUN, AXIS1_DRIVER_VELOCITY};
    ServoTmc2209 driver1(1, &ServoPinsAxis1, &ServoSettingsAxis1);
  #endif

  ServoMotor motor1(1, ((ServoDriver*)&driver1), &encAxis1, &pidAxis1, &servoControlAxis1);
#endif

#ifdef AXIS1_STEP_DIR_PRESENT
  const StepDirDriverPins DriverPinsAxis1 = {AXIS1_M0_PIN, AXIS1_M1_PIN, AXIS1_M2_PIN, AXIS1_M2_ON_STATE, AXIS1_M3_PIN, AXIS1_DECAY_PIN, AXIS1_FAULT_PIN};
  const StepDirDriverSettings DriverSettingsAxis1 = {AXIS1_DRIVER_MODEL, AXIS1_DRIVER_MICROSTEPS, AXIS1_DRIVER_MICROSTEPS_GOTO, AXIS1_DRIVER_IHOLD, AXIS1_DRIVER_IRUN, AXIS1_DRIVER_IGOTO, AXIS1_DRIVER_DECAY, AXIS1_DRIVER_DECAY_GOTO, AXIS1_DRIVER_STATUS};
  #if defined(AXIS1_STEP_DIR_LEGACY)
    StepDirGeneric driver1(1, &DriverPinsAxis1, &DriverSettingsAxis1);
  #elif defined(AXIS1_STEP_DIR_TMC_SPI)
    StepDirTmcSPI driver1(1, &DriverPinsAxis1, &DriverSettingsAxis1);
  #elif defined(AXIS1_STEP_DIR_TMC_UART)
    StepDirTmcUART driver1(1, &DriverPinsAxis1, &DriverSettingsAxis1);
  #endif

  const StepDirPins StepDirPinsAxis1 = {AXIS1_STEP_PIN, AXIS1_STEP_STATE, AXIS1_DIR_PIN, AXIS1_ENABLE_PIN, AXIS1_ENABLE_STATE};
  StepDirMotor motor1(1, &StepDirPinsAxis1, ((StepDirDriver*)&driver1));
#endif

const AxisPins PinsAxis1 = {AXIS1_SENSE_LIMIT_MIN_PIN, AXIS1_SENSE_HOME_PIN, AXIS1_SENSE_LIMIT_MAX_PIN, {AXIS1_SENSE_HOME, AXIS1_SENSE_HOME_INIT, degToRadF(AXIS1_SENSE_HOME_DIST_LIMIT), AXIS1_SENSE_LIMIT_MIN, AXIS1_SENSE_LIMIT_MAX, AXIS1_SENSE_LIMIT_INIT}};
const AxisSettings SettingsAxis1 = {AXIS1_STEPS_PER_DEGREE*RAD_DEG_RATIO, AXIS1_REVERSE, {degToRadF(AXIS1_LIMIT_MIN), degToRadF(AXIS1_LIMIT_MAX)}, siderealToRad(TRACK_BACKLASH_RATE)};
Axis axis1(1, &PinsAxis1, &SettingsAxis1, AXIS_MEASURE_RADIANS);

#ifdef AXIS2_ODRIVE_PRESENT
  const ODriveDriverSettings ODriveSettingsAxis2 = {AXIS2_DRIVER_MODEL, AXIS2_DRIVER_STATUS};
  ODriveMotor motor2(2, &ODriveSettingsAxis2);
#endif

#ifdef AXIS2_SERVO_PRESENT
  ServoControl servoControlAxis2;

  #if AXIS2_SERVO_ENCODER == ENC_AB
    Encoder encAxis2(AXIS2_SERVO_ENC1_PIN, AXIS2_SERVO_ENC2_PIN);
  #else
    Encoder encAxis2(AXIS2_SERVO_ENC1_PIN, AXIS2_SERVO_ENC2_PIN, AXIS2_SERVO_ENCODER, AXIS2_SERVO_ENCODER_TRIGGER, &servoControlAxis2.directionHint);
  #endif

  #if AXIS2_SERVO_FEEDBACK == FB_PID
    Pid pidAxis2(AXIS2_SERVO_P, AXIS2_SERVO_I, AXIS2_SERVO_D, AXIS2_SERVO_P_GOTO, AXIS2_SERVO_I_GOTO, AXIS2_SERVO_D_GOTO);
  #endif

  #if defined(AXIS2_SERVO_DC)
    const ServoDcPins ServoPinsAxis2 = {AXIS2_SERVO_PH1_PIN, AXIS2_SERVO_PH1_STATE, AXIS2_SERVO_PH2_PIN, AXIS2_SERVO_PH2_STATE, AXIS2_ENABLE_PIN, AXIS2_ENABLE_STATE, AXIS2_FAULT_PIN};
    const ServoDcSettings ServoSettingsAxis2 = {AXIS2_DRIVER_MODEL, AXIS2_DRIVER_STATUS};
    ServoDc driver2(2, &ServoPinsAxis2, &ServoSettingsAxis2);
  #elif defined(AXIS2_SERVO_TMC2209)
    const ServoTmcPins ServoPinsAxis2 = {AXIS2_STEP_PIN, AXIS2_DIR_PIN, AXIS2_ENABLE_PIN, AXIS2_ENABLE_STATE, AXIS2_M0_PIN, AXIS2_M1_PIN, AXIS2_FAULT_PIN};
    const ServoTmcSettings ServoSettingsAxis2 = {AXIS2_DRIVER_MODEL, AXIS2_DRIVER_STATUS, AXIS2_DRIVER_MICROSTEPS, AXIS2_DRIVER_IRUN, AXIS2_DRIVER_VELOCITY};
    ServoTmc2209 driver2(2, &ServoPinsAxis2, &ServoSettingsAxis2);
  #endif

  ServoMotor motor2(2, ((ServoDriver*)&driver2), &encAxis2, &pidAxis2, &servoControlAxis2);
  IRAM_ATTR void moveAxis2() { motor2.move(); }
#endif

#ifdef AXIS2_STEP_DIR_PRESENT
  const StepDirDriverPins StepDirDriverPinsAxis2 = {AXIS2_M0_PIN, AXIS2_M1_PIN, AXIS2_M2_PIN, AXIS2_M2_ON_STATE, AXIS2_M3_PIN, AXIS2_DECAY_PIN, AXIS2_FAULT_PIN};
  const StepDirDriverSettings StepDirDriverSettingsAxis2 = {AXIS2_DRIVER_MODEL, AXIS2_DRIVER_MICROSTEPS, AXIS1_DRIVER_MICROSTEPS_GOTO, AXIS2_DRIVER_IHOLD, AXIS2_DRIVER_IRUN, AXIS2_DRIVER_IGOTO, AXIS2_DRIVER_DECAY, AXIS2_DRIVER_DECAY_GOTO, AXIS2_DRIVER_STATUS};
  #if defined(AXIS2_STEP_DIR_LEGACY)
    StepDirGeneric driver2(2, &StepDirDriverPinsAxis2, &StepDirDriverSettingsAxis2);
  #elif defined(AXIS2_STEP_DIR_TMC_SPI)
    StepDirTmcSPI driver2(2, &StepDirDriverPinsAxis2, &StepDirDriverSettingsAxis2);
  #elif defined(AXIS2_STEP_DIR_TMC_UART)
    StepDirTmcUART driver2(2, &StepDirDriverPinsAxis2, &StepDirDriverSettingsAxis2);
  #endif

  const StepDirPins StepDirPinsAxis2 = {AXIS2_STEP_PIN, AXIS2_STEP_STATE, AXIS2_DIR_PIN, AXIS2_ENABLE_PIN, AXIS2_ENABLE_STATE};
  StepDirMotor motor2(2, &StepDirPinsAxis2, ((StepDirDriver*)&driver2));
#endif

const AxisPins PinsAxis2 = {AXIS2_SENSE_LIMIT_MIN_PIN, AXIS2_SENSE_HOME_PIN, AXIS2_SENSE_LIMIT_MAX_PIN, {AXIS2_SENSE_HOME, AXIS2_SENSE_HOME_INIT, degToRadF(AXIS2_SENSE_HOME_DIST_LIMIT), AXIS2_SENSE_LIMIT_MIN, AXIS2_SENSE_LIMIT_MAX, AXIS2_SENSE_LIMIT_INIT}};
const AxisSettings SettingsAxis2 = {AXIS2_STEPS_PER_DEGREE*RAD_DEG_RATIO, AXIS2_REVERSE, {degToRadF(AXIS2_LIMIT_MIN), degToRadF(AXIS2_LIMIT_MAX)}, siderealToRad(TRACK_BACKLASH_RATE)};
Axis axis2(2, &PinsAxis2, &SettingsAxis2, AXIS_MEASURE_RADIANS);

#endif
