#!/usr/bin/env python3

from __future__ import print_function

import os
import shutil
import sys
import subprocess
import platform

names = [ "ship", "asteroid", "scrap", "scrap2", "scrap3" ]
need_lines = [ False, False, True, False, False ]
input_path = "/Users/aarni/dev/comjam"
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

def read_u32(bytes, position):
    return bytes[position] + (bytes[position + 1] << 8) + (bytes[position + 2] << 16) + (bytes[position + 3] << 24)

def read_s32(bytes, position):
    i = read_u32(bytes, position)
    if i > (1 << 31):
        i = i - (1 << 32)
    return i

for mesh_index, name in enumerate(names):
    print("name: " + name)

    mesh_path = os.path.join(input_path, name + ".mesh")
    mesh_bytes = read_binary_file(mesh_path)

    num_vertices = read_u16(mesh_bytes, 0)
    num_indices = read_u16(mesh_bytes, 2)
    num_triangles = int(num_indices / 3)

    print("num_vertices " + str(num_vertices))
    print("num_indices " + str(num_indices))

    aabb_size_x = read_u32(mesh_bytes, 4)
    aabb_size_y = read_u32(mesh_bytes, 8)
    aabb_size_z = read_u32(mesh_bytes, 12)
    aabb_center_x = read_s32(mesh_bytes, 16)
    aabb_center_y = read_s32(mesh_bytes, 20)
    aabb_center_z = read_s32(mesh_bytes, 24)

    print("aabb_size_x " + str(aabb_size_x))
    print("aabb_size_y " + str(aabb_size_y))
    print("aabb_size_z " + str(aabb_size_z))

    print("aabb_center_x " + str(aabb_center_x))
    print("aabb_center_y " + str(aabb_center_y))
    print("aabb_center_z " + str(aabb_center_z))

    position = 28

    with open(os.path.join(output_path, name + ".h"), "w") as f:
        f.write("#include \"fxtypes.h\"\n\n")

        f.write("#define " + name + "_num_vertices " + str(num_vertices) + "\n")
        f.write("#define " + name + "_num_indices " + str(num_indices) + "\n\n")

        f.write("static const fx3_t " + name + "_center = { " + str(aabb_center_x) + ", " + str(aabb_center_y) + ", " + str(aabb_center_z) + " };\n")
        f.write("static const fx3_t " + name + "_size = { " + str(aabb_size_x) + ", " + str(aabb_size_y) + ", " + str(aabb_size_z) + " };\n\n")

        #

        indices = []

        for i in range(num_indices):
            index = read_u16(mesh_bytes, position)
            position = position + 2
            indices.append(index)

        face_colors = []

        for i in range(num_triangles):
            color = read_u8(mesh_bytes, position)
            position = position + 1
            face_colors.append(color)

        vertices = []

        for i in range(num_vertices):
            x = read_s8(mesh_bytes, position + 0)
            y = read_s8(mesh_bytes, position + 1)
            z = read_s8(mesh_bytes, position + 2)
            position = position + 3
            vertices.append((x, y, z))

        lines = []

        def append_line(p0, p1, c):
            if p0[0] < 0 or p1[0] < 0:
                return

            x0 = p0[1]
            y0 = p0[2]
            x1 = p1[1]
            y1 = p1[2]

            x0 = x0 * aabb_size_y + aabb_center_y
            y0 = y0 * aabb_size_z + aabb_center_z
            x1 = x1 * aabb_size_y + aabb_center_y
            y1 = y1 * aabb_size_z + aabb_center_z

            x0 = int(round(x0 * 0.55))
            y0 = int(round(y0 * 0.55))
            x1 = int(round(x1 * 0.55))
            y1 = int(round(y1 * 0.55))

            x0 >>= 16
            y0 >>= 16
            x1 >>= 16
            y1 >>= 16

            y0 = -y0
            y1 = -y1

            for line in lines:
                if line[0] == x0 and line[1] == y0 and line[2] == x1 and line[3] == y1:
                    return
                if line[0] == x1 and line[1] == y1 and line[2] == x0 and line[3] == y0:
                    return

            lines.append((x0, y0, x1, y1, c))

        if need_lines[mesh_index]:
            for i in range(num_triangles):
                a = indices[i * 3 + 0]
                b = indices[i * 3 + 1]
                c = indices[i * 3 + 2]
                v0 = vertices[a]
                v1 = vertices[b]
                v2 = vertices[c]
                c = face_colors[i]
                append_line(v0, v1, c)
                append_line(v1, v2, c)
                append_line(v2, v0, c)

        #

        f.write("static const uint16_t " + name + "_indices[] = {\n")

        for i, index in enumerate(indices):
            f.write(str(index))
            if (i + 1) % 16 == 0:
                f.write(",\n")
            else:
                f.write(", ")

        f.write("};\n\n")

        f.write("static const uint8_t " + name + "_face_colors[] = {\n")

        for i, color in enumerate(face_colors):
            f.write(str(color))
            if (i + 1) % 16 == 0:
                f.write(",\n")
            else:
                f.write(", ")

        f.write("};\n\n")

        f.write("static const int8_t " + name + "_vertices[] = {\n")

        for p in vertices:
            f.write(str(p[0]) + ", " + str(p[1]) + ", " + str(p[2]) + ",\n")

        f.write("};\n")

        if need_lines[mesh_index]:
            f.write("#define " + name + "_num_lines " + str(len(lines)) + "\n")

            f.write("static const int8_t " + name + "_lines[] = {\n")

            for line in lines:
                f.write(str(line[0]) + ", " + str(line[1]) + ", " + str(line[2]) + ", " + str(line[3]) + ", " + str(line[4]) + ",\n")

            f.write("};\n")
