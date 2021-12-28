#include "Retryer.h"
#include <Arduino.h>

/*------------------------------------------------------------------------------
*/
void Retryer::retry()
{
  const uint32_t currentDate = millis();
  if (mRetryInterval == 0 || (currentDate - mLastRetryDate) < mRetryInterval) {
    if (mRetryCount == 0) {
      mRetrySegmentCount++;
    }
    mRetryCount++;
    if (mRetryCount > mCountLimit) {
      ESP.restart();
    }
  }
  mLastRetryDate = currentDate;
}
