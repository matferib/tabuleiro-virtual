#ifndef NET_SOCKET_H
#define NET_SOCKET_H

// Representacao da interface de socket independente de plataforma. Interface assincrona por callbacks.
// O sincronizador garantira que todos os callbacks serao chamados na mesma thread onde Roda eh chamado.
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>

namespace net {

// Classe de erro.
class Erro {
 public:
  // Erro boost.
  explicit Erro(const boost::system::error_code& ec);
  // Constroi objeto de erro com a mensagem passada.
  explicit Erro(const std::string& msg);
  // Constroi objeto representando sucesso (erro_ = false).
  Erro();

  // Retorna true se o erro for de conexao fechada.
  bool ConexaoFechada() const;

  // Verificacao de erro.
  operator bool() const { return erro_; }
  // Mesagem de erro.
  const std::string& mensagem() const { return msg_; }

 private:
  const boost::system::error_code ec_;
  bool erro_;  // Ha erro?
  const std::string msg_;  // Mensagem de erro.
};

// Classe usada para realizar tarefas assincronas (em background) e chamar o callback na thread principal.
class Sincronizador {
 public:
  // Constroi o sincronizador com dados internos passados. A implemetacao devera saber como usar interno.
  explicit Sincronizador(void* depende_plataforma); 
  ~Sincronizador();

  // Roda o que houver para rodar, retornando o numero de tarefas executadas.
  int Roda();

  boost::asio::io_service* Servico();

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
  void Envia(int porta, const std::vector<char>& dados, CallbackEnvio callback_envio_cliente);


  // Recebe dados na conexao UDP de forma assincrona e NAO continua. Preenche dados e endereco de quem enviou,
  // chamando callback_recepcao_cliente.
  // @throws std::exception em caso de erro.
  typedef std::function<void(const Erro& erro, std::size_t bytes_recebidos)> CallbackRecepcao;
  void Recebe(
      std::vector<char>* dados, boost::asio::ip::udp::endpoint* endereco, CallbackRecepcao callback_recepcao_cliente);

 private:
  std::unique_ptr<boost::asio::ip::udp::socket> socket_;
};

// Abstracao do socket.
class Socket {
 public:
  explicit Socket(Sincronizador* sincronizador);
  ~Socket();

  // Conecta o socket a um endereco.
  // Endereco pode ser IP: XXX.XXX.XXX.XXX ou dominio: www.teste.com. Porta eh a representacao string de um numero.
  // @throws std::exception em caso de erro.
  void Conecta(const std::string& endereco, const std::string& porta);

  // Fecha o socket.
  void Fecha();

  // Funcao assincrona para enviar dados atraves do socket. Parametros 'dados' deve viver ate o fim.
  // Lanca std::exception em caso de erro.
  typedef std::function<void(const Erro& erro, std::size_t bytes_enviados)> CallbackEnvio;
  void Envia(const std::vector<char>& dados, CallbackEnvio callback_envio_cliente);

  // Funcao assincrona para receber dados do socket.
  // Parametro 'dados' deve viver ate o fim e sera alterado para o tamanho certo.
  // Lanca std::exception em caso de erro.
  typedef std::function<void(const Erro& ec, std::size_t bytes_recebidos)> CallbackRecepcao;
  void Recebe(std::vector<char>* dados, CallbackRecepcao callback_recepcao_cliente); 

 private:
  friend class Aceitador;
  Sincronizador* sincronizador_;
  std::unique_ptr<boost::asio::ip::tcp::socket> socket_;
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
  // Usado para o aceitador se chamar recursivamente.
  static void CallbackConexao(
      boost::system::error_code ec,
      boost::asio::ip::tcp::acceptor* aceitador,
      CallbackConexaoCliente callback_conexao_cliente);

  Sincronizador* sincronizador_ = nullptr;
  std::unique_ptr<boost::asio::ip::tcp::acceptor> aceitador_;
};

}  // namespace

#endif
