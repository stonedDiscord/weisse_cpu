# docker run  -v .:/src/ -it z88dk/z88dk z88dk-sccz80 main.c
docker run  -v .:/src/ -it z88dk/z88dk zcc +rc2014 -subtype=uart85 -v --c-code-in-asm --list main.c -o testrom -create-app
