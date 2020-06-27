/** @file main.cpp o inicio de tudo. Responsavel por instanciar a interface grafica principal. */

#if ANDROID
#include <android_native_app_glue.h>
#endif

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
#include <QSurfaceFormat>
#include <boost/asio.hpp>
#include "arq/arquivo.h"
#include "ent/tabelas.h"
#include "ent/tabuleiro.h"
#include "ent/tabuleiro_interface.h"
#include "ent/util.h"
#include "goog/stringprintf.h"
//#include "gltab/gl.h"
#include "ifg/qt/principal.h"
#include "ifg/tecladomouse.h"
#include "ifg/qt/qt_interface.h"
#include "m3d/m3d.h"
#include "net/cliente.h"
#include "net/servidor.h"
#include "ntf/notificacao.h"
#include "log/log.h"
#include "som/som.h"
#include "tex/texturas.h"

#if USAR_GFLAGS
DEFINE_bool(iluminacao_por_pixel, true, "Se verdadeiro, usa luz por pixel (caro mas melhor).");
DEFINE_string(tabuleiro, "", "Se nao vazio, carrega o tabuleiro passado ao iniciar.");
#endif

using namespace std;

#if 0
// Para capturar excecoes do QT.
class MyApp : public QApplication {
 public:
  MyApp(int& argc, char** argv) : QApplication(argc, argv) {}
  bool notify(QObject* receiver, QEvent* event) {
    try {
      return QApplication::notify(receiver, event);
    } catch (const std::exception& e) {
      receiver->dumpObjectInfo();
      LOG(INFO) << "exception: " << e.what();
    }
    return false;
  }
};
#endif

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

QSurfaceFormat Formato() {
  QSurfaceFormat formato;
  formato.setVersion(2, 1);
  formato.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
  formato.setRedBufferSize(8);
  formato.setGreenBufferSize(8);
  formato.setBlueBufferSize(8);
  // Nao faca isso! Isso aqui deixara a janela transparente, quebrando a transparencia.
  //formato.setAlphaBufferSize(8);
  formato.setDepthBufferSize(24);
  formato.setStencilBufferSize(1);
  formato.setRenderableType(QSurfaceFormat::OpenGL);
  formato.setSamples(2);
  return formato;
}

class SomEscopo {
 public:
  SomEscopo(const ent::OpcoesProto& opcoes) { som::Inicia(opcoes); }
  ~SomEscopo() { som::Finaliza(); }
};

}  // namespace

#if ANDROID
extern "C" {
void android_main(struct android_app* state) {
  int argc = 0;
  char* argv[] = {};
#else
int main(int argc, char** argv) {
#endif

#if USAR_GLOG
  meulog::Inicializa(&argc, &argv);
  //google::ParseCommandLineFlags(&argc, &argv, true);
#endif
#if USAR_GFLAGS
  google::ParseCommandLineFlags(&argc, &argv, true);
#endif
#if __APPLE__
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
  //MyApp q_app(argc, argv);
  QSurfaceFormat::setDefaultFormat(Formato());
  QApplication q_app(argc, argv);
  QDir dir(QCoreApplication::applicationDirPath());

  LOG(INFO) << "Iniciando programa: LOG LIGADO";
  // Arq::Inicializa tem que vir antes, porque os outros leem varias coisas de arquivos.
#if ANDROID
  arq::Inicializa(state->activity->env, state->activity->assetManager, state->activity->internalDataPath);
#else
  arq::Inicializa(dir.absolutePath().toStdString());
#endif
  ent::OpcoesProto opcoes;
  CarregaConfiguracoes(&opcoes);
  boost::asio::io_service servico_io;
  net::Sincronizador sincronizador(&servico_io);
  ntf::CentralNotificacoes central;
  ent::Tabelas tabelas(&central);
  net::Servidor servidor(&sincronizador, &central);
  net::Cliente cliente(&sincronizador, &central);
  tex::Texturas texturas(&central);
  m3d::Modelos3d modelos3d(&central);
  ent::Tabuleiro tabuleiro(opcoes, tabelas, &texturas, &modelos3d, &central);
  ifg::TratadorTecladoMouse teclado_mouse(&central, &tabuleiro);
  //ent::InterfaceGraficaOpengl guiopengl(&teclado_mouse, &central);
  //tabuleiro.AtivaInterfaceOpengl(&guiopengl);
  SomEscopo som(tabuleiro.Opcoes());

  std::unique_ptr<ifg::qt::Principal> p(
      ifg::qt::Principal::Cria(&q_app, tabelas, &tabuleiro, &modelos3d, &texturas, &teclado_mouse, &central));
  ifg::qt::InterfaceGraficaQt igqt(tabelas, p.get(), &teclado_mouse, &tabuleiro, &central);
#if USAR_GFLAGS
  if (!FLAGS_tabuleiro.empty()) {
    // Carrega o tabuleiro.
    auto n = ntf::NovaNotificacao(ntf::TN_DESERIALIZAR_TABULEIRO);
    n->set_endereco(std::string("://") + FLAGS_tabuleiro);
    central.AdicionaNotificacao(n.release());
  }
#else
  if (argc >= 2 && argv[1][0] != '-') {
    // Carrega o tabuleiro.
    auto n = ntf::NovaNotificacao(ntf::TN_DESERIALIZAR_TABULEIRO);
    n->set_endereco(std::string("://") + argv[1]);
    central.AdicionaNotificacao(n.release());
  }
#endif
  // As vezes o carregamento falha por diretorios errados. Conferir se tabela carregou (pois nao da erro apos construcao).
  if (tabelas.todas().tabela_classes().info_classes().empty()) {
    central.AdicionaNotificacao(ntf::NovaNotificacaoErro(
          google::protobuf::StringPrintf(
            "%s: %s", "Erro carregando tabelas, caminho: ", dir.absolutePath().toStdString().c_str())));
  }

  try {
    p->Executa();
  }
  catch (exception& e) {
    std::cerr << e.what() << std::endl;
#if ANDROID
    return;
#else
    return 1;
#endif
  }
#if ANDROID
  return;
#else
  ent::ImprimeDadosRolados();
  return 0;
#endif
}

#if ANDROID
}  // extern C
#endif
