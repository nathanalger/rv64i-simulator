#pragma once
#include <cstdint>
#include "InstructionType.h"

class Processor;

struct DecodedInstruction
{
   // Instruction type for execution
   InstructionType type;

   // Standard instruction fields
   uint32_t opcode;
   uint32_t rd;
   uint32_t rs1;
   uint32_t rs2;
   uint32_t funct3;
   uint32_t funct7;
   int32_t imm;

   // Helpful for branch instructions
   int64_t pc;
};

class Interpreter
{
public:
   using ExecFunc = void (*)(const DecodedInstruction &, Processor &);
   bool handle(uint32_t raw, Processor &processor);

private:
   InstructionType interpret(uint32_t instruction);
   DecodedInstruction decode(uint32_t raw, Processor &processor);
};