from registry import register
from formats import encode_b
from parse_reg import parse_reg

def assemble(parts, labels, current_address):
    if len(parts) != 4:
        raise ValueError(f"Invalid beq format at address {current_address}")

    rs1 = parse_reg(parts[1])
    rs2 = parse_reg(parts[2])
    target = parts[3]

    if target in labels:
        offset = labels[target] - current_address
    else:
        offset = int(target)

    # Note: Range and alignment checks are safely handled in formats.py
    return encode_b(offset, rs2, rs1, 0b000, 0b1100011)

register("beq", assemble)