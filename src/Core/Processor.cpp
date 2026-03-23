#include "Processor.h"
#include "Memory.h"
#include "Debug.h"
#include "IODevice.h"
#include "Constants.h"

#include <cstdlib>

// TODO: Memory is not currently strictly protected.

struct CSRRange
{
   uint16_t start;
   uint16_t end;
};

const CSRRange PROBE_SHIELD[] = {
    {0x107, 0x11F}, // S-Mode Configs (includes 268/0x10C)
    {0x307, 0x31F}, // M-Mode Configs (includes 780/0x30C)
    {0x345, 0x34F}, // M-Mode Extra Traps
    {0x3A0, 0x3EF}, // PMP (Physical Memory Protection)
    {0x600, 0x7FF}, // Hypervisor & Custom Extensions
    {0xDA0, 0xDBF}, // Scalar Crypto / Entropy
    {0xFB0, 0xFBF}, // Debug / Trigger Context
    {0xC00, 0xC1F}, // User-mode Performance Counters
    {0xF00, 0xF10}};

bool isOptionalProbe(uint16_t address)
{
   if (address == 0x14D)
      return true; // Single outlier: Count Overflow
   for (const auto &range : PROBE_SHIELD)
   {
      if (address >= range.start && address <= range.end)
         return true;
   }
   return false;
}

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

   bus.setProcessor(this);
};

/**
 * Execute a single instruction
 */
bool Processor::step()
{
   mtime++;
   uint64_t physical_pc;
   if (!translate(program_counter, physical_pc, AccessType::FETCH))
      return false;

   uint64_t pc_before_exec = program_counter;
   uint16_t first_half = bus.readHalf(physical_pc);
   uint32_t raw_instruction;
   uint8_t length;

   if ((first_half & 0x3) != 0x3)
   {
      // It's compressed
      raw_instruction = decompress(first_half);
      length = 2;
   }
   else
   {
      // It's 32-bit
      uint16_t second_half = bus.readHalf(physical_pc + 2);
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
   registers[0] = 0;
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
   trap_pending = true;
   writeCSR(0x341, trap_pc);
   writeCSR(0x342, static_cast<uint64_t>(cause));

   uint64_t trap_value = 0;
   if (cause == TrapCause::ILLEGAL_INSTRUCTION)
   {
      uint16_t first = bus.readHalf(trap_pc);
      if ((first & 0x3) != 0x3)
      {
         trap_value = first; // 16-bit compressed
      }
      else
      {
         uint16_t second = bus.readHalf(trap_pc + 2);
         trap_value = (static_cast<uint32_t>(second) << 16) | first;
      }
   }

   writeCSR(0x343, trap_value);

   // 4. Update mstatus (0x300) to save current privilege and disable interrupts
   uint64_t current_mstatus = readCSR(0x300);

   // Extract current MIE (Machine Interrupt Enable) bit (Bit 3)
   uint64_t mie = (current_mstatus & 0x8) >> 3;

   // Clear MPIE (Bit 7), MIE (Bit 3), and MPP (Bits 11:12)
   current_mstatus &= ~(0x80 | 0x8 | 0x1800);

   // Set MPIE to the old MIE value
   current_mstatus |= (mie << 7);

   // Set MPP (Machine Previous Privilege) to our current mode before the trap
   current_mstatus |= (static_cast<uint64_t>(mode) << 11);

   // Write back the updated mstatus
   writeCSR(0x300, current_mstatus);

   // 5. Force the processor into Machine Mode to handle the trap
   mode = PrivilegeMode::Machine;

   // 6. Calculate the jump address from mtvec (0x305)
   uint64_t current_mtvec = readCSR(0x305);
   uint64_t base_address = current_mtvec & ~3ULL; // The top 62 bits
   uint64_t mode_flag = current_mtvec & 3ULL;     // The bottom 2 bits

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
   io->writeString(" at PC: ");
   io->writeInt(trap_pc);
   io->writeString(" | Raw Instr/Tval: ");
   io->writeInt(trap_value);
   io->writeString(" | Redirected to: ");
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
   uint32_t privilege_required = (address >> 8) & 0x3;
   if (static_cast<uint32_t>(mode) < privilege_required)
   {
      DEBUG_BEGIN()
      io->writeString("READCSR TOP");
      DEBUG_END()
      raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, program_counter);
      return 0;
   }

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

   case 0xC00: // cycle
   case 0xB00: // mcycle
   case 0xC02: // instret
   case 0xB02: // minstret
      return step_count;

      // Machine Information
   case 0xF11:
      return 0; // mvendorid
   case 0xF12:
      return 0; // marchid
   case 0xF13:
      return 0; // mimpid
   case 0xF14:
      return 0; // mhartid (Current CPU ID)
   case 0x306:
      return mcounteren;

   case 0xC01:                 // time
      return step_count / 100; // Simplified scaling

   case 0x001:
      return fcsr & 0x1F; // fflags
   case 0x002:
      return (fcsr >> 5) & 0x7; // frm
   case 0x003:
      return fcsr & 0xFF; // fcsr (flags + frm)

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
   case 0x106:
      return scounteren;
   case 0x30A: // menvcfg
      return menvcfg;
   case 0x10A: // senvcfg
      return senvcfg;

   case 0xB03 ... 0xB1F: // mhpmcounter3 to mhpmcounter31
      return 0;
   case 0x320 ... 0x33F: // mhpmevent3 to mhpmevent31
      return 0;

   default:
      if (isOptionalProbe(address))
      {
         return 0;
      }

      DEBUG_BEGIN()
      io->writeString("READCSR BOTTOM - Unimplemented CSR: ");
      io->writeInt(address);
      io->writeString("\n");
      DEBUG_END()

      // If we get here, it's a truly unimplemented CSR.
      raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, program_counter);
      return 0;
   }
}

