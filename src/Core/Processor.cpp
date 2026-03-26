#include "Processor.h"
#include "Memory.h"
#include "Debug.h"
#include "IODevice.h"
#include "Constants.h"

struct CSRRange
{
   uint16_t start;
   uint16_t end;
};

const uint64_t STACK_GUARD = 8;

void Processor::write_reg(uint8_t rd, uint64_t value)
{
   if (rd != 0)
   {
      registers[rd] = value;
   }
}

Processor::Processor(Bus &b) : bus(b)
{
   initialize();
}

void Processor::reset()
{
   // Set all registers to a low state
   for (int i = 0; i < 32; i++)
   {
      write_reg(i, 0);
   }

   // Reset program counter
   program_counter = 0;
   trap = false;
};

void Processor::initialize()
{
   // Set all registers to a low state
   for (int i = 0; i < 32; i++)
   {
      write_reg(i, 0);
   }

   // Reset program counter
   program_counter = 0;
   trap = false;

   bus.setProcessor(this);
};

/**
 * Execute a single instruction
 */
bool Processor::step()
{
   mtime++;
   write_reg(0, 0);
   uint64_t physical_pc;

   if (program_counter == 0x80200000)
   {
      io->writeString(">>> KERNEL ENTRY REACHED <<<\n");
   }

   if (!translate(program_counter, physical_pc, AccessType::FETCH))
      return false;

   uint64_t pc_before_exec = program_counter;
   uint16_t first_half = bus.readHalf(physical_pc);
   uint32_t raw_instruction;
   uint8_t length;

   if ((first_half & 0x3) != 0x3)
   {
      // It's 16-bit compressed
      raw_instruction = decompress(first_half);
      length = 2;
   }
   else
   {
      // It's 32-bit. Watch out for page boundaries!
      uint64_t physical_pc_second;

      // If PC is at the very last 2 bytes of a 4KB page
      if ((program_counter & 0xFFF) == 0xFFE)
      {
         // Translate the next page to find the second half safely
         if (!translate(program_counter + 2, physical_pc_second, AccessType::FETCH))
            return false;
      }
      else
      {
         physical_pc_second = physical_pc + 2;
      }

      uint16_t second_half = bus.readHalf(physical_pc_second);
      raw_instruction = (static_cast<uint32_t>(second_half) << 16) | first_half;
      length = 4;
   }

   interpreter.handle(raw_instruction, *this, length);
   trap_pending = false;

   if (program_counter == pc_before_exec)
   {
      program_counter += length;
   }
   step_count++;
   write_reg(0, 0);
   return true;
}

/**
 * Run program continuously
 */
void Processor::run()
{
   while (!trap)
   {
      step();
   }

   if (trap_cause != TrapCause::NONE)
   {
      io->writeString("TRAP raised at PC: ");
      io->writeInt(trap_pc);
      io->writeString(". Cause: ");
      io->writeString(trapCauseToString(trap_cause));
      io->writeString("\n");
   }
   else
   {
      io->writeString("Program exited with code 0.\n");
   }
}

