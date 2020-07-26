#include <functional>
#include <stack>
#include <string>
#include <vector>
#include <jni.h>

#include "ent/tabelas.h"
#include "ifg/interface_android.h"
#include "ifg/modelos.pb.h"
#include "log/log.h"

using std::placeholders::_1;

namespace ifg {

void InterfaceGraficaAndroid::MostraMensagem(
    bool erro, const std::string& mensagem, std::function<void()> funcao_volta) {
  if (env_ == nullptr) {
    auto n = ntf::NovaNotificacao(ntf::TN_ERRO);
    n->set_erro("env_ null, esqueceu de chamar setEnvThisz?");
    central_->AdicionaNotificacao(n.release());
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
    auto n = ntf::NovaNotificacao(ntf::TN_ERRO);
    n->set_erro("env_ null, esqueceu de chamar setEnvThisz?");
    central_->AdicionaNotificacao(n.release());
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
    auto n = ntf::NovaNotificacao(ntf::TN_ERRO);
    n->set_erro("env_ null, esqueceu de chamar setEnvThisz?");
    central_->AdicionaNotificacao(n.release());
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

void InterfaceGraficaAndroid::EscolheModeloEntidade(
    const MenuModelos& menu_modelos,
    std::function<void(const std::string& nome)> funcao_volta) {
  if (env_ == nullptr) {
    auto n = ntf::NovaNotificacao(ntf::TN_ERRO);
    n->set_erro("env_ null, esqueceu de chamar setEnvThisz?");
    central_->AdicionaNotificacao(n.release());
    return;
  }
  const auto& modelos_tabelados = tabelas_.TodosModelosEntidades();
  std::set<std::string> modelos;
  for (const auto& modelo_tabelado : modelos_tabelados.modelo()) {
    modelos.insert(modelo_tabelado.id());
  }
  jmethodID metodo = Metodo("abreDialogoAbrirTabuleiro", "([Ljava/lang/String;[Ljava/lang/String;J)V");
  jobjectArray joa = (jobjectArray)env_->NewObjectArray(
      modelos.size(),
      env_->FindClass("java/lang/String"), env_->NewStringUTF(""));
  {
    int i = 0;
    for (const auto& s : modelos) {
      jstring sj = env_->NewStringUTF(s.c_str());
      env_->SetObjectArrayElement(joa, i++, sj);
    }
  }

  auto adaptador = [funcao_volta] (const std::string& nome, arq::tipo_e tipo) {
    funcao_volta(nome);
  };
  env_->CallVoidMethod(thisz_, metodo, joa, nullptr, (jlong)new std::function<void(const std::string&, arq::tipo_e)>(adaptador));
}

void InterfaceGraficaAndroid::EscolheItemLista(
    const std::string& titulo,
    const std::optional<std::string>& rotulo_ok,
    const std::vector<std::string>& lista,
    std::function<void(bool, int)> funcao_volta) {
  if (env_ == nullptr) {
    auto n = ntf::NovaNotificacao(ntf::TN_ERRO);
    n->set_erro("env_ null, esqueceu de chamar setEnvThisz?");
    central_->AdicionaNotificacao(n.release());
    return;
  }
  jmethodID metodo = Metodo("abreDialogoItemsLista", "([Ljava/lang/String;J)V");
  jobjectArray joa = (jobjectArray)env_->NewObjectArray(
      lista.size(),
      env_->FindClass("java/lang/String"), env_->NewStringUTF(""));
  {
    int i = 0;
    for (const auto& s : lista) {
      jstring sj = env_->NewStringUTF(s.c_str());
      env_->SetObjectArrayElement(joa, i++, sj);
    }
  }

  // A volta deletera.
  jlong funcao_volta_ptr = (jlong)new std::function<void(bool, int)>(funcao_volta);
  env_->CallVoidMethod(thisz_, metodo, joa, funcao_volta_ptr);
}

void InterfaceGraficaAndroid::EscolheItemsLista(
    const std::string& titulo,
    const std::vector<std::string>& lista,
    std::function<void(bool, std::vector<int>)> funcao_volta) {
  // TODO fazer essa funcao direito para permitir multipla selecao no android.
  if (env_ == nullptr) {
    auto n = ntf::NovaNotificacao(ntf::TN_ERRO);
    n->set_erro("env_ null, esqueceu de chamar setEnvThisz?");
    central_->AdicionaNotificacao(n.release());
    return;
  }
  auto adaptador_volta = [funcao_volta](bool ok, int indice) {
    std::vector<int> v;
    if (indice != -1) v.push_back(indice);
    funcao_volta(ok, v);
  };
  EscolheItemLista(titulo, std::nullopt, lista, adaptador_volta);
}

}  // namespace ifg
