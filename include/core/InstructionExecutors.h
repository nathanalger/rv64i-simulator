#pragma once

#include "Interpreter.h"
#include "Processor.h"

// RV64I
void exec_add(const DecodedInstruction &, Processor &);
void exec_sub(const DecodedInstruction &, Processor &);
void exec_addi(const DecodedInstruction &, Processor &);
void exec_lw(const DecodedInstruction &, Processor &);
void exec_sw(const DecodedInstruction &, Processor &);
void exec_beq(const DecodedInstruction &, Processor &);
void exec_jal(const DecodedInstruction &, Processor &);
void exec_unknown(const DecodedInstruction &, Processor &);

// Loads
void exec_lb(const DecodedInstruction &, Processor &);
void exec_lh(const DecodedInstruction &, Processor &);
void exec_ld(const DecodedInstruction &, Processor &);
void exec_lbu(const DecodedInstruction &, Processor &);
void exec_lhu(const DecodedInstruction &, Processor &);
void exec_lwu(const DecodedInstruction &, Processor &);