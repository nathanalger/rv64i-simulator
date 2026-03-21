#include "UART.h"

UART::UART(IO *io_device, uint64_t base_addr)
    : io(io_device), base_address(base_addr)
{
   // Initialize all registers to 0
   for (int i = 0; i < 8; i++)
   {
      registers[i] = 0;
   }
   // LSR (Line Status Register) - Bit 5 (0x20) means TX is empty/ready to receive data
   registers[5] = 0x20;
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
      // Offset 0: Transmitter Holding Register (THR)
      // A write here means the CPU wants to output a character!
      if (io)
      {
         io->writeChar(static_cast<char>(value));
      }
   }
   else if (offset < 8)
   {
      // Write to other config registers (LCR, IER, etc.)
      // Advanced OS's will configure baud rates here, but we just save the value.
      registers[offset] = value;
   }
}