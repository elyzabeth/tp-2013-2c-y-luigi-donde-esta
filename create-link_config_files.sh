#!/bin/bash
# Crea los links simbolicos de los archivos de configuracion dentro del directorio Debug para ejecutar desde consola.
for x in nivel personaje plataforma; do cd ${x}/Debug; ln -s ../${x}.conf ${x}.conf; cd ../..; done
