#include "Debug.h"
#include "IODevice.h"
#include "Interpreter.h"
#include "Processor.h"
#include "InstructionRegistry.h"
#include "DefaultRegistry.h"

void exec_lb(const DecodedInstruction &inst, Processor &processor)
{
   if (inst.rd == 0)
      return;
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint8_t value = processor.bus.readByte(address);

   int8_t signed_value = static_cast<int8_t>(value);
   processor.write_reg(inst.rd, signed_value);

   TRACE_BEGIN()
   Debug::writeString("LB x");
   Debug::writeInt(inst.rd);
   Debug::writeString(", ");
   Debug::writeInt(inst.imm);
   Debug::writeString("(x");
   Debug::writeInt(inst.rs1);
   Debug::writeString(") -> addr: ");
   Debug::writeInt(address);
   Debug::writeString(" -> value: ");
   Debug::writeInt(signed_value);
   Debug::writeString(" (raw: ");
   Debug::writeInt(value);
   Debug::writeString(")");
   DEBUG_END()
}

void exec_lh(const DecodedInstruction &inst, Processor &processor)
{
   if (inst.rd == 0)
      return;
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint16_t value = processor.bus.readHalf(address);

   int16_t signed_value = static_cast<int16_t>(value);
   processor.write_reg(inst.rd, signed_value);

   TRACE_BEGIN()
   Debug::writeString("LH x");
   Debug::writeInt(inst.rd);
   Debug::writeString(", ");
   Debug::writeInt(inst.imm);
   Debug::writeString("(x");
   Debug::writeInt(inst.rs1);
   Debug::writeString(") -> addr: ");
   Debug::writeInt(address);
   Debug::writeString(" -> value: ");
   Debug::writeInt(signed_value);
   Debug::writeString(" (raw: ");
   Debug::writeInt(value);
   Debug::writeString(")");
   DEBUG_END()
}

void exec_ld(const DecodedInstruction &inst, Processor &processor)
{
   if (inst.rd == 0)
      return;

   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint64_t value = processor.bus.readDouble(address);

   processor.write_reg(inst.rd, value);

   TRACE_BEGIN()
   Debug::writeString("LD x");
   Debug::writeInt(inst.rd);
   Debug::writeString(", ");
   Debug::writeInt(inst.imm);
   Debug::writeString("(x");
   Debug::writeInt(inst.rs1);
   Debug::writeString(") -> addr: ");
   Debug::writeInt(address);
   Debug::writeString(" -> value: ");
   Debug::writeInt(value);
   DEBUG_END()
}

void exec_lbu(const DecodedInstruction &inst, Processor &processor)
{
   if (inst.rd == 0)
      return;
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint8_t value = processor.bus.readByte(address);

   processor.write_reg(inst.rd, static_cast<uint8_t>(value));

   TRACE_BEGIN()
   Debug::writeString("LBU x");
   Debug::writeInt(inst.rd);
   Debug::writeString(", ");
   Debug::writeInt(inst.imm);
   Debug::writeString("(x");
   Debug::writeInt(inst.rs1);
   Debug::writeString(") -> addr: ");
   Debug::writeInt(address);
   Debug::writeString(" -> value: ");
   Debug::writeInt(value);
   DEBUG_END()
}

void exec_lhu(const DecodedInstruction &inst, Processor &processor)
{
   if (inst.rd == 0)
      return;
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint16_t value = processor.bus.readHalf(address);

   processor.write_reg(inst.rd, static_cast<uint16_t>(value));

   TRACE_BEGIN()
   Debug::writeString("LHU x");
   Debug::writeInt(inst.rd);
   Debug::writeString(", ");
   Debug::writeInt(inst.imm);
   Debug::writeString("(x");
   Debug::writeInt(inst.rs1);
   Debug::writeString(") -> addr: ");
   Debug::writeInt(address);
   Debug::writeString(" -> value: ");
   Debug::writeInt(value);
   DEBUG_END()
}

void exec_lwu(const DecodedInstruction &inst, Processor &processor)
{
   if (inst.rd == 0)
      return;
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint32_t value = processor.bus.readWord(address);

   processor.write_reg(inst.rd, static_cast<uint32_t>(value));

   TRACE_BEGIN()
   Debug::writeString("LWU x");
   Debug::writeInt(inst.rd);
   Debug::writeString(", ");
   Debug::writeInt(inst.imm);
   Debug::writeString("(x");
   Debug::writeInt(inst.rs1);
   Debug::writeString(") -> addr: ");
   Debug::writeInt(address);
   Debug::writeString(" -> value: ");
   Debug::writeInt(value);
   DEBUG_END()
}

void exec_lui(const DecodedInstruction &inst, Processor &processor)
{
   if (inst.rd == 0)
      return;

   int64_t val = static_cast<int64_t>(static_cast<int32_t>(inst.imm));
   processor.write_reg(inst.rd, val);

   TRACE_BEGIN()
   Debug::writeString("LUI x");
   Debug::writeInt(inst.rd);
   Debug::writeString(", ");
   Debug::writeHex(inst.imm);
   Debug::writeString(" -> value: ");
   Debug::writeInt(processor.registers[inst.rd]);
   DEBUG_END()
}

void DefaultRegistry::register_loads()
{
   using IR = InstructionRegistry;

   IR::register_i(0x03, 0b000, exec_lb);
   IR::register_i(0x03, 0b001, exec_lh);
   IR::register_i(0x03, 0b011, exec_ld);
   IR::register_i(0x03, 0b100, exec_lbu);
   IR::register_i(0x03, 0b101, exec_lhu);
   IR::register_i(0x03, 0b110, exec_lwu);
   IR::register_u(0x37, exec_lui);
}