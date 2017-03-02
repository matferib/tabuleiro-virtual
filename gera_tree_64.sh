#!/bin/bash

OUTPUT=win32/tree_64

mkdir -p "${OUTPUT}"
mkdir -p "${OUTPUT}/texturas"
mkdir -p "${OUTPUT}/dados"
mkdir -p "${OUTPUT}/shaders"
mkdir -p "${OUTPUT}/modelos3d"
mkdir -p "${OUTPUT}/tabuleiros_salvos"

cp -f tabvirt.exe "${OUTPUT}"
cp -f icon.ico "${OUTPUT}"
cp -f win32/lib/*.dll "${OUTPUT}"
cp -f texturas/*.png "${OUTPUT}/texturas" 
cp -f dados/*.asciiproto "${OUTPUT}/dados" 
cp -f shaders/*.c "${OUTPUT}/shaders" 
cp -f modelos3d/*.binproto "${OUTPUT}/modelos3d"
cp -f tabuleiros_salvos/castelo.binproto "${OUTPUT}/tabuleiros_salvos"
cp -f tabuleiros_salvos/features.binproto "${OUTPUT}/tabuleiros_salvos"
cp -f tabuleiros_salvos/deserto.binproto "${OUTPUT}/tabuleiros_salvos"
