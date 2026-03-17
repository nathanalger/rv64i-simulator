#include "Processor.h"
#include "Debug.h"
#include "IODevice.h"
#include "ConsoleIO.h"

/**
 * This is an entrypoint for the simulator that is intended to be used on an operating system.
 * It relies on standard IO provided by the OS.
 */
int main()
{
   // Select debug mode
   Debug::enable();

   // Select IO device (currently OS console)
   io = new ConsoleIO();

   // Create a central processor with 1MB of RAM.
   Memory mem(1024 * 1024);
   DEBUG_LOG("Memory Initialized Successfully");

   Processor cpu(mem);
   DEBUG_LOG("Processor Initialized Successfully");

   cpu.memory.writeWord(0, 0x00500093); // addi x1, x0, 5
   cpu.memory.writeWord(4, 0x00a00113); // addi x2, x0, 10
   cpu.memory.writeWord(8, 0x002081b3); // add x3, x1, x2

   cpu.program_counter = 0;

   cpu.step();
   cpu.step();
   cpu.step();

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