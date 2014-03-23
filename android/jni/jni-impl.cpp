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

class DummyReceptor : public ntf::Receptor {
 public:
  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override {
    if (notificacao.tipo() == ntf::TN_ERRO) {
      __android_log_print(ANDROID_LOG_ERROR, "Tabuleiro", "%s", notificacao.erro().c_str());
    }
  }
};

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
std::unique_ptr<ntf::Receptor> g_receptor;

}  // namespace

extern "C" {

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeInit(JNIEnv* env) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeInit");
  int* argcp = nullptr;
  char** argvp = nullptr;
  gl::IniciaGl(argcp, argvp);
  ent::Tabuleiro::InicializaGL();
  g_central.reset(new ntf::CentralNotificacoes);
  g_texturas.reset(new DummyTexturas);
  g_tabuleiro.reset(new ent::Tabuleiro(g_texturas.get(), g_central.get()));
  g_servico_io.reset(new boost::asio::io_service);
  g_cliente.reset(new net::Cliente(g_servico_io.get(), g_central.get()));
  g_receptor.reset(new DummyReceptor);
  g_central->RegistraReceptor(g_receptor.get());

  auto* n = ntf::NovaNotificacao(ntf::TN_CONECTAR);
  n->set_endereco("192.168.1.10:11223");
  g_central->AdicionaNotificacao(n);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeResize(JNIEnv* env, jobject thiz, jint w, jint h) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "resize w=%d h=%d", w, h);
  g_tabuleiro->TrataRedimensionaJanela(w, h);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeDone(JNIEnv* env) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeDone");
}

void Java_com_matferib_Tabuleiro_TabuleiroSurfaceView_nativePause(JNIEnv* env) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativePause");
}

void Java_com_matferib_Tabuleiro_TabuleiroSurfaceView_nativeResume(JNIEnv* env) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeResume");
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeTouchPressed(JNIEnv* env, jobject thiz, jfloat x, jfloat y) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeTouchPressed: %f %f", x, y);
  g_tabuleiro->TrataBotaoEsquerdoPressionado(x, y, false);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeTouchMoved(JNIEnv* env, jfloat x, jfloat y) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeTouchMoved: %f %f", x, y);
  g_tabuleiro->TrataMovimentoMouse(x, y);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeTouchReleased(JNIEnv* env) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeTouchReleased");
  g_tabuleiro->TrataBotaoLiberado();
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeRender(JNIEnv* env) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeRender");
  g_tabuleiro->Desenha();
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeTimer(JNIEnv* env) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeTimer");
  auto* n = ntf::NovaNotificacao(ntf::TN_TEMPORIZADOR);
  g_central->AdicionaNotificacao(n);
  g_central->Notifica();
}

}  // extern "C"
