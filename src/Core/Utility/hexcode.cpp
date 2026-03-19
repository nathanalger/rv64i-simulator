#include "Utility.h"

char Utility::hexbuffer[19];
char *Utility::int64_to_hex(uint64_t value)
{
   const char *hex = "0123456789ABCDEF";

   Utility::hexbuffer[0] = '0';
   Utility::hexbuffer[1] = 'x';

   for (int i = 0; i < 16; i++)
   {
      int shift = (15 - i) * 4;
      uint8_t digit = (value >> shift) & 0xF;
      Utility::hexbuffer[2 + i] = hex[digit];
   }

   Utility::hexbuffer[18] = '\0';
   return Utility::hexbuffer;
}