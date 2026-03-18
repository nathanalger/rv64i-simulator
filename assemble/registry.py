INSTRUCTION_SET = {}

def register(name, func):
    INSTRUCTION_SET[name] = func

def get(name):
    if name not in INSTRUCTION_SET:
        raise ValueError(f"Unknown instruction: {name}")
    return INSTRUCTION_SET[name]