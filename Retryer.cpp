#include "Retryer.h"
#include <Arduino.h>

/*------------------------------------------------------------------------------
*/
void Retryer::retry()
{
  if (mRetryCount == 0) {
    mRetrySegmentCount++;
  }
  mRetryCount++;
  if (mRetryCount > mCountLimit) {
    ESP.restart();
  }
}
