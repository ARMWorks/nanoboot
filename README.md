# nanoboot
A bootloader for the FriendlyARM NanoPi (and Mini2451 in SD boot mode)

This bootloader supports booting a kernel with an optional initrd from the FAT
partition of a SD/MicroSD card.  Device tree support is in the planning stages.

To build, set your `CROSS_COMPILE` environment variable to your toolchain's
prefix.  Mine is `arm-buildroot-linux-gnueabi-` but yours may be different.
You'll also want to make sure your toolchain is in your `PATH`.  Then just run
`make`.

Installation is simple, just use the fuse script:

  `sudo ./fuse.sh /dev/sdX`

where sdX is the device your sd card is on.  It should detect if your card is
SD or SDHC automatically.  If it does not, please create an issue so we can
fix it.

## nanoboot.txt

`nanoboot.txt` is an optional text you can create within the root of the FAT
filesystem, which has a simple syntax allowing you to set various boot options:

* `mini2451` - set Mini2451 device type (128 MB memory)
* `nanopi` - (default) set NanoPi device type (64 MB memory)
* `quiet` - don't produce any messages except for errors
* `cmdline = ...` - set the kernel command line
  * default is `console=ttySAC0,115200 root=/dev/mmcblk0p2 rootfstype=ext4
    rootwait`
* `cmdline += ...` - append to the kernel command line
* `kernel = ...` - set the kernel filename
  * default is `zImage`
* `kernel_address = ...` - set the kernel load address
  * default is `0x30008000`
* `ramfsaddr = ...` - set the initrd load address
  * default is `0x30a00000`
* `ramfsfile = ...` - set the initrd file
  * default is blank, meaning no initrd
