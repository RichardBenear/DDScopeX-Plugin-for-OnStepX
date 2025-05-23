// -----------------------------------------------------------------------------------
// axis step/dir motor driver

// note: LegacyUART requires UART RX but TX is optional

#include "LegacyUART.h"

#if !defined(DRIVER_TMC_STEPPER) && defined(STEP_DIR_TMC_UART_PRESENT)

// help with pin names
#define rx m2
#define tx m3

// constructor
StepDirTmcUART::StepDirTmcUART(uint8_t axisNumber, const StepDirDriverPins *Pins, const StepDirDriverSettings *Settings) {
  this->axisNumber = axisNumber;
  this->Pins = Pins;
  settings = *Settings;
}

// set up driver and parameters: microsteps, microsteps goto, hold current, run current, goto current, unused
void StepDirTmcUART::init(float param1, float param2, float param3, float param4, float param5, float param6) {
  StepDirDriver::init(param1, param2, param3, param4, param5, param6);

  if (settings.currentRun != OFF) {
    // automatically set goto and hold current if they are disabled
    if (settings.currentGoto == OFF) settings.currentGoto = settings.currentRun;
    if (settings.currentHold == OFF) settings.currentHold = lround(settings.currentRun/2.0F);
  } else {
    // set current defaults for TMC drivers
    settings.currentRun = 600;
    if (settings.model == TMC2130) settings.currentRun = 2500;
    settings.currentGoto = settings.currentRun;
    settings.currentHold = lround(settings.currentRun/2.0F);
  }

  if (settings.decay == OFF) settings.decay = STEALTHCHOP;
  if (settings.decaySlewing == OFF) settings.decaySlewing = SPREADCYCLE;

  VF("MSG: StepDirDriver"); V(axisNumber); VF(", TMC ");
  if (settings.currentRun == OFF) {
    VLF("current control OFF (set by Vref)");
  } else {
    VF("Ihold="); V(settings.currentHold); VF("mA, ");
    VF("Irun="); V(settings.currentRun); VF("mA, ");
    VF("Igoto="); V(settings.currentGoto); VL("mA");
  }

  // get TMC UART driver ready
  pinModeEx(Pins->m0, OUTPUT);
  pinModeEx(Pins->m1, OUTPUT);

  driver = new TMC2209Stepper();
  if (driver == NULL) return; 

  int16_t rxPin = Pins->rx;

  delay(1);
  #if defined(SERIAL_TMC_HARDWARE_UART)
    // help user hard code the device addresses 0,1,2,3
    digitalWriteEx(Pins->m0, HIGH);
    digitalWriteEx(Pins->m1, HIGH);

    VF("MSG: StepDirDriver"); V(axisNumber); VF(", TMC ");
    VF("HW UART driver pins rx="); V(SERIAL_TMC_RX); VF(", tx="); V(SERIAL_TMC_TX); VF(", baud="); V(SERIAL_TMC_BAUD); VLF("bps");
    #if SERIAL_TMC_INVERT == ON
      driver->setup(SERIAL_TMC, SERIAL_TMC_BAUD, SERIAL_TMC_ADDRESS_MAP(axisNumber - 1), SERIAL_TMC_RX, SERIAL_TMC_TX, true);
    #else
      driver->setup(SERIAL_TMC, SERIAL_TMC_BAUD, SERIAL_TMC_ADDRESS_MAP(axisNumber - 1), SERIAL_TMC_RX, SERIAL_TMC_TX);
    #endif
  #else
    // pull MS1 and MS2 low for device address 0
    digitalWriteEx(Pins->m0, LOW);
    digitalWriteEx(Pins->m1, LOW);

    #if SERIAL_TMC_RX_DISABLE == true
      rxPin = OFF;
    #endif
    VF("MSG: StepDirDriver"); V(axisNumber); VF(", TMC ");
    VF("SW UART driver pins rx="); V(rxPin); VF(", tx="); V(Pins->tx); VF(", baud="); V(SERIAL_TMC_BAUD); VLF("bps");
    driver->setup(SERIAL_TMC_BAUD, SERIAL_TMC_ADDRESS_MAP(axisNumber - 1), rxPin, Pins->tx);
  #endif

  if (rxPin != OFF) {
    if (driver->isSetupAndCommunicating()) {
      VF("MSG: StepDirDriver"); V(axisNumber); VLF(", TMC UART driver found");
    } else {
      VF("WRN: StepDirDriver"); V(axisNumber); VLF(", TMC UART driver detection failed");
    }
  }

  driver->enable();
  driver->moveUsingStepDirInterface();
  driver->setPwmOffset(pc_pwm_ofs);
  driver->setPwmGradient(pc_pwm_grad);
  if (pc_pwm_auto) driver->enableAutomaticCurrentScaling();

  // calibrate stealthChop
  modeMicrostepTracking();
  if (settings.decay == STEALTHCHOP || settings.decaySlewing == STEALTHCHOP) {
    driver->setRunCurrent(settings.currentRun/25); // current in %
    driver->setHoldCurrent(settings.currentRun/25); // current in %
    driver->enableStealthChop();
    VF("MSG: StepDirDriver"); V(axisNumber); VL(", TMC standstill automatic current calibration");
    delay(100);
  }
  driver->setRunCurrent(settings.currentRun/25); // current in %
  driver->setHoldCurrent(settings.currentRun/25); // current in %
  if (settings.decay == SPREADCYCLE) driver->disableStealthChop(); else driver->enableStealthChop();

  // automatically set fault status for known drivers
  status.active = settings.status != OFF;

  // set fault pin mode
  if (settings.status == LOW) pinModeEx(Pins->fault, INPUT_PULLUP);
  #ifdef PULLDOWN
    if (settings.status == HIGH) pinModeEx(Pins->fault, INPUT_PULLDOWN);
  #else
    if (settings.status == HIGH) pinModeEx(Pins->fault, INPUT);
  #endif

  // set mode switching support flags
  // use low speed mode switch for TMC drivers or high speed otherwise
  modeSwitchAllowed = microstepRatio != 1;
  modeSwitchFastAllowed = false;
}

