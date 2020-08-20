QTDIR=../../../../libs/Qt/5.15.0/gcc_64/bin
#QTDIR=/usr/local/google/home/mribeiro/Qt5/5.15.0/gcc_64/bin
#QTDIR=/usr/local/Cellar/qt5/5.15.0/bin

all:
	${QTDIR}/uic entidade.ui -o entidade.h
	${QTDIR}/uic opcoes.ui -o opcoes.h
	${QTDIR}/uic cenario.ui -o cenario.h
	${QTDIR}/uic forma.ui -o forma.h
	${QTDIR}/uic listapaginada.ui -o listapaginada.h
	${QTDIR}/uic entradastring.ui -o entradastring.h
	${QTDIR}/uic dialogo_bonus.ui -o dialogobonus.h

clean:
	rm *.h
