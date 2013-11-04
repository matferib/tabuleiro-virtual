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

namespace {
/** variavel estatica: instancia unica da interface principal. */
Principal* g_inst = NULL;
}  // namespace

///////////////
// ESTATICAS //
///////////////

Principal* Principal::CriaInstancia(int& argc, char** argv){
	if (g_inst == NULL){
		glutInit(&argc, argv);
		g_inst = new Principal(new QApplication(argc, argv));
	}
	return g_inst;
}

Principal* Principal::Instancia(){
	if (g_inst == NULL){
		throw logic_error("instancia invalida");
	}
	return g_inst;
}

void Principal::DestroiInstancia(){
	delete g_inst;
	g_inst = NULL;
}

/////////////
// OBJETOS //
/////////////

Principal::Principal(QApplication* qAp) : QWidget(NULL), qAp_(qAp), menuPrincipal_(new MenuPrincipal(this)), v3d_(new Visualizador3d(this)) {}

Principal::~Principal(){}

void Principal::Executa(){

	// maximiza janela
	QDesktopWidget* qdw = QApplication::desktop();
	setGeometry(qdw->screenGeometry());

	// layout grid com o menu, barra de ferramentas e o tabuleiro
	QLayout* ql = new QGridLayout;
	setLayout(ql);

	ql->setMenuBar(menuPrincipal_);
	ql->addWidget(v3d_);


	// mostra a janela e entra no loop do QT
	show();
	qAp_->exec();
}

void Principal::TrataNotificacao(ntf::Notificacao* nn) {
	switch (nn->Tipo()) {
		case ntf::TN_SAIR:
			qAp_->quit();
		break;
		case ntf::TN_INICIAR:
			ent::Tabuleiro::Cria(TAM_TABULEIRO);
			v3d_->TrataNotificacao(*nn);
			menuPrincipal_->TrataNotificacao(*nn);
		break;
		default:
			v3d_->TrataNotificacao(*nn);
			menuPrincipal_->TrataNotificacao(*nn);
			if (ent::Tabuleiro::HaInstancia()) {
				ent::Tabuleiro::Instancia().TrataNotificacao(*nn);
			}
	}
	delete nn;
}


