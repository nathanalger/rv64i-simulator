#include "TrapCause.h"

const char *trapCauseToString(TrapCause cause)
{
   switch (cause)
   {
   case TrapCause::NONE:
      return "No trap";

   case TrapCause::ILLEGAL_INSTRUCTION:
      return "Illegal instruction";

   case TrapCause::MISALIGNED_LOAD:
      return "Misaligned load";

   case TrapCause::MISALIGNED_STORE:
      return "Misaligned store";

   case TrapCause::LOAD_ACCESS_FAULT:
      return "Load access fault";

   case TrapCause::STORE_ACCESS_FAULT:
      return "Store access fault";

   case TrapCause::INSTRUCTION_ACCESS_FAULT:
      return "Instruction access fault";

   case TrapCause::INSTRUCTION_MISALIGNED:
      return "Instruction address misaligned";

   case TrapCause::STACK_SIZE_EXCEEDS_MAX:
      return "Stack size exceeds maximum";

   case TrapCause::ECALL:
      return "Environment call (ecall)";

   case TrapCause::BREAKPOINT:
      return "Breakpoint";

   case TrapCause::TIMER_INTERRUPT:
      return "Timer interrupt";

   case TrapCause::EXTERNAL_INTERRUPT:
      return "External interrupt";

   default:
      return "Unknown trap cause";
   }
}