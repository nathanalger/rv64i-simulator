#pragma once
#include <stdint.h>
#include <libfdt.h>

namespace RiscVFeatures
{
   constexpr uint64_t RV32 = (1ULL << 30);
   constexpr uint64_t RV64 = (2ULL << 62);

   constexpr uint64_t A = (1ULL << 0);
   constexpr uint64_t C = (1ULL << 2);
   constexpr uint64_t D = (1ULL << 3);
   constexpr uint64_t F = (1ULL << 5);
   constexpr uint64_t I = (1ULL << 8);
   constexpr uint64_t M = (1ULL << 12);

   constexpr uint64_t S = (1ULL << 18);
   constexpr uint64_t U = (1ULL << 20);
}

class DTBBuilder
{
private:
   static void make_node_name(char *out, const char *prefix, uint64_t addr)
   {
      int i = 0;
      while (prefix[i] != '\0')
      {
         out[i] = prefix[i];
         i++;
      }
      out[i++] = '@';

      if (addr == 0)
      {
         out[i++] = '0';
      }
      else
      {
         const char hex_chars[] = "0123456789abcdef";
         char temp[16];
         int j = 0;
         while (addr > 0)
         {
            temp[j++] = hex_chars[addr % 16];
            addr /= 16;
         }
         // Reverse temp into out
         while (j > 0)
         {
            out[i++] = temp[--j];
         }
      }
      out[i] = '\0';
   }

public:
   static void generate_isa_string(uint64_t misa, char *out_buf, int max_len)
   {
      // Need at least enough room for "rv64" and a null terminator
      if (max_len < 6)
         return;

      int i = 0;

      // 1. Determine base architecture
      out_buf[i++] = 'r';
      out_buf[i++] = 'v';
      if (misa & (2ULL << 62)) // Checking for RV64
      {
         out_buf[i++] = '6';
         out_buf[i++] = '4';
      }
      else
      {
         out_buf[i++] = '3';
         out_buf[i++] = '2';
      }

      // 2. Append extensions in strict RISC-V canonical order
      if ((misa & (1ULL << 8)) && i < max_len - 1)
         out_buf[i++] = 'i'; // I
      if ((misa & (1ULL << 12)) && i < max_len - 1)
         out_buf[i++] = 'm'; // M
      if ((misa & (1ULL << 0)) && i < max_len - 1)
         out_buf[i++] = 'a'; // A
      if ((misa & (1ULL << 5)) && i < max_len - 1)
         out_buf[i++] = 'f'; // F
      if ((misa & (1ULL << 3)) && i < max_len - 1)
         out_buf[i++] = 'd'; // D
      if ((misa & (1ULL << 2)) && i < max_len - 1)
         out_buf[i++] = 'c'; // C

      // 3. Null-terminate the string
      out_buf[i] = '\0';
   }
   static int build(void *buf, int buf_size, uint64_t ram_base, uint64_t ram_size,
                    uint64_t uart_base = 0x10000000, uint64_t clint_base = 0x2000000,
                    const char *isa_string = "rv64imac")
   {
      fdt_create(buf, buf_size);
      fdt_finish_reservemap(buf);

      // Root Node "/"
      fdt_begin_node(buf, "");
      fdt_property_cell(buf, "#address-cells", 2);
      fdt_property_cell(buf, "#size-cells", 2);
      fdt_property_string(buf, "compatible", "nathans-simulator,riscv-virt");
      fdt_property_string(buf, "model", "Nathan's RISC-V Simulator");

      // Chosen Node
      fdt_begin_node(buf, "chosen");
      char stdout_path[64];
      make_node_name(stdout_path, "/soc/uart", uart_base); // "/soc/uart@10000000"
      fdt_property_string(buf, "stdout-path", stdout_path);
      fdt_end_node(buf);

      // CPUs Node
      fdt_begin_node(buf, "cpus");
      fdt_property_cell(buf, "#address-cells", 1);
      fdt_property_cell(buf, "#size-cells", 0);
      fdt_property_cell(buf, "timebase-frequency", 10000000);

      fdt_begin_node(buf, "cpu@0");
      fdt_property_string(buf, "device_type", "cpu");
      fdt_property_cell(buf, "reg", 0);
      fdt_property_string(buf, "status", "okay");
      fdt_property_string(buf, "compatible", "riscv");
      fdt_property_string(buf, "riscv,isa", isa_string);
      fdt_property_string(buf, "mmu-type", "riscv,sv39");
      fdt_property_cell(buf, "riscv,pmp", 16);
      fdt_property_cell(buf, "riscv,pmp-granularity", 4);

      // CPU Interrupt Controller (needs a phandle for CLINT to reference)
      fdt_begin_node(buf, "interrupt-controller");
      fdt_property_cell(buf, "#interrupt-cells", 1);
      fdt_property(buf, "interrupt-controller", 0, 0); // empty flag
      fdt_property_string(buf, "compatible", "riscv,cpu-intc");
      uint32_t phandle_intc = 1;
      fdt_property_cell(buf, "phandle", phandle_intc);
      fdt_property_cell(buf, "linux,phandle", phandle_intc);
      fdt_end_node(buf); // end interrupt-controller

      fdt_end_node(buf); // end cpu@0
      fdt_end_node(buf); // end cpus

      // Memory Node
      char mem_name[32];
      make_node_name(mem_name, "memory", ram_base);
      fdt_begin_node(buf, mem_name);
      fdt_property_string(buf, "device_type", "memory");
      uint32_t reg_mem[4] = {
          cpu_to_fdt32(ram_base >> 32), cpu_to_fdt32(ram_base & 0xFFFFFFFF),
          cpu_to_fdt32(ram_size >> 32), cpu_to_fdt32(ram_size & 0xFFFFFFFF)};
      fdt_property(buf, "reg", reg_mem, sizeof(reg_mem));
      fdt_end_node(buf);

      // SOC Node
      fdt_begin_node(buf, "soc");
      fdt_property_cell(buf, "#address-cells", 2);
      fdt_property_cell(buf, "#size-cells", 2);
      fdt_property_string(buf, "compatible", "simple-bus");
      fdt_property(buf, "ranges", 0, 0); // empty property

      // UART Node
      char uart_name[32];
      make_node_name(uart_name, "uart", uart_base);
      fdt_begin_node(buf, uart_name);
      fdt_property_string(buf, "compatible", "ns16550a");
      uint32_t reg_uart[4] = {
          cpu_to_fdt32(uart_base >> 32), cpu_to_fdt32(uart_base & 0xFFFFFFFF),
          cpu_to_fdt32(0), cpu_to_fdt32(0x100)};
      fdt_property(buf, "reg", reg_uart, sizeof(reg_uart));
      fdt_property_cell(buf, "clock-frequency", 3686400);
      fdt_property_cell(buf, "reg-shift", 0);
      fdt_property_cell(buf, "reg-io-width", 1);
      fdt_end_node(buf);

      // CLINT Node
      char clint_name[32];
      make_node_name(clint_name, "clint", clint_base);
      fdt_begin_node(buf, clint_name);
      fdt_property_string(buf, "compatible", "riscv,clint0");
      uint32_t reg_clint[4] = {
          cpu_to_fdt32(clint_base >> 32), cpu_to_fdt32(clint_base & 0xFFFFFFFF),
          cpu_to_fdt32(0), cpu_to_fdt32(0x10000)};
      fdt_property(buf, "reg", reg_clint, sizeof(reg_clint));
      uint32_t ints_ext[4] = {
          cpu_to_fdt32(phandle_intc), cpu_to_fdt32(3), // M-Soft
          cpu_to_fdt32(phandle_intc), cpu_to_fdt32(7)  // M-Timer
      };
      fdt_property(buf, "interrupts-extended", ints_ext, sizeof(ints_ext));
      fdt_end_node(buf);

      fdt_end_node(buf); // end soc
      fdt_end_node(buf); // end root

      fdt_finish(buf);

      fdt_pack(buf);

      return fdt_totalsize(buf);
   }
};