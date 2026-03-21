#include "Memory.h"

// Initializer and destructor
Memory::Memory(size_t size) : size(size)
{
   memory = new uint8_t[size];

   for (size_t i = 0; i < size; i++)
      memory[i] = 0;
}

Memory::~Memory()
{
   delete[] memory;
}

uint8_t Memory::readByte(uint64_t address)
{
   if (address < size)
   {
      return memory[address];
   }
   return 0; // Safe fallback
}

uint8_t Memory::writeByte(uint64_t address, uint8_t value)
{
   if (address < size)
   {
      memory[address] = value;
   }
   return value;
}

uint16_t Memory::readHalf(uint64_t address)
{
   if (address + 1 < size)
   {
      uint16_t value = 0;
      value |= memory[address];
      value |= memory[address + 1] << 8;
      return value;
   }
   return 0;
}

uint16_t Memory::writeHalf(uint64_t address, uint16_t value)
{
   if (address + 1 < size)
   {
      memory[address] = value & 0xFF;
      memory[address + 1] = (value >> 8) & 0xFF;
   }
   return value;
}

uint32_t Memory::readWord(uint64_t address)
{
   if (address + 3 < size)
   {
      return (uint32_t)memory[address + 0] |
             ((uint32_t)memory[address + 1] << 8) |
             ((uint32_t)memory[address + 2] << 16) |
             ((uint32_t)memory[address + 3] << 24);
   }
   return 0;
}

uint32_t Memory::writeWord(uint64_t address, uint32_t value)
{
   if (address + 3 < size)
   {
      memory[address + 0] = value & 0xFF;
      memory[address + 1] = (value >> 8) & 0xFF;
      memory[address + 2] = (value >> 16) & 0xFF;
      memory[address + 3] = (value >> 24) & 0xFF;
   }
   return value;
}

uint64_t Memory::readDouble(uint64_t address)
{
   if (address + 7 < size)
   {
      return (uint64_t)memory[address + 0] |
             ((uint64_t)memory[address + 1] << 8) |
             ((uint64_t)memory[address + 2] << 16) |
             ((uint64_t)memory[address + 3] << 24) |
             ((uint64_t)memory[address + 4] << 32) |
             ((uint64_t)memory[address + 5] << 40) |
             ((uint64_t)memory[address + 6] << 48) |
             ((uint64_t)memory[address + 7] << 56);
   }
   return 0;
}

uint64_t Memory::writeDouble(uint64_t address, uint64_t value)
{
   if (address + 7 < size)
   {
      memory[address + 0] = value & 0xFF;
      memory[address + 1] = (value >> 8) & 0xFF;
      memory[address + 2] = (value >> 16) & 0xFF;
      memory[address + 3] = (value >> 24) & 0xFF;
      memory[address + 4] = (value >> 32) & 0xFF;
      memory[address + 5] = (value >> 40) & 0xFF;
      memory[address + 6] = (value >> 48) & 0xFF;
      memory[address + 7] = (value >> 56) & 0xFF;
   }
   return value;
}

uint64_t Memory::getSize()
{
   return size;
}