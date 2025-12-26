#!/bin/bash

# Check if file exists and get its size
if [ ! -f a.rom ]; then
    echo "Error: a.rom not found"
    exit 1
fi

current_size=$(stat -c%s a.rom)
if [ $current_size -lt 4096 ]; then
    echo "Padding a.rom from $current_size to 4096 bytes"
    dd if=/dev/zero bs=1 count=$((4096 - current_size)) >> a.rom
fi

rm c_8.bin
rm c_64.bin

cat a.rom a.rom > c_8.bin
cat c_8.bin c_8.bin c_8.bin c_8.bin > c_64.bin
rm c_8.bin
cat c_64.bin c_64.bin > c_128.bin
rm c_64.bin

minipro -p "W27C512@DIP28" -w c_128.bin
