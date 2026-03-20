#include "Processor.h"
#include "Memory.h"
#include "Debug.h"
#include "IODevice.h"
#include "Cosntants.h"

const uint64_t STACK_GUARD = 8;

Processor::Processor(Memory &mem) : memory(mem)
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

   // Enforce text bounds
   if (program_counter >= text_end)
   {
      io->writeString("ERROR: PC out of bounds\n");
      raiseTrap(TrapCause::INSTRUCTION_ACCESS_FAULT, program_counter);
   }

   // Fetch instruction
   uint32_t instruction = memory.readWord(program_counter);

   // Enforce stack size
   if (registers[2] <= text_end + STACK_GUARD)
   {
      io->writeString("ERROR: Stack size exceeds lower bound!\n");
      raiseTrap(TrapCause::STACK_SIZE_EXCEEDS_MAX, program_counter);
   }

   // Send instruction to interpreter
   interpreter.handle(instruction, *this);

   // Enforce x0 = 0
   registers[0] = 0;

   // Enforce stack size (again, i know)
   if (registers[2] <= text_end + STACK_GUARD)
   {
      io->writeString("ERROR: Stack size exceeds lower bound!\n");
      raiseTrap(TrapCause::STACK_SIZE_EXCEEDS_MAX, program_counter);
   }

   // Detect bare-metal halt (infinite loop to itself)
   if (program_counter == old_pc)
   {
      raiseTrap(TrapCause::NONE, program_counter);
   }

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
   }
   else
   {
      io->writeString("Program exited with code 0.\n");
   }
}

void Processor::raiseTrap(TrapCause cause, uint64_t faulting_pc)
{
   writeCSR(0x341, faulting_pc);
   writeCSR(0x342, static_cast<uint64_t>(cause));

   uint64_t mstatus = readCSR(0x300);
   mstatus &= ~(3UL << 11);
   mstatus |= (static_cast<uint64_t>(mode) << 11);
   writeCSR(0x300, mstatus);

   mode = PrivilegeMode::Machine;

   uint64_t vector_base = readCSR(0x305);
   program_counter = vector_base & ~3UL;

   DEBUG_BEGIN()
   io->writeString("TRAP redirected to Handler at: ");
   io->writeInt(program_counter);
   DEBUG_END()

   // Note: We do NOT set trap = true here anymore.
   // Only set trap to true for fatal errors that the guest OS can't handle.
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
      return 0; // Or raise an Illegal Instruction trap
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
   // Note: misa is usually read-only in simple sims
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
   }
}