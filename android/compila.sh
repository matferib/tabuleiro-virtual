#!/bin/bash

# Pode ser XOOM=1, NEXUS7=1 etc ou vazio.
DISP=
PROF=
QT=
MODO=debug
DEBUG="1"
INSTALAR="1"
SUFIXO=debug

#echo num args: $#
while [[ $# > 0 ]]; do
  key="$1"
  echo ${key}
  case ${key} in
    -d|--dispositivo)
      DISP="${2}=1"
      shift
      ;;
    -c|--clean)
      ${ANDROID_NDK}/ndk-build V=1 ${DISP} clean
      rm -f bin/Tabuleiro*
      exit 0
      ;;
    -r|--release)
      MODO=release
      DEBUG="0"
      INSTALAR="0"
      ;;
    -p|--profile)
      PROF="PROFILER_LIGADO=1"
      NDK_MODULE_PATH=/home/matheus/Projetos/tabuleiro-virtual/android/jni
      ;;
    -q|--usar_qt)
      QT="USAR_QT=1"
      ;;
    *)
    ;;
  esac
  shift
done

echo "Dispositivo: ${DISP:-universal}"
echo "DEBUG: ${DEBUG}"
echo "PROFILER: ${PROF}"
echo "SUFIXO: ${SUFIXO}"
echo "QT: ${QT}"
pushd .
cd jni && ${ANDROID_NDK}/ndk-build -j 4 V=1 ${DISP} ${PROF} ${QT} DEBUG=${DEBUG} && cd .. && \
${ANTROOT}/bin/ant ${MODO} && \
test "${INSTALAR}" = "1" && echo "instalando" && ${ANDROID_DEV_TOOLKIT}/android_sdk/platform-tools/adb install -r bin/TabuleiroVirtual-debug.apk
popd
