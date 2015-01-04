#include <memory>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include "arq/arquivo.h"
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ifg/tecladomouse.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"
#include "gltab/gl.h"
#include "net/cliente.h"
#include "tex/texturas.h"

namespace {
// Contexto nativo.
std::unique_ptr<ntf::CentralNotificacoes> g_central;
std::unique_ptr<tex::Texturas> g_texturas;
std::unique_ptr<ent::Tabuleiro> g_tabuleiro;
std::unique_ptr<boost::asio::io_service> g_servico_io;
std::unique_ptr<net::Cliente> g_cliente;
std::unique_ptr<ifg::TratadorTecladoMouse> g_teclado_mouse;
    
}  // namespace native

void nativeCreate() {
  std::string nome_nativo;  // TODO
  std::string endereco_nativo;

  g_central.reset(new ntf::CentralNotificacoes);
  g_texturas.reset(new tex::Texturas(g_central.get()));
  g_tabuleiro.reset(new ent::Tabuleiro(g_texturas.get(), g_central.get()));
  g_servico_io.reset(new boost::asio::io_service);
  g_cliente.reset(new net::Cliente(g_servico_io.get(), g_central.get()));
  g_teclado_mouse.reset(
      new ifg::TratadorTecladoMouse(g_central.get(), g_tabuleiro.get()));

  int* argcp = nullptr;
  char** argvp = nullptr;
  gl::IniciaGl(argcp, argvp);
  g_tabuleiro->IniciaGL();
  g_texturas->Recarrega();
  
  auto* n = ntf::NovaNotificacao(ntf::TN_CONECTAR);
  n->set_id(nome_nativo);
  n->set_endereco(endereco_nativo);
  g_central->AdicionaNotificacao(n);
}
    
void nativeDestroy() {
  g_teclado_mouse.reset();
  g_cliente.reset();
  g_servico_io.reset();
  g_tabuleiro.reset();
  g_texturas.reset();
  g_central.reset();
  gl::FinalizaGl();
}

void nativeScale(float scale) {
  g_tabuleiro->TrataEscalaPorFator(scale);
}

void nativeRotate(float rad) {
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
  g_teclado_mouse->TrataBotaoMousePressionado(
      botao, toggle ? ifg::Modificador_Ctrl : 0, x, y);
}
  
void nativeTouchMoved(int x, int y) {
  g_teclado_mouse->TrataMovimentoMouse(x, y);
}
  
void nativeTouchReleased() {
  g_teclado_mouse->TrataBotaoMouseLiberado();
}

void nativeDoubleClick(int x, int y) {
  g_teclado_mouse->TrataDuploCliqueMouse(ifg::Botao_Esquerdo, 0, x, y);
}

