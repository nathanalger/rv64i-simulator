#include "Debug.h"
#include "IODevice.h"
#include "Interpreter.h"
#include "Processor.h"
#include "InstructionRegistry.h"
#include "DefaultRegistry.h"

// ==========================================
// Load-Reserved (LR) / Store-Conditional (SC)
// ==========================================

void exec_lr_w(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t addr = processor.registers[inst.rs1];
   uint32_t val = processor.readMemoryWord(addr);

   processor.load_reservation = addr;
   processor.reservation_valid = true;

   // 32-bit values must be sign-extended to 64-bit in the register
   processor.registers[inst.rd] = (int64_t)(int32_t)val;
}

void exec_sc_w(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t addr = processor.registers[inst.rs1];

   if (processor.reservation_valid && processor.load_reservation == addr)
   {
      processor.writeMemoryWord(addr, (uint32_t)processor.registers[inst.rs2]);
      processor.registers[inst.rd] = 0; // 0 indicates success
   }
   else
   {
      processor.registers[inst.rd] = 1; // Non-zero indicates failure
   }

   // Any SC instruction inherently invalidates the reservation
   processor.reservation_valid = false;
}

void exec_lr_d(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t addr = processor.registers[inst.rs1];
   uint64_t val = processor.readMemoryDouble(addr);

   processor.load_reservation = addr;
   processor.reservation_valid = true;

   processor.registers[inst.rd] = val;
}

void exec_sc_d(const DecodedInstruction &inst, Processor &processor)
{
   uint64_t addr = processor.registers[inst.rs1];

   if (processor.reservation_valid && processor.load_reservation == addr)
   {
      processor.writeMemoryDouble(addr, processor.registers[inst.rs2]);
      processor.registers[inst.rd] = 0;
   }
   else
   {
      processor.registers[inst.rd] = 1;
   }

   processor.reservation_valid = false;
}

// ==========================================
// Atomic Memory Operations (AMOs)
// ==========================================

// Macro for 32-bit (Word) AMOs
#define AMO_OP_W(name, type, op)                                              \
   void exec_##name##_w(const DecodedInstruction &inst, Processor &processor) \
   {                                                                          \
      uint64_t addr = processor.registers[inst.rs1];                          \
      type old_val = (type)processor.readMemoryWord(addr);                    \
      type src_val = (type)processor.registers[inst.rs2];                     \
      type new_val = op;                                                      \
      processor.writeMemoryWord(addr, (uint32_t)new_val);                     \
      processor.registers[inst.rd] = (int64_t)(int32_t)old_val;               \
   }

// Macro for 64-bit (Double) AMOs
#define AMO_OP_D(name, type, op)                                              \
   void exec_##name##_d(const DecodedInstruction &inst, Processor &processor) \
   {                                                                          \
      uint64_t addr = processor.registers[inst.rs1];                          \
      type old_val = (type)processor.readMemoryDouble(addr);                  \
      type src_val = (type)processor.registers[inst.rs2];                     \
      type new_val = op;                                                      \
      processor.writeMemoryDouble(addr, (uint64_t)new_val);                   \
      processor.registers[inst.rd] = (uint64_t)old_val;                       \
   }

// Implementations using the macros
AMO_OP_W(amoswap, uint32_t, src_val)
AMO_OP_D(amoswap, uint64_t, src_val)

AMO_OP_W(amoadd, uint32_t, old_val + src_val)
AMO_OP_D(amoadd, uint64_t, old_val + src_val)

AMO_OP_W(amoxor, uint32_t, old_val ^ src_val)
AMO_OP_D(amoxor, uint64_t, old_val ^ src_val)

AMO_OP_W(amoand, uint32_t, old_val &src_val)
AMO_OP_D(amoand, uint64_t, old_val &src_val)

AMO_OP_W(amoor, uint32_t, old_val | src_val)
AMO_OP_D(amoor, uint64_t, old_val | src_val)

// Min/Max (Signed)
AMO_OP_W(amomin, int32_t, (old_val < src_val) ? old_val : src_val)
AMO_OP_D(amomin, int64_t, (old_val < src_val) ? old_val : src_val)

AMO_OP_W(amomax, int32_t, (old_val > src_val) ? old_val : src_val)
AMO_OP_D(amomax, int64_t, (old_val > src_val) ? old_val : src_val)

// Min/Max (Unsigned)
AMO_OP_W(amominu, uint32_t, (old_val < src_val) ? old_val : src_val)
AMO_OP_D(amominu, uint64_t, (old_val < src_val) ? old_val : src_val)

AMO_OP_W(amomaxu, uint32_t, (old_val > src_val) ? old_val : src_val)
AMO_OP_D(amomaxu, uint64_t, (old_val > src_val) ? old_val : src_val)

// ==========================================
// Registration
// ==========================================

// Helper to register all 4 aq/rl combinations for an AMO
static void register_amo(uint8_t funct5, uint8_t funct3, void (*func)(const DecodedInstruction &, Processor &))
{
   for (uint8_t aqrl = 0; aqrl < 4; ++aqrl)
   {
      uint8_t funct7 = (funct5 << 2) | aqrl;
      InstructionRegistry::register_r(0x2F, funct3, funct7, func);
   }
}

void DefaultRegistry::register_atomics()
{
   // LR / SC (funct5: LR = 0b00010, SC = 0b00011)
   register_amo(0b00010, 0b010, exec_lr_w);
   register_amo(0b00011, 0b010, exec_sc_w);
   register_amo(0b00010, 0b011, exec_lr_d);
   register_amo(0b00011, 0b011, exec_sc_d);

   // AMOSWAP (funct5 = 0b00001)
   register_amo(0b00001, 0b010, exec_amoswap_w);
   register_amo(0b00001, 0b011, exec_amoswap_d);

   // AMOADD (funct5 = 0b00000)
   register_amo(0b00000, 0b010, exec_amoadd_w);
   register_amo(0b00000, 0b011, exec_amoadd_d);

   // AMOXOR (funct5 = 0b00100)
   register_amo(0b00100, 0b010, exec_amoxor_w);
   register_amo(0b00100, 0b011, exec_amoxor_d);

   // AMOAND (funct5 = 0b01100)
   register_amo(0b01100, 0b010, exec_amoand_w);
   register_amo(0b01100, 0b011, exec_amoand_d);

   // AMOOR (funct5 = 0b01000)
   register_amo(0b01000, 0b010, exec_amoor_w);
   register_amo(0b01000, 0b011, exec_amoor_d);

   // AMOMIN (funct5 = 0b10000)
   register_amo(0b10000, 0b010, exec_amomin_w);
   register_amo(0b10000, 0b011, exec_amomin_d);

   // AMOMAX (funct5 = 0b10100)
   register_amo(0b10100, 0b010, exec_amomax_w);
   register_amo(0b10100, 0b011, exec_amomax_d);

   // AMOMINU (funct5 = 0b11000)
   register_amo(0b11000, 0b010, exec_amominu_w);
   register_amo(0b11000, 0b011, exec_amominu_d);

   // AMOMAXU (funct5 = 0b11100)
   register_amo(0b11100, 0b010, exec_amomaxu_w);
   register_amo(0b11100, 0b011, exec_amomaxu_d);
}