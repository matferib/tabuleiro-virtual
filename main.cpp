/** @file main.cpp o inicio de tudo. Responsavel por instanciar a interface grafica principal. */

#include <iostream>
#include <memory>
#include <stdexcept>
#include "ifg/qt/principal.h"
#include "net/cliente.h"
#include "net/servidor.h"
#include "ntf/notificacao.h"

using namespace std;

int main(int argc, char** argv){
  ntf::CentralNotificacoes central;
  net::Servidor servidor(&central);
  net::Cliente cliente(&central);
  std::unique_ptr<ifg::qt::Principal> p(ifg::qt::Principal::Cria(argc, argv, &central));
	try {
		p->Executa();
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		return 1;
	}
	return 0;
}
