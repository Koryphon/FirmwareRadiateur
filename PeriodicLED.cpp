/*

*/

#include "PeriodicLED.h"
#include <Arduino.h>

/*------------------------------------------------------------------------------
*/
void PeriodicLED::execute()
{
  if (mLEDState == mInitialLEDState) {
    mNextDelay = mOnTime;
  } else {
    mNextDelay = mPeriod - mOnTime;
  }
  mLEDState = ! mLEDState;
  digitalWrite(mLEDPin, mLEDState);
}

/*------------------------------------------------------------------------------
*/
PeriodicLED::PeriodicLED(const uint8_t inLEDPin, const uint32_t inPeriod, const uint32_t inOnTime)
  : TimeObject(inOnTime < inPeriod ? inPeriod - inOnTime : 0),
    mPeriod(inPeriod),
    mOnTime(inOnTime < inPeriod ? inOnTime : inPeriod),
    mLEDPin(inLEDPin),
    mLEDState(LOW),
    mInitialLEDState(LOW)
{}

/*------------------------------------------------------------------------------
*/
void PeriodicLED::begin(const uint8_t inInitialLEDState)
{
  mInitialLEDState = inInitialLEDState != 0 ? HIGH : LOW;
  mLEDState = mInitialLEDState;
  pinMode(mLEDPin, OUTPUT);
  digitalWrite(mLEDPin, mLEDState);
}
