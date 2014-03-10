/** Wrapper JNI para o tabuleiro baseado no Tabuleiro. */
#include <memory>
#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#include <stdint.h>
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"
#include "gl/gl.h"
#include "net/cliente.h"

namespace {

// Textura dummy.
class DummyTexturas : public ent::Texturas {
 public:
  virtual unsigned int Textura(const std::string& id) const override { return GL_INVALID_VALUE; }
};
std::unique_ptr<ntf::CentralNotificacoes> g_central;
std::unique_ptr<DummyTexturas> g_texturas;
std::unique_ptr<ent::Tabuleiro> g_tabuleiro;
std::unique_ptr<boost::asio::io_service> g_servico_io;
std::unique_ptr<net::Cliente> g_cliente;

}  // namespace

extern "C" {

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeInit(JNIEnv* env) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeInit");
  int* argcp = nullptr;
  char** argvp = nullptr;
  gl::IniciaGl(argcp, argvp);
  ent::Tabuleiro::InicializaGL();
  g_texturas.reset(new DummyTexturas);
  g_central.reset(new ntf::CentralNotificacoes);
  g_tabuleiro.reset(new ent::Tabuleiro(g_texturas.get(), g_central.get()));
  g_servico_io.reset(new boost::asio::io_service);
  net::Cliente cliente(g_servico_io.get(), g_central.get());

  auto* n = ntf::NovaNotificacao(ntf::TN_CONECTAR);
  n->set_endereco("192.168.1.6:11223");
  g_central->AdicionaNotificacao(n);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeResize(JNIEnv* env, jobject thiz, jint w, jint h) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "resize w=%d h=%d", w, h);
  g_tabuleiro->TrataRedimensionaJanela(w, h);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeDone(JNIEnv* env) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeDone");
}

void Java_com_matferib_Tabuleiro_TabuleiroSurfaceView_nativeTogglePauseResume(JNIEnv* env) {
  if (g_central.get() != nullptr) {
    __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeTogglePauseResume");
    auto* n = ntf::NovaNotificacao(ntf::TN_TEMPORIZADOR);
    g_central->AdicionaNotificacao(n);
    g_central->Notifica();
  } else {
    __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeTogglePauseResume antes");
  }
}

void Java_com_matferib_Tabuleiro_TabuleiroSurfaceView_nativePause(JNIEnv* env) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativePause");
}

void Java_com_matferib_Tabuleiro_TabuleiroSurfaceView_nativeResume(JNIEnv* env) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeResume");
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeRender(JNIEnv* env) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeRender");
  // TODO hack.
  g_tabuleiro->Desenha();
}

}  // extern "C"
