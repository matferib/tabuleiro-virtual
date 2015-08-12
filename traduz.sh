#!/bin/bash

# Gera arquivo para traducoes:
echo "ids: " > dados/strings.cpp
grep id: dados/menumodelos.asciiproto | grep -v texto: | sed -e "s/.*id: '\(.*\)'.*/QT_TRANSLATE_NOOP(\"ifg::qt::MenuPrincipal\", \"\1\");/" >> dados/strings.cpp
echo "textos: " >> dados/strings.cpp
grep texto: dados/menumodelos.asciiproto | sed -e "s/.*texto: '\(.*\)'.*/QT_TRANSLATE_NOOP(\"ifg::qt::MenuPrincipal\", \"\1\");/" >> dados/strings.cpp
lupdate ifg/qt/ dados/ -ts traducoes/tabuleiro_en.ts
