#pragma once

enum class InstructionType
{
   // RV64I
   ADD,
   SUB,
   ADDI,
   LW,
   SW,
   BEQ,
   JAL,
   UNKNOWN,

   // Loads
   LB,
   LH,
   LD,
   LBU,
   LHU,
   LWU,

   ECALL,
   EBREAK
};

enum class InstructionFormat
{
   R,
   I,
   S,
   B,
   U,
   J
};