#include "Debug.h"
#include <Arduino.h>

/*------------------------------------------------------------------------------
  Affiche un nombre entier positif sur le nombre minimum de caractères spécifiés
*/
void fmtPrint(uint32_t inVal, uint8_t inFieldSize) {
  /* calcule le nombre de caractères nécessaire */
  uint32_t tmp = inVal / 10;
  uint8_t s = 1;
  while (tmp > 0) {
    s++;
    tmp /= 10;
  }
  for (uint8_t i = 0; i < (inFieldSize - s); i++)
    Serial.print('0');
  Serial.print(inVal);
}

/*------------------------------------------------------------------------------
  Affiche la date sous une forme HH:MM:SS:MS
*/
void logTime() {
  uint32_t date = millis();
  uint32_t ms = date % 1000;
  uint32_t s = (date / 1000) % 60;
  uint32_t m = (date / 60000) % 60;
  uint32_t h = date / 3600000;
  fmtPrint(h, 2);
  Serial.print(':');
  fmtPrint(m, 2);
  Serial.print(':');
  fmtPrint(s, 2);
  Serial.print('.');
  fmtPrint(ms, 3);
  Serial.print(" : ");
}
