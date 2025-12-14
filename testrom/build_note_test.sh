#!/bin/bash

docker run  -v .:/src/ z88dk/z88dk z88dk-z80asm -m8085 -b note_test.asm

# Pad note_test.bin to 256 bytes
current_size=$(stat -c%s note_test.bin 2>/dev/null || echo 0)
if [ $current_size -lt 256 ]; then
    dd if=/dev/zero bs=1 count=$((256 - current_size)) >> note_test.bin 2>/dev/null
fi

rm testrom.zip
rm gs_main.bin
mv note_test.bin gs_main.bin
7z a testrom.zip gs_main.bin
cp testrom.zip /run/media/stoned/schrott/Roms/mame/roms/