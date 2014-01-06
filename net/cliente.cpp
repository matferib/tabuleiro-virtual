#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/asio.hpp>
#include <vector>
#include <string>

#include "log/log.h"
#include "net/cliente.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

using boost::asio::ip::tcp;

namespace net {

Cliente::Cliente(boost::asio::io_service* servico_io, ntf::CentralNotificacoes* central) {
  servico_io_ = servico_io;
  central_ = central;
  central_->RegistraReceptor(this);
  // tamanho maximo da mensagem: 1MB.
  buffer_.resize(1 * 1024 * 1024);
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
  }
  return false;
}

bool Cliente::TrataNotificacaoRemota(const ntf::Notificacao& notificacao) {
  EnviaDados(notificacao.SerializeAsString());
  return true;
}

void Cliente::EnviaDados(const std::string& dados) {
  size_t bytes_enviados = socket_->send(boost::asio::buffer(dados.c_str(), dados.size()));
  if (bytes_enviados != dados.size()) {
    LOG(ERROR) << "Erro enviando dados, enviado: " << bytes_enviados;
  } else {
    LOG(INFO) << "Enviei " << dados.size() << " bytes pro servidor.";
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
    LOG(INFO) << "Conexão bem sucedida";
  } catch (std::exception& e) {
    socket_.reset();
    auto* notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_RESPOSTA_CONEXAO);
    notificacao->set_erro(e.what());
    central_->AdicionaNotificacao(notificacao);
    LOG(INFO) << "Falha de conexão";
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
  LOG(INFO) << "Cliente::RecebeDados";
  socket_->async_receive(
    boost::asio::buffer(buffer_),
    [this](boost::system::error_code ec, std::size_t bytes_recebidos) {
      if (ec) {
        LOG(ERROR) << "Erro recebendo dados: " << ec.message();
        Desconecta();
        return;
      }
      // Recebe mensagem.
      auto* notificacao = new ntf::Notificacao;
      if (notificacao->ParseFromString(std::string(buffer_.begin(), buffer_.begin() + bytes_recebidos))) {
        central_->AdicionaNotificacao(notificacao);
      } else {
        // TODO adicionar alguma coisa aqui.
        LOG(ERROR) << "Erro ParseFromString recebendo dados do servidor.";
        delete notificacao;
      }
      RecebeDados();
    }
  );
}

}  // namespace net
