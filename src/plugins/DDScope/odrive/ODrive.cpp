// =====================================================
// ODrive.cpp
// ============== Odrive Support ==============
// ============================================
// Author: Richard Benear 2020
// ODrive Driver Functions
// ODrive communication via Teensy 4.0
// Uses GitHub ODrive Arduino library by Copyright (c) 2017 Oskar Weigl

#include "../display/Display.h"
#include "ODrive.h"
#include "ODriveArduino.h"
#include "../../../lib/axis/Axis.h"
#include "../../../lib/tasks/OnTask.h"
#include "../../../lib/axis/motor/oDrive/ODrive.h"



unsigned long dumpErrorUpdate;
int updateMotorsHandle = 0;
bool axis1Enabled = false;
bool axis2Enabled = false;
bool ODpositionUpdateEnabled = true;

void updateMotorWrapper() { odrive.updateOdriveMotorPositions(); }


//==============================================
//======= ODrive Controller Error Status =======
//==============================================


// Set Axis # to IDLE
bool ODrive::idleOdriveMotor(int axis) {
  int requested_state;
  int motornum = axis;
  
  requested_state = AXIS_STATE_IDLE;
  //SerialA << "Axis" << axis << ": Requesting state " << requested_state << '\n';
  
  if(!odriveArduino.runState(motornum, requested_state, false, 0.01f)) {
    VLF("MSG: Closed loop timeout");
    return false; 
  } else {
      VLF("MSG: Odrive Axis Idle");
      if (axis == odAZM) { 
        axis1Enabled = false; 
      } else if (axis == odALT) { 
        axis2Enabled = false;
      }
    return true;
  }
}

// Turn off both axis motors
void ODrive::stopMotors() { 
  //mount.enable(false);
  //axis1.enable(false); // turn off the motors
  //axis2.enable(false);
  odriveAZOff = true;
  odriveALTOff = true;
  digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZ LED
  digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn Off ALT LED
}

// Turn Axis # Motor ON 
bool ODrive::turnOnOdriveMotor(int axis) {
  int requested_state;
  int motornum = axis;
  requested_state = AXIS_STATE_CLOSED_LOOP_CONTROL;
  if(!odriveArduino.runState(motornum, requested_state, false, 0.5f)) {
    VLF("MSG: Closed loop timeout");
  return false; 
  } else {
    VLF("MSG: Odrive Motor in Closed Loop");
    if (axis == odAZM) axis1Enabled = true; else axis2Enabled = true;
  return true;
  }
}

// Read bus voltage
float ODrive::getOdriveBusVoltage() {
  odrive_serial << "r vbus_voltage\n";
  float bat_volt = (float)(odriveArduino.readFloat());
return (float)bat_volt;  
}

// get absolute Encoder positions in degrees
float ODrive::getEncoderPositionDeg(int axis) {
  float turns;
  odrive_serial << "r axis" << axis << ".encoder.pos_estimate\n"; 
  turns = odriveArduino.readFloat();
  return turns*360;
}  

// get motor positions in turns
float ODrive::getMotorPositionTurns(int axis) {
  odrive_serial << "r axis" << axis << ".encoder.pos_estimate\n"; 
return odriveArduino.readFloat();
}  

// get motor position in counts
int ODrive::getMotorPositionCounts(int axis) {
  odrive_serial << "r axis" << axis << ".encoder.pos_estimate_counts\n";
return odriveArduino.readInt();
} 

// get Motor Current
float ODrive::getMotorCurrent(int axis) {
  odrive_serial << "r axis" << axis << ".motor.I_bus\n";  
return odriveArduino.readFloat();
}  

// read current requested state
int ODrive::getOdriveRequestedState() {
  odrive_serial << "r axis0.requested_state\n";
  return odriveArduino.readInt();
}

float ODrive::getMotorPositionDelta(int axis) {
  odrive_serial << "r axis" << axis << ".controller.pos_setpoint\n";
  float reqPos = odriveArduino.readFloat();   
  odrive_serial << "r axis" << axis << ".encoder.pos_estimate\n";
  float posEst = odriveArduino.readFloat();   
  float deltaPos = abs(reqPos - posEst);
  return deltaPos;
}

// Update both ALT and AZ axis positions in turns
// Two slew modes are supported for DDscope
// 1) When DDT_SLEW_MODE_STEPPER defined, then motors are updated at the Onstep step rate
// 2) When DDT_SLEW_MODE_STEPPER not defined, then motors slew at the max limits set in ODrive
// 3) Both modes use the Onstep tracking updates during tracking
// Slewing with ODrive is faster and based on a trapezoidal trajectory
// Odrive slews reach their target faster but must still wait for Onstep to finish slews before
// switching back to tracking.
#ifdef DDT_SLEW_MODE_STEPPER

