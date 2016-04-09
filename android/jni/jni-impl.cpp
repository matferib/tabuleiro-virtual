/** Wrapper JNI para o tabuleiro baseado no Tabuleiro. */
#include <memory>
#include <stdlib.h>
#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <boost/asio.hpp>
#include <stdint.h>
#include "arq/arquivo.h"
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ent/util.h"
#include "ifg/tecladomouse.h"
#include "ifg/interface_android.h"
#include "m3d/m3d.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"
#include "gltab/gl.h"
#include "net/cliente.h"
#include "net/servidor.h"
#include "net/socket.h"
#include "tex/texturas.h"

#if PROFILER_LIGADO
#include "prof.h"
#endif

namespace {

// Trata notificacoes tipo TN_ERRO.
class ReceptorErro : public ntf::Receptor {
 public:
  ReceptorErro() {}

  /** O env da thread de desenho eh diferente do da inicializacao. */
  void setEnvThisz(JNIEnv* env, jobject thisz) { env_ = env; thisz_ = thisz; }

  bool TrataNotificacao(const ntf::Notificacao& notificacao) override {
    if (env_ == nullptr) {
      __android_log_print(ANDROID_LOG_ERROR, "Tabuleiro", "%s", "env invalido");
      return true;
    }
    try {
      switch (notificacao.tipo()) {
        case ntf::TN_ABRIR_DIALOGO_ENTIDADE:
          TrataNotificacaoAbrirDialogoEntidade(notificacao);
          break;
        default:
          return false;
      }
    } catch (const std::exception& e) {
      __android_log_print(ANDROID_LOG_ERROR, "Tabuleiro", "%s", e.what());
    }
    return true;
  }

 private:
  jmethodID Metodo(const char* nome_metodo, const char* assinatura_metodo) {
    jclass classe = env_->FindClass("com/matferib/Tabuleiro/TabuleiroRenderer");
    if (classe == nullptr) {
      throw std::logic_error("classe invalida");
    }
    jmethodID metodo = env_->GetMethodID(classe, nome_metodo, assinatura_metodo);
    if (metodo == nullptr) {
      throw std::logic_error("metodo invalido");
    }
    return metodo;
  }

  void TrataNotificacaoAbrirDialogoEntidade(const ntf::Notificacao& notificacao) {
    jmethodID metodo = Metodo("abreDialogoEntidade", "([B)V");
    const std::string nstr = notificacao.entidade().SerializeAsString();
    jbyteArray java_nstr = env_->NewByteArray(nstr.size());
    env_->SetByteArrayRegion(java_nstr, 0, nstr.size(), (const jbyte*)nstr.c_str());
    env_->CallVoidMethod(thisz_, metodo, java_nstr);
    env_->DeleteLocalRef(java_nstr);
  }

  JNIEnv* env_ = nullptr;
  jobject thisz_ = nullptr;
};

// Converte string java para C++. A jstr nao eh const por causa da alocacao de objetos.
const std::string ConverteString(JNIEnv* env, jstring jstr) {
  const char* cstr = env->GetStringUTFChars(jstr, 0);
  std::string cppstr(cstr);
  env->ReleaseStringUTFChars(jstr, cstr);
  return cppstr;
}

// Contexto nativo.
std::unique_ptr<ent::OpcoesProto> g_opcoes;
std::unique_ptr<ntf::CentralNotificacoes> g_central;
std::unique_ptr<tex::Texturas> g_texturas;
std::unique_ptr<m3d::Modelos3d> g_modelos3d;
std::unique_ptr<ent::Tabuleiro> g_tabuleiro;
std::unique_ptr<boost::asio::io_service> g_servico_io;
std::unique_ptr<net::Sincronizador> g_sincronizador;
std::unique_ptr<net::Cliente> g_cliente;
std::unique_ptr<net::Servidor> g_servidor;
std::unique_ptr<ReceptorErro> g_receptor;
std::unique_ptr<ifg::TratadorTecladoMouse> g_teclado_mouse;
std::unique_ptr<ifg::InterfaceGraficaAndroid> g_interface_android;

}  // namespace

