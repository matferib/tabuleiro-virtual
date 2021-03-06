#import <UIKit/UIKit.h>

#include "native.h"
#import "GameViewController.h"

#include <boost/asio.hpp>
#include <boost/asio/error.hpp>
#include <memory>
#include <cstdlib>
#include <cstdio>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include "arq/arquivo.h"
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ifg/tecladomouse.h"
#include "ifg/interface_ios.h"
#include "m3d/m3d.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"
#include "gltab/gl.h"
#include "net/cliente.h"
#include "net/servidor.h"
#include "net/socket.h"
#include "tex/texturas.h"

namespace {
// Contexto nativo.
std::unique_ptr<ntf::CentralNotificacoes> g_central;
std::unique_ptr<tex::Texturas> g_texturas;
std::unique_ptr<m3d::Modelos3d> g_modelos3d;
std::unique_ptr<ent::Tabuleiro> g_tabuleiro;
std::unique_ptr<boost::asio::io_service> g_servico_io;
std::unique_ptr<net::Sincronizador> g_sincronizador;
std::unique_ptr<net::Cliente> g_cliente;
std::unique_ptr<net::Servidor> g_servidor;
std::unique_ptr<ifg::TratadorTecladoMouse> g_teclado_mouse;
std::unique_ptr<ifg::InterfaceIos> g_interface;
GameViewController* g_view;  // ponteiro para o view principal.

// Carrega o tabuleiro do castelo quando o load de rede falhar.
class CarregadorTabuleiro : public ntf::Receptor {
 public:
  explicit CarregadorTabuleiro(ntf::CentralNotificacoes* central) {
    central->RegistraReceptor(this);
  }
  bool TrataNotificacao(const ntf::Notificacao& n) override {
    if (n.tipo() == ntf::TN_RESPOSTA_CONEXAO) {
      if (n.has_erro()) {
        // Carrega tab.
        auto* ntf_tab = new ntf::Notificacao;
        arq::LeArquivoBinProto(arq::TIPO_TABULEIRO_ESTATICO, "castelo.binproto", ntf_tab);
        ntf_tab->set_tipo(ntf::TN_DESERIALIZAR_TABULEIRO);
        g_central->AdicionaNotificacao(ntf_tab);
      }
      g_central->DesregistraReceptor(this);
    }
    return true;
  }
};
std::unique_ptr<CarregadorTabuleiro> g_carregador;

class TratadorDialogos : public ntf::Receptor {
 public:
  explicit TratadorDialogos(ntf::CentralNotificacoes* central) {
    central->RegistraReceptor(this);
  }
  bool TrataNotificacao(const ntf::Notificacao& n) override {
    return [g_view trataNotificacao:&n];
  }

 private:
};
std::unique_ptr<TratadorDialogos> g_tratador_dialogos;

void LeOpcoes(ent::OpcoesProto* proto) {
  try {
    arq::LeArquivoAsciiProto(arq::TIPO_CONFIGURACOES, "configuracoes.asciiproto", proto);
  } catch (const std::exception& e) {
    NSLog(@"Erro lendo opcoes iniciais, usando default: %s", e.what());
  }
}

void SalvaOpcoes(const ent::OpcoesProto& opcoes) {
  try {
    arq::EscreveArquivoAsciiProto(arq::TIPO_CONFIGURACOES, "configuracoes.asciiproto", opcoes);
  } catch (const std::exception& e) {
    NSLog(@"Erro salvando opcoes iniciais: %s", e.what());
  }
}

}  // namespace

// Essa eh a primeira chamada.
void nativeArqInitAndReadOptions(ent::OpcoesProto* proto) {
  arq::Inicializa();
  LeOpcoes(proto);
}

