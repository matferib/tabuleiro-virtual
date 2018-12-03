#include "ifg/qt/util.h"
#include "log/log.h"

connect_functor_helper::connect_functor_helper(
    QObject *parent, const std::function<void()> &f) : QObject(parent), function_(f) {}

void connect_functor_helper::signaled() {
  function_();
}

float ConverteCor(int cor_int) {
  return cor_int / 255.0;
}

int ConverteCor(float cor_float) {
  int cor = static_cast<int>(255.0f * cor_float);
  if (cor < 0) {
    LOG(WARNING) << "Cor menor que zero!";
    cor = 0;
  } else if (cor > 255) {
    LOG(WARNING) << "Cor maior que 255.";
    cor = 255;
  }
  return cor;
}

const QColor ProtoParaCor(const ent::Cor& cor) {
  return QColor(ConverteCor(cor.r()),
                ConverteCor(cor.g()),
                ConverteCor(cor.b()),
                ConverteCor(cor.a()));
}

const ent::Cor CorParaProto(const QColor& qcor) {
  ent::Cor cor;
  cor.set_r(ConverteCor(qcor.red()));
  cor.set_g(ConverteCor(qcor.green()));
  cor.set_b(ConverteCor(qcor.blue()));
  cor.set_a(ConverteCor(qcor.alpha()));
  return cor;
}

// A funcao minimumSizeHint eh lazy, ou seja, computada apenas a primeira vez. Depois impossivel altera-la.
// Portanto, quando ExpandeComboBox for chamado, todas as possibilidades de combo ja devem ter sido preenchidas.
void ExpandeComboBox(QComboBox* combo) {
#if WIN32 || __APPLE__
  combo->view()->setMinimumWidth(combo->minimumSizeHint().width());
#endif
}

