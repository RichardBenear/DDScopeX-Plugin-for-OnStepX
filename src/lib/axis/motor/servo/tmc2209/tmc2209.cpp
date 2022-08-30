// -----------------------------------------------------------------------------------
// axis servo DC motor driver

#include "tmc2209.h"

#ifdef SERVO_TMC2209_PRESENT

#include "../../../../tasks/OnTask.h"

// help with pin names
#define rx m2
#define tx m3

ServoTmc2209::ServoTmc2209(uint8_t axisNumber, const ServoTmcPins *Pins, const ServoTmcSettings *TmcSettings) {
  this->axisNumber = axisNumber;

  this->Pins = Pins;
  enablePin = Pins->enable;
  enabledState = Pins->enabledState;
  faultPin = Pins->fault;

  this->Settings = TmcSettings;
  model = TmcSettings->model;
  statusMode = TmcSettings->status;
  motorPwrMax = TmcSettings->velocityMax;
}

void ServoTmc2209::init() {
  ServoDriver::init();

  // automatically set fault status for known drivers
  status.active = statusMode != OFF;

  // get TMC UART driver ready
  pinModeEx(Pins->m0, OUTPUT);
  pinModeEx(Pins->m1, OUTPUT);
  pinModeEx(Pins->step, OUTPUT);
  pinModeEx(Pins->dir, OUTPUT);

  // set S/D motion pins to known, fixed state
  digitalWriteEx(Pins->step, LOW);
  digitalWriteEx(Pins->dir, LOW);

  // initialize the serial port
  VF("MSG: ServoDriver"); V(axisNumber); VF(", TMC ");
  #if defined(SERIAL_TMC_HARDWARE_UART)
    // help user hard code the device addresses 0,1,2,3
    digitalWriteEx(Pins->m0, HIGH);
    digitalWriteEx(Pins->m1, HIGH);
    #define SerialTMC SERIAL_TMC
    static bool initialized = false;
    if (!initialized) {
      #if defined(SERIAL_TMC_RX) && defined(SERIAL_TMC_TX) && !defined(SERIAL_TMC_RXTX_SET)
        VF("HW UART driver pins rx="); V(SERIAL_TMC_RX); VF(", tx="); V(SERIAL_TMC_TX); VF(", baud="); V(SERIAL_TMC_BAUD); VLF("bps");
        SerialTMC.begin(SERIAL_TMC_BAUD, SERIAL_8N1, SERIAL_TMC_RX, SERIAL_TMC_TX);
      #else
        VF("HW UART driver pins on port default"); VF(", baud="); V(SERIAL_TMC_BAUD); VLF("bps");
        SerialTMC.begin(SERIAL_TMC_BAUD);
      #endif
      initialized = true;
    }
  #else
    // pull MS1 and MS2 low for device address 0
    digitalWriteEx(Pins->m0, LOW);
    digitalWriteEx(Pins->m1, LOW);
    VF("SW UART driver pins rx="); V(Pins->rx); VF(", tx="); V(Pins->tx); VF(", baud="); V(SERIAL_TMC_BAUD); VLF("bps");
    SerialTMC = new SoftwareSerial(Pins->rx, Pins->tx);
    SerialTMC.begin(SERIAL_TMC_BAUD);
  #endif

  driver = new TMC2209Stepper(&SERIAL_TMC, 0.11F, SERIAL_TMC_ADDRESS_MAP(axisNumber - 1));
  driver->begin();
  driver->intpol(false);

  VF("MSG: ServoDriver"); V(axisNumber); VF(", TMC ");
  if (Settings->current == OFF) {
    VLF("current control OFF (set by Vref)");
    driver->rms_current(2.5F*0.707F);
  } else {
    VF("Irun="); V(Settings->current); VLF("mA");
    driver->rms_current(Settings->current*0.707F);
  }
  driver->hold_multiplier(1.0F);

  // automatically set fault status for known drivers
  status.active = statusMode != OFF;

  // set fault pin mode
  if (statusMode == LOW) pinModeEx(faultPin, INPUT_PULLUP);
  #ifdef PULLDOWN
    if (statusMode == HIGH) pinModeEx(faultPin, INPUT_PULLDOWN);
  #else
    if (statusMode == HIGH) pinModeEx(faultPin, INPUT);
  #endif
}

// power level to the motor (in microsteps/s)
void ServoTmc2209::setMotorPower(float power) {
  if (!enabled) motorPwr = 0.0F; else motorPwr = power;
  if (motorPwr > motorPwrMax) motorPwr = motorPwrMax; else
  if (motorPwr < -motorPwrMax) motorPwr = -motorPwrMax;
  driver->VACTUAL(round((motorPwr/0.715F)*4.0F));
}

// update status info. for driver
void ServoTmc2209::updateStatus() {
  if (statusMode == ON) {
    if ((long)(millis() - timeLastStatusUpdate) > 200) {

      uint32_t status_word;
      TMC2208_n::DRV_STATUS_t status_result;
      status_result.sr = ((TMC2209Stepper*)driver)->DRV_STATUS();
      status.outputA.shortToGround = status_result.s2ga;
      status.outputA.openLoad      = status_result.ola;
      status.outputB.shortToGround = status_result.s2gb;
      status.outputB.openLoad      = status_result.olb;
      status.overTemperatureWarning = status_result.otpw;
      status.overTemperature       = status_result.ot;
      status.standstill            = status_result.stst;

      // open load indication is not reliable in standstill
      if (status.outputA.shortToGround ||
          status.outputB.shortToGround ||
          status.overTemperatureWarning ||
          status.overTemperature) status.fault = true; else status.fault = false;

      timeLastStatusUpdate = millis();
    }
  } else
  if (statusMode == LOW || statusMode == HIGH) {
    status.fault = digitalReadEx(Pins->fault) == statusMode;
  }
}

// secondary way to power down not using the enable pin
void ServoTmc2209::enable(bool state) {
  enabled = state;
  if (enablePin == SHARED) {
    VF("MSG: ServoDriver"); V(axisNumber);
    VF(", powered "); if (state) { VF("up"); } else { VF("down"); } VLF(" using UART");
    int I_run = 0;
    if (state) { I_run = Settings->current; }
    driver->rms_current(I_run*0.707F);
  } else {
    if (!enabled) { digitalWriteF(enablePin, !enabledState); } else { digitalWriteF(enablePin, enabledState); }
  }

  ServoDriver::updateStatus();
}

#endif
