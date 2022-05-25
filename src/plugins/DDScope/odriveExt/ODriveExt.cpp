// =====================================================
// ODriveExt.cpp
// ======== Odrive Extended Support ===========
// ============================================
// Author: Richard Benear 2022
// ODrive Extended Functions
// ODrive communication via Teensy 4.0
// Uses GitHub ODrive Arduino library by 

#include <ODriveArduino.h>
#include "ODriveExt.h"
#include "../display/Display.h"
#include "../../../telescope/mount/Mount.h"

//==============================================
//======= ODrive Controller Error Status =======
//==============================================

// Read bus voltage
float ODriveExt::getOdriveBusVoltage() {
  ODRIVE_SERIAL << "r vbus_voltage\n";
  float bat_volt = (float)(oDriveArduino.readFloat());
return (float)bat_volt;  
}

// get absolute Encoder positions in degrees
float ODriveExt::getEncoderPositionDeg(int axis) {
  float turns;
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n"; 
  turns = oDriveArduino.readFloat();
  return turns*360;
}  

// get motor positions in turns
float ODriveExt::getMotorPositionTurns(int axis) {
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n"; 
  return oDriveArduino.readFloat();
}  

// get motor position in counts
int ODriveExt::getMotorPositionCounts(int axis) {
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate_counts\n";
  return oDriveArduino.readInt();
} 

// get Motor Current
float ODriveExt::getMotorCurrent(int axis) {
  ODRIVE_SERIAL << "r axis" << axis << ".motor.I_bus\n";  
  return oDriveArduino.readFloat();
}  

// read current requested state
int ODriveExt::getOdriveRequestedState() {
  ODRIVE_SERIAL << "r axis0.requested_state\n";
  return oDriveArduino.readInt();
}

float ODriveExt::getMotorPositionDelta(int axis) {
  ODRIVE_SERIAL << "r axis" << axis << ".controller.pos_setpoint\n";
  float reqPos = oDriveArduino.readFloat();   
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n";
  float posEst = oDriveArduino.readFloat();   
  float deltaPos = abs(reqPos - posEst);
  return deltaPos;
}

// Odrive clear ALL errors
void ODriveExt::clearAllOdriveErrors() {
  ODRIVE_SERIAL << "w sc\n"; 
} 

// Odrive clear subcategory errors
void ODriveExt::clearOdriveErrors(int axis, int comp) {
  switch (comp) {
    case ENCODER:
      ODRIVE_SERIAL << "w axis"<<axis<<".encoder.error 0\n";
      VLF("MSG: Clearing Odrive errors");
      break;
    case MOTOR:
      ODRIVE_SERIAL << "w axis"<<axis<<".motor.error 0\n";
      VLF("MSG: Clearing Odrive errors");
      break;
    case CONTROLLER:
      ODRIVE_SERIAL << "w axis"<<axis<<".controller.error 0\n";
      VLF("MSG: Clearing Odrive errors");
      break;
    case AXIS:
      ODRIVE_SERIAL << "w axis"<<axis<<".error 0\n";
      VLF("MSG: Clearing Odrive errors");
      break;
  }
}

// Dump Error for specific module
int ODriveExt::dumpOdriveErrors(int axis, int comp) {   
  switch (comp) { 
    case ENCODER:
      ODRIVE_SERIAL << "r axis"<<axis<<".encoder.error\n";
      return oDriveArduino.readInt();
      VLF("MSG: dump Odrive encoder errors");
      break;
    case MOTOR:
    ODRIVE_SERIAL << "r axis"<<axis<<".motor.error\n";
    return oDriveArduino.readInt();
      VLF("MSG: dump Odrive motor errors");
      break;
    case CONTROLLER:
      ODRIVE_SERIAL << "r axis"<<axis<<".controller.error\n";
      return oDriveArduino.readInt();
      VLF("MSG: dump Odrive controller errors");
      break;
    case AXIS:
      ODRIVE_SERIAL << "r axis"<<axis<<".error\n";
      return oDriveArduino.readInt();
      VLF("MSG: dump Odrive axis errors");
      break;
  }
  return oDriveArduino.readInt();
}

