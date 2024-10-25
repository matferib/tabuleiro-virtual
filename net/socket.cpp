#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>

#include "absl/strings/str_format.h"

#include "log/log.h"
#include "net/socket.h"
#include "net/util.h"

namespace net {
namespace {
}  // namespace

//-----
// Erro
//-----
struct Erro::Interno {
  boost::system::error_code ec;
};

const Erro ConverteErro(const boost::system::error_code& ec) {
  return Erro((void*)&ec);
}

Erro::Erro(void* dependente_plataforma) : Erro() {
  interno_->ec = *((boost::system::error_code*)dependente_plataforma);
  erro_ = interno_->ec.value() != boost::system::errc::success;
  msg_ = interno_->ec.message();
  msg_ += ", cod: " + to_string(interno_->ec.value());
}
Erro::Erro(const std::string& msg) : Erro(true) {
  msg_ = msg;
}
Erro::Erro(bool erro) : interno_(new Interno), erro_(erro) {}

bool Erro::ConexaoFechada() const {
  // O valor 2 esta sendo recebido quando a conexao eh fechada (eof).
  return interno_->ec == boost::system::errc::connection_aborted || interno_->ec == boost::system::errc::connection_reset || interno_->ec.value() == boost::asio::error::eof;
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

int Sincronizador::Roda() {
  return interno_->servico_io->poll();
}

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
  boost::asio::ip::udp::endpoint endereco(boost::asio::ip::udp::v6(), porta);
  interno_.reset(new Interno(new boost::asio::ip::udp::socket(*sincronizador->interno_->servico_io, endereco)));
}

SocketUdp::~SocketUdp() {}

void SocketUdp::Abre() {
  boost::system::error_code erro;
  interno_->socket->open(boost::asio::ip::udp::v6(), erro);
  boost::asio::socket_base::broadcast option(true);
  interno_->socket->set_option(option);
}

bool SocketUdp::Aberto() const { return interno_->socket->is_open(); }

void SocketUdp::Fecha() {
  boost::system::error_code ec;
  interno_->socket->close(ec);
}

void SocketUdp::Envia(int porta, const std::string& dados, CallbackEnvio callback_envio_cliente) {
  interno_->socket->async_send_to(
      boost::asio::buffer(dados),
      boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("255.255.255.255"), porta),
      [callback_envio_cliente, &dados] (const boost::system::error_code& ec, std::size_t bytes_enviados) {
    VLOG(2) << "UDP Enviados: " << bytes_enviados << ", buffer: " << dados.size() << ", erro? " << ec.message();
    callback_envio_cliente(ConverteErro(ec), bytes_enviados);
  });
}

void SocketUdp::Recebe(
    std::string* dados, std::string* endereco, CallbackRecepcao callback_recepcao_cliente) {
  auto* endpoint = new boost::asio::ip::udp::endpoint;
  interno_->socket->async_receive_from(
      boost::asio::buffer(&(*dados)[0], dados->size()),
      *endpoint,
      [endpoint, endereco, callback_recepcao_cliente, dados] (const boost::system::error_code& ec, std::size_t bytes_enviados) {
    VLOG(2) << "UDP Recebidos: " << bytes_enviados << ", buffer: " << dados->size() << ", erro? " << ec.message();
    endereco->assign(endpoint->address().to_string());
    delete endpoint;
    callback_recepcao_cliente(ConverteErro(ec), bytes_enviados);
  });
}

//-------
// Socket
//-------
struct Socket::Interno {
  explicit Interno(boost::asio::ip::tcp::socket* socket) : socket(socket) {}

  std::string IpString() const {
    return socket == nullptr ? "null" : socket->remote_endpoint().address().to_string();
  }

  std::unique_ptr<boost::asio::ip::tcp::socket> socket;
};

Socket::Socket(Sincronizador* sincronizador)
    : sincronizador_(sincronizador),
      interno_(new Interno(new boost::asio::ip::tcp::socket(*sincronizador->interno_->servico_io))) {
  try {
    boost::asio::ip::tcp::no_delay option(true);
    interno_->socket->set_option(option);
    LOG(INFO) << "No delay setado com sucesso";
  } catch (const std::exception& e) {
    LOG(WARNING) << "Nao consegui setar no_delay, realtime pode ser comprometido";
  }
}

Socket::~Socket() {}

void Socket::Conecta(const std::string& endereco, const std::string& porta) {
  boost::asio::ip::tcp::resolver resolver(*sincronizador_->interno_->servico_io);
  auto endereco_resolvido = resolver.resolve({endereco, porta});
  boost::asio::connect(*interno_->socket, endereco_resolvido);
  // http://tldp.org/HOWTO/html_single/TCP-Keepalive-HOWTO/#preventingdisconnection.
  boost::asio::socket_base::keep_alive option(true);
  interno_->socket->set_option(option);

  {
    boost::asio::socket_base::keep_alive option;
    interno_->socket->get_option(option);
    LOG(INFO) << "keep alive: " << option.value();
  }
}

