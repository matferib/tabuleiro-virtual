#!/bin/bash

VERSAO=""
while [[ $# > 0 ]]; do
  key="$1"
  case ${key} in
    -v|--versao)
      VERSAO="${2}"
      shift
      ;;
    *)
    ;;
  esac
  shift
done

test -z "$VERSAO" && echo "Preciso da versao" && exit 0 || echo "Assinando versao: ${VERSAO}"

cat AndroidManifest.xml | grep 'android:debuggable="true"' && echo "Android manifest com debuggable." || echo "Android Manifest ok"

bash compila.sh --release -v "${VERSAO}"

rm -f bin/TabuleiroVirtual-release-${VERSAO}.apk && \
jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore tabuleiro-virtual.keystore bin/TabuleiroVirtual-release-unsigned.apk tabvirt && \
${ANDROID_DEV_TOOLKIT}/android_sdk/build-tools/23.0.3/zipalign -v 4 bin/TabuleiroVirtual-release-unsigned.apk bin/TabuleiroVirtual-release-${VERSAO}.apk &&
echo "instalando" && ${ANDROID_DEV_TOOLKIT}/android_sdk/platform-tools/adb install -r bin/TabuleiroVirtual-release-${VERSAO}.apk || \
echo "Erro assinando"