void ODriveExt::setOdriveVelGain(int axis, float level) {
  ODRIVE_SERIAL << "w axis"<<axis<<".controller.config.vel_gain "<<level<<'\n';
}

void ODriveExt::setOdriveVelIntGain(int axis, float level) {
  ODRIVE_SERIAL << "w axis"<<axis<<".controller.config.vel_integrator_gain "<<level<<'\n';
}

void ODriveExt::setOdrivePosGain(int axis, float level) {
  ODRIVE_SERIAL << "w axis"<<axis<<".controller.config.pos_gain "<<level<<'\n';
}

float ODriveExt::getOdriveVelGain(int axis) {
  ODRIVE_SERIAL << "r axis"<<axis<<".controller.config.vel_gain\n";
  return oDriveArduino.readFloat();
}

float ODriveExt::getOdriveVelIntGain(int axis) {
  ODRIVE_SERIAL << "r axis"<<axis<<".controller.config.vel_integrator_gain\n";
  return oDriveArduino.readFloat();
}

float ODriveExt::getOdrivePosGain(int axis) {
  ODRIVE_SERIAL << "r axis"<<axis<<".controller.config.pos_gain\n";
  return oDriveArduino.readFloat();
}

// =========== Motor Thermistor Support =============
float ODriveExt::getMotorTemp(int axis) {
  int Ro = 9, B =  3950; //Nominal resistance 10K, Beta constant, 9k at 68 deg
  int Rseries = 10.0;// Series resistor 10K
  float To = 293; // Nominal Temperature 68 deg calibration point
  float Vi = 0;

  /*Read analog output of NTC module,
    i.e the voltage across the thermistor */
  // IMPORTANT: USE 3.3V for Thermistor!! Teensy pins are NOT 5V tolerant!
  if (axis == 1) 
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

// =================== Demo Mode ====================
// This requires that the OnStep update to the Axis is stopped but the Motor power is ON
void ODriveExt::demoMode(bool onState) {
  // choose some AZM and ALT positions in fractional "Turns"
  // ALT position should never be negative in actual use but it "can" go negative in demo
  float pos_one = 0.15;
  float pos_two = 0.3;
  float pos_three = -0.1;
  float pos_four = -0.4;
  float pos_five = 0.4;
  int demo_pos = 0;
  display.setLocalCmd(":Q#"); // does not turn off Motor power
  
  if (onState) {
    switch(demo_pos) {
      case 0:
        oDriveArduino.SetPosition(ALT_MOTOR, pos_one);
        oDriveArduino.SetPosition(AZM_MOTOR, pos_one);
        ++demo_pos;
        break;
      case 1:
        oDriveArduino.SetPosition(ALT_MOTOR, pos_two);
        oDriveArduino.SetPosition(AZM_MOTOR, pos_two);
        ++demo_pos;
        break;
      case 2:
        oDriveArduino.SetPosition(ALT_MOTOR, pos_three);
        oDriveArduino.SetPosition(AZM_MOTOR, pos_one);
        ++demo_pos;
        break;
      case 3:
        oDriveArduino.SetPosition(ALT_MOTOR, pos_four);
        oDriveArduino.SetPosition(AZM_MOTOR, pos_five);
        demo_pos = 0;
        break;
      default:
        oDriveArduino.SetPosition(ALT_MOTOR, pos_one);
        oDriveArduino.SetPosition(AZM_MOTOR, pos_one);
        demo_pos = 0;
        break;
    }
  } else {
    oDriveArduino.SetPosition(ALT_MOTOR, 0);
    oDriveArduino.SetPosition(AZM_MOTOR, 0);
  }
}

ODriveExt oDriveExt;
ODriveArduino oDriveArduino(ODRIVE_SERIAL);