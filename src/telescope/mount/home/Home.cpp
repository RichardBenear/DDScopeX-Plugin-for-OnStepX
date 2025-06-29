//--------------------------------------------------------------------------------------------------
// telescope mount control, homing

#include "Home.h"

#ifdef MOUNT_PRESENT

#include "../Mount.h"
#include "../goto/Goto.h"
#include "../guide/Guide.h"
#include "../../../lib/tasks/OnTask.h"

// init the home position (according to settings and mount type)
void Home::init() {
  #ifndef AXIS1_HOME_DEFAULT
    if (transform.mountType == GEM) position.h = Deg90; else { position.h = 0; position.z = 0; }
  #else
    if (transform.mountType == ALTAZM) position.z = degToRad(AXIS1_HOME_DEFAULT); else position.h = degToRad(AXIS1_HOME_DEFAULT);
  #endif
  #ifndef AXIS2_HOME_DEFAULT
    if (transform.mountType == ALTAZM) position.a = 0.0; else position.d = site.locationEx.latitude.sign*Deg90;
  #else
    if (transform.mountType == ALTAZM) position.a = degToRad(AXIS2_HOME_DEFAULT); else position.d = degToRad(AXIS2_HOME_DEFAULT);
  #endif
  position.pierSide = PIER_SIDE_NONE;
}

// move mount to the home position
CommandError Home::request() {
  #if GOTO_FEATURE == ON
    if (goTo.state != GS_NONE) return CE_SLEW_IN_MOTION;
    if (guide.state != GU_NONE) {
      if (guide.state == GU_HOME_GUIDE) guide.stop();
      return CE_SLEW_IN_MOTION;
    }

    if ((AXIS1_SENSE_HOME) != OFF && (AXIS2_SENSE_HOME) != OFF) {
      CommandError e = reset();
      if (e != CE_NONE) return e;
    }

    #if AXIS2_TANGENT_ARM == OFF
      // stop tracking
      wasTracking = mount.isTracking();
      mount.tracking(false);
    #endif

    // make sure the motors are powered on
    mount.enable(true);
    goTo.firstGoto = false;

    VLF("MSG: Mount, moving to home");

    if (((AXIS1_SENSE_HOME) != OFF && (AXIS2_SENSE_HOME) != OFF) ||
         (AXIS2_TANGENT_ARM != OFF && (AXIS2_SENSE_HOME) != OFF)) {
      isRequestWithReset = false;
      guide.startHome();
    } else {
      #if AXIS2_TANGENT_ARM == OFF
        state = HS_HOMING;
        if (transform.mountType == ALTAZM) transform.horToEqu(&position);
        return goTo.request(&position, PSS_EAST_ONLY, false);
      #else
        axis2.setFrequencySlew(goTo.rate);
        axis2.setTargetCoordinate(axis2.getIndexPosition());
        VLF("Mount, axis2 home target coordinates set");
        axis2.autoGoto(degToRadF((float)(SLEW_ACCELERATION_DIST)));
      #endif
    }
  #endif
  return CE_NONE;
}

// reset mount, moves to the home position first if home switches are present
CommandError Home::requestWithReset() {
  if ((AXIS1_SENSE_HOME) != OFF && (AXIS2_SENSE_HOME) != OFF) {
    CommandError result = request();
    isRequestWithReset = true;
    return result;
  } else return reset();
}

// clear home state on abort
void Home::requestAborted() {
  state = HS_NONE;
  mount.tracking(wasTracking);
}

// once homed mark as done
void Home::requestDone() {
  state = HS_NONE;
  reset(false);
}

// reset mount at home
CommandError Home::reset(bool fullReset) {
  #if GOTO_FEATURE == ON
    if (goTo.state != GS_NONE) return CE_SLEW_IN_MOTION;
  #endif
  if (guide.state != GU_NONE) {
    if (guide.state == GU_HOME_GUIDE) guide.stop();
    return CE_SLEW_IN_MOTION;
  }

  // refresh the home position
  init();

  // stop tracking and set default rate
  mount.tracking(false);
  mount.trackingRate = hzToSidereal(SIDEREAL_RATE_HZ);
  goTo.firstGoto = true;

  tasks.yieldMicros(10000);

  // setup axis1 and axis2
  if (axis1.resetPosition(0.0L) != 0) { DL("WRN: Home::reset(), failed to resetPosition Axis1"); }
  if (axis2.resetPosition(0.0L) != 0) { DL("WRN: Home::reset(), failed to resetPosition Axis2"); }

  if (transform.mountType == ALTAZM) {
    axis1.setInstrumentCoordinate(position.z);
    axis2.setInstrumentCoordinate(position.a);
  } else {
    axis1.setInstrumentCoordinate(position.h);
    axis2.setInstrumentCoordinate(position.d);
  }

  axis1.setBacklash(mount.settings.backlash.axis1);
  axis2.setBacklash(mount.settings.backlash.axis2);

  axis1.setFrequencySlew(degToRadF(0.1F));
  axis2.setFrequencySlew(degToRadF(0.1F));

  // make sure the motors are powered off
  if (fullReset) mount.enable(false);

  #if GOTO_FEATURE == ON
    if (fullReset) goTo.alignReset();
  #endif

  mount.setHome(true);

  if (fullReset) { VLF("MSG: Mount, reset at home and in standby"); } else { VLF("MSG: Mount, reset at home"); }

  return CE_NONE;
}

Home home;

#endif
