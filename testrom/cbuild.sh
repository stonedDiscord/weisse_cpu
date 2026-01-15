docker run  -v .:/src/ z88dk/z88dk sh ./build.sh

rm testromc.zip
7z a testromc.zip a.rom

cp a.rom merkur_full_house_ic1.ice6
cp a.rom merkur_full_house_ic2.icd6
rm fullhous.zip
7z a fullhous.zip merkur_full_house_ic1.ice6 merkur_full_house_ic2.icd6

cp testromc.zip /run/media/stoned/schrott/Roms/mame/roms/
cp fullhous.zip /run/media/stoned/schrott/Roms/mame/roms/