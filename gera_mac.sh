#!/bin/bash

VERSAO=
while [[ $# > 0 ]]; do
  key="$1"
  echo ${key}
  case ${key} in
    -v|--version)
      VERSAO=${2}
      shift
      ;;
    *)
    ;;
  esac
  shift
done
if test -z "${VERSAO}"; then
  echo "Preciso da versao (-v)"
  exit 1
fi

make apple && \
cp tabvirt mac && \
cp -f dados/*asciiproto mac/dados && \
cp -f shaders/*.c mac/shaders && \
for i in libprotobuf.9.dylib libboost_system.dylib libboost_timer.dylib libboost_filesystem.dylib libgflags.2.dylib libglog.0.dylib QtOpenGL.framework/Versions/4/QtOpenGL QtGui.framework/Versions/4/QtGui QtCore.framework/Versions/4/QtCore; do install_name_tool -change /usr/local/lib/${i} @executable_path/lib/${i} mac/tabvirt; done && \
pkgbuild --identifier com.matferib.TabuleiroVirtual --version ${VERSAO} --install-location=/Applications/TabuleiroVirtual --root ./mac TabuleiroVirtual-${VERSAO}.pkg
