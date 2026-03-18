#include "InstructionRegistry.h"
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
   uint32_t index = key & (TABLE_SIZE - 1);
   uint32_t start_index = index;

   // Linear probing
   while (table[index].func != nullptr)
   {

      index = (index + 1) & (TABLE_SIZE - 1);

      if (index == start_index)
      {
         return;
      }
   }

   table[index].key = key;
   table[index].mask = mask;
   table[index].func = func;
}

InstructionRegistry::ExecFunc InstructionRegistry::lookup(uint32_t raw)
{
   uint32_t index = raw & (TABLE_SIZE - 1);
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