void Processor::writeCSR(uint16_t address, uint64_t val)
{
   uint32_t privilege_required = (address >> 8) & 0x3;
   if (static_cast<uint32_t>(mode) < privilege_required)
   {
      DEBUG_BEGIN()
      io->writeString("WRITECSR TOP");
      exit(0);
      DEBUG_END()
      raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, program_counter);
      return;
   }

   switch (address)
   {
   // Machine
   case 0x300:
      mstatus = val;
      break;
   case 0x301:
      // Readonly
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
   case 0x306:
      mcounteren = val;
      return;
   case 0x30A:
      menvcfg = val;
      break;
   case 0x10A:
      senvcfg = val;
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

   case 0xB03 ... 0xB1F: // mhpmcounter3 to mhpmcounter31
   case 0x320 ... 0x33F: // mhpmevent3 to mhpmevent31
      return;

   default:
      if (isOptionalProbe(address))
      {
         return;
      }

      // If we get here, it's a truly unimplemented CSR.
      DEBUG_BEGIN()
      io->writeString("WRITECSR BOTTOM - Unimplemented CSR: ");
      io->writeInt(address);
      io->writeString("\n");
      DEBUG_END()

      raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, program_counter);
      break;
   }
}

uint32_t Processor::decompress(uint16_t c)
{
   uint8_t op = c & 0x3; // Quadrant
   uint8_t funct3 = (c >> 13) & 0x7;

   switch (op)
   {
   case 0: // Quadrant 0
      switch (funct3)
      {
      case 0:
      { // C.ADDI4SPN -> addi rd', x2, imm
         if (c == 0)
            return 0xFFFFFFFF; // All zeros is an illegal instruction

         uint32_t rd = ((c >> 2) & 0x7) + 8;
         uint32_t imm = (((c >> 7) & 0xF) << 6) |
                        (((c >> 11) & 0x3) << 4) |
                        (((c >> 5) & 0x1) << 3) |
                        (((c >> 6) & 0x1) << 2);

         // Return: ADDI rd, x2, imm
         return (imm << 20) | (2 << 15) | (0 << 12) | (rd << 7) | 0x13;
      }
      case 2:
      { // C.LW -> lw rd', offset(rs1')
         uint32_t rd = ((c >> 2) & 0x7) + 8;
         uint32_t rs1 = ((c >> 7) & 0x7) + 8;
         uint32_t imm = (((c >> 6) & 0x1) << 2) | (((c >> 10) & 0x7) << 3) | (((c >> 5) & 0x1) << 6);
         return (imm << 20) | (rs1 << 15) | (2 << 12) | (rd << 7) | 0x03;
      }
      case 3:
      { // C.LD -> ld rd', offset(rs1')
         uint32_t rd = ((c >> 2) & 0x7) + 8;
         uint32_t rs1 = ((c >> 7) & 0x7) + 8;
         uint32_t imm = (((c >> 10) & 0x7) << 3) | (((c >> 5) & 0x3) << 6);
         return (imm << 20) | (rs1 << 15) | (3 << 12) | (rd << 7) | 0x03;
      }
      case 6:
      { // C.SW -> sw rs2', offset(rs1')
         uint32_t rs2 = ((c >> 2) & 0x7) + 8;
         uint32_t rs1 = ((c >> 7) & 0x7) + 8;
         uint32_t imm = (((c >> 6) & 0x1) << 2) | (((c >> 10) & 0x7) << 3) | (((c >> 5) & 0x1) << 6);
         return (((imm >> 5) & 0x7F) << 25) | (rs2 << 20) | (rs1 << 15) | (2 << 12) | ((imm & 0x1F) << 7) | 0x23;
      }
      case 7:
      { // C.SD -> sd rs2', offset(rs1')
         uint32_t rs2 = ((c >> 2) & 0x7) + 8;
         uint32_t rs1 = ((c >> 7) & 0x7) + 8;
         uint32_t imm = (((c >> 10) & 0x7) << 3) | (((c >> 5) & 0x3) << 6);
         return (((imm >> 5) & 0x7F) << 25) | (rs2 << 20) | (rs1 << 15) | (3 << 12) | ((imm & 0x1F) << 7) | 0x23;
      }
      }
      break;

   case 1: // Quadrant 1
      switch (funct3)
      {
      case 0:
      { // C.ADDI -> addi rd, rd, imm
         uint32_t rd = (c >> 7) & 0x1F;
         if (rd == 0)
            return 0x00000013; // C.NOP
         int32_t imm = ((c >> 2) & 0x1F) | ((c & 0x1000) ? 0xFFFFFFE0 : 0);
         return ((imm & 0xFFF) << 20) | (rd << 15) | (0 << 12) | (rd << 7) | 0x13;
      }
      case 1:
      { // C.ADDIW -> addiw rd, rd, imm
         uint32_t rd = (c >> 7) & 0x1F;
         int32_t imm = ((c >> 2) & 0x1F) | ((c & 0x1000) ? 0xFFFFFFE0 : 0);
         return ((imm & 0xFFF) << 20) | (rd << 15) | (0 << 12) | (rd << 7) | 0x1B;
      }
      case 2:
      { // C.LI -> addi rd, x0, imm (This is what failed!)
         uint32_t rd = (c >> 7) & 0x1F;
         int32_t imm = ((c >> 2) & 0x1F) | ((c & 0x1000) ? 0xFFFFFFE0 : 0);
         return ((imm & 0xFFF) << 20) | (0 << 15) | (0 << 12) | (rd << 7) | 0x13;
      }
      case 3:
      { // C.LUI / C.ADDI16SP
         uint32_t rd = (c >> 7) & 0x1F;
         if (rd == 2)
         { // C.ADDI16SP -> addi x2, x2, imm
            int32_t imm = (((c >> 6) & 0x1) << 4) | (((c >> 2) & 0x1) << 5) | (((c >> 5) & 0x1) << 6) | (((c >> 3) & 0x3) << 7) | ((c & 0x1000) ? 0xFFFFFE00 : 0);
            return ((imm & 0xFFF) << 20) | (2 << 15) | (0 << 12) | (2 << 7) | 0x13;
         }
         else
         { // C.LUI -> lui rd, imm
            int32_t imm = (((c >> 2) & 0x1F) << 12) | ((c & 0x1000) ? 0xFFFE0000 : 0);
            return (imm & 0xFFFFF000) | (rd << 7) | 0x37;
         }
      }
      case 4:
      { // ALU Operations (SRLI, SRAI, ANDI, SUB, XOR, OR, AND, etc.)
         uint32_t funct2 = (c >> 10) & 0x3;
         uint32_t rd = ((c >> 7) & 0x7) + 8;
         if (funct2 == 0)
         { // C.SRLI -> srli rd, rd, shamt
            uint32_t shamt = (((c >> 2) & 0x1F) | (((c >> 12) & 0x1) << 5));
            return (shamt << 20) | (rd << 15) | (5 << 12) | (rd << 7) | 0x13;
         }
         else if (funct2 == 1)
         { // C.SRAI -> srai rd, rd, shamt
            uint32_t shamt = (((c >> 2) & 0x1F) | (((c >> 12) & 0x1) << 5));
            return (0x20 << 25) | (shamt << 20) | (rd << 15) | (5 << 12) | (rd << 7) | 0x13;
         }
         else if (funct2 == 2)
         { // C.ANDI -> andi rd, rd, imm
            int32_t imm = ((c >> 2) & 0x1F) | ((c & 0x1000) ? 0xFFFFFFE0 : 0);
            return ((imm & 0xFFF) << 20) | (rd << 15) | (7 << 12) | (rd << 7) | 0x13;
         }
         else if (funct2 == 3)
         {
            uint32_t rs2 = ((c >> 2) & 0x7) + 8;
            uint32_t funct1 = (c >> 5) & 0x3;
            if ((c >> 12) & 1)
            { // ADDW / SUBW
               if (funct1 == 0)
                  return (0x20 << 25) | (rs2 << 20) | (rd << 15) | (0 << 12) | (rd << 7) | 0x3B; // SUBW
               if (funct1 == 1)
                  return (0x00 << 25) | (rs2 << 20) | (rd << 15) | (0 << 12) | (rd << 7) | 0x3B; // ADDW
            }
            else
            { // SUB / XOR / OR / AND
               if (funct1 == 0)
                  return (0x20 << 25) | (rs2 << 20) | (rd << 15) | (0 << 12) | (rd << 7) | 0x33; // SUB
               if (funct1 == 1)
                  return (0x00 << 25) | (rs2 << 20) | (rd << 15) | (4 << 12) | (rd << 7) | 0x33; // XOR
               if (funct1 == 2)
                  return (0x00 << 25) | (rs2 << 20) | (rd << 15) | (6 << 12) | (rd << 7) | 0x33; // OR
               if (funct1 == 3)
                  return (0x00 << 25) | (rs2 << 20) | (rd << 15) | (7 << 12) | (rd << 7) | 0x33; // AND
            }
         }
      }
      break;
      case 5:
      { // C.J -> jal x0, offset
         int32_t imm = (((c >> 3) & 0x7) << 1) | (((c >> 11) & 0x1) << 4) | (((c >> 2) & 0x1) << 5) | (((c >> 7) & 0x1) << 6) | (((c >> 6) & 0x1) << 7) | (((c >> 9) & 0x3) << 8) | (((c >> 8) & 0x1) << 10) | (((c >> 12) & 0x1) << 11);
         if (c & 0x1000)
            imm |= 0xFFFFF000;
         uint32_t imm20_10_1_11_19_12 = (((imm >> 20) & 0x1) << 31) | (((imm >> 1) & 0x3FF) << 21) | (((imm >> 11) & 0x1) << 20) | (((imm >> 12) & 0xFF) << 12);
         return imm20_10_1_11_19_12 | (0 << 7) | 0x6F;
      }
      case 6: // C.BEQZ -> beq rs1', x0, offset
      case 7:
      { // C.BNEZ -> bne rs1', x0, offset
         uint32_t rs1 = ((c >> 7) & 0x7) + 8;
         int32_t imm = (((c >> 3) & 0x3) << 1) | (((c >> 10) & 0x3) << 3) | (((c >> 2) & 0x1) << 5) | (((c >> 5) & 0x3) << 6) | (((c >> 12) & 0x1) << 8);
         if (c & 0x1000)
            imm |= 0xFFFFFE00;
         uint32_t funct3_b = (funct3 == 6) ? 0 : 1;
         return (((imm >> 12) & 0x1) << 31) | (((imm >> 5) & 0x3F) << 25) | (0 << 20) | (rs1 << 15) | (funct3_b << 12) | (((imm >> 1) & 0xF) << 8) | (((imm >> 11) & 0x1) << 7) | 0x63;
      }
      }
      break;

   case 2: // Quadrant 2
      switch (funct3)
      {
      case 0:
      { // C.SLLI -> slli rd, rd, shamt
         uint32_t rd = (c >> 7) & 0x1F;
         uint32_t shamt = ((c >> 2) & 0x1F) | (((c >> 12) & 0x1) << 5);
         return (shamt << 20) | (rd << 15) | (1 << 12) | (rd << 7) | 0x13;
      }
      case 1:
      { // C.FLDSP -> fld rd, offset(x2)
         uint32_t rd = (c >> 7) & 0x1F;

         // Offset calculation (matches C.LDSP)
         uint32_t imm = (((c >> 5) & 0x3) << 3) | (((c >> 12) & 0x1) << 5) | (((c >> 2) & 0x7) << 6);

         // Map to: FLD rd, offset(x2)
         // I-Type format: opcode = 0x07, funct3 = 3, rs1 = 2 (sp)
         return (imm << 20) | (2 << 15) | (3 << 12) | (rd << 7) | 0x07;
      }
      case 2:
      { // C.LWSP -> lw rd, offset(x2)
         uint32_t rd = (c >> 7) & 0x1F;
         uint32_t imm = (((c >> 4) & 0x7) << 2) | (((c >> 12) & 0x1) << 5) | (((c >> 2) & 0x3) << 6);
         return (imm << 20) | (2 << 15) | (2 << 12) | (rd << 7) | 0x03;
      }
      case 3:
      { // C.LDSP -> ld rd, offset(x2)
         uint32_t rd = (c >> 7) & 0x1F;
         uint32_t imm = (((c >> 5) & 0x3) << 3) | (((c >> 12) & 0x1) << 5) | (((c >> 2) & 0x7) << 6);
         return (imm << 20) | (2 << 15) | (3 << 12) | (rd << 7) | 0x03;
      }
      case 4:
      {
         uint32_t rd = (c >> 7) & 0x1F;
         uint32_t rs2 = (c >> 2) & 0x1F;
         if ((c & 0x1000) == 0)
         {
            if (rs2 == 0)
               return (0 << 20) | (rd << 15) | (0 << 12) | (0 << 7) | 0x67; // C.JR -> jalr x0, rd, 0
            else
               return (0 << 25) | (rs2 << 20) | (0 << 15) | (0 << 12) | (rd << 7) | 0x33; // C.MV -> add rd, x0, rs2
         }
         else
         {
            if (rs2 == 0)
            {
               if (rd == 0)
                  return 0x00100073; // C.EBREAK
               else
                  return (0 << 20) | (rd << 15) | (0 << 12) | (1 << 7) | 0x67; // C.JALR -> jalr x1, rd, 0
            }
            else
               return (0 << 25) | (rs2 << 20) | (rd << 15) | (0 << 12) | (rd << 7) | 0x33; // C.ADD -> add rd, rd, rs2
         }
      }
      case 5: // C.FSDSP
      {
         uint32_t rs2 = (c >> 2) & 0x1F;

         // Offset for C.FSDSP: imm[5:3] are at c[12:10], imm[8:6] are at c[9:7]
         uint32_t offset = (((c >> 10) & 0x7) << 3) | (((c >> 7) & 0x7) << 6);

         // Convert to S-Type Immediate format
         uint32_t imm11_5 = (offset >> 5) & 0x7F;
         uint32_t imm4_0 = offset & 0x1F;

         // Map to: FSD rs2, offset(x2)
         // opcode = 0x27, funct3 = 3, rs1 = 2 (sp)
         return (imm11_5 << 25) | (rs2 << 20) | (2 << 15) | (3 << 12) | (imm4_0 << 7) | 0x27;
      }
      case 6:
      { // C.SWSP -> sw rs2, offset(x2)
         uint32_t rs2 = (c >> 2) & 0x1F;
         uint32_t imm = (((c >> 9) & 0xF) << 2) | (((c >> 7) & 0x3) << 6);
         return (((imm >> 5) & 0x7F) << 25) | (rs2 << 20) | (2 << 15) | (2 << 12) | ((imm & 0x1F) << 7) | 0x23;
      }
      case 7:
      { // C.SDSP -> sd rs2, offset(x2)
         uint32_t rs2 = (c >> 2) & 0x1F;
         uint32_t imm = (((c >> 10) & 0x7) << 3) | (((c >> 7) & 0x7) << 6);
         return (((imm >> 5) & 0x7F) << 25) | (rs2 << 20) | (2 << 15) | (3 << 12) | ((imm & 0x1F) << 7) | 0x23;
      }
      }
      break;
   }

   // Fallback triggers a proper Trap 2 (Illegal Instruction)
   return 0xFFFFFFFF;
}