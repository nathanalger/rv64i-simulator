.global _start
.text

_start:
    # t0 = UART base address (0x10000000)
    li t0, 0x10000000
    
    # t1 = Address of the string we want to print
    la t1, hello_str

print_loop:
    # Load the next byte (character) from the string into t2
    lbu t2, 0(t1)
    
    # If the character is 0 (null terminator), we are done printing
    beqz t2, halt

wait_ready:
    # Read the LSR (Line Status Register) at offset 5: 0x10000005
    lbu t3, 5(t0)
    
    # Bit 5 (0x20) tells us if the Transmitter Holding Register is empty
    andi t3, t3, 0x20
    
    # If the bit is 0, the UART is busy. Jump back and keep waiting.
    beqz t3, wait_ready

    # The UART is ready! Store the character (t2) into the THR at offset 0
    sb t2, 0(t0)

    # Move to the next character in the string
    addi t1, t1, 1
    
    # Repeat the loop
    j print_loop

halt:
    # Infinite loop to trap the CPU when finished
    j halt

.data
hello_str:
    .asciz "Hello from the UART!\n"
    