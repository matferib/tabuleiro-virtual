#include <algorithm>
#include "net/cliente.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

using boost::asio::ip::tcp;

namespace net {

Cliente::Cliente(ntf::CentralNotificacoes* central) {
  central->RegistraReceptor(this);
  central_ = central;
}

bool Cliente::TrataNotificacao(const ntf::Notificacao& notificacao) {
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
