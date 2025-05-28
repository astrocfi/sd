#!/bin/bash

cd ~
cp sd/sd.ini .
cd sd
rm -rf *
mkdir regress
cd ~
rm sddoc/*
rm staging/*

(The first of these will complain about the subdirectory "regress".)

cd staging
unzip -a ../sd_vnnnn
mv * ../sd
unzip -a ../r_vnnnn
mv * ../sd/regress
unzip -a ../doc_vnnnn
mv * ../sddoc
cd ~/sd
cp ../sd.ini .
make -f makefile.linux64 clean
make -f makefile.linux64 all
cd regress
chmod 775 *.sh
./doit.sh
cd ..
make -f makefile.linux64 tarball
