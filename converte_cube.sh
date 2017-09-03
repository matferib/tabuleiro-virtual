#!/bin/bash

PREFIXO=
NOVO_PREFIXO=
while [[ $# > 0 ]]; do
  key="$1"
  echo ${key}
  case ${key} in
    -p|--prefix)
      PREFIXO=${2}
      shift
      ;;
    -n|--new)
      NOVO_PREFIXO=${2}
      shift
      ;;
    *)
    ;;
  esac
  shift
done
if [ -z "${PREFIXO}" ]; then
  echo "Preciso do prefixo (-p)"
  exit 1
fi
if [ -z "${NOVO_PREFIXO}" ]; then
  echo "Preciso do novo prefixo sem skybox_ (-n)"
  exit 1
fi
NOVO_PREFIXO=skybox_${NOVO_PREFIXO}

echo "Removendo 2048 do nome"
for i in ${PREFIXO}*; do mv "$i" ${i/2048/}; done
echo "Trocando prefixo de ${PREFIXO} para ${NOVO_PREFIXO}"
for i in ${PREFIXO}*; do mv "$i" ${i/$PREFIXO/$NOVO_PREFIXO}; done

echo "Convertendo sufixos"
mv ${NOVO_PREFIXO}Front.png ${NOVO_PREFIXO}frente.png
mv ${NOVO_PREFIXO}Back.png  ${NOVO_PREFIXO}atras.png
mv ${NOVO_PREFIXO}Up.png    ${NOVO_PREFIXO}cima.png
mv ${NOVO_PREFIXO}Down.png  ${NOVO_PREFIXO}baixo.png
mv ${NOVO_PREFIXO}Left.png  ${NOVO_PREFIXO}esquerda.png
mv ${NOVO_PREFIXO}Right.png ${NOVO_PREFIXO}direita.png

echo "Convertendo tamanhos para 512"
for i in ${NOVO_PREFIXO}*; do convert $i -resize 512x512 $i; done
