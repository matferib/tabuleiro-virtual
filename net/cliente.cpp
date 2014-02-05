#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/asio.hpp>
#include <vector>
#include <string>

#include "log/log.h"
#include "net/cliente.h"
#include "net/util.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

using boost::asio::ip::tcp;

namespace net {

Cliente::Cliente(boost::asio::io_service* servico_io, ntf::CentralNotificacoes* central) {
  servico_io_ = servico_io;
  central_ = central;
  central_->RegistraReceptor(this);
  // TODO fazer alguma verificacao disso.
  // tamanho maximo da mensagem: 1MB.
  buffer_.resize(1 * 1024 * 1024);
  // Tamanho maximo da notificacao: 10MB.
  buffer_notificacao_.reserve(10 * 1024 * 1024);
  a_receber_ = 0;
}

bool Cliente::TrataNotificacao(const ntf::Notificacao& notificacao) {
  if (notificacao.tipo() == ntf::TN_TEMPORIZADOR) {
    if (Ligado()) {
      servico_io_->poll_one();
    }
    return true;
  } else if (notificacao.tipo() == ntf::TN_CONECTAR) {
    Conecta(notificacao.endereco());
    return true;
  } else if (notificacao.tipo() == ntf::TN_DESCONECTAR) {
    Desconecta();
    return true;
  }
  return false;
}

bool Cliente::TrataNotificacaoRemota(const ntf::Notificacao& notificacao) {
  EnviaDados(notificacao.SerializeAsString());
  return true;
}

void Cliente::EnviaDados(const std::string& dados) {
  auto dados_codificados(CodificaDados(dados));
  size_t bytes_enviados = socket_->send(boost::asio::buffer(dados_codificados));
  if (bytes_enviados != dados_codificados.size()) {
    LOG(ERROR) << "Erro enviando dados, enviado: " << bytes_enviados;
  } else {
    VLOG(1) << "Enviei " << dados.size() << " bytes pro servidor.";
  }
}
void Cliente::Conecta(const std::string& endereco_str) {
  std::vector<std::string> endereco_porta;
  boost::split(endereco_porta, endereco_str, boost::algorithm::is_any_of(":"));
  if (endereco_porta.size() == 0) {
    // Endereco padrao.
    endereco_porta.push_back("localhost");
  } else if (endereco_porta[0].empty()) {
    endereco_porta[0] = "localhost";
  }
  if (endereco_porta.size() == 1) {
    // Porta padrao.
    endereco_porta.push_back("11223");
  }
  try {
    socket_.reset(new boost::asio::ip::tcp::socket(*servico_io_));
    boost::asio::ip::tcp::resolver resolver(*servico_io_);
    auto endereco_resolvido = resolver.resolve({endereco_porta[0], endereco_porta[1]});
    boost::asio::connect(*socket_, endereco_resolvido);
    // Handler de leitura.
    auto* notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_RESPOSTA_CONEXAO);
    central_->AdicionaNotificacao(notificacao);
    central_->RegistraReceptorRemoto(this);
    RecebeDados();
    VLOG(1) << "Conexão bem sucedida";
  } catch (std::exception& e) {
    socket_.reset();
    auto* notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_RESPOSTA_CONEXAO);
    notificacao->set_erro(e.what());
    central_->AdicionaNotificacao(notificacao);
    VLOG(1) << "Falha de conexão";
  }
}

void Cliente::Desconecta() {
  if (!Ligado()) {
    return;
  }
  socket_->close();
  socket_.reset();
  auto* notificacao = new ntf::Notificacao;
  notificacao->set_tipo(ntf::TN_DESCONECTADO);
  central_->AdicionaNotificacao(notificacao);
  central_->DesregistraReceptorRemoto(this);
  std::cout << "Desconectando..." << std::endl;
}

bool Cliente::Ligado() const {
  return socket_ != nullptr;
}

void Cliente::RecebeDados() {
  VLOG(2) << "Cliente::RecebeDados";
  socket_->async_receive(
    boost::asio::buffer(buffer_),
    [this](boost::system::error_code ec, std::size_t bytes_recebidos) {
      if (ec) {
        LOG(ERROR) << "Erro recebendo dados: " << ec.message();
        Desconecta();
        return;
      }
      auto buffer_inicio = buffer_.begin();
      auto buffer_fim = buffer_inicio + bytes_recebidos;
      do {
        if (a_receber_ == 0) {
          if (bytes_recebidos < 4) {
            LOG(ERROR) << "Erro recebendo tamanho de dados do servidor, bytes recebidos: " << bytes_recebidos;
            Desconecta();
            return;
          }
          a_receber_ = DecodificaTamanho(buffer_);
          buffer_inicio += 4;
          VLOG(2) << "Recebendo notificacao tamanho a receber: " << a_receber_ << ", total: " << bytes_recebidos;
        } else {
          VLOG(2) << "Continuando recepcao de notificao tamanho: " << a_receber_ << ", total: " << bytes_recebidos;
        }
        if (buffer_fim - buffer_inicio >= a_receber_) {
          VLOG(2) << "Recebendo notificacao completa";
          // Quantidade de dados recebida eh maior ou igual ao esperado (por exemplo, ao receber duas mensagens juntas).
          buffer_notificacao_.insert(buffer_notificacao_.end(), buffer_inicio, buffer_inicio + a_receber_);
          // Decodifica mensagem e poe na central.
          auto* notificacao = new ntf::Notificacao;
          if (!notificacao->ParseFromString(buffer_notificacao_)) {
            LOG(ERROR) << "Erro ParseFromString recebendo dados do servidor. Tamanho buffer_notificacao: " << buffer_notificacao_.size();
            delete notificacao;
            Desconecta();
            return;
          }
          central_->AdicionaNotificacao(notificacao);
          buffer_notificacao_.clear();
          buffer_inicio += a_receber_;
          a_receber_ = 0;
        } else {
          VLOG(2) << "Recebendo notificacao parcial";
          // Quantidade de dados recebida eh menor que o esperado. Poe no buffer
          // e sai.
          buffer_notificacao_.insert(buffer_notificacao_.end(), buffer_inicio, buffer_fim);
          a_receber_ -= (buffer_fim - buffer_inicio);
          buffer_inicio = buffer_fim;
        }
      } while (buffer_inicio != buffer_fim);
      VLOG(2) << "Tudo recebido";
      RecebeDados();
    }
  );
}

}  // namespace net
