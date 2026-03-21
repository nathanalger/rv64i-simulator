#include "DefaultRegistry.h"
#include "InstructionRegistry.h"
#include "Processor.h"
#include "Debug.h"
#include "IODevice.h"
#include "Constants.h"

void exec_ecall(const DecodedInstruction & /*inst*/, Processor &processor)
{
   if (env != nullptr)
   {
      // Host environment
      env->handle_ecall(processor);
      processor.program_counter += 4;
   }
   else
   {
      // No host environment
      if (processor.mode == PrivilegeMode::User)
      {
         processor.raiseTrap(TrapCause::ECALL_FROM_U_MODE, processor.program_counter);
      }
      else if (processor.mode == PrivilegeMode::Supervisor)
      {
         processor.raiseTrap(TrapCause::ECALL_FROM_S_MODE, processor.program_counter);
      }
      else
      {
         processor.raiseTrap(TrapCause::ECALL_FROM_M_MODE, processor.program_counter);
      }
   }
}

void exec_ebreak(const DecodedInstruction & /*inst*/, Processor &processor)
{
   if (env != nullptr)
   {
      // Host Mode
      env->handle_ebreak(processor);
      processor.program_counter += 4;
   }
   else
   {
      // Core Mode
      processor.raiseTrap(TrapCause::BREAKPOINT, processor.program_counter);
   }
}

void exec_csrrw(const DecodedInstruction &inst, Processor &processor)
{
   // Atomic Read/Write: Read old value, write new value from rs1
   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);
   uint64_t old_val = processor.readCSR(csr_addr);

   // Write rs1 to CSR
   processor.writeCSR(csr_addr, processor.registers[inst.rs1]);

   // Write old CSR value to rd
   if (inst.rd != 0)
   {
      processor.registers[inst.rd] = old_val;
   }

   processor.program_counter += 4;
}

void exec_csrrs(const DecodedInstruction &inst, Processor &processor)
{
   // Atomic Read and Set Bits: Read old value, CSR = old | rs1
   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);
   uint64_t old_val = processor.readCSR(csr_addr);

   if (inst.rd != 0)
   {
      processor.registers[inst.rd] = old_val;
   }

   // Only write if rs1 is not x0 (standard RISC-V optimization/behavior)
   if (inst.rs1 != 0)
   {
      processor.writeCSR(csr_addr, old_val | processor.registers[inst.rs1]);
   }

   processor.program_counter += 4;
}

void exec_csrrc(const DecodedInstruction &inst, Processor &processor)
{
   // Atomic Read and Clear Bits: Read old value, CSR = old & ~rs1
   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);
   uint64_t old_val = processor.readCSR(csr_addr);

   if (inst.rd != 0)
   {
      processor.registers[inst.rd] = old_val;
   }

   if (inst.rs1 != 0)
   {
      processor.writeCSR(csr_addr, old_val & ~processor.registers[inst.rs1]);
   }

   processor.program_counter += 4;
}

void exec_csrrwi(const DecodedInstruction &inst, Processor &processor)
{
   // Immediate Read/Write
   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);
   uint64_t old_val = processor.readCSR(csr_addr);

   // In immediate CSR instructions, the rs1 field holds the 5-bit zero-extended immediate
   uint64_t zimm = inst.rs1;

   processor.writeCSR(csr_addr, zimm);

   if (inst.rd != 0)
   {
      processor.registers[inst.rd] = old_val;
   }

   processor.program_counter += 4;
}

void exec_csrrsi(const DecodedInstruction &inst, Processor &processor)
{
   // Immediate Read and Set Bits
   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);
   uint64_t old_val = processor.readCSR(csr_addr);
   uint64_t zimm = inst.rs1;

   if (inst.rd != 0)
   {
      processor.registers[inst.rd] = old_val;
   }

   if (zimm != 0)
   {
      processor.writeCSR(csr_addr, old_val | zimm);
   }

   processor.program_counter += 4;
}

void exec_csrrci(const DecodedInstruction &inst, Processor &processor)
{
   // Immediate Read and Clear Bits
   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);
   uint64_t old_val = processor.readCSR(csr_addr);
   uint64_t zimm = inst.rs1;

   if (inst.rd != 0)
   {
      processor.registers[inst.rd] = old_val;
   }

   if (zimm != 0)
   {
      processor.writeCSR(csr_addr, old_val & ~zimm);
   }

   processor.program_counter += 4;
}

