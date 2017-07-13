#ifndef IFG_QT_DIALOGO_EVENTO_UTIL_H
#define IFG_QT_DIALOGO_EVENTO_UTIL_H

#include <QComboBox>
#include "ent/entidade.pb.h"
#include "ifg/qt/util.h"
#include "log/log.h"

namespace ifg {
namespace qt {

// Modelo de evento para ser usado pelos views de tabela.
class ModeloEvento : public QAbstractTableModel {
 public:
  using Evento = ent::EntidadeProto::Evento;

  ModeloEvento(const google::protobuf::RepeatedPtrField<Evento>& eventos, QTableView* tabela)
      : QAbstractTableModel(tabela), tabela_(tabela), eventos_(eventos) {}

  // Numero de linhas da tabela.
  int rowCount(const QModelIndex& parent =  QModelIndex()) const override {
    return eventos_.size();
  }

  // 0: id efeito. 1: complemento. 2: rodadas. 3. descricao.
  int columnCount(const QModelIndex& parent = QModelIndex()) const override {
    return 4;
  }

  bool insertRows(int row, int count, const QModelIndex& parent) override {
    if (count != 1) return false;
    beginInsertRows(parent, 0, 0);
    eventos_.Add();
    endInsertRows();
    return true;
  }

  bool removeRows(int row, int count, const QModelIndex& parent) override {
    beginRemoveRows(parent, row, row + count - 1);
    eventos_.DeleteSubrange(row, count);
    endRemoveRows();
    return true;
  }

  // Os cabeçalhos.
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
    if (orientation == Qt::Vertical || role != Qt::DisplayRole) {
      return QVariant();
    }
    switch (section) {
      case 0: return QVariant("Id");
      case 1: return QVariant("Complemento");
      case 2: return QVariant("Rodadas");
      case 3: return QVariant("Descrição");
    }
    return QVariant("Desconhecido");
  }

  // Dado de cada celula.
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
    if (role != Qt::DisplayRole && role != Qt::EditRole) {
      return QVariant();
    }

    const int row = index.row();
    if (row < 0 || row >= eventos_.size()) {
      LOG(INFO) << "Linha invalida " << row;
      return QVariant();
    }
    const int column = index.column();
    const auto& evento = eventos_.Get(row);
    switch (column) {
      case 0: return QVariant(evento.id_efeito());
      case 1: return QVariant(evento.complemento());
      case 2: return QVariant(evento.rodadas());
      case 3: return QVariant(QString::fromUtf8(evento.descricao().c_str()));
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
    if (row < 0 || row >= eventos_.size()) {
      LOG(INFO) << "Linha invalida " << row;
      return false;
    }
    const int column = index.column();
    auto* evento = eventos_.Mutable(row);
    switch (column) {
      case 0: {
        evento->set_id_efeito(value.toInt());
        return true;
      }
      case 1: {
        evento->set_complemento(value.toInt());
        return true;
      }
      case 2: {
        evento->set_rodadas(value.toInt());
        return true;
      }
      case 3: {
        evento->set_descricao(value.toString().toUtf8().constData());
      }
    }
    return false;
  }

  Qt::ItemFlags flags(const QModelIndex & index) const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
  }

  const google::protobuf::RepeatedPtrField<Evento> Eventos() const { return eventos_; }

 private:
  QTableView* tabela_;
  google::protobuf::RepeatedPtrField<Evento> eventos_;
};

}  // namespace qt
}  // namespace ifg

#endif
