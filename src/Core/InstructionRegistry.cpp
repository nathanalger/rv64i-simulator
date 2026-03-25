#include "InstructionRegistry.h"
#include "DefaultRegistry.h"
#include "IODevice.h"

InstructionRegistry::Entry InstructionRegistry::table[TABLE_SIZE];

void InstructionRegistry::init()
{
   for (int i = 0; i < TABLE_SIZE; i++)
   {
      table[i].key = 0xFFFFFFFF;
      table[i].mask = 0;
      table[i].func = nullptr;
   }

   DefaultRegistry::init_default_instructions();
}

static uint32_t get_mask(MaskType type)
{
   switch (type)
   {
   case MaskType::R:
      // funct7 | funct3 | opcode
      return 0b1111111'00000'00000'111'00000'1111111;

   case MaskType::I:
      // funct3 | opcode
      return (0b111 << 12) | 0x7F;

   case MaskType::S:
      // funct3 | opcode
      return (0b111 << 12) | 0x7F;

   case MaskType::B:
      // funct3 | opcode
      return (0b111 << 12) | 0x7F;

   case MaskType::U:
      // opcode only
      return 0x7F;

   case MaskType::J:
      // opcode only
      return 0x7F;

   case MaskType::OPCODE_ONLY:
      return 0x7F;
   }

   return 0;
}

void InstructionRegistry::register_inst(uint32_t key, MaskType type, ExecFunc func)
{
   uint32_t mask = get_mask(type);
   register_inst(key, mask, func);
}

void InstructionRegistry::register_inst(uint32_t key, uint32_t mask, ExecFunc func)
{
   uint32_t index = (key & 0x7F) & (TABLE_SIZE - 1);
   uint32_t start_index = index;

   while (table[index].func != nullptr)
   {
      index = (index + 1) & (TABLE_SIZE - 1);
      if (index == start_index)
         return;
   }

   table[index].key = key;
   table[index].mask = mask;
   table[index].func = func;
}

void InstructionRegistry::register_r(uint32_t opcode, uint32_t funct3, uint32_t funct7, ExecFunc func)
{
   uint32_t key = make_key(opcode, funct3, funct7);
   uint32_t mask = get_mask(MaskType::R);
   register_inst(key, mask, func);
}

void InstructionRegistry::register_i(uint32_t opcode, uint32_t funct3, ExecFunc func)
{
   uint32_t key = opcode | (funct3 << 12);
   uint32_t mask = get_mask(MaskType::I);
   register_inst(key, mask, func);
}

void InstructionRegistry::register_s(uint32_t opcode, uint32_t funct3, ExecFunc func)
{
   uint32_t key = opcode | (funct3 << 12);
   uint32_t mask = get_mask(MaskType::S);
   register_inst(key, mask, func);
}

void InstructionRegistry::register_b(uint32_t opcode, uint32_t funct3, ExecFunc func)
{
   uint32_t key = opcode | (funct3 << 12);
   uint32_t mask = get_mask(MaskType::B);
   register_inst(key, mask, func);
}

void InstructionRegistry::register_u(uint32_t opcode, ExecFunc func)
{
   uint32_t mask = get_mask(MaskType::U);
   register_inst(opcode, mask, func);
}

void InstructionRegistry::register_exact(uint32_t key, ExecFunc func)
{
   register_inst(key, 0xFFFFFFFF, func);
}

void InstructionRegistry::register_opcode(uint32_t opcode, ExecFunc func)
{
   uint32_t mask = get_mask(MaskType::OPCODE_ONLY);
   register_inst(opcode, mask, func);
}

InstructionRegistry::ExecFunc InstructionRegistry::lookup(uint32_t raw)
{
   // Hash only using the 7-bit opcode
   uint32_t index = (raw & 0x7F) & (TABLE_SIZE - 1);
   uint32_t start_index = index;

   while (table[index].func != nullptr)
   {
      if ((raw & table[index].mask) == table[index].key)
      {
         return table[index].func;
      }

      index = (index + 1) & (TABLE_SIZE - 1);
      if (index == start_index)
         break;
   }

   return nullptr;
}

uint32_t InstructionRegistry::make_key(uint32_t opcode, uint32_t funct3, uint32_t funct7)
{
   return (opcode) | (funct3 << 12) | (funct7 << 25);
}