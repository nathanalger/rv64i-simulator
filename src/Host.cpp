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

int main(int argc, char *argv[])
{
   io = new ConsoleIO();
   env = new WindowsEnvironment();

   CLI config = parseCommandLine(argc, argv);
   if (!config.valid)
      return 1;

   if (config.debug_enabled)
   {
      Debug::enable();
   }

   InstructionRegistry::init();

   Memory mem(config.memory_kb * 1024);
   DEBUG_LOG("Memory Initialized Successfully");
   UART *my_uart = new UART(io, 0x10000000);

   const uint64_t RAM_BASE = 0x80000000;
   Bus bus(mem, io, RAM_BASE, my_uart);

   Processor cpu(bus);
   DEBUG_LOG("Processor Initialized Successfully");

   loader = new InjectionLoader(config.filename.c_str());
   loader->load(bus, RAM_BASE);

   cpu.program_counter = RAM_BASE;
   cpu.registers[2] = mem.getSize();

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