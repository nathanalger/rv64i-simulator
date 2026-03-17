#include "Processor.h"
#include <iostream>

int main()
{
   // Create a central processor with 1GB of RAM.
   Processor cpu(1024 * 1024 * 1024);

   cpu.memory.writeWord(0, 0x00500093); // addi x1, x0, 5
   cpu.memory.writeWord(4, 0x00a00113); // addi x2, x0, 10
   cpu.memory.writeWord(8, 0x002081b3); // add x3, x1, x2

   cpu.program_counter = 0;

   cpu.step();
   cpu.step();
   cpu.step();

   std::cout << cpu.registers[3] << std::endl; // should be 15

   return 0;
}