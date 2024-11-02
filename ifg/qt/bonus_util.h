#ifndef IFG_QT_DIALOGO_BONUS_UTIL_H
#define IFG_QT_DIALOGO_BONUS_UTIL_H

#include <QtWidgets/QComboBox>
#include <QtWidgets/QItemDelegate>
#include "ifg/qt/ui/dialogobonus.h"
#include "ent/entidade.pb.h"
#include "ent/util.h"
#include "ifg/qt/util.h"
#include "log/log.h"

namespace ifg {
namespace qt {

void AbreDialogoBonus(QWidget* pai, ent::Bonus* bonus);

inline std::string NomeBonus(ent::TipoBonus tb) {
  return ent::TipoBonus_Name(tb).substr(3);
}

// Modelo de bonus para ser usado pelos views de tabela.
class ModeloBonus : public QAbstractTableModel {
 public:
  ModeloBonus(const ent::Bonus& bonus, QTableView* tabela)
      : QAbstractTableModel(tabela), modelo_(BonusParaModelo(bonus)), tabela_(tabela) {}
  ~ModeloBonus() override;

  // Numero de linhas da tabela.
  int rowCount(const QModelIndex& parent =  QModelIndex()) const override {
    return modelo_.size();
  }

  // 0: tipo. 1: origem. 2: valor.
  int columnCount(const QModelIndex& parent = QModelIndex()) const override {
    return 3;
  }

  bool insertRows(int row, int count, const QModelIndex& parent) override {
    if (count != 1) return false;
    beginInsertRows(parent, 0, 0);
    modelo_.insert(modelo_.begin() + row, Linha());
    endInsertRows();
    return true;
  }

  bool removeRows(int row, int count, const QModelIndex& parent) override {
    beginRemoveRows(parent, row, row + count - 1);
    modelo_.erase(modelo_.begin() + row, modelo_.begin() + row + count);
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

    ent::TipoBonus tipo;
    std::string origem;
    int valor;
    std::tie(tipo, origem, valor) = DadosEm(index);
    const int column = index.column();
    switch (column) {
      case 0: return role == Qt::EditRole ? QVariant() : QVariant(NomeBonus(tipo).c_str());
      case 1: return QVariant(QString::fromUtf8(origem.c_str()));
      case 2: return QVariant(valor);
    }
    // Nunca deveria chegar aqui.
    LOG(INFO) << "Coluna invalida: " << column;
    return QVariant();
  }

  bool setData(const QModelIndex& index, const QVariant& value, int role) override {
    if (role != Qt::EditRole) {
      return false;
    }
    const int linha = index.row();
    if (linha < 0 || linha >= (int)modelo_.size()) {
      return false;
    }
    const int coluna = index.column();
    if (coluna < 0 || (int)coluna > 2) {
      return false;
    }

    switch (coluna) {
      case 0: {
        if (!ent::TipoBonus_IsValid(value.toInt())) {
          LOG(INFO) << "Tipo de bonus invalido: " << value.toInt();
          return false;
        }
        modelo_[linha].tipo = static_cast<ent::TipoBonus>(value.toInt());
        tabela_->setIndexWidget(index, nullptr);
        return true;
      }
      case 1: {
        modelo_[linha].origem = value.toString().toUtf8().constData();
        return true;
      }
      case 2: {
        bool ok = false;
        int valor = value.toInt(&ok);
        if (!ok) return false;
        modelo_[linha].valor = valor;
        return true;
      }
    }
    return false;
  }

  Qt::ItemFlags flags(const QModelIndex & index) const override {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
  }

  ent::Bonus ModeloParaBonus() const {
    ent::Bonus bonus;
    for (const auto& linha : modelo_) {
      ent::AtribuiBonus(linha.valor, linha.tipo, linha.origem, &bonus);
    }
    return bonus;
  }

 private:
  // Cada origem sera transformada em uma linha.
  struct Linha {
    Linha(ent::TipoBonus tipo, const ent::BonusIndividual::PorOrigem& po) : tipo(tipo), origem(po.origem()), valor(po.valor()) {}
    Linha() : tipo(ent::TB_SEM_NOME), valor(0) {}

    ent::TipoBonus tipo;
    std::string origem;
    int valor;
  };

  std::vector<Linha> BonusParaModelo(const ent::Bonus& bonus) const {
    std::vector<Linha> modelo;
    for (const auto& bi : bonus.bonus_individual()) {
      for (const auto& po : bi.por_origem()) {
        modelo.emplace_back(bi.tipo(), po);
      }
    }
    return modelo;
  }

  std::tuple<ent::TipoBonus, std::string, int> DadosEm(int linha) const {
    if (linha < 0 || linha >= (int)modelo_.size()) return std::make_tuple(ent::TB_SEM_NOME, "", 0);
    const auto& linha_modelo = modelo_[linha];
    return std::make_tuple(linha_modelo.tipo, linha_modelo.origem, linha_modelo.valor);
  }

  std::tuple<ent::TipoBonus, std::string, int> DadosEm(const QModelIndex& index) const {
    return DadosEm(index.row());
  }

 private:
  // O bonus eh convertido para modelo, que na volta eh convertido para bonus.
  std::vector<Linha> modelo_;
  QTableView* tabela_;
};

// Responsavel por tratar a edicao do tipo de bonus.
class TipoBonusDelegate : public QItemDelegate {
 public:
  TipoBonusDelegate(QTableView* tabela, ModeloBonus* modelo, QObject* parent)
      : QItemDelegate(), tabela_(tabela), modelo_(modelo) {
  }
  ~TipoBonusDelegate() override;

  QWidget* createEditor(
      QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    return PreencheConfiguraComboBonus(new QComboBox(parent));
  }

  void setEditorData(QWidget* editor, const QModelIndex& index) const override {
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (combo == nullptr) {
      LOG(INFO) << "combo == nullptr em setEditorData";
      return;
    }
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

 private slots:
  void commitAndCloseEditor() {
    QComboBox* combo = qobject_cast<QComboBox*>(sender());
    setModelData(combo, modelo_, combo->rootModelIndex());
    emit commitData(combo);
    emit closeEditor(combo);
  }

 private:
  // Preenche o combo box de bonus. Retorna o combo.
  QComboBox* PreencheConfiguraComboBonus(QComboBox* combo) const {
    for (int tipo = ent::TipoBonus_MIN; tipo <= ent::TipoBonus_MAX; tipo++) {
      if (!ent::TipoBonus_IsValid(tipo)) continue;
      combo->addItem(NomeBonus(static_cast<ent::TipoBonus>(tipo)).c_str(), QVariant(tipo));
    }
    combo->setCurrentIndex(0);
    ExpandeComboBox(combo);
    connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(commiAndCloseEditor()));
    return combo;
  }

  QTableView* tabela_;
  ModeloBonus* modelo_;
};

}  // namespace qt
}  // namespace ifg

#endif
