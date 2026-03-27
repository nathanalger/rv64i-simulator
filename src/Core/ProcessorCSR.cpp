#include "Processor.h"
#include "Debug.h"
#include "Constants.h"

uint64_t Processor::readCSR(uint16_t address)
{
   uint32_t privilege_required = (address >> 8) & 0x3;

   if (static_cast<uint32_t>(mode) < privilege_required)
   {
      io->writeString("Unprivileged Read Access: ");
      io->writeInt(address);
      io->writeString("\n");
      raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, program_counter, bus.readWord(program_counter));
      return 0;
   }

   uint64_t result = 0;
   if (!readCSRRaw(address, result))
   {
      io->writeString("Unimplemented CSR Read: ");
      io->writeInt(address);
      io->writeString("\n");
      raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, program_counter, bus.readWord(program_counter));
      return 0;
   }

   return result;
}

void Processor::writeCSR(uint16_t address, uint64_t val)
{
   uint32_t privilege_required = (address >> 8) & 0x3;
   bool is_read_only = ((address >> 10) & 0x3) == 0x3;

   if (static_cast<uint32_t>(mode) < privilege_required || is_read_only)
   {
      io->writeString(is_read_only ? "Write to Read-Only CSR: " : "Unprivileged Write Access: ");
      io->writeInt(address);
      io->writeString("\n");
      raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, program_counter, bus.readWord(program_counter));
      return;
   }

   if (!writeCSRRaw(address, val))
   {
      io->writeString("Unimplemented CSR Write: ");
      io->writeInt(address);
      io->writeString("\n");
      raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, program_counter, bus.readWord(program_counter));
   }
}

bool Processor::readCSRRaw(uint16_t address, uint64_t &out_val)
{
   switch (address)
   {
   case 0x015:
      out_val = 0;
      break;
   case 0x100:
   case 0x120:
      out_val = mstatus & SSTATUS_MASK;
      break;
   case 0x300:
      out_val = mstatus;
      break;
   case 0x301:
      out_val = misa;
      break;
   case 0x302:
      out_val = medeleg;
      break;
   case 0x303:
      out_val = mideleg;
      break;
   case 0x304:
      out_val = mie;
      break;
   case 0x305:
      out_val = mtvec;
      break;
   case 0x340:
      out_val = mscratch;
      break;
   case 0x341:
      out_val = mepc;
      break;
   case 0x342:
      out_val = mcause;
      break;
   case 0x343:
      out_val = mtval;
      break;
   case 0x344:
      out_val = mip;
      break;

   case 0xC00:
   case 0xB00:
   case 0xC02:
   case 0xB02:
      out_val = step_count;
      break;

   case 0xF11:
      out_val = 0;
      break;
   case 0xF12:
      out_val = 0x80000000;
      break;
   case 0xF13:
      out_val = 0x20240501;
      break;
   case 0xF14:
      out_val = 0;
      break;
   case 0x306:
      out_val = mcounteren;
      break;

   case 0xC01:
      out_val = step_count / 100;
      break;

   case 0x001:
      out_val = fcsr & 0x1F;
      break;
   case 0x002:
      out_val = (fcsr >> 5) & 0x7;
      break;
   case 0x003:
      out_val = fcsr & 0xFF;
      break;
   case 0x104:
      out_val = mie & mideleg;
      break;
   case 0x105:
      out_val = stvec;
      break;
   case 0x140:
      out_val = sscratch;
      break;
   case 0x141:
      out_val = sepc;
      break;
   case 0x142:
      out_val = scause;
      break;
   case 0x143:
      out_val = stval;
      break;
   case 0x144:
      out_val = mip & mideleg;
      break;
   case 0x180:
      out_val = satp;
      break;
   case 0x106:
      out_val = scounteren;
      break;
   case 0x30A:
      out_val = menvcfg;
      break;
   case 0x10A:
      out_val = senvcfg;
      break;

   case 0x3A0 ... 0x3AF:
   {
      uint16_t idx = address - 0x3A0;
      out_val = (idx % 2 == 0) ? pmpcfg[idx / 2] : 0;
      break;
   }
   case 0x3B0 ... 0x3EF:
      out_val = pmpaddr[address - 0x3B0];
      break;

   case 0xB03 ... 0xB1F:
   case 0x320 ... 0x33F:
      out_val = 0;
      break;

   default:
      return false;
   }
   return true;
}

bool Processor::writeCSRRaw(uint16_t address, uint64_t val)
{
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
      break;
   case 0x30A:
      menvcfg = val;
      break;
   case 0x10A:
      senvcfg = val;
      break;
   case 0x104:
      mie = (mie & ~mideleg) | (val & mideleg);
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
      mip = (mip & ~mideleg) | (val & mideleg);
      break;
   case 0x180:
      satp = val;
      break;
   case 0x106:
      scounteren = val;
      break;

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
         pmpcfg[idx / 2] = val;
      break;
   }
   case 0x3B0 ... 0x3EF:
      pmpaddr[address - 0x3B0] = val;
      break;

   case 0xB03 ... 0xB1F:
   case 0x320 ... 0x33F:
      break;

   default:
      return false;
   }
   return true;
}