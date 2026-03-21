#include "Bus.h"

Bus::Bus(Memory &ram_ref, IO *io_ptr, uint64_t base_address, UART *uart_ptr)
    : ram(ram_ref), io(io_ptr), ram_base(base_address), uart(uart_ptr) {}

uint8_t Bus::readByte(uint64_t address)
{
   if (uart && uart->contains(address))
   {
      return uart->readByte(address);
   }

   if (address >= ram_base && address < ram_base + ram.getSize())
   {
      return ram.readByte(address - ram_base);
   }
   return 0;
}

uint8_t Bus::writeByte(uint64_t address, uint8_t value)
{
   if (uart && uart->contains(address))
   {
      uart->writeByte(address, value);
      return value;
   }

   if (address >= ram_base && address < ram_base + ram.getSize())
   {
      return ram.writeByte(address - ram_base, value);
   }
   return value;
}

uint16_t Bus::readHalf(uint64_t address)
{
   if (address >= ram_base && address + 1 < ram_base + ram.getSize())
   {
      return ram.readHalf(address - ram_base);
   }
   return 0;
}

uint16_t Bus::writeHalf(uint64_t address, uint16_t value)
{
   if (address >= ram_base && address + 1 < ram_base + ram.getSize())
   {
      return ram.writeHalf(address - ram_base, value);
   }
   return value;
}

uint32_t Bus::readWord(uint64_t address)
{
   if (address >= ram_base && address + 3 < ram_base + ram.getSize())
   {
      return ram.readWord(address - ram_base);
   }
   return 0;
}

uint32_t Bus::writeWord(uint64_t address, uint32_t value)
{
   if (address >= ram_base && address + 3 < ram_base + ram.getSize())
   {
      return ram.writeWord(address - ram_base, value);
   }
   return value;
}

uint64_t Bus::readDouble(uint64_t address)
{
   if (address >= ram_base && address + 7 < ram_base + ram.getSize())
   {
      return ram.readDouble(address - ram_base);
   }
   return 0;
}

uint64_t Bus::writeDouble(uint64_t address, uint64_t value)
{
   if (address >= ram_base && address + 7 < ram_base + ram.getSize())
   {
      return ram.writeDouble(address - ram_base, value);
   }
   return value;
}