/** @file ifg/qt/MenuPrincipal.cpp implementacao do menu principal. */

#include <QActionGroup>
#include <QBoxLayout>
#include <QColor>
#include <QColorDialog>
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
const char* g_menu_strs[] = { "&Jogo", "&Tabuleiro", "&Entidades", "&Ações", "&Desenho", "&Sobre" };

// Strs dos items de cada menu, nullptr para separador e "FIM" para demarcar fim.
const char* g_menuitem_strs[] = {
  // jogo
  "&Iniciar jogo mestre", "&Conectar no jogo mestre", nullptr, "&Sair", g_fim,
  // Tabuleiro.
  "Desfazer (Ctrl + Z)", "Refazer (Ctrl + Y)", nullptr, "&Opções", "&Propriedades", nullptr,
      "&Reiniciar", "&Salvar (Ctrl + S)",  "S&alvar Como", "R&estaurar", "Res&taurar mantendo Entidades", "Re&iniciar Câmera", g_fim,
  // Entidades.
  "&Selecionar modelo", "&Propriedades", nullptr, "&Adicionar", "&Remover", g_fim,
  // Acoes.
  g_fim,
  // Desenho.
  "&Cilindro", "Cí&rculo", "C&one", "C&ubo", "&Esfera", "&Livre", "&Pirâmide", "&Retângulo", nullptr, "&Selecionar Cor", g_fim,
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

MenuPrincipal::MenuPrincipal(ent::Tabuleiro* tabuleiro, ntf::CentralNotificacoes* central, QWidget* pai)
    : QMenuBar(pai), tabuleiro_(tabuleiro), central_(central) {
  // inicio das strings para o menu corrente
  unsigned int controle_item = 0;
  for (
    unsigned int controle_menu = ME_JOGO;
    controle_menu < ME_NUM;
    ++controle_menu
  ) {
    QMenu* menu = new QMenu(tr(g_menu_strs[controle_menu]), this);
    QActionGroup* grupo_menu = nullptr;
    if (controle_menu == ME_DESENHO || controle_menu == ME_ACOES) {
      // Menus com acoes exclusivas.
      grupo_menu = new QActionGroup(this);
      grupo_menu->setExclusive(true);
    }
    menus_.push_back(menu);
    // para cada item no menu, cria os items (acoes)
    acoes_.push_back(std::vector<QAction*>());
    const char* menuitem_str = nullptr;
    int id_item = 0;  // Equivale aos MI_*.
    while ((menuitem_str = g_menuitem_strs[controle_item]) != g_fim) {
      if (menuitem_str == nullptr) {
        menu->addSeparator();
      } else if (std::string(menuitem_str) == "&Selecionar modelo") {
        // Esse sub menu tem tratamento especial.
        auto* grupo = new QActionGroup(this);
        grupo->setExclusive(true);
        auto* menu_modelos = menu->addMenu(tr(menuitem_str));
        std::vector<std::pair<std::string, const ent::EntidadeProto*>> modelos_ordenados;
        for (const auto& modelo_it : tabuleiro_->MapaModelos()) {
          auto par = std::make_pair(modelo_it.first, modelo_it.second.get());
          modelos_ordenados.push_back(par);
        }
        std::sort(modelos_ordenados.begin(), modelos_ordenados.end(),
                  [] (const std::pair<std::string, const ent::EntidadeProto*>& esq,
                      const std::pair<std::string, const ent::EntidadeProto*>& dir) {
            return esq.first < dir.first;
        });
        for (const auto& modelo_it : modelos_ordenados) {
          auto* sub_acao = new QAction(tr(modelo_it.first.c_str()), menu);
          sub_acao->setCheckable(true);
          sub_acao->setData(QVariant::fromValue(QString(modelo_it.first.c_str())));
          grupo->addAction(sub_acao);
          menu_modelos->addAction(sub_acao);
          acoes_modelos_.push_back(sub_acao);
          if (modelo_it.second == tabuleiro->ModeloSelecionado()) {
            sub_acao->setChecked(true);
          }
        }
        connect(menu_modelos, SIGNAL(triggered(QAction*)), this, SLOT(TrataAcaoModelo(QAction*)));
      } else {
        // menuitem nao NULL, adiciona normalmente da lista de menuitems
        // incrementando para o proximo no final
        auto* acao = new QAction(tr(menuitem_str), menu);
        // Outro hack so pra selecao de cor...
        if (grupo_menu != nullptr && std::string(menuitem_str) != "&Selecionar Cor") {
          acao->setCheckable(true);
          grupo_menu->addAction(acao);
        }
        acao->setData((QVariant::fromValue<int>(id_item)));
        acoes_[controle_menu].push_back(acao);
        menu->addAction(acao);
      }
      ++controle_item;
      ++id_item;
    }
    ++controle_item;  // pula o FIM.
    // Tratamento especifico de menus.
    if (controle_menu == ME_ACOES) {
      // Esse menu tem tratamento especial.
      std::vector<std::pair<std::string, const ent::AcaoProto*>> acoes_ordenadas;
      for (const auto& acao_it : tabuleiro_->MapaAcoes()) {
        auto par = std::make_pair(acao_it.first, acao_it.second.get());
        acoes_ordenadas.push_back(par);
      }
      std::sort(acoes_ordenadas.begin(), acoes_ordenadas.end(), [] (
          const std::pair<std::string, const ent::AcaoProto*>& esq,
          const std::pair<std::string, const ent::AcaoProto*>& dir) {
        return esq.first < dir.first;
      });
      for (const auto& acao_it : acoes_ordenadas) {
        auto* acao = new QAction(tr(acao_it.first.c_str()), menu);
        //acao->setCheckable(true);
        acao->setData(QVariant::fromValue(QString(acao_it.first.c_str())));
        grupo_menu->addAction(acao);
        menu->addAction(acao);
      }
      connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(TrataAcaoAcoes(QAction*)));
    } else if (controle_menu == ME_DESENHO) {
      for (auto* a : grupo_menu->actions()) {
        if (a->data().toInt() == tabuleiro_->FormaDesenhoSelecionada()) {
          a->setChecked(true);
          break;
        }
      }
    }
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
    case ntf::TN_REINICIAR_TABULEIRO:
      // TODO Forma selecionada e desenho selecionado.
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
  menus_[menu]->setEnabled(estado);
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
    for (menu_e menu : { ME_JOGO, ME_TABULEIRO, ME_ENTIDADES, ME_ACOES, ME_SOBRE }) {
      EstadoMenu(true, menu);
    }
    break;
  case MM_MESTRE:
    EstadoItemMenu(false, ME_JOGO, { MI_INICIAR, MI_CONECTAR });
    break;
  case MM_JOGADOR:
    EstadoItemMenu(false, ME_JOGO, { MI_INICIAR, MI_CONECTAR });
    EstadoItemMenu(false, ME_TABULEIRO, { MI_PROPRIEDADES, MI_REINICIAR, MI_SALVAR, MI_SALVAR_COMO, MI_RESTAURAR, MI_RESTAURAR_MANTENDO_ENTIDADES, });
    EstadoMenu(false, ME_DESENHO);
    for (auto* acao : acoes_modelos_) {
      std::string id = acao->data().toString().toStdString();
      const ent::EntidadeProto* e_proto = tabuleiro_->BuscaModelo(id);
      if (e_proto == nullptr) {
        LOG(ERROR) << "Falha ao buscar modelo: " << id;
        continue;
      }
      acao->setEnabled(e_proto->tipo() != ent::TE_COMPOSTA);
    }
    break;
  }
}

