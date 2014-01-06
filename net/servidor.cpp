#include <algorithm>

#include "log/log.h"
#include "net/servidor.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

namespace net {

Servidor::Servidor(boost::asio::io_service* servico_io, ntf::CentralNotificacoes* central) {
  servico_io_ = servico_io;
  central_ = central;
  central_->RegistraReceptor(this);
  // tamanho maximo da mensagem: 1MB.
  buffer_.resize(1 * 1024 * 1024);
}

bool Servidor::TrataNotificacao(const ntf::Notificacao& notificacao) {
  if (notificacao.tipo() == ntf::TN_TEMPORIZADOR) {
    if (Ligado()) {
      servico_io_->poll_one();
    }
    return true;
  } else if (notificacao.tipo() == ntf::TN_INICIAR) {
    Liga();
    return true;
  } else if (notificacao.tipo() == ntf::TN_SAIR) {
    Desliga();
    return true;
  }
  return false;
}

bool Servidor::TrataNotificacaoRemota(const ntf::Notificacao& notificacao) {
  const std::string ns = notificacao.SerializeAsString();
  if (notificacao.tipo() == ntf::TN_DESERIALIZAR_TABULEIRO) {
    // Envia o tabuleiro a todos os clientes pendentes.
    for (auto* c : clientes_pendentes_) {
      LOG(INFO) << "Enviando tabuleiro para cliente pendente";
      EnviaDadosCliente(c, ns);
      RecebeDadosCliente(c);
      clientes_.push_back(c);
    }
    clientes_pendentes_.clear();
  } else {
    for (auto* c : clientes_) {
      VLOG(1) << "Enviando notificacao para cliente";
      EnviaDadosCliente(c, ns);
    }
  }

  return true;
}

bool Servidor::Ligado() const {
  return aceitador_ != nullptr;
}

void Servidor::Liga() {
  try {
    cliente_.reset(new boost::asio::ip::tcp::socket(*servico_io_));
    aceitador_.reset(new boost::asio::ip::tcp::acceptor(
        *servico_io_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 11223)));
    central_->RegistraReceptorRemoto(this);
    EsperaCliente();
  } catch(std::exception& e) {
    // TODO fazer o tipo de erro e tratar notificacao em algum lugar.
    auto* notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_ERRO);
    central_->AdicionaNotificacao(notificacao);
  }
}

void Servidor::Desliga() {
  if (!Ligado()) {
    LOG(ERROR) << "Servidor ja está desligado.";
    return;
  }
  central_->DesregistraReceptorRemoto(this);
  aceitador_.reset();
  for (auto* c : clientes_) {
    delete c;
  }
  for (auto* c : clientes_pendentes_) {
    delete c;
  }
}

void Servidor::EsperaCliente() {
  aceitador_->async_accept(*cliente_, [this](boost::system::error_code ec) {
    if (!ec) {
      LOG(INFO) << "Recebendo cliente...";
      clientes_pendentes_.push_back(cliente_.release());
      cliente_.reset(new boost::asio::ip::tcp::socket(*servico_io_));
      auto* notificacao = new ntf::Notificacao;
      notificacao->set_tipo(ntf::TN_SERIALIZAR_TABULEIRO);
      central_->AdicionaNotificacao(notificacao);
      EsperaCliente();
    } else {
      LOG(ERROR) << "Recebendo erro..." << ec.message();
    }
  });
}

void Servidor::EnviaDadosCliente(boost::asio::ip::tcp::socket* cliente, const std::string& dados) {
  size_t bytes_enviados = cliente->send(boost::asio::buffer(dados.c_str(), dados.size()));
  if (bytes_enviados != dados.size()) {
    LOG(ERROR) << "Erro enviando dados, enviado: " << bytes_enviados;
  } else {
    LOG(INFO) << "Enviei " << dados.size() << " bytes pro cliente.";
  }
}

void Servidor::RecebeDadosCliente(boost::asio::ip::tcp::socket* cliente) {
  cliente->async_receive(
    boost::asio::buffer(buffer_),  
    [this, cliente](boost::system::error_code ec, std::size_t bytes_transferred) {
      if (!ec) {
        std::string str(buffer_.begin(), buffer_.begin() + bytes_transferred);
        VLOG(1) << "Recebi dados de um cliente: " << str;
        auto* n = new ntf::Notificacao;
        if (n->ParseFromString(str)) {
          // Envia a notificacao para os outros clientes.
          for (auto* c : clientes_) {
            if (c == cliente) {
              // Nao envia para o cliente original.
              continue;
            }
            EnviaDadosCliente(c, str);
          }
          // Processa localmente.
          central_->AdicionaNotificacao(n);
        } else {
          // TODO adicionar alguma coisa aqui.
          LOG(ERROR) << "Erro ParseFromString recebendo dados do cliente.";
          delete n;
        }
        RecebeDadosCliente(cliente);
      } else {
        // remove o cliente.
        LOG(INFO) << "Removendo cliente: " << ec.message();
        clientes_.erase(std::find(clientes_.begin(), clientes_.end(), cliente));
        delete cliente;
      }
    }
  );
}

}  // namespace net
