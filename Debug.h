#ifndef __DEBUG_H__
#define __DEBUG_H__

void logTime();

#define DEBUG

#ifdef DEBUG
#define DEBUG_DO(inst) inst
#define DEBUG_P(mess) Serial.print(mess)
#define DEBUG_PLN(mess) Serial.println(mess)
#define LOGT logTime()
#else
#define DEBUG_DO(inst)
#define DEBUG_P(mess)
#define DEBUG_PLN(mess)
#define LOGT
#endif

#endif