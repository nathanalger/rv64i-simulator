#include "Interpreter.h"

Instruction Interpreter::interpret(uint32_t instruction)
{

   uint32_t opcode = instruction & 0x7F;
   uint32_t funct3 = (instruction >> 12) & 0x7;
   uint32_t funct7 = (instruction >> 25) & 0x7F;

   switch (opcode)
   {

   case 0x33: // R-Type
      if (funct3 == 0 && funct7 == 0x00)
         return ADD;

      if (funct3 == 0 && funct7 == 0x20)
         return SUB;

      break;

   case 0x13: // I-Type arithmetic
      if (funct3 == 0)
         return ADDI;
      break;

   case 0x03: // Loads
      if (funct3 == 2)
         return LW;
      break;

   case 0x23: // Stores
      if (funct3 == 2)
         return SW;
      break;

   case 0x63: // Branch
      if (funct3 == 0)
         return BEQ;
      break;

   case 0x6F:
      return JAL;
   }

   return UNKNOWN;
}

void Interpreter::execute(const DecodedInstruction &Instruction, Processor &processor)
{
}

DecodedInstruction Interpreter::decode(uint32_t raw, Processor &processor)
{
   DecodedInstruction inst;

   inst.opcode = raw & 0x7F;
   inst.rd = (raw >> 7) & 0x1F;
   inst.funct3 = (raw >> 12) & 0x7;
   inst.rs1 = (raw >> 15) & 0x1F;
   inst.rs2 = (raw >> 20) & 0x1F;
   inst.funct7 = (raw >> 25) & 0x7F;

   inst.pc = processor.program_counter;

   inst.imm = (int32_t)raw >> 20;
}

void Interpreter::handle(uint32_t raw, Processor &processor)
{
   execute(decode(raw, processor), processor);
}