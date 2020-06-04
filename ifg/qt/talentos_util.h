#ifndef IFG_QT_DIALOGO_TALENTOS_UTIL_H
#define IFG_QT_DIALOGO_TALENTOS_UTIL_H

#include <QLabel>
#include <QItemDelegate>
#include <QHeaderView>
#include <QComboBox>
#include <QTableView>
#include <QScrollBar>
#include "ent/entidade.pb.h"
#include "ent/tabelas.h"
#include "ifg/qt/util.h"
#include "log/log.h"

namespace ifg {
namespace qt {

// https://forum.qt.io/topic/21915/qabstracttablemodel-subclass-with-hyperlink/3.
class RichTextDelegate : public QItemDelegate {
Q_OBJECT
 public:
  explicit RichTextDelegate(QTableView* parent) : QItemDelegate(parent), view_(parent) {}
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(painter);
    Q_UNUSED(option);
    if (view_->indexWidget(index) != nullptr) return;
    QLabel *label = new QLabel;
    label->setTextFormat(Qt::RichText);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setOpenExternalLinks(true);
    label->setText(index.data().toString());
    view_->setIndexWidget(index, label);
  }

 private:
  QTableView *view_ = nullptr;
};

// Modelo de talento para ser usado pelos views de tabela.
class ModeloTalentos : public QAbstractTableModel {
 public:
  using InfoTalentos = ent::EntidadeProto::InfoTalentos;

  ModeloTalentos(const ent::Tabelas& tabelas, InfoTalentos* talentos, QTableView* tabela)
      : QAbstractTableModel(tabela), tabelas_(tabelas) {
    for (const auto& tg : talentos->gerais()) {
      modelo_.emplace_back(tg.id(), tg.complemento(), true, tabelas_.Talento(tg.id()).link());
    }
    for (const auto& ta : talentos->automaticos()) {
      modelo_.emplace_back(ta.id(), ta.complemento(), false, tabelas_.Talento(ta.id()).link());
    }
    for (const auto& to : talentos->outros()) {
      modelo_.emplace_back(to.id(), to.complemento(), false, tabelas_.Talento(to.id()).link());
    }
  }

  // Numero de linhas da tabela.
  int rowCount(const QModelIndex& parent =  QModelIndex()) const override {
    return modelo_.size();
  }

  // 0: id talento. 1. complemento. 2. geral. 3. link.
  int columnCount(const QModelIndex& parent = QModelIndex()) const override {
    return 4;
  }

  bool insertRows(int row, int count, const QModelIndex& parent) override {
    if (count != 1) return false;
    beginInsertRows(parent, 0, 0);
    modelo_.insert(modelo_.begin() + row, count, IdComplementoGeral("", "", true, ""));
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

  int LarguraColuna(int coluna) const {
    auto* pai = qobject_cast<QTableView*>(QObject::parent());
    if (pai == nullptr) return 0;

    const int w = pai->viewport()->size().width() - pai->verticalScrollBar()->sizeHint().width();
    switch (coluna) {
      case 0: return w * 0.35;
      case 1: return w * 0.35;
      case 2: return w * 0.10;
      case 3: return w * 0.20;
      default: return 0;
    }
  }

  QSize TamanhoCelula(int coluna) const {
    const auto* pai = qobject_cast<QTableView*>(QObject::parent());
    if (pai == nullptr) return QSize();
    QSize qs;
    qs.setHeight(pai->verticalHeader()->defaultSectionSize());
    qs.setWidth(LarguraColuna(coluna));
    return qs;
  }

  // Os cabeçalhos.
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
    if (orientation == Qt::Vertical) {
      return QVariant();
    }
    if (role == Qt::SizeHintRole) {
      return QVariant(TamanhoCelula(section));
    }
    if (role == Qt::SizeHintRole) {
      auto* pai = qobject_cast<QTableView*>(QObject::parent());
      if (pai == nullptr) return QVariant();
      const int w = pai->width();
      QSize qs = QAbstractTableModel::headerData(section, orientation, role).toSize();
      qs.setHeight(pai->verticalHeader()->defaultSectionSize());
      switch (section) {
        case 0:
          qs.setWidth(w * 0.35);
          return QVariant(qs);
        case 1:
          qs.setWidth(w * 0.35);
          return QVariant(qs);
        case 2:
          qs.setWidth(w * 0.1);
          return QVariant(qs);
        case 3:
          qs.setWidth(w * 0.2);
          return QVariant(qs);
        default: ;
      }
      return QVariant();
    }
    if (role == Qt::DisplayRole) {
      switch (section) {
        case 0: return QVariant(QString::fromUtf8("Talento"));
        case 1: return QVariant(QString::fromUtf8("Complemento"));
        case 2: return QVariant(QString::fromUtf8("Geral"));
        case 3: return QVariant(QString::fromUtf8("Link"));
        default: ;
      }
    }
    return QVariant();
  }

