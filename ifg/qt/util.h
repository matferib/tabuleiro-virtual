#ifndef IFG_QT_UTIL_H
#define IFG_QT_UTIL_H

#include <boost/function.hpp>

// O objetivo desta classe eh permitir a utilizacao de lambdas nas funcoes de conexao do QT.

class connect_functor_helper : public QObject {
Q_OBJECT
 public:
  connect_functor_helper(QObject *parent, const boost::function<void()> &f) 
      : QObject(parent), function_(f) {}

 public Q_SLOTS:
  void signaled() {
    function_();
  }

 private:
  boost::function<void()> function_;
};

template <class T>
bool connect(QObject *sender,
             const char *signal,
             const T &reciever,
             Qt::ConnectionType type = Qt::AutoConnection) {
  return QObject::connect(
      sender, signal, new connect_functor_helper(sender, reciever), SLOT(signaled()), type);
}

#endif  // IFG_QT_UTIL_H
