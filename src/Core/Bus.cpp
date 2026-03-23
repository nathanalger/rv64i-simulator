#include "Bus.h"
#include "Processor.h"

Bus::Bus(Memory &ram_ref, IO *io_ptr, uint64_t base_address, UART *uart_ptr)
    : ram(ram_ref), io(io_ptr), ram_base(base_address), uart(uart_ptr) {}

// Helper to check if address is CLINT (0x02000000 - 0x0200FFFF)
bool is_clint(uint64_t addr)
{
   return addr >= 0x02000000 && addr < 0x02010000;
}

uint8_t Bus::readByte(uint64_t address)
{
   if (uart && uart->contains(address))
      return uart->readByte(address);
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
   if (uart && uart->contains(address))
      return uart->readByte(address);
   if (address >= ram_base && address + 1 <= ram_base + ram.getSize())
   {
      return ram.readHalf(address - ram_base);
   }
   return 0;
}

uint16_t Bus::writeHalf(uint64_t address, uint16_t value)
{
   if (uart && uart->contains(address))
   {
      uart->writeByte(address, (uint8_t)value);
      return value;
   }
   if (address >= ram_base && address + 1 <= ram_base + ram.getSize())
   {
      return ram.writeHalf(address - ram_base, value);
   }
   return value;
}

uint32_t Bus::readWord(uint64_t address)
{
   if (uart && uart->contains(address))
      return uart->readByte(address);

   if (cpu)
   {
      if (address == 0x0200BFF8)
         return (uint32_t)(cpu->mtime & 0xFFFFFFFF);
      if (address == 0x0200BFFC)
         return (uint32_t)(cpu->mtime >> 32);
      if (address == 0x02004000)
         return (uint32_t)(cpu->mtimecmp & 0xFFFFFFFF);
      if (address == 0x02004004)
         return (uint32_t)(cpu->mtimecmp >> 32);
   }

   if (address >= ram_base && address + 3 <= ram_base + ram.getSize())
   {
      return ram.readWord(address - ram_base);
   }
   return 0;
}

uint32_t Bus::writeWord(uint64_t address, uint32_t value)
{
   if (uart && uart->contains(address))
   {
      uart->writeByte(address, (uint8_t)value);
      return value;
   }
   if (address >= ram_base && address + 3 <= ram_base + ram.getSize())
   {
      return ram.writeWord(address - ram_base, value);
   }
   return value;
}

uint64_t Bus::readDouble(uint64_t address)
{
   // OpenSBI often uses 64-bit loads for time
   if (cpu)
   {
      if (address == 0x0200BFF8)
         return cpu->mtime;
      if (address == 0x02004000)
         return cpu->mtimecmp;
   }

   if (address >= ram_base && address + 7 <= ram_base + ram.getSize())
   {
      return ram.readDouble(address - ram_base);
   }
   return 0;
}

uint64_t Bus::writeDouble(uint64_t address, uint64_t value)
{
   if (cpu && address == 0x02004000)
   {
      cpu->mtimecmp = value;
      return value;
   }

   if (address >= ram_base && address + 7 <= ram_base + ram.getSize())
   {
      return ram.writeDouble(address - ram_base, value);
   }
   return value;
}

void Bus::setProcessor(Processor *p)
{
   cpu = p;
}