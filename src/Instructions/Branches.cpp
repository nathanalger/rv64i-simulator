#include "Debug.h"
#include "IODevice.h"
#include "Interpreter.h"
#include "Processor.h"
#include "InstructionRegistry.h"
#include "DefaultRegistry.h"

void exec_bne(const DecodedInstruction &inst, Processor &processor)
{
   int64_t old_pc = processor.program_counter;

   if (processor.registers[inst.rs1] != processor.registers[inst.rs2])
   {
      processor.program_counter = old_pc + inst.imm;
   }
   else
   {
      processor.program_counter += 4;
   }
}

void exec_blt(const DecodedInstruction &inst, Processor &processor)
{
   int64_t old_pc = processor.program_counter;

   if ((int64_t)processor.registers[inst.rs1] < (int64_t)processor.registers[inst.rs2])
   {
      processor.program_counter = old_pc + inst.imm;
   }
   else
   {
      processor.program_counter += 4;
   }
}

void exec_bge(const DecodedInstruction &inst, Processor &processor)
{
   int64_t old_pc = processor.program_counter;

   if ((int64_t)processor.registers[inst.rs1] >= (int64_t)processor.registers[inst.rs2])
   {
      processor.program_counter = old_pc + inst.imm;
   }
   else
   {
      processor.program_counter += 4;
   }
}

void exec_bltu(const DecodedInstruction &inst, Processor &processor)
{
   int64_t old_pc = processor.program_counter;

   if ((uint64_t)processor.registers[inst.rs1] < (uint64_t)processor.registers[inst.rs2])
   {
      processor.program_counter = old_pc + inst.imm;
   }
   else
   {
      processor.program_counter += 4;
   }
}

void exec_bgeu(const DecodedInstruction &inst, Processor &processor)
{
   int64_t old_pc = processor.program_counter;

   if ((uint64_t)processor.registers[inst.rs1] >= (uint64_t)processor.registers[inst.rs2])
   {
      processor.program_counter = old_pc + inst.imm;
   }
   else
   {
      processor.program_counter += 4;
   }
}

void exec_jalr(const DecodedInstruction &inst, Processor &processor)
{
   int64_t old_pc = processor.program_counter;

   int64_t target = (processor.registers[inst.rs1] + inst.imm) & ~1;

   processor.registers[inst.rd] = old_pc + 4;
   processor.program_counter = target;
}

void DefaultRegistry::register_branch_jump()
{
   // Branches
   InstructionRegistry::register_b(0x63, 0b001, exec_bne);
   InstructionRegistry::register_b(0x63, 0b100, exec_blt);
   InstructionRegistry::register_b(0x63, 0b101, exec_bge);
   InstructionRegistry::register_b(0x63, 0b110, exec_bltu);
   InstructionRegistry::register_b(0x63, 0b111, exec_bgeu);

   // JALR
   InstructionRegistry::register_i(0x67, 0b000, exec_jalr);
}