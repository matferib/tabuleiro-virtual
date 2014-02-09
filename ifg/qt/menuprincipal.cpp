/** @file ifg/qt/MenuPrincipal.cpp implementacao do menu principal. */

#include <QActionGroup>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QVariant>
#include <google/protobuf/text_format.h>

#include "ifg/qt/constantes.h"
#include "ifg/qt/menuprincipal.h"
#include "ifg/qt/principal.h"
#include "ifg/qt/util.h"
#include "log/log.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

namespace ifg {
namespace qt {

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
  "&Opções", "&Propriedades", nullptr, "&Salvar", "R&estaurar", g_fim,
  // Entidades. 
  "&Selecionar modelo", "&Propriedades", nullptr, "&Adicionar", "&Remover", g_fim,
  // Sobre
  "&Tabuleiro virtual", g_fim,
};

// Cria um proto de entidade a partir da string texto.
ent::EntidadeProto* CriaProto(const std::string& str = "") {
  auto* ent = new ent::EntidadeProto;
  if (!google::protobuf::TextFormat::ParseFromString(str, ent)) {
    LOG(ERROR) << "Falha no parser de modelo, str: " << str;
  }
  return ent;
}

}  // namespace

MenuPrincipal::MenuPrincipal(ntf::CentralNotificacoes* central, QWidget* pai)
    : QMenuBar(pai), central_(central) {
  mapa_modelos_.insert(std::make_pair("&Padrão", std::unique_ptr<ent::EntidadeProto>(CriaProto())));
  mapa_modelos_.insert(std::make_pair("Teste", std::unique_ptr<ent::EntidadeProto>(CriaProto("pontos_vida: 5"))));
  modelo_selecionado_ = mapa_modelos_.find("&Padrão")->second.get();

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
      if (menuitem_str == nullptr) {
        menu->addSeparator();
      } else if (std::string(menuitem_str) == "&Selecionar modelo") {
        // Esse menu tem tratamento especial. TODO ler de um arquivo os modelos.
        auto* grupo = new QActionGroup(this); 
        grupo->setExclusive(true);
        auto* menu_modelos = menu->addMenu(tr(menuitem_str));
        for (const auto& modelo_it : mapa_modelos_) {
          auto* sub_acao = new QAction(tr(modelo_it.first.c_str()), menu);
          sub_acao->setCheckable(true);
          sub_acao->setData(QVariant::fromValue((void*)modelo_it.second.get()));
          grupo->addAction(sub_acao);
          menu_modelos->addAction(sub_acao);
          if (modelo_it.first == "&Padrão") {
            sub_acao->setChecked(true);
          }
        }
        connect(menu_modelos, SIGNAL(triggered(QAction*)), this, SLOT(TrataAcaoModelo(QAction*)));
      } else {
        // menuitem nao NULL, adiciona normalmente da lista de menuitems
        // incrementando para o proximo no final
        auto* acao = new QAction(tr(menuitem_str), menu);
        acoes_[controle_menu].push_back(acao);
        menu->addAction(acao);
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

void MenuPrincipal::TrataAcaoModelo(QAction* acao){
  modelo_selecionado_ = reinterpret_cast<const ent::EntidadeProto*>(acao->data().value<void*>());
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
  } else if (acao == acoes_[ME_ENTIDADES][MI_PROPRIEDADES_ENTIDADE]) {
    // TODO
  } else if (acao == acoes_[ME_ENTIDADES][MI_ADICIONAR]) {
    notificacao = new ntf::Notificacao; 
    notificacao->set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
    notificacao->mutable_entidade()->CopyFrom(*modelo_selecionado_);
  } else if (acao == acoes_[ME_ENTIDADES][MI_REMOVER]) {
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
  } else if (acao == acoes_[ME_TABULEIRO][MI_PROPRIEDADES]) {
    notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_ABRIR_DIALOGO_PROPRIEDADES_TABULEIRO);
  } else if (acao == acoes_[ME_TABULEIRO][MI_OPCOES]) {
    notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_ABRIR_DIALOGO_OPCOES);
  }
  // .. 
  else if (acao == acoes_[ME_SOBRE][MI_TABVIRT]) {
    // mostra a caixa de dialogo da versao
    QMessageBox::about(
        qobject_cast<QWidget*>(parent()),
        tr("Sobre o tabuleiro virtual"), 
        tr("Tabuleiro virtual versão 0.1\n"
           "Bibliotecas: QT, OpenGL, Protobuf, Boost\n"
           "Autor: Matheus Ribeiro <mfribeiro@gmail.com>"));
  }

  if (notificacao != nullptr) {
    central_->AdicionaNotificacao(notificacao);
  }
}

}  // namespace ifg
}  // namespace qt
