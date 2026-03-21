#include "Processor.h"
#include "Debug.h"
#include "IODevice.h"
#include "DefaultRegistry.h"
#include "InstructionRegistry.h"
#include "LoaderDevice.h"
#include "Bus.h"
#include "DefaultSystem.h"

/**
 * This is an entrypoint for the simulator that is intended to be used on an operating system.
 * It relies on standard IO provided by the OS.
 */
int main()
{
   // Select IO device (no current IO device for baremetal implementation)
   io = nullptr;
   loader = nullptr;
   env = nullptr;

   // Initialize the registry
   InstructionRegistry::init();

   ISystem *system = new DefaultSystem();

   // Create a central processor with 1MB of RAM.
   Memory mem(1024 * 1024);
   Bus bus(mem, io, 0);
   Processor cpu(bus);

   system->boot(cpu, mem, bus);
   cpu.run();

   delete system;
   delete io;
   delete loader;
   delete env;

   return 0;
}