void ODrive::updateOdriveMotorPositions() { 
  double alt, az;
  double alt_pos = 0;
  double az_pos = 0;

  // get real-time altitude and azimuth in degrees
  alt = getPosition(CR_MOUNT_HOR).a;
  az = getPosition(CR_MOUNT_HOR).z;
  while (az >= 360.0) az -= 360.0; 
  while (az < 0.0)  az += 360.0; // limits to + or - 360
  
  // ALT position; calculate turns and put on limits
  alt_pos = alt/360.00;
  if (alt_pos < 0.00) alt_pos = 0.00; // does exactly 0 cause CommandError=CE_GOTO_ERR_BELOW_HORIZON?
  if (alt_pos > 90.00) alt_pos = 90.00; // does exactly 90 cause CommandError=CE_GOTO_ERR_ABOVE_OVERHEAD?
  
  // AZ axis only turns 180 deg left or right to keep cables from twisting too much
  if (az > 180) az = az - 360.00; // flip sign too
  az_pos = az/360.00; // convert to turns
  
  // setPosition is in "turns". Always fractional.
  // Altitude range is between 0.0 and 0.5 turns
  // Azimuth range is between 0.5 and -0.5 turns
  if (!odriveAZOff ) odriveArduino.setPosition(odAZM, az_pos); 
  if (!odriveALTOff ) odriveArduino.setPosition(odALT, alt_pos);
}

#else // Slew at the ODrive controller trapezoidal vel limit speed, then track with Onstep

void ODrive::updateOdriveMotorPositions() {
  double alt = 0.0;
  double az = 0.0;
  double ot_azm_d, ot_alt_d;
  double alt_pos = 0;
  double az_pos = 0;
  char azmDMS[11] = "";
  char altDMS[12] = "";

  if (!mountStatus.isSlewing()) { // not slewing
    display.getLocalCmdTrim(":GZ#", azmDMS); // DDD*MM'SS# 
    shc.dmsToDouble(&ot_azm_d, azmDMS, false, true);

    display.getLocalCmdTrim(":GA#", altDMS);	// sDD*MM'SS#
    shc.dmsToDouble(&ot_alt_d, altDMS, true, true);

    // Target AZ in degrees
    if ((az != ot_azm_d)) {
      az = ot_azm_d;
    }
    
    // Target ALT in degrees
    if ((alt != ot_alt_d)) {
      alt = ot_alt_d;
    }
  } else {  // not slewing, but tracking so use real-time updates
      // get real-time altitude and azimuth in degrees
      alt = 0;//mount.getPosition(CR_MOUNT_HOR).a;
      az = 0;//mount.getPosition(CR_MOUNT_HOR).z;
      while (az >= 360.0) az -= 360.0; 
      while (az < 0.0)  az += 360.0; // limits to + or - 360
  }

  // ALT position; calculate turns and put on limits
  alt_pos = alt/360.00;
  if (alt_pos < 0.00) alt_pos = 0.00; // does exactly 0 cause CommandError=CE_GOTO_ERR_BELOW_HORIZON?
  if (alt_pos > 90.00) alt_pos = 90.00; // does exactly 90 cause CommandError=CE_GOTO_ERR_ABOVE_OVERHEAD?
  
  // AZ axis only turns 180 deg left or right to keep cables from twisting too much
  if (az > 180) az = az - 360.00; // flip sign too
  az_pos = az/360.00; // convert to turns
  
  // setPosition is in "turns". Always fractional.
  // Altitude range is between 0.0 and 0.5 turns
  // Azimuth range is between 0.5 and -0.5 turns
  if (!odriveAZOff ) odriveArduino.setPosition(odAZM, az_pos); 
  if (!odriveALTOff ) odriveArduino.setPosition(odALT, alt_pos);
}
#endif

// Odrive clear ALL errors
void ODrive::clearAllOdriveErrors() {
  odrive_serial << "w sc\n"; 
} 

// Odrive clear subcategory errors
void ODrive::clearOdriveErrors(int axis, int comp) {
    switch (comp) {
      case ENCODER:
        odrive_serial << "w axis"<<axis<<".encoder.error 0\n";
        VLF("MSG: Clearing Odrive errors");
        break;
      case MOTOR:
        odrive_serial << "w axis"<<axis<<".motor.error 0\n";
        VLF("MSG: Clearing Odrive errors");
        break;
      case CONTROLLER:
        odrive_serial << "w axis"<<axis<<".controller.error 0\n";
        VLF("MSG: Clearing Odrive errors");
        break;
      case AXIS:
        odrive_serial << "w axis"<<axis<<".error 0\n";
        VLF("MSG: Clearing Odrive errors");
        break;
    }
}

