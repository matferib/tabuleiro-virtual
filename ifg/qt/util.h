#ifndef IFG_QT_UTIL_H
#define IFG_QT_UTIL_H

#include <functional>
#include <map>
#include <string>
#include <QColor>
#include <QItemDelegate>
#include <QObject>
#include <QComboBox>
#include "ent/entidade.pb.h"
#include "log/log.h"

class QColor;

// O objetivo desta classe eh permitir a utilizacao de lambdas nas funcoes de conexao do QT.
// Fonte: http://blog.codef00.com/2011/03/27/combining-qts-signals-and-slots-with-c0x-lamdas/
class connect_functor_helper : public QObject {
Q_OBJECT
 public:
  connect_functor_helper(QObject *parent, const std::function<void()> &f);

 public Q_SLOTS:
  void signaled();

 private:
  std::function<void()> function_;
};

// Esta é a função que deve ser usada no lugar do connect.
template <class T>
bool lambda_connect(
    QObject *sender,
    const char *signal,
    const T &reciever,
    Qt::ConnectionType type = Qt::AutoConnection) {
  return QObject::connect(
      sender, signal, new connect_functor_helper(sender, reciever), SLOT(signaled()), type);
}

// Responsavel por tratar items representados por um mapa de string(nome) -> string (id).
class MapaDelegate : public QItemDelegate {
 public:
  MapaDelegate(const std::map<std::string, std::string> mapa, QAbstractTableModel* modelo, QObject* parent)
      : QItemDelegate(), mapa_(mapa), modelo_(modelo) {}

  QWidget* createEditor(
      QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    return PreencheConfiguraCombo(new QComboBox(parent));
  }

  // Escreve o valor do combo.
  void setEditorData(QWidget* editor, const QModelIndex& index) const override {
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (combo == nullptr) {
      LOG(INFO) << "combo == nullptr em setEditorData";
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
  QComboBox* PreencheConfiguraCombo(QComboBox* combo) const {
    // O min eh -1, invalido. Entao comeca do 0.
    for (const auto& kv : mapa_) {
      combo->addItem(QString::fromUtf8(kv.first.c_str()), QVariant(kv.second.c_str()));
    }
    //connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(commitAndCloseEditor()));
    lambda_connect(combo, SIGNAL(currentIndexChanged(int)), [this, combo]() {
      auto* thiz = const_cast<MapaDelegate*>(this);
      thiz->commitAndCloseEditor(combo);
    });
    return combo;
  }

  std::map<std::string, std::string> mapa_;
  QAbstractTableModel* modelo_;
};


/** Converte uma cor de inteiro [0.255] para float [0..1.0]. */
float ConverteCor(int cor_int);

/** Converte uma cor de float [0..1.0] para inteiro [0.255]. */
int ConverteCor(float cor_float);

/** Converte cor do QT para ent::Cor. */
const ent::Cor CorParaProto(const QColor& qcor);

/** Converte o proto para cor Qt. */
const QColor ProtoParaCor(const ent::Cor& cor);

/** Funcao faltante para QComboBox. */
inline QVariant CurrentData(QComboBox* combo) { return combo->itemData(combo->currentIndex()); }

#endif  // IFG_QT_UTIL_H
