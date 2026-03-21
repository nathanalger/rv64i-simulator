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
      cpu.program_counter = getRamBase();

      // Stack pointer to top of RAM
      cpu.registers[2] = getRamBase() + mem.getSize();

      // a0 (x10) = Hart ID 0
      cpu.registers[10] = 0;

      // a1 (x11) = DTB Address (1MB from the end of RAM)
      uint64_t dtb_address = getRamBase() + mem.getSize() - (1024 * 1024);
      cpu.registers[11] = dtb_address;

      // Use the abstract loader to push the DTB into the bus
      const char *sysfilename = "system.dtb";
      if (loader->load(bus, dtb_address, sysfilename))
      {
         uint64_t next_addr = loader->load(bus, dtb_address, "system.dtb");
         if (next_addr == 0)
         {
            // Log: "Failed to load DTB!"
         }
      }
   }
};