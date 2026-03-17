#include "Processor.h"
#include "Memory.h"

Processor::Processor() : memory(1024 * 1024)
{
   initialize(1024 * 1024);
}

Processor::Processor(size_t size) : memory(size)
{
   initialize(size);
}

void Processor::initialize(size_t size)
{
   memory_size = size;
   reset();
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

void Processor::initialize(size_t size)
{
   // Set all registers to a low state
   for (int i = 0; i < 32; i++)
   {
      registers[i] = 0;
   }

   // Reset program counter
   program_counter = 0;

   memory = Memory::Memory(size);
};

/**
 * Execute a single instruction
 */
void Processor::step()
{
   // Fetch instruction
   uint32_t instruction = memory.readWord(program_counter);

   // Advance program counter by one instruction
   program_counter += 4;

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