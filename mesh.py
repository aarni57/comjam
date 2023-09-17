#!/usr/bin/env python3

from __future__ import print_function

import os
import shutil
import sys
import subprocess
import platform

name = "torus"
input_path = "/Users/aarni/Desktop"
output_path = "/Users/aarni/dev/comjam"

def read_binary_file(path):
    with open(path, 'rb') as f:
        return f.read()

def read_u8(bytes, position):
    return bytes[position]

def read_s8(bytes, position):
    i = read_u8(bytes, position)
    if i > 127:
        i = i - 256
    return i

def read_u16(bytes, position):
    return bytes[position] + (bytes[position + 1] << 8)

def read_s16(bytes, position):
    i = read_u16(bytes, position)
    if i > 32767:
        i = i - 65536
    return i

mesh_path = os.path.join(input_path, name + ".mesh")
mesh_bytes = read_binary_file(mesh_path)

num_vertices = read_u16(mesh_bytes, 0)
num_indices = read_u16(mesh_bytes, 2)

print("num_vertices " + str(num_vertices))
print("num_indices " + str(num_indices))

aabb_size_x = read_u16(mesh_bytes, 4)
aabb_size_y = read_u16(mesh_bytes, 6)
aabb_size_z = read_u16(mesh_bytes, 8)
aabb_center_x = read_u16(mesh_bytes, 10)
aabb_center_y = read_u16(mesh_bytes, 12)
aabb_center_z = read_u16(mesh_bytes, 14)

print("aabb_size_x " + str(aabb_size_x))
print("aabb_size_y " + str(aabb_size_y))
print("aabb_size_z " + str(aabb_size_z))

print("aabb_center_x " + str(aabb_center_x))
print("aabb_center_y " + str(aabb_center_y))
print("aabb_center_z " + str(aabb_center_z))

position = 16

with open(os.path.join(output_path, name + ".h"), "w") as f:
    f.write("#define " + name + "_num_vertices " + str(num_vertices) + "\n")
    f.write("#define " + name + "_num_indices " + str(num_indices) + "\n\n")

    f.write("#define " + name + "_center_x " + str(aabb_center_x) + "\n")
    f.write("#define " + name + "_center_y " + str(aabb_center_y) + "\n")
    f.write("#define " + name + "_center_z " + str(aabb_center_z) + "\n\n")

    f.write("#define " + name + "_size_x " + str(aabb_size_x) + "\n")
    f.write("#define " + name + "_size_y " + str(aabb_size_y) + "\n")
    f.write("#define " + name + "_size_z " + str(aabb_size_z) + "\n\n")

    f.write("const uint8_t " + name + "_indices[] = {\n")

    for i in range(num_indices):
        index = read_u8(mesh_bytes, position)
        position = position + 1
        f.write(str(index))
        if (i + 1) % 16 == 0:
            f.write(",\n")
        else:
            f.write(", ")

    f.write("};\n\n")

    f.write("const int8_t " + name + "_vertices[] = {\n")

    for i in range(num_vertices):
        x = read_s8(mesh_bytes, position + 0)
        y = read_s8(mesh_bytes, position + 1)
        z = read_s8(mesh_bytes, position + 2)
        position = position + 3
        f.write(str(x) + ", " + str(y) + ", " + str(z) + ",\n")

    f.write("};\n")
