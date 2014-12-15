/** Wrapper JNI para o tabuleiro baseado no Tabuleiro. */
#include <memory>
#include <stdlib.h>
#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
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

#if PROFILER_LIGADO
#include "prof.h"
#endif

namespace {

// Trata notificacoes tipo TN_ERRO.
class ReceptorErro : public ntf::Receptor {
 public:
  ReceptorErro(JNIEnv* env, jobject thisz) : env_(env), thisz_(thisz) {}

  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override {
    if (notificacao.tipo() == ntf::TN_ERRO) {
      __android_log_print(ANDROID_LOG_ERROR, "Tabuleiro", "%s", notificacao.erro().c_str());
      return true;
    }
    if (notificacao.tipo() == ntf::TN_INFO) {
      __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "%s", notificacao.erro().c_str());
      return true;
      /*jclass classe = env_->FindClass("com/matferib/Tabuleiro/TabuleiroActivity");
      if (classe == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, "Tabuleiro", "%s", "classe invalida");
        return true;
      }
      jmethodID metodo = env_->GetMethodID(classe, "teste", "()V");
      if (metodo == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, "Tabuleiro", "%s", "metodo invalido");
        return true;
      }
      env_->CallVoidMethod(thisz_, metodo);
      return true;
      */
    }
    return false;
  }

 private:
  JNIEnv* env_ = nullptr;
  jobject thisz_ = nullptr;
};

// Teste
const ntf::Notificacao LeTabuleiro(JNIEnv* env, jobject assets) {
  AAssetManager* aman = AAssetManager_fromJava(env, assets);
  ntf::Notificacao ntf_tabuleiro;
  AAsset* asset = nullptr;
  //std::string caminho_asset("tabuleiros_salvos/deck_matheus.binproto");
  std::string caminho_asset("tabuleiros_salvos/castelo.binproto");
  asset = AAssetManager_open(aman, caminho_asset.c_str(), AASSET_MODE_BUFFER);
  if (asset == nullptr) {
    __android_log_print(ANDROID_LOG_ERROR, "Tabuleiro", "falha abrindo asset: %s", caminho_asset.c_str());
    throw 1;
  }
  off_t tam = AAsset_getLength(asset);
  if (tam <= 0) {
    __android_log_print(ANDROID_LOG_ERROR, "Tabuleiro", "falha com tamanho do asset: %ld", tam);
    throw 2;
  }
  std::string dados;
  dados.resize(tam);
  memcpy(&dados[0], AAsset_getBuffer(asset), tam);
  if (asset != nullptr) {
    AAsset_close(asset);
  }
  ntf_tabuleiro.ParseFromString(dados);
  __android_log_print(ANDROID_LOG_ERROR, "Tabuleiro", "asset lido: tam %ld, dados: %s", tam, ntf_tabuleiro.ShortDebugString().c_str());
  return ntf_tabuleiro;
}

// Contexto nativo.
std::unique_ptr<ntf::CentralNotificacoes> g_central;
std::unique_ptr<tex::Texturas> g_texturas;
std::unique_ptr<ent::Tabuleiro> g_tabuleiro;
std::unique_ptr<boost::asio::io_service> g_servico_io;
std::unique_ptr<net::Cliente> g_cliente;
std::unique_ptr<ntf::Receptor> g_receptor;
std::unique_ptr<ifg::TratadorTecladoMouse> g_teclado_mouse;

}  // namespace

