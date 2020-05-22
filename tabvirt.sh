#!/bin/bash
#export LD_LIBRARY_PATH=/home/matheus/Projetos/libs/Qt/5.11.1/gcc_64/lib
export LD_LIBRARY_PATH=/home/matheus/Projetos/libs/Qt/5.11.1/gcc_64/lib:/home/matheus/Projetos/libs/boost_1_67_0/stage/lib/

if [ "$1" == "debug" ]; then
#ddd -tui ./tabvirt
lldb ./tabvirt
else
./tabvirt --logtostderr $@
fi
