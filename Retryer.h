/*

*/

#ifndef __RETRYER_H__
#define __RETRYER_H__

#include <stdint.h>

class Retryer
{
    uint32_t mRetryCount;
    uint32_t mCountLimit;
    uint32_t mRetrySegmentCount; /* The number of times a retry series has been performed */
    uint32_t mLastRetryDate;
    uint32_t mRetryInterval;

  public:
    Retryer(const uint32_t inCountLimit, const uint32_t inRetryInterval = UINT32_MAX)
      : mRetryCount(0ul), mCountLimit(inCountLimit), 
        mRetrySegmentCount(0ul), mLastRetryDate(0ul), mRetryInterval(inRetryInterval) {}
    void retry();
    void reset() { mRetryCount = 0ul; }
    uint32_t count() { return mRetryCount; }
    uint32_t segCount() { return mRetrySegmentCount; }
};

#endif
