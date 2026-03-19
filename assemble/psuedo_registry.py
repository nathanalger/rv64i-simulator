from parse_reg import parse_reg
from formats import encode_i, encode_j, encode_u

PSEUDO_HANDLERS = {}

def register_pseudo(name, func):
    PSEUDO_HANDLERS[name] = func

# --- The Handlers ---

def handle_li(parts, labels, pc):
    # li rd, immediate
    rd_str = parts[1].replace(',', '')
    rd = parse_reg(rd_str)
    
    # Support hex (0x) and decimal
    val = int(parts[2], 0)
    
    # Case 1: Fits in a signed 12-bit immediate (-2048 to 2047)
    if -2048 <= val <= 2047:
        # addi rd, x0, val
        return [encode_i(val, 0, 0b000, rd, 0b0010011)]
    
    # Case 2: 32-bit value (needs LUI + ADDI)
    # Extract bottom 12 bits and top 20 bits
    low = val & 0xFFF
    if low >= 0x800:
        low -= 0x1000
    high = (val - low) >> 12
    
    # lui rd, high (Opcode 0x37)
    lui_inst = ((high & 0xFFFFF) << 12) | (rd << 7) | 0x37
    # addi rd, rd, low (Opcode 0x13)
    addi_inst = ((low & 0xFFF) << 20) | (rd << 15) | (0 << 12) | (rd << 7) | 0x13
    
    return [lui_inst, addi_inst]

def handle_mv(parts, labels, pc):
    # mv rd, rs  =>  addi rd, rs, 0
    rd = parse_reg(parts[1])
    rs = parse_reg(parts[2])
    return [encode_i(0, rs, 0b000, rd, 0b0010011)]

def handle_j(parts, labels, pc):
    # j label  =>  jal x0, label
    target = parts[1]
    offset = labels[target] - pc if target in labels else int(target)
    return [encode_j(offset, 0, 0b1101111)]

def handle_nop(parts, labels, pc):
    # nop  =>  addi x0, x0, 0
    return [encode_i(0, 0, 0b000, 0, 0b0010011)]

def handle_ret(parts, labels, pc):
    # ret  =>  jalr x0, x1, 0 (jump to return address)
    return [0x00008067] # Hardcoded JALR x0, x1, 0 for speed

def expand_la(parts, labels, current_address):
    if len(parts) != 3:
        raise ValueError(f"Invalid la format: la rd, label")

    rd_str = parts[1].replace(',', '')
    rd = parse_reg(rd_str)
    target_label = parts[2]

    if target_label not in labels:
        raise ValueError(f"Label '{target_label}' not found")

    target_addr = labels[target_label]
    offset = target_addr - current_address

    # RISC-V splitting logic for AUIPC (20-bit) + ADDI (12-bit)
    # Since ADDI sign-extends, if the 12th bit of the offset is 1, 
    # we have to add 1 to the high part to compensate.
    low = offset & 0xFFF
    if low >= 0x800:
        low -= 0x1000
    high = (offset - low) >> 12

    # 1. AUIPC rd, high (Opcode 0x17)
    auipc_inst = (high << 12) | (rd << 7) | 0x17
    
    # 2. ADDI rd, rd, low (Opcode 0x13, funct3 0)
    addi_inst = (low << 20) | (rd << 15) | (0 << 12) | (rd << 7) | 0x13

    return [auipc_inst, addi_inst]

# Register them all
register_pseudo("li", handle_li)
register_pseudo("mv", handle_mv)
register_pseudo("j", handle_j)
register_pseudo("nop", handle_nop)
register_pseudo("ret", handle_ret)
register_pseudo("la", expand_la) # Move your 'la' logic here!