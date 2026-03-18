from registry import register
from formats import encode_j
from parse_reg import parse_reg

def assemble(parts, labels, current_address):
    if len(parts) != 3:
        raise ValueError(f"Invalid jal format at address {current_address}")

    rd = parse_reg(parts[1])
    target = parts[2]

    # Resolve label or immediate
    if target in labels:
        imm = labels[target] - current_address
    else:
        imm = int(target)

    # Note: Range and alignment checks are safely handled in formats.py
    return encode_j(imm, rd, 0b1101111)

register("jal", assemble)