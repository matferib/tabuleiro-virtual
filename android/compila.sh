#!/bin/bash

# Pode ser XOOM=1, NEXUS7=1 etc ou vazio.
DISP=
PROF=
MODO=debug
DEBUG="1"
INSTALAR="1"

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
      shift
      ;;
    -p|--profile)
      PROF="PROFILER_LIGADO=1"
      NDK_MODULE_PATH=/home/matheus/Projetos/tabuleiro-virtual/android/jni
      shift
      ;;
    *)
    ;;
  esac
  shift
done

echo "Dispositivo: ${DISP:-universal}"
echo "DEBUG: ${DEBUG}"
echo "PROFILER: ${PROF}"
pushd .
cd jni && ${ANDROID_NDK}/ndk-build V=1 ${DISP} ${PROF} DEBUG=${DEBUG} && cd .. && \
${ANTROOT}/bin/ant ${MODO} && \
test "${INSTALAR}" = "1" && echo "instalando" && ${ANDROID_DEV_TOOLKIT}/sdk/platform-tools/adb install -r bin/TabuleiroVirtual-${MODO}.apk
popd
