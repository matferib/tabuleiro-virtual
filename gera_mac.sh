#!/bin/bash

VERSAO=
BUNDLEDIR=macqt5
while [[ $# > 0 ]]; do
  key="$1"
  echo ${key}
  case ${key} in
    -v|--version)
      VERSAO=${2}
      shift
      ;;
    -d|--dir)
      BUNDLEDIR=${2}
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

echo "Escrevendo em ${BUNDLEDIR}"

make apple && \
mkdir -p ${BUNDLEDIR}/dados ${BUNDLEDIR}/shaders ${BUNDLEDIR}/texturas ${BUNDLEDIR}/modelos3d && \
cp tabvirt ${BUNDLEDIR} && \
cp -f dados/*asciiproto ${BUNDLEDIR}/dados && \
cp -f shaders/*.c ${BUNDLEDIR}/shaders && \
cp -f texturas/*.png ${BUNDLEDIR}/texturas && \
cp -f modelos3d/*.binproto ${BUNDLEDIR}/modelos3d && \
chmod -R a+r ${BUNDLEDIR}/ &&  \
for i in libprotobuf.9.dylib libboost_system.dylib libboost_timer.dylib libboost_filesystem.dylib libgflags.2.dylib libglog.0.dylib; do
  set -x
  install_name_tool -change /usr/local/lib/${i} @executable_path/lib/${i} ${BUNDLEDIR}/tabvirt;
  set +x
done && \
for i in QtOpenGL.framework/Versions/5/QtOpenGL QtGui.framework/Versions/5/QtGui QtCore.framework/Versions/5/QtCore QtWidgets.framework/Versions/5/QtWidgets; do
  set -x
  install_name_tool -change /usr/local/opt/qt/lib/${i} @executable_path/lib/${i} ${BUNDLEDIR}/tabvirt;
  set +x
done && \
pkgbuild --identifier com.matferib.TabuleiroVirtual --version ${VERSAO} --install-location=/Applications/TabuleiroVirtual --root ./${BUNDLEDIR} TabuleiroVirtual-${VERSAO}.pkg
