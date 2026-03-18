from registry import register
from formats import encode_r
from parse_reg import parse_reg

def assemble(parts, labels, current_address):
    # Safety check: 'add rd, rs1, rs2' should have exactly 4 parts
    if len(parts) != 4:
        raise ValueError(f"Invalid add format at address {current_address}. Expected: add rd, rs1, rs2")

    # Use parse_reg instead of string replacement
    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    rs2 = parse_reg(parts[3])

    # funct7=0x00, funct3=0x0, opcode=0x33
    return encode_r(0b0000000, rs2, rs1, 0b000, rd, 0b0110011)

register("add", assemble)