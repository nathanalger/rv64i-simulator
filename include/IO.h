#pragma once
#include <cstdint>

class IO
{
public:
   virtual void writeChar(char c) = 0;

   virtual void writeString(const char *str)
   {
      while (*str)
      {
         writeChar(*str++);
      }
   }

   void writeInt(uint64_t value)
   {
      char buffer[21];
      int i = 0;

      if (value == 0)
      {
         writeChar('0');
         return;
      }

      while (value > 0)
      {
         buffer[i++] = '0' + (value % 10);
         value /= 10;
      }

      // reverse
      while (i--)
      {
         writeChar(buffer[i]);
      }
   }

   virtual ~IO() = default;
};