#ifndef CRC_CCITT_H
#define CRC_CCITT_H


#include <stdint.h>

//                                      16   12   5
// this is the CCITT CRC 16 polynomial X  + X  + X  + 1.
// This works out to be 0x1021, but the way the algorithm works
// lets us use 0x8408 (the reverse of the bit pattern).  The high
// bit is always assumed to be set, thus we only use 16 bits to
// represent the 17 bit value.

#define POLY 0x8408

uint16_t inline crc16(uint8_t *data_p, uint16_t length)
{
      uint8_t i;
      uint16_t data;
      uint16_t crc = 0xffff;

      if (length == 0)
            return (~crc);

      do
      {
            for (i=0, data=0xff & *data_p++;
                 i < 8;
                 i++, data >>= 1)
            {
                  if ((crc & 0x0001) ^ (data & 0x0001))
                        crc = (crc >> 1) ^ POLY;
                  else  crc >>= 1;
            }
      } while (--length);

      crc = ~crc;
      data = crc;
      crc = (crc << 8) | (data >> 8 & 0xff);

      return (crc);
}



#endif // CRC_CCITT_H
