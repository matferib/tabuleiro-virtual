#ifndef IFG_QT_UTIL_H
#define IFG_QT_UTIL_H

#include <functional>
#include <string>
#include <QColor>
#include <QObject>
#include "ent/entidade.pb.h"

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

/** Converte uma cor de inteiro [0.255] para float [0..1.0]. */
float ConverteCor(int cor_int);

/** Converte uma cor de float [0..1.0] para inteiro [0.255]. */
int ConverteCor(float cor_float);

/** Converte cor do QT para ent::Cor. */
const ent::Cor CorParaProto(const QColor& qcor);

/** Converte o proto para cor Qt. */
const QColor ProtoParaCor(const ent::Cor& cor);

#endif  // IFG_QT_UTIL_H
