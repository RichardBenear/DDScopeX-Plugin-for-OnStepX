// =====================================================
// ODriveExt.cpp
// ======== ODrive Extended Support ===========
// ============================================
// Author: Richard Benear 2022
// 
// ODrive communication via Teensy 4.0
// Uses GitHub ODrive Arduino library

#include <ODriveArduino.h>
#include "ODriveExt.h"
#include "../display/Display.h"
#include "../../../telescope/mount/Mount.h"
#include "../../../lib/tasks/OnTask.h"

const char* ODriveComponentsStr[4] = {
            "None",   
            "Controller",
            "Motor",
            "Encoder"        
};

ODriveArduino oDriveArduino(ODRIVE_SERIAL);

//=========================================================
// Read bus voltage
float ODriveExt::getODriveBusVoltage() {
  ODRIVE_SERIAL << "r vbus_voltage\n";
  delay(1);
  float bat_volt = (float)(oDriveArduino.readFloat());
return (float)bat_volt;  
}

// get absolute Encoder positions in degrees
float ODriveExt::getEncoderPositionDeg(int axis) {
  noInterrupts();
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n"; 
  float turns = oDriveArduino.readFloat();
  interrupts();
  return turns*360;
}  

// get motor positions in turns
float ODriveExt::getMotorPositionTurns(int axis) {
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n"; 
  delay(1);
  return oDriveArduino.readFloat();
}  

// get motor position in counts
int ODriveExt::getMotorPositionCounts(int axis) {
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate_counts\n";
  delay(1);
  return oDriveArduino.readInt();
} 

// get Motor Current
float ODriveExt::getMotorCurrent(int axis) {
  ODRIVE_SERIAL << "r axis" << axis << ".motor.I_bus\n";
  delay(1);  
  return oDriveArduino.readFloat();
}  

// read current requested state
int ODriveExt::getODriveRequestedState() {
  ODRIVE_SERIAL << "r axis0.requested_state\n";
  delay(1);
  return oDriveArduino.readInt();
}

float ODriveExt::getMotorPositionDelta(int axis) {
  ODRIVE_SERIAL << "r axis" << axis << ".controller.pos_setpoint\n";
  delay(1);
  float reqPos = oDriveArduino.readFloat();   
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n";
  delay(1);
  float posEst = oDriveArduino.readFloat();   
  float deltaPos = abs(reqPos - posEst);
  return deltaPos;
}


// ODrive clear ALL errors
void ODriveExt::clearAllODriveErrors() {
  ODRIVE_SERIAL << "w sc\n"; 
} 

// ODrive clear subcategory errors
void ODriveExt::clearODriveErrors(int axis, int comp) {
  switch (comp) {
    case ENCODER:
      ODRIVE_SERIAL << "w axis"<<axis<<".encoder.error 0\n";
      VLF("MSG: Clearing Drive errors");
      break;
    case MOTOR:
      ODRIVE_SERIAL << "w axis"<<axis<<".motor.error 0\n";
      VLF("MSG: Clearing ODrive errors");
      break;
    case CONTROLLER:
      ODRIVE_SERIAL << "w axis"<<axis<<".controller.error 0\n";
      VLF("MSG: Clearing ODrive errors");
      break;
    case AXIS:
      ODRIVE_SERIAL << "w axis"<<axis<<".error 0\n";
      VLF("MSG: Clearing ODrive errors");
      break;
  }
}

void ODriveExt::setODriveVelGain(int axis, float level) {
  ODRIVE_SERIAL << "w axis"<<axis<<".controller.config.vel_gain "<<level<<'\n';
}

void ODriveExt::setODriveVelIntGain(int axis, float level) {
  ODRIVE_SERIAL << "w axis"<<axis<<".controller.config.vel_integrator_gain "<<level<<'\n';
}

void ODriveExt::setODrivePosGain(int axis, float level) {
  ODRIVE_SERIAL << "w axis"<<axis<<".controller.config.pos_gain "<<level<<'\n';
}

float ODriveExt::getODriveVelGain(int axis) {
  ODRIVE_SERIAL << "r axis"<<axis<<".controller.config.vel_gain\n";
  delay(1);
  return oDriveArduino.readFloat();
}

float ODriveExt::getODriveVelIntGain(int axis) {
  ODRIVE_SERIAL << "r axis"<<axis<<".controller.config.vel_integrator_gain\n";
  delay(1);
  return oDriveArduino.readFloat();
}

float ODriveExt::getODrivePosGain(int axis) {
  ODRIVE_SERIAL << "r axis"<<axis<<".controller.config.pos_gain\n";
  delay(1);
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

// ========  Get the ODRIVE errors ========
uint32_t ODriveExt::getODriveErrors(int axis, Component component) {
  if (axis == -1) { // ODrive top level error
    ODRIVE_SERIAL << "r odrive.error\n";
    return oDriveArduino.readInt();
  }

  if (component == NONE) {
    ODRIVE_SERIAL << "r odrive.axis"<<axis<<".error\n";
    return oDriveArduino.readInt();
  } else {
    ODRIVE_SERIAL << "r odrive.axis"<<axis<<"."<<ODriveComponentsStr[component]<<".error\n";
    return oDriveArduino.readInt();
  }
}

void ODriveExt::getODriveVersion(ODriveVersion oDversion) {
  ODRIVE_SERIAL << "r hw_version_major\n"; 
  oDversion.hwMajor = oDriveArduino.readInt();
  ODRIVE_SERIAL << "r hw_version_minor\n"; 
  oDversion.hwMinor = oDriveArduino.readInt();
  ODRIVE_SERIAL << "r hw_version_variant\n"; 
  oDversion.hwVar = oDriveArduino.readInt();
  ODRIVE_SERIAL << "r fw_version_major\n"; 
  oDversion.fwMajor = oDriveArduino.readInt();
  ODRIVE_SERIAL << "r fw_version_minor\n"; 
  oDversion.fwMinor = oDriveArduino.readInt();
  ODRIVE_SERIAL << "r fw_version_revision\n"; 
  oDversion.fwRev = oDriveArduino.readInt();
}

// =================== Demo Mode ====================
// Demo mode requires that the OnStep updates to the Axis be stopped but the Motor power is ON
void ODriveExt::demoMode(bool onState) {
  tasks.setDurationComplete(tasks.getHandleByName("Target_0")); // not sure about the name
  // choose some AZM and ALT positions in fractional "Turns"
  // ALT position should never be negative in actual use because of the position of the focuser but it "can" go negative in demo
  // ALT axis at Turn = 0.0 is telescope vertical
  // AZ axis at Turn = 0.0 is telescope pointing North (when physically aligned north)
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
  } else { // off
    oDriveArduino.SetPosition(ALT_MOTOR, 0);
    oDriveArduino.SetPosition(AZM_MOTOR, 0);
    axis1.init(&motor1); // start motor timers
    axis2.init(&motor2);
  }
}

ODriveExt oDriveExt;