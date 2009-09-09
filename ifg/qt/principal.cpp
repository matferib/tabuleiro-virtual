/** @file ifg/Principal.cpp implementacao da classe principal. */

#include <stdlib.h>

// QT
#include <QApplication>
#include <QDesktopWidget>
#include <QMenuBar>
#include <QLayout>

#include "ifg/qt/principal.h"
#include "ifg/qt/menuprincipal.h"
#include "ifg/qt/visualizador3d.h"
#include "ntf/notificacao.h"

using namespace ifg::qt;

///////////////
// ESTATICAS //
///////////////

/** variavel estatica: instancia unica da interface principal. */
Principal* Principal::inst = NULL;

Principal& Principal::instancia(int argc, char** argv){
	if (inst == NULL){
		inst = new Principal(new QApplication(argc, argv));
	}
	return *inst;
}

void Principal::destroiInstancia(){
	delete inst;
	inst = NULL;
}

/////////////
// OBJETOS //
/////////////

Principal::Principal(QApplication* qAp) : QWidget(NULL), qAp(qAp){
}

Principal::~Principal(){
}

void Principal::executa(){

	// maximiza janela
	QDesktopWidget* qdw = QApplication::desktop();
	QRect qr = qdw->screenGeometry();
	setGeometry(qr);

	// layout grid com o menu, barra de ferramentas e o tabuleiro
	QLayout* ql = new QGridLayout;
	setLayout(ql);

	ql->setMenuBar(new MenuPrincipal(this));
	ql->addWidget(new Visualizador3d(this));

	// mostra a janela e entra no loop do QT
	show();
	//QObject::connect(&janelaPrincipal, SIGNAL(clicked()), &app, SLOT(quit()));
	qAp->exec();
}

void Principal::trataNotificacao(ntf::Notificacao* nn){
	if (nn->tipo() == ntf::TN_SAIR) {
		qAp->quit();
	}
	delete nn;
}


