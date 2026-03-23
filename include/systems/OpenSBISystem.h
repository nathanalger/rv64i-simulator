#pragma once
#include "ISystem.h"
#include "ILoader.h"
#include "InjectionLoader.h"

class OpenSBISystem : public ISystem
{
public:
   /**
    * loader should point to a dtb file.
    */
   OpenSBISystem() {};

   uint64_t getRamBase() const override
   {
      return 0x80000000;
   }

   void boot(Processor &cpu, Memory &mem, Bus &bus)
   {
      DEBUG_BEGIN()
      io->writeString("Booting with ");
      io->writeInt(mem.getSize());
      io->writeString(" bytes of virtual memory.");
      DEBUG_END()

      cpu.program_counter = getRamBase();

      cpu.writeCSR(0x301, (2ULL << 62) | (1 << 0) | (1 << 3) | (1 << 5) | (1 << 8) | (1 << 12) | (1 << 18) | (1 << 20));
      cpu.writeCSR(0x300, (3ULL << 11));
      cpu.writeCSR(0x3A0, 0x1F);
      cpu.writeCSR(0x3B0, 0xFFFFFFFFFFFFFFFFULL);

      cpu.registers[2] = getRamBase() + mem.getSize();

      cpu.registers[10] = 0;

      uint64_t dtb_address = getRamBase() + mem.getSize() - (1024 * 1024);
      cpu.registers[11] = dtb_address;

      uint64_t next_addr = loader->load(bus, dtb_address, "system.dtb");

      if (next_addr == 0)
      {
         io->writeString("Failed to load DTB.");
      }
   }
};