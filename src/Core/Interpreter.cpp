#include "Interpreter.h"
#include "InstructionFormat.h"
#include "Processor.h"
#include "InstructionRegistry.h"
#include "Debug.h"
#include "IODevice.h"

static InstructionFormat get_format(uint32_t opcode);
static int32_t sign_extend(int32_t value, int bits);
static int32_t decode_i(uint32_t raw);
static int32_t decode_s(uint32_t raw);
static int32_t decode_b(uint32_t raw);
static int32_t decode_u(uint32_t raw);
static int32_t decode_j(uint32_t raw);

DecodedInstruction Interpreter::decode(uint32_t raw, Processor &processor, uint8_t length)
{
   DecodedInstruction inst{};

   // Set the length and PC first
   inst.length = length;
   inst.pc = processor.program_counter;

   inst.opcode = raw & 0x7F;

   inst.rd = (raw >> 7) & 0x1F;
   inst.funct3 = (raw >> 12) & 0x7;
   inst.rs1 = (raw >> 15) & 0x1F;
   inst.rs2 = (raw >> 20) & 0x1F;
   inst.funct7 = (raw >> 25) & 0x7F;

   InstructionFormat format = get_format(inst.opcode);

   switch (format)
   {
   case InstructionFormat::I:
      inst.imm = decode_i(raw);
      break;
   case InstructionFormat::S:
      inst.imm = decode_s(raw);
      break;
   case InstructionFormat::B:
      inst.imm = decode_b(raw);
      break;
   case InstructionFormat::U:
      inst.imm = decode_u(raw);
      break;
   case InstructionFormat::J:
      inst.imm = decode_j(raw);
      break;
   default:
      inst.imm = 0;
   }

   return inst;
}

static InstructionFormat get_format(uint32_t opcode)
{
   switch (opcode)
   {
   case 0x0F:
      return InstructionFormat::I;
   case 0x33:
      return InstructionFormat::R;
   case 0x3B:
      return InstructionFormat::R;

   case 0x13:
      return InstructionFormat::I;
   case 0x1B:
      return InstructionFormat::I;
   case 0x03:
      return InstructionFormat::I;
   case 0x67:
      return InstructionFormat::I;
   case 0x73:
      return InstructionFormat::I;

   case 0x23:
      return InstructionFormat::S;
   case 0x63:
      return InstructionFormat::B;
   case 0x37:
      return InstructionFormat::U;
   case 0x17:
      return InstructionFormat::U;
   case 0x6F:
      return InstructionFormat::J;
   case 0x07:
      return InstructionFormat::I;
   case 0x27:
      return InstructionFormat::S;

   default:
      return InstructionFormat::R;
   }
}

static int32_t sign_extend(int32_t value, int bits)
{
   int32_t shift = 32 - bits;
   return (value << shift) >> shift;
}

static int32_t decode_i(uint32_t raw)
{
   return sign_extend(raw >> 20, 12);
}

static int32_t decode_s(uint32_t raw)
{
   int32_t imm =
       ((raw >> 25) << 5) |
       ((raw >> 7) & 0x1F);

   return sign_extend(imm, 12);
}

static int32_t decode_b(uint32_t raw)
{
   int32_t imm =
       ((raw >> 31) << 12) |
       (((raw >> 7) & 1) << 11) |
       (((raw >> 25) & 0x3F) << 5) |
       (((raw >> 8) & 0xF) << 1);

   return sign_extend(imm, 13);
}

static int32_t decode_u(uint32_t raw)
{
   return static_cast<int32_t>(raw & 0xFFFFF000);
}

static int32_t decode_j(uint32_t raw)
{
   int32_t imm =
       ((raw >> 31) << 20) |
       (((raw >> 12) & 0xFF) << 12) |
       (((raw >> 20) & 1) << 11) |
       (((raw >> 21) & 0x3FF) << 1);

   return sign_extend(imm, 21);
}

bool Interpreter::handle(uint32_t raw, Processor &processor, uint8_t length)
{
   DecodedInstruction inst = decode(raw, processor, length);
   InstructionFormat format = get_format(inst.opcode);

   uint32_t lookup_funct7 = inst.funct7;
   if (format != InstructionFormat::R)
   {
      bool is_shift = (inst.opcode == 0x13 || inst.opcode == 0x1B) &&
                      (inst.funct3 == 1 || inst.funct3 == 5);

      bool is_system = (inst.opcode == 0x73);

      if (!is_shift && !is_system)
      {
         lookup_funct7 = 0;
      }
      else if (is_shift)
      {
         lookup_funct7 &= ~0x01;
      }
   }

   uint32_t key = InstructionRegistry::make_key(
       inst.opcode,
       inst.funct3,
       lookup_funct7);

   ExecFunc func = InstructionRegistry::lookup(key);

   if (func)
   {
      func(inst, processor);
   }
   else
   {
      processor.raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, inst.pc, raw);
   }

   processor.write_reg(0, 0);
   return true;
}