// validate driver parameters
bool StepDirTmcUART::validateParameters(float param1, float param2, float param3, float param4, float param5, float param6) {
  StepDirDriver::validateParameters(param1, param2, param3, param4, param5, param6);

  int maxCurrent;
  if (settings.model == TMC2209) maxCurrent = 2000; else
  {
    DF("ERR: StepDirDriver::validateParameters(), Axis"); D(axisNumber); DLF(" unknown driver model!");
    return false;
  }

  long currentHold = round(param3);
  long currentRun = round(param4);
  long currentGoto = round(param5);
  UNUSED(param6);

  if (currentHold != OFF && (currentHold < 0 || currentHold > maxCurrent)) {
    DF("ERR: StepDirDriver::validateParameters(), Axis"); D(axisNumber); DF(" bad current hold="); DL(currentHold);
    return false;
  }

  if (currentRun != OFF && (currentRun < 0 || currentRun > maxCurrent)) {
    DF("ERR: StepDirDriver::validateParameters(), Axis"); D(axisNumber); DF(" bad current run="); DL(currentRun);
    return false;
  }

  if (currentGoto != OFF && (currentGoto < 0 || currentGoto > maxCurrent)) {
    DF("ERR: StepDirDriver::validateParameters(), Axis"); D(axisNumber); DF(" bad current goto="); DL(currentGoto);
    return false;
  }

  return true;
}

void StepDirTmcUART::modeMicrostepTracking() {
  driver->setMicrostepsPerStep(settings.microsteps);
}

int StepDirTmcUART::modeMicrostepSlewing() {
  if (microstepRatio > 1) {
    driver->setMicrostepsPerStep(settings.microstepsSlewing);
  }
  return microstepRatio;
}

void StepDirTmcUART::modeDecayTracking() {
  if (settings.decay == SPREADCYCLE) driver->disableStealthChop(); else driver->enableStealthChop();
  driver->setRunCurrent(settings.currentRun/25); // current in %
}

void StepDirTmcUART::modeDecaySlewing() {
  int IGOTO = settings.currentGoto;
  if (IGOTO == OFF) IGOTO = settings.currentRun;
  if (settings.decaySlewing == SPREADCYCLE) driver->disableStealthChop(); else driver->enableStealthChop();
  driver->setRunCurrent(IGOTO/25); // current in %
}

void StepDirTmcUART::updateStatus() {
  if (settings.status == ON) {
    if ((long)(millis() - timeLastStatusUpdate) > 200) {
      TMC2209Stepper::Status tmc2209Status = driver->getStatus();
      status.outputA.shortToGround = (bool)tmc2209Status.short_to_ground_a || (bool)tmc2209Status.low_side_short_a;
      status.outputA.openLoad      = (bool)tmc2209Status.open_load_a;
      status.outputB.shortToGround = (bool)tmc2209Status.short_to_ground_b || (bool)tmc2209Status.low_side_short_b;
      status.outputB.openLoad      = (bool)tmc2209Status.open_load_b;
      status.overTemperatureWarning = (bool)tmc2209Status.over_temperature_warning;
      status.standstill            = (bool)tmc2209Status.standstill;

      // open load indication is not reliable in standstill
      if (
        status.outputA.shortToGround ||
        status.outputB.shortToGround ||
        status.overTemperatureWarning ||
        status.overTemperature
      ) status.fault = true; else status.fault = false;

      timeLastStatusUpdate = millis();
    }
  } else
  if (settings.status == LOW || settings.status == HIGH) {
    status.fault = digitalReadEx(Pins->fault) == settings.status;
  }

  StepDirDriver::updateStatus();
}

// secondary way to power down not using the enable pin
void StepDirTmcUART::enable(bool state) {
  VF("MSG: StepDirDriver"); V(axisNumber);
  VF(", powered "); if (state) { VF("up"); } else { VF("down"); } VLF(" using SPI or UART");
  int I_run = 0, I_hold = 0;
  if (state) { I_run = settings.currentRun; I_hold = settings.currentHold; }
  driver->setRunCurrent(I_run/25); // current in %
  driver->setHoldCurrent(I_hold/25); // current in %
}

#endif
