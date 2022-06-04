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
#include "src/lib/axis/motor/oDrive/ODrive.h"
#include "../display/Display.h"
#include "../../../telescope/mount/Mount.h"
#include "../../../lib/tasks/OnTask.h"
#include "src/telescope/mount/status/Status.h"

const char* ODriveComponentsStr[4] = {
            "None",   
            "Controller",
            "Motor",
            "Encoder"        
};

// ODrive serial
ODriveArduino *_oDserial;

// constructor
ODriveExt::ODriveExt() {
  _oDserial = new ODriveArduino(ODRIVE_SERIAL);
}

//=========================================================
// Read ODrive bus voltage which is approx. the battery voltage
float ODriveExt::getODriveBusVoltage() {
  ODRIVE_SERIAL << "r vbus_voltage\n";
  tasks.yield(ODRIVE_SERIAL_WAIT);
  float battery_voltage = _oDserial->readFloat();
  
  if (battery_voltage < BATTERY_LOW_VOLTAGE && battery_voltage > 3) { // battery low
    digitalWrite(BATTERY_LOW_LED_PIN, LOW); // LED on
    batLowLED = true;
    status.sound.alert();
  } else { // battery either above low voltage limit (ok) or in development mode  
    digitalWrite(BATTERY_LOW_LED_PIN, HIGH); // LED off
    batLowLED = false;
  }
  return battery_voltage;
}

// get absolute Encoder positions in degrees
float ODriveExt::getEncoderPositionDeg(int axis) {
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n"; 
  tasks.yield( ODRIVE_SERIAL_WAIT);
  float turns = _oDserial->readFloat();
  return turns*360;
}  

// get motor positions in turns
float ODriveExt::getMotorPositionTurns(int axis) {
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n"; 
  tasks.yield(ODRIVE_SERIAL_WAIT);
  return _oDserial->readFloat();
}  

// get motor position in counts
int ODriveExt::getMotorPositionCounts(int axis) {
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate_counts\n";
  tasks.yield(ODRIVE_SERIAL_WAIT);
  return _oDserial->readInt();
} 

// get Motor Current
float ODriveExt::getMotorCurrent(int axis) {
  ODRIVE_SERIAL << "r axis" << axis << ".motor.I_bus\n";
  tasks.yield(ODRIVE_SERIAL_WAIT);
  return _oDserial->readFloat();
}  

// read current requested state
int ODriveExt::getODriveRequestedState() {
  ODRIVE_SERIAL << "r axis0.requested_state\n";
  tasks.yield(ODRIVE_SERIAL_WAIT);
  return _oDserial->readInt();
}

float ODriveExt::getMotorPositionDelta(int axis) {
  ODRIVE_SERIAL << "r axis" << axis << ".controller.pos_setpoint\n";
  tasks.yield(ODRIVE_SERIAL_WAIT);
  float reqPos = _oDserial->readFloat();   
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n";
  tasks.yield(ODRIVE_SERIAL_WAIT);
  float posEst = _oDserial->readFloat();   
  float deltaPos = abs(reqPos - posEst);
  return deltaPos;
}

// Check encoders to see if positions are too far outside range of requested position inferring that there are interfering forces
// This will warn that the motors may be getting too hot since more current is required trying to move them to the requested position
// Beeping occurs at higher frequency as position delta from target increases
void ODriveExt::MotorEncoderDelta() {
  if (odriveAzmPwr) {
    float AZposDelta = getMotorPositionDelta(AZM_MOTOR);
    if (AZposDelta > 0.002 && AZposDelta < 0.03) display.soundFreq(AZposDelta * 50000, 20);
    else if (AZposDelta > 0.03) display.soundFreq(3000, 20); // saturated
  }
  
  if (oDriveExt.odriveAltPwr) {
    float ALTposDelta = getMotorPositionDelta(ALT_MOTOR);
    if (ALTposDelta > .002 && ALTposDelta < 0.03) {
      display.soundFreq(ALTposDelta * 50000, 40);
      delay(1);
      display.soundFreq(ALTposDelta * 40000, 40); // double beep to distinguish ALT from AZM
    }
    else if (ALTposDelta > 0.03) display.soundFreq(3000, 40); 
  }
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
  tasks.yield(ODRIVE_SERIAL_WAIT);
  return _oDserial->readFloat();
}

