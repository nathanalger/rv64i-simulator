#include "Interpreter.h"
#include "InstructionType.h"
#include "InstructionExecutors.h"

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
      if (funct3 == 2)
         return InstructionType::LW;
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
   }

   return InstructionType::UNKNOWN;
}

DecodedInstruction Interpreter::decode(uint32_t raw, Processor &processor)
{
   DecodedInstruction inst;

   inst.type = interpret(raw);

   inst.opcode = raw & 0x7F;
   inst.rd = (raw >> 7) & 0x1F;
   inst.funct3 = (raw >> 12) & 0x7;
   inst.rs1 = (raw >> 15) & 0x1F;
   inst.rs2 = (raw >> 20) & 0x1F;
   inst.funct7 = (raw >> 25) & 0x7F;

   inst.pc = processor.program_counter;

   inst.imm = (int32_t)raw >> 20;

   return inst;
}

/**
 * Note that this corresponds to the order of the InstructionType enumerator.
 * This dispatches the proper function based on the raw instruction type.
 */
Interpreter::ExecFunc Interpreter::dispatch[] =
    {
        exec_add,
        exec_sub,
        exec_addi,
        exec_lw,
        exec_sw,
        exec_beq,
        exec_jal,
        nullptr};

void Interpreter::handle(uint32_t raw, Processor &processor)
{
   DecodedInstruction inst = decode(raw, processor);

   ExecFunc func = dispatch[static_cast<int>(inst.type)];

   if (func)
      func(inst, processor);

   processor.registers[0] = 0;
}