#include "Processor.h"
#include "Debug.h"
#include "IODevice.h"
#include "ConsoleIO.h"
#include "InjectionLoader.h"
#include "InstructionRegistry.h"
#include "DefaultRegistry.h"

int main(int argc, char *argv[])
{
   size_t memory_kb = 1024;
   bool debug_enabled = false;

   // Setup IO
   io = new ConsoleIO();

   // CLI check
   if (argc < 2)
   {
      io->writeString("Usage: rv64i <binary_file> [--memory kb] [--debug]\n");
      return 1;
   }

   const char *filename = argv[1];

   for (int i = 2; i < argc; i++)
   {
      std::string arg = argv[i];

      if (arg == "--debug")
      {
         debug_enabled = true;
      }
      else if (arg == "--memory")
      {
         if (i + 1 >= argc)
         {
            io->writeString("Error: --memory requires a value\n");
            return 1;
         }

         memory_kb = std::stoul(argv[i + 1]);
         i++; // skip next argument
      }
      else
      {
         io->writeString("Unknown argument: ");
         io->writeString(arg.c_str());
         io->writeString("\n");
         return 1;
      }
   }

   if (debug_enabled)
   {
      Debug::enable();
   }

   // Initialize the registry
   InstructionRegistry::init();
   register_rv64i();
   register_loads();

   // Setup memory + CPU
   Memory mem(memory_kb * 1024);
   DEBUG_LOG("Memory Initialized Successfully");

   Processor cpu(mem);
   DEBUG_LOG("Processor Initialized Successfully");

   // Load program
   InjectionLoader loader(filename);

   uint64_t text_end = loader.load(mem);
   if (text_end == 0)
   {
      io->writeString("Failed to load file.\n");
      return 1;
   }

   cpu.text_end = text_end;

   cpu.program_counter = 0;
   cpu.registers[2] = mem.getSize();

   cpu.run();

   // Debug output
   if (Debug::enabled())
   {
      DEBUG_LOG("Final register values after execution:");

      for (int i = 0; i < 32; i++)
      {
         DEBUG_BEGIN()
         io->writeString("x");
         io->writeInt(i);
         io->writeString(" - ");
         io->writeInt(cpu.registers[i]);
         DEBUG_END()
      }
   }

   return 0;
}