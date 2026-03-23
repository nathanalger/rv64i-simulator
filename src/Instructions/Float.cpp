#include "DefaultRegistry.h"
#include "InstructionRegistry.h"
#include "Processor.h"
#include "Debug.h"
#include "IODevice.h"
#include "Constants.h"

static void exec_fsd(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t addr = processor.registers[inst.rs1] + inst.imm;
   uint64_t value = processor.f_registers[inst.rs2];

   processor.bus.writeDouble(addr, value);
}

static void exec_fld(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t addr = processor.registers[inst.rs1] + inst.imm;
   uint64_t value = processor.bus.readDouble(addr);

   processor.f_registers[inst.rd] = value;
}

static void exec_fmv_w_x(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t val = processor.registers[inst.rs1] & 0xFFFFFFFF;
   processor.f_registers[inst.rd] = val | 0xFFFFFFFF00000000ULL;
}

static void exec_fmv_x_w(const DecodedInstruction &inst, Processor &processor)
{
   // Move 32-bit value from float rs1 to integer rd (sign-extended)
   int32_t val = static_cast<int32_t>(processor.f_registers[inst.rs1] & 0xFFFFFFFF);
   processor.registers[inst.rd] = static_cast<int64_t>(val);
}

static void exec_fmv_d_x(const DecodedInstruction &inst, Processor &processor)
{
   // Move 64-bit value from integer rs1 to float rd
   processor.f_registers[inst.rd] = processor.registers[inst.rs1];
}

static void exec_fmv_x_d(const DecodedInstruction &inst, Processor &processor)
{
   // Move 64-bit value from float rs1 to integer rd
   processor.registers[inst.rd] = processor.f_registers[inst.rs1];
}

void DefaultRegistry::register_float()
{
   InstructionRegistry::register_s(0x27, 3, exec_fsd);
   InstructionRegistry::register_i(0x07, 3, exec_fld);

   InstructionRegistry::register_r(0x53, 0, 0x78, exec_fmv_w_x); // fmv.w.x
   InstructionRegistry::register_r(0x53, 0, 0x70, exec_fmv_x_w); // fmv.x.w
   InstructionRegistry::register_r(0x53, 0, 0x79, exec_fmv_d_x); // fmv.d.x
   InstructionRegistry::register_r(0x53, 0, 0x71, exec_fmv_x_d); // fmv.x.d
}