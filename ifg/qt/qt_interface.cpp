#include <set>
#include <stack>

#include <QtCore/QString>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

#include "arq/arquivo.h"
#include "ifg/modelos.pb.h"
#include "ifg/qt/qt_interface.h"
#include "ifg/qt/ui/entradastring.h"
#include "ifg/qt/ui/listapaginada.h"
#include "ifg/qt/visualizador3d.h"
#include "ifg/qt/util.h"
#include "log/log.h"

namespace ifg {
namespace qt {

namespace {
}

void InterfaceGraficaQt::MostraMensagem(
    bool erro, const std::string& mensagem, std::function<void()> funcao_volta) {
  pai_->v3d()->LiberaContexto();
  // Com qt fechando, não se deve mais mostrar mensagens.
  if (!pai_->Fechando()) {
    if (erro) {
      QMessageBox::warning(pai_, pai_->tr("Erro"), pai_->tr(mensagem.c_str()));
    } else {
      QMessageBox::information(pai_, pai_->tr("Informação"), pai_->tr(mensagem.c_str()));
    }
  }
  pai_->v3d()->PegaContexto();
  funcao_volta();
}

void InterfaceGraficaQt::EscolheItemLista(
    const std::string& titulo,
    const std::optional<std::string>& rotulo_ok,
    const std::vector<RotuloTipoTesouro>& lista,
    std::function<void(bool, int, std::optional<ent::TipoTesouro>)> funcao_volta) {
  pai_->v3d()->LiberaContexto();
  // Popula.
  ifg::qt::Ui::ListaPaginada gerador;
  auto* dialogo = new QDialog(pai_);
  gerador.setupUi(dialogo);
  dialogo->setWindowTitle(QString::fromUtf8(titulo.c_str()));
  for (const auto& [item_str, tipo_tesouro] : lista) {
    auto* item = new QListWidgetItem(gerador.lista);
    auto* label = new QLabel(QString::fromUtf8(item_str.c_str()));
    label->setTextFormat(Qt::RichText);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setOpenExternalLinks(true);
    gerador.lista->setItemWidget(item, label);
  }
  gerador.lista->setFocus();
  auto lambda_aceito = [this, &gerador, dialogo, lista, funcao_volta] () {
    // Importantissimo! Esses lambdas rodam dentro do contexto do dialogo, portanto qualquer operação que causar
    // uma atualização parcial, que cause alguma operação grafica irá corromper o contexto e gerar um erro grafico
    // totalmente aleatorio.
    pai_->v3d()->PegaContexto();
    if (gerador.lista->currentRow() >= 0 && gerador.lista->currentRow() < (int)lista.size()) {
      funcao_volta(/*ok=*/true, gerador.lista->currentRow(), lista[gerador.lista->currentRow()].tipo_tesouro);
    } else {
      funcao_volta(false, -1, std::nullopt);
    }
    pai_->v3d()->LiberaContexto();
    dialogo->accept();
  };
  auto lambda_rejeitado = [this, &gerador, dialogo, lista, funcao_volta] () {
    pai_->v3d()->PegaContexto();
    funcao_volta(false, -1, std::nullopt);
    pai_->v3d()->LiberaContexto();
    dialogo->reject();
  };
  if (rotulo_ok.has_value()) {
    gerador.botoes->button(QDialogButtonBox::Ok)->setText(QString::fromUtf8(rotulo_ok->c_str()));
  }

  lambda_connect(gerador.lista, SIGNAL(itemDoubleClicked(QListWidgetItem*)), lambda_aceito);
  lambda_connect(gerador.botoes, SIGNAL(accepted()), lambda_aceito);
  lambda_connect(gerador.botoes, SIGNAL(rejected()), lambda_rejeitado);
  dialogo->exec();
  delete dialogo;
  pai_->v3d()->PegaContexto();
}

void InterfaceGraficaQt::EscolheItemsLista(
    const std::string& titulo,
    const std::vector<std::string>& lista,
    std::function<void(bool, std::vector<int>)> funcao_volta) {
  pai_->v3d()->LiberaContexto();
  // Popula.
  ifg::qt::Ui::ListaPaginada gerador;
  auto* dialogo = new QDialog(pai_);
  gerador.setupUi(dialogo);
  dialogo->setWindowTitle(QString::fromUtf8(titulo.c_str()));
  for (const auto& item : lista) {
    new QListWidgetItem(QString::fromUtf8(item.c_str()), gerador.lista);
  }
  gerador.lista->setSelectionMode(QAbstractItemView::ExtendedSelection);
  gerador.lista->setFocus();
  auto lambda_aceito = [this, &gerador, dialogo, lista, funcao_volta] () {
    std::vector<int> selecionados;
    for (const auto* item : gerador.lista->selectedItems()) {
      selecionados.push_back(gerador.lista->row(item));
    }
    pai_->v3d()->PegaContexto();
    funcao_volta(!selecionados.empty(), selecionados);
    pai_->v3d()->LiberaContexto();
    dialogo->accept();
  };
  auto lambda_rejeitado = [this, &gerador, dialogo, lista, funcao_volta] () {
    pai_->v3d()->PegaContexto();
    funcao_volta(false, {});
    pai_->v3d()->LiberaContexto();
    dialogo->reject();
  };

  lambda_connect(gerador.lista, SIGNAL(itemDoubleClicked(QListWidgetItem*)), lambda_aceito);
  lambda_connect(gerador.botoes, SIGNAL(accepted()), lambda_aceito);
  lambda_connect(gerador.botoes, SIGNAL(rejected()), lambda_rejeitado);
  dialogo->exec();
  delete dialogo;
  pai_->v3d()->PegaContexto();
}

void InterfaceGraficaQt::EscolheArquivoAbrirTabuleiro(
    const std::vector<std::string>& tab_estaticos,
    const std::vector<std::string>& tab_dinamicos,
    std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta) {
  pai_->v3d()->LiberaContexto();
  // Popula.
  ifg::qt::Ui::ListaPaginada gerador;
  auto* dialogo = new QDialog(pai_);
  gerador.setupUi(dialogo);
  auto* wie = new QListWidgetItem(pai_->tr("Estáticos"), gerador.lista);
  wie->setFlags(Qt::NoItemFlags);
  for (const auto& tab : tab_estaticos) {
    new QListWidgetItem(tab.c_str(), gerador.lista);
  }
  auto* wid = new QListWidgetItem(pai_->tr("Salvos"), gerador.lista);
  wid->setFlags(Qt::NoItemFlags);
  for (const auto& tab : tab_dinamicos) {
    new QListWidgetItem(tab.c_str(), gerador.lista);
  }
  gerador.lista->setFocus();
  auto lambda_aceito = [this, &gerador, dialogo, tab_estaticos, tab_dinamicos, funcao_volta] () {
    int indice = gerador.lista->currentRow();
    int tamanho_total = tab_estaticos.size() + tab_dinamicos.size();
    // +2 por causa dos 2 items label adicionados.
    if (indice > 0 && indice < (tamanho_total + 2)) {
      pai_->v3d()->PegaContexto();
      if (static_cast<unsigned int>(indice - 1) < tab_estaticos.size()) {
        funcao_volta(tab_estaticos[indice - 1], arq::TIPO_TABULEIRO_ESTATICO);
      } else {
        funcao_volta(tab_dinamicos[indice - 2 - tab_estaticos.size()], arq::TIPO_TABULEIRO);
      }
      pai_->v3d()->LiberaContexto();
      dialogo->accept();
    }
  };
  auto lambda_rejeitado = [this, &gerador, dialogo, tab_estaticos, tab_dinamicos, funcao_volta] () {
    pai_->v3d()->PegaContexto();
    funcao_volta("", arq::TIPO_TABULEIRO);
    pai_->v3d()->LiberaContexto();
    dialogo->reject();
  };
  lambda_connect(gerador.lista, SIGNAL(itemDoubleClicked(QListWidgetItem*)), lambda_aceito);
  lambda_connect(gerador.botoes, SIGNAL(accepted()), lambda_aceito);
  lambda_connect(gerador.botoes, SIGNAL(rejected()), lambda_rejeitado);
  dialogo->exec();
  delete dialogo;
  pai_->v3d()->PegaContexto();
}

void InterfaceGraficaQt::EscolheArquivoAbrirImagem(
  const std::vector<std::string>& imagens_locais, const std::vector<std::string>& imagens_globais, std::function<void(const std::string& nome, arq::tipo_e)> funcao_volta) {
  pai_->v3d()->LiberaContexto();
  // Popula.
  ifg::qt::Ui::ListaPaginada gerador;
  auto* dialogo = new QDialog(pai_);
  gerador.setupUi(dialogo);
  {
    auto* wie = new QListWidgetItem(pai_->tr("Locais"), gerador.lista);
    wie->setFlags(Qt::NoItemFlags);
  }
  for (const auto& imagem : imagens_locais) {
    new QListWidgetItem(imagem.c_str(), gerador.lista);
  }
  {
    auto* wie = new QListWidgetItem(pai_->tr("Estáticos"), gerador.lista);
    wie->setFlags(Qt::NoItemFlags);
  }
  for (const auto& imagem : imagens_globais) {
    new QListWidgetItem(imagem.c_str(), gerador.lista);
  }
  gerador.lista->setFocus();
  auto lambda_aceito = [this, &gerador, dialogo, imagens_locais, imagens_globais, funcao_volta]() {
    int indice = gerador.lista->currentRow();
    int tamanho_total = imagens_locais.size() + imagens_globais.size();
    if (indice != 0 && indice != (imagens_locais.size() + 1) && indice < (tamanho_total + 2)) {
      pai_->v3d()->PegaContexto();
      if (indice < (imagens_locais.size() + 1)) {
        funcao_volta(imagens_locais[indice - 1], arq::TIPO_TEXTURA_LOCAL);
      } else {
        funcao_volta(imagens_globais[indice - 2 - imagens_locais.size()], arq::TIPO_TEXTURA);
      }
      pai_->v3d()->LiberaContexto();
      dialogo->accept();
    }
    };
  auto lambda_rejeitado = [this, &gerador, dialogo, imagens_locais, funcao_volta]() {
    pai_->v3d()->PegaContexto();
    funcao_volta("", arq::TIPO_TEXTURA);
    pai_->v3d()->LiberaContexto();
    dialogo->reject();
  };
  lambda_connect(gerador.lista, SIGNAL(itemDoubleClicked(QListWidgetItem*)), lambda_aceito);
  lambda_connect(gerador.botoes, SIGNAL(accepted()), lambda_aceito);
  lambda_connect(gerador.botoes, SIGNAL(rejected()), lambda_rejeitado);
  dialogo->exec();
  delete dialogo;
  pai_->v3d()->PegaContexto();
}

void InterfaceGraficaQt::EscolheArquivoSalvarTabuleiro(
    std::function<void(const std::string& nome)> funcao_volta) {
  pai_->v3d()->LiberaContexto();
  ifg::qt::Ui::EntradaString gerador;
  auto* dialogo = new QDialog(pai_);
  gerador.setupUi(dialogo);
  gerador.label_titulo->setText(QDialog::tr("Salvar Tabuleiro"));
  gerador.nome->setFocus();
  auto lambda_aceito = [this, &gerador, dialogo, funcao_volta] () {
    if (gerador.nome->text().length() > 0) {
      pai_->v3d()->PegaContexto();
      funcao_volta(gerador.nome->text().toUtf8().constData());
      pai_->v3d()->LiberaContexto();
      dialogo->accept();
    }
  };
  lambda_connect(gerador.botoes, SIGNAL(accepted()), lambda_aceito);
  QObject::connect(gerador.botoes, SIGNAL(rejected()), dialogo, SLOT(reject()));
  dialogo->exec();
  delete dialogo;
  pai_->v3d()->PegaContexto();
}

namespace {

std::set<std::string> ExtraiItensMenu(const MenuModelos& menu_modelos) {
  std::stack<const MenuModelos*> menus;
  menus.push(&menu_modelos);
  std::set<std::string> ret;
  do {
    const auto* menu = menus.top();
    for (const auto& item_menu : menu->item_menu()) {
      // ATENCAO: Se o texto for usado, deve-se manter um mapeamento de texto->id porque o callback usa o id.
      ret.insert(item_menu.id());
    }
    menus.pop();
    for (const auto& sub_menu : menu->sub_menu()) {
      menus.push(&sub_menu);
    }
  } while (!menus.empty());
  return ret;
}

}  // namespace

void InterfaceGraficaQt::EscolheModeloEntidade(const MenuModelos& menu_modelos, std::function<void(const std::string& nome)> funcao_volta) {
  pai_->v3d()->LiberaContexto();
  ifg::qt::Ui::ListaPaginada gerador;
  auto* dialogo = new QDialog(pai_);
  gerador.setupUi(dialogo);
  auto* wie = new QListWidgetItem(pai_->tr("Modelos"), gerador.lista);
  wie->setFlags(Qt::NoItemFlags);
  std::set<std::string> ids_itens = ExtraiItensMenu(menu_modelos);
  for (const std::string& id : ids_itens) {
    new QListWidgetItem(pai_->tr(id.c_str()), gerador.lista);
  }
  gerador.lista->setFocus();
  auto lambda_aceito = [this, &gerador, dialogo, ids_itens, funcao_volta] () {
    pai_->v3d()->PegaContexto();
    int indice = gerador.lista->currentRow();
    if (indice > 0) {
      auto it = ids_itens.begin();
      // -1 por causa do label adicionado.
      std::advance(it, indice - 1);
      funcao_volta(*it);
    } else {
      funcao_volta("");
    }
    pai_->v3d()->LiberaContexto();
    //LOG(INFO) << "aceitei: " << (indice > 0 ? modelos.begin() + (indice - 1) : "nada");
    dialogo->accept();
  };
  lambda_connect(gerador.lista, SIGNAL(itemDoubleClicked(QListWidgetItem*)), lambda_aceito);
  lambda_connect(gerador.botoes, SIGNAL(accepted()), lambda_aceito);
  QObject::connect(gerador.botoes, SIGNAL(rejected()), dialogo, SLOT(reject()));
  dialogo->exec();
  delete dialogo;
  pai_->v3d()->PegaContexto();
}

void InterfaceGraficaQt::EscolheCor(
    TipoCor tc, std::optional<int> id_cenario, float r, float g, float b, float a, 
    std::function<void(bool, TipoCor, std::optional<int>, float, float, float, float)> funcao_volta) {
  pai_->v3d()->LiberaContexto();
  QColor cor_ida(r, g, b);
  QColor cor = QColorDialog::getColor(cor_ida, pai_, QObject::tr("Escolha Cor"));
  pai_->v3d()->PegaContexto();
  if (!cor.isValid()) {
    funcao_volta(false, tc, id_cenario, r, g, b, a);
  } else {
    funcao_volta(true, tc, id_cenario, cor.redF(), cor.greenF(), cor.blueF(), cor.alphaF());
  }
}

void InterfaceGraficaQt::EscolheValorDadoForcado(const std::string& titulo, int nfaces, std::function<void(int)> funcao_volta) {
  pai_->v3d()->LiberaContexto();
  auto* dialogo = new QDialog(pai_);
  int numero_colunas = nfaces == 100
      ? 10
      : nfaces >= 10 ? (nfaces / 2) : nfaces;
  int val = 0;
  auto* layout = new QGridLayout(dialogo);
  dialogo->setWindowTitle(absl::StrCat("Escolha o valor do d", nfaces).c_str());
  for (int i = 1; i <= nfaces; ++i) {
    auto* botao = new QPushButton(absl::StrCat(i).c_str());
    lambda_connect(botao, SIGNAL(clicked()), [&val, i, dialogo]() { val = i; dialogo->accept(); });
    layout->addWidget(botao, /*row=*/(i-1) / numero_colunas, /*column=*/(i-1) % numero_colunas);
  }
  dialogo->exec();
  pai_->v3d()->PegaContexto();
  funcao_volta(val);
}

}  // namespace qt
}  // namespace ent
