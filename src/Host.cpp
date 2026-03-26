#include "Processor.h"
#include "Debug.h"
#include "IODevice.h"
#include "ConsoleIO.h"
#include "InjectionLoader.h"
#include "InstructionRegistry.h"
#include "DefaultRegistry.h"
#include "Utility.h"
#include "WindowsEnvironment.h"
#include "LoaderDevice.h"
#include "EnvironmentDevice.h"
#include "Bus.h"
#include "CLI.h"
#include "ISystem.h"
#include "SystemDevice.h"
#include "OpenSBISystem.h"

int main(int argc, char *argv[])
{
   // Module Initialization
   io = new ConsoleIO();
   env = new WindowsEnvironment();
   sys = new OpenSBISystem();
   loader = new InjectionLoader();

   // CLI Reads
   CLI config = parseCommandLine(argc, argv);
   if (!config.valid)
      return 1;

   if (config.debug_enabled)
      Debug::enable();

   if (config.trace_enabled)
      Debug::enableTrace();

   // Load operation registry
   InstructionRegistry::init();

   // Fetch ram base from system
   const uint64_t RAM_BASE = sys->getRamBase();

   // Initialize Components
   Memory mem(config.memory_kb * 1024);
   UART *uart = new UART(io, 0x10000000);
   Bus bus(mem, io, RAM_BASE, uart);
   Processor cpu(bus);
   DEBUG_LOG("Hardware Initialized Successfully");

   // Load payload into memory and boot
   loader->load(bus, RAM_BASE, config.filename);
   sys->boot(cpu, mem, bus);
   cpu.run();

   if (Debug::enabled())
   {
      Debug::printDebugRegisters(cpu);
   }

   delete loader;
   delete env;
   delete io;
   delete uart;

   return 0;
}