void exec_mret(const DecodedInstruction &, Processor &processor)
{
   // 1. Read current mstatus
   uint64_t mstatus = processor.readCSR(0x300);

   // 2. Extract MPP (Machine Previous Privilege)
   uint64_t mpp = (mstatus & MSTATUS_MPP_MASK) >> MSTATUS_MPP_SHIFT;
   processor.mode = static_cast<PrivilegeMode>(mpp);

   // 3. Restore MIE (Machine Interrupt Enable) from MPIE
   uint64_t mpie = (mstatus & MSTATUS_MPIE) >> 7;
   if (mpie)
   {
      mstatus |= MSTATUS_MIE;
   }
   else
   {
      mstatus &= ~MSTATUS_MIE;
   }

   // 4. Set MPIE to 1, and MPP to User Mode (0) as per RISC-V spec
   mstatus |= MSTATUS_MPIE;
   mstatus &= ~MSTATUS_MPP_MASK;

   // 5. Write back the updated mstatus
   processor.writeCSR(0x300, mstatus);

   // 6. Set PC to mepc (Machine Exception Program Counter)
   processor.program_counter = processor.readCSR(0x341);

   DEBUG_BEGIN()
   io->writeString("MRET -> Switched to Mode: ");
   io->writeInt(mpp);
   io->writeString(", Returning to PC: ");
   io->writeInt(processor.program_counter);
   DEBUG_END()

   // Notice we DO NOT increment program_counter by 4 here.
}

void exec_sret(const DecodedInstruction &, Processor &processor)
{
   // 1. Read current sstatus
   uint64_t sstatus = processor.readCSR(0x100);

   // 2. Extract SPP (Supervisor Previous Privilege) - 1 bit
   uint64_t spp = (sstatus & SSTATUS_SPP_MASK) >> SSTATUS_SPP_SHIFT;

   // SPP is 1 bit: 1 means Supervisor, 0 means User
   processor.mode = spp ? PrivilegeMode::Supervisor : PrivilegeMode::User;

   // 3. Restore SIE from SPIE
   uint64_t spie = (sstatus & SSTATUS_SPIE) >> 5;
   if (spie)
   {
      sstatus |= SSTATUS_SIE;
   }
   else
   {
      sstatus &= ~SSTATUS_SIE;
   }

   // 4. Set SPIE to 1, and SPP to User Mode (0)
   sstatus |= SSTATUS_SPIE;
   sstatus &= ~SSTATUS_SPP_MASK;

   // 5. Write back the updated sstatus
   processor.writeCSR(0x100, sstatus);

   // 6. Set PC to sepc (Supervisor Exception Program Counter)
   processor.program_counter = processor.readCSR(0x141);

   DEBUG_BEGIN()
   io->writeString("SRET -> Switched to Mode: ");
   io->writeInt(static_cast<uint32_t>(processor.mode));
   io->writeString(", Returning to PC: ");
   io->writeInt(processor.program_counter);
   DEBUG_END()
}

// Add this helper function above your register_csr function
static void exec_system_dispatch(const DecodedInstruction &inst, Processor &processor)
{
   // The 12-bit immediate field dictates the specific system instruction
   switch (inst.imm)
   {
   case 0x000:
      exec_ecall(inst, processor);
      break;
   case 0x001:
      exec_ebreak(inst, processor);
      break;
   case 0x102:
      exec_sret(inst, processor);
      break;
   case 0x302:
      exec_mret(inst, processor);
      break;
   default:
      // Unknown system instruction (e.g., wfi)
      processor.raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, processor.program_counter);
      break;
   }
}

// Update your registry function:
void DefaultRegistry::register_csr()
{
   // System & Trap Instructions (Using the I-type signature)
   InstructionRegistry::register_i(0x73, 0b000, exec_system_dispatch);

   // Standard CSRs (Register)
   InstructionRegistry::register_i(0x73, 0x1, exec_csrrw);
   InstructionRegistry::register_i(0x73, 0x2, exec_csrrs);
   InstructionRegistry::register_i(0x73, 0x3, exec_csrrc);

   // Standard CSRs (Immediate)
   InstructionRegistry::register_i(0x73, 0x5, exec_csrrwi);
   InstructionRegistry::register_i(0x73, 0x6, exec_csrrsi);
   InstructionRegistry::register_i(0x73, 0x7, exec_csrrci);
}