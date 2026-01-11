docker run  -v .:/src/ z88dk/z88dk \
zcc +z80 -clib=8085 -pragma-define:CRT_ORG_BSS=0xc000 -pragma-define:REGISTER_SP=0xc7f0 -pragma-define:CRT_ENABLE_INT55=2 -pragma-define:CRT_ENABLE_INT65=2 -pragma-define:CRT_ENABLE_INT75=2 main.c -create-app

rm testromc.zip
7z a testromc.zip a.rom

cp a.rom merkur_full_house_ic1.ice6
cp a.rom merkur_full_house_ic2.icd6
rm fullhous.zip
7z a fullhous.zip merkur_full_house_ic1.ice6 merkur_full_house_ic2.icd6

cp testromc.zip /run/media/stoned/schrott/Roms/mame/roms/
cp fullhous.zip /run/media/stoned/schrott/Roms/mame/roms/