void Processor::raiseTrap(TrapCause cause, uint64_t trap_pc, uint64_t trap_value)
{
   trap_pending = true;

   bool is_interrupt = (static_cast<uint64_t>(cause) & INTERRUPT_BIT) != 0;
   uint64_t exception_code = static_cast<uint64_t>(cause) & ~INTERRUPT_BIT;
   bool delegate_to_s = false;

   if (mode <= PrivilegeMode::Supervisor)
   {
      if (is_interrupt)
         delegate_to_s = (mideleg & (1ULL << exception_code)) != 0;
      else
         delegate_to_s = (medeleg & (1ULL << exception_code)) != 0;
   }

   if (delegate_to_s)
   {
      writeCSR(0x141, trap_pc);
      writeCSR(0x142, static_cast<uint64_t>(cause));
      writeCSR(0x143, trap_value);

      uint64_t sstatus = readCSR(0x100);
      uint64_t sie = (sstatus & 0x2) >> 1;

      sstatus &= ~(0x20 | 0x2 | 0x100);

      sstatus |= (sie << 5);                                                                                    // SPIE = SIE
      sstatus |= (static_cast<uint64_t>(mode) == static_cast<uint64_t>(PrivilegeMode::User) ? 0 : (1ULL << 8)); // SPP = previous mode

      writeCSR(0x100, sstatus);
      mode = PrivilegeMode::Supervisor;

      uint64_t stvec_val = readCSR(0x105);
      uint64_t base_address = stvec_val & ~3ULL;
      uint64_t mode_flag = stvec_val & 3ULL;

      if (mode_flag == 1 && is_interrupt)
         program_counter = base_address + (4 * exception_code);
      else
         program_counter = base_address;
   }
   else
   {
      mode = PrivilegeMode::Machine;
      writeCSR(0x341, trap_pc);
      writeCSR(0x342, static_cast<uint64_t>(cause));
      writeCSR(0x343, trap_value);

      uint64_t mstatus = readCSR(0x300);
      uint64_t mie = (mstatus & 0x8) >> 3;

      mstatus &= ~(0x80 | 0x8 | 0x1800);
      mstatus |= (mie << 7);
      mstatus |= (static_cast<uint64_t>(mode) << 11);

      writeCSR(0x300, mstatus);
      mode = PrivilegeMode::Machine;

      uint64_t mtvec_val = readCSR(0x305);
      uint64_t base_address = mtvec_val & ~3ULL;
      uint64_t mode_flag = mtvec_val & 3ULL;

      if (mode_flag == 1 && is_interrupt)
         program_counter = base_address + (4 * exception_code);
      else
         program_counter = base_address;
   }

   DEBUG_BEGIN()
   Debug::writeString("\nSoftware Trap Encountered \n");
   Debug::writeString("Cause:     ");
   Debug::writeInt(static_cast<uint64_t>(cause));
   Debug::writeString(is_interrupt ? " (Interrupt)\n" : " (Exception)\n");
   Debug::writeString("Bad PC:    0x");
   Debug::writeInt(trap_pc);
   Debug::writeString("\nVal/Tval:  0x");
   Debug::writeInt(trap_value);
   Debug::writeString("\nMode:      ");
   Debug::writeString(delegate_to_s ? "Supervisor" : "Machine");

   if (delegate_to_s)
   {
      Debug::writeString("\n[S-Mode CSRs] sepc: 0x");
      Debug::writeInt(readCSR(0x141));
      Debug::writeString(" | scause: 0x");
      Debug::writeInt(readCSR(0x142));
      Debug::writeString(" | stval: 0x");
      Debug::writeInt(readCSR(0x143));
      Debug::writeString(" | sstatus: 0x");
      Debug::writeInt(readCSR(0x100));
   }
   else
   {
      Debug::writeString("\n[M-Mode CSRs] mepc: 0x");
      Debug::writeInt(readCSR(0x341));
      Debug::writeString(" | mcause: 0x");
      Debug::writeInt(readCSR(0x342));
      Debug::writeString(" | mtval: 0x");
      Debug::writeInt(readCSR(0x343));
      Debug::writeString(" | mstatus: 0x");
      Debug::writeInt(readCSR(0x300));
   }

   Debug::writeString("\nJumped to: 0x");
   Debug::writeInt(program_counter);
   Debug::writeString("\nEnd Trap Details \n");
   DEBUG_END()
}

