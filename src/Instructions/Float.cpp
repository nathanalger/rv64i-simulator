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

   // Assuming your IODevice/Bus has a writeDouble method
   processor.bus.writeDouble(addr, value);
}

static void exec_fld(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t addr = processor.registers[inst.rs1] + inst.imm;

   // Read 64 bits from memory
   // (Assuming your bus has a readDouble or read64 method)
   uint64_t value = processor.bus.readDouble(addr);

   // Store it into the destination float register
   processor.f_registers[inst.rd] = value;
}

void DefaultRegistry::register_float()
{
   InstructionRegistry::register_s(0x27, 3, exec_fsd);
   InstructionRegistry::register_i(0x07, 3, exec_fld);
}