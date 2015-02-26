/** @file main.cpp o inicio de tudo. Responsavel por instanciar a interface grafica principal. */

#include <iostream>
#include <memory>
#include <stdexcept>

#include <boost/asio.hpp>
#include "gltab/gl.h"
#include "ifg/qt/principal.h"
#include "net/cliente.h"
#include "net/servidor.h"
#include "ntf/notificacao.h"
#include "log/log.h"
#include "tex/texturas.h"
//#include "ifg/qt/texturas.h"

using namespace std;

int main(int argc, char** argv) {
  meulog::Inicializa(&argc, &argv);
  LOG(INFO) << "Iniciando programa: LOG LIGADO";
  boost::asio::io_service servico_io;
  ntf::CentralNotificacoes central;
  net::Servidor servidor(&servico_io, &central);
  net::Cliente cliente(&servico_io, &central);
  tex::Texturas texturas(&central);
  ent::Tabuleiro tabuleiro(&texturas, &central);
  std::unique_ptr<ifg::qt::Principal> p(ifg::qt::Principal::Cria(argc, argv, &tabuleiro, &texturas, &central));
  if (argc == 2) {
    // Carrega o tabuleiro.
    auto* n = ntf::NovaNotificacao(ntf::TN_DESERIALIZAR_TABULEIRO);
    n->set_endereco(argv[1]);
    central.AdicionaNotificacao(n);
  }
  try {
    p->Executa();
  }
  catch (exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
