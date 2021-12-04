#include "Heater.h"
#include <Arduino.h>

/*------------------------------------------------------------------------------
*/
Heater::Heater(const uint8_t inPinStop, const uint8_t inPinAntifreeze)
  : mTimeStop(0),
    mTimeComfort(0),
    mTimeAntifreeze(0),
    mTimeEco(0),
    mState(COMFORT),
    mDateChangedState(0),
    mLastRefresh(0),
    mDate(0),
    mPinStop(inPinStop),
    mPinAntifreeze(inPinAntifreeze)
{
}

/*------------------------------------------------------------------------------
*/
void Heater::begin()
{
  pinMode(mPinStop, OUTPUT);
  pinMode(mPinAntifreeze, OUTPUT);
  setComfort();
}

/*------------------------------------------------------------------------------
*/
void Heater::refreshDate()
{
  const uint32_t currentDate = millis();
  mDate += currentDate - mLastRefresh;
  mLastRefresh = currentDate;
}

/*------------------------------------------------------------------------------
*/
void Heater::changeStateTo(const HeaterState inState)
{
  refreshDate();
  if (inState != mState) {
    const uint64_t lastedInState = mDate - mDateChangedState;
    switch (mState) {
      case STOP:    mTimeStop       += lastedInState; break;
      case COMFORT: mTimeComfort    += lastedInState; break;
      case ANTI:    mTimeAntifreeze += lastedInState; break;
      case ECO:     mTimeEco        += lastedInState; break;
    }
    mDateChangedState = mDate;
    mState = inState;
  }
}

/*------------------------------------------------------------------------------
*/
void Heater::setStop()
{
  changeStateTo(STOP);
  digitalWrite(mPinAntifreeze, LOW);
  digitalWrite(mPinStop, HIGH);
}

/*------------------------------------------------------------------------------
*/
void Heater::setComfort()
{
  changeStateTo(COMFORT);
  digitalWrite(mPinAntifreeze, LOW);
  digitalWrite(mPinStop, LOW);
}

/*------------------------------------------------------------------------------
*/
void Heater::setAntifreeze()
{
  changeStateTo(ANTI);
  digitalWrite(mPinAntifreeze, HIGH);
  digitalWrite(mPinStop, LOW);
}

/*------------------------------------------------------------------------------
*/
void Heater::setEco()
{
  changeStateTo(ECO);
  digitalWrite(mPinAntifreeze, HIGH);
  digitalWrite(mPinStop, HIGH);
}

/*------------------------------------------------------------------------------
*/
Heater::HeaterState Heater::state() const
{
  return mState;
}

/*------------------------------------------------------------------------------
*/
const char * const Heater::stringState() const
{
  switch (mState) {
    case STOP: return "stop";
    case COMFORT: return "comfort";
    case ANTI: return "antifreeze";
    case ECO: return "eco";
    default: return "?";
  }
}

/*------------------------------------------------------------------------------
*/
uint64_t Heater::accumulatedTimeStop()
{
  return mTimeStop;
}

/*------------------------------------------------------------------------------
*/
uint64_t Heater::accumulatedTimeComfort()
{
  return mTimeComfort;
}

/*------------------------------------------------------------------------------
*/
uint64_t Heater::accumulatedTimeAntifreeze()
{
  return mTimeAntifreeze;
}

/*------------------------------------------------------------------------------
*/
uint64_t Heater::accumulatedTimeEco()
{
  return mTimeEco;
}

/*------------------------------------------------------------------------------
*/
uint64_t Heater::currentTimeStop()
{
  refreshDate();
  return accumulatedTimeStop() +
         (mState == STOP ? mDate - mDateChangedState : 0);
}

/*------------------------------------------------------------------------------
*/
uint64_t Heater::currentTimeComfort()
{
  refreshDate();
  return accumulatedTimeComfort() +
         (mState == COMFORT ? mDate - mDateChangedState : 0);
}

/*------------------------------------------------------------------------------
*/
uint64_t Heater::currentTimeAntifreeze()
{
  refreshDate();
  return accumulatedTimeAntifreeze() +
         (mState == ANTI ? mDate - mDateChangedState : 0);
}

/*------------------------------------------------------------------------------
*/
uint64_t Heater::currentTimeEco()
{
  refreshDate();
  return accumulatedTimeEco() +
         (mState == ECO ? mDate - mDateChangedState : 0);
}

/*------------------------------------------------------------------------------
*/
uint64_t Heater::totalTime()
{
  return currentTimeStop() +
         currentTimeComfort() +
         currentTimeAntifreeze() +
         currentTimeEco();
}

/*------------------------------------------------------------------------------
*/
float Heater::comfortRatio()
{
  return 100.0 * (float)currentTimeComfort() / (float)totalTime();
}
