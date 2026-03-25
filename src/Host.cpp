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
#include "OpenSBISystem.h"
#include "DefaultSystem.h"

int main(int argc, char *argv[])
{
   io = new ConsoleIO();
   env = new WindowsEnvironment();

   // Debug::enableTrace();

   CLI config = parseCommandLine(argc, argv);
   if (!config.valid)
      return 1;

   if (config.debug_enabled)
   {
      Debug::enable();
   }

   InstructionRegistry::init();

   OpenSBISystem system;
   const uint64_t RAM_BASE = system.getRamBase();

   Memory mem(config.memory_kb * 1024);
   UART *my_uart = new UART(io, 0x10000000);
   Bus bus(mem, io, RAM_BASE, my_uart);
   Processor cpu(bus);
   DEBUG_LOG("Hardware Initialized Successfully");

   loader = new InjectionLoader();
   loader->load(bus, RAM_BASE, config.filename);

   system.boot(cpu, mem, bus);

   cpu.run();

   if (Debug::enabled())
   {
      Debug::printDebugRegisters(cpu);
   }

   delete loader;
   delete env;
   delete io;
   delete my_uart;

   return 0;
}