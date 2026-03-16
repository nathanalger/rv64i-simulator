#pragma once
#include <cstdint>
#include "Processor.h"

enum Instruction
{
   ADD,
   SUB,
   ADDI,
   LW,
   SW,
   BEQ,
   JAL,
   UNKNOWN
};

struct DecodedInstruction
{
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
   /**
    * Takes a provided instruction and returns the instruction being carried out.
    */
   Instruction interpret(uint32_t instruction);
   DecodedInstruction decode(uint32_t raw, Processor &processor);
   void execute(const DecodedInstruction &Instruction, Processor &processor);

public:
   /**
    * Handles a standard RV64I Instruction
    */
   void handle(uint32_t raw, Processor &processor);
};