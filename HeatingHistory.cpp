#include "HeatingHistory.h"

/*------------------------------------------------------------------------------
 * Constructor
 */
HeatingHistory::HeatingHistory() :
  mShortTermCounter(0), mAverageTermCounter(0), mLongTermCounter(0),
  mAverageTermSum(0.0), mLongTermSum(0.0)
{
  
}

/*------------------------------------------------------------------------------
 * Push a new heating slot state
 */
void HeatingHistory::push(const uint32_t inBit)
{
  mShortTermHistory.push(inBit);
  mShortTermCounter++;

  if (mShortTermCounter == kShortTermSize) {

    mShortTermCounter = 0;

    const float stEnergy = shortTermEnergy();
    if (mAverageTermHistory.isFull()) {
      /* Remove the oldest element and substract it from the sum */
      float removed;
      mAverageTermHistory.pop(removed);
      mAverageTermSum -= removed;
    }
    mAverageTermHistory.push(stEnergy);
    mAverageTermSum += stEnergy;

    mAverageTermCounter++;

    if (mAverageTermCounter == kAverageTermSize) {

      mAverageTermCounter = 0;

      const float atEnergy = averageTermEnergy();
      if (mLongTermHistory.isFull()) {
        /* Remove the oldest element and substract it from the sum */
        float removed;
        mLongTermHistory.pop(removed);
        mLongTermSum -= removed;
      }
      mLongTermHistory.push(atEnergy);
      mLongTermSum += atEnergy;
    }
  }
}

float HeatingHistory::shortTermEnergy() const
{
  return mShortTermHistory.loadAverage();
}

float HeatingHistory::averageTermEnergy() 
{
  if (mAverageTermHistory.isEmpty()) {
    return 0.0;
  } else {
    return mAverageTermSum / (float)mAverageTermHistory.size();
  }
}

float HeatingHistory::longTermEnergy()
{
  if (mLongTermHistory.isEmpty()) {
    return 0.0;
  } else {
    return mLongTermSum / (float)mLongTermHistory.size();
  }

}
