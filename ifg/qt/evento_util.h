#ifndef IFG_QT_DIALOGO_EVENTO_UTIL_H
#define IFG_QT_DIALOGO_EVENTO_UTIL_H

#include <algorithm>
#include <QComboBox>
#include <QTableView>
#include "ent/entidade.pb.h"
#include "ent/util.h"
#include "ifg/qt/util.h"
#include "log/log.h"

namespace ifg {
namespace qt {

namespace {

bool ComplementoEventoString(const ent::EntidadeProto::Evento& evento) {
  switch (evento.id_efeito()) {
    case ent::EFEITO_VENENO:
    case ent::EFEITO_ALINHAR_ARMA:
    case ent::EFEITO_ABENCOAR_ARMA:
    case ent::EFEITO_SUPORTAR_ELEMENTOS:
    case ent::EFEITO_RESISTENCIA_ELEMENTOS:
    case ent::EFEITO_ARMA_MAGICA:
    case ent::EFEITO_PRESA_MAGICA:
      return true;
    default: return false;
  }
}

QString ComplementosParaString(const google::protobuf::RepeatedField<int>& complementos) {
  QString s;
  for (int c : complementos) {
    s.append(" ");
    s.append(QString::number(c));
  }
  if (!s.isEmpty()) {
    s.remove(0, 1);
  }
  return s;
}

QString ComplementosStrParaString(const google::protobuf::RepeatedPtrField<std::string>& complementos_str) {
  QString s;
  for (const std::string& cs : complementos_str) {
    s.append(";");
    s.append(cs.c_str());
  }
  if (!s.isEmpty()) {
    s.remove(0, 1);
  }
  return s;
}

const google::protobuf::RepeatedField<int> StringParaComplementos(const QString& complementos) {
  google::protobuf::RepeatedField<int> cs;
  QStringList lista = complementos.split(" ",  QString::SkipEmptyParts);
  for (const auto& s : lista) {
    bool ok;
    int c = s.toInt(&ok);
    if (ok) cs.Add(c);
  }
  return cs;
}

const google::protobuf::RepeatedPtrField<std::string> StringParaComplementosStr(const QString& complementos) {
  google::protobuf::RepeatedPtrField<std::string> ss;
  QStringList lista = complementos.split(";",  QString::SkipEmptyParts);
  for (const auto& s : lista) {
    *ss.Add() = s.toStdString();
  }
  return ss;
}

}  // namespace

// Modelo de evento para ser usado pelos views de tabela.
class ModeloEvento : public QAbstractTableModel {
  Q_OBJECT
 public:
  using Evento = ent::EntidadeProto::Evento;
  using Eventos = google::protobuf::RepeatedPtrField<Evento>;

  ModeloEvento(const Eventos& eventos, QTableView* tabela)
      : QAbstractTableModel(tabela), eventos_(eventos) {}

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
    auto* e = eventos_.Add();
    e->set_id_unico(AchaIdUnicoEvento(eventos_));
    endInsertRows();
    emit dataChanged(parent, parent);
    return true;
  }

  bool removeRows(int row, int count, const QModelIndex& parent) override {
    for (int i = 0; i < count; ++i ) {
      if (row + i < 0 || row + i >= eventos_.size()) break;
      eventos_.Mutable(row + i)->set_rodadas(-1);
    }
    emit dataChanged(parent, parent);
    beginRemoveRows(parent, row, eventos_.size() - 1);
    eventos_.DeleteSubrange(row, count);
    endRemoveRows();
    emit dataChanged(parent, parent);
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
    if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::ToolTipRole) {
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
      case 0: 
        return role == Qt::DisplayRole || role == Qt::ToolTipRole
            ? QVariant(QString::fromUtf8(StringEfeito(evento.id_efeito()).c_str())) 
            : QVariant(evento.id_efeito());
      case 1: return QVariant(ComplementoEventoString(evento) ?
                  ComplementosStrParaString(evento.complementos_str()) : ComplementosParaString(evento.complementos()));
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
    const int rodadas = evento->rodadas();
    switch (column) {
      case 0: {
        if (!ent::TipoEfeito_IsValid(value.toInt())) return false;
        // Anula o efeito anterior primeiro, depois aplica o novo.
        evento->set_rodadas(-1);
        emit dataChanged(index, index);
        evento->set_id_efeito(ent::TipoEfeito(value.toInt()));
        evento->set_rodadas(rodadas);
        emit dataChanged(index, index);
        return true;
      }
      case 1: {
        evento->set_rodadas(-1);
        emit dataChanged(index, index);
        if (ComplementoEventoString(*evento)) {
          *evento->mutable_complementos_str() = StringParaComplementosStr(value.toString());
        } else {
          *evento->mutable_complementos() = StringParaComplementos(value.toString());
        }
        evento->set_rodadas(rodadas);
        emit dataChanged(index, index);
        return true;
      }
      case 2: {
        evento->set_rodadas(-1);
        emit dataChanged(index, index);
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

  Qt::ItemFlags flags(const QModelIndex & index) const override {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
  }

  // Sinaliza que o modelo foi atualizado externamente.
  void ModeloAtualizado() {
    beginResetModel();
    endResetModel();
  }

  Eventos LeEventos() const { return eventos_; }

 private:
  Eventos eventos_;
};

// Responsavel por tratar a edicao do tipo de efeito.
class TipoEfeitoDelegate : public QItemDelegate {
 public:
  TipoEfeitoDelegate(QTableView* tabela, ModeloEvento* modelo, QObject* parent)
      : QItemDelegate(), modelo_(modelo) {}

  QWidget* createEditor(
      QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    return PreencheConfiguraComboEvento(new QComboBox(parent));
  }

  // Escreve o valor do combo.
  void setEditorData(QWidget* editor, const QModelIndex& index) const override {
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (combo == nullptr) {
      LOG(INFO) << "combo == nullptr em setEditorData";
      return;
    }
    const QVariant& data = modelo_->data(index, Qt::EditRole);
    if (!ent::TipoEfeito_IsValid(data.toInt())) return;
    combo->setCurrentIndex(combo->findData(data.toInt()));
  }

  // Salva o valor do combo no modelo.
  void setModelData(
      QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (combo == nullptr) {
      LOG(ERROR) << "combo == nullptr em setEditorData";
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
  QComboBox* PreencheConfiguraComboEvento(QComboBox* combo) const {
    // O min eh -1, invalido. Entao comeca do 0.
    std::map<std::string, int> efeitos_ordenados;
    for (int tipo = 0; tipo <= ent::TipoEfeito_MAX; tipo++) {
      if (!ent::TipoEfeito_IsValid(tipo)) continue;
      std::string efeito_str = StringEfeito(ent::TipoEfeito(tipo));
      efeitos_ordenados.insert(std::make_pair(efeito_str, tipo));
    }
    for (const auto& par_str_id : efeitos_ordenados) {
      combo->addItem(par_str_id.first.c_str(), QVariant(par_str_id.second));
    }
    ExpandeComboBox(combo);
    //connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(commitAndCloseEditor()));
    lambda_connect(combo, SIGNAL(currentIndexChanged(int)), [this, combo]() {
      auto* thiz = const_cast<TipoEfeitoDelegate*>(this);
      thiz->commitAndCloseEditor(combo);
    });
    return combo;
  }

  ModeloEvento* modelo_;
};


}  // namespace qt
}  // namespace ifg

#endif
