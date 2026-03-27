#include "DefaultRegistry.h"
#include "InstructionRegistry.h"
#include "Processor.h"
#include "Debug.h"
#include "IODevice.h"
#include "Constants.h"

void exec_ecall(const DecodedInstruction &inst, Processor &processor)
{
   if (env != nullptr)
   {
      env->handle_ecall(processor);
   }
   else
   {
      if (processor.mode == PrivilegeMode::User)
         processor.raiseTrap(TrapCause::ECALL_FROM_U_MODE, inst.pc, 0);
      else if (processor.mode == PrivilegeMode::Supervisor)
         processor.raiseTrap(TrapCause::ECALL_FROM_S_MODE, inst.pc, 0);
      else
         processor.raiseTrap(TrapCause::ECALL_FROM_M_MODE, inst.pc, 0);
   }
}

void exec_ebreak(const DecodedInstruction &inst, Processor &processor)
{
   processor.raiseTrap(TrapCause::BREAKPOINT, inst.pc, inst.pc);

   if (env != nullptr)
   {
      env->handle_ebreak(processor);
   }

   if (io)
   {
      Debug::dump();
   }
}

void exec_mret(const DecodedInstruction &, Processor &processor)
{
   if (processor.mode < PrivilegeMode::Machine)
   {
      processor.raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, processor.program_counter, processor.bus.readWord(processor.program_counter));
      return;
   }

   uint64_t mstatus = 0;
   processor.readCSRRaw(0x300, mstatus);
   uint64_t mpp = (mstatus & MSTATUS_MPP_MASK) >> MSTATUS_MPP_SHIFT;

   if ((mstatus & MSTATUS_MPIE))
      mstatus |= MSTATUS_MIE;
   else
      mstatus &= ~MSTATUS_MIE;

   mstatus |= MSTATUS_MPIE;
   mstatus &= ~MSTATUS_MPP_MASK;

   processor.writeCSRRaw(0x300, mstatus);

   uint64_t mepc = 0;
   processor.readCSRRaw(0x341, mepc);
   processor.program_counter = mepc;

   processor.mode = static_cast<PrivilegeMode>(mpp);

   TRACE_BEGIN()
   Debug::writeString("MRET -> Switched to Mode: ");
   Debug::writeInt(mpp);
   Debug::writeString(", Target PC: ");
   Debug::writeInt(processor.program_counter);
   DEBUG_END()
}

void exec_sret(const DecodedInstruction &inst, Processor &processor)
{
   if (processor.mode < PrivilegeMode::Supervisor)
   {
      processor.raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, inst.pc, processor.bus.readWord(inst.pc));
      return;
   }

   uint64_t sstatus = 0;
   processor.readCSRRaw(0x100, sstatus);
   uint64_t spp = (sstatus & SSTATUS_SPP_MASK) >> SSTATUS_SPP_SHIFT;

   if ((sstatus & SSTATUS_SPIE))
      sstatus |= SSTATUS_SIE;
   else
      sstatus &= ~SSTATUS_SIE;

   sstatus |= SSTATUS_SPIE;
   sstatus &= ~SSTATUS_SPP_MASK;

   processor.writeCSRRaw(0x100, sstatus);

   uint64_t sepc = 0;
   processor.readCSRRaw(0x141, sepc);
   processor.program_counter = sepc;

   processor.mode = spp ? PrivilegeMode::Supervisor : PrivilegeMode::User;
}

void exec_csrrw(const DecodedInstruction &inst, Processor &processor)
{
   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);
   uint64_t value_to_write = processor.registers[inst.rs1];
   uint64_t old_val = 0;

   // 1. Conditionally read (only if rd != 0)
   if (inst.rd != 0)
   {
      old_val = processor.readCSR(csr_addr);
      if (processor.trap_pending)
         return;
   }

   // 2. Unconditionally write
   processor.writeCSR(csr_addr, value_to_write);
   if (processor.trap_pending)
      return;

   // 3. Commit to register file safely
   if (inst.rd != 0)
   {
      processor.write_reg(inst.rd, old_val);
   }
}

