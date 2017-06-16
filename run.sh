#!/bin/bash

qemu -S -s \
	-fda floppy.img \
	-boot a -m 64 &
sleep 1
cgdb -x gdbinit
