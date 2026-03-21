#pragma once
#include "ISystem.h"

class DefaultSystem : public ISystem
{
public:
   DefaultSystem() {};
   uint64_t getRamBase() const override
   {
      return 0x80000000;
   }

   void boot(Processor &cpu, Memory &mem, Bus &) override
   {
      cpu.program_counter = getRamBase();
      cpu.registers[2] = mem.getSize();
   }
};