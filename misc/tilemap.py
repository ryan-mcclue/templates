#!/usr/bin/python3
# SPDX-License-Identifier: zlib-acknowledgement

import sys

tile_map = open("run/jungle.map", mode="r")
tile_map_data = tile_map.read()

tile_map_rows = tile_map_data.split("\n")
num_tiles_y = len(tile_map_rows)
num_tiles_x = len(tile_map_rows[0].split(","))

# IMPORTANT(Ryan): If exceeding 1 byte tiles:
# import struct
# TILE_SIZE_BYTES = 4
# tile_map_bin = bytearray(num_tiles_y * num_tiles_x * TILE_SIZE_BYTES)
# struct.pack_into()

b = bytearray()
b.extend(num_tiles_x.to_bytes(1, sys.byteorder))
b.extend(num_tiles_y.to_bytes(1, sys.byteorder))

joined = ",".join(tile_map_rows).split(",")
for tile_map_val in joined:
  int_equiv = int(tile_map_val)
  b.extend(int_equiv.to_bytes(1, sys.byteorder))

tile_map_binary = open("run/jungle.bin", mode="wb")
tile_map_binary.write(b)
