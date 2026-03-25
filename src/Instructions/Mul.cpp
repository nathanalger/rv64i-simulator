#include "Debug.h"
#include "IODevice.h"
#include "Interpreter.h"
#include "Processor.h"
#include "InstructionRegistry.h"
#include "DefaultRegistry.h"

static uint64_t multiply_high_u64(uint64_t u, uint64_t v)
{
   uint64_t u_lo = u & 0xFFFFFFFF;
   uint64_t u_hi = u >> 32;
   uint64_t v_lo = v & 0xFFFFFFFF;
   uint64_t v_hi = v >> 32;

   uint64_t t = u_lo * v_lo;
   uint64_t k = t >> 32;

   t = u_hi * v_lo + k;
   uint64_t w1 = t & 0xFFFFFFFF;
   uint64_t w2 = t >> 32;

   t = u_lo * v_hi + w1;
   k = t >> 32;

   return u_hi * v_hi + w2 + k;
}

void exec_mul(const DecodedInstruction &inst, Processor &processor)
{
   processor.write_reg(inst.rd, processor.registers[inst.rs1] * processor.registers[inst.rs2]);
}

void exec_div(const DecodedInstruction &inst, Processor &processor)
{
   int64_t a = (int64_t)processor.registers[inst.rs1];
   int64_t b = (int64_t)processor.registers[inst.rs2];

   if (b == 0)
   {
      processor.write_reg(inst.rd, -1);
   }

   else if ((uint64_t)a == 0x8000000000000000ULL && b == -1)
   {
      processor.write_reg(inst.rd, a);
   }
   else
   {
      processor.write_reg(inst.rd, a / b);
   }
}

void exec_divu(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t a = processor.registers[inst.rs1];
   uint64_t b = processor.registers[inst.rs2];
   if (b == 0)
      processor.write_reg(inst.rd, 0xFFFFFFFFFFFFFFFFULL);
   else
      processor.write_reg(inst.rd, a / b);
}

void exec_rem(const DecodedInstruction &inst, Processor &processor)
{
   int64_t a = (int64_t)processor.registers[inst.rs1];
   int64_t b = (int64_t)processor.registers[inst.rs2];
   if (b == 0)
   {
      processor.write_reg(inst.rd, a);
   }
   else if ((uint64_t)a == 0x8000000000000000ULL && b == -1)
   {
      processor.write_reg(inst.rd, 0);
   }
   else
   {
      processor.write_reg(inst.rd, a % b);
   }
}

void exec_remu(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t a = processor.registers[inst.rs1];
   uint64_t b = processor.registers[inst.rs2];
   if (b == 0)
      processor.write_reg(inst.rd, a);
   else
      processor.write_reg(inst.rd, a % b);
}

void exec_mulw(const DecodedInstruction &inst, Processor &processor)
{
   int32_t res = (int32_t)processor.registers[inst.rs1] * (int32_t)processor.registers[inst.rs2];
   processor.write_reg(inst.rd, (int64_t)res);
}

void exec_divw(const DecodedInstruction &inst, Processor &processor)
{
   int32_t a = (int32_t)processor.registers[inst.rs1];
   int32_t b = (int32_t)processor.registers[inst.rs2];
   if (b == 0)
      processor.write_reg(inst.rd, -1);
   else if (a == (int32_t)0x80000000 && b == -1)
      processor.write_reg(inst.rd, (int64_t)a);
   else
      processor.write_reg(inst.rd, (int64_t)(a / b));
}

void exec_divuw(const DecodedInstruction &inst, Processor &processor)
{
   uint32_t a = (uint32_t)processor.registers[inst.rs1];
   uint32_t b = (uint32_t)processor.registers[inst.rs2];
   if (b == 0)
      processor.write_reg(inst.rd, 0xFFFFFFFFFFFFFFFFULL);
   else
      processor.write_reg(inst.rd, (int64_t)(int32_t)(a / b));
}

void exec_remw(const DecodedInstruction &inst, Processor &processor)
{
   int32_t a = (int32_t)processor.registers[inst.rs1];
   int32_t b = (int32_t)processor.registers[inst.rs2];
   if (b == 0)
      processor.write_reg(inst.rd, (int64_t)a);
   else if (a == (int32_t)0x80000000 && b == -1)
      processor.write_reg(inst.rd, 0);
   else
      processor.write_reg(inst.rd, (int64_t)(a % b));
}

void exec_remuw(const DecodedInstruction &inst, Processor &processor)
{
   uint32_t a = (uint32_t)processor.registers[inst.rs1];
   uint32_t b = (uint32_t)processor.registers[inst.rs2];
   if (b == 0)
      processor.write_reg(inst.rd, (int64_t)(int32_t)a);
   else
      processor.write_reg(inst.rd, (int64_t)(int32_t)(a % b));
}

void exec_mulh(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t u = processor.registers[inst.rs1];
   uint64_t v = processor.registers[inst.rs2];
   uint64_t res_hi = multiply_high_u64(u, v);

   if ((int64_t)u < 0)
      res_hi -= v;
   if ((int64_t)v < 0)
      res_hi -= u;

   processor.write_reg(inst.rd, res_hi);
}

void exec_mulhsu(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t u = processor.registers[inst.rs1];
   uint64_t v = processor.registers[inst.rs2];
   uint64_t res_hi = multiply_high_u64(u, v);

   if ((int64_t)u < 0)
      res_hi -= v;

   processor.write_reg(inst.rd, res_hi);
}

void exec_mulhu(const DecodedInstruction &inst, Processor &processor)
{
   processor.registers[inst.rd] = multiply_high_u64(
       processor.registers[inst.rs1],
       processor.registers[inst.rs2]);
}

void DefaultRegistry::register_mul()
{
   InstructionRegistry::register_r(0x33, 0b000, 0b0000001, exec_mul);
   InstructionRegistry::register_r(0x33, 0b001, 0b0000001, exec_mulh);
   InstructionRegistry::register_r(0x33, 0b010, 0b0000001, exec_mulhsu);
   InstructionRegistry::register_r(0x33, 0b011, 0b0000001, exec_mulhu);
   InstructionRegistry::register_r(0x33, 0b100, 0b0000001, exec_div);
   InstructionRegistry::register_r(0x33, 0b101, 0b0000001, exec_divu);
   InstructionRegistry::register_r(0x33, 0b110, 0b0000001, exec_rem);
   InstructionRegistry::register_r(0x33, 0b111, 0b0000001, exec_remu);

   InstructionRegistry::register_r(0x3B, 0b000, 0b0000001, exec_mulw);
   InstructionRegistry::register_r(0x3B, 0b100, 0b0000001, exec_divw);
   InstructionRegistry::register_r(0x3B, 0b101, 0b0000001, exec_divuw);
   InstructionRegistry::register_r(0x3B, 0b110, 0b0000001, exec_remw);
   InstructionRegistry::register_r(0x3B, 0b111, 0b0000001, exec_remuw);
}