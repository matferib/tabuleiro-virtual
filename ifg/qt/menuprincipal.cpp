/** @file ifg/qt/MenuPrincipal.cpp implementacao do menu principal. */

/*
TRANSLATOR ifg::qt::MenuPrincipal
Necessary for lupdate.
*/

#include <stack>
#include <set>
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
#include <boost/asio/ip/host_name.hpp>

#include "arq/arquivo.h"
#include "ifg/qt/constantes.h"
#include "ifg/qt/menuprincipal.h"
#include "ifg/modelos.pb.h"
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
const char* g_menu_strs[] = { QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Jogo"),
                              QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Tabuleiro"),
                              QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Entidades"),
                              QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Ações"),
                              QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Desenho"),
                              QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Sobre") };

// Strs dos items de cada menu, nullptr para separador e "FIM" para demarcar fim.
const char* g_menuitem_strs[] = {
  // jogo
  QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Iniciar jogo mestre"), QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Conectar no jogo mestre"), nullptr,
    QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Sair"), g_fim,
  // Tabuleiro.
  QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Desfazer (Ctrl + Z)"), QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Refazer (Ctrl + Y)"), nullptr,
    QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Opções"), QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Propriedades"), nullptr,
    QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Reiniciar"), QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Salvar (Ctrl + S)"),
    QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "S&alvar Como"), QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "R&estaurar"),
    QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Res&taurar mantendo Entidades"), QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Salvar &Câmera"),
    QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Re&iniciar Câmera"), g_fim,
  // Entidades.
  QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Selecionar modelo"), QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Propriedades"), nullptr,
    QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Adicionar"), QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Remover"), nullptr,
    QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Salvar selecionáveis"), QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Restaurar selecionáveis"), g_fim,
  // Acoes.
  g_fim,
  // Desenho.
  QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Cilindro"), QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Cí&rculo"), QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "C&one"),
    QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "C&ubo"), QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Esfera"), QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Livre"),
    QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Pirâmide"), QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Retângulo"), nullptr,
    QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Selecionar Cor"), g_fim,
  // Sobre
  QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "&Tabuleiro virtual"), g_fim,
};

// Preenche o menu recursivamente atraves do proto de menus. O menu ficara ordenado alfabeticamente.
void PreencheMenu(const MenuModelos& menu_modelos, QMenu* menu, QActionGroup* grupo) {
  struct DadosMenu {
    DadosMenu(const std::string& id, const QString& str) : id(id), str_menu(str), sub_menu(nullptr) {}
    DadosMenu(const QString& str, const MenuModelos* menu) : str_menu(str), sub_menu(menu) {}
    std::string id;        // sem traducao, pois tem que bater com outro arquivo.
    QString str_menu;  // traduzido.
    const MenuModelos* sub_menu;  // submenus.
    bool operator<(const DadosMenu& rhs) const {
      return str_menu < rhs.str_menu;
    }
  };
  std::set<DadosMenu> conjunto;
  for (const auto& m : menu_modelos.modelo()) {
    conjunto.insert(DadosMenu(m.id(), ifg::qt::MenuPrincipal::tr((m.texto().empty() ? m.id() : m.texto()).c_str())));
  }
  for (const auto& s : menu_modelos.sub_menu()) {
    conjunto.insert(DadosMenu(ifg::qt::MenuPrincipal::tr(s.id().c_str()), &s));
  }
  // Agora preenche os menus.
  for (const auto& dado : conjunto) {
    const std::string& id = dado.id;
    const QString& texto = dado.str_menu;
    const MenuModelos* modelo = dado.sub_menu;
    if (modelo == nullptr) {
      QAction* acao = menu->addAction(texto);
      acao->setCheckable(true);
      grupo->addAction(acao);
      acao->setData(QVariant::fromValue(QString(id.c_str())));
    } else {
      PreencheMenu(*modelo, menu->addMenu(texto), grupo);
    }
  }
}

