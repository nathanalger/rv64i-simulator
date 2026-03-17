#pragma once

#include "Interpreter.h"
#include "Processor.h"

void exec_add(const DecodedInstruction &, Processor &);
void exec_sub(const DecodedInstruction &, Processor &);
void exec_addi(const DecodedInstruction &, Processor &);
void exec_lw(const DecodedInstruction &, Processor &);
void exec_sw(const DecodedInstruction &, Processor &);
void exec_beq(const DecodedInstruction &, Processor &);
void exec_jal(const DecodedInstruction &, Processor &);
