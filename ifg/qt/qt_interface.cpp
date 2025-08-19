#include <set>
#include <stack>

#include <QtCore/QString>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

#include "arq/arquivo.h"
#include "ifg/modelos.pb.h"
#include "ifg/qt/qt_interface.h"
#include "ifg/qt/ui/entradastring.h"
#include "ifg/qt/ui/listapaginada.h"
#include "ifg/qt/util.h"
#include "log/log.h"

namespace ifg {
namespace qt {

namespace {
}

void InterfaceGraficaQt::MostraMensagem(
    bool erro, const std::string& mensagem, std::function<void()> funcao_volta) {
  if (erro) {
    QMessageBox::warning(pai_, pai_->tr("Erro"), pai_->tr(mensagem.c_str()));
  } else {
    QMessageBox::information(pai_, pai_->tr("Informação"), pai_->tr(mensagem.c_str()));
  }
  funcao_volta();
}

void InterfaceGraficaQt::EscolheItemLista(
    const std::string& titulo,
    const std::optional<std::string>& rotulo_ok,
    const std::vector<std::string>& lista,
    std::function<void(bool, int)> funcao_volta) {
  // Popula.
  ifg::qt::Ui::ListaPaginada gerador;
  auto* dialogo = new QDialog(pai_);
  gerador.setupUi(dialogo);
  dialogo->setWindowTitle(QString::fromUtf8(titulo.c_str()));
  for (const auto& item_str : lista) {
    auto* item = new QListWidgetItem(gerador.lista);
    auto* label = new QLabel(QString::fromUtf8(item_str.c_str()));
    label->setTextFormat(Qt::RichText);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setOpenExternalLinks(true);
    gerador.lista->setItemWidget(item, label);
  }
  gerador.lista->setFocus();
  auto lambda_aceito = [this, &gerador, dialogo, lista, funcao_volta] () {
    funcao_volta(gerador.lista->currentRow() >= 0 && gerador.lista->currentRow() < (int)lista.size(), gerador.lista->currentRow());
    dialogo->accept();
  };
  auto lambda_rejeitado = [this, &gerador, dialogo, lista, funcao_volta] () {
    funcao_volta(false, -1);
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
}

void InterfaceGraficaQt::EscolheItemsLista(
    const std::string& titulo,
    const std::vector<std::string>& lista,
    std::function<void(bool, std::vector<int>)> funcao_volta) {
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
    funcao_volta(!selecionados.empty(), selecionados);
    dialogo->accept();
  };
  auto lambda_rejeitado = [this, &gerador, dialogo, lista, funcao_volta] () {
    funcao_volta(false, {});
    dialogo->reject();
  };

  lambda_connect(gerador.lista, SIGNAL(itemDoubleClicked(QListWidgetItem*)), lambda_aceito);
  lambda_connect(gerador.botoes, SIGNAL(accepted()), lambda_aceito);
  lambda_connect(gerador.botoes, SIGNAL(rejected()), lambda_rejeitado);
  dialogo->exec();
  delete dialogo;
}

void InterfaceGraficaQt::EscolheArquivoAbrirTabuleiro(
    const std::vector<std::string>& tab_estaticos,
    const std::vector<std::string>& tab_dinamicos,
    std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta) {
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
      if (static_cast<unsigned int>(indice - 1) < tab_estaticos.size()) {
        funcao_volta(tab_estaticos[indice - 1], arq::TIPO_TABULEIRO_ESTATICO);
      } else {
        funcao_volta(tab_dinamicos[indice - 2 - tab_estaticos.size()], arq::TIPO_TABULEIRO);
      }
      dialogo->accept();
    }
  };
  auto lambda_rejeitado = [this, &gerador, dialogo, tab_estaticos, tab_dinamicos, funcao_volta] () {
    funcao_volta("", arq::TIPO_TABULEIRO);
    dialogo->reject();
  };
  lambda_connect(gerador.lista, SIGNAL(itemDoubleClicked(QListWidgetItem*)), lambda_aceito);
  lambda_connect(gerador.botoes, SIGNAL(accepted()), lambda_aceito);
  lambda_connect(gerador.botoes, SIGNAL(rejected()), lambda_rejeitado);
  dialogo->exec();
  delete dialogo;
}

