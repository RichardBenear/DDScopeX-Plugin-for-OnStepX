// ============================================
// ODriveExt.cpp
//
// ODrive Extended Support functions
// Author: Richard Benear 2022
// 
// ODrive communication via Teensy 4.0 serial
// Uses GitHub ODrive Arduino library

#include "ODriveExt.h"
#include "src/lib/axis/motor/oDrive/ODrive.h"
#include "../../../telescope/mount/Mount.h"
#include "../../../lib/tasks/OnTask.h"

const char* ODriveComponentsStr[4] = {
            "None",   
            "Controller",
            "Motor",
            "Encoder"        
};

// ==============================================================================
// NOTE: A change to the HardwareSerial.cpp library was made.
// In HardwareSerial.cpp, this line was changed: PUS(3) was changed to PUS(2)
// PUS() is Pullup Strength for the Teensy RX PAD. 3 = 22K ohm, 2 = 100K ohm
// When set to 22K ohm, the ODrive TX signal would only have a low of .4 volt above ground.
// When set to 100K ohm, the ODrive TX signal would be about 20mv above ground.
// Apparently, the ODrive TX drive strength isn't very high
// But, it's not clear if this has any functional benefit; maybe better noise immunity.
//	*(portControlRegister(hardware->rx_pins[rx_pin_index_].pin)) = IOMUXC_PAD_DSE(7) | IOMUXC_PAD_PKE | IOMUXC_PAD_PUE | IOMUXC_PAD_PUS(2) | IOMUXC_PAD_HYS;
//===============================================================================

// ================ ODrive "writes" ======================
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

// Set ODrive Gains
void ODriveExt::setODriveVelGain(int axis, float level) {
  ODRIVE_SERIAL << "w axis"<<axis<<".controller.config.vel_gain "<<level<<'\n';
}

void ODriveExt::setODriveVelIntGain(int axis, float level) {
  ODRIVE_SERIAL << "w axis"<<axis<<".controller.config.vel_integrator_gain "<<level<<'\n';
}

void ODriveExt::setODrivePosGain(int axis, float level) {
  ODRIVE_SERIAL << "w axis"<<axis<<".controller.config.pos_gain "<<level<<'\n';
}

// ================ ODrive "reads" ======================
// NOTE: Since the ODriveArduino library has up to 1000ms timeout waiting for a RX character,
// a tasks.yield() is added after every read command.
// NOTE: if the RX data from ODrive drops out or the ODrive is off during debug, then just return
// from any of the follow "read" routines.

// Read ODrive bus voltage which is approx. the battery voltage
// Battery Low LED is only on when battery is below low threashold
float ODriveExt::getODriveBusVoltage() {
  ODRIVE_SERIAL << "r vbus_voltage\n";
  float battery_voltage = _oDriveDriver->readFloat();
  if (battery_voltage <= 0.20F) {
    oDriveRXoff = true; 
    return battery_voltage;
  } else { 
    oDriveRXoff = false;
  }
  
  // 3 volt qualification keeps ALERT from happening when under development without ODrive powered
  if (battery_voltage < BATTERY_LOW_VOLTAGE && battery_voltage > 3) { 
    digitalWrite(BATTERY_LOW_LED_PIN, LOW); // LED on
    batLowLED = true;
    ALERT;
  } else { // battery ok
    digitalWrite(BATTERY_LOW_LED_PIN, HIGH); // LED off
    batLowLED = false;
  }
  return battery_voltage;
}

// get absolute Encoder positions in degrees
float ODriveExt::getEncoderPositionDeg(int axis) {
  if (oDriveRXoff) return -1.0;
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n"; Y;
  float turns = _oDriveDriver->readFloat();
  return turns*360;
}  

// get motor positions in turns
float ODriveExt::getMotorPositionTurns(int axis) {
  if (oDriveRXoff) return -1.0;
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n"; Y;
  return _oDriveDriver->readFloat();
}  

// get motor position in counts
int ODriveExt::getMotorPositionCounts(int axis) {
  if (oDriveRXoff) return -1.0;
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate_counts\n"; Y;
  return _oDriveDriver->readInt();
} 

// get Motor Current
float ODriveExt::getMotorCurrent(int axis) {
  if (oDriveRXoff) return -1.0;
  ODRIVE_SERIAL << "r axis" << axis << ".motor.I_bus\n"; Y;
  return _oDriveDriver->readFloat();
}  

// read current state
int ODriveExt::getODriveCurrentState(int axis) {
  if (oDriveRXoff) return -1.0;
  ODRIVE_SERIAL << "r axis" << axis << ".current_state\n";
  return _oDriveDriver->readInt();
}

