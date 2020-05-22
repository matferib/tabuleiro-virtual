#!/bin/bash
#export LD_LIBRARY_PATH=/home/matheus/Projetos/libs/Qt/5.11.1/gcc_64/lib
export LD_LIBRARY_PATH=/home/matheus/Projetos/libs/Qt/5.11.1/gcc_64/lib:/home/matheus/Projetos/libs/boost_1_67_0/stage/lib/

if [ "$1" == "debug" ]; then
gdb -tui ./teste_ent_util
#lldb ./teste_ent_util
else
./teste_ent_util $@ && \
./teste_ent_acoes $@
fi
