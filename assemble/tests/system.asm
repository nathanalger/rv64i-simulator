addi x5, x0, 10    # x5 = 10
ebreak             # Pause here! (If debugger attached)
addi x5, x5, 10    # x5 = 20

# Graceful exit (syscall 93, code 0)
addi x17, x0, 93
addi x10, x0, 0
ecall