#pragma once
#include <cstdint>

/**
 * Stores the information on the processor, specifically register values and the program counter.
 * Additionally contains some helper functions.
 */
class Processor {
   public:
      // Array 32 count 64-bit width registers 
      uint64_t registers[32];

      // 64 bit width integer that stores current instruction address
      uint64_t program_counter;

      // Constructor
      Processor();

      // Reset program to default state
      void reset();
};