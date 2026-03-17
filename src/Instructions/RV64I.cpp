#include "InstructionExecutors.h"

void exec_add(const DecodedInstruction &inst, Processor &processor)
{
   // add rd, rs1, rs2
   // Adds rs1 to rs2 and stores in rd
   processor.registers[inst.rd] =
       processor.registers[inst.rs1] +
       processor.registers[inst.rs2];

   processor.program_counter += 4;
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
}

void exec_lw(const DecodedInstruction &inst, Processor &processor)
{
   // lw rd,offset(rs1)
   // Fetches a 32-bit word from memory address of rs1 + offset, stores in rd
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint32_t value = processor.memory.readWord(address);

   processor.registers[inst.rd] = static_cast<int32_t>(value);
   processor.program_counter += 4;
}

void exec_sw(const DecodedInstruction &inst, Processor &processor)
{
   // sw rs2,offset(rs1)
   // Stores value in rs2 at location of address in rs1 + offset.
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint32_t value = static_cast<uint32_t>(processor.registers[inst.rs2]);

   processor.memory.writeWord(address, value);
   processor.program_counter += 4;
}

void exec_beq(const DecodedInstruction &inst, Processor &processor)
{
   // beq rs1,rs2,offset
   // Branch if rs1 and rs2 are equal to location offset
   if (inst.rs1 == inst.rs2)
   {
      processor.program_counter += inst.pc + inst.imm;
   }
   else
   {
      processor.program_counter += 4;
   }
}

void exec_jal(const DecodedInstruction &inst, Processor &processor)
{
   // jal rd,offset
   // Jump to address and place return address in rd
   processor.registers[inst.rd] = inst.pc + 4;
   processor.program_counter = inst.pc + inst.imm;
}