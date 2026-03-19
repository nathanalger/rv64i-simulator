# Recursive Fibonacci using stack + jal/jalr

# Registers:
# x2 = sp (stack pointer)
# x10 = argument n
# x11 = return value
# x1 = ra (return address)

# Setup stack pointer
addi x2, x0, 1024     # stack base (adjust if needed)

# Input: n = 6
addi x10, x0, 6

# Call fib(n)
jal x1, fib

# End (halt)
end:
    jal x0, end


# -------------------------
# fib(n)
# -------------------------
fib:
    # Base case: if n < 2 return n
    addi x5, x0, 2
    blt x10, x5, fib_base

    # Allocate stack frame (16 bytes)
    addi x2, x2, -16

    # Save return address and n
    sd x1, 8(x2)
    sd x10, 0(x2)

    # -------- fib(n-1) --------
    addi x10, x10, -1
    jal x1, fib
    add x6, x11, x0        # save result fib(n-1)

    # Restore n
    ld x10, 0(x2)

    # -------- fib(n-2) --------
    addi x10, x10, -2
    jal x1, fib

    # x11 = fib(n-2)
    add x11, x11, x6       # fib(n-1) + fib(n-2)

    # Restore return address
    ld x1, 8(x2)

    # Free stack
    addi x2, x2, 16

    # Return
    jalr x0, x1, 0


fib_base:
    # return n
    add x11, x10, x0
    jalr x0, x1, 0