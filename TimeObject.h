/*

*/

#ifndef __TIMEOBJECT_H__
#define __TIMEOBJECT_H__

#include <stdint.h>

class TimeObject {
    uint32_t mLastDate;
    TimeObject *mNext;
    static TimeObject *sTimeObjectList;

    void objectLoop(const uint32_t inDate);
    virtual void execute() {}

  protected:
    uint32_t mNextDelay;

  public:
    TimeObject(const uint32_t inNextDelay);
    static void setup();
    static void loop();
};

#endif
