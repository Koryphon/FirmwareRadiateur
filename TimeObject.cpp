/*

*/

#include "TimeObject.h"
#include <Arduino.h>

/*------------------------------------------------------------------------------
*/
TimeObject *TimeObject::sTimeObjectList = NULL;


/*------------------------------------------------------------------------------
*/
TimeObject::TimeObject(const uint32_t inNextDelay)
  : mLastDate(0), mNextDelay(inNextDelay)
{
  mNext = sTimeObjectList;
  sTimeObjectList = this;
}

/*------------------------------------------------------------------------------
*/
void TimeObject::objectLoop(const uint32_t inDate)
{
  if ((inDate - mLastDate) >= mNextDelay) {
    mLastDate += mNextDelay;
    execute();
  }
}

/*------------------------------------------------------------------------------
  setup doit être appeler à la fin du setup du sketch Arduino afin de marquer
  l'instant initial des TimeObject. 
*/
void TimeObject::setup()
{
  const uint32_t currentDate = millis();
  TimeObject *obj = sTimeObjectList;
  while (obj != NULL) {
    obj->mLastDate = currentDate;
    obj = obj->mNext;
  }
}

/*------------------------------------------------------------------------------
  loop doit être appelé aussi souvent que possible dans le loop du sketch
  Arduino afin d'exécuter les TimeObject de la manière la plus précise possible.
*/
void TimeObject::loop()
{
  const uint32_t currentDate = millis();
  TimeObject *obj = sTimeObjectList;
  while (obj != NULL) {
    obj->objectLoop(currentDate);
    obj = obj->mNext;
  }
}
