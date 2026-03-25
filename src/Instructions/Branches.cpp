#include "Debug.h"
#include "IODevice.h"
#include "Interpreter.h"
#include "Processor.h"
#include "InstructionRegistry.h"
#include "DefaultRegistry.h"

void exec_bne(const DecodedInstruction &inst, Processor &processor)
{
   // branch relative to the start of this instruction
   if (processor.registers[inst.rs1] != processor.registers[inst.rs2])
   {
      processor.program_counter = inst.pc + inst.imm;
   }
   // Else: Do nothing. Processor::step will add inst.length automatically.
}

void exec_blt(const DecodedInstruction &inst, Processor &processor)
{
   if (static_cast<int64_t>(processor.registers[inst.rs1]) < static_cast<int64_t>(processor.registers[inst.rs2]))
   {
      processor.program_counter = inst.pc + inst.imm;
   }
}

void exec_bge(const DecodedInstruction &inst, Processor &processor)
{
   if (static_cast<int64_t>(processor.registers[inst.rs1]) >= static_cast<int64_t>(processor.registers[inst.rs2]))
   {
      processor.program_counter = inst.pc + inst.imm;
   }
}

void exec_bltu(const DecodedInstruction &inst, Processor &processor)
{
   if (static_cast<uint64_t>(processor.registers[inst.rs1]) < static_cast<uint64_t>(processor.registers[inst.rs2]))
   {
      processor.program_counter = inst.pc + inst.imm;
   }
}

void exec_bgeu(const DecodedInstruction &inst, Processor &processor)
{
   if (static_cast<uint64_t>(processor.registers[inst.rs1]) >= static_cast<uint64_t>(processor.registers[inst.rs2]))
   {
      processor.program_counter = inst.pc + inst.imm;
   }
}

void exec_jalr(const DecodedInstruction &inst, Processor &processor)
{
   // 1. Calculate the target (rs1 + imm) and clear the lowest bit
   int64_t target = (processor.registers[inst.rs1] + inst.imm) & ~1;

   // 2. Link register stores the address of the NEXT instruction
   // We use inst.length (2 or 4) to handle compressed JALR expansions correctly
   processor.write_reg(inst.rd, inst.pc + inst.length);

   // 3. Update the PC
   processor.program_counter = target;

   TRACE_BEGIN()
   Debug::writeString("JALR x");
   Debug::writeInt(inst.rd);
   Debug::writeString(", x");
   Debug::writeInt(inst.rs1);
   Debug::writeString(" -> PC: ");
   Debug::writeInt(processor.program_counter);
   DEBUG_END()
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