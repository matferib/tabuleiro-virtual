#!/bin/bash
export LD_LIBRARY_PATH=/home/matheus/Projetos/libs/Qt/5.11.1/gcc_64/lib

if [ "$1" == "debug" ]; then
gdb -tui ./tabvirt
#lldb ./tabvirt
else
./tabvirt $@
fi
