#include "Processor.h"
#include "Debug.h"
#include "Constants.h"

uint64_t Processor::readCSR(uint16_t address)
{
   uint32_t privilege_required = (address >> 8) & 0x3;
   if (static_cast<uint32_t>(mode) < privilege_required)
   {
      io->writeString("Unprivileged Access Observed \n");
      raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, program_counter, bus.readWord(program_counter));
      return 0;
   }

   switch (address)
   {
   case 0x015:
      return 0;
   case 0x100:
   case 0x120:
      return mstatus & SSTATUS_MASK;
   case 0x300:
      return mstatus;
   case 0x301:
      return misa;
   case 0x302:
      return medeleg;
   case 0x303:
      return mideleg;
   case 0x304:
      return mie;
   case 0x305:
      return mtvec;
   case 0x340:
      return mscratch;
   case 0x341:
      return mepc;
   case 0x342:
      return mcause;
   case 0x343:
      return mtval;
   case 0x344:
      return mip;

   case 0xC00:
   case 0xB00:
   case 0xC02:
   case 0xB02:
      return step_count;

   case 0xF11:
      return 0;
   case 0xF12:
      return 0x80000000;
   case 0xF13:
      return 0x20240501;
   case 0xF14:
      return 0;
   case 0x306:
      return mcounteren;

   case 0xC01:
      return step_count / 100;

   case 0x001:
      return fcsr & 0x1F;
   case 0x002:
      return (fcsr >> 5) & 0x7;
   case 0x003:
      return fcsr & 0xFF;
   case 0x104:
      return sie;
   case 0x105:
      return stvec;
   case 0x140:
      return sscratch;
   case 0x141:
      return sepc;
   case 0x142:
      return scause;
   case 0x143:
      return stval;
   case 0x144:
      return sip;
   case 0x180:
      return satp;
   case 0x106:
      return scounteren;
   case 0x30A:
      return menvcfg;
   case 0x10A:
      return senvcfg;

   case 0x3A0 ... 0x3AF:
   {
      uint16_t idx = address - 0x3A0;

      if (idx % 2 == 0)
      {
         return pmpcfg[idx];
      }
      return 0;
   }
   case 0x3B0 ... 0x3EF:
      return pmpaddr[address - 0x3B0];

   case 0xB03 ... 0xB1F:
      return 0;
   case 0x320 ... 0x33F:
      return 0;

   default:
      io->writeString("Unimplemented CSR Read: ");
      io->writeInt(address);
      io->writeString("\n");
      raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, program_counter, bus.readWord(program_counter));
      return 0;
   }
}

void Processor::writeCSR(uint16_t address, uint64_t val)
{
   uint32_t privilege_required = (address >> 8) & 0x3;
   if (static_cast<uint32_t>(mode) < privilege_required)
   {
      io->writeString("Unprivileged Access\n");
      raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, program_counter, bus.readWord(program_counter));
      return;
   }

   switch (address)
   {
   case 0x100:
   case 0x120:
      mstatus = (mstatus & ~SSTATUS_MASK) | (val & SSTATUS_MASK);
      break;
   case 0x300:
      mstatus = val;
      break;
   case 0x301:
      break;
   case 0x302:
      medeleg = val & ~(1ULL << 11);
      break;
   case 0x303:
      mideleg = val & ~((1ULL << 7) | (1ULL << 3));
      break;
   case 0x304:
      mie = val;
      break;
   case 0x305:
      mtvec = val;
      break;
   case 0x340:
      mscratch = val;
      break;
   case 0x341:
      mepc = val;
      break;
   case 0x342:
      mcause = val;
      break;
   case 0x343:
      mtval = val;
      break;
   case 0x344:
      mip = val;
      break;
   case 0x306:
      mcounteren = val;
      return;
   case 0x30A:
      menvcfg = val;
      break;
   case 0x10A:
      senvcfg = val;
      break;
   case 0x104:
      sie = val;
      break;
   case 0x105:
      stvec = val;
      break;
   case 0x140:
      sscratch = val;
      break;
   case 0x141:
      sepc = val;
      break;
   case 0x142:
      scause = val;
      break;
   case 0x143:
      stval = val;
      break;
   case 0x144:
      sip = val;
      break;
   case 0x180:
      satp = val;
      break;
   case 0x106:
      scounteren = val;
      return;

   case 0x001:
      fcsr = (fcsr & ~0x1F) | (val & 0x1F);
      break;
   case 0x002:
      fcsr = (fcsr & ~0xE0) | ((val & 0x7) << 5);
      break;
   case 0x003:
      fcsr = val & 0xFF;
      break;

   case 0x3A0 ... 0x3AF:
   {
      uint16_t idx = address - 0x3A0;
      if (idx % 2 == 0)
      {
         pmpcfg[idx] = val;
      }
      break;
   }
   case 0x3B0 ... 0x3EF:
      pmpaddr[address - 0x3B0] = val;
      break;

   case 0xB03 ... 0xB1F:
   case 0x320 ... 0x33F:
      return;

   default:
      io->writeString("Unimplemented CSR Read: ");
      io->writeInt(address);
      io->writeString("\n");
      raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, program_counter, bus.readWord(program_counter));
      break;
   }
}
