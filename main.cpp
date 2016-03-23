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

#if USAR_GFLAGS
DEFINE_bool(iluminacao_por_pixel, true, "Se verdadeiro, usa luz por pixel (caro mas melhor).");
DEFINE_bool(mapeamento_de_sombras, true, "Se verdadeiro, usa mapemento de sombras caro mas melhor)..");
DEFINE_string(tabuleiro, "", "Se nao vazio, carrega o tabuleiro passado ao iniciar.");
#endif

using namespace std;

namespace {
void CarregaConfiguracoes(ent::OpcoesProto* proto) {
  try {
    arq::LeArquivoAsciiProto(arq::TIPO_CONFIGURACOES, "configuracoes.asciiproto", proto);
#if USAR_GFLAGS
    proto->set_iluminacao_por_pixel(FLAGS_iluminacao_por_pixel);
    proto->set_mapeamento_sombras(FLAGS_mapeamento_de_sombras);
#else
    proto->set_iluminacao_por_pixel(true);
    proto->set_mapeamento_sombras(true);
#endif
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
      ifg::qt::Principal::Cria(argc, argv, &tabuleiro, &texturas, &teclado_mouse, &central));
  ifg::qt::InterfaceGraficaQt igqt(p.get(), &teclado_mouse, &tabuleiro, &central);
#if USAR_GLAGS
  if (!FLAGS_tabuleiro.empty()) {
    // Carrega o tabuleiro.
    auto* n = ntf::NovaNotificacao(ntf::TN_DESERIALIZAR_TABULEIRO);
    n->set_endereco(FLAGS_tabuleiro);
    central.AdicionaNotificacao(n);
  }
#else
  if (argc >= 2 && argv[1][0] != '-') {
    // Carrega o tabuleiro.
    auto* n = ntf::NovaNotificacao(ntf::TN_DESERIALIZAR_TABULEIRO);
    n->set_endereco(argv[1]);
    central.AdicionaNotificacao(n);
  }
#endif
  try {
    p->Executa();
  }
  catch (exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