  // Dado de cada celula.
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
    if (role == Qt::SizeHintRole) {
      return QVariant(TamanhoCelula(index.column()));
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::ToolTipRole) {
      return QVariant();
    }

    const unsigned int row = index.row();
    if (row >= modelo_.size()) return QVariant();
    const int column = index.column();
    switch (column) {
      case 0:
        // Nome.
        if (role == Qt::DisplayRole) {
          return QVariant(QString::fromUtf8(tabelas_.Talento(modelo_[row].id).nome().c_str()));
        } else if (role == Qt::ToolTipRole) {
          return QVariant(QString::fromUtf8(tabelas_.Talento(modelo_[row].id).descricao().c_str()));
        } else if (role == Qt::EditRole) {
          return QVariant(modelo_[row].id.c_str());
        }
      case 1:
        // complemento.
        return role == Qt::DisplayRole || role == Qt::EditRole ? QVariant(QString::fromUtf8(modelo_[row].complemento.c_str())) : QVariant();
      case 2:
        // tipo.
        return role == Qt::DisplayRole ? QVariant(modelo_[row].geral) : QVariant();
      case 3: {
        // link.
        return role == Qt::DisplayRole ? QVariant(QString::fromUtf8("<a href='%1'>link</a>").arg(QString::fromStdString(modelo_[row].link))) : QVariant();
      }

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
    if (index.column() == 2) {
      return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    } else {
      return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }
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
    IdComplementoGeral(const std::string& id, const std::string& complemento, bool geral, const std::string& link)
        : id(id), complemento(complemento), geral(geral), link(link) {}
    std::string id;
    std::string complemento;
    bool geral;
    std::string link;
  };
  std::vector<IdComplementoGeral> modelo_;
};

class ComplementoTalentoDelegate : public QItemDelegate {
 public:
  ComplementoTalentoDelegate(const ent::Tabelas& tabelas, QAbstractTableModel* modelo, QObject* parent)
      : QItemDelegate(), tabelas_(tabelas), modelo_(modelo) {}

  QWidget* createEditor(
      QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    QVariant valor = modelo_->data(index.child(index.row(), 0), Qt::EditRole);
    const auto& talento = tabelas_.Talento(valor.toString().toStdString());
    switch (talento.tipo_complemento()) {
      case ent::TCT_ARMA_SIMPLES:
      case ent::TCT_ARMA_COMUM:
      case ent::TCT_ARMA_EXOTICA:
      case ent::TCT_ARMA: {
        std::map<std::string, std::string> mapa;
        for (const auto& a : tabelas_.todas().tabela_armas().armas()) {
          if ((talento.tipo_complemento() == ent::TCT_ARMA_COMUM && a.categoria_pericia() != ent::CATPER_COMUM) ||
              (talento.tipo_complemento() == ent::TCT_ARMA_EXOTICA && a.categoria_pericia() != ent::CATPER_EXOTICA) ||
              (talento.tipo_complemento() == ent::TCT_ARMA_SIMPLES && a.categoria_pericia() != ent::CATPER_SIMPLES)) {
            continue;
          }
          if (a.has_id_arma_base()) continue;
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
    ExpandeComboBox(combo);

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

}  // namespace qt
}  // namespace ifg

#endif
