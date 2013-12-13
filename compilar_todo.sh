#!/bin/bash
dir=`pwd`

for x in so-commons-library plataforma personaje nivel-gui nivel fileSystem
do
	cd ${x}/Debug/
	make clean
	make all
	cd ${dir}
done 

