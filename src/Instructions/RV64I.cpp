#include "Debug.h"
#include "IODevice.h"
#include "Interpreter.h"
#include "Processor.h"
#include "InstructionRegistry.h"
#include "DefaultRegistry.h"

void exec_add(const DecodedInstruction &inst, Processor &processor)
{
   // add rd, rs1, rs2
   // Adds rs1 to rs2 and stores in rd
   processor.registers[inst.rd] =
       processor.registers[inst.rs1] +
       processor.registers[inst.rs2];

   processor.program_counter += 4;

   DEBUG_BEGIN()
   io->writeString("ADD x");
   io->writeInt(inst.rd);
   io->writeString(", x");
   io->writeInt(inst.rs1);
   io->writeString(", x");
   io->writeInt(inst.rs2);
   DEBUG_END()
}

void exec_sub(const DecodedInstruction &inst, Processor &processor)
{
   // sub rd,rs1,rs2
   // Subtracts rs2 from rs1 and stores in rd
   processor.registers[inst.rd] =
       processor.registers[inst.rs1] -
       processor.registers[inst.rs2];

   processor.program_counter += 4;
}

void exec_addi(const DecodedInstruction &inst, Processor &processor)
{
   // addi rd,rs1,imm
   // Adds rs1 to imm and stores in rd
   processor.registers[inst.rd] =
       processor.registers[inst.rs1] + inst.imm;

   processor.program_counter += 4;

   DEBUG_BEGIN()
   io->writeString("ADDI x");
   io->writeInt(inst.rd);
   io->writeString(", x");
   io->writeInt(inst.rs1);
   io->writeString(", ");
   io->writeSignedInt(inst.imm);
   DEBUG_END()
}

void exec_lw(const DecodedInstruction &inst, Processor &processor)
{
   // lw rd,offset(rs1)
   // Fetches a 32-bit word from memory address of rs1 + offset, stores in rd
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint32_t value = processor.memory.readWord(address);

   processor.registers[inst.rd] = static_cast<int32_t>(value);
   processor.program_counter += 4;

   DEBUG_BEGIN()
   io->writeString("LW x");
   io->writeInt(inst.rd);
   io->writeString(", ");
   io->writeInt(inst.imm);
   io->writeString("(x");
   io->writeInt(inst.rs1);
   io->writeString(") -> addr: ");
   io->writeInt(address);
   io->writeString(" -> value: ");
   io->writeInt(static_cast<int32_t>(value));
   DEBUG_END()
}

void exec_sw(const DecodedInstruction &inst, Processor &processor)
{
   // sw rs2,offset(rs1)
   // Stores value in rs2 at location of address in rs1 + offset.
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint32_t value = static_cast<uint32_t>(processor.registers[inst.rs2]);

   processor.memory.writeWord(address, value);
   processor.program_counter += 4;

   DEBUG_BEGIN()
   io->writeString("SW x");
   io->writeInt(inst.rs2);
   io->writeString(", ");
   io->writeInt(inst.imm);
   io->writeString("(x");
   io->writeInt(inst.rs1);
   io->writeString(") -> addr: ");
   io->writeInt(address);
   io->writeString(" <- value: ");
   io->writeInt(static_cast<int32_t>(value));
   DEBUG_END()
}

void exec_beq(const DecodedInstruction &inst, Processor &processor)
{
   int64_t old_pc = processor.program_counter;

   if (processor.registers[inst.rs1] == processor.registers[inst.rs2])
   {
      processor.program_counter = static_cast<int64_t>(processor.program_counter) + inst.imm;

      DEBUG_BEGIN()
      io->writeString("BEQ x");
      io->writeInt(inst.rs1);
      io->writeString(", x");
      io->writeInt(inst.rs2);
      io->writeString(" taken -> PC: ");
      io->writeInt(old_pc);
      io->writeString(" -> ");
      io->writeInt(processor.program_counter);
      DEBUG_END()
   }
   else
   {
      processor.program_counter += 4;

      DEBUG_BEGIN()
      io->writeString("BEQ x");
      io->writeInt(inst.rs1);
      io->writeString(", x");
      io->writeInt(inst.rs2);
      io->writeString(" not taken -> PC: ");
      io->writeInt(old_pc);
      io->writeString(" -> ");
      io->writeInt(processor.program_counter);
      DEBUG_END()
   }
}

void exec_jal(const DecodedInstruction &inst, Processor &processor)
{
   int64_t old_pc = processor.program_counter;
   processor.registers[inst.rd] = processor.program_counter + 4;

   int64_t current_pc = static_cast<int64_t>(processor.program_counter);
   processor.program_counter = current_pc + inst.imm;

   DEBUG_BEGIN()
   io->writeString("JAL x");
   io->writeInt(inst.rd);
   io->writeString(", ");
   io->writeSignedInt(static_cast<int64_t>(processor.program_counter) - static_cast<int64_t>(old_pc));
   io->writeString(" (PC: ");
   io->writeInt((int32_t)old_pc);
   io->writeString(" -> ");
   io->writeInt((int32_t)processor.program_counter);
   io->writeString(")");
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
}