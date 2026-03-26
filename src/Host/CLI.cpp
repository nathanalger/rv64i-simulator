#include "CLI.h"
#include "IODevice.h"

CLI parseCommandLine(int argc, char *argv[])
{
   CLI config;

   if (argc < 2)
   {
      if (io)
         io->writeString("Usage: rv64i <binary_file> [--memory kb] [--debug] [--trace]\n");
      return config;
   }

   config.filename = argv[1];
   config.valid = true;

   for (int i = 2; i < argc; i++)
   {
      std::string arg = argv[i];

      if (arg == "--debug")
      {
         config.debug_enabled = true;
      }
      else if (arg == "--trace")
      {
         config.trace_enabled = true;
      }
      else if (arg == "--memory")
      {
         if (i + 1 >= argc)
         {
            if (io)
               io->writeString("Error: --memory requires a value\n");
            config.valid = false;
            return config;
         }
         config.memory_kb = std::stoul(argv[i + 1]);
         i++;
      }
      else
      {
         if (io)
         {
            io->writeString("Unknown argument: ");
            io->writeString(arg.c_str());
            io->writeString("\n");
         }
         config.valid = false;
         return config;
      }
   }
   return config;
}