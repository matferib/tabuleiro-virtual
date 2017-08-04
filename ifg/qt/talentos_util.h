#ifndef IFG_QT_DIALOGO_TALENTOS_UTIL_H
#define IFG_QT_DIALOGO_TALENTOS_UTIL_H

#include <QComboBox>
#include "ent/entidade.pb.h"
#include "ifg/qt/util.h"
#include "log/log.h"

namespace ifg {
namespace qt {

// Modelo de talento para ser usado pelos views de tabela.
class ModeloTalentos : public QAbstractTableModel {
 public:
  using InfoTalentos = ent::EntidadeProto::InfoTalentos;

  ModeloTalentos(const ent::Tabelas& tabelas, InfoTalentos* talentos, QTableView* tabela)
      : QAbstractTableModel(tabela), tabelas_(tabelas) {
    for (const auto& tg : talentos->gerais()) {
      modelo_.emplace_back(tg.id(), tg.complemento(), true);
    }
    for (const auto& to : talentos->outros()) {
      modelo_.emplace_back(to.id(), to.complemento(), false);
    }
  }

  // Numero de linhas da tabela.
  int rowCount(const QModelIndex& parent =  QModelIndex()) const override {
    return modelo_.size();
  }

  // 0: id talento. 1. complemento. 2. geral.
  int columnCount(const QModelIndex& parent = QModelIndex()) const override {
    return 3;
  }

  bool insertRows(int row, int count, const QModelIndex& parent) override {
    if (count != 1) return false;
    beginInsertRows(parent, 0, 0);
    modelo_.insert(modelo_.begin() + row, count, IdComplementoGeral("", "", false));
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
    if (orientation == Qt::Horizontal && role == Qt::SizeHintRole && section == 0) {
      // Hack pra aumentar o tamanho da primeira coluna.
      QSize qs;
      qs.setWidth(100);
      return QVariant(qs);
    }
    if (orientation == Qt::Vertical || role != Qt::DisplayRole) {
      return QVariant();
    }
    switch (section) {
      case 0: return QVariant(QString::fromUtf8("Talento"));
      case 1: return QVariant(QString::fromUtf8("Complemento"));
      case 2: return QVariant(QString::fromUtf8("Geral"));
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
          return QVariant(QString::fromUtf8(tabelas_.Talento(modelo_[row].id).nome().c_str()));
        } else if (role == Qt::EditRole) {
          return QVariant(modelo_[row].id.c_str());
        }
      case 1: return QVariant(QString::fromUtf8(modelo_[row].complemento.c_str()));
      case 2: return QVariant(modelo_[row].geral);
    }
    // Nunca deveria chegar aqui.
    LOG(INFO) << "Coluna invalida: " << column;
    return QVariant();
  }

  bool setData(const QModelIndex& index, const QVariant& value, int role) override {
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
        modelo_[row].id = value.toString().toUtf8().constData();
        emit dataChanged(index, index);
        return true;
      }
      case 1: {
        modelo_[row].complemento = value.toString().toUtf8().constData();
        emit dataChanged(index, index);
        return true;
      }
      case 2: {
        modelo_[row].geral = value.toBool();
        emit dataChanged(index, index);
        return true;
      }
    }
    return false;
  }

  Qt::ItemFlags flags(const QModelIndex & index) const override {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
  }

  InfoTalentos Converte() const {
    InfoTalentos ret;
    for (const auto& icg : modelo_) {
      auto* t = icg.geral ? ret.add_gerais() : ret.add_outros();
      t->set_id(icg.id);
      t->set_complemento(icg.complemento);
    }
    return ret;
  }

 private:
  const ent::Tabelas& tabelas_;
  // Esse eh o modelo verdadeiro. Id do talento, origem, geral.
  struct IdComplementoGeral {
    IdComplementoGeral(const std::string& id, const std::string& complemento, bool geral)
        : id(id), complemento(complemento), geral(geral) {}
    std::string id;
    std::string complemento;
    bool geral;
  };
  std::vector<IdComplementoGeral> modelo_;
};

}  // namespace qt
}  // namespace ifg

#endif
