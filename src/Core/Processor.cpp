#include "Processor.h"
#include "Memory.h"

Processor::Processor(Memory &mem) : memory(mem)
{
   initialize();
}

void Processor::reset()
{
   // Set all registers to a low state
   for (int i = 0; i < 32; i++)
   {
      registers[i] = 0;
   }

   // Reset program counter
   program_counter = 0;
};

void Processor::initialize()
{
   // Set all registers to a low state
   for (int i = 0; i < 32; i++)
   {
      registers[i] = 0;
   }

   // Reset program counter
   program_counter = 0;
};

/**
 * Execute a single instruction
 */
void Processor::step()
{
   // Fetch instruction
   uint32_t instruction = memory.readWord(program_counter);

   // Send instruction to interpreter
   interpreter.handle(instruction, *this);

   // Enforce x0 = 0
   registers[0] = 0;
}

/**
 * Run program continuously
 */
void Processor::run()
{
   while (true)
   {
      step();
   }
}