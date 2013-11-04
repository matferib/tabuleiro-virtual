/** @file ifg/Principal.cpp implementacao da classe principal. */

#include <stdexcept>
#include <stdlib.h>

// QT
#include <QApplication>
#include <QDesktopWidget>
#include <QMenuBar>
#include <QLayout>
#include <GL/glut.h>

#include "ifg/qt/principal.h"
#include "ifg/qt/menuprincipal.h"
#include "ifg/qt/visualizador3d.h"
#include "ntf/notificacao.h"
#include "ent/tabuleiro.h"

const double TAM_TABULEIRO = 20.0;  // tamanho do lado do tabuleiro em quadrados

using namespace ifg::qt;
using namespace std;

///////////////
// ESTATICAS //
///////////////

/** variavel estatica: instancia unica da interface principal. */
Principal* Principal::inst = NULL;

Principal* Principal::CriaInstancia(int& argc, char** argv){
	if (inst == NULL){
		glutInit(&argc, argv);
		inst = new Principal(new QApplication(argc, argv));
	}
	return inst;
}

Principal* Principal::Instancia(){
	if (inst == NULL){
		throw logic_error("instancia invalida");
	}
	return inst;
}

void Principal::DestroiInstancia(){
	delete inst;
	inst = NULL;
}

/////////////
// OBJETOS //
/////////////

Principal::Principal(QApplication* qAp) : QWidget(NULL), qAp(qAp), menuPrincipal_(new MenuPrincipal(this)), v3d_(new Visualizador3d(this)) {}

Principal::~Principal(){}

void Principal::Executa(){

	// maximiza janela
	QDesktopWidget* qdw = QApplication::desktop();
	QRect qr = qdw->screenGeometry();
	setGeometry(qr);

	// layout grid com o menu, barra de ferramentas e o tabuleiro
	QLayout* ql = new QGridLayout;
	setLayout(ql);

	ql->setMenuBar(menuPrincipal_);
	ql->addWidget(v3d_);


	// mostra a janela e entra no loop do QT
	show();
	qAp->exec();
}

void Principal::TrataNotificacao(ntf::Notificacao* nn) {
	switch (nn->tipo()) {
		case ntf::TN_SAIR:
			qAp->quit();
		break;
		case ntf::TN_INICIAR:
			ent::Tabuleiro::cria(TAM_TABULEIRO);
			v3d_->trataNotificacao(*nn);
			menuPrincipal_->trataNotificacao(*nn);
		break;
		default:
			v3d_->trataNotificacao(*nn);
			menuPrincipal_->trataNotificacao(*nn);
			if (ent::Tabuleiro::haInstancia()) {
				ent::Tabuleiro::instancia().trataNotificacao(*nn);
			}
	}
	delete nn;
}


