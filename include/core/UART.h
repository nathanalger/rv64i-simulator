#pragma once
#include <cstdint>
#include "IO.h"

class UART
{
private:
   IO *io;
   uint64_t base_address;

   // The 16550 UART has several internal 8-bit registers.
   // We'll store their state here.
   uint8_t registers[8];

public:
   // Standard RISC-V QEMU UART base is 0x10000000
   UART(IO *io_device, uint64_t base_addr = 0x10000000);

   bool contains(uint64_t address) const;

   uint8_t readByte(uint64_t address);
   void writeByte(uint64_t address, uint8_t value);
};