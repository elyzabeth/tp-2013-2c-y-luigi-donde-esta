#!/bin/bash
dir=`pwd`

for x in so-commons-library plataforma personaje nivel-gui nivel 
do
	cd ${x}/Debug/
	make clean
	make all
	cd ${dir}
done 

