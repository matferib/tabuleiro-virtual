/** @file main.cpp o inicio de tudo. Responsavel por instanciar a interface grafica principal. */

#include <iostream>
#include <memory>
#include <stdexcept>

#include <boost/asio.hpp>
#include "ent/texturas.h"
#include "ifg/qt/principal.h"
#include "net/cliente.h"
#include "net/servidor.h"
#include "ntf/notificacao.h"
#include "log/log.h"

using namespace std;

int main(int argc, char** argv) {
  meulog::Inicializa(&argc, &argv);
  LOG(INFO) << "Iniciando programa: LOG LIGADO";
  boost::asio::io_service servico_io;
  ntf::CentralNotificacoes central;
  net::Servidor servidor(&servico_io, &central);
  net::Cliente cliente(&servico_io, &central);
  ent::Texturas texturas(&central);
  std::unique_ptr<ifg::qt::Principal> p(ifg::qt::Principal::Cria(argc, argv, &texturas, &central));
	try {
		p->Executa();
	}
	catch (exception& e) {
    std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}
