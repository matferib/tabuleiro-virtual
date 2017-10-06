/** @file main.cpp o inicio de tudo. Responsavel por instanciar a interface grafica principal. */

#include <iostream>
#include <memory>
#include <stdexcept>
#if USAR_GFLAGS
#include <gflags/gflags.h>
#endif

#include <QDir>
#include <QLocale>
#include <Qt>
#include <QApplication>
#include <boost/asio.hpp>
#include "arq/arquivo.h"
#include "ent/tabelas.h"
#include "ent/tabuleiro.h"
#include "ent/tabuleiro_interface.h"
//#include "gltab/gl.h"
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
DEFINE_string(tabuleiro, "", "Se nao vazio, carrega o tabuleiro passado ao iniciar.");
#endif

using namespace std;

namespace {
void CarregaConfiguracoes(ent::OpcoesProto* proto) {
  try {
    arq::LeArquivoAsciiProto(arq::TIPO_CONFIGURACOES, "configuracoes.asciiproto", proto);
    LOG(INFO) << "Carregando opcoes de arquivo: ";
  } catch (...) {
    proto->CopyFrom(ent::OpcoesProto::default_instance());
    LOG(INFO) << "Carregando opcoes padroes.";
  }
  if (!proto->has_iluminacao_por_pixel()) {
#if USAR_GFLAGS
    proto->set_iluminacao_por_pixel(FLAGS_iluminacao_por_pixel);
#else
    proto->set_iluminacao_por_pixel(true);
#endif
  }
  LOG(INFO) << "Opcoes inciais: " << proto->ShortDebugString();
}
}  // namespace


int main(int argc, char** argv) {
#if USAR_GLOG
  meulog::Inicializa(&argc, &argv);
  //google::ParseCommandLineFlags(&argc, &argv, true);
#endif
#if USAR_GFLAGS
  google::ParseCommandLineFlags(&argc, &argv, true);
#endif
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#if __APPLE__
  std::string x_path(argv[0]);
  bool bundle = x_path.find("Contents/MacOS") != std::string::npos;
  if (bundle) {
    //QStringList list(dir.absolutePath() + "../Frameworks");
    //list.append(dir.absolutePath() + "../Frameworks/qtplugins");
    QStringList list("/Applications/TabuleiroVirtual.app/Contents/Frameworks");
    list.append("/Applications/TabuleiroVirtual.app/Contents/Frameworks/qtplugins");
    QCoreApplication::setLibraryPaths(list);
  }
  QApplication q_app(argc, argv);
  QDir dir(QCoreApplication::applicationDirPath());
  LOG(ERROR) << "app dir: " << dir.absolutePath().toStdString();
#endif

  LOG(INFO) << "Iniciando programa: LOG LIGADO";
  ent::Tabelas tabelas;
  ent::OpcoesProto opcoes;
  CarregaConfiguracoes(&opcoes);
  arq::Inicializa(dir.absolutePath().toStdString());
  boost::asio::io_service servico_io;
  net::Sincronizador sincronizador(&servico_io);
  ntf::CentralNotificacoes central;
  net::Servidor servidor(&sincronizador, &central);
  net::Cliente cliente(&sincronizador, &central);
  tex::Texturas texturas(&central);
  m3d::Modelos3d modelos3d(&central);
  ent::Tabuleiro tabuleiro(opcoes, tabelas, &texturas, &modelos3d, &central);
  ifg::TratadorTecladoMouse teclado_mouse(&central, &tabuleiro);
  //ent::InterfaceGraficaOpengl guiopengl(&teclado_mouse, &central);
  //tabuleiro.AtivaInterfaceOpengl(&guiopengl);
  std::unique_ptr<ifg::qt::Principal> p(
      ifg::qt::Principal::Cria(&q_app, tabelas, &tabuleiro, &texturas, &teclado_mouse, &central));
  ifg::qt::InterfaceGraficaQt igqt(tabelas, p.get(), &teclado_mouse, &tabuleiro, &central);
#if USAR_GFLAGS
  if (!FLAGS_tabuleiro.empty()) {
    // Carrega o tabuleiro.
    auto* n = ntf::NovaNotificacao(ntf::TN_DESERIALIZAR_TABULEIRO);
    n->set_endereco(std::string("://") + FLAGS_tabuleiro);
    central.AdicionaNotificacao(n);
  }
#else
  if (argc >= 2 && argv[1][0] != '-') {
    // Carrega o tabuleiro.
    auto* n = ntf::NovaNotificacao(ntf::TN_DESERIALIZAR_TABULEIRO);
    n->set_endereco(std::string("://") + argv[1]);
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
