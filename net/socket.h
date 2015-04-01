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
  explicit Erro(const boost::system::error_code& ec) : erro_(ec), msg_(ec.message()) {}
  explicit Erro(const std::string& msg) : erro_(true), msg_(msg) {}
  Erro() : erro_(false) {}

  // Verificacao de erro.
  operator bool() const { return erro_; }
  // Mesagem de erro.
  const std::string& mensagem() const { return msg_; }

 private:
  bool erro_;
  const std::string msg_;
};

class Sincronizador {
 public:
  explicit Sincronizador(boost::asio::io_service* servico_io) : servico_io_(servico_io) {}
  ~Sincronizador() {}

  // Roda o que houver para rodar, retornando o numero de tarefas executadas.
  int Roda() { return servico_io_->poll(); }

  boost::asio::io_service* Servico() { return servico_io_; }

 private:
  boost::asio::io_service* servico_io_ = nullptr;
};

// Abstracao para socket UDP broadcast.
class SocketUdp {
 public:
  // Cria socket fechado.
  explicit SocketUdp(Sincronizador* sincronizador)
      : socket_(new boost::asio::ip::udp::socket(*sincronizador->Servico())) {
  }

  // Cria socket aberto para recepcao na porta.
  SocketUdp(Sincronizador* sincronizador, int porta) : SocketUdp(sincronizador) {
    boost::asio::ip::udp::endpoint endereco(boost::asio::ip::udp::v4(), porta);
    socket_.reset(new boost::asio::ip::udp::socket(*sincronizador->Servico(), endereco));
  }

  ~SocketUdp() {}

  // Abre o socket para broadcast. Da excecao em caso de falha.
  void Abre() {
    boost::system::error_code erro;
    socket_->open(boost::asio::ip::udp::v4(), erro);
    boost::asio::socket_base::broadcast option(true);
    socket_->set_option(option);
  }

  // Retorna se o socket esta aberto.
  bool Aberto() const { return socket_->is_open(); }

  void Fecha() {
    boost::system::error_code ec;
    socket_->close(ec);
  }

  typedef std::function<void(const Erro& erro, std::size_t bytes_enviados)> CallbackEnvio;

  // Envio de broadcast assincrono. Dados deve viver ate o fim.
  void Envia(int porta, const std::vector<char>& dados, CallbackEnvio callback_envio_cliente) {
    socket_->async_send_to(
        boost::asio::buffer(dados),
        boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("255.255.255.255"), porta),
        [callback_envio_cliente] (const boost::system::error_code& ec, std::size_t bytes_enviados) {
      callback_envio_cliente(Erro(ec), bytes_enviados);
    });
  }

  // Callback de recepcao da recepcao UDP.
  typedef std::function<void(const Erro& erro, std::size_t bytes_recebidos)> CallbackRecepcao;

  // Recebe dados na conexao UDP de forma assincrona e NAO continua. Preenche dados e endereco de quem enviou,
  // chamando callback_recepcao_cliente.
  // @throws std::exception em caso de erro.
  void Recebe(
      std::vector<char>* dados, boost::asio::ip::udp::endpoint* endereco, CallbackRecepcao callback_recepcao_cliente) {
    socket_->async_receive_from(
        boost::asio::buffer(*dados),
        *endereco,
        [callback_recepcao_cliente] (const boost::system::error_code& ec, std::size_t bytes_enviados) {
      callback_recepcao_cliente(Erro(ec), bytes_enviados);
    });
  }

 private:
  std::unique_ptr<boost::asio::ip::udp::socket> socket_;
};

// Abstracao do socket.
class Socket {
 public:
  explicit Socket(Sincronizador* sincronizador)
      : sincronizador_(sincronizador),
        socket_(new boost::asio::ip::tcp::socket(*sincronizador->Servico())) {}
  ~Socket() {}

