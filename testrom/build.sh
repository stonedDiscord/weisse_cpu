#!/bin/bash

docker run  -v .:/src/ z88dk/z88dk z88dk-z80asm -m8085 -b gs_main.asm