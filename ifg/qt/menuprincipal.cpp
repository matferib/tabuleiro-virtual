/** @file ifg/qt/MenuPrincipal.cpp implementacao do menu principal. */

#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>

#include "ifg/qt/constantes.h"
#include "ifg/qt/menuprincipal.h"
#include "ifg/qt/principal.h"
#include "ifg/qt/util.h"
#include "log/log.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

using namespace ifg::qt;

// enumeracao com os menus e seus items
namespace {

const char* g_fim = "FIM";

// Strs de cada menu.
const char* g_menu_strs[] = { "&Jogo", "&Tabuleiro", "&Entidades", "&Sobre" };

// Strs dos items de cada menu, nullptr para separador e "FIM" para demarcar fim.
const char* g_menuitem_strs[] = {
  // jogo
  "&Iniciar jogo mestre", "&Conectar no jogo mestre", nullptr, "&Sair", g_fim,
  // Tabuleiro.
  "&Iluminação e Textura", nullptr, "&Salvar", "R&estaurar", g_fim,
  // Entidades. 
  "&Adicionar", "&Remover", g_fim,
  // Sobre
  "&Tabuleiro virtual", g_fim,
};

}  // namespace

MenuPrincipal::MenuPrincipal(ntf::CentralNotificacoes* central, QWidget* pai)
    : QMenuBar(pai), central_(central) {
  // inicio das strings para o menu corrente
  unsigned int controle_item = 0;
  for (
    unsigned int controle_menu = ME_JOGO; 
    controle_menu < ME_NUM; 
    ++controle_menu
  ) {
    QMenu* menu = new QMenu(tr(g_menu_strs[controle_menu]), this);
    menus_.push_back(menu);
    // para cada item no menu, cria os items (acoes)
    acoes_.push_back(std::vector<QAction*>());
    const char* menuitem_str = nullptr;
    while ((menuitem_str = g_menuitem_strs[controle_item]) != g_fim) {
      if (menuitem_str != nullptr) {
        // menuitem nao NULL, adiciona normalmente da lista de menuitems
        // incrementando para o proximo no final
        auto* acao = new QAction(tr(menuitem_str), menu);
        acoes_[controle_menu].push_back(acao);
        menu->addAction(acao);
      } else {
        // menuitem NULL, adiciona separador e a acao NULL para manter contador
        menu->addSeparator();
      }
      ++controle_item;
    }
    ++controle_item;  // pula o FIM.
    // adiciona os menus ao menu principal
    connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(TrataAcaoItem(QAction*)));
    addMenu(menu);
  }

  Modo(MM_COMECO);
  central_->RegistraReceptor(this);
}

MenuPrincipal::~MenuPrincipal(){
}

bool MenuPrincipal::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_INICIADO:
      Modo(MM_MESTRE);
      return true;
    case ntf::TN_RESPOSTA_CONEXAO:
      if (notificacao.has_erro()) {
        // Mostra dialogo com mensagem de erro.
        QMessageBox::information(
            qobject_cast<QWidget*>(parent()), tr("Erro de Conexão"), tr(notificacao.erro().c_str()));
      } else {
        Modo(MM_JOGADOR);
      }
      return true;
    default:
      return false;
  }
}

void MenuPrincipal::EstadoMenu(bool estado, menu_e menu) {
  for (QAction* acao : acoes_[menu]) {
    if (acao != NULL){
      acao->setEnabled(estado);
    }
  }
  menus_[estado]->setEnabled(estado);
}

void MenuPrincipal::EstadoItemMenu(bool estado, menu_e menu, const std::vector<menuitem_e>& items) {
  for (menuitem_e item : items) {
    QAction* acao = acoes_[menu][item];
    acao->setEnabled(estado);
  }
}

void MenuPrincipal::Modo(modomenu_e modo){
  switch (modo){
  case MM_COMECO:
    for (menu_e menu : { ME_JOGO, ME_TABULEIRO, ME_ENTIDADES, ME_SOBRE }) {
      EstadoMenu(true, menu);
    }
    break;
  case MM_MESTRE:
  case MM_JOGADOR:
    EstadoItemMenu(false, ME_JOGO, { MI_INICIAR, MI_CONECTAR });
    EstadoItemMenu(false, ME_TABULEIRO, { MI_SALVAR, MI_RESTAURAR });
    break;
  }
}

