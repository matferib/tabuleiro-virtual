#include <set>
#include <stack>

#include <QBoxLayout>
#include <QColorDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QString>

#include "arq/arquivo.h"
#include "ifg/modelos.pb.h"
#include "ifg/qt/qt_interface.h"
#include "ifg/qt/ui/entradastring.h"
#include "ifg/qt/ui/listapaginada.h"
#include "ifg/qt/util.h"
#include "log/log.h"

namespace ifg {
namespace qt {

void InterfaceGraficaQt::MostraMensagem(
    bool erro, const std::string& mensagem, std::function<void()> funcao_volta) {
  if (erro) {
    QMessageBox::warning(pai_, pai_->tr("Erro"), pai_->tr(mensagem.c_str()));
  } else {
    QMessageBox::information(pai_, pai_->tr("Informação"), pai_->tr(mensagem.c_str()));
  }
  funcao_volta();
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
  lambda_connect(gerador.lista, SIGNAL(itemDoubleClicked(QListWidgetItem*)), lambda_aceito);
  lambda_connect(gerador.botoes, SIGNAL(accepted()), lambda_aceito);
  QObject::connect(gerador.botoes, SIGNAL(rejected()), dialogo, SLOT(reject()));
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
      funcao_volta(gerador.nome->text().toStdString());
      dialogo->accept();
    }
  };
  lambda_connect(gerador.botoes, SIGNAL(accepted()), lambda_aceito);
  QObject::connect(gerador.botoes, SIGNAL(rejected()), dialogo, SLOT(reject()));
  dialogo->exec();
  delete dialogo;
}

namespace {

std::set<std::string> ExtraiModelos(const MenuModelos& menu_modelos) {
  std::stack<const MenuModelos*> menus;
  menus.push(&menu_modelos);
  std::set<std::string> ret;
  do {
    const auto* menu = menus.top();
    for (const auto& modelo : menu->modelo()) {
      // ATENCAO: Se o texto for usado, deve-se manter um mapeamento de texto->id porque o callback usa o id.
      ret.insert(modelo.id());
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
  std::set<std::string> modelos = ExtraiModelos(menu_modelos);
  for (const auto& nome : modelos) {
    new QListWidgetItem(pai_->tr(nome.c_str()), gerador.lista);
  }
  gerador.lista->setFocus();
  auto lambda_aceito = [this, &gerador, dialogo, modelos, funcao_volta] () {
    int indice = gerador.lista->currentRow();
    // -1 por causa do label adicionado.
    if (indice > 1) {
      auto it = modelos.begin();
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

