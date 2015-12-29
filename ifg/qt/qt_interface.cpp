#include <QBoxLayout>
#include <QColorDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QListWidgetItem>
#include <QString>

#include "arq/arquivo.h"
#include "ifg/qt/qt_interface.h"
#include "ifg/qt/ui/listapaginada.h"
#include "ifg/qt/util.h"

namespace ifg {
namespace qt {

void InterfaceGraficaQt::EscolheArquivoAbrirTabuleiro(
    const std::vector<std::string>& tab_estaticos,
    const std::vector<std::string>& tab_dinamicos,
    std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta) {
  // Popula.
  ifg::qt::Ui::ListaPaginada gerador;
  auto* dialogo = new QDialog(pai_);
  gerador.setupUi(dialogo);
  auto* wie = new QListWidgetItem(pai_->tr("Estáticos"), gerador.lista);
  wie->setFlags(Qt::ItemIsUserCheckable);
  for (const auto& tab : tab_estaticos) {
    new QListWidgetItem(tab.c_str(), gerador.lista);
  }
  auto* wid = new QListWidgetItem(pai_->tr("Salvos"), gerador.lista);
  wid->setFlags(Qt::ItemIsUserCheckable);
  for (const auto& tab : tab_dinamicos) {
    new QListWidgetItem(tab.c_str(), gerador.lista);
  }
  auto lambda_aceito = [this, &gerador, dialogo, tab_estaticos, tab_dinamicos, funcao_volta] () {
    int indice = gerador.lista->currentRow();
    int tamanho_total = tab_estaticos.size() + tab_dinamicos.size();
    if (indice >= 0 && indice < tamanho_total) {
      if (static_cast<unsigned int>(indice) < tab_estaticos.size()) {
        funcao_volta(tab_estaticos[indice], arq::TIPO_TABULEIRO_ESTATICO);
      } else {
        funcao_volta(tab_dinamicos[indice - tab_estaticos.size()], arq::TIPO_TABULEIRO);
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
}



}  // namespace qt
}  // namespace ent

