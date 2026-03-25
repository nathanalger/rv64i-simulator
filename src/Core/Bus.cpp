#include "Bus.h"
#include "Processor.h"

Bus::Bus(Memory &ram_ref, IO *io_ptr, uint64_t base_address, UART *uart_ptr)
    : ram(ram_ref), io(io_ptr), ram_base(base_address), uart(uart_ptr) {}

// --- READ OPERATIONS ---

uint8_t Bus::readByte(uint64_t address)
{
   if (cpu && !cpu->checkPMP(address, Processor::AccessType::LOAD))
   {
      cpu->raiseTrap(TrapCause::LOAD_ACCESS_FAULT, cpu->program_counter, address);
      return 0;
   }

   if (uart && uart->contains(address))
      return uart->readByte(address);
   if (address >= ram_base && address < ram_base + ram.getSize())
      return ram.readByte(address - ram_base);

   cpu->raiseTrap(TrapCause::LOAD_ACCESS_FAULT, cpu->program_counter, address);
   return 0;
}

uint16_t Bus::readHalf(uint64_t address)
{
   if (cpu && !cpu->checkPMP(address, Processor::AccessType::LOAD))
   {
      cpu->raiseTrap(TrapCause::LOAD_ACCESS_FAULT, cpu->program_counter, address);
      return 0;
   }

   if (address >= ram_base && address + 1 <= ram_base + ram.getSize())
      return ram.readHalf(address - ram_base);

   cpu->raiseTrap(TrapCause::LOAD_ACCESS_FAULT, cpu->program_counter, address);
   return 0;
}

uint32_t Bus::readWord(uint64_t address)
{
   if (cpu && !cpu->checkPMP(address, Processor::AccessType::LOAD))
   {
      cpu->raiseTrap(TrapCause::LOAD_ACCESS_FAULT, cpu->program_counter, address);
      return 0;
   }

   // CLINT reads (mtime, mtimecmp, msip)
   if (cpu)
   {
      if (address == 0x02000000)
         return cpu->msip;
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
      return ram.readWord(address - ram_base);

   cpu->raiseTrap(TrapCause::LOAD_ACCESS_FAULT, cpu->program_counter, address);
   return 0;
}

uint64_t Bus::readDouble(uint64_t address)
{
   if (cpu && !cpu->checkPMP(address, Processor::AccessType::LOAD))
   {
      cpu->raiseTrap(TrapCause::LOAD_ACCESS_FAULT, cpu->program_counter, address);
      return 0;
   }

   if (cpu)
   {
      if (address == 0x0200BFF8)
         return cpu->mtime;
      if (address == 0x02004000)
         return cpu->mtimecmp;
   }

   if (address >= ram_base && address + 7 <= ram_base + ram.getSize())
      return ram.readDouble(address - ram_base);

   cpu->raiseTrap(TrapCause::LOAD_ACCESS_FAULT, cpu->program_counter, address);
   return 0;
}

// --- WRITE OPERATIONS ---

uint8_t Bus::writeByte(uint64_t address, uint8_t value)
{
   if (cpu && !cpu->checkPMP(address, Processor::AccessType::STORE))
   {
      cpu->raiseTrap(TrapCause::STORE_ACCESS_FAULT, cpu->program_counter, address);
      return value;
   }

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

uint16_t Bus::writeHalf(uint64_t address, uint16_t value)
{
   if (cpu && !cpu->checkPMP(address, Processor::AccessType::STORE))
   {
      cpu->raiseTrap(TrapCause::STORE_ACCESS_FAULT, cpu->program_counter, address);
      return value;
   }

   if (address >= ram_base && address + 1 <= ram_base + ram.getSize())
      return ram.writeHalf(address - ram_base, value);
   return value;
}

uint32_t Bus::writeWord(uint64_t address, uint32_t value)
{
   if (cpu && !cpu->checkPMP(address, Processor::AccessType::STORE))
   {
      cpu->raiseTrap(TrapCause::STORE_ACCESS_FAULT, cpu->program_counter, address);
      return value;
   }

   if (cpu)
   {
      if (address == 0x02000000)
      {
         cpu->msip = value & 1;
         uint64_t current_mip = cpu->readCSR(0x344);
         cpu->writeCSR(0x344, cpu->msip ? (current_mip | (1ULL << 3)) : (current_mip & ~(1ULL << 3)));
         return value;
      }
      if (address == 0x02004000)
      {
         cpu->mtimecmp = (cpu->mtimecmp & 0xFFFFFFFF00000000ULL) | value;
         return value;
      }
      if (address == 0x02004004)
      {
         cpu->mtimecmp = (cpu->mtimecmp & 0x00000000FFFFFFFFULL) | ((uint64_t)value << 32);
         return value;
      }
   }

   if (address >= ram_base && address + 3 <= ram_base + ram.getSize())
      return ram.writeWord(address - ram_base, value);
   return value;
}

uint64_t Bus::writeDouble(uint64_t address, uint64_t value)
{
   if (cpu && !cpu->checkPMP(address, Processor::AccessType::STORE))
   {
      cpu->raiseTrap(TrapCause::STORE_ACCESS_FAULT, cpu->program_counter, address);
      return value;
   }

   if (cpu && address == 0x02004000)
   {
      cpu->mtimecmp = value;
      return value;
   }

   if (address >= ram_base && address + 7 <= ram_base + ram.getSize())
      return ram.writeDouble(address - ram_base, value);
   return value;
}

void Bus::setProcessor(Processor *p) { cpu = p; }