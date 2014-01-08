/** @file ifg/qt/MenuPrincipal.cpp implementacao do menu principal. */

#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>

#include "ifg/qt/menuprincipal.h"
#include "ifg/qt/principal.h"
#include "ifg/qt/util.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

using namespace ifg::qt;

// enumeracao com os menus e seus items
namespace {

enum menu_e { ME_JOGO, ME_TABULEIRO, ME_ENTIDADES, ME_SOBRE, ME_NUM }; // menus da barra

enum menuitem_e { // items de cada menu 
  MI_INICIAR = 0, MI_CONECTAR, MI_SAIR,
  MI_ILUMINACAO = 0, MI_SALVAR, MI_RESTAURAR,
  MI_ADICIONAR = 0, MI_ADICIONAR_LUZ, MI_REMOVER,
  MI_TABVIRT = 0,
  MI_SEP = 0
};

const char* g_fim = "FIM";

// Strs de cada menu.
const char* g_menu_strs[] = { "&Jogo", "&Tabuleiro", "&Entidades", "&Sobre" };

// Strs dos items de cada menu, nullptr para separador e "FIM" para demarcar fim.
const char* g_menuitem_strs[] = {
  // jogo
  "&Iniciar jogo mestre", "&Conectar no jogo mestre", nullptr, "&Sair", g_fim,
  // Tabuleiro.
  "&Iluminação", nullptr, "&Salvar", "R&estaurar", g_fim,
  // Entidades. 
  "&Adicionar", "Adicionar &Luz", "&Remover", g_fim,
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
        //acoes_[controle_menu].push_back(nullptr);
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

void MenuPrincipal::Modo(modomenu_e modo){
  // jogo e sobre sempre habilitados
  menus_[ME_JOGO]->setEnabled(true);
  menus_[ME_SOBRE]->setEnabled(true);

  switch (modo){
  case MM_COMECO:
    // habilita todos do jogo
    for (
      std::vector<QAction*>::iterator it = acoes_[ME_JOGO].begin();
      it != acoes_[ME_JOGO].end();
      ++it
    ) {
      QAction* acao = *it;
      if (acao != NULL){
        acao->setEnabled(true);
      }
    }
    // desabilita jogadores
    menus_[ME_ENTIDADES]->setEnabled(false);
    break;
  case MM_MESTRE:
  case MM_JOGADOR:
    // desabilita tudo menos sair no jogo
    for (
      std::vector<QAction*>::iterator it = acoes_[ME_JOGO].begin();
      it != acoes_[ME_JOGO].end();
      ++it
    ) {
      QAction* acao = *it;
      if (acao != NULL) {
        acao->setEnabled(false);
      }
    }
    acoes_[ME_JOGO][MI_SAIR]->setEnabled(true);

    // Jogadores habilitado so no modo mestre
    menus_[ME_ENTIDADES]->setEnabled(modo == MM_MESTRE ? true : false);
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
  } else if (acao == acoes_[ME_ENTIDADES][MI_ADICIONAR_LUZ]) {
    notificacao = new ntf::Notificacao; 
    notificacao->set_tipo(ntf::TN_ADICIONAR_LUZ);
  } else if (acao == acoes_[ME_ENTIDADES][MI_REMOVER]) {
    // @todo abrir dialogo modal pedindo dados do jogador
    notificacao = new ntf::Notificacao; 
    notificacao->set_tipo(ntf::TN_REMOVER_ENTIDADE);
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