extern "C" {

// Nativos de TabuleiroActivity. Endereco pode ser nullptr para auto conexao.
void Java_com_matferib_Tabuleiro_TabuleiroActivity_nativeCreate(
    JNIEnv* env, jobject thisz, jboolean servidor, jstring nome, jstring endereco,
    jboolean mapeamento_sombras, jboolean luz_por_pixel, jobject assets, jstring dir_dados) {
  arq::Inicializa(env, assets, ConverteString(env, dir_dados));
  g_opcoes.reset(new ent::OpcoesProto);
  g_opcoes->set_mapeamento_sombras(mapeamento_sombras);
  g_opcoes->set_iluminacao_por_pixel(luz_por_pixel);
  g_central.reset(new ntf::CentralNotificacoes);
  g_texturas.reset(new tex::Texturas(g_central.get()));
  g_modelos3d.reset(new m3d::Modelos3d(g_central.get()));
  g_tabuleiro.reset(new ent::Tabuleiro(*g_opcoes, g_texturas.get(), g_modelos3d.get(), g_central.get()));
  g_servico_io.reset(new boost::asio::io_service);
  g_sincronizador.reset(new net::Sincronizador(g_servico_io.get()));
  g_cliente.reset(new net::Cliente(g_sincronizador.get(), g_central.get()));
  g_servidor.reset(new net::Servidor(g_sincronizador.get(), g_central.get()));
  g_receptor.reset(new ReceptorErro);
  g_central->RegistraReceptor(g_receptor.get());
  g_teclado_mouse.reset(new ifg::TratadorTecladoMouse(g_central.get(), g_tabuleiro.get()));
  g_interface_android.reset(new ifg::InterfaceGraficaAndroid(
        g_teclado_mouse.get(), g_tabuleiro.get(), g_central.get()));

  /*{
    ntf::Notificacao ninfo;
    ninfo.set_tipo(ntf::TN_INFO);
    ninfo.set_erro("TESTE");
    g_receptor->TrataNotificacao(ninfo);
  }*/
  if (servidor) {
    auto* n = ntf::NovaNotificacao(ntf::TN_INICIAR);
    g_central->AdicionaNotificacao(n);
    // TESTE
    try {
      //auto* ntf_tab = new ntf::Notificacao;
      //arq::LeArquivoBinProto(arq::TIPO_TABULEIRO_ESTATICO, "deserto.binproto", ntf_tab);
      //ntf_tab->set_tipo(ntf::TN_DESERIALIZAR_TABULEIRO);
      //g_central->AdicionaNotificacao(ntf_tab);
    } catch (...) {
      __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "Falha lendo tabuleiro");
    }
  } else {
    std::string nome_nativo = ConverteString(env, nome);
    std::string endereco_nativo = ConverteString(env, endereco);
    __android_log_print(
        ANDROID_LOG_INFO, "Tabuleiro", "nativeCreate nome %s, endereco: '%s'", nome_nativo.c_str(), endereco_nativo.c_str());

    auto* n = ntf::NovaNotificacao(ntf::TN_CONECTAR);
    n->set_id_rede(nome_nativo);
    n->set_endereco(endereco_nativo);
    g_central->AdicionaNotificacao(n);
  }
}

void Java_com_matferib_Tabuleiro_TabuleiroActivity_nativeDestroy(JNIEnv* env, jobject thisz) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeDestroy");
  g_teclado_mouse.reset();
  g_central->DesregistraReceptor(g_receptor.get());
  g_receptor.reset();
  g_cliente.reset();
  g_servidor.reset();
  g_servico_io.reset();
  g_tabuleiro.reset();
  g_texturas.reset();
  g_central.reset();
}

