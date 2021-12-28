#include "Heater.h"
#include "Debug.h"
#include <Arduino.h>

/*------------------------------------------------------------------------------
 */
Heater::Heater(const uint8_t *const inPinAddr, const uint8_t inPinStop,
               const uint8_t inPinAntifreeze)
    : mPinStop(inPinStop), mPinAntifreeze(inPinAntifreeze),
      mPinAddr(inPinAddr) {
  setEco();
}

/*------------------------------------------------------------------------------
 */
void Heater::readHeaterNum() {
  uint8_t num = 0;
  for (uint32_t pinIdx = 0; pinIdx < 6; pinIdx++) {
    pinMode(mPinAddr[pinIdx], INPUT_PULLUP);
  }
  for (uint32_t pinIdx = 0; pinIdx < 6; pinIdx++) {
    num |= (!digitalRead(mPinAddr[pinIdx])) << pinIdx;
  }
  LOGT;
  DEBUG_P("Numero radiateur : ");
  DEBUG_PLN(num);
  mNum = num;
}

/*------------------------------------------------------------------------------
 */
void Heater::stop() {
  digitalWrite(mPinAntifreeze, LOW);
  digitalWrite(mPinStop, HIGH);
}
/*------------------------------------------------------------------------------
 */
void Heater::comfort() {
  digitalWrite(mPinAntifreeze, LOW);
  digitalWrite(mPinStop, LOW);
}
/*------------------------------------------------------------------------------
 */
void Heater::antifreeze() {
  digitalWrite(mPinAntifreeze, HIGH);
  digitalWrite(mPinStop, LOW);
}
/*------------------------------------------------------------------------------
 */
void Heater::eco() {
  digitalWrite(mPinAntifreeze, HIGH);
  digitalWrite(mPinStop, HIGH);
}

/*------------------------------------------------------------------------------
 */
void Heater::begin() {
  readHeaterNum();
  pinMode(mPinStop, OUTPUT);
  pinMode(mPinAntifreeze, OUTPUT);
  setEco();
}

/*------------------------------------------------------------------------------
 */
void Heater::changeStateTo(const HeaterState inState) {
  if (inState != mState) {
    mState = inState;
  }
}

/*------------------------------------------------------------------------------
 */
void Heater::setStop() {
  changeStateTo(STOP);
  stop();
}

/*------------------------------------------------------------------------------
 */
void Heater::setAuto() {
  changeStateTo(AUTO);
  stop();
}

/*------------------------------------------------------------------------------
 */
void Heater::setAntifreeze() {
  changeStateTo(ANTI);
  antifreeze();
}

/*------------------------------------------------------------------------------
 */
void Heater::setEco() {
  changeStateTo(ECO);
  eco();
}

/*------------------------------------------------------------------------------
 */
void Heater::setMode(const HeaterState inMode) {
  switch (inMode) {
  case STOP:
    setStop();
    break;
  case AUTO:
    setAuto();
    break;
  case ANTI:
    setAntifreeze();
    break;
  case ECO:
    setEco();
    break;
  default: /* back to ECO */
    setEco();
    break;
  }
}

/*------------------------------------------------------------------------------
 */
void Heater::loop() {
  if (mState == AUTO) {
    if (mRoomTemperature < mSetpointTemperature) {
      comfort();
      mHistory.push(1);
    } else {
      stop();
      mHistory.push(0);
    }
  }
}

/*------------------------------------------------------------------------------
 */
const char *const Heater::stringState() const {
  switch (mState) {
  case STOP:
    return "stop";
  case AUTO:
    return "auto";
  case ANTI:
    return "anti";
  case ECO:
    return "eco";
  default:
    return "?";
  }
}
