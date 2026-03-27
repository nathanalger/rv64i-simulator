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
#include "DefaultSystem.h"
#include "OpenSBISystem.h"

int main(int argc, char *argv[])
{
   // Module Initialization
   io = new ConsoleIO();
   env = new WindowsEnvironment();
   loader = new InjectionLoader();

   // CLI Reads
   CLI config = parseCommandLine(argc, argv);

   if (config.systemType == SystemType::OpenSBI)
   {
      if (config.memory_kb < 50000)
         config.memory_kb = 50000;
      sys = new OpenSBISystem();
   }
   else
   {
      sys = new DefaultSystem();
   }

   if (!config.valid)
      return 1;

   if (config.debug_enabled)
      Debug::enable();

   if (config.trace_enabled)
      Debug::enableTrace();

   // Load operation registry
   InstructionRegistry::init();

   // Fetch ram base from system
   const uint64_t PAYLOAD_BASE = sys->payloadLocation();

   // Initialize Components
   Memory mem(config.memory_kb * 1024);
   UART *uart = new UART(io, 0x10000000);
   Bus bus(mem, io, sys->getRamBase(), uart);
   Processor cpu(bus);
   DEBUG_LOG("Hardware Initialized Successfully");

   // Load payload into memory and boot
   uint64_t payloadLoaded = loader->load(bus, PAYLOAD_BASE, config.filename);
   if (!payloadLoaded)
   {
      io->writeString("Payload file \"");
      io->writeString(config.filename);
      io->writeString("\" is not available or is incompatible.");
   }

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