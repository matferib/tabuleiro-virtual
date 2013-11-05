/** @file ifg/Principal.cpp implementacao da classe principal. */

#include <stdexcept>
#include <stdlib.h>

// QT
#include <QApplication>
#include <QDesktopWidget>
#include <QMenuBar>
#include <QTimer>
#include <QLayout>

// Glut.
#include <GL/glut.h>

#include "ifg/qt/principal.h"
#include "ifg/qt/menuprincipal.h"
#include "ifg/qt/visualizador3d.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"
#include "ent/tabuleiro.h"

const double TAM_TABULEIRO = 20.0;  // tamanho do lado do tabuleiro em quadrados
const int INTERVALO_NOTIFICACAO_MS = 10;

using namespace ifg::qt;
using namespace std;

/////////////
// OBJETOS //
/////////////

Principal* Principal::Cria(int& argc, char** argv, ntf::CentralNotificacoes* central) {
	glutInit(&argc, argv);
  auto* q_app = new QApplication(argc, argv);
  return new Principal(central, q_app);
}

Principal::Principal(ntf::CentralNotificacoes* central, QApplication* q_app)
    : QWidget(NULL), central_(central), q_app_(q_app), q_timer_(new QTimer(this)),
      menu_principal_(new MenuPrincipal(central, this)), v3d_(new Visualizador3d(central, this)) {
  central->RegistraReceptor(this);
  connect(q_timer_, SIGNAL(timeout()), this, SLOT(Temporizador()));
}

Principal::~Principal() {
  central_->DesregistraReceptor(this);
}

void Principal::Executa() {
	// maximiza janela
	//QDesktopWidget* qdw = QApplication::desktop();
	//setGeometry(qdw->screenGeometry());
	setGeometry(QRect(100, 100, 500, 500));

	// layout grid com o menu, barra de ferramentas e o tabuleiro
	QLayout* ql = new QGridLayout;
	setLayout(ql);

	ql->setMenuBar(menu_principal_);
	ql->addWidget(v3d_);

  q_timer_->start(INTERVALO_NOTIFICACAO_MS);

	// mostra a janela e entra no loop do QT
	show();
	q_app_->exec();
  q_timer_->stop();
}

void Principal::Temporizador() {
  central_->Notifica();
}

bool Principal::TrataNotificacao(const ntf::Notificacao& notificacao) {
	switch (notificacao.tipo()) {
		case ntf::TN_SAIR:
			q_app_->quit();
      return true;
		case ntf::TN_INICIAR: {
			ent::Tabuleiro::Cria(TAM_TABULEIRO, central_);
      auto* notificacao_iniciado = new ntf::Notificacao;
      notificacao_iniciado->set_tipo(ntf::TN_INICIADO);
      central_->AdicionaNotificacao(notificacao_iniciado);
      return true;
    }
		default:
      return false;
	}
}


