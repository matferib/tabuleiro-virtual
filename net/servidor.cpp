#include "net/servidor.h"

using boost::asio::ip::tcp;

namespace net {

struct Servidor::Dados {
  boost::asio::io_service servico;
};

Servidor::Servidor(int porta) {

}

void Servidor::Liga() {

}

void Servidor::Desliga() {
}
 
}  // namespace net
