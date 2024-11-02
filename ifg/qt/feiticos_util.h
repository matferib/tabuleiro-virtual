#ifndef IFG_QT_DIALOGO_FEITICOS_UTIL_H
#define IFG_QT_DIALOGO_FEITICOS_UTIL_H

#include <QtWidgets/QTreeWidgetItem>
#include "absl/strings/str_format.h"
#include "ent/entidade.pb.h"
#include "ent/tabelas.h"
#include "ent/util.h"
#include "ifg/qt/util.h"
#include "log/log.h"

namespace ifg {
namespace qt {

// Item para conhecido (a barra com texto conhecido).
class ItemConhecidos : public QTreeWidgetItem {
 public:
  ItemConhecidos(const std::string& id_classe, int nivel, QTreeWidgetItem* pai)
      : QTreeWidgetItem(pai, QTreeWidgetItem::UserType) {
    auto* hwidget = new QWidget;
    auto* layout = new QHBoxLayout;
    QLabel* label = new QLabel("Conhecidos");
    auto* botao_mais = new QPushButton("+");
    lambda_connect(botao_mais, SIGNAL(clicked()), [this]() {
      emitDataChanged();
    });
    layout->addWidget(label, 0, Qt::AlignLeft);
    layout->addWidget(botao_mais, 0, Qt::AlignLeft);

    hwidget->setLayout(layout);
    treeWidget()->setItemWidget(this, 0, hwidget);
  }
};

// Item para feiticos conhecidos.
class ItemFeiticoConhecido : public QTreeWidgetItem {
 public:
  ItemFeiticoConhecido(
      const ent::Tabelas& tabelas, const std::string& id_classe, int nivel, const ent::EntidadeProto& proto,
      QTreeWidgetItem* pai)
      : QTreeWidgetItem(pai, QTreeWidgetItem::UserType), proto_(proto) {
    auto* hwidget = new QWidget;
    auto* hbox = new QHBoxLayout;
    linha_ = new QLineEdit();
    lambda_connect(linha_, SIGNAL(editingFinished()), [this] () {
      setIdNome("", linha_->text());
    });
    hbox->addWidget(linha_, 0, Qt::AlignLeft);
    hbox->addWidget(CriaComboConhecido(tabelas, id_classe, nivel, linha_));
    hwidget->setLayout(hbox);
    treeWidget()->setItemWidget(this, 0, hwidget);
  }

  void setIdNome(const QString &id, const QString& nome) {
    linha_->blockSignals(true);
    linha_->setText(nome);
    linha_->blockSignals(false);
    setData(TCOL_NOME_FEITICO, Qt::UserRole, QVariant(nome));
    setData(TCOL_ID_FEITICO, Qt::UserRole, QVariant(id));
    SelecionaItemCombo(id, combo_);
    emitDataChanged();
  }

 private:
  void PreencheComboConhecido(
      const ent::Tabelas& tabelas, const std::string& id_classe, int nivel, QComboBox* combo) {
    combo->clear();
    combo->addItem(QString::fromUtf8("Não tabelado"), QVariant(""));
    std::map<QString, std::string> feiticos_ordenados;
    for (const auto& feitico : tabelas.Feiticos(ent::IdParaMagia(tabelas, id_classe), nivel)) {
      feiticos_ordenados[QString::fromStdString(feitico->nome())] = feitico->id();
    }
    for (const auto& dominio : FeiticosClasse(id_classe, proto_).dominios()) {
      for (const auto& feitico : tabelas.Feiticos(dominio, nivel)) {
        feiticos_ordenados[QString::fromStdString(absl::StrFormat("%s (D)", feitico->nome().c_str()))] = feitico->id();
      }
    }

    // Adiciona no combo.
    for (auto par_nome_id : feiticos_ordenados) {
      combo->addItem(par_nome_id.first, QVariant(QString::fromStdString(par_nome_id.second)));
    }
  }

  QWidget* CriaComboConhecido(const ent::Tabelas& tabelas, const std::string& id_classe, int nivel, QLineEdit* linha) {
    combo_ = new QComboBox();
    combo_->setObjectName("combo_conhecido");
    PreencheComboConhecido(tabelas, id_classe, nivel, combo_);
    lambda_connect(combo_, SIGNAL(currentIndexChanged(int)), [this] (int indice) {
      QVariant data = combo_->currentData();
      this->setIdNome(data.toString(), combo_->currentText());
    });
    return combo_;
  }

  void SelecionaItemCombo(const QString& id, QComboBox* combo) {
    int index = combo->findData(QVariant(id));
    if (index == -1) {
      index = 0;
    }
    combo->blockSignals(true);
    combo->setCurrentIndex(index);
    combo->blockSignals(false);
  }

  QLineEdit* linha_ = nullptr;
  QComboBox* combo_ = nullptr;
  const ent::EntidadeProto& proto_;
};

}  // namespace qt
}  // namespace ifg

#endif
