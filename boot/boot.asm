
boot.elf:     file format elf64-littleriscv


Disassembly of section .text:

0000000000000000 <_start>:
   0:	00000297          	auipc	t0,0x0
   4:	03c28293          	addi	t0,t0,60 # 3c <trap_handler>
   8:	30529073          	csrw	mtvec,t0
   c:	0200c2b7          	lui	t0,0x200c
  10:	ff82829b          	addiw	t0,t0,-8 # 200bff8 <__global_pointer$+0x200a7a4>
  14:	02004337          	lui	t1,0x2004
  18:	0002b383          	ld	t2,0(t0)
  1c:	03238393          	addi	t2,t2,50
  20:	00733023          	sd	t2,0(t1) # 2004000 <__global_pointer$+0x20027ac>
  24:	08000293          	li	t0,128
  28:	3042a073          	csrs	mie,t0
  2c:	00800293          	li	t0,8
  30:	3002a073          	csrs	mstatus,t0

0000000000000034 <wait_for_interrupt>:
  34:	001e0e13          	addi	t3,t3,1
  38:	ffdff06f          	j	34 <wait_for_interrupt>

000000000000003c <trap_handler>:
  3c:	02004337          	lui	t1,0x2004
  40:	00033383          	ld	t2,0(t1) # 2004000 <__global_pointer$+0x20027ac>
  44:	03238393          	addi	t2,t2,50
  48:	00733023          	sd	t2,0(t1)
  4c:	001e8e93          	addi	t4,t4,1
  50:	30200073          	mret
