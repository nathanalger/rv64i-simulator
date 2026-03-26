#pragma once
#include "Memory.h"
#include "IODevice.h"
#include "UART.h"
#include <stddef.h>
#include <stdint.h>

class Processor;
class Bus
{
private:
   Memory &ram;
   IO *io;
   uint64_t ram_base;
   UART *uart;
   Processor *cpu;

public:
   Bus(Memory &ram_ref, IO *io_ptr, uint64_t base_address, UART *uart_ptr = nullptr);

   uint8_t readByte(uint64_t address);
   uint8_t writeByte(uint64_t address, uint8_t value);

   uint16_t readHalf(uint64_t address);
   uint16_t writeHalf(uint64_t address, uint16_t value);

   uint32_t readWord(uint64_t address);
   uint32_t writeWord(uint64_t address, uint32_t value);

   uint64_t readDouble(uint64_t address);
   uint64_t writeDouble(uint64_t address, uint64_t value);

   void setProcessor(Processor *p);

   uint64_t getRamBase() const { return ram_base; }
   uint64_t getRamSize() const { return ram.getSize(); }
};