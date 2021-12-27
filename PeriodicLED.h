/*

*/

#ifndef __PERIODICLED_H__
#define __PERIODICLED_H__

#include "TimeObject.h"

class PeriodicLED : public TimeObject {
  uint32_t mPeriod;
  uint32_t mDuty;
  uint8_t mLEDPin;
  uint8_t mLEDState;
  uint8_t mInitialLEDState;

  virtual void execute();

public:
  PeriodicLED(const uint8_t inLEDPin, const uint32_t inPeriod,
              const uint32_t inDuty);
  void begin(const uint8_t inInitialLEDState);
  void setPeriodAndDuty(const uint32_t inPeriod, const uint32_t inDuty);
};

#endif
