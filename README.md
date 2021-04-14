# nanoboot
A bootloader for the Samsung S5PV210/S5PC110 SoC

TODO: more documentation

This bootloader supports booting a kernel with an optional initrd from the FAT
partition of a SD/MicroSD card. Device tree support is planned.

To build, you should use your Linux distrobution's arm-none-eabi- toolchain.

Installation is simple, just use the fuse script:

  `./fuse.sh /dev/sdX`

where /dev/sdX is the device your sd card is on. SD or SDHC does not matter,
though this script does try to detect it.

There is also the `fuse-uart.sh` script that can be used with `tools/loader.py`
for rapid development. `tools/loader.py` will automatically send `bl2.bin`
when the target board is reset via serial.
