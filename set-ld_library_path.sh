#!/bin/bash
# Para que tenga efecto ejecutar asi: . ./set-ld_library_path.sh
dir=../..
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${dir}/so-commons-library/Debug/:${dir}/nivel-gui/Debug/:.

