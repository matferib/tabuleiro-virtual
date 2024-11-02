#ifndef IFG_QT_DIALOGO_INIMIGO_PREDILETO_UTIL_H
#define IFG_QT_DIALOGO_INIMIGO_PREDILETO_UTIL_H

#include <QtWidgets/QHeaderView>
#include <QtWidgets/QComboBox>
#include "ent/comum.pb.h"
#include "ent/entidade.pb.h"
#include "ifg/qt/util.h"
#include "log/log.h"

namespace ifg {
namespace qt {

// Modelo de talento para ser usado pelos views de tabela.
class ModeloInimigoPredileto : public QAbstractTableModel {
 public:
  using InimigoPredileto = ent::InimigoPredileto;

  ModeloInimigoPredileto(const ent::Tabelas& tabelas, const ent::EntidadeProto& proto, QTableView* tabela)
      : QAbstractTableModel(tabela)/*, tabelas_(tabelas)*/ {
    for (const auto& ip : proto.dados_ataque_global().inimigos_prediletos()) {
      modelo_.push_back(ip);
    }
  }

  // Numero de linhas da tabela.
  int rowCount(const QModelIndex& parent =  QModelIndex()) const override {
    return modelo_.size();
  }

  // 0: id talento. 1. complemento. 2. geral.
  int columnCount(const QModelIndex& parent = QModelIndex()) const override {
    return 4;
  }

  bool insertRows(int row, int count, const QModelIndex& parent) override {
    if (count != 1) return false;
    beginInsertRows(parent, 0, 0);
    modelo_.insert(modelo_.begin() + row, count, InimigoPredileto());
    endInsertRows();
    return true;
  }

  bool removeRows(int row, int count, const QModelIndex& parent) override {
    beginRemoveRows(parent, row, row + count - 1);
    modelo_.erase(modelo_.begin() + row, modelo_.begin() + row + count);
    endRemoveRows();
    emit dataChanged(parent, parent);
    return true;
  }

  // Os cabeçalhos.
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
    if (orientation == Qt::Vertical) {
      return QVariant();
    }
    if (role == Qt::DisplayRole) {
      switch (section) {
        case 0: return QVariant(QString::fromUtf8("Classe"));
        case 1: return QVariant(QString::fromUtf8("Vezes"));
        case 2: return QVariant(QString::fromUtf8("Tipo"));
        case 3: return QVariant(QString::fromUtf8("Sub-Tipo"));
        default: ;
      }
    }
    return QVariant();
  }

  // Dado de cada celula.
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
    if (role != Qt::DisplayRole && role != Qt::EditRole) {
      return QVariant();
    }

    unsigned int row = index.row();
    if (row >= modelo_.size()) return QVariant();
    const int column = index.column();
    switch (column) {
      case 0: return QVariant(modelo_[row].classe().c_str());
      case 1: return QVariant(modelo_[row].vezes());
	    case 2: return role == Qt::EditRole
        ? QVariant(modelo_[row].has_tipo() ? modelo_[row].has_tipo() : -1)
        : QVariant(QString::fromUtf8(modelo_[row].has_tipo() ? ent::TipoDnD_Name(modelo_[row].tipo()).c_str() : ""));
      case 3: return role == Qt::EditRole
        ? QVariant(modelo_[row].has_sub_tipo() ? modelo_[row].sub_tipo() : -1)
        : QVariant(QString::fromUtf8(modelo_[row].has_sub_tipo() ? ent::SubTipoDnD_Name(modelo_[row].sub_tipo()).c_str() : ""));
      default: ;
    }
    // Nunca deveria chegar aqui.
    LOG(INFO) << "Coluna invalida: " << column;
    return QVariant();
  }

  bool setData(const QModelIndex& index, const QVariant& value, int role) override {
    if (role != Qt::EditRole) {
      return false;
    }

    const unsigned int row = index.row();
    if (row >= modelo_.size()) {
      LOG(INFO) << "Linha invalida " << row;
      return false;
    }
    const int column = index.column();
    switch (column) {
      case 0: {
        modelo_[row].set_classe(value.toString().toUtf8().constData());
        emit dataChanged(index, index);
        return true;
      }
      case 1: {
        modelo_[row].set_vezes(value.toInt());
        emit dataChanged(index, index);
        return true;
      }
      case 2: {
        int t = value.toInt();
        if (!ent::TipoDnD_IsValid(t)) {
          modelo_[row].clear_tipo();
        } else {
          modelo_[row].set_tipo(ent::TipoDnD(t));
        }
        emit dataChanged(index, index);
        return true;
      }
      case 3: {
        int st = value.toInt();
        if (!ent::SubTipoDnD_IsValid(st)) {
          modelo_[row].clear_sub_tipo();
        } else {
          modelo_[row].set_sub_tipo(ent::SubTipoDnD(st));
        }
        emit dataChanged(index, index);
        return true;
      }
      default: ;
    }
    return false;
  }

