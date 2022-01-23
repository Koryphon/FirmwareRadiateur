#ifndef __TEMPERATUREHISTORY_H__
#define __TEMPERATUREHISTORY_H__

#include "Config.h"
#include <RingBuf.h>

class TemperatureHistory
{
  RingBuf<float, kTemperatureMeasurementSlots> mTemperatureBuffer;
  float mSum;

public:
  TemperatureHistory() : mSum(0.0) {}
  float mean();
  void add(const float inTemp);
};

#endif
