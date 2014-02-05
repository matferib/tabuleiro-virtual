#include <algorithm>

#include "log/log.h"
#include "net/servidor.h"
#include "net/util.h"
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

Servidor::~Servidor() {
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
      VLOG(1) << "Enviando tabuleiro para cliente pendente";
      EnviaDadosCliente(c->socket.get(), ns);
      RecebeDadosCliente(c);
      clientes_.push_back(c);
    }
    clientes_pendentes_.clear();
  } else {
    for (auto* c : clientes_) {
      VLOG(1) << "Enviando notificacao para cliente";
      EnviaDadosCliente(c->socket.get(), ns);
    }
  }

  return true;
}

bool Servidor::Ligado() const {
  return aceitador_ != nullptr;
}

void Servidor::Liga() {
  VLOG(1) << "Ligando servidor.";
  try {
    cliente_.reset(new Cliente(new boost::asio::ip::tcp::socket(*servico_io_)));
    aceitador_.reset(new boost::asio::ip::tcp::acceptor(
        *servico_io_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 11223)));
    central_->RegistraReceptorRemoto(this);
    EsperaCliente();
    VLOG(1) << "Servidor ligado.";
  } catch(std::exception& e) {
    LOG(ERROR) << "Erro ligando servidor: " << e.what();
    // TODO fazer o tipo de erro e tratar notificacao em algum lugar.
    auto* notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_ERRO);
    notificacao->set_erro(e.what());
    central_->AdicionaNotificacao(notificacao);
  }
}

void Servidor::Desliga() {
  VLOG(1) << "Desligando servidor.";
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
  VLOG(1) << "Servidor desligado.";
}

void Servidor::EsperaCliente() {
  aceitador_->async_accept(*cliente_->socket.get(), [this](boost::system::error_code ec) {
    if (!ec) {
      VLOG(1) << "Recebendo cliente...";
      clientes_pendentes_.push_back(cliente_.release());
      cliente_.reset(new Cliente(new boost::asio::ip::tcp::socket(*servico_io_)));
      auto* notificacao = ntf::NovaNotificacao(ntf::TN_SERIALIZAR_TABULEIRO);
      central_->AdicionaNotificacao(notificacao);
      EsperaCliente();
    } else {
      LOG(ERROR) << "Recebendo erro..." << ec.message();
    }
  });
}

void Servidor::EnviaDadosCliente(boost::asio::ip::tcp::socket* cliente, const std::string& dados) {
  std::vector<char> dados_codificados(CodificaDados(dados));
  size_t bytes_enviados = cliente->send(boost::asio::buffer(dados_codificados));
  if (bytes_enviados != dados_codificados.size() ) {
    LOG(ERROR) << "Erro enviando dados, enviados: " << bytes_enviados;
  } else {
    VLOG(2) << "Enviei " << dados.size() << " bytes pro cliente.";
  }
}

void Servidor::DesconectaCliente(Cliente* cliente) {
  clientes_.erase(std::find(clientes_.begin(), clientes_.end(), cliente));
  delete cliente;
}

void Servidor::RecebeDadosCliente(Cliente* cliente) {
  cliente->socket->async_receive(
    boost::asio::buffer(buffer_),
    [this, cliente](boost::system::error_code ec, std::size_t bytes_recebidos) {
      if (ec) {
        // remove o cliente.
        VLOG(1) << "Removendo cliente: " << ec.message();
        DesconectaCliente(cliente);
        return;
      }
      auto buffer_inicio = buffer_.begin();
      auto buffer_fim = buffer_inicio + bytes_recebidos;
      do {
        if (cliente->a_receber_ == 0) {
          if (bytes_recebidos < 4) {
            LOG(ERROR) << "Erro recebendo dados de um cliente, msg menor que tamanho.";
            DesconectaCliente(cliente);
            return;
          }
          cliente->a_receber_ = DecodificaTamanho(buffer_);
          buffer_inicio += 4;
        }
        VLOG(2) << "Recebi " << bytes_recebidos << " dados de um cliente";
        if (buffer_fim - buffer_inicio >= cliente_->a_receber_) {
          // Quantidade de dados recebida eh maior ou igual ao esperado (por exemplo, ao receber duas mensagens juntas).
          cliente->buffer_notificacao.insert(cliente->buffer_notificacao.end(), buffer_inicio, buffer_inicio + cliente->a_receber_);
          // Decodifica mensagem e poe na central.
          std::unique_ptr<ntf::Notificacao> notificacao(new ntf::Notificacao);
          if (!notificacao->ParseFromString(cliente->buffer_notificacao)) {
            LOG(ERROR) << "Erro ParseFromString recebendo dados de um cliente.";
            DesconectaCliente(cliente);
            return;
          }
          // Envia a notificacao para os outros clientes.
          for (auto* c : clientes_) {
            if (c == cliente) {
              // Nao envia para o cliente original.
              continue;
            }
            EnviaDadosCliente(c->socket.get(), cliente->buffer_notificacao);
          }
          // Processa localmente.
          central_->AdicionaNotificacao(notificacao.release());
          cliente->buffer_notificacao.clear();
          buffer_inicio += cliente->a_receber_;
          cliente->a_receber_ = 0;
        } else {
          VLOG(2) << "Recebendo notificacao parcial";
          // Quantidade de dados recebida eh menor que o esperado. Poe no buffer
          // e sai.
          cliente->buffer_notificacao.insert(cliente->buffer_notificacao.end(), buffer_inicio, buffer_fim);
          cliente->a_receber_ -= (buffer_fim - buffer_inicio);
          buffer_inicio = buffer_fim;
        }
      } while (buffer_inicio != buffer_fim);
      VLOG(2) << "Tudo recebido";
      RecebeDadosCliente(cliente);
    }
  );
}

}  // namespace net
