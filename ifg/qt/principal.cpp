/** @file ifg/Principal.cpp implementacao da classe principal. */

#include <absl/strings/str_format.h>
#include <stdexcept>
#include <stdlib.h>

// QT
#include <boost/timer/timer.hpp>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QDockWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QLabel>
#include <QLayout>
#include <QLibraryInfo>
#include <QLocale>
#include <QPushButton>
#include <QSpacerItem>
#include <QTextCodec>
#include <QTextEdit>
#include <QTimer>
#include <QTranslator>

#include "gltab/gl.h"
#include "ifg/qt/principal.h"
#include "ifg/qt/menuprincipal.h"
#include "ifg/qt/util.h"
#include "ifg/qt/visualizador3d.h"
#include "ent/constantes.h"
#include "ent/tabuleiro.h"
#include "goog/stringprintf.h"
#include "log/log.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

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
    : QMainWindow(NULL), central_(central), q_app_(q_app),
      tabuleiro_(tabuleiro),
      v3d_(new Visualizador3d(tabelas, m3d, texturas, teclado_mouse, central, tabuleiro, this)),
      menu_principal_(new MenuPrincipal(tabelas, tabuleiro, v3d_, central, this)) {
  central->RegistraReceptor(this);
}

Principal::~Principal() {
  central_->DesregistraReceptor(this);
  delete v3d_;
  v3d_ = nullptr;
}

void Principal::LogAlternado() {
  tabuleiro_->AlternaMostraLogEventos();
  LOG(INFO)<< "aqui";
}

namespace {
std::string MontaStringLog(const std::list<std::string>& log) {
  std::string str;
  for (const auto& linha : log) {
    absl::StrAppendFormat(&str, "%s\n-----------------------------------------------\n", linha.c_str());
  }
  return str;
}

class MeuDock : public QDockWidget {
 public:
  MeuDock(Principal* principal, ntf::CentralNotificacoes* central, ent::Tabuleiro* tabuleiro) : QDockWidget("", principal) {
    central_ = central;
    tabuleiro_ = tabuleiro;
    edit_ = new QTextEdit();
    edit_->setReadOnly(true);
    setAllowedAreas(Qt::BottomDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetClosable);
    setWidget(edit_);
    setVisible(tabuleiro->Opcoes().mostra_log_eventos());
    QWidget* title_bar = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout;
    rotulo_ = new QLabel("Log própio");
    botao_proprio_ = new QPushButton("Próprio");
    lambda_connect(botao_proprio_, SIGNAL(pressed()), [this] () { BotaoProprio(); });
    botao_requisitar_clientes_ = new QPushButton("Requisitar clientes");
    lambda_connect(botao_requisitar_clientes_, SIGNAL(pressed()), [this] () { BotaoRequisitarClientes(); });
    botao_alternar_cliente_ = new QPushButton("Alternar clientes");
    lambda_connect(botao_alternar_cliente_, SIGNAL(pressed()), [this] () { BotaoAlternarCliente(); });
    botao_fechar_ = new QPushButton("Fechar");
    lambda_connect(botao_fechar_, SIGNAL(pressed()), [this] () { BotaoFechar(); });
    layout->addWidget(rotulo_);
    layout->addWidget(botao_proprio_);
    layout->addWidget(botao_requisitar_clientes_);
    layout->addWidget(botao_alternar_cliente_);
    layout->addStretch();
    layout->addWidget(botao_fechar_);
    title_bar->setLayout(layout);
    setTitleBarWidget(title_bar);

    lambda_connect(this, SIGNAL(visibilityChanged(bool)), [this]() {
      if (isVisible()) {
        BotaoProprio();
      } else {
      }
    });
  }

 private:
  void BotaoProprio() {
    rotulo_->setText("Log próprio");
    edit_->clear();
    edit_->insertPlainText(MontaStringLog(tabuleiro_->LogEventos()).c_str());
  }

  void BotaoRequisitarClientes() {
    auto n = ntf::NovaNotificacao(ntf::TN_REQUISITAR_LOG_EVENTOS);
    central_->AdicionaNotificacaoRemota(std::move(n));
  }

  void BotaoAlternarCliente() {
    const auto& log_por_cliente = tabuleiro_->LogEventosClientes();
    if (log_por_cliente.empty()) return;
    auto it = log_por_cliente.find(cliente_corrente_);
    if (++it == log_por_cliente.end()) it = log_por_cliente.begin();
    cliente_corrente_ = it->first;
    rotulo_->setText(QString("Log de ").append(cliente_corrente_.c_str()));
    edit_->clear();
    edit_->insertPlainText(it->second.c_str());
  }

  void BotaoFechar() {
    hide();
    tabuleiro_->AlternaMostraLogEventos();
  }

 private:
  QLabel* rotulo_;
  QTextEdit* edit_;
  QPushButton* botao_proprio_;
  QPushButton* botao_requisitar_clientes_;
  QPushButton* botao_alternar_cliente_;
  QPushButton* botao_fechar_;
  ntf::CentralNotificacoes* central_;
  ent::Tabuleiro* tabuleiro_;
  std::string cliente_corrente_;
};

}  // namespace

void Principal::Executa() {
  // maximiza janela
  //QDesktopWidget* qdw = QApplication::desktop();
  //setGeometry(qdw->screenGeometry());
  setGeometry(QRect(100, 100, 1024, 768));

  setMenuBar(menu_principal_);
  connect(menu_principal_, SIGNAL(LogAlternado()), this, SLOT(LogAlternado()));

  setCentralWidget(v3d_);

  dock_log_ = new MeuDock(this, central_, tabuleiro_);
  addDockWidget(Qt::BottomDockWidgetArea, dock_log_);

  const auto& opcoes = tabuleiro_->Opcoes();
  float intervalo_notificacao_ms = 1000.0 / opcoes.fps();
  LOG(INFO) << "Resolucao timer: " << intervalo_notificacao_ms;
  LOG(INFO) << "FPS: " << opcoes.fps();
  QTimer::singleShot(intervalo_notificacao_ms, this, &Principal::Temporizador);

  // mostra a janela e entra no loop do QT
  show();
  q_app_->exec();
}

void Principal::closeEvent(QCloseEvent *event) {
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

  if (tabuleiro_->Opcoes().mostra_log_eventos() && !dock_log_->isVisible()) {
    dock_log_->show();
  } else if (!tabuleiro_->Opcoes().mostra_log_eventos() && dock_log_->isVisible()) {
    dock_log_->hide();
  }

  // Realiza a notificação de todos.
  auto* notificacao = new ntf::Notificacao;
  notificacao->set_tipo(ntf::TN_TEMPORIZADOR);
  central_->AdicionaNotificacao(notificacao);
  v3d_->PegaContexto();
  central_->Notifica();
  v3d_->LiberaContexto();

  const auto& opcoes = tabuleiro_->Opcoes();
  float intervalo_notificacao_ms = 1000.0 / opcoes.fps();
  QTimer::singleShot(intervalo_notificacao_ms, this, &Principal::Temporizador);
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
