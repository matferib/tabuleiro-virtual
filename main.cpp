/** @file main.cpp o inicio de tudo. Responsavel por instanciar a interface grafica principal. */

#include <iostream>
#include <memory>
#include <stdexcept>

#include <boost/asio.hpp>
#include "arq/arquivo.h"
#include "ent/tabuleiro_interface.h"
#include "gltab/gl.h"
#include "ifg/qt/principal.h"
#include "ifg/tecladomouse.h"
#include "ifg/qt/qt_interface.h"
#include "net/cliente.h"
#include "net/servidor.h"
#include "ntf/notificacao.h"
#include "log/log.h"
#include "tex/texturas.h"

using namespace std;

namespace {
void CarregaConfiguracoes(ent::OpcoesProto* proto) {
  try {
    arq::LeArquivoAsciiProto(arq::TIPO_CONFIGURACOES, "configuracoes.asciiproto", proto);
    LOG(INFO) << "Carregando opcoes de arquivo.";
  } catch (...) {
    proto->CopyFrom(ent::OpcoesProto::default_instance());
    LOG(INFO) << "Carregando opcoes padroes.";
  }
}
}  // namespace


int main(int argc, char** argv) {
  meulog::Inicializa(&argc, &argv);
  LOG(INFO) << "Iniciando programa: LOG LIGADO";
  ent::OpcoesProto opcoes;
  CarregaConfiguracoes(&opcoes);
  arq::Inicializa();
  boost::asio::io_service servico_io;
  net::Sincronizador sincronizador(&servico_io);
  ntf::CentralNotificacoes central;
  net::Servidor servidor(&sincronizador, &central);
  net::Cliente cliente(&sincronizador, &central);
  tex::Texturas texturas(&central);
  m3d::Modelos3d modelos3d;
  ent::Tabuleiro tabuleiro(opcoes, &texturas, &modelos3d, &central);
  ifg::TratadorTecladoMouse teclado_mouse(&central, &tabuleiro);
  //ent::InterfaceGraficaOpengl guiopengl(&teclado_mouse, &central);
  //tabuleiro.AtivaInterfaceOpengl(&guiopengl);
  std::unique_ptr<ifg::qt::Principal> p(
      ifg::qt::Principal::Cria(argc, argv, opcoes.anti_aliasing(), &tabuleiro, &texturas, &teclado_mouse, &central));
  ifg::qt::InterfaceGraficaQt igqt(p.get(), &teclado_mouse, &tabuleiro, &central);
  if (argc >= 2 && argv[1][0] != '-') {
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
