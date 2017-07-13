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
  using Eventos = google::protobuf::RepeatedPtrField<Evento>;

  ModeloEvento(Eventos* eventos, QTableView* tabela)
      : QAbstractTableModel(tabela), tabela_(tabela), eventos_(eventos) {}

  // Numero de linhas da tabela.
  int rowCount(const QModelIndex& parent =  QModelIndex()) const override {
    return eventos_->size();
  }

  // 0: id efeito. 1: complemento. 2: rodadas. 3. descricao.
  int columnCount(const QModelIndex& parent = QModelIndex()) const override {
    return 4;
  }

  bool insertRows(int row, int count, const QModelIndex& parent) override {
    if (count != 1) return false;
    beginInsertRows(parent, 0, 0);
    eventos_->Add();
    endInsertRows();
    return true;
  }

  bool removeRows(int row, int count, const QModelIndex& parent) override {
    beginRemoveRows(parent, row, row + count - 1);
    eventos_->DeleteSubrange(row, count);
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
      case 0: return QVariant(QString::fromUtf8("Id"));
      case 1: return QVariant(QString::fromUtf8("Complemento"));
      case 2: return QVariant(QString::fromUtf8("Rodadas"));
      case 3: return QVariant(QString::fromUtf8("Descrição"));
    }
    return QVariant("Desconhecido");
  }

  // Dado de cada celula.
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
    if (role != Qt::DisplayRole && role != Qt::EditRole) {
      return QVariant();
    }

    const int row = index.row();
    if (row < 0 || row >= eventos_->size()) {
      LOG(INFO) << "Linha invalida " << row;
      return QVariant();
    }
    const int column = index.column();
    const auto& evento = eventos_->Get(row);
    switch (column) {
      case 0: return role == Qt::DisplayRole ? QVariant(ent::TipoEvento_Name(evento.id_efeito()).c_str()) : QVariant(evento.id_efeito());
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
    if (row < 0 || row >= eventos_->size()) {
      LOG(INFO) << "Linha invalida " << row;
      return false;
    }
    const int column = index.column();
    auto* evento = eventos_->Mutable(row);
    switch (column) {
      case 0: {
        if (!ent::TipoEvento_IsValid(value.toInt())) return false;
        evento->set_id_efeito(ent::TipoEvento(value.toInt()));
        emit dataChanged(index, index);
        return true;
      }
      case 1: {
        evento->set_complemento(value.toInt());
        emit dataChanged(index, index);
        return true;
      }
      case 2: {
        evento->set_rodadas(value.toInt());
        emit dataChanged(index, index);
        return true;
      }
      case 3: {
        evento->set_descricao(value.toString().toUtf8().constData());
        emit dataChanged(index, index);
        return true;
      }
    }
    return false;
  }

  Qt::ItemFlags flags(const QModelIndex & index) const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
  }

 private:
  QTableView* tabela_;
  Eventos* eventos_;
};

// Responsavel por tratar a edicao do tipo de efeito.
class TipoEventoDelegate : public QItemDelegate {
 public:
  TipoEventoDelegate(QTableView* tabela, ModeloEvento* modelo, QObject* parent)
      : QItemDelegate(), tabela_(tabela), modelo_(modelo) {}

  QWidget* createEditor(
      QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    QComboBox* combo = new QComboBox(parent);
    PreencheComboEvento(combo);
    return combo;
  }

  // Escreve o valor do combo.
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
    const QVariant& data = modelo_->data(index, Qt::EditRole);
    if (!ent::TipoEvento_IsValid(data.toInt())) return;
    combo->setCurrentIndex(data.toInt());
  }

  // Salva o valor do combo no modelo.
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
  void PreencheComboEvento(QComboBox* combo) const {
    // O min eh -1, invalido. Entao comeca do 0.
    for (int tipo = 0; tipo <= ent::TipoEvento_MAX; tipo++) {
      if (!ent::TipoEvento_IsValid(tipo)) continue;
      combo->addItem(ent::TipoEvento_Name(ent::TipoEvento(tipo)).c_str(), QVariant(tipo));
    }
  }

  QTableView* tabela_;
  ModeloEvento* modelo_;
  ent::TipoEvento tipo_;
};


}  // namespace qt
}  // namespace ifg

#endif
