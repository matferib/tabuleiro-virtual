#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>

#include "net/socket.h"

namespace net {

//-----
// Erro
//-----
Erro::Erro(const boost::system::error_code& ec) : ec_(ec), erro_(ec), msg_(ec.message()) {}
Erro::Erro(const std::string& msg) : erro_(true), msg_(msg) {}
Erro::Erro() : erro_(false) {}

bool Erro::ConexaoFechada() const {
  return ec_.value() == boost::asio::error::eof;
}

//--------------
// Sincronizador
//--------------
struct Sincronizador::Interno {
  Interno(void* depende_plataforma) : servico_io((boost::asio::io_service*)depende_plataforma) {}

  boost::asio::io_service* servico_io = nullptr;
};
Sincronizador::Sincronizador(void* depende_plataforma) : interno_(new Interno(depende_plataforma)) {}
Sincronizador::~Sincronizador() {}

int Sincronizador::Roda() { return interno_->servico_io->poll(); }

//----------
// SocketUdp
//----------
struct SocketUdp::Interno {
  Interno(boost::asio::ip::udp::socket* socket) : socket(socket) {}

  std::unique_ptr<boost::asio::ip::udp::socket> socket;
};

SocketUdp::SocketUdp(Sincronizador* sincronizador)
    : interno_(new Interno(new boost::asio::ip::udp::socket(*sincronizador->interno_->servico_io))) {}

SocketUdp::SocketUdp(Sincronizador* sincronizador, int porta) {
  boost::asio::ip::udp::endpoint endereco(boost::asio::ip::udp::v4(), porta);
  interno_.reset(new Interno(new boost::asio::ip::udp::socket(*sincronizador->interno_->servico_io, endereco)));
}

SocketUdp::~SocketUdp() {}

void SocketUdp::Abre() {
  boost::system::error_code erro;
  interno_->socket->open(boost::asio::ip::udp::v4(), erro);
  boost::asio::socket_base::broadcast option(true);
  interno_->socket->set_option(option);
}

bool SocketUdp::Aberto() const { return interno_->socket->is_open(); }

void SocketUdp::Fecha() {
  boost::system::error_code ec;
  interno_->socket->close(ec);
}

void SocketUdp::Envia(int porta, const std::vector<char>& dados, CallbackEnvio callback_envio_cliente) {
  interno_->socket->async_send_to(
      boost::asio::buffer(dados),
      boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("255.255.255.255"), porta),
      [callback_envio_cliente] (const boost::system::error_code& ec, std::size_t bytes_enviados) {
    callback_envio_cliente(Erro(ec), bytes_enviados);
  });
}

void SocketUdp::Recebe(
    std::vector<char>* dados, std::string* endereco, CallbackRecepcao callback_recepcao_cliente) {
  auto* endpoint = new boost::asio::ip::udp::endpoint;
  interno_->socket->async_receive_from(
      boost::asio::buffer(*dados),
      *endpoint,
      [endpoint, endereco, callback_recepcao_cliente] (const boost::system::error_code& ec, std::size_t bytes_enviados) {
    endereco->assign(endpoint->address().to_string());
    delete endpoint;
    callback_recepcao_cliente(Erro(ec), bytes_enviados);
  });
}

//-------
// Socket
//-------
Socket::Socket(Sincronizador* sincronizador)
    : sincronizador_(sincronizador),
      socket_(new boost::asio::ip::tcp::socket(*sincronizador->interno_->servico_io)) {}

Socket::~Socket() {}

void Socket::Conecta(const std::string& endereco, const std::string& porta) {
  boost::asio::ip::tcp::resolver resolver(*sincronizador_->interno_->servico_io);
  auto endereco_resolvido = resolver.resolve({endereco, porta});
  boost::asio::connect(*socket_, endereco_resolvido);
}

void Socket::Fecha() {
  socket_->close();
}

void Socket::Envia(const std::vector<char>& dados, CallbackEnvio callback_envio_cliente) {
  boost::asio::async_write(*socket_.get(),
                           boost::asio::buffer(dados),
                           [callback_envio_cliente] (const boost::system::error_code& ec, std::size_t bytes_enviados) {
   callback_envio_cliente(Erro(ec), bytes_enviados);
 });
}

void Socket::Recebe(std::vector<char>* dados, CallbackRecepcao callback_recepcao_cliente) {
  socket_->async_receive(boost::asio::buffer(*dados),
      [callback_recepcao_cliente](const boost::system::error_code& ec, std::size_t bytes_recebidos) {
    callback_recepcao_cliente(Erro(ec), bytes_recebidos);
  });
}

//----------
// Aceitador
//----------
Aceitador::Aceitador(Sincronizador* sincronizador) : sincronizador_(sincronizador) {}
Aceitador::~Aceitador() {}

bool Aceitador::Liga(int porta,
          Socket* socket_cliente,
          CallbackConexaoCliente callback_conexao_cliente) {
  aceitador_.reset(new boost::asio::ip::tcp::acceptor(
      *sincronizador_->interno_->servico_io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), porta)));
  // Quando um cliente for recebido, CallbackConexao sera chamado.
  aceitador_->async_accept(
      *socket_cliente->socket_.get(),
      std::bind(&Aceitador::CallbackConexao,
                std::placeholders::_1,  // ec, a ser preenchido.
                aceitador_.get(),
                callback_conexao_cliente)
  );
  return true;
}

bool Aceitador::Ligado() const { return aceitador_ != nullptr; }

void Aceitador::Desliga() {
  aceitador_.reset();
}

// static
void Aceitador::CallbackConexao(
    boost::system::error_code ec,
    boost::asio::ip::tcp::acceptor* aceitador,
    CallbackConexaoCliente callback_conexao_cliente) {
  // Chama callback do cliente com erro recebido.
  Socket* novo_socket_cliente = callback_conexao_cliente(Erro(ec));
  if (novo_socket_cliente != nullptr) {
    // Se recebeu socket, chama de novo.
    aceitador->async_accept(
        *novo_socket_cliente->socket_.get(),
        std::bind(CallbackConexao,
                  std::placeholders::_1,  // ec, a ser preenchido.
                  aceitador,
                  callback_conexao_cliente));
  }
}

}  // namespace net
