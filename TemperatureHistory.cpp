#include "TemperatureHistory.h"

float TemperatureHistory::mean()
{
  if (mTemperatureBuffer.isEmpty()) {
    return kDefaultTemperature;
  } else {
    return mSum / (float)mTemperatureBuffer.size();
  }
}

void TemperatureHistory::add(const float inTemp)
{
  if (mTemperatureBuffer.isFull()) {
    /* Remove the oldest element and substract it from the sum */
    float removed;
    mTemperatureBuffer.pop(removed);
    mSum -= removed;
  }
  mTemperatureBuffer.push(inTemp);
  mSum += inTemp;
}
