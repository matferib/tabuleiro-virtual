#QTDIR=../../../../libs/Qt/5.11.1/gcc_64/bin
#QTDIR="C:\Qt\5.15.0\msvc2017_64\bin"
QTDIR="c:/Qt5/5.15.0/msvc2019_64/bin"

${QTDIR}/uic entidade.ui -o entidade.h
${QTDIR}/uic opcoes.ui -o opcoes.h
${QTDIR}/uic cenario.ui -o cenario.h
${QTDIR}/uic forma.ui -o forma.h
${QTDIR}/uic listapaginada.ui -o listapaginada.h
${QTDIR}/uic entradastring.ui -o entradastring.h
${QTDIR}/uic dialogo_bonus.ui -o dialogobonus.h
