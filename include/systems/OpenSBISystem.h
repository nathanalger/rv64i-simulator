#pragma once
#include "ISystem.h"
#include "DTBBuilder.h"
#include "Debug.h"
#include <fstream>

class OpenSBISystem : public ISystem
{
public:
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
      io->writeString(" bytes of virtual memory. \n");
      DEBUG_END()

      cpu.program_counter = getRamBase();

      uint64_t uart_base = 0x10000000;
      uint64_t clint_base = 0x02000000;

      cpu.misa = RiscVFeatures::RV64 |
                 RiscVFeatures::I | RiscVFeatures::M | RiscVFeatures::A | RiscVFeatures::C |
                 RiscVFeatures::S | RiscVFeatures::U | RiscVFeatures::F;

      char system_isa[32] = {0};
      DTBBuilder::generate_isa_string(cpu.misa, system_isa, sizeof(system_isa));

      cpu.writeCSR(0x300, (3ULL << 11));
      cpu.writeCSR(0x3A0, 0x1F);
      cpu.writeCSR(0x3B0, 0xFFFFFFFFFFFFFFFFULL);

      cpu.registers[10] = 0;

      uint64_t dtb_address = (getRamBase() + mem.getSize() - (1024 * 1024)) & ~0x7ULL;
      cpu.registers[11] = dtb_address;
      cpu.registers[2] = dtb_address;

      char fdt_buf[4096] = {0};
      int fdt_size = DTBBuilder::build(fdt_buf, sizeof(fdt_buf), getRamBase(), mem.getSize(), uart_base, clint_base, system_isa);

      if (fdt_size <= 0)
      {
         io->writeString("Failed to build DTB. Buffer may be too small.\n");
         return;
      }

      for (int i = 0; i < fdt_size; i++)
      {
         bus.writeByte(dtb_address + i, (uint8_t)fdt_buf[i]);
      }

      DEBUG_BEGIN()
      io->writeString("Generated and injected DTB successfully!\n");
      DEBUG_END()
   }
};