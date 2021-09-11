#!/bin/bash

# Copyright (c) Jeff Kent <jeff@jkent.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# Automatically re-run script under sudo if not root

if [[ `id -u` -ne 0 ]]; then
  (cd bl1-dnw; make)
fi
dd if=/dev/zero bs=512 count=32 2> /dev/null | tr "\000" "\377" > nanoboot.bin

if [[ $(id -u) -ne 0 ]]; then
  echo "Rerunning script under sudo..."
  sudo "$0" "$@"
  exit
fi

if [[ -z $1 ]]; then
	echo "Usage: $0 DEVICE"
	exit 0
fi

case $1 in
/dev/sd[a-z] | /dev/loop0)
	if [[ ! -e $1 ]]; then
		echo "Error: $1 does not exist."
		exit 1
	fi
	DEV_NAME=`basename $1`
	BLOCK_CNT=`cat /sys/block/${DEV_NAME}/size`
	;;
*)
	echo "error: unsupported device"
	exit 0
esac

if [[ -z ${BLOCK_CNT} || ${BLOCK_CNT} -le 0 ]]; then
	echo "error: $1 is inaccessible"
	exit 1
fi

if [[ ${BLOCK_CNT} -gt 134217727 ]]; then
	echo "error: $1 size (${BLOCK_CNT}) is too large"
	exit 1
fi

if [[ "sd$2" = "sdsd" || ${BLOCK_CNT} -lt 4194303 ]]; then
	echo "card type: sd"
	BL1_OFFSET=0
else
	echo "card type: sdhc"
	BL1_OFFSET=1024
fi

if [ "${NDEBUG}" == "1" ]; then
	PROFILE=release
else
	PROFILE=debug
fi

let NANOBOOT_POSITION=1

dd if=bl1-dnw/build/${PROFILE}/bl1.bin of=nanoboot.bin conv=notrunc 2> /dev/null
dd if=nanoboot.bin of=/dev/${DEV_NAME} bs=512 seek=1 conv=fdatasync &> /dev/null

echo "nanoboot fused"
