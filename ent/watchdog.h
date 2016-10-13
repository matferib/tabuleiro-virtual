#ifndef ENT_WATCHDOG_H
#define ENT_WATCHDOG_H

#include <condition_variable>
#include <functional>
#include <memory>
#include <thread>

namespace ent {

class Tabuleiro;

// Um watchdog para salvar o tabuleiro em caso de travadas.
class Watchdog {
 public:
  Watchdog();
  ~Watchdog();

  // Inicia o watchdog, a partir desse momento ele devera ser refrescado. Caso contrario, funcao sera chamada.
  void Inicia(std::function<void()> funcao);
  // Reinicia o watchdog usando a funcao previamente definida em inicia.
  void Reinicia();

  // Retorna se o watchdog esta iniciado.
  bool Iniciado() const { return iniciado_; }

  // Para o watchdog.
  void Para();

  // Refresca o watchdog.
  void Refresca();

 private:
  // Loop que verifica se houve refrescamento.
  void Loop();

  std::unique_ptr<std::thread> thread_;
  std::mutex cond_lock_;
  std::condition_variable cond_inicio_;
  std::condition_variable cond_fim_;
  // Indica que watchdog foi refrescado.
  bool refrescado_ = false;
  bool iniciado_ = false;
  std::function<void()> funcao_;

  // Intervalo maximo entre refrescagens.
  constexpr static unsigned int INTERVALO_MAXIMO_MS = 10000;
};

}  // namespace ent

#endif