void MenuPrincipal::TrataAcaoItem(QAction* acao){
  //cout << (const char*)acao->text().toAscii() << endl;
  ntf::Notificacao* notificacao = nullptr;
  if (acao == acoes_[ME_JOGO][MI_INICIAR]) {
    notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_INICIAR);
  } else if (acao == acoes_[ME_JOGO][MI_CONECTAR]) {
    // mostra a caixa de dialogo da conexao. 
    QDialog* qd = new QDialog(qobject_cast<QWidget*>(parent()));
    qd->setModal(true);
    QLayout* ql = new QBoxLayout(QBoxLayout::TopToBottom, qd);
    auto* le = new QLineEdit();
    le->setPlaceholderText(tr("IP:porta ou nome do servidor")); 
    ql->addWidget(le);
    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    lambda_connect(bb, SIGNAL(accepted()), [&notificacao, qd, le] {
      notificacao = new ntf::Notificacao;
      notificacao->set_tipo(ntf::TN_CONECTAR);
      notificacao->set_endereco(le->text().toStdString());
      qd->accept();
    });
    connect(bb, SIGNAL(rejected()), qd, SLOT(reject()));
    ql->addWidget(bb);
    qd->setWindowTitle(tr("Endereço do Servidor"));
    qd->exec();
    delete qd;
  } else if (acao == acoes_[ME_JOGO][MI_SAIR]) {
    notificacao = new ntf::Notificacao; 
    notificacao->set_tipo(ntf::TN_SAIR);
  } else if (acao == acoes_[ME_ENTIDADES][MI_ADICIONAR]) {
    // @todo abrir dialogo modal pedindo dados do jogador
    notificacao = new ntf::Notificacao; 
    notificacao->set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
  } else if (acao == acoes_[ME_ENTIDADES][MI_REMOVER]) {
    // @todo abrir dialogo modal pedindo dados do jogador
    notificacao = new ntf::Notificacao; 
    notificacao->set_tipo(ntf::TN_REMOVER_ENTIDADE);
  } else if (acao == acoes_[ME_TABULEIRO][MI_SALVAR]) {
    // Abre dialogo de arquivo.
    QString file_str = QFileDialog::getSaveFileName(
        qobject_cast<QWidget*>(parent()),
        tr("Salvar tabuleiro"),
        tr(DIR_TABULEIROS));
    if (file_str.isEmpty()) {
      VLOG(1) << "Operação de salvar cancelada.";
      return;
    }
    notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_SERIALIZAR_TABULEIRO);
    notificacao->set_endereco(file_str.toStdString());
  } else if (acao == acoes_[ME_TABULEIRO][MI_RESTAURAR]) {
    QString file_str = QFileDialog::getOpenFileName(qobject_cast<QWidget*>(parent()),
                                                    tr("Abrir tabuleiro"),
                                                    DIR_TABULEIROS);
    if (file_str.isEmpty()) {
      VLOG(1) << "Operação de restaurar cancelada.";
      return;
    }
    notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_DESERIALIZAR_TABULEIRO);
    notificacao->set_endereco(file_str.toStdString());
  } else if (acao == acoes_[ME_TABULEIRO][MI_ILUMINACAO]) {
    notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_ABRIR_DIALOGO_ILUMINACAO);
  }
  // .. 
  else if (acao == acoes_[ME_SOBRE][MI_TABVIRT]) {
    // mostra a caixa de dialogo da versao
    QMessageBox::about(
        qobject_cast<QWidget*>(parent()),
        tr("Sobre o tabuleiro virtual"), 
        tr("Tabuleiro virtual versão 0.1\n"
           "Powered by QT and OpenGL\n"
           "Autor: Matheus Ribeiro <mfribeiro@gmail.com>"));
  }

  if (notificacao != nullptr) {
    central_->AdicionaNotificacao(notificacao);
  }
}





