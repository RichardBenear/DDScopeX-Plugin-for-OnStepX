// =====================================================
// ODrive.cpp
// ============== Odrive Support ==============
// ============================================
// Author: Richard Benear 2020
// ODrive Extended Functions
// ODrive communication via Teensy 4.0
// Uses GitHub ODrive Arduino library by 

#include "../display/Display.h"
#include "ODrive.h"
#include <ODriveArduino.h>
#include "../../../lib/axis/motor/oDrive/ODrive.h"

// Printing with stream operator helper functions
template<class T> inline Print& operator <<(Print& obj, T arg) { obj.print(arg);    return obj; }
template<>        inline Print& operator <<(Print& obj, float arg) { obj.print(arg, 4); return obj; }

//==============================================
//======= ODrive Controller Error Status =======
//==============================================

// Read bus voltage
float ODrive::getOdriveBusVoltage() {
  ODRIVE_SERIAL << "r vbus_voltage\n";
  float bat_volt = (float)(odriveArduino.readFloat());
return (float)bat_volt;  
}

// get absolute Encoder positions in degrees
float ODrive::getEncoderPositionDeg(int axis) {
  float turns;
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n"; 
  turns = odriveArduino.readFloat();
  return turns*360;
}  

// get motor positions in turns
float ODrive::getMotorPositionTurns(int axis) {
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n"; 
return odriveArduino.readFloat();
}  

// get motor position in counts
int ODrive::getMotorPositionCounts(int axis) {
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate_counts\n";
return odriveArduino.readInt();
} 

// get Motor Current
float ODrive::getMotorCurrent(int axis) {
  ODRIVE_SERIAL << "r axis" << axis << ".motor.I_bus\n";  
return odriveArduino.readFloat();
}  

// read current requested state
int ODrive::getOdriveRequestedState() {
  ODRIVE_SERIAL << "r axis0.requested_state\n";
  return odriveArduino.readInt();
}

float ODrive::getMotorPositionDelta(int axis) {
  ODRIVE_SERIAL << "r axis" << axis << ".controller.pos_setpoint\n";
  float reqPos = odriveArduino.readFloat();   
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n";
  float posEst = odriveArduino.readFloat();   
  float deltaPos = abs(reqPos - posEst);
  return deltaPos;
}

// Odrive clear ALL errors
void ODrive::clearAllOdriveErrors() {
  ODRIVE_SERIAL << "w sc\n"; 
} 

// Odrive clear subcategory errors
void ODrive::clearOdriveErrors(int axis, int comp) {
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
int ODrive::dumpOdriveErrors(int axis, int comp) {   
  switch (comp) { 
    case ENCODER:
      ODRIVE_SERIAL << "r axis"<<axis<<".encoder.error\n";
      return odriveArduino.readInt();
      VLF("MSG: dump Odrive encoder errors");
      break;
    case MOTOR:
    ODRIVE_SERIAL << "r axis"<<axis<<".motor.error\n";
    return odriveArduino.readInt();
      VLF("MSG: dump Odrive motor errors");
      break;
    case CONTROLLER:
      ODRIVE_SERIAL << "r axis"<<axis<<".controller.error\n";
      return odriveArduino.readInt();
      VLF("MSG: dump Odrive controller errors");
      break;
    case AXIS:
      ODRIVE_SERIAL << "r axis"<<axis<<".error\n";
      return odriveArduino.readInt();
      VLF("MSG: dump Odrive axis errors");
      break;
  }
  return odriveArduino.readInt();
}

void ODrive::setOdriveVelGain(int axis, float level) {
  ODRIVE_SERIAL << "w axis"<<axis<<".controller.config.vel_gain "<<level<<'\n';
}

void ODrive::setOdriveVelIntGain(int axis, float level) {
  ODRIVE_SERIAL << "w axis"<<axis<<".controller.config.vel_integrator_gain "<<level<<'\n';
}

void ODrive::setOdrivePosGain(int axis, float level) {
  ODRIVE_SERIAL << "w axis"<<axis<<".controller.config.pos_gain "<<level<<'\n';
}

float ODrive::getOdriveVelGain(int axis) {
  ODRIVE_SERIAL << "r axis"<<axis<<".controller.config.vel_gain\n";
  return odriveArduino.readFloat();
}

float ODrive::getOdriveVelIntGain(int axis) {
  ODRIVE_SERIAL << "r axis"<<axis<<".controller.config.vel_integrator_gain\n";
  return odriveArduino.readFloat();
}

float ODrive::getOdrivePosGain(int axis) {
  ODRIVE_SERIAL << "r axis"<<axis<<".controller.config.pos_gain\n";
  return odriveArduino.readFloat();
}

// =========== Motor Thermistor Support =============
float ODrive::getMotorTemp(int axis) {
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

// ======== Demo Mode ========
// This requires that the update on the Axis is stopped but the Motor power is ON
void ODrive::demoMode(int state) {
  // choose some AZM and ALT positions in fractional "Turns"
  // ALT position should never be negative in actual use but it "can" go negative in demo
  float pos_one = 0.15;
  float pos_two = 0.3;
  float pos_three = -0.1;
  float pos_four = -0.4;
  float pos_five = 0.4;
  int demo_pos = 0;
  display.setLocalCmd(":Q#"); // does this turn off motor power ???
  
  switch(demo_pos) {
    case 0:
      odriveArduino.setPosition(0, pos_one);
      odriveArduino.setPosition(1, pos_one);
      ++demo_pos;
      break;
    case 1:
      odriveArduino.setPosition(0, pos_two);
      odriveArduino.setPosition(1, pos_two);
      ++demo_pos;
      break;
    case 2:
      odriveArduino.setPosition(0, pos_three);
      odriveArduino.setPosition(1, pos_one);
      ++demo_pos;
      break;
    case 3:
      odriveArduino.setPosition(0, pos_four);
      odriveArduino.setPosition(1, pos_five);
      demo_pos = 0;
      break;
    default:
      odriveArduino.setPosition(0, pos_one);
      odriveArduino.setPosition(1, pos_one);
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
 