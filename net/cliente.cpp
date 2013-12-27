#include <algorithm>
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
  if () {
  } else {
  }
  return false;
}

void Cliente::Conecta() {
  try {
  } catch(std::exception& e) {
  }
}

void Cliente::Desconecta() {
}

}  // namespace net
