#!/bin/sh
PATH=$PATH:$N64_INST/bin

n64 codes -h
n64 staticdl -h
n64 logo

cp -a backup.inc hardware.inc
n64 neon64
rm hardware.inc

# Append Super Mario Bros 1 for CEN64 Debug Version
#n64tool -l 2M -h header -t "Neon64 1.2b 08/17/05" -o tmp.z64 neon64.bin
#cat tmp.z64 smb1.nes > neon64-cen64.z64
#chksum64 neon64-cen64.z64

# 64drive Version
n64tool -l 1052672B -h header -t "Neon64 1.2b 08/17/05" -o neon64bu.rom neon64.bin
chksum64 neon64bu.rom

# Clean
rm neon64.bin
rm codes.bin
rm staticdl.bin
rm logo.bin
#rm tmp.z64
rm staticdl.h
rm codes.h
