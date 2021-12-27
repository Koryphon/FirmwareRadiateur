/*==============================================================================
 * FirmwareRadiateur
 *
 * Ring buffer of bits to store the time slots in which the heater is on or off
 */

#ifndef __BITRINGBUF_H__
#define __BITRINGBUF_H__

// #define DOW

#include <stddef.h>
#include <stdint.h>

#ifdef DOW
#include <iostream>
#endif

/*------------------------------------------------------------------------------
 * S is the maximum number of bits in the bit ring buffer.
 */
template <size_t S> class BitRingBuf {
  uint32_t mBits[S / 32 + ((S % 32) != 0)];
  uint32_t mSize;
  uint32_t mWriteIndex;

  /*
   * Write a bit at the inIdex location
   */
  void writeBit(const uint32_t inIndex, const uint32_t inBit) {
    if (inIndex < mSize) {
      uint32_t arrayIdx = inIndex / 32;
      uint32_t bitIdx = inIndex % 32;
      mBits[arrayIdx] &= ~(1 << bitIdx);
      mBits[arrayIdx] |= (inBit & 1) << bitIdx;
    }
  }

  /*
   * Count the number of bits set to one in a word
   */
  uint32_t onBitCount(uint32_t inWord) const {
    uint32_t count = 0;
    if (inWord != 0) {
      for (uint32_t i = 0; i < 32; i++) {
        count += inWord & 1;
        inWord >>= 1;
      }
    }
    return count;
  }

  /*
   * Count the number of bits set to one in the buffer
   */
  uint32_t onCount() const {
    uint32_t count = 0;
    for (uint32_t i = 0; i < S / 32 + ((S % 32) != 0); i++) {
      count += onBitCount(mBits[i]);
    }
    return count;
  }

public:
  /*
   * At start all the bits are set to 0
   */
  BitRingBuf() : mSize(0), mWriteIndex(0) {
    for (uint32_t i = 0; i < S / 32 + ((S % 32) != 0); i++) {
      mBits[i] = 0;
    }
  }

  /*
   * actual size of the buffer
   */
  uint32_t size() const { return mSize; }

  /*
   * Read a bit at the inIdex location
   */
  uint32_t readBit(const uint32_t inIndex) const {
    if (inIndex < mSize) {
      uint32_t arrayIdx = inIndex / 32;
      uint32_t bitIdx = inIndex % 32;
      return mBits[arrayIdx] >> bitIdx & 1;
    } else
      return 0;
  }

  /*
   * Push a bit in the buffer
   */
  void push(const uint32_t inBit) {
    if (mSize < S) {
      mSize++;
    }
    writeBit(mWriteIndex, inBit);
    mWriteIndex++;
    if (mWriteIndex == S) {
      mWriteIndex = 0;
    }
  }

  uint32_t loadAverage() const { return 100 * onCount() / mSize; }
};

#ifdef DOW
template <size_t S>
std::ostream &operator<<(std::ostream &s, BitRingBuf<S> &b) {
  s << '(' << b.size() << ") ";
  for (uint32_t i = 0; i < b.size(); i++) {
    s << b.readBit(i);
  }
  return s;
}
#endif

#endif