void MisturaProtosMenu(const MenuModelos& entrada, MenuModelos* saida) {
  for (const auto& m : entrada.modelo()) {
    saida->add_modelo()->CopyFrom(m);
  }
  for (const auto& sub_entrada : entrada.sub_menu()) {
    MenuModelos* sub_saida = nullptr;
    for (auto& esta_sub_saida : *saida->mutable_sub_menu()) {
      if (esta_sub_saida.id() == sub_entrada.id()) {
        sub_saida = &esta_sub_saida;
        break;
      }
    }
    if (sub_saida == nullptr) {
      sub_saida = saida->add_sub_menu();
      sub_saida->set_id(sub_entrada.id());
    }
    MisturaProtosMenu(sub_entrada, sub_saida);
  }
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
        const char* ARQUIVO_MENU_MODELOS = "menumodelos.asciiproto";
        const char* ARQUIVO_MENU_MODELOS_NAO_SRD = "menumodelos_nao_srd.asciiproto";
        auto* grupo = new QActionGroup(this);
        grupo->setExclusive(true);
        const std::string arquivos_menu_modelos[] = { ARQUIVO_MENU_MODELOS, ARQUIVO_MENU_MODELOS_NAO_SRD };
        auto* menu_modelos = menu->addMenu(tr(menuitem_str));
        std::vector<ent::EntidadeProto*> entidades;
        MenuModelos menu_modelos_proto;
        for (const std::string& nome_arquivo_menu_modelo : arquivos_menu_modelos) {
          MenuModelos este_menu_modelos_proto;
          try {
            arq::LeArquivoAsciiProto(arq::TIPO_DADOS, nome_arquivo_menu_modelo, &este_menu_modelos_proto);
            VLOG(2) << "Este modelo: " << este_menu_modelos_proto.DebugString();
            MisturaProtosMenu(este_menu_modelos_proto, &menu_modelos_proto);
          } catch (const std::logic_error& erro) {
            LOG(ERROR) << erro.what();
          }
        }
        VLOG(1) << "Modelos final: " << menu_modelos_proto.DebugString();
        PreencheMenu(menu_modelos_proto, menu_modelos, grupo);
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
      static const char* strings_acoes[] = {
        QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Ácido"),
        QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Ataque Corpo a Corpo"),
        QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Ataque a Distância"),
        QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Bola de Fogo"),
        QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Cone de Gelo"),
        QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Feitiço de Toque"),
        QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Fogo Grego"),
        QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Mãos Flamejantes"),
        QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Míssil Mágico"),
        QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Pedrada (gigante)"),
        QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Raio"),
        QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Relâmpago"),
        QT_TRANSLATE_NOOP("ifg::qt::MenuPrincipal", "Sinalização") };
      VLOG(1) << "Compiler happy: " << strings_acoes[0];
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
      if (notificacao.local() && !notificacao.has_erro()) {
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
    auto* nome_rotulo = new QLabel(tr("Nome do jogador:"));
    auto* nome_le = new QLineEdit();
    std::string nome_completo(boost::asio::ip::host_name());
    std::string nome_simples = nome_completo.substr(0, nome_completo.find("."));
    nome_le->setText(tr(nome_simples.c_str()));
    ql->addWidget(nome_rotulo);
    ql->addWidget(nome_le);
    auto* ip_rotulo = new QLabel(tr("IP:"));
    auto* ip_le = new QLineEdit();
    ip_le->setPlaceholderText(tr("IP:porta ou nome do servidor"));
    ql->addWidget(ip_rotulo);
    ql->addWidget(ip_le);
    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    // Botao OK.
    lambda_connect(bb, SIGNAL(accepted()), [&notificacao, qd, nome_le, ip_le] {
      notificacao = new ntf::Notificacao;
      notificacao->set_tipo(ntf::TN_CONECTAR);
      notificacao->set_id_rede(nome_le->text().toStdString());
      notificacao->set_endereco(ip_le->text().toStdString());
      qd->accept();
    });
    // Botao Cancela.
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
  } else if (acao == acoes_[ME_ENTIDADES][MI_SALVAR_ENTIDADES]) {
    // Abre dialogo de arquivo.
    QString file_str = QFileDialog::getSaveFileName(
        qobject_cast<QWidget*>(parent()),
        tr("Salvar entidades selecionáveis"),
        tr(arq::Diretorio(arq::TIPO_ENTIDADES).c_str()));
    if (file_str.isEmpty()) {
      VLOG(1) << "Operação de salvar cancelada.";
      return;
    }
    notificacao = ntf::NovaNotificacao(ntf::TN_SERIALIZAR_ENTIDADES_SELECIONAVEIS);
    notificacao->set_endereco(file_str.toStdString());
  } else if (acao == acoes_[ME_ENTIDADES][MI_RESTAURAR_ENTIDADES]) {
    QString file_str = QFileDialog::getOpenFileName(qobject_cast<QWidget*>(parent()),
                                                    tr("Abrir entidades selecionáveis"),
                                                    arq::Diretorio(arq::TIPO_ENTIDADES).c_str());
    if (file_str.isEmpty()) {
      VLOG(1) << "Operação de restaurar cancelada.";
      return;
    }
    notificacao = ntf::NovaNotificacao(ntf::TN_DESERIALIZAR_ENTIDADES_SELECIONAVEIS);
    notificacao->set_endereco(file_str.toStdString());
  }
  // Tabuleiro.
  else if (acao == acoes_[ME_TABULEIRO][MI_DESFAZER]) {
    tabuleiro_->TrataComandoDesfazer();
  } else if (acao == acoes_[ME_TABULEIRO][MI_REFAZER]) {
    tabuleiro_->TrataComandoRefazer();
  } else if (acao == acoes_[ME_TABULEIRO][MI_REINICIAR]) {
    notificacao = ntf::NovaNotificacao(ntf::TN_REINICIAR_TABULEIRO);
  } else if (acao == acoes_[ME_TABULEIRO][MI_SALVAR]) {
    notificacao = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_SALVAR_TABULEIRO);
  } else if (acao == acoes_[ME_TABULEIRO][MI_SALVAR_COMO]) {
    notificacao = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_SALVAR_TABULEIRO);
    notificacao->mutable_tabuleiro()->set_nome("");  // nome vazio: deversa ser definido.
  } else if (acao == acoes_[ME_TABULEIRO][MI_RESTAURAR] ||
             acao == acoes_[ME_TABULEIRO][MI_RESTAURAR_MANTENDO_ENTIDADES]) {
    QString file_str = QFileDialog::getOpenFileName(qobject_cast<QWidget*>(parent()),
                                                    tr("Abrir tabuleiro"),
                                                    arq::Diretorio(arq::TIPO_TABULEIRO).c_str());
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
  } else if (acao == acoes_[ME_TABULEIRO][MI_SALVAR_CAMERA]) {
    notificacao = ntf::NovaNotificacao(ntf::TN_SALVAR_CAMERA);
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
        tr("Tabuleiro virtual versão 1.10.1\n"
           "Bibliotecas: QT, OpenGL, Protobuf, Boost\n"
           "Ícones: origem http://www.flaticon.com/\n"
           "- Designed by Freepik\n"
           "Autor: Matheus Ribeiro <mfribeiro+tabuleiro@gmail.com>"));
  }

  if (notificacao != nullptr) {
    central_->AdicionaNotificacao(notificacao);
  }
}

}  // namespace ifg
}  // namespace qt
