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
  Reinicia();
}

void Watchdog::Reinicia() {
  std::unique_lock<std::mutex> ul(cond_lock_);
  VLOG(1) << "Reiniciando watchdog";
  iniciado_ = true;
  thread_.reset(new std::thread(&Watchdog::Loop, this));
  // Espera inicio da thread para evitar deadlock na saida. Por exemplo:
  // 1- Para() eh chamado.
  // 2- Reinicia() eh chamado (thread loop nao iniciou).
  // 3- Para eh chamado de novo, a notificacao vai ser perdida e fica preso no thread->join.
  // 4- Loop executa para sempre dando timeout.
  // Com esse comando, a gente garante que ao sair de Reinicia, a thread de loop ja comecou.
  cond_inicio_.wait(ul);
}

void Watchdog::Para() {
  cond_lock_.lock();
  VLOG(1) << "Notificando termino do watchdog loop";
  cond_fim_.notify_one();
  cond_lock_.unlock();
  if (thread_.get() != nullptr) {
    thread_->join();
    thread_.reset();
  }
  iniciado_ = false;
}

void Watchdog::Refresca() {
  refrescado_ = true;
}

void Watchdog::Loop() {
  std::unique_lock<std::mutex> ul(cond_lock_);
  cond_inicio_.notify_one();
  do {
    if (cond_fim_.wait_for(ul, std::chrono::milliseconds(10000)) == std::cv_status::no_timeout) {
      // condicao de termino.
      VLOG(1) << "Terminando thread watchdog loop";
      break;
    }
    if (!refrescado_) {
      LOG(ERROR) << "WATCHDOG NAO REFRESCADO";
      funcao_();
      Para();
    } else {
      refrescado_ = false;
    }
  } while (true);
}

}  // namespace ent
