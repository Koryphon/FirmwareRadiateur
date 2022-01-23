#ifndef __HEATINGHISTORY_H__
#define __HEATINGHISTORY_H__

#include "config.h"
#include "BitRingBuf.h"
#include "RingBuf.h"

/*------------------------------------------------------------------------------
 * HeatingHistory stores the following informations
 * 1 - short term history for 10 minutes aka 600 seconds
 * 2 - average term history for 2 hours = 12 values 
 * 4 - long term history for 1 day = 12 values
 *
 */
class HeatingHistory {
    static const uint32_t kShortTermSize = (10 * 60) / (kHeatingSlotDuration / 1000);
    static const uint32_t kAverageTermSize = 12;
    static const uint32_t kLongTermSize = 12;
  
    BitRingBuf<kShortTermSize> mShortTermHistory;
    RingBuf<float, kAverageTermSize> mAverageTermHistory;
    RingBuf<float, kLongTermSize> mLongTermHistory;

    uint32_t mShortTermCounter;
    uint32_t mAverageTermCounter;
    uint32_t mLongTermCounter;

    float mAverageTermSum;
    float mLongTermSum;

  public:
    HeatingHistory();
    void push(const uint32_t inBit);
    float shortTermEnergy() const;
    float averageTermEnergy();
    float longTermEnergy();
};

#endif
