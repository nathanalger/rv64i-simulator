#include "DefaultRegistry.h"
#include "InstructionRegistry.h"
#include "Processor.h"
#include "Debug.h"
#include "IODevice.h"
#include "Constants.h"

void exec_ecall(const DecodedInstruction &inst, Processor &processor)
{
   // Use inst.pc instead of processor.program_counter for trap reporting
   if (env != nullptr)
   {
      env->handle_ecall(processor);
   }
   else
   {
      if (processor.mode == PrivilegeMode::User)
         processor.raiseTrap(TrapCause::ECALL_FROM_U_MODE, inst.pc);
      else if (processor.mode == PrivilegeMode::Supervisor)
         processor.raiseTrap(TrapCause::ECALL_FROM_S_MODE, inst.pc);
      else
         processor.raiseTrap(TrapCause::ECALL_FROM_M_MODE, inst.pc);
   }
}

void exec_ebreak(const DecodedInstruction &inst, Processor &processor)
{

   if (env != nullptr)
   {
      env->handle_ebreak(processor);
   }
   else
   {
      processor.raiseTrap(TrapCause::BREAKPOINT, inst.pc);
   }

   if (io)
   {
      Debug::dump();
      io->pause();
   }
}

void exec_mret(const DecodedInstruction &, Processor &processor)
{
   if (processor.mode < PrivilegeMode::Machine)
   {
      processor.raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, processor.program_counter);
      return;
   }

   uint64_t mstatus = processor.readCSR(0x300);
   uint64_t mpp = (mstatus & MSTATUS_MPP_MASK) >> MSTATUS_MPP_SHIFT;

   processor.mode = static_cast<PrivilegeMode>(mpp);

   // Restore MIE from MPIE
   if ((mstatus & MSTATUS_MPIE))
      mstatus |= MSTATUS_MIE;
   else
      mstatus &= ~MSTATUS_MIE;

   // Set MPIE to 1, MPP to 0
   mstatus |= MSTATUS_MPIE;
   mstatus &= ~MSTATUS_MPP_MASK;

   processor.writeCSR(0x300, mstatus);

   processor.program_counter = processor.readCSR(0x341);

   TRACE_BEGIN()
   Debug::writeString("MRET -> Switched to Mode: ");
   Debug::writeInt(mpp);
   Debug::writeString(", Target PC: ");
   Debug::writeInt(processor.program_counter);
   DEBUG_END()
}

void exec_sret(const DecodedInstruction &, Processor &processor)
{
   if (processor.mode < PrivilegeMode::Supervisor)
   {
      processor.raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, processor.program_counter);
      return;
   }

   uint64_t sstatus = processor.readCSR(0x100);
   uint64_t spp = (sstatus & SSTATUS_SPP_MASK) >> SSTATUS_SPP_SHIFT;

   processor.mode = spp ? PrivilegeMode::Supervisor : PrivilegeMode::User;

   if ((sstatus & SSTATUS_SPIE))
      sstatus |= SSTATUS_SIE;
   else
      sstatus &= ~SSTATUS_SIE;

   sstatus |= SSTATUS_SPIE;
   sstatus &= ~SSTATUS_SPP_MASK;

   processor.writeCSR(0x100, sstatus);
   processor.program_counter = processor.readCSR(0x141);
}

void exec_csrrw(const DecodedInstruction &inst, Processor &processor)
{
   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);

   if (inst.rd != 0)
   {
      uint64_t old_val = processor.readCSR(csr_addr);
      if (processor.trap_pending)
         return;
      processor.write_reg(inst.rd, old_val);
   }

   processor.writeCSR(csr_addr, processor.registers[inst.rs1]);
}

void exec_csrrs(const DecodedInstruction &inst, Processor &processor)
{

   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);
   uint64_t old_val = processor.readCSR(csr_addr);

   if (processor.trap_pending)
      return;

   if (inst.rd != 0)
      processor.write_reg(inst.rd, old_val);

   if (inst.rs1 != 0)
   {
      processor.writeCSR(csr_addr, old_val | processor.registers[inst.rs1]);
   }
}

void exec_csrrc(const DecodedInstruction &inst, Processor &processor)
{
   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);
   uint64_t old_val = processor.readCSR(csr_addr);

   if (inst.rd != 0)
      processor.write_reg(inst.rd, old_val);
   if (inst.rs1 != 0)
   {
      processor.writeCSR(csr_addr, old_val & ~processor.registers[inst.rs1]);
   }
}

// Immediate variants
void exec_csrrwi(const DecodedInstruction &inst, Processor &processor)
{
   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);
   uint64_t old_val = processor.readCSR(csr_addr);
   uint64_t zimm = inst.rs1; // 5-bit zero extended immediate

   processor.writeCSR(csr_addr, zimm);
   if (inst.rd != 0)
      processor.write_reg(inst.rd, old_val);
}

void exec_csrrsi(const DecodedInstruction &inst, Processor &processor)
{
   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);
   uint64_t old_val = processor.readCSR(csr_addr);
   uint64_t zimm = inst.rs1;

   if (inst.rd != 0)
      processor.write_reg(inst.rd, old_val);
   if (zimm != 0)
      processor.writeCSR(csr_addr, old_val | zimm);
}

void exec_csrrci(const DecodedInstruction &inst, Processor &processor)
{
   uint16_t csr_addr = static_cast<uint16_t>(inst.imm & 0xFFF);
   uint64_t old_val = processor.readCSR(csr_addr);
   uint64_t zimm = inst.rs1;

   if (inst.rd != 0)
      processor.write_reg(inst.rd, old_val);
   if (zimm != 0)
      processor.writeCSR(csr_addr, old_val & ~zimm);
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
   case 0x105:
      break;
   default:
      TRACE_BEGIN()
      Debug::writeString("UNIMPLEMENTED CSR");
      DEBUG_END()
      processor.raiseTrap(TrapCause::ILLEGAL_INSTRUCTION, inst.pc);
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