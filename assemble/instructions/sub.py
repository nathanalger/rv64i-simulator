from registry import register
from formats import encode_r
from parse_reg import parse_reg

def assemble(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid sub format at address {current_address}")

    rd = parse_reg(parts[1])
    rs1 = parse_reg(parts[2])
    rs2 = parse_reg(parts[3])

    return encode_r(0b0100000, rs2, rs1, 0b000, rd, 0b0110011)

register("sub", assemble)