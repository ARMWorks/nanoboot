from argparse import ArgumentParser, FileType
import struct
import usb.core, usb.util

def main():
    parser = ArgumentParser(description='Python Samsung DNW tool')
    parser.add_argument('-v', default=0x04e8, help='Vendor ID, defaults to 0x04e8', metavar='VID', dest='vendor')
    parser.add_argument('-p', default=0x1234, help='Product ID, defaults to 0x1234', metavar='PID', dest='product')
    parser.add_argument('-e', default=2, help='Endpoint number, defaults to 2', metavar='ENDPOINT', dest='endpoint')
    parser.add_argument('-c', default=1024, help='Max chunk size, defaults to 16K', metavar='CHUNKSIZE', dest='chunksize')
    parser.add_argument('FILE', type=FileType('rb', 0), help='File to load and execute')
    parser.add_argument('ADDRESS', nargs='?', default=0x20000000, help='Load and execute address, defaults to 0x20000000')

    args = parser.parse_args()
    print(args)

    dev = usb.core.find(idVendor=args.vendor, idProduct=args.product)

    if dev is None:
        raise ValueError('Device not found')

    data = args.FILE.read()
    data = struct.pack('<II', args.ADDRESS, len(data) + 10) + data
    checksum = 0
    for byte in data:
        checksum += byte
    data += struct.pack('<H', checksum & 0xFFFF)

    while data:
        data, chunk = data[args.chunksize:], data[:args.chunksize]
        dev.write(args.endpoint, chunk)

if __name__ == '__main__':
    main()
