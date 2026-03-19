#include "Debug.h"
#include "IODevice.h"
#include "Interpreter.h"
#include "Processor.h"
#include "InstructionRegistry.h"
#include "DefaultRegistry.h"

void exec_lb(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint8_t value = processor.memory.readByte(address);

   int8_t signed_value = static_cast<int8_t>(value);
   processor.registers[inst.rd] = signed_value;
   processor.program_counter += 4;

   DEBUG_BEGIN()
   io->writeString("LB x");
   io->writeInt(inst.rd);
   io->writeString(", ");
   io->writeInt(inst.imm);
   io->writeString("(x");
   io->writeInt(inst.rs1);
   io->writeString(") -> addr: ");
   io->writeInt(address);
   io->writeString(" -> value: ");
   io->writeInt(signed_value);
   io->writeString(" (raw: ");
   io->writeInt(value);
   io->writeString(")");
   DEBUG_END()
}

void exec_lh(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint16_t value = processor.memory.readHalf(address);

   int16_t signed_value = static_cast<int16_t>(value);
   processor.registers[inst.rd] = signed_value;
   processor.program_counter += 4;

   DEBUG_BEGIN()
   io->writeString("LH x");
   io->writeInt(inst.rd);
   io->writeString(", ");
   io->writeInt(inst.imm);
   io->writeString("(x");
   io->writeInt(inst.rs1);
   io->writeString(") -> addr: ");
   io->writeInt(address);
   io->writeString(" -> value: ");
   io->writeInt(signed_value);
   io->writeString(" (raw: ");
   io->writeInt(value);
   io->writeString(")");
   DEBUG_END()
}

void exec_ld(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint64_t value = processor.memory.readDouble(address);

   processor.registers[inst.rd] = value;
   processor.program_counter += 4;

   DEBUG_BEGIN()
   io->writeString("LD x");
   io->writeInt(inst.rd);
   io->writeString(", ");
   io->writeInt(inst.imm);
   io->writeString("(x");
   io->writeInt(inst.rs1);
   io->writeString(") -> addr: ");
   io->writeInt(address);
   io->writeString(" -> value: ");
   io->writeInt(value);
   DEBUG_END()
}

void exec_lbu(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint8_t value = processor.memory.readByte(address);

   processor.registers[inst.rd] = static_cast<uint8_t>(value);
   processor.program_counter += 4;

   DEBUG_BEGIN()
   io->writeString("LBU x");
   io->writeInt(inst.rd);
   io->writeString(", ");
   io->writeInt(inst.imm);
   io->writeString("(x");
   io->writeInt(inst.rs1);
   io->writeString(") -> addr: ");
   io->writeInt(address);
   io->writeString(" -> value: ");
   io->writeInt(value);
   DEBUG_END()
}

void exec_lhu(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint16_t value = processor.memory.readHalf(address);

   processor.registers[inst.rd] = static_cast<uint16_t>(value);
   processor.program_counter += 4;

   DEBUG_BEGIN()
   io->writeString("LHU x");
   io->writeInt(inst.rd);
   io->writeString(", ");
   io->writeInt(inst.imm);
   io->writeString("(x");
   io->writeInt(inst.rs1);
   io->writeString(") -> addr: ");
   io->writeInt(address);
   io->writeString(" -> value: ");
   io->writeInt(value);
   DEBUG_END()
}

void exec_lwu(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint32_t value = processor.memory.readWord(address);

   processor.registers[inst.rd] = static_cast<uint32_t>(value);
   processor.program_counter += 4;

   DEBUG_BEGIN()
   io->writeString("LWU x");
   io->writeInt(inst.rd);
   io->writeString(", ");
   io->writeInt(inst.imm);
   io->writeString("(x");
   io->writeInt(inst.rs1);
   io->writeString(") -> addr: ");
   io->writeInt(address);
   io->writeString(" -> value: ");
   io->writeInt(value);
   DEBUG_END()
}

void register_loads()
{
   using IR = InstructionRegistry;

   IR::register_i(0x03, 0b000, exec_lb);
   IR::register_i(0x03, 0b001, exec_lh);
   IR::register_i(0x03, 0b011, exec_ld);
   IR::register_i(0x03, 0b100, exec_lbu);
   IR::register_i(0x03, 0b101, exec_lhu);
   IR::register_i(0x03, 0b110, exec_lwu);
}