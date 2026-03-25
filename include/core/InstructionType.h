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
   UNKNOWN,

   LB,
   LH,
   LD,
   LBU,
   LHU,
   LWU,

   ADDIW,
   SLLIW,
   SRAIW,
   SRLIW,

   OP_FP,

   MRET,
   SRET,
   WFI,
   CSRRW,
   CSRRS,
   CSRRC,
   CSRRWI,
   CSRRSI,
   CSRRCI,

   FSD,
   FLD,

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