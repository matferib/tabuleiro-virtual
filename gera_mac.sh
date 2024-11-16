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
MACDEPLOYQT=/opt/homebrew/Cellar/qt/6.7.2_2/bin/macdeployqt6

make apple && \
mkdir -p ${XDIR} ${RESOURCESDIR}/tabuleiros_salvos ${RESOURCESDIR}/dados ${RESOURCESDIR}/shaders ${RESOURCESDIR}/texturas ${RESOURCESDIR}/modelos3d ${RESOURCESDIR}/sons && \
chmod +w bazel-bin/tabvirt &&
cp -f bazel-bin/tabvirt ${XDIR} && \
cp -f icon.ico ${RESOURCESDIR}/iconfile.icns && \
cp -f dados/*asciiproto ${RESOURCESDIR}/dados && \
cp -f shaders/*.c ${RESOURCESDIR}/shaders && \
cp -f texturas/*.png ${RESOURCESDIR}/texturas && \
cp -f modelos3d/*.binproto ${RESOURCESDIR}/modelos3d && \
cp -f sons/*.wav ${RESOURCESDIR}/sons && \
cp -f tabuleiros_salvos/*.binproto ${RESOURCESDIR}/tabuleiros_salvos && \
chmod -R a+r ${BUNDLEDIR}/ &&  \
${MACDEPLOYQT} macqt5bundle -always-overwrite && \
codesign -s - --deep macqt5bundle/Contents/MacOS/tabvirt && \
pkgbuild --identifier com.matferib.TabuleiroVirtual --version ${VERSAO} --install-location=/Applications/TabuleiroVirtual.app --root ./${BUNDLEDIR} TabuleiroVirtual-${VERSAO}.pkg
