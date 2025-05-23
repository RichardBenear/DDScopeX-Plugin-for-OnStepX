// -----------------------------------------------------------------------------------
// axis step/dir motor driver

// note: StepperSPI requires MOSI, SCK, CS, and MISO

#include "StepperSPI.h"
 
#if defined(DRIVER_TMC_STEPPER) && defined(STEP_DIR_TMC_SPI_PRESENT)

// help with pin names
#define mosi m0
#define sck  m1
#define cs   m2
#define miso m3

// constructor
StepDirTmcSPI::StepDirTmcSPI(uint8_t axisNumber, const StepDirDriverPins *Pins, const StepDirDriverSettings *Settings) {
  this->axisNumber = axisNumber;
  this->Pins = Pins;
  settings = *Settings;
}

// sets driver parameters: microsteps, microsteps goto, hold current, run current, goto current, unused
void StepDirTmcSPI::init(float param1, float param2, float param3, float param4, float param5, float param6) {
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

  if (settings.model == TMC2130) {
    driver = new TMC2130Stepper(Pins->cs, Pins->mosi, Pins->miso, Pins->sck);
    ((TMC2130Stepper*)driver)->begin();
    ((TMC2130Stepper*)driver)->intpol(true);
  } else
  if (settings.model == TMC5160) {
    driver = new TMC5160Stepper(Pins->cs, Pins->mosi, Pins->miso, Pins->sck);
    ((TMC5160Stepper*)driver)->begin();
    ((TMC5160Stepper*)driver)->intpol(true);
  } else
  if (settings.model == TMC5161) {
    driver = new TMC5161Stepper(Pins->cs, Pins->mosi, Pins->miso, Pins->sck);
    ((TMC5161Stepper*)driver)->begin();
    ((TMC5161Stepper*)driver)->intpol(true);
  }
  
  // calibrate stealthChop
  modeMicrostepTracking();
  if (settings.decay == STEALTHCHOP || settings.decaySlewing == STEALTHCHOP) {
    driver->rms_current(settings.currentRun*0.707F);
    driver->hold_multiplier(1.0F);
    setDecayMode(STEALTHCHOP);
    VF("MSG: StepDirDriver"); V(axisNumber); VL(", TMC standstill automatic current calibration");
    delay(100);
  }
  driver->rms_current(settings.currentRun*0.707F);
  driver->hold_multiplier(settings.currentHold/settings.currentRun);
  setDecayMode(settings.decay);

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
bool StepDirTmcSPI::validateParameters(float param1, float param2, float param3, float param4, float param5, float param6) {
  StepDirDriver::validateParameters(param1, param2, param3, param4, param5, param6);

  int maxCurrent;
  if (settings.model == TMC2130) maxCurrent = 1500; else
  if (settings.model == TMC5160) maxCurrent = 3000; else
  if (settings.model == TMC5161) maxCurrent = 3500; else
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

void StepDirTmcSPI::modeMicrostepTracking() {
  driver->microsteps(settings.microsteps);
}

int StepDirTmcSPI::modeMicrostepSlewing() {
  if (microstepRatio > 1) {
    driver->microsteps(settings.microstepsSlewing);
  }
  return microstepRatio;
}

void StepDirTmcSPI::modeDecayTracking() {
  setDecayMode(settings.decay);
  driver->rms_current(settings.currentRun*0.707F);
}

void StepDirTmcSPI::modeDecaySlewing() {
  setDecayMode(settings.decaySlewing);
  int IGOTO = settings.currentGoto;
  if (IGOTO == OFF) IGOTO = settings.currentRun;
  driver->rms_current(IGOTO*0.707F);
}

// set the decay mode STEALTHCHOP or SPREADCYCLE
void StepDirTmcSPI::setDecayMode(int decayMode) {
  if (settings.model == TMC2130) { ((TMC2130Stepper*)driver)->en_pwm_mode(decayMode != SPREADCYCLE); } else
  if (settings.model == TMC5160) { ((TMC5160Stepper*)driver)->en_pwm_mode(decayMode != SPREADCYCLE); } else
  if (settings.model == TMC5161) { ((TMC5161Stepper*)driver)->en_pwm_mode(decayMode != SPREADCYCLE); }
}

void StepDirTmcSPI::updateStatus() {
  if (settings.status == ON) {
    if ((long)(millis() - timeLastStatusUpdate) > 200) {
      uint32_t status_word;
      TMC2130_n::DRV_STATUS_t status_result;
      if (settings.model == TMC2130) { status_result.sr = ((TMC2130Stepper*)driver)->DRV_STATUS(); } else
      if (settings.model == TMC5160) { status_result.sr = ((TMC5160Stepper*)driver)->DRV_STATUS(); } else
      if (settings.model == TMC5161) { status_result.sr = ((TMC5161Stepper*)driver)->DRV_STATUS(); }
      status.outputA.shortToGround = status_result.s2ga;
      status.outputA.openLoad      = status_result.ola;
      status.outputB.shortToGround = status_result.s2gb;
      status.outputB.openLoad      = status_result.olb;
      status.overTemperatureWarning= status_result.otpw;
      status.overTemperature       = status_result.ot;
      status.standstill            = status_result.stst;

      // open load indication is not reliable in standstill
      if (status.outputA.shortToGround || status.outputB.shortToGround ||
          status.overTemperatureWarning || status.overTemperature) status.fault = true; else status.fault = false;

      timeLastStatusUpdate = millis();
    }
  } else
  if (settings.status == LOW || settings.status == HIGH) {
    status.fault = digitalReadEx(Pins->fault) == settings.status;
  }

  StepDirDriver::updateStatus();
}

// secondary way to power down not using the enable pin
void StepDirTmcSPI::enable(bool state) {
  VF("MSG: StepDirDriver"); V(axisNumber);
  VF(", powered "); if (state) { VF("up"); } else { VF("down"); } VLF(" using SPI or UART");
  int I_run = 0, I_hold = 0;
  if (state) { I_run = settings.currentRun; I_hold = settings.currentHold; }
  driver->rms_current(I_run*0.707F);
}

#endif
