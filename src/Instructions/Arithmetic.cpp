#include "Debug.h"
#include "IODevice.h"
#include "Interpreter.h"
#include "Processor.h"
#include "InstructionRegistry.h"
#include "DefaultRegistry.h"

void exec_and(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = processor.registers[inst.rs1] & processor.registers[inst.rs2];
   processor.program_counter += 4;
}

void exec_or(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = processor.registers[inst.rs1] | processor.registers[inst.rs2];
   processor.program_counter += 4;
}

void exec_xor(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = processor.registers[inst.rs1] ^ processor.registers[inst.rs2];
   processor.program_counter += 4;
}

void exec_sll(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = processor.registers[inst.rs1] << (processor.registers[inst.rs2] & 0x3F);
   processor.program_counter += 4;
}

void exec_srl(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = processor.registers[inst.rs1] >> (processor.registers[inst.rs2] & 0x3F);
   processor.program_counter += 4;
}

void exec_sra(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = static_cast<int64_t>(processor.registers[inst.rs1]) >> (processor.registers[inst.rs2] & 0x3F);
   processor.program_counter += 4;
}

void exec_slt(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = (static_cast<int64_t>(processor.registers[inst.rs1]) < static_cast<int64_t>(processor.registers[inst.rs2])) ? 1 : 0;
   processor.program_counter += 4;
}

void exec_sltu(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = (processor.registers[inst.rs1] < processor.registers[inst.rs2]) ? 1 : 0;
   processor.program_counter += 4;
}

void exec_andi(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = processor.registers[inst.rs1] & inst.imm;
   processor.program_counter += 4;
}

void exec_ori(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = processor.registers[inst.rs1] | inst.imm;
   processor.program_counter += 4;
}

void exec_xori(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = processor.registers[inst.rs1] ^ inst.imm;
   processor.program_counter += 4;
}

void exec_slli(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = processor.registers[inst.rs1] << (inst.imm & 0x3F);
   processor.program_counter += 4;
}

void exec_srli(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = processor.registers[inst.rs1] >> (inst.imm & 0x3F);
   processor.program_counter += 4;
}

void exec_srai(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = static_cast<int64_t>(processor.registers[inst.rs1]) >> (inst.imm & 0x3F);
   processor.program_counter += 4;
}

void exec_slti(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = (static_cast<int64_t>(processor.registers[inst.rs1]) < inst.imm) ? 1 : 0;
   processor.program_counter += 4;
}

void exec_sltiu(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = (processor.registers[inst.rs1] < static_cast<uint64_t>(inst.imm)) ? 1 : 0;
   processor.program_counter += 4;
}

void exec_addw(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = static_cast<int32_t>(processor.registers[inst.rs1] + processor.registers[inst.rs2]);
   processor.program_counter += 4;
}

void exec_subw(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = static_cast<int32_t>(processor.registers[inst.rs1] - processor.registers[inst.rs2]);
   processor.program_counter += 4;
}

void exec_addiw(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = static_cast<int32_t>(processor.registers[inst.rs1] + inst.imm);
   processor.program_counter += 4;
}

void exec_sllw(const DecodedInstruction &inst, Processor &processor)
{
   uint32_t val = static_cast<uint32_t>(processor.registers[inst.rs1]) << (processor.registers[inst.rs2] & 0x1F);
   processor.registers[inst.rd] = static_cast<int32_t>(val);
   processor.program_counter += 4;
}

void exec_srlw(const DecodedInstruction &inst, Processor &processor)
{
   uint32_t val = static_cast<uint32_t>(processor.registers[inst.rs1]) >> (processor.registers[inst.rs2] & 0x1F);
   processor.registers[inst.rd] = static_cast<int32_t>(val);
   processor.program_counter += 4;
}

void exec_sraw(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = static_cast<int32_t>(static_cast<int32_t>(processor.registers[inst.rs1]) >> (processor.registers[inst.rs2] & 0x1F));
   processor.program_counter += 4;
}

static void exec_auipc(const DecodedInstruction &inst, Processor &processor)
{
   // Add the sign-extended immediate to the current PC
   processor.registers[inst.rd] = static_cast<int64_t>(inst.pc) + static_cast<int64_t>(inst.imm);

   processor.program_counter += 4;
}

// Register them in DefaultRegistry
void DefaultRegistry::register_rv64i_arithmetic()
{
   InstructionRegistry::register_r(0x33, 0b111, 0b0000000, exec_and);
   InstructionRegistry::register_r(0x33, 0b110, 0b0000000, exec_or);
   InstructionRegistry::register_r(0x33, 0b100, 0b0000000, exec_xor);
   InstructionRegistry::register_r(0x33, 0b001, 0b0000000, exec_sll);
   InstructionRegistry::register_r(0x33, 0b101, 0b0000000, exec_srl);
   InstructionRegistry::register_r(0x33, 0b101, 0b0100000, exec_sra);
   InstructionRegistry::register_r(0x33, 0b010, 0b0000000, exec_slt);
   InstructionRegistry::register_r(0x33, 0b011, 0b0000000, exec_sltu);
   InstructionRegistry::register_r(0x13, 0b001, 0b0000000, exec_slli);
   InstructionRegistry::register_r(0x13, 0b101, 0b0000000, exec_srli);
   InstructionRegistry::register_r(0x13, 0b101, 0b0100000, exec_srai);

   InstructionRegistry::register_r(0x3B, 0b000, 0b0000000, exec_addw);
   InstructionRegistry::register_r(0x3B, 0b000, 0b0100000, exec_subw);
   InstructionRegistry::register_r(0x3B, 0b001, 0b0000000, exec_sllw);
   InstructionRegistry::register_r(0x3B, 0b101, 0b0000000, exec_srlw);
   InstructionRegistry::register_r(0x3B, 0b101, 0b0100000, exec_sraw);

   InstructionRegistry::register_i(0x13, 0b111, exec_andi);
   InstructionRegistry::register_i(0x13, 0b110, exec_ori);
   InstructionRegistry::register_i(0x13, 0b100, exec_xori);
   InstructionRegistry::register_i(0x1B, 0b000, exec_addiw);
   InstructionRegistry::register_i(0x13, 0b010, exec_slti);
   InstructionRegistry::register_i(0x13, 0b011, exec_sltiu);

   InstructionRegistry::register_u(0x17, exec_auipc);
}