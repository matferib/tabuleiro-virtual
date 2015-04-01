#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>

#include "net/socket.h"

namespace net {

//----------
// SocketUdp
//----------
SocketUdp::SocketUdp(Sincronizador* sincronizador)
    : socket_(new boost::asio::ip::udp::socket(*sincronizador->Servico())) {}

SocketUdp::SocketUdp(Sincronizador* sincronizador, int porta) : SocketUdp(sincronizador) {
  boost::asio::ip::udp::endpoint endereco(boost::asio::ip::udp::v4(), porta);
  socket_.reset(new boost::asio::ip::udp::socket(*sincronizador->Servico(), endereco));
}

SocketUdp::~SocketUdp() {}

void SocketUdp::Abre() {
  boost::system::error_code erro;
  socket_->open(boost::asio::ip::udp::v4(), erro);
  boost::asio::socket_base::broadcast option(true);
  socket_->set_option(option);
}

bool SocketUdp::Aberto() const { return socket_->is_open(); }

void SocketUdp::Fecha() {
  boost::system::error_code ec;
  socket_->close(ec);
}

void SocketUdp::Envia(int porta, const std::vector<char>& dados, CallbackEnvio callback_envio_cliente) {
  socket_->async_send_to(
      boost::asio::buffer(dados),
      boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("255.255.255.255"), porta),
      [callback_envio_cliente] (const boost::system::error_code& ec, std::size_t bytes_enviados) {
    callback_envio_cliente(Erro(ec), bytes_enviados);
  });
}

void SocketUdp::Recebe(
    std::vector<char>* dados, boost::asio::ip::udp::endpoint* endereco, CallbackRecepcao callback_recepcao_cliente) {
  socket_->async_receive_from(
      boost::asio::buffer(*dados),
      *endereco,
      [callback_recepcao_cliente] (const boost::system::error_code& ec, std::size_t bytes_enviados) {
    callback_recepcao_cliente(Erro(ec), bytes_enviados);
  });
}

}  // namespace net