void nativeCreate(void* view) {
  g_view = (__bridge GameViewController*)view;
  g_central.reset(new ntf::CentralNotificacoes);
  g_texturas.reset(new tex::Texturas(g_central.get()));
  g_modelos3d.reset(new m3d::Modelos3d(g_central.get()));
  // Le de novo, pra num ter que criar global. O valor vai ser o mesmo.
  ent::OpcoesProto opcoes;
  LeOpcoes(&opcoes);
  opcoes.set_mapeamento_sombras(g_view->usar_mapeamento_sombras_);
  opcoes.set_iluminacao_por_pixel(g_view->usar_iluminacao_por_pixel_);
  SalvaOpcoes(opcoes);
  g_tabuleiro.reset(new ent::Tabuleiro(opcoes, g_texturas.get(), g_modelos3d.get(), g_central.get()));
  g_servico_io.reset(new boost::asio::io_service);
  g_sincronizador.reset(new net::Sincronizador(g_servico_io.get()));
  g_cliente.reset(new net::Cliente(g_sincronizador.get(), g_central.get()));
  g_servidor.reset(new net::Servidor(g_sincronizador.get(), g_central.get()));
  g_teclado_mouse.reset(
      new ifg::TratadorTecladoMouse(g_central.get(), g_tabuleiro.get()));
  g_carregador.reset(new CarregadorTabuleiro(g_central.get()));
  g_interface.reset(new ifg::InterfaceIos(view, g_teclado_mouse.get(), g_tabuleiro.get(), g_central.get()));
  g_tratador_dialogos.reset(new TratadorDialogos(g_central.get()));

  gl::IniciaGl(g_view->usar_iluminacao_por_pixel_);
  g_tabuleiro->IniciaGL();
  g_texturas->Recarrega();

  bool modo_servidor = g_view->id_cliente_ == nil;
  if (modo_servidor) {
    auto* n = ntf::NovaNotificacao(ntf::TN_INICIAR);
    g_central->AdicionaNotificacao(n);
  } else {
    auto* n = ntf::NovaNotificacao(ntf::TN_CONECTAR);
    n->set_id_rede([g_view->id_cliente_ UTF8String]);
    n->set_endereco([g_view->endereco_servidor_ UTF8String]);
    g_central->AdicionaNotificacao(n);
  }
}

void nativeDestroy() {
  g_tratador_dialogos.reset();
  g_teclado_mouse.reset();
  g_cliente.reset();
  g_servidor.reset();
  g_servico_io.reset();
  g_tabuleiro.reset();
  g_texturas.reset();
  g_central.reset();
  gl::FinalizaGl();
}

void nativeScale(float scale) {
  //NSLog(@"nativeScale");
  g_teclado_mouse->TrataPincaEscala(scale);
}

void nativeRotate(float rad) {
  //NSLog(@"nativeRotate");
  g_tabuleiro->TrataRotacaoPorDelta(rad);
}

void nativeResize(int w, int h) {
  g_tabuleiro->TrataRedimensionaJanela(w, h);
}

void nativeTimer() {
  auto* n = ntf::NovaNotificacao(ntf::TN_TEMPORIZADOR);
  g_central->AdicionaNotificacao(n);
  g_central->Notifica();
}

void nativeRender() {
  g_tabuleiro->Desenha();
}

void nativeTouchPressed(ifg::botoesmouse_e botao, bool toggle, int x, int y) {
  //NSLog(@"nativeTouchPressed");
  g_teclado_mouse->TrataBotaoMousePressionado(
      botao, toggle ? ifg::Modificador_Ctrl : 0, x, y);
}

void nativeTouchMoved(int x, int y) {
  //NSLog(@"NativeMoved");
  g_teclado_mouse->TrataMovimentoMouse(x, y);
}

void nativeTouchReleased() {
  //NSLog(@"nativeReleased");
  g_teclado_mouse->TrataBotaoMouseLiberado();
}

void nativeDoubleClick(int x, int y) {
  //NSLog(@"nativeDoubleClick");
  g_teclado_mouse->TrataDuploCliqueMouse(ifg::Botao_Esquerdo, 0, x, y);
}

void nativeTilt(float delta) {
  g_tabuleiro->TrataInclinacaoPorDelta(delta);
}

ntf::CentralNotificacoes* nativeCentral() {
  return g_central.get();
}

void nativeDoubleTouchPressed(int x1, int y1, int x2, int y2) {
  //NSLog(@"nativeDoubleTouchPressed");
  g_teclado_mouse->TrataInicioPinca(x1, y1, x2, y2);
}

// Teclado
void nativeKeyboard(int id_tecla) {
  g_teclado_mouse->TrataTeclaPressionada(ifg::teclas_e(ifg::Tecla_A + id_tecla), ifg::modificadores_e(0));
}
void nativeKeyboardCima() {
  g_teclado_mouse->TrataTeclaPressionada(ifg::Tecla_Cima, ifg::modificadores_e(0));
}
void nativeKeyboardBaixo() {
  g_teclado_mouse->TrataTeclaPressionada(ifg::Tecla_Baixo, ifg::modificadores_e(0));
}
void nativeKeyboardEsquerda() {
  g_teclado_mouse->TrataTeclaPressionada(ifg::Tecla_Esquerda, ifg::modificadores_e(0));
}
void nativeKeyboardDireita() {
  g_teclado_mouse->TrataTeclaPressionada(ifg::Tecla_Direita, ifg::modificadores_e(0));
}



