// Get the difference between ODrive setpoint and the encoder
float ODriveExt::getMotorPositionDelta(int axis) {
  if (oDriveRXoff) return -1.0;
  ODRIVE_SERIAL << "r axis" << axis << ".controller.pos_setpoint\n";
  float reqPos = _oDriveDriver->readFloat();   
  ODRIVE_SERIAL << "r axis" << axis << ".encoder.pos_estimate\n";
  float posEst = _oDriveDriver->readFloat();   
  float deltaPos = abs(reqPos - posEst);
  return deltaPos;
}

// Check encoders to see if positions are too far outside range of requested position inferring that there are interfering forces
// This will warn that the motors may be getting too hot since more current is required trying to move them to the requested position
// Beeping occurs at higher frequency as position delta from target increases
void ODriveExt::MotorEncoderDelta() {
  if (!oDriveRXoff) return;
  if (axis1.isEnabled()) {
    float AZposDelta = getMotorPositionDelta(AZM_MOTOR);
    if (AZposDelta > 0.002 && AZposDelta < 0.03) soundFreq(AZposDelta * 50000, 20);
    else if (AZposDelta > 0.03) soundFreq(3000, 20); // saturated
  }
  
  if (axis2.isEnabled()) {
    float ALTposDelta = getMotorPositionDelta(ALT_MOTOR); Y;
    if (ALTposDelta > .002 && ALTposDelta < 0.03) {
      soundFreq(ALTposDelta * 50000, 40);
    
      soundFreq(ALTposDelta * 40000, 40); // double beep to distinguish ALT from AZM
    }
    else if (ALTposDelta > 0.03) soundFreq(3000, 40); 
  }
}

// Get ODrive gains
float ODriveExt::getODriveVelGain(int axis) {
  if (oDriveRXoff) return -1.0;
  ODRIVE_SERIAL << "r axis"<<axis<<".controller.config.vel_gain\n"; Y;
  return _oDriveDriver->readFloat();
}

float ODriveExt::getODriveVelIntGain(int axis) {
  if (oDriveRXoff) return -1.0;
  ODRIVE_SERIAL << "r axis"<<axis<<".controller.config.vel_integrator_gain\n"; Y;
  return _oDriveDriver->readFloat();
}

float ODriveExt::getODrivePosGain(int axis) {
  if (oDriveRXoff) return -1.0;
  ODRIVE_SERIAL << "r axis"<<axis<<".controller.config.pos_gain\n"; Y;
  return _oDriveDriver->readFloat();
}

// ========  Get the ODRIVE errors ========
uint32_t ODriveExt::getODriveErrors(int axis, Component component) {
  if (oDriveRXoff) return -1.0;
  if (axis == -1) { // ODrive top level error
    ODRIVE_SERIAL << "r odrive.error\n";
    return (uint32_t)_oDriveDriver->readInt(); Y;
  }

  if (component == NONE) {
    ODRIVE_SERIAL << "r odrive.axis"<<axis<<".error\n"; Y;
    return (uint32_t)_oDriveDriver->readInt();
  } else {
    ODRIVE_SERIAL << "r odrive.axis"<<axis<<"."<<ODriveComponentsStr[component]<<".error\n"; Y;
    return (uint32_t)_oDriveDriver->readInt();
  }
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
// Demo mode requires that the OnStep updates to the Axis be stopped but the Motor power is ON
// Demo mode repeats a sequence of moves
void ODriveExt::demoMode() {
  // choosing some AZM and ALT positions in fractional "Turns"
  // ALT position should never be negative in actual use because of the position of the focuser but it "can" go negative in demo
 
  float pos_one = 0.15;
  float pos_two = 0.3;
  float pos_three = -0.1;
  float pos_four = -0.4;
  float pos_five = 0.4;
  int demo_pos = 0;

  switch(demo_pos) {
    case 0:
      _oDriveDriver->SetPosition(ALT_MOTOR, pos_one);
      _oDriveDriver->SetPosition(AZM_MOTOR, pos_one);
      ++demo_pos;
      break;
    case 1:
      _oDriveDriver->SetPosition(ALT_MOTOR, pos_two);
      _oDriveDriver->SetPosition(AZM_MOTOR, pos_two);
      ++demo_pos;
      break;
    case 2:
      _oDriveDriver->SetPosition(ALT_MOTOR, pos_three);
      _oDriveDriver->SetPosition(AZM_MOTOR, pos_one);
      ++demo_pos;
      break;
    case 3:
      _oDriveDriver->SetPosition(ALT_MOTOR, pos_four);
      _oDriveDriver->SetPosition(AZM_MOTOR, pos_five);
      demo_pos = 0;
      break;
    default:
      _oDriveDriver->SetPosition(ALT_MOTOR, pos_one);
      _oDriveDriver->SetPosition(AZM_MOTOR, pos_one);
      demo_pos = 0;
      break;
  }
}

ODriveExt oDriveExt;