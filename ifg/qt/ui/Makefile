QTDIR=/opt/homebrew/Cellar/qt/6.7.2_2/share/qt/libexec/

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
