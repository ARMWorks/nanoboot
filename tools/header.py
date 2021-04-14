#!/usr/bin/env python
# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2021 Jeff Kent <jeff@jkent.net>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; version 2.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

import os
import struct
import sys

MIN_IMAGE_SIZE = (16 * 1024)

if __name__ == '__main__':
    path = sys.argv[1]

    with open(path, 'ab') as f:
        size = f.tell()
        if size <= MIN_IMAGE_SIZE:
            f.write(b'\xFF' * (MIN_IMAGE_SIZE - size))
            size = MIN_IMAGE_SIZE

    with open(path, 'rb+') as f:
        f.seek(16, os.SEEK_SET)
        checksum = 0
        byte = f.read(1)
        while byte != b'':
            checksum += ord(byte)
            byte = f.read(1)
        checksum %= 0xFFFFFFFF
        header = struct.pack('<IIII', size, 0, checksum, 0)
        f.seek(0, os.SEEK_SET)
        f.write(header)
