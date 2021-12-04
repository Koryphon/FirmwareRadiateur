/*
  Classe Heater.

  - pilotage du radiateur via le fil pilote en fonction de l'état demandé.
  - mémorisation de l'état du radiateur
  - mémorisation du temps cumulé passé dans chaque état (en ms sur 64 bits)
  - % de temps passé en mode confort.
*/

#ifndef __HEATER_H__
#define __HEATER_H__

#include <stdint.h>

class Heater
{
    typedef enum { STOP, COMFORT, ANTI, ECO } HeaterState;
    uint64_t mTimeStop;
    uint64_t mTimeComfort;
    uint64_t mTimeAntifreeze;
    uint64_t mTimeEco;
    HeaterState mState;
    uint64_t mDateChangedState;
    uint32_t mLastRefresh;
    uint64_t mDate;
    uint8_t mPinStop;
    uint8_t mPinAntifreeze;

    void refreshDate();
    void changeStateTo(const HeaterState inState);
    uint64_t totalTime();

  public:
    Heater(const uint8_t inPinStop, const uint8_t inPinAntifreeze);
    void begin();
    void setStop();
    void setComfort();
    void setAntifreeze();
    void setEco();
    HeaterState state() const;
    const char * const stringState() const;
    uint64_t accumulatedTimeStop();
    uint64_t accumulatedTimeComfort();
    uint64_t accumulatedTimeAntifreeze();
    uint64_t accumulatedTimeEco();
    uint64_t currentTimeStop();
    uint64_t currentTimeComfort();
    uint64_t currentTimeAntifreeze();
    uint64_t currentTimeEco();
    float comfortRatio();
};

#endif
