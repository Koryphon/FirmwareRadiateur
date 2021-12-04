#include "Retryer.h"
#include <Arduino.h>

/*------------------------------------------------------------------------------
*/
void Retryer::retry()
{
  delay(mDelay);
  mRetryCount++;
  if (mRetryCount > mCountLimit) {
    ESP.restart();
  }
}
