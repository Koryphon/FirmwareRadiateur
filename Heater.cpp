#include "Config.h"
#include "Heater.h"
#include "Debug.h"
#include <Arduino.h>

/*------------------------------------------------------------------------------
 */
Heater::Heater(const uint8_t *const inPinAddr, const uint8_t inPinStop,
               const uint8_t inPinAntifreeze)
    : mPinStop(inPinStop), mPinAntifreeze(inPinAntifreeze),
      mPinAddr(inPinAddr), mProportionalCoeff(kProportionalParameter), 
      mIntegralCoeff(kIntegralParameter),
      mDerivativeCoeff(kDerivativeParameter), mIntegralComponent(0.0), 
      mLastMeanTemperature(kDefaultTemperature), mPWMOffset(k50PercentPWM), 
      mPWMCycle(kHeatingSlots), mPWMCounter(0) {
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
void Heater::begin(const float inDefaultRoomTemperature) {
  mRoomTemperature = inDefaultRoomTemperature;
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
    if (inState == AUTO) {
      mPWMCounter = 0;
    }
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
    if (mPWMCounter == 0) {
      /* Start of a PWM cycle */
      float currentTemperature = meanRoomTemperature();
      float error = mSetpointTemperature - currentTemperature;
      mIntegralComponent += error; 
      mDerivative = currentTemperature - mLastMeanTemperature;
      mLastMeanTemperature = currentTemperature;
      /* 
       * When we reach an integral component that corresponds to the dynamics
       * of the PWM, we limit. 
       */
      if (abs(mIntegralComponent * mIntegralCoeff) > mPWMOffset) {
        if (mIntegralComponent > 0) {
          mIntegralComponent = mPWMOffset / mIntegralCoeff;
        } else {
          mIntegralComponent = - mPWMOffset / mIntegralCoeff;
        }
      }
      
      mPWMDuty = (error * mProportionalCoeff) +
                 (mIntegralComponent * mIntegralCoeff) -
                 (mDerivative * mDerivativeCoeff) +
                 mPWMOffset + 0.5;
      int32_t pwm = mPWMDuty;
      if (pwm < 0) {
        pwm = 0;
      } else if (pwm > mPWMCycle) {
        pwm = mPWMCycle;
      }
      mActualPWM = pwm;

    }

    Serial.print("Compteur="); Serial.print(mPWMCounter); Serial.print(", PWM="); Serial.println(mActualPWM); 
    
    if (mPWMCounter < mActualPWM) {
      comfort();
      mHistory.push(1);
    } else {
      stop();
      mHistory.push(0);
    }

    mPWMCounter = (mPWMCounter >= (mPWMCycle - 1)) ? 0 : mPWMCounter + 1;
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
