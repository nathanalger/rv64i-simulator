.section .text
.global _start

_start:
    # 1. Set mtvec (Machine Trap Vector) to our custom handler
    la t0, trap_handler
    csrw mtvec, t0

    # 2. Schedule the first timer interrupt (mtimecmp = mtime + 50)
    li t0, 0x0200BFF8      # CLINT_MTIME address
    li t1, 0x02004000      # CLINT_MTIMECMP address
    ld t2, 0(t0)           # Read current mtime
    addi t2, t2, 50        # Add 50 cycles
    sd t2, 0(t1)           # Write to mtimecmp

    # 3. Enable Machine Timer Interrupts (MTIE is bit 7 in mie register)
    li t0, 0x80            # 1000 0000 in binary
    csrs mie, t0

    # 4. Enable Global Machine Interrupts (MIE is bit 3 in mstatus)
    li t0, 0x8             # 1000 in binary
    csrs mstatus, t0

wait_for_interrupt:
    # 5. Spin and wait. We increment t3 just so the PC changes 
    # and doesn't trigger your bare-metal infinite loop halt detector.
    addi t3, t3, 1         
    j wait_for_interrupt   

trap_handler:
    # --- The CPU teleports here when mtime >= mtimecmp ---

    # 1. We could read mcause here to check why we trapped, 
    # but since we only enabled the timer, we know what caused it.
    
    # 2. Schedule the next interrupt (mtimecmp += 50)
    li t1, 0x02004000      # CLINT_MTIMECMP
    ld t2, 0(t1)
    addi t2, t2, 50
    sd t2, 0(t1)
    
    # Increment t4 just so we can observe the handler ran!
    addi t4, t4, 1

    # 3. Return to the wait_for_interrupt loop
    mret
    