from registry import get
import instructions 

def parse_line(line, labels, current_address):
    # Clean up commas by turning them into spaces, then split
    # This handles "addi x1, x2, 10" and "addi x1 x2 10" identically
    parts = line.replace(",", " ").split()

    if not parts:
        return None

    op = parts[0].lower() # Case insensitivity is standard for assembly
    try:
        handler = get(op)
    except ValueError:
        raise ValueError(f"Unknown instruction '{op}' at address {current_address}")

    return handler(parts, labels, current_address)