#pragma once
#include <cstdint>
#include "Processor.h"
#include "InstructionType.h"

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
   uint64_t pc;
};

class Interpreter
{
private:
   using ExecFunc = void (*)(const DecodedInstruction &, Processor &);

   static ExecFunc dispatch[];

   InstructionType interpret(uint32_t instruction);
   DecodedInstruction decode(uint32_t raw, Processor &processor);

   // Helper Functions

   InstructionFormat get_format(int32_t opcode);
   int32_t sign_extend(int32_t, int);
   int32_t decode_i(uint32_t raw);
   int32_t decode_s(uint32_t raw);
   int32_t decode_b(uint32_t raw);
   int32_t decode_u(uint32_t raw);
   int32_t decode_j(uint32_t raw);

public:
   void handle(uint32_t raw, Processor &processor);
};