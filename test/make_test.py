import struct

instructions = [
    0x00500093,
    0x00a00113,
    0x002081b3
]

with open("test.bin", "wb") as f:
    for inst in instructions:
        f.write(struct.pack("<I", inst)) 