docker run  -v .:/src/ z88dk/z88dk sh ./build.sh

rm testromc.zip
7z a testromc.zip a.rom

cp a.rom merkur_full_house_ic1.ice6
cp a.rom merkur_full_house_ic2.icd6
rm fullhous.zip
7z a fullhous.zip merkur_full_house_ic1.ice6 merkur_full_house_ic2.icd6

cp testromc.zip /run/media/stoned/schrott/Roms/mame/roms/
cp fullhous.zip /run/media/stoned/schrott/Roms/mame/roms/

docker run  -v .:/src/ z88dk/z88dk sh ./build.sh

split -b 2048 a.rom excellent_part_
mv excellent_part_aa excellent.ice5
mv excellent_part_ab excellent.ice6
mv excellent_part_ac excellent.icd5
mv excellent_part_ad excellent.icd6
mv excellent_part_ae excellent.icc5
rm excellnt.zip
7z a excellnt.zip excellent.ice5 excellent.ice6 excellent.icd5 excellent.icd6 excellent.icc5


cp excellnt.zip /run/media/stoned/schrott/Roms/mame/roms/excellnt.zip