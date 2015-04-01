// Representacao da interface de socket independente de plataforma. Interface assincrona por callbacks.
// O sincronizador garantira que todos os callbacks serao chamados na mesma thread onde Roda eh chamado.
#include <functional>

namespace net {

class Sincronizador {
 public:
  Sincronizador();
  ~Sincronizador();

  // Roda o que houver para rodar, retornando o numero de tarefas executadas.
  int Roda();
};

// Abstracao do socket.
class Socket {
 public:
  Socket();
  ~Socket();

  // Funcao assincrona para enviar dados atraves do socket.
  void Envia(std::function<void(const std::string& dados, bool erro)>);
  // Funcao assincrona para receber dados do socket.
  void Recebe(std::function<void(const std::string* dados, bool erro)>);
};

class Aceitador {
 public:
  Aceitador();
  ~Aceitador();

  // Inicia o aceitador, de forma que sempre que uma conexao acontecer, callback_conexao sera chamado.
  bool Inicia(std::function<void(Socket*, const std::string& erro)> callback_conexao);
};


}  // namespace
