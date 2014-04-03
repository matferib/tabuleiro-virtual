#include <boost/timer/timer.hpp>
#ifndef _GLIBCXX_USE_NANOSLEEP
#define _GLIBCXX_USE_NANOSLEEP
#include <thread>
#endif

#include "ent/watchdog.h"
#include "log/log.h"

namespace ent {

Watchdog::Watchdog() {
}

Watchdog::~Watchdog() {
  Para();
}

void Watchdog::Inicia(std::function<void()> funcao) {
  thread_.reset(new std::thread(&Watchdog::Loop, this));
  funcao_ = funcao;
}

void Watchdog::Para() {
  finalizar_loop_ = true;
  if (thread_.get() != nullptr) {
    thread_->join();
    thread_.reset();
  }
}

void Watchdog::Refresca() {
  refrescado_ = true;
}

void Watchdog::Loop() {
  do {
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    if (!refrescado_) {
      LOG(ERROR) << "WATCHDOG NAO REFRESCADO";
      funcao_();
      finalizar_loop_ = true;
    } else {
      refrescado_ = false;
    }
  } while (!finalizar_loop_);
}

}  // namespace ent
