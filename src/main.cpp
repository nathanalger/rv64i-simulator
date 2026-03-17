#include "Processor.h"
#include "Debug.h"
#include "IODevice.h"
#include "ConsoleIO.h"

int main()
{
   // Select debug mode
   Debug::enable();

   // Select IO device (currently OS console)
   io = new ConsoleIO();

   // Create a central processor with 1GB of RAM.
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

   DEBUG_BEGIN()
   io->writeString("Register x3 value: ");
   io->writeInt(cpu.registers[3]);
   io->writeChar('\n');
   DEBUG_END()

   return 0;
}