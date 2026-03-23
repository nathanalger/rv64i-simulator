#include "Debug.h"
#include "IODevice.h"
#include "Interpreter.h"
#include "Processor.h"
#include "InstructionRegistry.h"
#include "DefaultRegistry.h"

void exec_sb(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint8_t value = static_cast<uint8_t>(processor.registers[inst.rs2]);

   processor.bus.writeByte(address, value);

   TRACE_BEGIN()
   io->writeString("SB x");
   io->writeInt(inst.rs2);
   io->writeString(", ");
   io->writeInt(inst.imm);
   io->writeString("(x");
   io->writeInt(inst.rs1);
   io->writeString(") -> addr: ");
   io->writeInt(address);
   io->writeString(" <- value: ");
   io->writeInt(value);
   DEBUG_END()
}

void exec_sh(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint16_t value = static_cast<uint16_t>(processor.registers[inst.rs2]);

   processor.bus.writeHalf(address, value);

   TRACE_BEGIN()
   io->writeString("SH x");
   io->writeInt(inst.rs2);
   io->writeString(", ");
   io->writeInt(inst.imm);
   io->writeString("(x");
   io->writeInt(inst.rs1);
   io->writeString(") -> addr: ");
   io->writeInt(address);
   io->writeString(" <- value: ");
   io->writeInt(value);
   DEBUG_END()
}

void exec_sd(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t address = processor.registers[inst.rs1] + inst.imm;
   uint64_t value = processor.registers[inst.rs2];

   processor.bus.writeDouble(address, value);

   TRACE_BEGIN()
   io->writeString("SD x");
   io->writeInt(inst.rs2);
   io->writeString(", ");
   io->writeInt(inst.imm);
   io->writeString("(x");
   io->writeInt(inst.rs1);
   io->writeString(") -> addr: ");
   io->writeInt(address);
   io->writeString(" <- value: ");
   io->writeInt(value);
   DEBUG_END()
}

void DefaultRegistry::register_stores()
{
   using IR = InstructionRegistry;

   IR::register_s(0x23, 0b000, exec_sb);
   IR::register_s(0x23, 0b001, exec_sh);
   IR::register_s(0x23, 0b011, exec_sd);
}