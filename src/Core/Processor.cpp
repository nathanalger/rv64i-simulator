#include "Processor.h"
#include "Memory.h"
#include "Debug.h"
#include "IODevice.h"
#include "Constants.h"

// TODO: Memory is not currently strictly protected.

const uint64_t STACK_GUARD = 8;

Processor::Processor(Bus &b) : bus(b)
{
   initialize();
}

void Processor::reset()
{
   // Set all registers to a low state
   for (int i = 0; i < 32; i++)
   {
      registers[i] = 0;
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
      registers[i] = 0;
   }

   // Reset program counter
   program_counter = 0;
   trap = false;
};

/**
 * Execute a single instruction
 */
bool Processor::step()
{
   // Store old pc to detect infinite-loop halts
   uint64_t old_pc = program_counter;

   uint64_t physical_pc;
   if (!translate(program_counter, physical_pc, AccessType::FETCH))
   {
      return false; // Translation failed (trap already raised inside translate)
   }

   // Fetch instruction using the TRANSLATED physical address
   uint32_t instruction = bus.readWord(physical_pc);

   // Send instruction to interpreter
   interpreter.handle(instruction, *this);

   // Enforce x0 = 0
   registers[0] = 0;

   // Detect bare-metal halt (infinite loop to itself)
   if (program_counter == old_pc)
   {
      raiseTrap(TrapCause::NONE, program_counter);
      trap_cause = TrapCause::NONE;
      trap = true;
   }

   checkInterrupts();

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

void Processor::raiseTrap(TrapCause cause, uint64_t trap_pc)
{
   // 1. Save the PC where the trap happened into mepc
   writeCSR(0x341, trap_pc);

   // 2. Write the cause to mcause
   writeCSR(0x342, static_cast<uint64_t>(cause));

   // 3. Write the trap value to mtval (0x343).
   // For now, setting it to 0 is perfectly fine. Advanced page faults use this later.
   writeCSR(0x343, 0);

   // 4. Update mstatus (0x300) to save current privilege and disable interrupts
   uint64_t mstatus = readCSR(0x300);

   // Extract current MIE (Machine Interrupt Enable) bit (Bit 3)
   uint64_t mie = (mstatus & 0x8) >> 3;

   // Clear MPIE (Bit 7), MIE (Bit 3), and MPP (Bits 11:12)
   mstatus &= ~(0x80 | 0x8 | 0x1800);

   // Set MPIE to the old MIE value
   mstatus |= (mie << 7);

   // Set MPP (Machine Previous Privilege) to our current mode before the trap
   mstatus |= (static_cast<uint64_t>(mode) << 11);

   // Write back the updated mstatus
   writeCSR(0x300, mstatus);

   // 5. Force the processor into Machine Mode to handle the trap
   mode = PrivilegeMode::Machine;

   // 6. Calculate the jump address from mtvec (0x305)
   uint64_t mtvec = readCSR(0x305);
   uint64_t base_address = mtvec & ~3ULL; // The top 62 bits
   uint64_t mode_flag = mtvec & 3ULL;     // The bottom 2 bits

   // If mode_flag is 1 (Vectored) AND this is an asynchronous interrupt
   if (mode_flag == 1 && (static_cast<uint64_t>(cause) & INTERRUPT_BIT))
   {
      // Strip the interrupt bit to get the raw exception code, multiply by 4
      uint64_t exception_code = static_cast<uint64_t>(cause) & ~INTERRUPT_BIT;
      program_counter = base_address + (4 * exception_code);
   }
   else
   {
      // Direct mode: All traps jump to the exact base address
      program_counter = base_address;
   }

   DEBUG_BEGIN()
   io->writeString("TRAP ");
   io->writeInt(static_cast<uint64_t>(cause));
   io->writeString(" redirected to Handler at: ");
   io->writeInt(program_counter);
   DEBUG_END()
}

bool Processor::translate(uint64_t vaddr, uint64_t &paddr, AccessType type)
{
   // 1. Machine mode usually bypasses the MMU
   if (mode == PrivilegeMode::Machine)
   {
      paddr = vaddr;
      return true;
   }

   // 2. Read SATP to check the MMU mode
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
      // Unsupported SATP mode
      return false;
   }

   // --- THE PAGE WALK ALGORITHM ---

   // Extract the three 9-bit Virtual Page Numbers (VPNs) and the 12-bit Offset
   uint64_t vpn[3];
   vpn[0] = (vaddr >> 12) & 0x1FF;
   vpn[1] = (vaddr >> 21) & 0x1FF;
   vpn[2] = (vaddr >> 30) & 0x1FF;
   uint64_t offset = vaddr & 0xFFF;

   // Start at the root page table (Level 2), getting the physical address from SATP
   uint64_t a = (satp_val & SATP_PPN_MASK) * PAGE_SIZE;
   int i = 2;

   while (true)
   {
      // Calculate the physical address of the Page Table Entry (PTE)
      // Each PTE is 8 bytes long.
      uint64_t pte_addr = a + (vpn[i] * 8);

      // Read 64-bit PTE (using two 32-bit reads)
      uint64_t pte = bus.readDouble(pte_addr);

      // Check if PTE is Valid. Also, Write-only pages are illegal in RISC-V.
      if ((pte & PTE_V) == 0 || (((pte & PTE_R) == 0) && ((pte & PTE_W) == PTE_W)))
      {
         goto page_fault;
      }

      // Check if it's a leaf node (Read, Write, or Execute bit is set)
      if ((pte & PTE_R) || (pte & PTE_W) || (pte & PTE_X))
      {
         // --- LEAF NODE FOUND ---

         // 1. Basic Permission Checks
         if (type == AccessType::FETCH && !(pte & PTE_X))
            goto page_fault;
         if (type == AccessType::LOAD && !(pte & PTE_R))
            goto page_fault;
         if (type == AccessType::STORE && !(pte & PTE_W))
            goto page_fault;

         // 2. Privilege Mode Checks
         // User mode cannot access Supervisor pages unless U bit is set
         if (mode == PrivilegeMode::User && !(pte & PTE_U))
            goto page_fault;
         // Supervisor generally can't access User pages
         if (mode == PrivilegeMode::Supervisor && (pte & PTE_U))
            goto page_fault;

         // 3. Extract Physical Page Number from PTE
         uint64_t pte_ppn = (pte >> PTE_PPN_SHIFT) & PTE_PPN_MASK;

         // 4. Calculate final physical address
         if (i == 0)
         {
            // Standard 4KB page
            paddr = (pte_ppn * PAGE_SIZE) + offset;
         }
         else
         {
            // Superpage (2MB or 1GB)
            uint64_t mask = (i == 1) ? 0x1FF : 0x3FFFF;
            // Superpages require the lower bits of PPN to be zero
            if ((pte_ppn & mask) != 0)
               goto page_fault;

            uint64_t adjusted_ppn = (pte_ppn & ~mask) | ((vaddr >> 12) & mask);
            paddr = (adjusted_ppn * PAGE_SIZE) + offset;
         }

         return true; // Translation successful!
      }
      else
      {
         i--;
         if (i < 0)
         {
            // Walked too deep, page table is corrupted
            goto page_fault;
         }
         // Set 'a' to the physical address of the next page table
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

   raiseTrap(cause, vaddr);
   return false;
}

uint8_t Processor::readMemoryByte(uint64_t vaddr)
{
   uint64_t paddr;
   if (translate(vaddr, paddr, AccessType::LOAD))
      return bus.readByte(paddr);
   raiseTrap(TrapCause::LOAD_ACCESS_FAULT, vaddr);
   return 0;
}

void Processor::writeMemoryByte(uint64_t vaddr, uint8_t value)
{
   uint64_t paddr;
   if (translate(vaddr, paddr, AccessType::STORE))
      bus.writeByte(paddr, value);
   else
      raiseTrap(TrapCause::STORE_ACCESS_FAULT, vaddr);
}

uint32_t Processor::readMemoryWord(uint64_t vaddr)
{
   uint64_t paddr;
   if (translate(vaddr, paddr, AccessType::LOAD))
   {
      return bus.readWord(paddr);
   }
   else
   {
      raiseTrap(TrapCause::LOAD_ACCESS_FAULT, vaddr);
      return 0;
   }
}

uint16_t Processor::readMemoryHalf(uint64_t vaddr)
{
   uint64_t paddr;
   if (translate(vaddr, paddr, AccessType::LOAD))
   {
      return bus.readHalf(paddr);
   }
   else
   {
      raiseTrap(TrapCause::LOAD_ACCESS_FAULT, vaddr);
      return 0;
   }
}

void Processor::writeMemoryHalf(uint64_t vaddr, uint16_t value)
{
   uint64_t paddr;
   if (translate(vaddr, paddr, AccessType::STORE))
   {
      bus.writeHalf(paddr, value);
   }
   else
   {
      raiseTrap(TrapCause::STORE_ACCESS_FAULT, vaddr);
   }
}

void Processor::writeMemoryWord(uint64_t vaddr, uint32_t value)
{
   uint64_t paddr;
   if (translate(vaddr, paddr, AccessType::STORE))
   {
      bus.writeWord(paddr, value);
   }
   else
   {
      raiseTrap(TrapCause::STORE_ACCESS_FAULT, vaddr);
   }
}

uint64_t Processor::readMemoryDouble(uint64_t vaddr)
{
   uint64_t paddr;
   if (translate(vaddr, paddr, AccessType::LOAD))
   {
      // Intercept CLINT reads
      if (paddr == CLINT_MTIME)
         return mtime;
      if (paddr == CLINT_MTIMECMP)
         return mtimecmp;

      return bus.readDouble(paddr);
   }
   raiseTrap(TrapCause::LOAD_ACCESS_FAULT, vaddr);
   return 0;
}

void Processor::writeMemoryDouble(uint64_t vaddr, uint64_t value)
{
   uint64_t paddr;
   if (translate(vaddr, paddr, AccessType::STORE))
   {
      // Intercept CLINT writes
      if (paddr == CLINT_MTIMECMP)
      {
         mtimecmp = value;
         // Clear the pending interrupt bit when a new compare value is written
         mip &= ~MIP_MTIP;
         return;
      }
      // Note: mtime is usually read-only to software, but some implementations allow writes.
      // We'll leave it out for strictness unless you find your OS needs it.

      bus.writeDouble(paddr, value);
   }
   else
   {
      raiseTrap(TrapCause::STORE_ACCESS_FAULT, vaddr);
   }
}

void Processor::checkInterrupts()
{
   // 1. Tick the hardware clock (1 tick per instruction for now)
   mtime++;

   // 2. Pending logic: Is the timer past the compare value?
   uint64_t mip = readCSR(0x344);
   if (mtime >= mtimecmp)
   {
      // Set Machine Timer Interrupt Pending (MTIP) - Bit 7
      writeCSR(0x344, mip | (1ULL << 7));
   }
   else
   {
      // Clear MTIP if mtimecmp was updated to the future
      writeCSR(0x344, mip & ~(1ULL << 7));
   }

   // 3. Re-read mip just in case it changed
   mip = readCSR(0x344);
   uint64_t mie_reg = readCSR(0x304); // Machine Interrupt Enable Register
   uint64_t mstatus = readCSR(0x300);

   // 4. Are machine interrupts globally enabled?
   bool m_interrupts_enabled = false;
   if (mode == PrivilegeMode::Machine)
   {
      m_interrupts_enabled = (mstatus & 0x8) != 0; // Check MIE bit in mstatus
   }
   else
   {
      m_interrupts_enabled = true; // Always enabled if in a lower privilege mode
   }

   // 5. If global interrupts are on, MTIE (Enable bit 7) is on, and MTIP (Pending bit 7) is on: FIRE!
   if (m_interrupts_enabled && (mie_reg & (1ULL << 7)) && (mip & (1ULL << 7)))
   {
      raiseTrap(TrapCause::MACHINE_TIMER_INTERRUPT, program_counter);
   }
}

uint64_t Processor::readCSR(uint16_t address)
{
   switch (address)
   {
   // Machine
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

   // Supervisor
   case 0x100:
      return mstatus & SSTATUS_MASK;
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

   default:
      // PMP Configuration Registers (0x3A0 - 0x3AF)
      if (address >= 0x3A0 && address <= 0x3AF)
      {
         return pmpcfg[address - 0x3A0];
      }
      // PMP Address Registers (0x3B0 - 0x3EF)
      if (address >= 0x3B0 && address <= 0x3EF)
      {
         return pmpaddr[address - 0x3B0];
      }

      // If we get here, it's a truly unimplemented CSR.
      raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, program_counter);
      return 0;
   }
}

void Processor::writeCSR(uint16_t address, uint64_t val)
{
   switch (address)
   {
   // Machine
   case 0x300:
      mstatus = val;
      break;
   case 0x301:
      misa = val;
      break;
   case 0x302:
      medeleg = val;
      break;
   case 0x303:
      mideleg = val;
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

   // Supervisor
   case 0x100:
      mstatus = (mstatus & ~SSTATUS_MASK) | (val & SSTATUS_MASK);
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

   default:
      // PMP Configuration Registers (0x3A0 - 0x3AF)
      if (address >= 0x3A0 && address <= 0x3AF)
      {
         pmpcfg[address - 0x3A0] = val;
         return;
      }
      // PMP Address Registers (0x3B0 - 0x3EF)
      if (address >= 0x3B0 && address <= 0x3EF)
      {
         pmpaddr[address - 0x3B0] = val;
         return;
      }

      // If we get here, it's a truly unimplemented CSR.
      raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, program_counter);
      break;
   }
}