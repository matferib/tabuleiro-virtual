#ifndef IFG_INTERFACE_ANDROID
#define IFG_INTERFACE_ANDROID

#include <jni.h>
#include "ifg/interface.h"

namespace ifg {

class InterfaceGraficaAndroid : public InterfaceGrafica {
 public:
  InterfaceGraficaAndroid(
      TratadorTecladoMouse* teclado_mouse, ent::Tabuleiro* tabuleiro, ntf::CentralNotificacoes* central)
      : InterfaceGrafica(teclado_mouse, tabuleiro, central) {}

  ~InterfaceGraficaAndroid() override {}

  // Chamado pelo timer para arrumar as variaveis.
  void setEnvThisz(JNIEnv* env, jobject thisz) { env_ = env; thisz_ = thisz; }

  void MostraMensagem(bool erro, const std::string& mensagem, std::function<void()> funcao_volta) override;

  void EscolheArquivoAbrirTabuleiro(
      const std::vector<std::string>& tab_estaticos,
      const std::vector<std::string>& tab_dinamicos,
      std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta) override;

  void EscolheArquivoSalvarTabuleiro(
      std::function<void(const std::string& nome)> funcao_volta) override;

  void EscolheModeloEntidade(
      const MenuModelos& modelos,
      std::function<void(const std::string& nome)> funcao_volta) override;

 private:
  jmethodID Metodo(const char* nome_metodo, const char* assinatura_metodo);

  JNIEnv* env_ = nullptr;  // ponteiro do environment java.
  jobject thisz_ = nullptr;  // ponteiro para o TabuleiroRenderer.
};

}  // namespace ifg

#endif