  // Conecta o socket a um endereco.
  // Endereco pode ser IP: XXX.XXX.XXX.XXX ou dominio: www.teste.com. Porta eh a representacao string de um numero.
  // @throws std::exception em caso de erro.
  void Conecta(const std::string& endereco, const std::string& porta) {
    boost::asio::ip::tcp::resolver resolver(*sincronizador_->Servico());
    auto endereco_resolvido = resolver.resolve({endereco, porta});
    boost::asio::connect(*socket_, endereco_resolvido);
  }

  void Fecha() {
    socket_->close();
  }

  // Ao terminar o envio, recebera codigo de erro e numero de bytes enviados.
  typedef std::function<void(const boost::system::error_code& ec, std::size_t bytes_enviados)> CallbackEnvio;

  // Funcao assincrona para enviar dados atraves do socket. Parametros 'dados' deve viver ate o fim.
  // Lanca std::exception em caso de erro.
  void Envia(const std::vector<char>& dados, CallbackEnvio callback_envio_cliente) {
    boost::asio::async_write(*socket_.get(),
                             boost::asio::buffer(dados),
                             callback_envio_cliente);
  }

  typedef std::function<void(const boost::system::error_code& ec, std::size_t bytes_recebidos)> CallbackRecepcao;

  // Funcao assincrona para receber dados do socket.
  // Parametro 'dados' deve viver ate o fim e sera alterado para o tamanho certo.
  // Lanca std::exception em caso de erro.
  void Recebe(std::vector<char>* dados, CallbackRecepcao callback_recepcao_cliente) {
    socket_->async_receive(boost::asio::buffer(*dados), callback_recepcao_cliente);
  }

  // Retorna o socket boost.
  boost::asio::ip::tcp::socket* Boost() { return socket_.get(); }

 private:
  Sincronizador* sincronizador_;
  std::unique_ptr<boost::asio::ip::tcp::socket> socket_;
};

class Aceitador {
 public:
  explicit Aceitador(Sincronizador* sincronizador) : sincronizador_(sincronizador) {}
  ~Aceitador() {}

  // Callback do cliente deve receber um codigo de erro e retornar um socket. Este NAO eh de responsabilidade do aceitador.
  typedef std::function<Socket*(boost::system::error_code ec)> CallbackConexaoCliente;

  // Inicia o aceitador, de forma que sempre que uma conexao acontecer, callback_conexao sera chamado. O socket novo
  // retornado sera usado para esperar mais clientes.
  bool Liga(int porta,
            Socket* socket_cliente,
            CallbackConexaoCliente callback_conexao_cliente) {
    aceitador_.reset(new boost::asio::ip::tcp::acceptor(
        *sincronizador_->Servico(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), porta)));
    // Quando um cliente for recebido, CallbackConexao sera chamado.
    aceitador_->async_accept(
        *socket_cliente->Boost(),
        std::bind(&Aceitador::CallbackConexao,
                  std::placeholders::_1,  // ec, a ser preenchido.
                  aceitador_.get(),
                  callback_conexao_cliente)
    );
    return true;
  }

  bool Ligado() const { return aceitador_ != nullptr; }

  void Desliga() {
    aceitador_.reset();
  }

 private:
  static void CallbackConexao(
      boost::system::error_code ec,
      boost::asio::ip::tcp::acceptor* aceitador,
      CallbackConexaoCliente callback_conexao_cliente) {
    // Chama callback do cliente com erro recebido.
    Socket* novo_socket_cliente = callback_conexao_cliente(ec);
    if (novo_socket_cliente != nullptr) {
      // Se recebeu socket, chama de novo.
      aceitador->async_accept(
          *novo_socket_cliente->Boost(),
          std::bind(CallbackConexao,
                    std::placeholders::_1,  // ec, a ser preenchido.
                    aceitador,
                    callback_conexao_cliente));
    }
  }

  Sincronizador* sincronizador_ = nullptr;
  std::unique_ptr<boost::asio::ip::tcp::acceptor> aceitador_;
};

}  // namespace

#endif
