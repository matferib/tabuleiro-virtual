#ifndef IFG_QT_UTIL_H
#define IFG_QT_UTIL_H

#include <functional>
#include <map>
#include <string>
#include <QtCore/QObject>
#include <QtGui/QColor>
#include <QtWidgets/QItemDelegate>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QTreeWidgetItem>
#include "ent/entidade.pb.h"
#include "log/log.h"

// No windows, o drop down do combo aparece do tamanho do combo. Isso aqui tenta corrigir o
// problema. Ver: https://bugreports.qt.io/browse/QTBUG-3097.
// Solucao: https://stackoverflow.com/questions/3151798/how-do-i-set-the-qcombobox-width-to-fit-the-largest-item
void ExpandeComboBox(QComboBox* combo); 

// Objeto para tratar resize events.
class ResizeHelper : public QObject {
Q_OBJECT
 public:
  ResizeHelper(QObject *parent, const std::function<void(QResizeEvent*)> f)
      : QObject(parent), function_(f) {}

 public slots:
  void resized(QResizeEvent* event) {
    function_(event);
  }

 private:
  std::function<void(QResizeEvent*)> function_;
};

// Lambda connect util para combo boxes.
inline bool lambda_connect(
    QObject *sender,
    const char *signal,
    const std::function<void(QResizeEvent*)> receiver,
    Qt::ConnectionType type = Qt::AutoConnection) {
  return QObject::connect(
      sender, signal, new ResizeHelper(sender, receiver), SLOT(resized(QResizeEvent*)), type);
}


// Objeto para tratar mudancas de combo.
class ComboBoxHelper : public QObject {
Q_OBJECT
 public:
  ComboBoxHelper(QObject *parent, const std::function<void(int)> f)
      : QObject(parent), function_(f) {}

 public slots:
  void indexChanged(int indice) {
    function_(indice);
  }

 private:
  std::function<void(int)> function_;
};

// Lambda connect util para combo boxes.
inline bool lambda_connect(
    QObject *sender,
    const char *signal,
    const std::function<void(int)> receiver,
    Qt::ConnectionType type = Qt::AutoConnection) {
  return QObject::connect(
      sender, signal, new ComboBoxHelper(sender, receiver), SLOT(indexChanged(int)), type);
}


// Objeto para tratar mudancas de arvores.
class TreeHelper : public QObject {
Q_OBJECT
 public:
  TreeHelper(QObject *parent, const std::function<void(QTreeWidgetItem*, int)> f)
      : QObject(parent), function_(f) {}

 public slots:
  void itemChanged(QTreeWidgetItem* item, int column) {
    function_(item, column);
  }

 private:
  std::function<void(QTreeWidgetItem*, int)> function_;
};

// Lambda connect para arvores.
inline bool lambda_connect(
    QObject *sender,
    const char *signal,
    const std::function<void(QTreeWidgetItem*, int)> receiver,
    Qt::ConnectionType type = Qt::AutoConnection) {
  return QObject::connect(
      sender, signal, new TreeHelper(sender, receiver), SLOT(itemChanged(QTreeWidgetItem*, int)), type);
}

// Objeto para tratar menus de contexto.
class ContextMenuHelper : public QObject {
Q_OBJECT
 public:
  ContextMenuHelper(QObject *parent, const std::function<void(const QPoint& pos)> f)
      : QObject(parent), function_(f) {}

 public slots:
  void pressed(const QPoint& pos) {
    function_(pos);
  }

 private:
  std::function<void(const QPoint& pos)> function_;
};

// Lambda connect para menus.
inline bool lambda_connect(
    QObject *sender,
    const char *signal,
    const std::function<void(const QPoint& pos)> receiver,
    Qt::ConnectionType type = Qt::AutoConnection) {
  return QObject::connect(
      sender, signal, new ContextMenuHelper(sender, receiver), SLOT(pressed(const QPoint&)), type);
}

// O objetivo desta classe eh permitir a utilizacao de lambdas nas funcoes de conexao do QT.
// Fonte: http://blog.codef00.com/2011/03/27/combining-qts-signals-and-slots-with-c0x-lamdas/
class connect_functor_helper : public QObject {
Q_OBJECT
 public:
  connect_functor_helper(QObject *parent, const std::function<void()> &f);

 public slots:
  void signaled();

 private:
  std::function<void()> function_;
};

// Esta é a função que deve ser usada no lugar do connect.
inline bool lambda_connect(
    QObject *sender,
    const char *signal,
    const std::function<void()>& receiver,
    Qt::ConnectionType type = Qt::AutoConnection) {
  return QObject::connect(
      sender, signal, new connect_functor_helper(sender, receiver), SLOT(signaled()), type);
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
    for (const auto& kv : mapa_) {
      combo->addItem(QString::fromUtf8(kv.first.c_str()), QVariant(kv.second.c_str()));
    }
    ExpandeComboBox(combo);
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
