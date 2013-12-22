#include <algorithm>
#include "net/servidor.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

using boost::asio::ip::tcp;

namespace net {

Servidor::Servidor(ntf::CentralNotificacoes* central) {
  central->RegistraReceptor(this);
  central_ = central;
  // tamanho maximo da mensagem: 1MB.
  buffer_.resize(1 * 1024 * 1024);
}

bool Servidor::TrataNotificacao(const ntf::Notificacao& notificacao) {
  if (notificacao.tipo() == ntf::TN_TEMPORIZADOR) {
    if (Ligado()) {
      servico_io_.poll_one();
    }
  } else if (notificacao.tipo() == ntf::TN_INICIAR) {
    Liga();
    return true;
  } else if (notificacao.tipo() == ntf::TN_SAIR) {
    Desliga();
    return true;
  }
  return false;
}

bool Servidor::Ligado() const {
  return aceitador_ != nullptr;
}

void Servidor::Liga() {
  try {
    cliente_.reset(new boost::asio::ip::tcp::socket(servico_io_));
    aceitador_.reset(new boost::asio::ip::tcp::acceptor(servico_io_, tcp::endpoint(tcp::v4(), 11223)));
    EsperaCliente();
  } catch(std::exception& e) {
    // TODO fazer o tipo de erro e tratar notificacao em algum lugar.
    auto* notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_ERRO);
    central_->AdicionaNotificacao(notificacao);
  }
}

void Servidor::Desliga() {
  if (Ligado()) {
    servico_io_.stop();
    aceitador_.reset();
    for (auto* c : clientes_) {
      delete c;
    }
  }
}

void Servidor::EsperaCliente() {
  aceitador_->async_accept(*cliente_, [this](boost::system::error_code ec) {
    if (!ec) {
      std::cout << "Recebendo cliente..." << std::endl;
      RecebeDadosCliente(cliente_.get());
      clientes_.push_back(cliente_.release());
      cliente_.reset(new boost::asio::ip::tcp::socket(servico_io_));
      EsperaCliente();
    } else {
      std::cout << "Recebendo erro..." << ec.message() << std::endl;
    }
  });
}

void Servidor::RecebeDadosCliente(boost::asio::ip::tcp::socket* cliente) {
  cliente->async_receive(
      boost::asio::buffer(buffer_),  
      [this, cliente](boost::system::error_code ec, std::size_t bytes_transferred) {
    if (!ec) {
    std::string str(buffer_.begin(), buffer_.begin() + bytes_transferred);
      std::cout << "Recebi: " << str;
      RecebeDadosCliente(cliente);
    } else {
      // remove o cliente.
      std::cout << "Removendo cliente: " << ec.message() << std::endl;
      clientes_.erase(std::find(clientes_.begin(), clientes_.end(), cliente));
      delete cliente;
    }
  });
}

}  // namespace net
