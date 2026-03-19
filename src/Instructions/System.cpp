#include "EnvironmentDevice.h"
#include "Debug.h"
#include "IODevice.h"
#include "Interpreter.h"
#include "Processor.h"
#include "InstructionRegistry.h"
#include "DefaultRegistry.h"

static void exec_system(const DecodedInstruction &inst, Processor &processor)
{
   if (inst.imm == 0)
   {
      if (env)
         env->handle_ecall(processor);
   }
   else if (inst.imm == 1)
   {
      if (env)
         env->handle_ebreak(processor);
   }

   // Advance the program counter after returning from the environment trap
   processor.program_counter += 4;
}

void DefaultRegistry::register_system_operations()
{
   InstructionRegistry::register_i(0x73, 0b000, exec_system);
}