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

rm *dll
if [ "${VERSAO}" == "d" ]; then
  echo "Instalando debug..."
  cp debug/*dll .
else
  echo "Instalando release..."
  cp release/*dll .
fi
