/*
  Classe PeriodicAction

  permet d'appeler une fonction à intervalles réguliers et après un offset
*/

#ifndef __PERIODICACTION_H__
#define __PERIODICACTION_H__

#include "TimeObject.h"

class PeriodicAction : public TimeObject
{
    uint32_t mPeriod;
    void (*mAction)();
    virtual void execute();

  public:
    PeriodicAction(const uint32_t inOffset, const uint32_t inPeriod);
    void begin(void (*inAction)());
};

#endif
