/*==============================================================================
 * Implementation file for a simple class to handle communication time out
 */

#include "Timeout.h"

void Timeout::timestamp() { mTimestamp = millis(); }

bool Timeout::isTimedout() { return (millis() - mTimestamp) > mInterval; }

bool Timeout::isNotTimedout() { return (millis() - mTimestamp) <= mInterval; }
