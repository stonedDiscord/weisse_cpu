rm *.zip
rm *.ic*

docker run  -v .:/src/ z88dk/z88dk sh ./build.sh -DBOARD4087

7z a testromc.zip a.rom

cp a.rom merkur_full_house_ic1.ice6
truncate -s 16K merkur_full_house_ic1.ice6
cp a.rom merkur_full_house_ic2.icd6
truncate -s 16K merkur_full_house_ic2.icd6
rm fullhous.zip
7z a fullhous.zip merkur_full_house_ic1.ice6 merkur_full_house_ic2.icd6

cp testromc.zip /run/media/stoned/schrott/Roms/mame/roms/
cp fullhous.zip /run/media/stoned/schrott/Roms/mame/roms/

docker run  -v .:/src/ z88dk/z88dk sh ./build.sh -DBOARD4040

split -b 2048 a.rom excellent_part_
mv excellent_part_aa excellent.ice5
mv excellent_part_ab excellent.ice6
mv excellent_part_ac excellent.icd5
mv excellent_part_ad excellent.icd6
mv excellent_part_ae excellent.icc5
rm excellnt.zip
7z a excellnt.zip excellent.ice5 excellent.ice6 excellent.icd5 excellent.icd6 excellent.icc5

cp excellnt.zip /run/media/stoned/schrott/Roms/mame/roms/excellnt.zip

docker run  -v .:/src/ z88dk/z88dk sh ./build.sh -DBOARD4109
cp a.rom merkur_astro_pr1.ice6
truncate -s 32K merkur_astro_pr1.ice6
cp a.rom merkur_astro_pr2.icd6
truncate -s 32K merkur_astro_pr2.icd6
rm mastro.zip
7z a mastro.zip merkur_astro_pr1.ice6 merkur_astro_pr2.icd6

cp mastro.zip /run/media/stoned/schrott/Roms/mame/roms/mastro.zip
