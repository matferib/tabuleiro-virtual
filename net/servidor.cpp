#include <algorithm>
#include "net/servidor.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

namespace net {

Servidor::Servidor(ntf::CentralNotificacoes* central) {
  central->RegistraReceptor(this);
  central->RegistraReceptorRemoto(this);
  central_ = central;
  // tamanho maximo da mensagem: 1MB.
  buffer_.resize(1 * 1024 * 1024);
}

bool Servidor::TrataNotificacao(const ntf::Notificacao& notificacao) {
  if (notificacao.tipo() == ntf::TN_TEMPORIZADOR) {
    if (Ligado()) {
      servico_io_.poll_one();
    }
    return true;
  } else if (notificacao.tipo() == ntf::TN_INICIAR) {
    Liga();
    return true;
  } else if (notificacao.tipo() == ntf::TN_SAIR) {
    Desliga();
    return true;
  } else if (notificacao.tipo() == ntf::TN_TABULEIRO) {
    for (auto* c : clientes_pendentes_) {
      EnviaDadosCliente(c, notificacao.SerializeAsString());
      RecebeDadosCliente(c);
      clientes_.push_back(c);
    }
    clientes_pendentes_.clear();
  }
  return false;
}

bool Servidor::TrataNotificacaoRemota(const ntf::Notificacao& notificacao) {
  return false;
}

bool Servidor::Ligado() const {
  return aceitador_ != nullptr;
}

void Servidor::Liga() {
  try {
    cliente_.reset(new boost::asio::ip::tcp::socket(servico_io_));
    aceitador_.reset(new boost::asio::ip::tcp::acceptor(
        servico_io_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 11223)));
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
    for (auto* c : clientes_pendentes_) {
      delete c;
    }
  }
}

void Servidor::EsperaCliente() {
  aceitador_->async_accept(*cliente_, [this](boost::system::error_code ec) {
    if (!ec) {
      std::cout << "Recebendo cliente..." << std::endl;
      clientes_pendentes_.push_back(cliente_.release());
      cliente_.reset(new boost::asio::ip::tcp::socket(servico_io_));
      auto* notificacao = new ntf::Notificacao;
      notificacao->set_tipo(ntf::TN_CLIENTE_PENDENTE);
      central_->AdicionaNotificacao(notificacao);
      EsperaCliente();
    } else {
      std::cout << "Recebendo erro..." << ec.message() << std::endl;
    }
  });
}

void Servidor::EnviaDadosCliente(boost::asio::ip::tcp::socket* cliente, const std::string& dados) {
  buffer_.assign(dados.begin(), dados.end());
  cliente->send(boost::asio::buffer(buffer_));
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
      }
  );
}

}  // namespace net
