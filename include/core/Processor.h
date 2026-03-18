#pragma once
#include "Interpreter.h"
#include "Memory.h"
#include <cstdint>

/**
 * Stores the information on the processor, specifically register values and the program counter.
 * Additionally contains some helper functions.
 */
class Processor
{
public:
   // Array 32 count 64-bit width registers
   uint64_t registers[32];

   // 64 bit width integer that stores current instruction address
   uint64_t program_counter;

   uint64_t text_end;

   Memory &memory;
   uint32_t memory_size;

   Interpreter interpreter;

   /**
    * Default constructor
    * Creates processor with default memory size (1 MB)
    */
   Processor(Memory &mem);

   /**
    * Reset the processor to its base state.
    */
   void reset();

   /**
    * Execute the next step in the program execution.
    */
   bool step();

   /**
    * Trigger the processor to run the entire provided program.
    */
   void run();

private:
   /**
    * Initialize a processor instance
    */
   void initialize();
};