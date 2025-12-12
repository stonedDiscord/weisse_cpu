# docker run  -v .:/src/ -it z88dk/z88dk z88dk-sccz80 main.c
docker run  -v .:/src/ z88dk/z88dk zcc +z80 -clib=8085 -pragma-define:CRT_ORG_BSS=0xc0000 main.c -create-app
