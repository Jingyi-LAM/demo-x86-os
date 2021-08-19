#!/bin/bash

fs_file="./source/bl1.bin"
bl_file="./source/bl2.bin"
kn_file="./source/kn.bin"
mount_image="./output/floppy.img"
mount_point="/mnt/floppy"

if [ ! -d "$mount_point" ]
then
	mkdir "$mount_point"
fi

cd source
make all
cd ..
dd if="$fs_file" of="$mount_image" bs=512 count=1 conv=notrunc
mount "$mount_image" -o loop "$mount_point"
cp "$bl_file" "$mount_point"
cp "$kn_file" "$mount_point"
umount "$mount_point"
cd source
make clean
