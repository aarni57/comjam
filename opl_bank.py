#!/usr/bin/env python3

num_bytes_per_instrument = 13
num_bytes_per_instrument_out = 11 # Leaving out note and velocity offsets

def to_hex(x):
    return "0x" + format(x, "02x")

with open("opl_bank.tmb", "rb") as infile:
    data = infile.read()

    with open("opl_bank.h", "w") as f:
        f.write("const uint8_t opl_bank[128][")
        f.write(str(num_bytes_per_instrument_out))
        f.write("] = {\n")

        offset = 0
        for instrument_index in range(0, 128):
            f.write("{ ")
            for i in range(0, num_bytes_per_instrument_out):
                f.write(to_hex(data[offset + i]))
                if i < num_bytes_per_instrument_out - 1:
                    f.write(", ")
            f.write(" },\n")
            offset = offset + num_bytes_per_instrument

        f.write("};\n")