extern "C" {

// Nativos de TabuleiroActivity. Endereco pode ser nullptr para auto conexao.
void Java_com_matferib_Tabuleiro_TabuleiroActivity_nativeCreate(
    JNIEnv* env, jobject thisz, jstring nome, jstring endereco, jobject assets) {
  std::string nome_nativo;
  {
    const char* nome_nativo_c = env->GetStringUTFChars(nome, 0);
    nome_nativo = nome_nativo_c;
    env->ReleaseStringUTFChars(nome, nome_nativo_c);
  }

  std::string endereco_nativo;
  if (endereco != nullptr) {
    const char* endereco_nativo_c = env->GetStringUTFChars(endereco, 0);
    endereco_nativo = endereco_nativo_c;
    env->ReleaseStringUTFChars(endereco, endereco_nativo_c);
  }
  __android_log_print(
      ANDROID_LOG_INFO, "Tabuleiro", "nativeCreate nome %s, endereco: %s", nome_nativo.c_str(), endereco_nativo.c_str());
  arq::Inicializa(env, assets);
  g_central.reset(new ntf::CentralNotificacoes);
  g_texturas.reset(new tex::Texturas(g_central.get()));
  g_tabuleiro.reset(new ent::Tabuleiro(g_texturas.get(), g_central.get()));
  g_servico_io.reset(new boost::asio::io_service);
  g_cliente.reset(new net::Cliente(g_servico_io.get(), g_central.get()));
  g_receptor.reset(new ReceptorErro(env, thisz));
  g_central->RegistraReceptor(g_receptor.get());
  g_teclado_mouse.reset(new ifg::TratadorTecladoMouse(g_central.get(), g_tabuleiro.get()));

  // TESTE
  try {
    ntf::Notificacao* ntf_tab = ntf::NovaNotificacao(ntf::TN_DESERIALIZAR_TABULEIRO);
    auto tab_lido = LeTabuleiro(env, assets);
    ntf_tab->Swap(&tab_lido);
    ntf_tab->set_tipo(ntf::TN_DESERIALIZAR_TABULEIRO);
    g_central->AdicionaNotificacao(ntf_tab);
  } catch (...) {
    __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "Falha lendo tabuleiro");
  }
  /*{
    ntf::Notificacao ninfo;
    ninfo.set_tipo(ntf::TN_INFO);
    ninfo.set_erro("TESTE");
    g_receptor->TrataNotificacao(ninfo);
  }*/
  auto* n = ntf::NovaNotificacao(ntf::TN_CONECTAR);
  n->set_id(nome_nativo);
  if (endereco != nullptr) {
    n->set_endereco(endereco_nativo);
  }
  g_central->AdicionaNotificacao(n);
}

void Java_com_matferib_Tabuleiro_TabuleiroActivity_nativeDestroy(JNIEnv* env, jobject thisz) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeDestroy");
  g_teclado_mouse.reset();
  g_central->DesregistraReceptor(g_receptor.get());
  g_receptor.reset();
  g_cliente.reset();
  g_servico_io.reset();
  g_tabuleiro.reset();
  g_texturas.reset();
  g_central.reset();
}

// Nativos de TabuleiroRenderer.
void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeInitGl(JNIEnv* env, jobject thisz) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeInitGl");
  int* argcp = nullptr;
  char** argvp = nullptr;
  gl::IniciaGl(argcp, argvp);
  g_tabuleiro->IniciaGL();
  g_texturas->Recarrega();
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeResize(JNIEnv* env, jobject thiz, jint w, jint h) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "resize w=%d h=%d", w, h);
  g_tabuleiro->TrataRedimensionaJanela(w, h);
}

void Java_com_matferib_Tabuleiro_TabuleiroSurfaceView_nativePause(JNIEnv* env, jobject thiz) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativePause");
  //setenv("CPUPROFILE", "/data/data/com.matferib.Tabuleiro/files/gmon.out", 1);
#if PROFILER_LIGADO
  moncleanup();
#endif
}

void Java_com_matferib_Tabuleiro_TabuleiroSurfaceView_nativeResume(JNIEnv* env, jobject thiz) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeResume");
#if PROFILER_LIGADO
  monstartup("tabuleiro.so");
#endif
}

// Touch.
void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeDoubleClick(JNIEnv* env, jobject thiz, jint x, jint y) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeDoubleClick: %d %d", x, y);
  // Detalha a entidade.
  g_teclado_mouse->TrataDuploCliqueMouse(ifg::Botao_Esquerdo, 0, x, y);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeTouchPressed(JNIEnv* env, jobject thiz, jboolean toggle, jint x, jint y) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeTouchPressed: %d %d", x, y);
  g_teclado_mouse->TrataBotaoMousePressionado(ifg::Botao_Esquerdo, toggle ? ifg::Modificador_Ctrl : 0, x, y);
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
    JNIEnv* env, jobject thiz, jint x, jint y) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeTranslation: (%d %d) (%d %d)", x, y, nx, ny);
  g_tabuleiro->TrataBotaoDireitoPressionado(x, y);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeTilt(
    JNIEnv* env, jobject thiz, jfloat delta) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeTilt: %f", delta);
  g_tabuleiro->TrataInclinacaoPorDelta(delta);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeKeyboard(
    JNIEnv* env, jobject thiz, jint key, jint modifiers) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeKeyboard: %d %d", key, modifiers);
  g_teclado_mouse->TrataTeclaPressionada(static_cast<ifg::teclas_e>(key),
                                         static_cast<ifg::modificadores_e>(modifiers));
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeAction(JNIEnv* env, jobject thiz, jboolean signal, jint x, jint y) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeHover: %d %d", x, y);
  g_teclado_mouse->TrataBotaoMousePressionado(signal ? ifg::Botao_Direito: ifg::Botao_Esquerdo, ifg::Modificador_Alt, x, y);
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
