#ifndef ENT_WATCHDOG_H
#define ENT_WATCHDOG_H

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

  // Para o watchdog.
  void Para();

  // Refresca o watchdog.
  void Refresca();

 private:
  // Loop que verifica se houve refrescamento.
  void Loop();

  std::unique_ptr<std::thread> thread_;
  // Controle do loop.
  bool finalizar_loop_ = false;
  // Indica que watchdog foi refrescado.
  bool refrescado_ = false;
  std::function<void()> funcao_;

  // Intervalo maximo entre refrescagens.
  constexpr static unsigned int INTERVALO_MAXIMO_MS = 10000;
};

}  // namespace ent

#endif
