/*

*/

#include "PeriodicAction.h"
#include <Arduino.h>

/*------------------------------------------------------------------------------
*/
void PeriodicAction::execute()
{
  if (mAction != NULL) mAction();
  mNextDelay = mPeriod;
}

/*------------------------------------------------------------------------------
*/
PeriodicAction::PeriodicAction(const uint32_t inOffset, const uint32_t inPeriod)
  : TimeObject(inOffset), mPeriod(inPeriod), mAction(NULL)
{}

/*------------------------------------------------------------------------------
*/
void PeriodicAction::begin(void (*inAction)())
{
  mAction = inAction;
}
