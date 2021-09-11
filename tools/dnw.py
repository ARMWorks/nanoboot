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
import struct
import usb.core, usb.util

if __name__ == '__main__':
    parser = ArgumentParser(description='Python Samsung DNW tool')
    parser.add_argument('-v', default=0x04e8,
            help='vendor ID, defaults to 0x04e8', metavar='VID', dest='vendor')
    parser.add_argument('-p', default=0x1234,
            help='product ID, defaults to 0x1234', metavar='PID',
            dest='product')
    parser.add_argument('-c', default=16384,
            help='max chunk size, defaults to 16K', metavar='CHUNKSIZE',
            dest='chunksize')
    parser.add_argument('FILE', type=FileType('rb', 0),
            help='file to load and execute')
    parser.add_argument('ADDRESS', nargs='?', type=lambda x: int(x, 0),
            default=0xD0020000,
            help='load and execute address, defaults to 0xD0020000')
    args = parser.parse_args()

    dev = usb.core.find(idVendor=args.vendor, idProduct=args.product)
    if dev is None:
        raise ValueError('device not found')

    dev.set_configuration()
    cfg = dev.get_active_configuration()
    intf = cfg[(0, 0)]

    ep = usb.util.find_descriptor(intf, custom_match = lambda e: \
            usb.util.endpoint_direction(e.bEndpointAddress) == \
            usb.util.ENDPOINT_OUT)

    assert ep is not None

    data = args.FILE.read()
    data = struct.pack('<II', args.ADDRESS, len(data) + 10) + data
    checksum = 0
    for byte in data:
        checksum += byte
    data += struct.pack('<H', checksum & 0xFFFF)

    while data:
        data, chunk = data[args.chunksize:], data[:args.chunksize]
        ep.write(chunk)
