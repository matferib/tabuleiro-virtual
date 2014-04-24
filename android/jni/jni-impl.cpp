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
#include "gltab/gl.h"
#include "net/cliente.h"
#include "tex/texturas.h"

namespace {

class ReceptorErro : public ntf::Receptor {
 public:
  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override {
    if (notificacao.tipo() == ntf::TN_ERRO) {
      __android_log_print(ANDROID_LOG_ERROR, "Tabuleiro", "%s", notificacao.erro().c_str());
    }
  }
};

class DummyTexturas : public ent::Texturas {
 public:
  virtual unsigned int Textura(const std::string& id) const {
    return GL_INVALID_VALUE;
  }
};

std::unique_ptr<ntf::CentralNotificacoes> g_central;
std::unique_ptr<ent::Texturas> g_texturas;
std::unique_ptr<ent::Tabuleiro> g_tabuleiro;
std::unique_ptr<boost::asio::io_service> g_servico_io;
std::unique_ptr<net::Cliente> g_cliente;
std::unique_ptr<ntf::Receptor> g_receptor;

}  // namespace

extern "C" {

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeInit(JNIEnv* env, jobject thisz, jstring endereco) {
  std::string endereco_nativo;
  {
    const char* endereco_nativo_c = env->GetStringUTFChars(endereco, 0);
    endereco_nativo = endereco_nativo_c;
    env->ReleaseStringUTFChars(endereco, endereco_nativo_c);
  }
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeInit endereco: %s", endereco_nativo.c_str());
  int* argcp = nullptr;
  char** argvp = nullptr;
  gl::IniciaGl(argcp, argvp);
  ent::Tabuleiro::InicializaGL();
  g_central.reset(new ntf::CentralNotificacoes);
  //g_texturas.reset(new tex::Texturas(g_central.get()));
  g_texturas.reset(new tex::Texturas(g_central.get()));
  g_tabuleiro.reset(new ent::Tabuleiro(g_texturas.get(), g_central.get()));
  g_servico_io.reset(new boost::asio::io_service);
  g_cliente.reset(new net::Cliente(g_servico_io.get(), g_central.get()));
  g_receptor.reset(new ReceptorErro);
  g_central->RegistraReceptor(g_receptor.get());

  auto* n = ntf::NovaNotificacao(ntf::TN_CONECTAR);
  n->set_endereco(endereco_nativo);
  g_central->AdicionaNotificacao(n);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeResize(JNIEnv* env, jobject thiz, jint w, jint h) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "resize w=%d h=%d", w, h);
  g_tabuleiro->TrataRedimensionaJanela(w, h);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeDone(JNIEnv* env, jobject thiz) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeDone");
}

void Java_com_matferib_Tabuleiro_TabuleiroSurfaceView_nativePause(JNIEnv* env, jobject thiz) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativePause");
}

void Java_com_matferib_Tabuleiro_TabuleiroSurfaceView_nativeResume(JNIEnv* env, jobject thiz) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeResume");
}

// Touch.
void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeDoubleClick(JNIEnv* env, jobject thiz, jint x, jint y) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeDoubleClick: %d %d", x, y);
  g_tabuleiro->TrataDuploCliqueEsquerdo(x, y);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeTouchPressed(JNIEnv* env, jobject thiz, jint x, jint y) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeTouchPressed: %d %d", x, y);
  g_tabuleiro->TrataBotaoEsquerdoPressionado(x, y, false);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeTouchMoved(JNIEnv* env, jobject thiz, jint x, jint y) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeTouchMoved: %d %d", x, y);
  g_tabuleiro->TrataMovimentoMouse(x, y);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeTouchReleased(JNIEnv* env, jobject thiz) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeTouchReleased");
  g_tabuleiro->TrataBotaoLiberado();
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeHover(JNIEnv* env, jobject thiz, jint x, jint y) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeHover: %d %d", x, y);
  g_tabuleiro->TrataMouseParadoEm(x, y);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeScale(JNIEnv* env, jobject thiz, jfloat s) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeScale: %f", s);
  g_tabuleiro->TrataEscalaPorFator(s);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeRotation(JNIEnv* env, jobject thiz, jfloat rad) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeScale: %f", s);
  g_tabuleiro->TrataRotacaoPorDelta(rad);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeTranslation(
    JNIEnv* env, jobject thiz, jint x, jint y, jint nx, jint ny) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeTranslation: (%d %d) (%d %d)", x, y, nx, ny);
  g_tabuleiro->TrataTranslacaoPorDelta(x, y, nx, ny);
}

// Render.
void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeRender(JNIEnv* env, jobject thiz) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeRender");
  g_tabuleiro->Desenha();
}

// Timer.
void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeTimer(JNIEnv* env, jobject thiz) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeTimer");
  auto* n = ntf::NovaNotificacao(ntf::TN_TEMPORIZADOR);
  g_central->AdicionaNotificacao(n);
  g_central->Notifica();
}

}  // extern "C"
