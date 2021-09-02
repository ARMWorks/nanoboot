#!/usr/bin/env python
# SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause)
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

from argparse import ArgumentParser, FileType
import os
import struct

if __name__ == '__main__':
    parser = ArgumentParser(description='Samsung bl1 header fill tool')
    parser.add_argument('-p', default=0, type=lambda x: int(x, 0),
            help='padding, none by default', metavar='PADDING', dest='padding')
    parser.add_argument('FILE', type=FileType('rb+', 0), help='file to modify')
    args = parser.parse_args()

    f = args.FILE

    f.seek(0, os.SEEK_END)
    size = f.tell()
    if size < args.padding:
        f.write(b'\xFF' * (args.padding - size))
        size = args.padding

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
    f.close()
