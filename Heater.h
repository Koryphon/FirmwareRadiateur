/*==============================================================================
 * Class Heater.
 *
 * - control of the heater via the pilot wire according to the requested state.
 * - storage of the state of the heater.
 * - storage of the cumulated time spent in each state (in ms on 64 bits)
 * - % of time spent in comfort mode.
 */

#ifndef __HEATER_H__
#define __HEATER_H__

#include "BitRingBuf.h"
#include "TemperatureHistory.h"
#include "HeatingHistory.h"
#include <WString.h>
#include <stdint.h>

class Heater {
public:
  typedef enum { STOP, AUTO, ANTI, ECO } HeaterState;

private:
  /* mHistory stores the satisfaction history of the setpoint */
  HeatingHistory mHistory;
  /* State of the heater */
  HeaterState mState;
  /* Room temperature and setpoint temperature */
  float mRoomTemperature;
  float mSetpointTemperature;
  TemperatureHistory tempHistory;

  float mProportionalCoeff;
  float mIntegralCoeff;
  float mDerivativeCoeff;
  float mIntegralComponent;
  float mLastMeanTemperature;
  float mDerivative;
  float mPWMDuty;
  float mPWMOffset;

  uint32_t mActualPWM;
  uint32_t mPWMCycle;
  uint32_t mPWMCounter;

  /* Pins */
  uint8_t mPinStop;
  uint8_t mPinAntifreeze;
  const uint8_t *const mPinAddr;
  /* Heater Num */
  uint8_t mNum;
  /* Heater Id */
  String mId;

  void changeStateTo(const HeaterState inState);
  void readHeaterNum();
  void stop();
  void comfort();
  void antifreeze();
  void eco();

public:
  Heater(const uint8_t *const inPinAddr, const uint8_t inPinStop,
         const uint8_t inPinAntifreeze);
  void begin(const float inDefaultRoomTemperature);
  void setStop();
  void setAuto();
  void setAntifreeze();
  void setEco();
  void setMode(const HeaterState inMode);
  void setSetpoint(const float inSetpoint) {
    mSetpointTemperature = inSetpoint;
  }
  void setRoomTemperature(const float inRoomTemperature) {
    mRoomTemperature = inRoomTemperature;
    tempHistory.add(inRoomTemperature);
  }
  void loop();
  uint32_t num() const        { return mNum; }
  const String &id() const    { return mId; }
  HeaterState state() const   { return mState; }
  float pwmDuty()             { return mPWMDuty; }
  float integralComponent()   { return mIntegralComponent; }
  uint32_t actualPWM()        { return mActualPWM; }
  uint32_t pwmCounter()       { return mPWMCounter; }
  uint32_t pwmCycle()         { return mPWMCycle; }
  float meanRoomTemperature() { return tempHistory.mean(); }
  float derivative()          { return mDerivative; }
  float shortTermEnergy()     { return mHistory.shortTermEnergy(); }
  float averageTermEnergy()   { return mHistory.averageTermEnergy(); }
  float longTermEnergy()      { return mHistory.longTermEnergy(); }
  const char *const stringState() const;
};

#endif