bool Processor::translate(uint64_t vaddr, uint64_t &paddr, AccessType type)
{
   if (mode == PrivilegeMode::Machine)
   {
      paddr = vaddr;
      return true;
   }

   uint64_t satp_val = readCSR(0x180);
   uint64_t satp_mode = (satp_val & SATP_MODE_MASK) >> SATP_MODE_SHIFT;

   if (satp_mode == SATP_MODE_BARE)
   {
      // MMU is off
      paddr = vaddr;
      return true;
   }

   if (satp_mode != SATP_MODE_SV39)
   {
      return false;
   }

   uint64_t vpn[3];
   vpn[0] = (vaddr >> 12) & 0x1FF;
   vpn[1] = (vaddr >> 21) & 0x1FF;
   vpn[2] = (vaddr >> 30) & 0x1FF;
   uint64_t offset = vaddr & 0xFFF;

   uint64_t a = (satp_val & SATP_PPN_MASK) * PAGE_SIZE;
   int i = 2;

   while (true)
   {
      uint64_t pte_addr = a + (vpn[i] * 8);

      uint64_t pte = bus.readDouble(pte_addr);

      if ((pte & PTE_V) == 0 || (((pte & PTE_R) == 0) && ((pte & PTE_W) == PTE_W)))
      {
         goto page_fault;
      }

      if ((pte & PTE_R) || (pte & PTE_W) || (pte & PTE_X))
      {
         if (type == AccessType::FETCH && !(pte & PTE_X))
            goto page_fault;
         if (type == AccessType::LOAD && !(pte & PTE_R))
            goto page_fault;
         if (type == AccessType::STORE && !(pte & PTE_W))
            goto page_fault;

         if (mode == PrivilegeMode::User && !(pte & PTE_U))
            goto page_fault;
         if (mode == PrivilegeMode::Supervisor && (pte & PTE_U))
            goto page_fault;

         uint64_t pte_ppn = (pte >> PTE_PPN_SHIFT) & PTE_PPN_MASK;

         if (i == 0)
         {
            paddr = (pte_ppn * PAGE_SIZE) + offset;
         }
         else
         {
            uint64_t mask = (i == 1) ? 0x1FF : 0x3FFFF;
            if ((pte_ppn & mask) != 0)
               goto page_fault;

            uint64_t adjusted_ppn = (pte_ppn & ~mask) | ((vaddr >> 12) & mask);
            paddr = (adjusted_ppn * PAGE_SIZE) + offset;
         }

         return true;
      }
      else
      {
         i--;
         if (i < 0)
         {
            goto page_fault;
         }
         a = ((pte >> PTE_PPN_SHIFT) & PTE_PPN_MASK) * PAGE_SIZE;
      }
   }

page_fault:
   TrapCause cause;
   if (type == AccessType::FETCH)
      cause = TrapCause::INSTRUCTION_ACCESS_FAULT;
   else if (type == AccessType::LOAD)
      cause = TrapCause::LOAD_ACCESS_FAULT;
   else
      cause = TrapCause::STORE_ACCESS_FAULT;

   raiseTrap(cause, program_counter, vaddr);
   return false;
}

void Processor::checkInterrupts()
{
   uint64_t mip = readCSR(0x344);
   if (mtime >= mtimecmp)
   {
      writeCSR(0x344, mip | (1ULL << 7));
   }
   else
   {
      writeCSR(0x344, mip & ~(1ULL << 7));
   }
   mip = readCSR(0x344);
   uint64_t mie_reg = readCSR(0x304);
   uint64_t mstatus = readCSR(0x300);

   bool m_interrupts_enabled = false;
   if (mode == PrivilegeMode::Machine)
   {
      m_interrupts_enabled = (mstatus & 0x8) != 0;
   }
   else
   {
      m_interrupts_enabled = true;
   }

   if (m_interrupts_enabled && (mie_reg & (1ULL << 7)) && (mip & (1ULL << 7)))
   {
      raiseTrap(TrapCause::MACHINE_TIMER_INTERRUPT, program_counter, 0);
   }

   if (m_interrupts_enabled && (mie_reg & (1ULL << 3)) && (mip & (1ULL << 3)))
   {
      raiseTrap(TrapCause::MACHINE_SOFTWARE_INTERRUPT, program_counter, 0);
   }
}

bool Processor::checkPMP(uint64_t addr, AccessType type)
{
   for (int i = 0; i < 64; i++)
   {
      uint8_t cfg = (pmpcfg[i / 8] >> ((i % 8) * 8)) & 0xFF;
      uint8_t mode = (cfg >> 3) & 0x3;
      if (mode == 0)
         continue;

      uint64_t start = 0, end = 0;
      if (mode == 1)
      {
         start = (i == 0) ? 0 : (pmpaddr[i - 1] << 2);
         end = (pmpaddr[i] << 2);
      }

      if (addr >= start && addr < end)
      {
         bool r = cfg & 0x1;
         bool w = cfg & 0x2;
         bool x = cfg & 0x4;
         bool l = cfg & 0x8;

         if (this->mode == PrivilegeMode::Machine && !l)
            return true;

         if (type == AccessType::LOAD && !r)
            return false;
         if (type == AccessType::STORE && !w)
            return false;
         if (type == AccessType::FETCH && !x)
            return false;

         return true;
      }
   }

   return (this->mode == PrivilegeMode::Machine);
}