#pragma once
#include <stddef.h>
#include <stdint.h>
#include "IO.h"

class UART
{
private:
   IO *io;
   uint64_t base_address;

   uint8_t registers[8];

public:
   UART(IO *io_device, uint64_t base_addr = 0x10000000);

   bool contains(uint64_t address) const;

   uint8_t readByte(uint64_t address);
   void writeByte(uint64_t address, uint8_t value);
};