// Dump Error for specific module
int ODrive::dumpOdriveErrors(int axis, int comp) {   
  switch (comp) { 
    case ENCODER:
      odrive_serial << "r axis"<<axis<<".encoder.error\n";
      return odriveArduino.readInt();
      VLF("MSG: dump Odrive encoder errors");
      break;
    case MOTOR:
    odrive_serial << "r axis"<<axis<<".motor.error\n";
    return odriveArduino.readInt();
      VLF("MSG: dump Odrive motor errors");
      break;
    case CONTROLLER:
      odrive_serial << "r axis"<<axis<<".controller.error\n";
      return odriveArduino.readInt();
      VLF("MSG: dump Odrive controller errors");
      break;
    case AXIS:
      odrive_serial << "r axis"<<axis<<".error\n";
      return odriveArduino.readInt();
      VLF("MSG: dump Odrive axis errors");
      break;
  }
  return odriveArduino.readInt();
}

void ODrive::setOdriveVelGain(int axis, float level) {
  odrive_serial << "w axis"<<axis<<".controller.config.vel_gain "<<level<<'\n';
}

void ODrive::setOdriveVelIntGain(int axis, float level) {
  odrive_serial << "w axis"<<axis<<".controller.config.vel_integrator_gain "<<level<<'\n';
}

void ODrive::setOdrivePosGain(int axis, float level) {
  odrive_serial << "w axis"<<axis<<".controller.config.pos_gain "<<level<<'\n';
}

float ODrive::getOdriveVelGain(int axis) {
  odrive_serial << "r axis"<<axis<<".controller.config.vel_gain\n";
  return odriveArduino.readFloat();
}

float ODrive::getOdriveVelIntGain(int axis) {
  odrive_serial << "r axis"<<axis<<".controller.config.vel_integrator_gain\n";
  return odriveArduino.readFloat();
}

float ODrive::getOdrivePosGain(int axis) {
  odrive_serial << "r axis"<<axis<<".controller.config.pos_gain\n";
  return odriveArduino.readFloat();
}

// =========== Motor Thermistor Support =============
float ODrive::getMotorTemp(int motor) {
  int Ro = 9, B =  3950; //Nominal resistance 10K, Beta constant, 9k at 68 deg
  int Rseries = 10.0;// Series resistor 10K
  float To = 293; // Nominal Temperature 68 deg calibration point
  float Vi = 0;

  /*Read analog output of NTC module,
    i.e the voltage across the thermistor */
  // IMPORTANT: USE 3.3V for Thermistor!! Teensy pins are NOT 5V tolerant!
  if (motor == odALT)
    Vi = analogRead(ALT_THERMISTOR_PIN) * (3.3 / 1023.0);
  else
    Vi = analogRead(AZ_THERMISTOR_PIN) * (3.3 / 1023.0);
  //Convert voltage measured to resistance value
  //All Resistance are in kilo ohms.
  float R = (Vi * Rseries) / (3.3 - Vi);
  /*Use R value in steinhart and hart equation
    Calculate temperature value in kelvin*/
  float T =  1 / ((1 / To) + ((log(R / Ro)) / B));
  float Tc = T - 273.15; // Converting kelvin to celsius
  float Tf = Tc * 9.0 / 5.0 + 32.0; // Converting celsius to Fahrenheit
  return Tf;
}

// ======== Demo Mode ========
void ODrive::demoModeOn() {
  // choose some AZM and ALT positions in fractional "Turns"
  // ALT position should never be negative in actual use but it "can" go negative in demo
  float pos_one = 0.15;
  float pos_two = 0.3;
  float pos_three = -0.1;
  float pos_four = -0.4;
  float pos_five = 0.4;
  int demo_pos = 0;
  display.setLocalCmd(":Q#");
  // !!!!! still need to disable the updateMotors task here
  switch(demo_pos) {
    case 0:
      odriveArduino.setPosition(odAZM, pos_one);
      odriveArduino.setPosition(odALT, pos_one);
      ++demo_pos;
      break;
    case 1:
      odriveArduino.setPosition(odAZM, pos_two);
      odriveArduino.setPosition(odALT, pos_two);
      ++demo_pos;
      break;
    case 2:
      odriveArduino.setPosition(odAZM, pos_three);
      odriveArduino.setPosition(odALT, pos_one);
      ++demo_pos;
      break;
    case 3:
      odriveArduino.setPosition(odAZM, pos_four);
      odriveArduino.setPosition(odALT, pos_five);
      demo_pos = 0;
      break;
    default:
      odriveArduino.setPosition(odAZM, pos_one);
      odriveArduino.setPosition(odALT, pos_one);
      demo_pos = 0;
      break;
  }
}

void ODrive::demoModeOff() {
  odriveArduino.setPosition(odAZM, 0);
  odriveArduino.setPosition(odALT, 0);
  // !!!!! still need to enable the updateMotors task here
}

 ODrive odrive;
 