/** @file ifg/Principal.cpp implementacao da classe principal. */

#include <stdexcept>
#include <stdlib.h>

// QT
#include <QApplication>
#include <QDesktopWidget>
#include <QMenuBar>
#include <QLayout>
#include <QTextCodec>
#include <QTimer>

#include "gltab/gl.h"
#include "ifg/qt/principal.h"
#include "ifg/qt/menuprincipal.h"
#include "ifg/qt/visualizador3d.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"
#include "ent/tabuleiro.h"

const int INTERVALO_NOTIFICACAO_MS = 10;

using namespace ifg::qt;
using namespace std;

/////////////
// OBJETOS //
/////////////

Principal* Principal::Cria(int& argc, char** argv,
                           Texturas* texturas, ntf::CentralNotificacoes* central) {
  auto* q_app = new QApplication(argc, argv);
  QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
  return new Principal(texturas, central, q_app);
}

Principal::Principal(Texturas* texturas,
                     ntf::CentralNotificacoes* central,
                     QApplication* q_app)
    : QWidget(NULL), central_(central), q_app_(q_app), q_timer_(new QTimer(this)),
      tabuleiro_(texturas, central), menu_principal_(new MenuPrincipal(&tabuleiro_, central, this)),
      v3d_(new Visualizador3d(central, &tabuleiro_, this)) {
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
  // Realiza a notificação de todos.
  auto* notificacao = new ntf::Notificacao;
  notificacao->set_tipo(ntf::TN_TEMPORIZADOR);
  central_->AdicionaNotificacao(notificacao);
  central_->Notifica();
}

bool Principal::TrataNotificacao(const ntf::Notificacao& notificacao) {
	switch (notificacao.tipo()) {
		case ntf::TN_SAIR:
			q_app_->quit();
      return true;
		case ntf::TN_INICIAR: {
      auto* notificacao_iniciado = new ntf::Notificacao;
      notificacao_iniciado->set_tipo(ntf::TN_INICIADO);
      central_->AdicionaNotificacao(notificacao_iniciado);
      return true;
    }
		default:
      return false;
	}
}