void Socket::Fecha() {
  interno_->socket->close();
}

void Socket::PortaLocal(int porta_local) {
  interno_->socket->open(boost::asio::ip::tcp::v6());
  boost::system::error_code ec;
  interno_->socket->bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v6(), porta_local), ec);
  if (ec) {
    throw std::logic_error(absl::StrFormat("Erro no bind do cliente na porta %d", porta_local));
  }
}

void Socket::Envia(const std::string& dados, CallbackEnvio callback_envio_cliente) {
#if 1
  // Write envia tudo em uma so chamada.
  boost::asio::async_write(
      *interno_->socket.get(),
      boost::asio::buffer(dados),
      [callback_envio_cliente, &dados] (const boost::system::error_code& ec, std::size_t bytes_enviados) {
    VLOG(1) << "TCP Enviados " << bytes_enviados << ", buffer: " << dados.size() << ", erro? " << ec.message();
    callback_envio_cliente(ConverteErro(ec), bytes_enviados);
 });
#else
  // Send tem que enviar cada parte.
  int* enviado = new int(0);
  auto* callback_send = new std::function<void(const boost::system::error_code& ec, std::size_t bytes_enviados)>;
  (*callback_send) = [this, callback_envio_cliente, &dados, enviado, callback_send] (const boost::system::error_code& ec, std::size_t bytes_enviados) {
    *enviado += bytes_enviados;
    if (*enviado < dados.size() && !ec) {
      VLOG(1) << "TCP parcialmente enviado: " << *enviado << " de buffer tamanho: " << dados.size();
      interno_->socket->async_send(boost::asio::buffer(dados.data() + (*enviado), dados.size() - *enviado), *callback_send);
    } else {
      VLOG(1) << "TCP finalizado, enviado " << *enviado << " de buffer: " << dados.size() << ", erro? " << ec.message();
      callback_envio_cliente(ConverteErro(ec), bytes_enviados);
      delete callback_send;
      delete enviado;
    }
  };
  interno_->socket->async_send(boost::asio::buffer(dados), *callback_send);
#endif
}

void Socket::Recebe(std::string* dados, CallbackRecepcao callback_recepcao_cliente) {
  boost::asio::async_read(
      *interno_->socket.get(),
      boost::asio::buffer(&(*dados)[0], dados->size()),
      [callback_recepcao_cliente, dados](const boost::system::error_code& ec, std::size_t bytes_recebidos) {
    VLOG(1) << "TCP Recebidos " << bytes_recebidos << ", buffer: " << dados->size() << ", erro? " << ec.message();
    callback_recepcao_cliente(ConverteErro(ec), bytes_recebidos);
  });
}

std::string Socket::IpString() const {
  return interno_->IpString();
}

//----------
// Aceitador
//----------
struct Aceitador::Interno {
  std::unique_ptr<boost::asio::ip::tcp::acceptor> aceitador;
};
Aceitador::Aceitador(Sincronizador* sincronizador) : sincronizador_(sincronizador), interno_(new Interno) {

}
Aceitador::~Aceitador() {}

bool Aceitador::Liga(int porta,
          Socket* socket_cliente,
          CallbackConexaoCliente callback_conexao_cliente) {
  interno_->aceitador.reset(new boost::asio::ip::tcp::acceptor(
      *sincronizador_->interno_->servico_io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v6(), porta)));
  // Funcao recursiva para ser chamada a cada conexao.
  // Primeiro a gente cria o objeto para poder passa-lo para ele mesmo no lambda.
  auto* callback_conexao = new std::function<void(boost::system::error_code, CallbackConexaoCliente)>;
  (*callback_conexao) = [this, callback_conexao] (boost::system::error_code ec,
                                                  CallbackConexaoCliente callback_conexao_cliente) {
    // Chama callback do cliente com erro recebido.
    Socket* novo_socket_cliente = callback_conexao_cliente(Erro(&ec));
    if (novo_socket_cliente != nullptr) {
      // Se recebeu socket, chama de novo.
      interno_->aceitador->async_accept(
          *novo_socket_cliente->interno_->socket.get(),
          std::bind(*callback_conexao,
                    std::placeholders::_1,  // ec, a ser preenchido.
                    callback_conexao_cliente));
    } else {
      delete callback_conexao;
    }
  };

  // Quando um cliente for recebido, CallbackConexao sera chamado.
  interno_->aceitador->async_accept(
      *socket_cliente->interno_->socket.get(),
      std::bind(*callback_conexao,
                std::placeholders::_1,  // ec, a ser preenchido.
                callback_conexao_cliente)
  );
  return true;
}

bool Aceitador::Ligado() const { return interno_->aceitador.get() != nullptr; }

void Aceitador::Desliga() {
  interno_->aceitador.reset();
}

}  // namespace net
