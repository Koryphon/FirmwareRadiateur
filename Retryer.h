/*

*/

#ifndef __RETRYER_H__
#define __RETRYER_H__

#include <stdint.h>

class Retryer
{
    uint32_t mRetryCount;
    uint32_t mCountLimit;
    uint32_t mRetrySegmentCount; /* Le nombre de fois où une série de retry a été effectuée */

  public:
    Retryer(const uint32_t inCountLimit)
      : mRetryCount(0), mCountLimit(inCountLimit), mRetrySegmentCount(0) {}
    void retry();
    void reset() { mRetryCount = 0; }
    uint32_t count() { return mRetryCount; }
    uint32_t segCount() { return mRetrySegmentCount; }
};

#endif
