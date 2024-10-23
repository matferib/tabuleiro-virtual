#!/bin/bash
#export LD_LIBRARY_PATH=/home/matheus/Projetos/libs/Qt/5.11.1/gcc_64/lib
export LD_LIBRARY_PATH=/home/matheus/Documentos/Projetos/libs/Qt/5.15.0/gcc_64/lib:/home/matheus/Documentos/Projetos/libs/boost_1_67_0/stage/lib/:/home/matheus/Documentos/Projetos/libs/tbb/build/linux_intel64_gcc_cc9.3.0_libc2.31_kernel5.4.0_release/
#export QT_DEBUG_PLUGINS=1

if [ "$1" == "debug" ]; then
#ddd -tui ./tabvirt
lldb ./tabvirt -- $@
elif [ "$1" == "perf" ]; then
perf stat -d ./tabvirt -- $@
else
./tabvirt $@
fi
