#include "Processor.h"

Processor::Processor() {
   reset();
}

void Processor::reset() {
   // Set all registers to a low state
   for (int i = 0; i < 32; i++) {
      registers[i] = 0;
   }

   // Reset program counter
   program_counter = 0;
}