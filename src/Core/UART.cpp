#include "UART.h"

UART::UART(IO *io_device, uint64_t base_addr)
    : io(io_device), base_address(base_addr)
{
   for (int i = 0; i < 8; i++)
   {
      registers[i] = 0;
   }
   // LSR (offset 5) - Bits 5 and 6 (0x60) mean TX is completely empty and ready
   registers[5] = 0x60;
}

bool UART::contains(uint64_t address) const
{
   // The 16550 standard registers span 8 bytes from the base
   return address >= base_address && address < base_address + 8;
}

uint8_t UART::readByte(uint64_t address)
{
   uint64_t offset = address - base_address;

   // Simply return the state of the requested register
   if (offset < 8)
   {
      return registers[offset];
   }
   return 0;
}

void UART::writeByte(uint64_t address, uint8_t value)
{
   uint64_t offset = address - base_address;

   if (offset == 0)
   {
      // Check if DLAB (Bit 7 of LCR at offset 3) is set
      if ((registers[3] & 0x80) != 0)
      {
         return;
      }
      else
      {
         // DLAB is 0: This is a normal character write to the Transmitter Holding Register
         if (io)
         {
            io->writeChar(static_cast<char>(value));
         }
      }
   }
   else if (offset < 8)
   {
      // Protect the Line Status Register (offset 5) from being overwritten by the guest
      if (offset != 5)
      {
         registers[offset] = value;
      }
   }
}