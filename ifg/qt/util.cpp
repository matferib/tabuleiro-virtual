#include "ifg/qt/util.h"

connect_functor_helper::connect_functor_helper(
    QObject *parent, const std::function<void()> &f) : QObject(parent), function_(f) {}

void connect_functor_helper::signaled() {
  function_();
}
