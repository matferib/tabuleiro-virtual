#ifndef IFG_QT_DIALOGO_BONUS_UTIL_H
#define IFG_QT_DIALOGO_BONUS_UTIL_H

#include <QComboBox>
#include <QItemDelegate>
#include "ifg/qt/ui/dialogobonus.h"
#include "ent/entidade.pb.h"
#include "ent/util.h"
#include "ifg/qt/util.h"
#include "log/log.h"

namespace ifg {
namespace qt {

void AbreDialogoBonus(QWidget* pai, ent::Bonus* bonus);

// Modelo de bonus para ser usado pelos views de tabela.
class ModeloBonus : public QAbstractTableModel {
 public:
  ModeloBonus(const ent::Bonus& bonus, QTableView* tabela)
      : QAbstractTableModel(tabela), tabela_(tabela), bonus_(bonus) {}

  // Numero de linhas da tabela.
  int rowCount(const QModelIndex& parent =  QModelIndex()) const override {
    int total = 0;
    for (const auto& bi : bonus_.bonus_individual()) {
      total += bi.por_origem_size();
    }
    return total;
  }

  // 0: tipo. 1: origem. 2: valor.
  int columnCount(const QModelIndex& parent = QModelIndex()) const override {
    return 3;
  }

  bool insertRows(int row, int count, const QModelIndex& parent) override {
    if (count != 1) return false;
    beginInsertRows(parent, 0, 0);
    ent::AtribuiBonus(0, ent::TB_BASE, "origem", &bonus_);
    endInsertRows();
    return true;
  }

  bool removeRows(int row, int count, const QModelIndex& parent) override {
    beginRemoveRows(parent, row, row + count - 1);
    std::vector<std::pair<ent::BonusIndividual*, ent::BonusIndividual_PorOrigem*>> bis_pos(count);
    while (count--) {
      auto& bi_po = bis_pos[count];
      std::tie(bi_po.first, bi_po.second) = DadosEm(row);
    }
    for (const auto& bi_po : bis_pos) {
      RemoveBonus(bi_po.first->tipo(), bi_po.second->origem(), &bonus_);
    }
    endRemoveRows();
    return true;
  }

  // Os cabeçalhos.
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
    if (orientation == Qt::Vertical || role != Qt::DisplayRole) {
      return QVariant();
    }
    switch (section) {
      case 0: return QVariant("Tipo");
      case 1: return QVariant("Origem");
      case 2: return QVariant("Valor");
    }
    return QVariant("Desconhecido");
  }

  // Dado de cada celula.
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
    if (role != Qt::DisplayRole && role != Qt::EditRole) {
      return QVariant();
    }

    const ent::BonusIndividual* bi;
    const ent::BonusIndividual_PorOrigem* po;
    std::tie(bi, po) = DadosEm(index);
    if (bi == nullptr || po == nullptr) {
      LOG(INFO) << "bi == nullptr?  " << (bi == nullptr ? "YES" : "NO")
                << ", po == nullptr? " << (po == nullptr ? "YES" : "NO");
      return QVariant();
    }
    const int column = index.column();
    switch (column) {
      case 0: return role == Qt::EditRole ? QVariant() : QVariant(ent::TipoBonus_Name(bi->tipo()).c_str());
      case 1: return QVariant(po->origem().c_str());
      case 2: return QVariant(po->valor());
    }
    // Nunca deveria chegar aqui.
    LOG(INFO) << "Coluna invalida: " << column;
    return QVariant();
  }

  bool setData(const QModelIndex& index, const QVariant& value, int role) override {
    if (role != Qt::EditRole) {
      return false;
    }

    ent::BonusIndividual* bi = nullptr;
    ent::BonusIndividual_PorOrigem* po = nullptr;
    std::tie(bi, po) = DadosEm(index);
    if (bi == nullptr || po == nullptr) {
      LOG(INFO) << "bi == nullptr?  " << (bi == nullptr ? "YES" : "NO")
                << ", po == nullptr? " << (po == nullptr ? "YES" : "NO");
      return false;
    }
    const int column = index.column();
    switch (column) {
      case 0: {
        int tipo = value.toInt();
        if (!ent::TipoBonus_IsValid(tipo)) {
          LOG(INFO) << "Tipo de bonus invalido: " << tipo;
          return false;
        }
        if (tipo == bi->tipo()) {
          LOG(INFO) << "Sem mudanca de tipo: " << value.toString().toUtf8().constData();
          return false;
        }
        // Adiciona nova origem.
        ent::AtribuiBonus(po->valor(), ent::TipoBonus(tipo), po->origem(), &bonus_);
        // Remove a origem do tipo corrente.
        RemoveBonus(bi->tipo(), po->origem(), &bonus_);
        LOG(INFO) << "novo proto: " << bonus_.DebugString();
        tabela_->setIndexWidget(index, nullptr);
        return true;
      }
      case 1: {
        po->set_origem(value.toString().toUtf8().constData());
        return true;
      }
      case 2: {
        bool ok = false;
        int valor = value.toInt(&ok);
        if (!ok) return false;
        po->set_valor(valor);
        return true;
      }
    }
    return false;
  }

