from registry import register
from formats import encode_s
from parse_reg import parse_reg

def assemble(parts, labels, current_address):
    if len(parts) != 3:
        raise ValueError(f"Invalid sw format at address {current_address}")

    rs2 = parse_reg(parts[1])

    # Safely parse offset(rs1)
    offset_part = parts[2]
    if "(" not in offset_part or not offset_part.endswith(")"):
        raise ValueError(f"Invalid memory operand '{offset_part}' at address {current_address}")

    imm_str, reg_str = offset_part.split("(")
    imm = int(imm_str.strip())
    rs1 = parse_reg(reg_str.replace(")", ""))

    return encode_s(imm, rs2, rs1, 0b010, 0b0100011)

register("sw", assemble)