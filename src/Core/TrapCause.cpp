#include "TrapCause.h"

const char *trapCauseToString(TrapCause cause)
{
   switch (cause)
   {
   case TrapCause::NONE:
      return "No trap";
   case TrapCause::STACK_SIZE_EXCEEDS_MAX:
      return "Stack size exceeds maximum";

   case TrapCause::INSTRUCTION_MISALIGNED:
      return "Instruction address misaligned";
   case TrapCause::INSTRUCTION_ACCESS_FAULT:
      return "Instruction access fault";
   case TrapCause::ILLEGAL_INSTRUCTION:
      return "Illegal instruction";
   case TrapCause::BREAKPOINT:
      return "Breakpoint";
   case TrapCause::MISALIGNED_LOAD:
      return "Misaligned load";
   case TrapCause::LOAD_ACCESS_FAULT:
      return "Load access fault";
   case TrapCause::MISALIGNED_STORE:
      return "Misaligned store";
   case TrapCause::STORE_ACCESS_FAULT:
      return "Store access fault";
   case TrapCause::ECALL_FROM_U_MODE:
      return "Environment call from User mode";
   case TrapCause::ECALL_FROM_S_MODE:
      return "Environment call from Supervisor mode";
   case TrapCause::ECALL_FROM_M_MODE:
      return "Environment call from Machine mode";
   case TrapCause::INSTRUCTION_PAGE_FAULT:
      return "Instruction page fault";
   case TrapCause::LOAD_PAGE_FAULT:
      return "Load page fault";
   case TrapCause::STORE_PAGE_FAULT:
      return "Store page fault";

   case TrapCause::SUPERVISOR_TIMER_INTERRUPT:
      return "Supervisor timer interrupt";
   case TrapCause::MACHINE_TIMER_INTERRUPT:
      return "Machine timer interrupt";
   case TrapCause::MACHINE_EXTERNAL_INTERRUPT:
      return "Machine external interrupt";

   default:
      return "Unknown trap cause";
   }
}