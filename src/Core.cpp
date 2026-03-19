#include "Processor.h"
#include "Debug.h"
#include "IODevice.h"
#include "DefaultRegistry.h"
#include "InstructionRegistry.h"

/**
 * This is an entrypoint for the simulator that is intended to be used on an operating system.
 * It relies on standard IO provided by the OS.
 */
int main()
{
   // Select IO device (no current IO device for baremetal implementation)
   io = nullptr;

   // Initialize the registry
   InstructionRegistry::init();

   // Create a central processor with 1MB of RAM.
   Memory mem(1024 * 1024);
   Processor cpu(mem);

   cpu.run();
   return 0;
}