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
mkdir -p macqt5/dados macqt5/shaders macqt5/texturas macqt5/modelos3d && \
cp tabvirt macqt5 && \
cp -f dados/*asciiproto macqt5/dados && \
cp -f shaders/*.c macqt5/shaders && \
cp -f texturas/*.png macqt5/texturas && \
cp -f modelos3d/*.binproto macqt5/modelos3d && \
chmod -R a+r macqt5/ &&  \
for i in libprotobuf.9.dylib libboost_system.dylib libboost_timer.dylib libboost_filesystem.dylib libgflags.2.dylib libglog.0.dylib; do
  set -x
  install_name_tool -change /usr/local/lib/${i} @executable_path/lib/${i} macqt5/tabvirt;
  set +x
done && \
for i in QtOpenGL.framework/Versions/5/QtOpenGL QtGui.framework/Versions/5/QtGui QtCore.framework/Versions/5/QtCore QtWidgets.framework/Versions/5/QtWidgets; do
  set -x
  install_name_tool -change /usr/local/opt/qt/lib/${i} @executable_path/lib/${i} macqt5/tabvirt;
  set +x
done && \
pkgbuild --identifier com.matferib.TabuleiroVirtual --version ${VERSAO} --install-location=/Applications/TabuleiroVirtual --root ./macqt5 TabuleiroVirtual-${VERSAO}.pkg
