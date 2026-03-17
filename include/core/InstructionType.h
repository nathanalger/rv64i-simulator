#pragma once

enum class InstructionType
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

enum class InstructionFormat
{
   R,
   I,
   S,
   B,
   U,
   J
};