#ifndef NET_SOCKET_H
#define NET_SOCKET_H

// Representacao da interface de socket independente de plataforma. Interface assincrona por callbacks.
// O sincronizador garantira que todos os callbacks serao chamados na mesma thread onde Roda eh chamado.
#include <functional>
#include <memory>

namespace net {

// Classe de erro.
class Erro {
 public:
  // Erro dependente de plataforma.
  explicit Erro(void* depende_plataforma);
  // Constroi objeto de erro com a mensagem passada.
  explicit Erro(const std::string& msg);
  // Constroi objeto representando sucesso (erro_ = false).
  Erro(bool erro = false);

  // Retorna true se o erro for de conexao fechada.
  bool ConexaoFechada() const;

  // Verificacao de erro.
  operator bool() const { return erro_; }
  // Mesagem de erro.
  const std::string& mensagem() const { return msg_; }

 private:
  struct Interno;
  std::unique_ptr<Interno> interno_;
  bool erro_;  // Ha erro?
  std::string msg_;  // Mensagem de erro.
};

// Classe usada para realizar tarefas assincronas (em background) e chamar o callback na thread principal.
class Sincronizador {
 public:
  // Constroi o sincronizador com dados internos passados. A implemetacao devera saber como usar interno.
  explicit Sincronizador(void* depende_plataforma);
  ~Sincronizador();

  // Roda o que houver para rodar, retornando o numero de tarefas executadas.
  int Roda();

#if ANDROID
  void AlternaHackAndroid();
#endif

 private:
  friend class SocketUdp;
  friend class Socket;
  friend class Aceitador;

  // Dados internos.
  struct Interno;
  std::unique_ptr<Interno> interno_;
};

// Abstracao para socket UDP broadcast.
class SocketUdp {
 public:
  // Cria socket fechado.
  explicit SocketUdp(Sincronizador* sincronizador);

  // Cria socket aberto para recepcao na porta.
  SocketUdp(Sincronizador* sincronizador, int porta);

  ~SocketUdp();

  // Abre o socket para broadcast. Da excecao em caso de falha.
  void Abre();

  // Retorna se o socket esta aberto.
  bool Aberto() const;

  // Fecha o socket.
  void Fecha();

  // Envio de broadcast assincrono. Dados deve viver ate o fim.
  typedef std::function<void(const Erro& erro, std::size_t bytes_enviados)> CallbackEnvio;
  void Envia(int porta, const std::string& dados, CallbackEnvio callback_envio_cliente);


  // Recebe dados na conexao UDP de forma assincrona e NAO continua. Preenche dados e endereco de quem enviou,
  // chamando callback_recepcao_cliente.
  // Parametros endereco recebera o IP do servidor fazendo broadcast (sem porta, que vira dentro da mensagem)
  // e deverao sobreviver ate o callback ser chamado.
  // @throws std::exception em caso de erro.
  typedef std::function<void(const Erro& erro, std::size_t bytes_recebidos)> CallbackRecepcao;
  void Recebe(
      std::string* dados, std::string* endereco, CallbackRecepcao callback_recepcao_cliente);

 private:
  // Sem construtor padrao e copia.
  SocketUdp();
  SocketUdp(const SocketUdp&);

  Sincronizador* sincronizador_;
  struct Interno;
  std::unique_ptr<Interno> interno_;
};

// Abstracao do socket.
class Socket {
 public:
  explicit Socket(Sincronizador* sincronizador);
  ~Socket();

#if ANDROID || 1
  // Faz o socket ouvir em determinada porta.
  void Ouve(int porta);
#endif

  // Faz o socket se ligar em uma porta especifica local. Sem ouvir.
  void PortaLocal(int porta);

  // Conecta o socket a um endereco.
  // Endereco pode ser IP: XXX.XXX.XXX.XXX ou dominio: www.teste.com. Porta eh a representacao string de um numero.
  // @throws std::exception em caso de erro.
  void Conecta(const std::string& endereco, const std::string& porta);

  // Fecha o socket.
  void Fecha();

  // Funcao assincrona para enviar dados atraves do socket. Parametros 'dados' deve viver ate o fim.
  // Lanca std::exception em caso de erro.
  typedef std::function<void(const Erro& erro, std::size_t bytes_enviados)> CallbackEnvio;
  void Envia(const std::string& dados, CallbackEnvio callback_envio_cliente);

  // Funcao assincrona para receber dados do socket.
  // Parametro 'dados' deve viver ate o fim e sera alterado para o tamanho certo.
  // Lanca std::exception em caso de erro.
  typedef std::function<void(const Erro& ec, std::size_t bytes_recebidos)> CallbackRecepcao;
  void Recebe(std::string* dados, CallbackRecepcao callback_recepcao_cliente);

  // Retorna o IP do socket como string.
  std::string IpString() const;

 private:
  friend class Aceitador;
  friend class Sincronizador;
  Sincronizador* sincronizador_;
  struct Interno;
  std::unique_ptr<Interno> interno_;
};

class Aceitador {
 public:
  explicit Aceitador(Sincronizador* sincronizador);
  ~Aceitador();

  // Inicia o aceitador, de forma que sempre que uma conexao acontecer, callback_conexao sera chamado. O socket novo
  // retornado sera usado para esperar mais clientes.
  typedef std::function<Socket*(const Erro& erro)> CallbackConexaoCliente;
  bool Liga(int porta,
            Socket* socket_cliente,
            CallbackConexaoCliente callback_conexao_cliente);

  // Retorna se o aceitador esta ligado.
  bool Ligado() const;

  // Desliga o aceitador.
  void Desliga();

 private:
  Sincronizador* sincronizador_ = nullptr;

  struct Interno;
  std::unique_ptr<Interno> interno_;
};

}  // namespace

#endif
