import struct
from parser import parse_line

def clean(line):
    # Remove everything after #, replace tabs with spaces, and strip
    return line.split('#')[0].replace('\t', ' ').strip()

def first_pass(lines):
    labels = {}
    address = 0
    for line in lines:
        # Remove comments and whitespace
        clean_line = line.split('#')[0].strip()
        if not clean_line:
            continue

        # Check for label anywhere in the line
        if ":" in clean_line:
            label_part, _, instruction_part = clean_line.partition(":")
            labels[label_part.strip()] = address
            # If there is an instruction after the colon, it takes up 4 bytes
            if instruction_part.strip():
                address += 4
        else:
            address += 4
    return labels

def second_pass(lines, labels):
    instructions = []
    address = 0

    # enumerate(lines, 1) gives us the line number starting at 1
    for line_num, line in enumerate(lines, 1):
        clean_line = clean(line)
        if not clean_line:
            continue

        if ":" in clean_line:
            _, _, instruction_part = clean_line.partition(":")
            clean_line = instruction_part.strip()
            
        if not clean_line:
            continue

        try:
            inst = parse_line(clean_line, labels, address)
            if inst is not None:
                instructions.append(inst)
                address += 4
        except Exception as e:
            # Now the user knows exactly where they messed up!
            print(f"Assembly Error on line {line_num}: {line.strip()}")
            print(f" -> {e}")
            exit(1)

    return instructions


def assemble_file(input_file, output_file):
    """
    Full assembly pipeline
    """
    with open(input_file) as f:
        lines = f.readlines()

    # Pass 1: labels
    labels = first_pass(lines)

    # Pass 2: instructions
    instructions = second_pass(lines, labels)

    # Write binary
    with open(output_file, "wb") as f:
        for inst in instructions:
            f.write(struct.pack("<I", inst))  # little-endian

    print(f"Wrote {len(instructions)} instructions to {output_file}")


if __name__ == "__main__":
    import sys

    if len(sys.argv) < 3:
        print("Usage: python assembler.py input.s output.bin")
        exit(1)

    assemble_file(sys.argv[1], sys.argv[2])