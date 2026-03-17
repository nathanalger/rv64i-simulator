#include "Processor.h"
#include <iostream>
#include "Debug.h"

int main()
{
   Debug::enable();

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

   if (cpu.registers[3] == 15)
      std::cout << "15" << std::endl;

   return 0;
}