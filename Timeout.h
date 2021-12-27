/*==============================================================================
 * Header file for a simple class to handle communication time out
 */
#ifndef __TIMEOUT_H__
#define __TIMEOUT_H__

#include <Arduino.h>

class Timeout {
  uint32_t mInterval;
  uint32_t mTimestamp;

public:
  Timeout(const uint32_t inInterval) : mInterval(inInterval), mTimestamp(0ul) {}
  void timestamp();
  bool isTimedout();
  bool isNotTimedout();
};

#endif