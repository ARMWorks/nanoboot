#!/bin/bash

if [ -z $1 ]; then
	echo "Usage: $0 DEVICE"
	exit 0
fi

case $1 in
/dev/sd[b-z] | /dev/loop0)
	DEV_NAME=`basename $1`
	BLOCK_CNT=`cat /sys/block/${DEV_NAME}/size`;;
*)
	echo "error: unsupported device"
	exit 0
esac

if [ ${BLOCK_CNT} -le 0 ]; then
	echo "error: $1 is inaccessible"
	exit 1
fi

if [ ${BLOCK_CNT} -gt 64000000 ]; then
	echo "error: $1 size (${BLOCK_CNT}) is too large"
	exit 1
fi

if [ "sd$2" = "sdsd" -o ${BLOCK_CNT} -lt 4000000 ]; then
	echo "card type: sd"
	RSD_BLKCOUNT=0
else
	echo "card type: sdhc"
	RSD_BLKCOUNT=1024
fi

let BL1_POSITION=${BLOCK_CNT}-${RSD_BLKCOUNT}-16-2
let BL2_POSITION=${BL1_POSITION}-512


# ----------------------------------------------------------
# Create a binary for movinand/mmc boot

# pad to 256k
dd if=/dev/zero bs=1k count=256 2> /dev/null | tr "\000" "\377" > build/nanoboot-256k.bin
dd if=build/nanoboot.bin of=build/nanoboot-256k.bin conv=notrunc &> /dev/null

# umount partitions first
umount /dev/${DEV_NAME}* > /dev/null 2>&1

dd if=build/nanoboot-256k.bin of=/dev/${DEV_NAME} bs=512 seek=${BL2_POSITION} conv=fdatasync &> /dev/null
dd if=build/nanoboot-256k.bin of=/dev/${DEV_NAME} bs=512 seek=${BL1_POSITION} count=16 conv=fdatasync &> /dev/null
rm build/nanoboot-256k.bin

echo "nanoboot fused"
