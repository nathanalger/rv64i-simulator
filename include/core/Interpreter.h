#pragma once
#include <stddef.h>
#include <stdint.h>

class Processor;

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
   int64_t pc;
   int64_t length;
};

class Interpreter
{
public:
   using ExecFunc = void (*)(const DecodedInstruction &, Processor &);
   bool handle(uint32_t raw, Processor &processor, uint8_t length);

private:
   DecodedInstruction decode(uint32_t raw, Processor &processor, uint8_t length);
};