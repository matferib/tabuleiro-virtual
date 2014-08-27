#include <boost/timer/timer.hpp>
// Esse define eh por causa de um bug na definicao do sleep for.
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
  funcao_ = funcao;
  thread_.reset(new std::thread(&Watchdog::Loop, this));
}

void Watchdog::Reinicia() {
  thread_.reset(new std::thread(&Watchdog::Loop, this));
}

void Watchdog::Para() {
  cond_lock_.lock();
  cond_fim_.notify_one();
  cond_lock_.unlock();
  if (thread_.get() != nullptr) {
    thread_->join();
    thread_.reset();
  }
}

void Watchdog::Refresca() {
  refrescado_ = true;
}

void Watchdog::Loop() {
  std::unique_lock<std::mutex> ul(cond_lock_);
  do {
    if (cond_fim_.wait_for(ul, std::chrono::milliseconds(10000)) == std::cv_status::no_timeout) {
      // condicao de termino.
      break;
    }
    if (!refrescado_) {
      LOG(ERROR) << "WATCHDOG NAO REFRESCADO";
      funcao_();
    } else {
      refrescado_ = false;
    }
  } while (true);
}

}  // namespace ent
