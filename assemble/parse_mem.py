from parse_reg import parse_reg 
def parse_mem_operand(operand, current_address):
    if "(" not in operand or not operand.endswith(")"):
        raise ValueError(f"Invalid memory operand '{operand}' at address {current_address}")

    imm_str, reg_str = operand.split("(")
    imm = int(imm_str.strip())
    rs1 = parse_reg(reg_str.replace(")", ""))

    return imm, rs1