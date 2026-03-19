#pragma once
#include "Interpreter.h"

enum class MaskType
{
   R,
   I,
   S,
   B,
   U,
   J,
   OPCODE_ONLY
};

class InstructionRegistry
{
public:
   using ExecFunc = Interpreter::ExecFunc;

   static void init();
   static void register_inst(uint32_t key, MaskType type, ExecFunc func);
   static ExecFunc lookup(uint32_t raw);
   static uint32_t make_key(uint32_t opcode, uint32_t funct3, uint32_t funct7);
   static void register_r(uint32_t opcode, uint32_t funct3, uint32_t funct7, ExecFunc func);
   static void register_i(uint32_t opcode, uint32_t funct3, ExecFunc func);
   static void register_s(uint32_t opcode, uint32_t funct3, ExecFunc func);
   static void register_b(uint32_t opcode, uint32_t funct3, ExecFunc func);
   static void register_u(uint32_t opcode, ExecFunc func);
   static void register_opcode(uint32_t opcode, ExecFunc func);

private:
   static void register_inst(uint32_t key, uint32_t mask, ExecFunc func);
   static constexpr int TABLE_SIZE = 2048;

   struct Entry
   {
      uint32_t key;
      uint32_t mask;
      ExecFunc func;
   };

   static Entry table[TABLE_SIZE];
};