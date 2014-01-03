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

Cliente::Cliente(ntf::CentralNotificacoes* central) {
  central->RegistraReceptor(this);
  central_ = central;
  // tamanho maximo da mensagem: 1MB.
  buffer_.resize(1 * 1024 * 1024);
}

bool Cliente::TrataNotificacao(const ntf::Notificacao& notificacao) {
  if (notificacao.tipo() == ntf::TN_TEMPORIZADOR) {
    if (Ligado()) {
      servico_io_.poll_one();
    }
    return true;
  } else if (notificacao.tipo() == ntf::TN_CONECTAR) {
    Conecta(notificacao.endereco());
    return true;
  }
  return false;
}

void Cliente::Conecta(const std::string& endereco_str) {
  boost::asio::ip::tcp::resolver resolver(servico_io_);
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
  //assert(endereco_porta.size() >= 2, "Endereco porta invalido");
  try {
    socket_.reset(new boost::asio::ip::tcp::socket(servico_io_));
    auto endereco_resolvido = resolver.resolve({endereco_porta[0], endereco_porta[1]});
    //std::cout << "TESTE" << std::endl;
    boost::asio::connect(*socket_, endereco_resolvido);
    // Handler de leitura.
    RecebeDados();
    auto* notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_RESPOSTA_CONEXAO);
    central_->AdicionaNotificacao(notificacao);
  } catch (std::exception& e) {
    auto* notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_RESPOSTA_CONEXAO);
    notificacao->set_erro(e.what());
    central_->AdicionaNotificacao(notificacao);
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
  std::cout << "Desconectando..." << std::endl;
}

bool Cliente::Ligado() const {
  return socket_ == nullptr;
}

void Cliente::RecebeDados() {
  boost::asio::async_read(
    *socket_,
    boost::asio::buffer(buffer_),
    [this](boost::system::error_code ec, std::size_t quantidade) {
      if (ec) {
        Desconecta();
        return;
      }
      // Recebe mensagem.
      auto* notificacao = new ntf::Notificacao;
      if (notificacao->ParseFromString(std::string(buffer_.begin(), buffer_.end()))) {
        // Inverte os bits de remota e local.
        notificacao->set_local(true);
        notificacao->set_remota(false);
        central_->AdicionaNotificacao(notificacao);
      } else {
        // TODO adicionar alguma coisa aqui.
        LOG(ERROR) << "Erro recebendo mensagem";
        delete notificacao;
      }
      RecebeDados();
    }
  );
}

}  // namespace net
