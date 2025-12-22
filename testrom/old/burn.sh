#!/bin/bash

# Check if file exists and get its size
if [ ! -f gs_main.bin ]; then
    echo "Error: gs_main.bin not found"
    exit 1
fi

current_size=$(stat -c%s gs_main.bin)
if [ $current_size -lt 512 ]; then
    echo "Padding gs_main.bin from $current_size to 512 bytes"
    dd if=/dev/zero bs=1 count=$((512 - current_size)) >> gs_main.bin
fi

cat gs_main.bin gs_main.bin gs_main.bin gs_main.bin > gs_8.bin
cat gs_8.bin gs_8.bin gs_8.bin gs_8.bin gs_8.bin gs_8.bin gs_8.bin gs_8.bin > gs_64.bin
rm gs_8.bin
cat gs_64.bin gs_64.bin gs_64.bin gs_64.bin > gs_512.bin
rm gs_64.bin

minipro -p "W27C512@DIP28" -w gs_512.bin
