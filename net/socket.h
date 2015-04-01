// Representacao da interface de socket independente de plataforma. Interface assincrona por callbacks.
// O sincronizador garantira que todos os callbacks serao chamados na mesma thread onde Roda eh chamado.
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>

namespace net {

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

#if 0
// Abstracao do socket.
class Socket {
 public:
  explicit Socket(Sincronizador* sincronizador);
  ~Socket();

  // Funcao assincrona para enviar dados atraves do socket.
  void Envia(std::function<void(const std::string& dados, bool erro)>);
  // Funcao assincrona para receber dados do socket.
  void Recebe(std::function<void(const std::string* dados, bool erro)>);
};
#endif

class Aceitador {
 public:
  explicit Aceitador(Sincronizador* sincronizador) : sincronizador_(sincronizador) {}
  ~Aceitador() {}

  // Inicia o aceitador, de forma que sempre que uma conexao acontecer, callback_conexao sera chamado. O socket novo
  // retornado sera usado para esperar mais clientes.
  bool Liga(int porta,
            boost::asio::ip::tcp::socket* socket_cliente,
            std::function<boost::asio::ip::tcp::socket*(boost::system::error_code ec)> callback_conexao_cliente) {
    aceitador_.reset(new boost::asio::ip::tcp::acceptor(
        *sincronizador_->Servico(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), porta)));
    // Quando um cliente for recebido, CallbackConexao sera chamado.
    aceitador_->async_accept(
        *socket_cliente,
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

  //boost::asio::ip::tcp::acceptor* Boost() { return aceitador_; }

 private:
  static void CallbackConexao(
      boost::system::error_code ec,
      boost::asio::ip::tcp::acceptor* aceitador,
      std::function<boost::asio::ip::tcp::socket*(boost::system::error_code ec)> callback_conexao_cliente) {
    // Chama callback do cliente com erro recebido.
    boost::asio::ip::tcp::socket* novo_socket_cliente = callback_conexao_cliente(ec);
    if (novo_socket_cliente != nullptr) {
      // Se recebeu socket, chama de novo.
      aceitador->async_accept(
          *novo_socket_cliente,
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
