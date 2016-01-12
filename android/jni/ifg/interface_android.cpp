#include <jni.h>

#include "ifg/interface_android.h"

namespace ifg {

void InterfaceGraficaAndroid::MostraMensagem(
    bool erro, const std::string& mensagem, std::function<void()> funcao_volta) {
  if (env_ == nullptr) {
    auto* n = ntf::NovaNotificacao(ntf::TN_ERRO);
    n->set_erro("env_ null, esqueceu de chamar setEnvThisz?");
    central_->AdicionaNotificacao(n);
    return;
  }
  auto* copia_volta = new std::function<void()>(funcao_volta);
  jmethodID metodo = Metodo("mensagem", "(ZLjava/lang/String;J)V");
  jstring msg = env_->NewStringUTF(mensagem.c_str());
  if (msg == nullptr) {
    throw std::logic_error("falha alocando string de mensagem");
  }
  env_->CallVoidMethod(thisz_, metodo, erro, msg, (jlong)copia_volta);
}

void InterfaceGraficaAndroid::EscolheArquivoAbrirTabuleiro(
    const std::vector<std::string>& tab_estaticos,
    const std::vector<std::string>& tab_dinamicos,
    std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta) {
  if (env_ == nullptr) {
    auto* n = ntf::NovaNotificacao(ntf::TN_ERRO);
    n->set_erro("env_ null, esqueceu de chamar setEnvThisz?");
    central_->AdicionaNotificacao(n);
    return;
  }
  jmethodID metodo = Metodo("abreDialogoAbrirTabuleiro", "([Ljava/lang/String;[Ljava/lang/String;J)V");
  jobjectArray jte = (jobjectArray)env_->NewObjectArray(
      tab_estaticos.size(),
      env_->FindClass("java/lang/String"), env_->NewStringUTF(""));
  jobjectArray jtd = (jobjectArray)env_->NewObjectArray(
      tab_dinamicos.size(),
      env_->FindClass("java/lang/String"), env_->NewStringUTF(""));
  {
    int i = 0;
    for (const auto& s : tab_estaticos) {
      jstring sj = env_->NewStringUTF(s.c_str());
      env_->SetObjectArrayElement(jte, i++, sj);
    }
    i = 0;
    for (const auto& s : tab_dinamicos) {
      jstring sj = env_->NewStringUTF(s.c_str());
      env_->SetObjectArrayElement(jtd, i++, sj);
    }
  }

  auto* copia_volta = new std::function<void(const std::string& nome, arq::tipo_e tipo)>(funcao_volta);
  env_->CallVoidMethod(thisz_, metodo, jte, jtd, (jlong)copia_volta);
}

void InterfaceGraficaAndroid::EscolheArquivoSalvarTabuleiro(
    std::function<void(const std::string& nome)> funcao_volta) {
  if (env_ == nullptr) {
    auto* n = ntf::NovaNotificacao(ntf::TN_ERRO);
    n->set_erro("env_ null, esqueceu de chamar setEnvThisz?");
    central_->AdicionaNotificacao(n);
    return;
  }
  jmethodID metodo = Metodo("abreDialogoSalvarTabuleiro", "(J)V");
  auto* copia_volta = new std::function<void(const std::string& nome)>(funcao_volta); 
  env_->CallVoidMethod(thisz_, metodo, (jlong)copia_volta);
}

jmethodID InterfaceGraficaAndroid::Metodo(const char* nome_metodo, const char* assinatura_metodo) {
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

}  // namespace ifg
