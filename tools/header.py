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
    parser.add_argument('-p', default=16<<10, type=lambda x: int(x, 0),
            help='padding, 16K default', metavar='PADDING', dest='padding')
    parser.add_argument('INPUT', type=FileType('rb', 0), help='input file')
    parser.add_argument('OUTPUT', type=FileType('wb+', 0), help='output file')
    args = parser.parse_args()

    header = struct.Struct('<IIII')
    fill = b'\xFF'

    inf = args.INPUT
    outf = args.OUTPUT

    inf.seek(0, os.SEEK_END)
    size = inf.tell()
    inf.seek(16, os.SEEK_SET)

    pad = 0
    if size + header.size < args.padding:
        pad = args.padding - size

    checksum = 0
    length = header.size
    chunk = inf.read(1024)
    while chunk:
        for byte in chunk:
            checksum += byte
        length += len(chunk)
        chunk = inf.read(1024)

    checksum += ord(fill) * pad
    length += pad

    outf.write(header.pack(length, 0, checksum & 0xFFFFFFFF, 0))

    inf.seek(16, os.SEEK_SET)
    chunk = inf.read(1024)
    while chunk:
        outf.write(chunk)
        chunk = inf.read(1024)

    outf.write(fill * pad)

    inf.close()
    outf.close()
