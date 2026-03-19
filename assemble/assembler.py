import struct
from parser import parse_line
from psuedo_registry import PSEUDO_HANDLERS

def process_line(clean_line, labels, current_pc):
    parts = clean_line.replace(',', ' ').split()
    op = parts[0]

    # 1. Check Pseudoinstructions first
    if op in PSEUDO_HANDLERS:
        return PSEUDO_HANDLERS[op](parts, labels, current_pc)

    # 2. Otherwise, treat as a single real instruction
    # We wrap it in a list so the return type is consistent!
    return [parse_line(clean_line, labels, current_pc)]

def get_instruction_size(clean_line):
    """Returns the size in bytes that this line will occupy in the binary."""
    if not clean_line or clean_line.startswith("."):
        return 0 # Sections handled separately, or directives like .string
    
    parts = clean_line.replace(',', ' ').split()
    op = parts[0]
    
    # la is currently our only 8-byte expansion
    if op == "la":
        return 8
    if op == "li":
        val = int(parts[2], 0)
        return 4 if -2048 <= val <= 2047 else 8
    
    return 4

def clean(line):
    # Remove everything after #, replace tabs with spaces, and strip
    return line.split('#')[0].replace('\t', ' ').strip()

def first_pass(lines):
    labels = {}
    text_size = 0
    data_size = 0
    current_section = ".text"

    for line in lines:
        clean_line = clean(line)
        if not clean_line: continue

        if ".section .data" in clean_line:
            current_section = ".data"
            continue
        elif ".section .text" in clean_line:
            current_section = ".text"
            continue

        if current_section == ".text":
            if ":" in clean_line:
                label_part, _, instruction_part = clean_line.partition(":")
                labels[label_part.strip()] = text_size
                text_size += get_instruction_size(instruction_part.strip())
            else:
                text_size += get_instruction_size(clean_line)
        
        else: # .data
            if ":" in clean_line:
                label_part, _, data_part = clean_line.partition(":")
                labels[label_part.strip()] = "PENDING_DATA_" + str(data_size)
                clean_line = data_part.strip()
            
            if clean_line.startswith(".string"):
                content = clean_line.split('"')[1]
                data_size += len(content) + 1
            elif clean_line.startswith(".word"):
                data_size += 4

    # Fix up data labels: Address = Total Text Size + Offset
    for label, val in labels.items():
        if isinstance(val, str) and val.startswith("PENDING_DATA_"):
            offset = int(val.replace("PENDING_DATA_", ""))
            labels[label] = text_size + offset

    return labels, text_size

def second_pass(lines, labels, text_size):
    text_bin = bytearray()
    data_bin = bytearray()
    current_section = ".text"
    current_text_addr = 0 

    for line_num, line in enumerate(lines, 1):
        clean_line = clean(line)
        if not clean_line: continue

        if ".section .data" in clean_line:
            current_section = ".data"
            continue
        elif ".section .text" in clean_line:
            current_section = ".text"
            continue

        # Strip label for processing
        if ":" in clean_line:
            _, _, clean_line = clean_line.partition(":")
            clean_line = clean_line.strip()
        if not clean_line: continue

        if current_section == ".text":
            # process_line now handles both regular and pseudo instructions!
            instructions = process_line(clean_line, labels, current_text_addr)
            
            for inst_uint in instructions:
                text_bin.extend(struct.pack("<I", inst_uint))
                current_text_addr += 4
        
        else: # .data section
            if clean_line.startswith(".string"):
                content = clean_line.split('"')[1]
                data_bin.extend(content.encode('ascii') + b'\x00')
            elif clean_line.startswith(".word"):
                # Handle hex (0x) or decimal
                val = int(clean_line.split()[1], 0)
                data_bin.extend(struct.pack("<I", val & 0xFFFFFFFF))

    return text_bin + data_bin

def assemble_file(input_file, output_file):
    with open(input_file) as f:
        lines = f.readlines()

    # Pass 1 gives us the symbols and the split-point for data
    labels, text_size = first_pass(lines)
    
    # Pass 2 generates the actual bits
    binary_payload = second_pass(lines, labels, text_size)

    with open(output_file, "wb") as f:
        f.write(binary_payload)

    print(f"Successfully assembled: {len(binary_payload)} bytes")
    print(f"Text section: {text_size} bytes")
    print(f"Data section: {len(binary_payload) - text_size} bytes")


if __name__ == "__main__":
    import sys

    if len(sys.argv) < 3:
        print("Usage: python assembler.py input.s output.bin")
        exit(1)

    assemble_file(sys.argv[1], sys.argv[2])