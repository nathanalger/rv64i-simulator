#pragma once
#include "Interpreter.h"
#include "Memory.h"
#include "TrapCause.h"
#include "EnvironmentDevice.h"
#include "Bus.h"
#include <cstdint>

enum class PrivilegeMode : uint8_t
{
   User = 0,
   Supervisor = 1,
   Machine = 3
};

/**
 * Stores the information on the processor, specifically register values and the program counter.
 * Additionally contains some helper functions.
 */
class Processor
{
public:
   enum class AccessType
   {
      LOAD,
      STORE,
      FETCH
   };

   // Array 32 count 64-bit width registers
   uint64_t registers[32];

   // Floating point registers
   uint64_t f_registers[32] = {0};

   // 64 bit width integer that stores current instruction address
   uint64_t program_counter;
   uint64_t step_count = 0;
   uint64_t text_end;

   uint64_t load_reservation = 0;
   bool reservation_valid = false;

   Bus &bus;
   uint32_t memory_size;

   bool trap;
   bool trap_pending = false;
   TrapCause trap_cause;
   uint64_t trap_pc;

   Interpreter interpreter;

   // CSR
   uint64_t csrs[4096];
   PrivilegeMode mode = PrivilegeMode::Machine;
   uint64_t mtime = 0;
   uint64_t mtimecmp = 0xFFFFFFFFFFFFFFFF;
   uint32_t msip = 0;

   bool translate(uint64_t vaddr, uint64_t &paddr, AccessType type);

   /**
    * Default constructor
    * Creates processor with default memory size (1 MB)
    */
   Processor(Bus &bus);

   /**
    * Reset the processor to its base state.
    */
   void reset();

   /**
    * Execute the next step in the program execution.
    */
   bool step();

   /**
    * Trigger the processor to run the entire provided program.
    */
   void run();

   /**
    * Throw a trap with reason to the processor and halt execution.
    */
   void raiseTrap(TrapCause cause, uint64_t trap_pc, uint64_t trap_value);

   uint32_t decompress(uint16_t instr);

   void write_reg(uint8_t rd, uint64_t value);

   /**
    * Read a Control & Status Register (CSR) value
    */
   uint64_t readCSR(uint16_t address);

   /**
    * Write a Control & Status Register (CSR) value
    */
   void writeCSR(uint16_t address, uint64_t value);

   void checkInterrupts();

   uint64_t misa = 0x800000000014112D; // ISA and extensions

   bool checkPMP(uint64_t addr, AccessType type);

private:
   /**
    * Initialize a processor instance
    */
   void initialize();

   // CSR List
   uint64_t mstatus = 0;
   uint64_t medeleg = 0;
   uint64_t mideleg = 0;
   uint64_t mie = 0;
   uint64_t mtvec = 0;
   uint64_t mscratch = 0;
   uint64_t mepc = 0;
   uint64_t mcause = 0;
   uint64_t mtval = 0;
   uint64_t mip = 0;
   uint64_t mcounteren = 0;
   uint64_t scounteren = 0;
   uint64_t menvcfg = 0;
   uint64_t senvcfg = 0;
   uint64_t fcsr = 0;
   uint64_t pmpcfg0 = 0;
   uint64_t pmpaddr0 = 0;

   // Supervisor-level CSRs
   uint64_t sie = 0;
   uint64_t stvec = 0;
   uint64_t sscratch = 0;
   uint64_t sepc = 0;
   uint64_t scause = 0;
   uint64_t stval = 0;
   uint64_t sip = 0;
   uint64_t satp = 0;
   uint64_t pmpcfg[16] = {0};
   uint64_t pmpaddr[64] = {0};
};