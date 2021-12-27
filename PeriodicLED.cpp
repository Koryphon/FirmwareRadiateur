/*

*/

#include "PeriodicLED.h"
#include <Arduino.h>

/*------------------------------------------------------------------------------
 */
void PeriodicLED::execute() {
  if (mLEDState == mInitialLEDState) {
    mNextDelay = mDuty;
  } else {
    mNextDelay = mPeriod - mDuty;
  }
  mLEDState = !mLEDState;
  digitalWrite(mLEDPin, mLEDState);
}

/*------------------------------------------------------------------------------
 */
PeriodicLED::PeriodicLED(const uint8_t inLEDPin, const uint32_t inPeriod,
                         const uint32_t inDuty)
    : TimeObject(inDuty < inPeriod ? inPeriod - inDuty : 0),
      mPeriod(inPeriod), mDuty(inDuty < inPeriod ? inDuty : inPeriod),
      mLEDPin(inLEDPin), mLEDState(LOW), mInitialLEDState(LOW) {}

/*------------------------------------------------------------------------------
 */
void PeriodicLED::begin(const uint8_t inInitialLEDState) {
  mInitialLEDState = inInitialLEDState != 0 ? HIGH : LOW;
  mLEDState = mInitialLEDState;
  pinMode(mLEDPin, OUTPUT);
  digitalWrite(mLEDPin, mLEDState);
}

/*------------------------------------------------------------------------------
 */
void PeriodicLED::setPeriodAndDuty(const uint32_t inPeriod,
                                   const uint32_t inDuty) {
  mPeriod = inPeriod;
  mDuty = inDuty < inPeriod ? inDuty : inPeriod;
}
