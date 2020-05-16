#!/bin/bash

VERSAO=
BUNDLEDIR=macqt5bundle
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

RESOURCESDIR=${BUNDLEDIR}/Contents/Resources
XDIR=${BUNDLEDIR}/Contents/MacOS
FRAMEWORKSDIR=${BUNDLEDIR}/Contents/Frameworks
MACDEPLOYQT=/usr/local/Cellar/qt5/5.14.0/bin/macdeployqt

make apple && \
mkdir -p ${XDIR} ${RESOURCESDIR}/tabuleiros_salvos ${RESOURCESDIR}/dados ${RESOURCESDIR}/shaders ${RESOURCESDIR}/texturas ${RESOURCESDIR}/modelos3d && ${RESOURCESDIR}/sons \
cp tabvirt ${XDIR} && \
cp -f dados/*asciiproto ${RESOURCESDIR}/dados && \
cp -f shaders/*.c ${RESOURCESDIR}/shaders && \
cp -f texturas/*.png ${RESOURCESDIR}/texturas && \
cp -f modelos3d/*.binproto ${RESOURCESDIR}/modelos3d && \
cp -f sons/*.binproto ${RESOURCESDIR}/sons && \
cp -f tabuleiros_salvos/*.binproto ${RESOURCESDIR}/tabuleiros_salvos && \
chmod -R a+r ${BUNDLEDIR}/ &&  \
for i in libprotobuf.9.dylib libboost_system.dylib libboost_timer.dylib libboost_filesystem.dylib libgflags.2.dylib libglog.0.dylib; do
  set -x
  install_name_tool -change /usr/local/lib/${i} @executable_path/../Frameworks/${i} ${XDIR}/tabvirt;
  set +x
done && \
${MACDEPLOYQT} macqt5bundle && \
pkgbuild --identifier com.matferib.TabuleiroVirtual --version ${VERSAO} --install-location=/Applications/TabuleiroVirtual.app --root ./${BUNDLEDIR} TabuleiroVirtual-${VERSAO}.pkg