float ODriveExt::getODriveVelIntGain(int axis) {
  ODRIVE_SERIAL << "r axis"<<axis<<".controller.config.vel_integrator_gain\n";
  tasks.yield(ODRIVE_SERIAL_WAIT);
  return _oDserial->readFloat();
}

float ODriveExt::getODrivePosGain(int axis) {
  ODRIVE_SERIAL << "r axis"<<axis<<".controller.config.pos_gain\n";
  tasks.yield(ODRIVE_SERIAL_WAIT);
  return _oDserial->readFloat();
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
    tasks.yield(ODRIVE_SERIAL_WAIT);
    return _oDserial->readInt();
  }

  if (component == NONE) {
    ODRIVE_SERIAL << "r odrive.axis"<<axis<<".error\n";
    tasks.yield(ODRIVE_SERIAL_WAIT);
    return _oDserial->readInt();
  } else {
    ODRIVE_SERIAL << "r odrive.axis"<<axis<<"."<<ODriveComponentsStr[component]<<".error\n";
    tasks.yield(ODRIVE_SERIAL_WAIT);
    return _oDserial->readInt();
  }
}

void ODriveExt::getODriveVersion(ODriveVersion oDversion) {
  ODRIVE_SERIAL << "r hw_version_major\n"; 
  tasks.yield(ODRIVE_SERIAL_WAIT);
  oDversion.hwMajor = _oDserial->readInt();
  ODRIVE_SERIAL << "r hw_version_minor\n"; 
  tasks.yield(ODRIVE_SERIAL_WAIT);
  oDversion.hwMinor = _oDserial->readInt();
  ODRIVE_SERIAL << "r hw_version_variant\n"; 
  tasks.yield(ODRIVE_SERIAL_WAIT);
  oDversion.hwVar = _oDserial->readInt();
  ODRIVE_SERIAL << "r fw_version_major\n"; 
  tasks.yield(ODRIVE_SERIAL_WAIT);
  oDversion.fwMajor = _oDserial->readInt();
  ODRIVE_SERIAL << "r fw_version_minor\n"; 
  tasks.yield(ODRIVE_SERIAL_WAIT);
  oDversion.fwMinor = _oDserial->readInt();
  ODRIVE_SERIAL << "r fw_version_revision\n"; 
  tasks.yield(ODRIVE_SERIAL_WAIT);
  oDversion.fwRev = _oDserial->readInt();
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
        _oDserial->SetPosition(ALT_MOTOR, pos_one);
        _oDserial->SetPosition(AZM_MOTOR, pos_one);
        ++demo_pos;
        break;
      case 1:
        _oDserial->SetPosition(ALT_MOTOR, pos_two);
        _oDserial->SetPosition(AZM_MOTOR, pos_two);
        ++demo_pos;
        break;
      case 2:
        _oDserial->SetPosition(ALT_MOTOR, pos_three);
        _oDserial->SetPosition(AZM_MOTOR, pos_one);
        ++demo_pos;
        break;
      case 3:
        _oDserial->SetPosition(ALT_MOTOR, pos_four);
        _oDserial->SetPosition(AZM_MOTOR, pos_five);
        demo_pos = 0;
        break;
      default:
        _oDserial->SetPosition(ALT_MOTOR, pos_one);
        _oDserial->SetPosition(AZM_MOTOR, pos_one);
        demo_pos = 0;
        break;
    }
  } else { // off
    _oDserial->SetPosition(ALT_MOTOR, 0);
    _oDserial->SetPosition(AZM_MOTOR, 0);
    axis1.init(&motor1); // start motor timers
    axis2.init(&motor2);
  }
}

ODriveExt oDriveExt;