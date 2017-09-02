#ifndef IFG_QT_DIALOGO_PERICIAS_UTIL_H
#define IFG_QT_DIALOGO_PERICIAS_UTIL_H

#include <QComboBox>
#include "ent/entidade.pb.h"
#include "ent/tabelas.h"
#include "ifg/qt/util.h"
#include "log/log.h"

namespace ifg {
namespace qt {

// Modelo de talento para ser usado pelos views de tabela.
class ModeloPericias : public QAbstractTableModel {
 public:
  using InfoPericia = ent::EntidadeProto::InfoPericia;

  ModeloPericias(const ent::Tabelas& tabelas, google::protobuf::RepeatedPtrField<InfoPericia>* pericias, QTableView* tabela)
      : QAbstractTableModel(tabela), tabelas_(tabelas) {
    std::unordered_map<std::string, const ent::PericiaProto*> mapa_pericias;
    for (const auto& pp : tabelas.todas().tabela_pericias().pericias()) {
      mapa_pericias.insert(std::make_pair(pp.nome(), &pp));
    }
    for (const auto& par_nome_pericia : mapa_pericias) {
      const auto& pp = *par_nome_pericia.second;
      InfoPericia pericia;
      pericia.set_id(pp.id());
      modelo_.push_back(pericia);
    }
  }

  // Numero de linhas da tabela.
  int rowCount(const QModelIndex& parent =  QModelIndex()) const override {
    return modelo_.size();
  }

  // 0: id pericia. 1. Ranks. 2. De classe. 3. Complemento. 4. Bonus.
  int columnCount(const QModelIndex& parent = QModelIndex()) const override {
    return 5;
  }

  bool insertRows(int row, int count, const QModelIndex& parent) override {
    if (count != 1) return false;
    beginInsertRows(parent, 0, 0);
    modelo_.insert(modelo_.begin() + row, count, InfoPericia());
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
      case 0: return QVariant(QString::fromUtf8("Perícia"));
      case 1: return QVariant(QString::fromUtf8("Pontos"));
      case 2: return QVariant(QString::fromUtf8("De Classe"));
      case 3: return QVariant(QString::fromUtf8("Complemento"));
      case 4: return QVariant(QString::fromUtf8("Bônus Total"));
    }
    return QVariant("Desconhecido");
  }

  // Dado de cada celula.
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
    if (role != Qt::DisplayRole && role != Qt::EditRole) {
      return QVariant();
    }

    const int row = index.row();
    if (row < 0 || row >= modelo_.size()) return QVariant();
    const int column = index.column();
    switch (column) {
      case 0:
        if (role == Qt::DisplayRole) {
          return QVariant(QString::fromUtf8(tabelas_.Pericia(modelo_[row].id()).nome().c_str()));
        } else {
          // Nao editavel.
          return QVariant();
        }
      case 1: return QVariant(modelo_[row].pontos());
      case 2: return QVariant(false);
      case 3: return QVariant(QString::fromUtf8(modelo_[row].complemento().c_str()));
      case 4: {
        if (role == Qt::DisplayRole) {
          return QVariant(BonusTotal(modelo_[row].bonus()));
        } else {
          // Nao editavel.
          return QVariant();
        }
      }
    }
    // Nunca deveria chegar aqui.
    LOG(INFO) << "Coluna invalida: " << column;
    return QVariant();
  }

  bool setData(const QModelIndex& index, const QVariant& value, int role) override {
    return false;
    if (role != Qt::EditRole) {
      return false;
    }

    const int row = index.row();
    if (row < 0 || row >= modelo_.size()) {
      LOG(INFO) << "Linha invalida " << row;
      return false;
    }
    const int column = index.column();
    switch (column) {
      case 0: {
        emit dataChanged(index, index);
        return true;
      }
      case 1: {
        emit dataChanged(index, index);
        return true;
      }
      case 2: {
        emit dataChanged(index, index);
        return true;
      }
      case 3: {
        emit dataChanged(index, index);
        return true;
      }
      case 4: {
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

  google::protobuf::RepeatedPtrField<InfoPericia> Converte() const {
    google::protobuf::RepeatedPtrField<InfoPericia> ret(modelo_.begin(), modelo_.end());
    return ret;
  }

 private:
  const ent::Tabelas& tabelas_;
  std::vector<InfoPericia> modelo_;
};

#if 0
class ComplementoTalentoDelegate : public QItemDelegate {
 public:
  ComplementoTalentoDelegate(const ent::Tabelas& tabelas, QAbstractTableModel* modelo, QObject* parent)
      : QItemDelegate(), tabelas_(tabelas), modelo_(modelo) {}

  QWidget* createEditor(
      QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    QVariant valor = modelo_->data(index.child(index.row(), 0), Qt::EditRole);
    const auto& talento = tabelas_.Talento(valor.toString().toStdString());
    switch (talento.tipo_complemento()) {
      case ent::TCT_ARMA_COMUM:
      case ent::TCT_ARMA_EXOTICA:
      case ent::TCT_ARMA: {
        std::map<std::string, std::string> mapa;
        for (const auto& a : tabelas_.todas().tabela_armas().armas()) {
          if ((talento.tipo_complemento() == ent::TCT_ARMA_COMUM && a.categoria_pericia() != ent::CATPER_COMUM) ||
              (talento.tipo_complemento() == ent::TCT_ARMA_EXOTICA && a.categoria_pericia() != ent::CATPER_EXOTICA)) {
            continue;
          }
          mapa.insert(std::make_pair(a.nome(), a.id()));
        }
        return PreencheConfiguraCombo(mapa, new QComboBox(parent));
      }
      case ent::TCT_ESCOLA_MAGIA: {
        std::map<std::string, std::string> mapa = {
          {"Abjuração", "abjuracao"},
          {"Adivinhação", "adivinhacao"},
          {"Conjuração", "conjuracao"},
          {"Encantamento", "encantamento"},
          {"Evocação", "evocacao"},
          {"Ilusão", "ilusao"},
          {"Necromancia", "necromancia"},
          {"Transmutação", "transmutacao"},
        };
        return PreencheConfiguraCombo(mapa, new QComboBox(parent));
      }
      case ent::TCT_PERICIA:
        // TODO
      case ent::TCT_NENHUM:
      default:
        return QItemDelegate::createEditor(parent, option, index);
    }
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

 private:
  void commitAndCloseEditor(QComboBox* combo) {
    emit commitData(combo);
    emit closeEditor(combo);
  }

  // Preenche o combo box de bonus.
  QComboBox* PreencheConfiguraCombo(const std::map<std::string, std::string>& mapa, QComboBox* combo) const {
    for (const auto& kv : mapa) {
      combo->addItem(QString::fromUtf8(kv.first.c_str()), QVariant(kv.second.c_str()));
    }

    //connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(commitAndCloseEditor()));
    lambda_connect(combo, SIGNAL(currentIndexChanged(int)), [this, combo]() {
      auto* thiz = const_cast<ComplementoTalentoDelegate*>(this);
      thiz->commitAndCloseEditor(combo);
    });
    return combo;
  }

  const ent::Tabelas& tabelas_;
  QAbstractTableModel* modelo_;
};
#endif

}  // namespace qt
}  // namespace ifg

#endif