void InterfaceGraficaQt::EscolheArquivoAbrirImagem(
  const std::vector<std::string>& imagens, std::function<void(const std::string& nome)> funcao_volta) {
  // Popula.
  ifg::qt::Ui::ListaPaginada gerador;
  auto* dialogo = new QDialog(pai_);
  gerador.setupUi(dialogo);
  for (const auto& imagem : imagens) {
    new QListWidgetItem(imagem.c_str(), gerador.lista);
  }
  gerador.lista->setFocus();
  auto lambda_aceito = [this, &gerador, dialogo, imagens, funcao_volta]() {
    int indice = gerador.lista->currentRow();
    int tamanho_total = imagens.size();
    if (indice >= 0 && indice < tamanho_total) {
      funcao_volta(imagens[indice]);
      dialogo->accept();
    }
    };
  auto lambda_rejeitado = [this, &gerador, dialogo, imagens, funcao_volta]() {
    funcao_volta("");
    dialogo->reject();
    };
  lambda_connect(gerador.lista, SIGNAL(itemDoubleClicked(QListWidgetItem*)), lambda_aceito);
  lambda_connect(gerador.botoes, SIGNAL(accepted()), lambda_aceito);
  lambda_connect(gerador.botoes, SIGNAL(rejected()), lambda_rejeitado);
  dialogo->exec();
  delete dialogo;
}

void InterfaceGraficaQt::EscolheArquivoSalvarTabuleiro(
    std::function<void(const std::string& nome)> funcao_volta) {
  ifg::qt::Ui::EntradaString gerador;
  auto* dialogo = new QDialog(pai_);
  gerador.setupUi(dialogo);
  gerador.label_titulo->setText(QDialog::tr("Salvar Tabuleiro"));
  gerador.nome->setFocus();
  auto lambda_aceito = [this, &gerador, dialogo, funcao_volta] () {
    if (gerador.nome->text().length() > 0) {
      funcao_volta(gerador.nome->text().toUtf8().constData());
      dialogo->accept();
    }
  };
  lambda_connect(gerador.botoes, SIGNAL(accepted()), lambda_aceito);
  QObject::connect(gerador.botoes, SIGNAL(rejected()), dialogo, SLOT(reject()));
  dialogo->exec();
  delete dialogo;
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
    int indice = gerador.lista->currentRow();
    if (indice > 0) {
      auto it = ids_itens.begin();
      // -1 por causa do label adicionado.
      std::advance(it, indice - 1);
      funcao_volta(*it);
    } else {
      funcao_volta("");
    }
    //LOG(INFO) << "aceitei: " << (indice > 0 ? modelos.begin() + (indice - 1) : "nada");
    dialogo->accept();
  };
  lambda_connect(gerador.lista, SIGNAL(itemDoubleClicked(QListWidgetItem*)), lambda_aceito);
  lambda_connect(gerador.botoes, SIGNAL(accepted()), lambda_aceito);
  QObject::connect(gerador.botoes, SIGNAL(rejected()), dialogo, SLOT(reject()));
  dialogo->exec();
  delete dialogo;
}

void InterfaceGraficaQt::EscolheCor(
    float r, float g, float b, float a, 
    std::function<void(bool, float, float, float, float)> funcao_volta) {
  QColor cor_ida(r, g, b);
  QColor cor = QColorDialog::getColor(cor_ida, pai_, QObject::tr("Escolha Cor"));
  if (!cor.isValid()) {
    funcao_volta(false, r, g, b, a);
  } else {
    funcao_volta(true, cor.redF(), cor.greenF(), cor.blueF(), cor.alphaF());
  }
}

}  // namespace qt
}  // namespace ent
