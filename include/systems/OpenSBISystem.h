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

      cpu.misa = (2ULL << 62) | (1 << 0) | (1 << 2) | (1 << 8) | (1 << 12) | (1 << 18) | (1 << 20);
      cpu.writeCSR(0x300, (3ULL << 11));
      cpu.writeCSR(0x3A0, 0x1F);
      cpu.writeCSR(0x3B0, 0xFFFFFFFFFFFFFFFFULL);

      cpu.registers[10] = 0;

      uint64_t dtb_address = (getRamBase() + mem.getSize() - (1024 * 1024)) & ~0x7ULL;
      cpu.registers[11] = dtb_address;
      cpu.registers[2] = dtb_address;

      uint64_t next_addr = loader->load(bus, dtb_address, "system.dtb");

      if (next_addr == 0)
      {
         io->writeString("Failed to load DTB.");
      }

      uint32_t magic_number = bus.readWord(dtb_address);
      io->writeString("\n--- DTB DIAGNOSTIC ---\n");
      io->writeString("DTB loaded at: ");
      io->writeInt(dtb_address);
      io->writeString("\nFirst 4 bytes (Magic Number): ");
      io->writeInt(magic_number);
      io->writeString("\n----------------------\n");
      uint32_t crash_instr = bus.readWord(0x80018BD4);
      io->writeString("\nInstruction at 0x80018BD4: 0x");
      io->writeInt(crash_instr);
      io->writeString("\n");
   }
};