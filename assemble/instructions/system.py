from registry import register

def assemble_ecall(parts, labels, current_address):
    return 0x00000073

def assemble_ebreak(parts, labels, current_address):
    return 0x00100073

register("ecall", assemble_ecall)
register("ebreak", assemble_ebreak)