#include "Interpreter.h"
#include "InstructionType.h"
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

InstructionType Interpreter::interpret(uint32_t instruction)
{

   uint32_t opcode = instruction & 0x7F;
   uint32_t funct3 = (instruction >> 12) & 0x7;
   uint32_t funct7 = (instruction >> 25) & 0x7F;

   switch (opcode)
   {

   case 0x33: // R-Type
      if (funct3 == 0 && funct7 == 0x00)
         return InstructionType::ADD;

      if (funct3 == 0 && funct7 == 0x20)
         return InstructionType::SUB;

      break;

   case 0x13: // I-Type arithmetic
      if (funct3 == 0)
         return InstructionType::ADDI;
      break;

   case 0x03: // Loads
      switch (funct3)
      {
      case 0b000:
         return InstructionType::LB;
      case 0b001:
         return InstructionType::LH;
      case 0b010:
         return InstructionType::LW;
      case 0b011:
         return InstructionType::LD;
      case 0b100:
         return InstructionType::LBU;
      case 0b101:
         return InstructionType::LHU;
      case 0b110:
         return InstructionType::LWU;
      }
      break;

   case 0x23: // Stores
      if (funct3 == 2)
         return InstructionType::SW;
      break;

   case 0x63: // Branch
      if (funct3 == 0)
         return InstructionType::BEQ;
      break;

   case 0x6F:
      return InstructionType::JAL;

   case 0x0F:
   {
      break;
   }

   case 0x53:
      return InstructionType::OP_FP;

   case 0x07:
      if (funct3 == 3)
         return InstructionType::FLD;
      break;

   case 0x27: // FSD (Already in your code)
      if (funct3 == 3)
         return InstructionType::FSD;
      break;

   case 0x1B:
      if (funct3 == 0)
         return InstructionType::ADDIW;
      if (funct3 == 1)
         return InstructionType::SLLIW;
      if (funct3 == 5)
      {
         if (funct7 == 0x20)
            return InstructionType::SRAIW;
         return InstructionType::SRLIW;
      }
      break;

   case 0x73: // SYSTEM / CSR instructions
      if (funct3 == 0)
      {
         uint32_t imm12 = instruction >> 20;
         switch (imm12)
         {
         case 0x000:
            return InstructionType::ECALL;
         case 0x001:
            return InstructionType::EBREAK;
         case 0x302:
            return InstructionType::MRET; // Machine Return (Crucial!)
         case 0x102:
            return InstructionType::SRET; // Supervisor Return
         case 0x105:
            return InstructionType::WFI; // Wait for Interrupt
         default:
            return InstructionType::UNKNOWN;
         }
      }
      else
      {
         // CSR Access Instructions
         switch (funct3)
         {
         case 1:
            return InstructionType::CSRRW;
         case 2:
            return InstructionType::CSRRS;
         case 3:
            return InstructionType::CSRRC;
         case 5:
            return InstructionType::CSRRWI;
         case 6:
            return InstructionType::CSRRSI;
         case 7:
            return InstructionType::CSRRCI;
         default:
            return InstructionType::UNKNOWN;
         }
      }
      break;
   }

   return InstructionType::UNKNOWN;
}

DecodedInstruction Interpreter::decode(uint32_t raw, Processor &processor, uint8_t length)
{
   DecodedInstruction inst{};

   // Set the length and PC first
   inst.length = length;
   inst.pc = processor.program_counter;

   inst.opcode = raw & 0x7F;
   inst.type = interpret(raw);

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

// Helper Functions

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
   // Pass the length to the decode function
   DecodedInstruction inst = decode(raw, processor, length);

   if (inst.opcode == 0x0F)
   {
      processor.write_reg(0, 0);
      return true;
   }

   InstructionFormat format = get_format(inst.opcode);

   uint32_t lookup_funct7 = inst.funct7;
   if (format != InstructionFormat::R)
   {
      bool is_shift = (inst.opcode == 0x13 || inst.opcode == 0x1B) &&
                      (inst.funct3 == 1 || inst.funct3 == 5);

      if (!is_shift)
      {
         lookup_funct7 = 0;
      }
      else
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
      DEBUG_BEGIN()
      Debug::writeString("INSTRUCTION LOOKUP RETURNED NULL");
      DEBUG_END()
      processor.raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, inst.pc);
   }

   processor.write_reg(0, 0);
   return true;
}
