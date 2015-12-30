#!/bin/bash

# Pode ser XOOM=1, NEXUS7=1 etc ou vazio.
DISP=
MODO=debug

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
      shift
      ;;
    *)
    ;;
  esac
  shift
done

echo "Dispositivo: ${DISP:-universal}"
pushd .
cd jni && ${ANDROID_NDK}/ndk-build V=1 ${DISP} && cd .. && \
${ANTROOT}/bin/ant ${MODO} && \
${ANDROID_DEV_TOOLKIT}/sdk/platform-tools/adb install -r bin/TabuleiroVirtual-${MODO}.apk
popd
