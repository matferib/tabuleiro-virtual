/** @file ifg/Principal.cpp implementacao da classe principal. */

#include <stdexcept>
#include <stdlib.h>

// QT
#include <boost/timer/timer.hpp>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QLayout>
#include <QLibraryInfo>
#include <QLocale>
#include <QTextCodec>
#include <QTimer>
#include <QTranslator>

#include "gltab/gl.h"
#include "ifg/qt/principal.h"
#include "ifg/qt/menuprincipal.h"
#include "ifg/qt/visualizador3d.h"
#include "ent/constantes.h"
#include "log/log.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"
#include "ent/tabuleiro.h"

using namespace ifg::qt;
using namespace std;

/////////////
// OBJETOS //
/////////////

Principal* Principal::Cria(
    QCoreApplication* q_app,
    const ent::Tabelas& tabelas,
    ent::Tabuleiro* tabuleiro, m3d::Modelos3d* m3d, tex::Texturas* texturas, ifg::TratadorTecladoMouse* teclado_mouse,
    ntf::CentralNotificacoes* central) {
  static QTranslator* tradutor_qt = new QTranslator();
  bool carregou = tradutor_qt->load("qt_" + QLocale::system().name(),
                                    QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  LOG(INFO) << "Traducao QT carregada? " << carregou;
  q_app->installTranslator(tradutor_qt);
  static QTranslator* tradutor_meu = new QTranslator;
  carregou = tradutor_meu->load("tabuleiro_" + QLocale::system().name(), "traducoes", "_", ".qm");
  LOG(INFO) << "Traducao minha carregada? " << carregou;
  LOG(INFO) << "Arquivo: tabuleiro." << QLocale::system().name().toUtf8().constData();
  q_app->installTranslator(tradutor_meu);

#if !USAR_QT5
  QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif
  return new Principal(tabelas, tabuleiro, m3d, texturas, teclado_mouse, central, q_app);
}

Principal::Principal(const ent::Tabelas& tabelas,
                     ent::Tabuleiro* tabuleiro,
                     m3d::Modelos3d* m3d,
                     tex::Texturas* texturas,
                     ifg::TratadorTecladoMouse* teclado_mouse,
                     ntf::CentralNotificacoes* central,
                     QCoreApplication* q_app)
    : QWidget(NULL), central_(central), q_app_(q_app), q_timer_(new QTimer(this)),
      tabuleiro_(tabuleiro),
      v3d_(new Visualizador3d(tabelas, m3d, texturas, teclado_mouse, central, tabuleiro, this)),
      menu_principal_(new MenuPrincipal(tabelas, tabuleiro, v3d_, central, this)) {
  central->RegistraReceptor(this);
  connect(q_timer_, SIGNAL(timeout()), this, SLOT(Temporizador()));
}

Principal::~Principal() {
  central_->DesregistraReceptor(this);
  delete v3d_;
  v3d_ = nullptr;
}

void Principal::Executa() {
  // maximiza janela
  //QDesktopWidget* qdw = QApplication::desktop();
  //setGeometry(qdw->screenGeometry());
  setGeometry(QRect(100, 100, 1024, 768));

  // layout grid com o menu, barra de ferramentas e o tabuleiro
  QLayout* ql = new QGridLayout;
  setLayout(ql);

  ql->setMenuBar(menu_principal_);
  ql->addWidget(v3d_);

  LOG(INFO) << "Resolucao timer: " << INTERVALO_NOTIFICACAO_MS;
  LOG(INFO) << "FPS: " << ATUALIZACOES_POR_SEGUNDO;
  q_timer_->start(INTERVALO_NOTIFICACAO_MS);

  // mostra a janela e entra no loop do QT
  show();
  q_app_->exec();
  q_timer_->stop();
}

void Principal::closeEvent(QCloseEvent *event) {
  q_timer_->stop();
  if (tabuleiro_->EmModoMestre()) {
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_SERIALIZAR_TABULEIRO);
    n.set_endereco("ultimo_tabuleiro_automatico.binproto");
    tabuleiro_->TrataNotificacao(n);
  } else if (tabuleiro_->HaEntidadesSelecionaveis()) {
    auto reply = QMessageBox::question(this, tr("Finalizar"), tr("Salvar Personagens?"),
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
      ntf::Notificacao n;
      n.set_tipo(ntf::TN_SERIALIZAR_ENTIDADES_SELECIONAVEIS);
      n.set_endereco("personagens_automatico.binproto");
      tabuleiro_->TrataNotificacao(n);
    }
  }
  event->accept();
}

void Principal::Temporizador() {
  //static boost::timer::cpu_timer timer;

  //auto passou_ms = timer.elapsed().wall / 1000000ULL;
  //LOG(ERROR) << "tempo_ms: " << (int)passou_ms;
  //timer.start();

  // Realiza a notificação de todos.
  auto* notificacao = new ntf::Notificacao;
  notificacao->set_tipo(ntf::TN_TEMPORIZADOR);
  central_->AdicionaNotificacao(notificacao);
  v3d_->makeCurrent();
  central_->Notifica();
  v3d_->doneCurrent();
}

bool Principal::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_SAIR:
      close();
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
