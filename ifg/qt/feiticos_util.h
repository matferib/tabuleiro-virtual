#ifndef IFG_QT_DIALOGO_FEITICOS_UTIL_H
#define IFG_QT_DIALOGO_FEITICOS_UTIL_H

#include <QTreeWidgetItem>
#include "ent/entidade.pb.h"
#include "ent/tabelas.h"
#include "ent/util.h"
#include "ifg/qt/util.h"
#include "log/log.h"

namespace ifg {
namespace qt {

class ItemFeiticoConhecido : public QTreeWidgetItem {
 public:
  ItemFeiticoConhecido(const ent::Tabelas& tabelas, const std::string& id_classe, int nivel, QTreeWidgetItem* pai)
      : QTreeWidgetItem(pai, QTreeWidgetItem::UserType) {
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
    combo->addItem(QString::fromUtf8(""), QVariant(""));
    for (const auto& feitico : tabelas.Feiticos(ent::IdParaMagia(tabelas, id_classe), nivel)) {
      combo->addItem(QString::fromStdString(feitico->nome()), QVariant(QString::fromStdString(feitico->id())));
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
};

}  // namespace qt
}  // namespace ifg

#endif