void MenuPrincipal::TrataAcaoModelo(QAction* acao){
  tabuleiro_->SelecionaModeloEntidade(acao->data().toString().toStdString());
}

void MenuPrincipal::TrataAcaoAcoes(QAction* acao){
  tabuleiro_->SelecionaAcao(acao->data().toString().toStdString());
}

void MenuPrincipal::TrataAcaoItem(QAction* acao){
  //cout << (const char*)acao->text().toAscii() << endl;
  ntf::Notificacao* notificacao = nullptr;
  // Jogo.
  if (acao == acoes_[ME_JOGO][MI_INICIAR]) {
    notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_INICIAR);
  } else if (acao == acoes_[ME_JOGO][MI_CONECTAR]) {
    // mostra a caixa de dialogo da conexao.
    QDialog* qd = new QDialog(qobject_cast<QWidget*>(parent()));
    qd->setModal(true);
    QLayout* ql = new QBoxLayout(QBoxLayout::TopToBottom, qd);
    auto* nome_rotulo = new QLabel("Nome do jogador:");
    auto* nome_le = new QLineEdit();
    nome_le->setPlaceholderText(tr("Nome do jogador"));
    ql->addWidget(nome_rotulo);
    ql->addWidget(nome_le);
    auto* ip_rotulo = new QLabel("IP:");
    auto* ip_le = new QLineEdit();
    ip_le->setPlaceholderText(tr("IP:porta ou nome do servidor"));
    ql->addWidget(ip_rotulo);
    ql->addWidget(ip_le);
    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    lambda_connect(bb, SIGNAL(accepted()), [&notificacao, qd, nome_le, ip_le] {
      notificacao = new ntf::Notificacao;
      notificacao->set_tipo(ntf::TN_CONECTAR);
      notificacao->set_id(nome_le->text().toStdString());
      notificacao->set_endereco(ip_le->text().toStdString());
      qd->accept();
    });
    connect(bb, SIGNAL(rejected()), qd, SLOT(reject()));
    ql->addWidget(bb);
    qd->setWindowTitle(tr("Endereço do Servidor"));
    qd->exec();
    delete qd;
  } else if (acao == acoes_[ME_JOGO][MI_SAIR]) {
    notificacao = ntf::NovaNotificacao(ntf::TN_SAIR);
  }
  // Entidades.
  else if (acao == acoes_[ME_ENTIDADES][MI_PROPRIEDADES_ENTIDADE]) {
    notificacao = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_ENTIDADE);
  } else if (acao == acoes_[ME_ENTIDADES][MI_ADICIONAR]) {
    notificacao = ntf::NovaNotificacao(ntf::TN_ADICIONAR_ENTIDADE);
  } else if (acao == acoes_[ME_ENTIDADES][MI_REMOVER]) {
    notificacao = ntf::NovaNotificacao(ntf::TN_REMOVER_ENTIDADE);
  }
  // Tabuleiro.
  else if (acao == acoes_[ME_TABULEIRO][MI_DESFAZER]) {
    tabuleiro_->TrataComandoDesfazer();
  } else if (acao == acoes_[ME_TABULEIRO][MI_REFAZER]) {
    tabuleiro_->TrataComandoRefazer();
  } else if (acao == acoes_[ME_TABULEIRO][MI_REINICIAR]) {
    notificacao = ntf::NovaNotificacao(ntf::TN_REINICIAR_TABULEIRO);
  } else if (acao == acoes_[ME_TABULEIRO][MI_SALVAR]) {
    notificacao = ntf::NovaNotificacao(ntf::TN_SERIALIZAR_TABULEIRO);
    notificacao->set_endereco("");  // Endereco vazio para sinalizar o uso do corrente.
  } else if (acao == acoes_[ME_TABULEIRO][MI_SALVAR_COMO]) {
    // Abre dialogo de arquivo.
    QString file_str = QFileDialog::getSaveFileName(
        qobject_cast<QWidget*>(parent()),
        tr("Salvar tabuleiro"),
        tr(DIR_TABULEIROS));
    if (file_str.isEmpty()) {
      VLOG(1) << "Operação de salvar cancelada.";
      return;
    }
    notificacao = ntf::NovaNotificacao(ntf::TN_SERIALIZAR_TABULEIRO);
    notificacao->set_endereco(file_str.toStdString());
  } else if (acao == acoes_[ME_TABULEIRO][MI_RESTAURAR] ||
             acao == acoes_[ME_TABULEIRO][MI_RESTAURAR_MANTENDO_ENTIDADES]) {
    QString file_str = QFileDialog::getOpenFileName(qobject_cast<QWidget*>(parent()),
                                                    tr("Abrir tabuleiro"),
                                                    DIR_TABULEIROS);
    if (file_str.isEmpty()) {
      VLOG(1) << "Operação de restaurar cancelada.";
      return;
    }
    notificacao = ntf::NovaNotificacao(ntf::TN_DESERIALIZAR_TABULEIRO);
    if (acao == acoes_[ME_TABULEIRO][MI_RESTAURAR_MANTENDO_ENTIDADES]) {
      notificacao->mutable_tabuleiro()->set_manter_entidades(true);
    }
    notificacao->set_endereco(file_str.toStdString());
  } else if (acao == acoes_[ME_TABULEIRO][MI_REINICIAR_CAMERA]) {
    notificacao = ntf::NovaNotificacao(ntf::TN_REINICIAR_CAMERA);
  } else if (acao == acoes_[ME_TABULEIRO][MI_PROPRIEDADES]) {
    notificacao = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_PROPRIEDADES_TABULEIRO);
  } else if (acao == acoes_[ME_TABULEIRO][MI_OPCOES]) {
    notificacao = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_OPCOES);
  }
  // Desenho.
  else if (acao == acoes_[ME_DESENHO][MI_CILINDRO] ||
           acao == acoes_[ME_DESENHO][MI_CIRCULO] ||
           acao == acoes_[ME_DESENHO][MI_CONE] ||
           acao == acoes_[ME_DESENHO][MI_CUBO] ||
           acao == acoes_[ME_DESENHO][MI_ESFERA] ||
           acao == acoes_[ME_DESENHO][MI_LIVRE] ||
           acao == acoes_[ME_DESENHO][MI_PIRAMIDE] ||
           acao == acoes_[ME_DESENHO][MI_RETANGULO]) {
    tabuleiro_->SelecionaFormaDesenho(static_cast<ent::TipoForma>(acao->data().toInt()));
  } else if (acao == acoes_[ME_DESENHO][MI_SELECIONAR_COR]) {
    QColor cor_anterior = ProtoParaCor(tabuleiro_->CorDesenho());
    QColor cor = QColorDialog::getColor(cor_anterior, this, QObject::tr("Cor do Desenho"));
    if (!cor.isValid()) {
      return;
    }
    tabuleiro_->SelecionaCorDesenho(CorParaProto(cor));
  }
  // ..
  else if (acao == acoes_[ME_SOBRE][MI_TABVIRT]) {
    // mostra a caixa de dialogo da versao
    QMessageBox::about(
        qobject_cast<QWidget*>(parent()),
        tr("Sobre o tabuleiro virtual"),
        tr("Tabuleiro virtual versão 1.6.0\n"
           "Bibliotecas: QT, OpenGL, Protobuf, Boost\n"
           "Autor: Matheus Ribeiro <mfribeiro@gmail.com>"));
  }

  if (notificacao != nullptr) {
    central_->AdicionaNotificacao(notificacao);
  }
}

}  // namespace ifg
}  // namespace qt
