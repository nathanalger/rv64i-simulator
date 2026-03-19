.section .data
msg: .string "RISC-V is alive!"

.section .text
    # This will turn into AUIPC and ADDI
    la a1, msg         
    
    # Setup syscall 64 (sys_write)
    addi a0, x0, 1     # fd = 1 (stdout)
    addi a2, x0, 16    # length of string
    addi a7, x0, 64    # syscall 64
    ecall

    # Setup syscall 93 (sys_exit)
    addi a0, x0, 0
    addi a7, x0, 93
    ecall