#include "Processor.h"
#include "Memory.h"
#include "Debug.h"
#include "IODevice.h"

const uint64_t STACK_GUARD = 8; // or 64 for safety

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
bool Processor::step()
{
   // Enforce text bounds
   if (program_counter >= text_end)
   {
      io->writeString("ERROR: PC out of bounds\n");
      return false;
   }

   // Fetch instruction
   uint32_t instruction = memory.readWord(program_counter);

   // Enforce stack size
   if (registers[2] <= text_end + STACK_GUARD)
   {
      io->writeString("ERROR: Stack size exceeds lower bound!\n");
      return false;
   }

   // Send instruction to interpreter
   if (!interpreter.handle(instruction, *this))
   {
      // Throw panic message
      return false;
   }

   // Enforce x0 = 0
   registers[0] = 0;

   // Enforce stack size
   if (registers[2] <= text_end + STACK_GUARD)
   {
      io->writeString("ERROR: Stack size exceeds lower bound post execution!\n");
      return false;
   }

   return true;
}

/**
 * Run program continuously
 */
void Processor::run()
{
   while (true)
   {
      bool success = step();
      if (!success)
         break;
   }
}