void exec_csrrs(const DecodedInstruction &inst, Processor &processor)
{
   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);
   uint64_t rs1_val = processor.registers[inst.rs1];

   // 1. Unconditionally read
   uint64_t old_val = processor.readCSR(csr_addr);
   if (processor.trap_pending)
      return;

   // 2. Conditionally write (only if rs1 != 0)
   if (inst.rs1 != 0)
   {
      processor.writeCSR(csr_addr, old_val | rs1_val);
      if (processor.trap_pending)
         return;
   }

   // 3. Commit to register file safely
   if (inst.rd != 0)
   {
      processor.write_reg(inst.rd, old_val);
   }
}

void exec_csrrc(const DecodedInstruction &inst, Processor &processor)
{
   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);
   uint64_t rs1_val = processor.registers[inst.rs1];

   // 1. Unconditionally read
   uint64_t old_val = processor.readCSR(csr_addr);
   if (processor.trap_pending)
      return;

   // 2. Conditionally write (only if rs1 != 0)
   if (inst.rs1 != 0)
   {
      processor.writeCSR(csr_addr, old_val & ~rs1_val);
      if (processor.trap_pending)
         return;
   }

   // 3. Commit to register file safely
   if (inst.rd != 0)
   {
      processor.write_reg(inst.rd, old_val);
   }
}

void exec_csrrwi(const DecodedInstruction &inst, Processor &processor)
{
   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);
   uint64_t zimm = inst.rs1; // For immediate variants, rs1 holds the zimm value
   uint64_t old_val = 0;

   // 1. Conditionally read (only if rd != 0)
   if (inst.rd != 0)
   {
      old_val = processor.readCSR(csr_addr);
      if (processor.trap_pending)
         return;
   }

   // 2. Unconditionally write
   processor.writeCSR(csr_addr, zimm);
   if (processor.trap_pending)
      return;

   // 3. Commit to register file safely
   if (inst.rd != 0)
   {
      processor.write_reg(inst.rd, old_val);
   }
}

void exec_csrrsi(const DecodedInstruction &inst, Processor &processor)
{
   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);
   uint64_t zimm = inst.rs1;

   // 1. Unconditionally read
   uint64_t old_val = processor.readCSR(csr_addr);
   if (processor.trap_pending)
      return;

   // 2. Conditionally write (only if zimm != 0)
   if (zimm != 0)
   {
      processor.writeCSR(csr_addr, old_val | zimm);
      if (processor.trap_pending)
         return;
   }

   // 3. Commit to register file safely
   if (inst.rd != 0)
   {
      processor.write_reg(inst.rd, old_val);
   }
}

void exec_csrrci(const DecodedInstruction &inst, Processor &processor)
{
   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);
   uint64_t zimm = inst.rs1;

   // 1. Unconditionally read
   uint64_t old_val = processor.readCSR(csr_addr);
   if (processor.trap_pending)
      return;

   // 2. Conditionally write (only if zimm != 0)
   if (zimm != 0)
   {
      processor.writeCSR(csr_addr, old_val & ~zimm);
      if (processor.trap_pending)
         return;
   }

   // 3. Commit to register file safely
   if (inst.rd != 0)
   {
      processor.write_reg(inst.rd, old_val);
   }
}

static void exec_system_dispatch(const DecodedInstruction &inst, Processor &processor)
{
   switch (inst.imm & 0xFFF)
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
   case 0x105: // WFI (Wait for Interrupt)
      break;
   default:
      TRACE_BEGIN()
      Debug::writeString("UNIMPLEMENTED CSR/SYSTEM INSTRUCTION");
      DEBUG_END()
      processor.raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, inst.pc, processor.bus.readWord(inst.pc));
      break;
   }
}

void DefaultRegistry::register_csr()
{
   InstructionRegistry::register_i(0x73, 0b000, exec_system_dispatch);
   InstructionRegistry::register_i(0x73, 0b001, exec_csrrw);
   InstructionRegistry::register_i(0x73, 0b010, exec_csrrs);
   InstructionRegistry::register_i(0x73, 0b011, exec_csrrc);
   InstructionRegistry::register_i(0x73, 0b101, exec_csrrwi);
   InstructionRegistry::register_i(0x73, 0b110, exec_csrrsi);
   InstructionRegistry::register_i(0x73, 0b111, exec_csrrci);
}