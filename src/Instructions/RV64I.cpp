#include "Debug.h"
#include "IODevice.h"
#include "Interpreter.h"
#include "Processor.h"
#include "InstructionRegistry.h"
#include "DefaultRegistry.h"

void exec_add(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] =
       processor.registers[inst.rs1] +
       processor.registers[inst.rs2];

   TRACE_BEGIN()
   Debug::writeString("ADD x");
   Debug::writeInt(inst.rd);
   Debug::writeString(", x");
   Debug::writeInt(inst.rs1);
   Debug::writeString(", x");
   Debug::writeInt(inst.rs2);
   DEBUG_END()
}

void exec_sub(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] =
       processor.registers[inst.rs1] -
       processor.registers[inst.rs2];
}

void exec_addi(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] =
       processor.registers[inst.rs1] + inst.imm;

   TRACE_BEGIN()
   Debug::writeString("ADDI x");
   Debug::writeInt(inst.rd);
   Debug::writeString(", x");
   Debug::writeInt(inst.rs1);
   Debug::writeString(", ");
   Debug::writeHex(inst.imm);
   DEBUG_END()
}

void exec_lw(const DecodedInstruction &inst, Processor &processor)
{
   if (inst.rd == 0)
      return;

   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint32_t value = processor.bus.readWord(address);

   processor.write_reg(inst.rd, static_cast<int32_t>(value));

   TRACE_BEGIN()
   Debug::writeString("LW x");
   Debug::writeInt(inst.rd);
   Debug::writeString(", ");
   Debug::writeInt(inst.imm);
   Debug::writeString("(x");
   Debug::writeInt(inst.rs1);
   Debug::writeString(") -> addr: ");
   Debug::writeInt(address);
   Debug::writeString(" -> value: ");
   Debug::writeInt(static_cast<int32_t>(value));
   DEBUG_END()
}

void exec_sw(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint32_t value = static_cast<uint32_t>(processor.registers[inst.rs2]);

   processor.bus.writeWord(address, value);

   TRACE_BEGIN()
   Debug::writeString("SW x");
   Debug::writeInt(inst.rs2);
   Debug::writeString(", ");
   Debug::writeInt(inst.imm);
   Debug::writeString("(x");
   Debug::writeInt(inst.rs1);
   Debug::writeString(") -> addr: ");
   Debug::writeInt(address);
   Debug::writeString(" <- value: ");
   Debug::writeInt(static_cast<int32_t>(value));
   DEBUG_END()
}

void exec_beq(const DecodedInstruction &inst, Processor &processor)
{
   int64_t current_pc = inst.pc;

   if (processor.registers[inst.rs1] == processor.registers[inst.rs2])
   {
      processor.program_counter = current_pc + inst.imm;

      TRACE_BEGIN()
      Debug::writeString("BEQ x");
      Debug::writeInt(inst.rs1);
      Debug::writeString(", x");
      Debug::writeInt(inst.rs2);
      Debug::writeString(" taken -> PC: ");
      Debug::writeInt(current_pc);
      Debug::writeString(" -> ");
      Debug::writeInt(processor.program_counter);
      DEBUG_END()
   }
   else
   {
      TRACE_BEGIN()
      Debug::writeString("BEQ x");
      Debug::writeInt(inst.rs1);
      Debug::writeString(", x");
      Debug::writeInt(inst.rs2);
      Debug::writeString(" not taken.");
      DEBUG_END()
   }
}

void exec_jal(const DecodedInstruction &inst, Processor &processor)
{
   processor.write_reg(inst.rd, inst.pc + inst.length);

   processor.program_counter = inst.pc + inst.imm;

   TRACE_BEGIN()
   Debug::writeString("JAL x");
   Debug::writeInt(inst.rd);
   Debug::writeString(", offset ");
   Debug::writeHex(inst.imm);
   Debug::writeString(" (PC: ");
   Debug::writeInt(inst.pc);
   Debug::writeString(" -> ");
   Debug::writeInt(processor.program_counter);
   Debug::writeString(")");
   DEBUG_END()
}

// Pipeline
void exec_fence(const DecodedInstruction &, Processor &)
{
   TRACE_BEGIN()
   Debug::writeString("FENCE...\n");
   DEBUG_END()
}

void exec_sfence_vma(const DecodedInstruction &, Processor &)
{
   TRACE_BEGIN()
   Debug::writeString("SFENCE.VMA... \n");
   DEBUG_END()
}

void exec_fence_i(const DecodedInstruction &, Processor &)
{
   TRACE_BEGIN()
   Debug::writeString("FENCE.I");
   DEBUG_END()
}

// Built in core extension
void DefaultRegistry::register_rv64i()
{
   InstructionRegistry::register_r(0x33, 0b000, 0b0000000, exec_add);
   InstructionRegistry::register_r(0x33, 0b000, 0b0100000, exec_sub);

   InstructionRegistry::register_i(0x13, 0b000, exec_addi);

   InstructionRegistry::register_i(0x03, 0b010, exec_lw);
   InstructionRegistry::register_s(0x23, 0b010, exec_sw);

   InstructionRegistry::register_b(0x63, 0b000, exec_beq);

   InstructionRegistry::register_opcode(0x6F, exec_jal);

   InstructionRegistry::register_i(0x0F, 0b000, exec_fence);
   InstructionRegistry::register_r(0x73, 0b000, 0x09, exec_sfence_vma);
   InstructionRegistry::register_i(0x0F, 0b001, exec_fence_i);
}