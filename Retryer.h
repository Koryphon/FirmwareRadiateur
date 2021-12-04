/*

*/

#ifndef __RETRYER_H__
#define __RETRYER_H__

#include <stdint.h>

class Retryer
{
    uint32_t mRetryCount;
    uint32_t mCountLimit;
    uint32_t mDelay;

  public:
    Retryer(const uint32_t inCountLimit, const uint32_t inDelay)
      : mRetryCount(0), mCountLimit(inCountLimit), mDelay(inDelay) {}
    void retry();
    void reset() { mRetryCount = 0; }
};

#endif