// Nativos de TabuleiroRenderer.
void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeInitGl(JNIEnv* env, jobject thisz) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeInitGl");
  // luz por pixel e sombra projetada.
  gl::IniciaGl(g_opcoes->iluminacao_por_pixel(), g_opcoes->mapeamento_sombras());
  g_tabuleiro->IniciaGL();
  g_texturas->Recarrega();
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeResize(JNIEnv* env, jobject thiz, jint w, jint h) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "resize w=%d h=%d", w, h);
  g_tabuleiro->TrataRedimensionaJanela(w, h);
}

jint Java_com_matferib_Tabuleiro_TabuleiroSurfaceView_nativeTempoEntreNotificacoes(JNIEnv* env, jobject thiz) {
  return INTERVALO_NOTIFICACAO_MS;
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
    JNIEnv* env, jobject thiz, jfloat delta_rad) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeTilt: %f", delta);
  g_tabuleiro->TrataInclinacaoPorDelta(delta_rad);
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
jint Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeRender(JNIEnv* env, jobject thiz) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeRender");
  return g_tabuleiro->Desenha();
}

// Timer.
void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeTimer(JNIEnv* env, jobject thiz) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeTimer");
  g_receptor->setEnvThisz(env, thiz);
  g_interface_android->setEnvThisz(env, thiz);
  auto* n = ntf::NovaNotificacao(ntf::TN_TEMPORIZADOR);
  g_central->AdicionaNotificacao(n);
  g_central->Notifica();
}

// Atualizacao de entidade.
void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeUpdateEntity(JNIEnv* env, jobject thiz, jbyteArray mensagem) {
  int tam_mensagem = env->GetArrayLength(mensagem);
  std::string mensagem_str;
  mensagem_str.resize(tam_mensagem);
  env->GetByteArrayRegion(mensagem, 0, tam_mensagem, (jbyte*)&mensagem_str[0]);
  auto* n = ntf::NovaNotificacao(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
  n->mutable_entidade()->ParseFromString(mensagem_str);
  // Desfaz o hack de eventos
  auto evento_deshackeado(ent::LeEventos(n->entidade().evento(0).descricao()));
  if (evento_deshackeado.size() == 0) {
    // Dummy sem rodadas so pra atualizacao parcial entender que eh pra limpar.
    evento_deshackeado.Add();
  }
  n->mutable_entidade()->mutable_evento()->Swap(&evento_deshackeado);
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "Proto: %s", n->DebugString().c_str());
  g_central->AdicionaNotificacao(n);
}

// Dialogo de mensagem fechado.
void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeMessage(JNIEnv* env, jobject thiz, jlong dados_volta) {
  std::unique_ptr<std::function<void()>> funcao_volta(
      reinterpret_cast<std::function<void()>*>(dados_volta));
  (*funcao_volta)();
}

// Dialogo de salvar tabuleiro fechado.
void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeSaveBoardName(
    JNIEnv* env, jobject thiz, jlong dados_volta, jstring nome_arquivo) {
  std::unique_ptr<std::function<void(const std::string&)>> funcao_volta(
      reinterpret_cast<std::function<void(const std::string&)>*>(dados_volta));
  std::string nome_arquivo_c = ConverteString(env, nome_arquivo);
  if (nome_arquivo_c.empty()) {
    return;
  }
  (*funcao_volta)(nome_arquivo_c);
}

// Dialogo de abrir tabuleiro fechado.
void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeOpenBoardName(
    JNIEnv* env, jobject thiz, jlong dados_volta, jstring nome_arquivo, jboolean estatico) {
  std::unique_ptr<std::function<void(const std::string&, arq::tipo_e tipo)>> funcao_volta(
      reinterpret_cast<std::function<void(const std::string&, arq::tipo_e tipo)>*>(dados_volta));
  std::string nome_arquivo_c = ConverteString(env, nome_arquivo);
  if (nome_arquivo_c.empty()) {
    return;
  }
  (*funcao_volta)(nome_arquivo_c, estatico ? arq::TIPO_TABULEIRO_ESTATICO : arq::TIPO_TABULEIRO);
}

}  // extern "C"
