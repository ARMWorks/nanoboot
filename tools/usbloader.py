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

import os
import struct
import subprocess
import sys
import time
from argparse import ArgumentParser, FileType

import usb.core
import usb.util


CHUNK_SIZE = 16 * 1024

if __name__ == '__main__':
    script_dir = os.path.dirname(os.path.realpath(__file__))

    parser = ArgumentParser(description='Python Samsung DNW tool')
    parser.add_argument('FILE', type=FileType('rb', 0),
            help='file to load and execute')
    parser.add_argument('ADDRESS', nargs='?', type=lambda x: int(x, 0),
            default=0x20000000,
            help='load and execute address, defaults to 0x20000000')
    args = parser.parse_args()

    timeout = 30
    start = time.time()
    while start + timeout > time.time():
        dev = usb.core.find(idVendor=0x04e8, idProduct=0x1234)
        time.sleep(0.1)
        if dev:
            break

    if dev is None:
        raise ValueError('timeout waiting for device')

    if usb.util.get_string(dev, dev.iProduct) != 'Nanoboot DNW':
        dnw_dir = os.path.join(script_dir, '..', 'bl1-dnw')
        popen_args = ['make', '-C', dnw_dir]
        p = subprocess.Popen(popen_args, env=os.environ)
        if p.wait():
            exit(1)

        dev.set_configuration()
        cfg = dev.get_active_configuration()
        intf = cfg[(0, 0)]

        ep = usb.util.find_descriptor(intf, custom_match = lambda e: \
                usb.util.endpoint_direction(e.bEndpointAddress) == \
                usb.util.ENDPOINT_OUT)

        assert ep is not None

        dnw_bin = os.path.join(dnw_dir, 'build', 'debug', 'dnw.bin')
        with open(dnw_bin, 'rb') as f:
            data = f.read()

        data = struct.pack('<II', 0xD0020000, len(data) + 10) + data
        checksum = 0
        for byte in data:
            checksum += byte
        data += struct.pack('<H', checksum & 0xFFFF)

        while data:
            data, chunk = data[CHUNK_SIZE:], data[:CHUNK_SIZE]
            ep.write(chunk)

        time.sleep(1)
        dev = usb.core.find(idVendor=0x04e8, idProduct=0x1234)
        if dev is None:
            raise ValueError('device not found')

    if usb.util.get_string(dev, dev.iProduct) != 'Nanoboot DNW':
        print('Nanoboot unavailable', file=sys.stderr)

    dev.set_configuration()
    cfg = dev.get_active_configuration()
    intf = cfg[(0, 0)]

    ep = usb.util.find_descriptor(intf, custom_match = lambda e: \
            usb.util.endpoint_direction(e.bEndpointAddress) == \
            usb.util.ENDPOINT_OUT)

    assert ep is not None

    f = args.FILE
    data = f.read()
    f.close()

    data = struct.pack('<II', args.ADDRESS, len(data) + 10) + data
    checksum = 0
    for byte in data:
        checksum += byte
    data += struct.pack('<H', checksum & 0xFFFF)

    while data:
        data, chunk = data[CHUNK_SIZE:], data[:CHUNK_SIZE]
        ep.write(chunk)
