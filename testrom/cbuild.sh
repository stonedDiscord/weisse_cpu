docker run  -v .:/src/ z88dk/z88dk zcc +z80 -clib=8085 -pragma-define:CRT_ORG_BSS=0xc000 -pragma-define:REGISTER_SP=0xc7f0 main.c -create-app
