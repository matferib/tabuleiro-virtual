#!/bin/bash

VERSAO=r
while [[ $# > 0 ]]; do
  key="$1"
  echo ${key}
  case ${key} in
    d|debug)
      VERSAO="d"
      ;;
    r|release)
      VERSAO="r"
      ;;
    *)
    ;;
  esac
  shift
done
if test -z "${VERSAO}"; then
  echo "Preciso de (d)ebug ou (r)elease"
  exit 1
fi

rm *dll platforms/qwindows*dll

if [ "${VERSAO}" == "d" ]; then
  echo "Instalando debug..."
  cp -f debug/*dll .
  cp debug/platforms/qwindowsd.dll platforms
else
  echo "Instalando release..."
  cp -f release/*dll .
  cp release/platforms/qwindows.dll platforms
fi