  Qt::ItemFlags flags(const QModelIndex & index) const override {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
  }

  google::protobuf::RepeatedPtrField<InimigoPredileto> Converte() const {
    google::protobuf::RepeatedPtrField<InimigoPredileto> ret;
    std::copy_if(modelo_.begin(), modelo_.end(), google::protobuf::RepeatedPtrFieldBackInserter(&ret), [](const InimigoPredileto& ip) {
      return ip.has_tipo();
    });
    return ret;
  }

 private:
  //const ent::Tabelas& tabelas_;
  std::vector<InimigoPredileto> modelo_;
};

// Classe base para delegados de enumeracao.
class TipoSubTipoDelegateBase : public QItemDelegate {
 public:
  TipoSubTipoDelegateBase(const ent::Tabelas& tabelas, QAbstractTableModel* modelo, QObject* parent)
      : QItemDelegate(), /*tabelas_(tabelas),*/ modelo_(modelo) {}


  QWidget* createEditor(
      QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    return PreencheConfiguraCombo(CriaMapa(), new QComboBox(parent));
  }

  // Escreve o valor do combo.
  void setEditorData(QWidget* editor, const QModelIndex& index) const override {
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (combo == nullptr) {
      QItemDelegate::setEditorData(editor, index);
      return;
    }
    const QVariant& data = modelo_->data(index, Qt::EditRole);
    combo->setCurrentIndex(combo->findData(data));
  }

  // Salva o valor do combo no modelo.
  void setModelData(
      QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (combo == nullptr) {
      QItemDelegate::setModelData(editor, model, index);
      return;
    }
    modelo_->setData(index, combo->itemData(combo->currentIndex()), Qt::EditRole);
  }

 protected:
  virtual std::map<std::string, int> CriaMapa() const = 0;

 private:
  void commitAndCloseEditor(QComboBox* combo) {
    emit commitData(combo);
    emit closeEditor(combo);
  }

  // Preenche o combo box de bonus.
  QComboBox* PreencheConfiguraCombo(const std::map<std::string, int>& mapa, QComboBox* combo) const {
    for (const auto& kv : mapa) {
      combo->addItem(QString::fromUtf8(kv.first.c_str()), QVariant(kv.second));
    }
    ExpandeComboBox(combo);

    //connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(commitAndCloseEditor()));
    lambda_connect(combo, SIGNAL(currentIndexChanged(int)), [this, combo]() {
      auto* thiz = const_cast<TipoSubTipoDelegateBase*>(this);
      thiz->commitAndCloseEditor(combo);
    });
    return combo;
  }

  //const ent::Tabelas& tabelas_;
  QAbstractTableModel* modelo_;
};

// Delegado de tipo.
class TipoDnDDelegate : public TipoSubTipoDelegateBase {
 public:
  TipoDnDDelegate(const ent::Tabelas& tabelas,  QAbstractTableModel* modelo, QObject* parent)
      : TipoSubTipoDelegateBase(tabelas, modelo, parent) {}

 protected:
  virtual std::map<std::string, int> CriaMapa() const override {
    std::map<std::string, int> mapa;
    mapa.insert(std::make_pair("Nenhum", -1));
    for (int i = ent::TipoDnD_MIN; i < ent::TipoDnD_MAX; ++i) {
      if (!ent::TipoDnD_IsValid(i)) continue;
      mapa.insert(std::make_pair(ent::TipoDnD_Name((ent::TipoDnD)i), i));
    }
    return mapa;
  }
};

// Delegado de subtipo.
class SubTipoDnDDelegate : public TipoSubTipoDelegateBase {
 public:
  SubTipoDnDDelegate(const ent::Tabelas& tabelas, QAbstractTableModel* modelo, QObject* parent)
      : TipoSubTipoDelegateBase(tabelas, modelo, parent) {}

 protected:
  virtual std::map<std::string, int> CriaMapa() const override {
    std::map<std::string, int> mapa;
    mapa.insert(std::make_pair("Nenhum", -1));
    for (int i = ent::SubTipoDnD_MIN; i < ent::SubTipoDnD_MAX; ++i) {
      if (!ent::SubTipoDnD_IsValid(i)) continue;
      mapa.insert(std::make_pair(ent::SubTipoDnD_Name((ent::SubTipoDnD)i), i));
    }
    return mapa; 
  }
};

}  // namespace qt
}  // namespace ifg

#endif
