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

void DefaultRegistry::register_float()
{
   InstructionRegistry::register_s(0x27, 3, exec_fsd);
   InstructionRegistry::register_i(0x07, 3, exec_fld);
}