  Qt::ItemFlags flags(const QModelIndex & index) const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
  }

  const ent::Bonus Bonus() const { return bonus_; }

 private:
  std::tuple<ent::BonusIndividual*, ent::BonusIndividual_PorOrigem*> DadosEm(int row) {
    while (row >= 0) {
      for (auto& bi : *bonus_.mutable_bonus_individual()) {
        if (row < bi.por_origem_size()) {
          // achou o bi.
          return std::make_tuple(&bi, bi.mutable_por_origem(row));
        } else {
          row -= bi.por_origem_size();
        }
      }
    }
    return std::make_tuple(nullptr, nullptr);
  }

  std::tuple<ent::BonusIndividual*, ent::BonusIndividual_PorOrigem*> DadosEm(const QModelIndex& index) {
    return DadosEm(index.row());
  }

  std::tuple<const ent::BonusIndividual*, const ent::BonusIndividual_PorOrigem*> DadosEm(
      const QModelIndex& index) const {
    return DadosEm(index.row());
  }

  std::tuple<const ent::BonusIndividual*, const ent::BonusIndividual_PorOrigem*> DadosEm(int row) const {
    while (row >= 0) {
      for (auto& bi : bonus_.bonus_individual()) {
        if (row < bi.por_origem_size()) {
          // achou o bi.
          return std::make_tuple(&bi, &bi.por_origem(row));
        } else {
          row -= bi.por_origem_size();
        }
      }
    }
    return std::make_tuple(nullptr, nullptr);
  }

 private:
  QTableView* tabela_;
  ent::Bonus bonus_;
};

// Responsavel por tratar a edicao do tipo de bonus.
class TipoBonusDelegate : public QItemDelegate {
 public:
  TipoBonusDelegate(QTableView* tabela, ModeloBonus* modelo, QObject* parent)
      : QItemDelegate(), tabela_(tabela), modelo_(modelo) {}

  QWidget* createEditor(
      QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    QComboBox* combo = new QComboBox(parent);
    PreencheComboBonus(ent::TB_BASE, combo);
    return combo;
  }

  void setEditorData(QWidget* editor, const QModelIndex& index) const override {
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (combo == nullptr) {
      LOG(INFO) << "combo == nullptr em setEditorData";
      return;
    }
    lambda_connect(combo, SIGNAL(currentIndexChanged(int)), [this, combo, index] () {
      setModelData(combo, modelo_, index);
      emit closeEditor(combo);
    });
    QVariant data = modelo_->data(index);
    ent::TipoBonus tipo;
    if (!ent::TipoBonus_Parse(data.toString().toUtf8().constData(), &tipo)) {
      return;
    }
    combo->setCurrentIndex(tipo);
  }

  void setModelData(
      QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (combo == nullptr) {
      LOG(INFO) << "combo == nullptr em setEditorData";
      return;
    }
    modelo_->setData(index, combo->currentIndex(), Qt::EditRole);
    tabela_->reset();
  }

 private:
  // Preenche o combo box de bonus.
  void PreencheComboBonus(ent::TipoBonus tipo, QComboBox* combo) const {
    for (int tipo = ent::TipoBonus_MIN; tipo <= ent::TipoBonus_MAX; tipo++) {
      if (!ent::TipoBonus_IsValid(tipo)) continue;
      combo->addItem(ent::TipoBonus_Name(ent::TipoBonus(tipo)).c_str(), QVariant(tipo));
    }
    combo->setCurrentIndex(tipo);
  }

  QTableView* tabela_;
  ModeloBonus* modelo_;
  ent::TipoBonus tipo_;
};

}  // namespace qt
}  // namespace ifg

#endif
