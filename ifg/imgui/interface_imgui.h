#pragma once

#include "ifg/interface.h"

#include "ent/tabuleiro.h"
#include "ntf/notificacao.h"

namespace ifg::imgui {

class InterfaceImgui : public ifg::InterfaceGrafica {
 public:
  InterfaceImgui(const ent::Tabelas& tabelas, TratadorTecladoMouse* teclado_mouse, ent::Tabuleiro* tabuleiro, ntf::CentralNotificacoes* central);
  ~InterfaceImgui() override;

  void Executa();

 protected:
   bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

  // Mostra um dialogo de erro ou informacao.
  void MostraMensagem(bool erro, const std::string& mensagem, std::function<void()> funcao_volta) { funcao_volta(); }

  // Funcao generica para retorno da escolha de um item da lista. funcao_volta eh chamada com false
  // em caso de cancelamento, ou com o indice escolhido caso contrario.
  void EscolheItemLista(
    const std::string& titulo,
    const std::optional<std::string>& rotulo_ok,
    const std::vector<std::string>& lista,
    std::function<void(bool, int)> funcao_volta) override { funcao_volta(false, 0); }

  // Funcao generica para retorno da escolha de um ou mais items da lista. funcao_volta eh chamada com false
  // em caso de cancelamento, ou com os indices escolhidos caso contrario.
  void EscolheItemsLista(
    const std::string& titulo,
    const std::vector<std::string>& lista,
    std::function<void(bool, std::vector<int>)> funcao_volta) override { funcao_volta(false, {}); } 

  // Mostra dialogo para escolher um item entre tab_estaticos e tab_dinamicos, chamando
  // a funcao de volta ao terminar.
  void EscolheArquivoAbrirTabuleiro(
    const std::vector<std::string>& tab_estaticos,
    const std::vector<std::string>& tab_dinamicos,
    std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta) override { funcao_volta("", arq::tipo_e::TIPO_TABULEIRO); }

  // Mostra dialogo para salvar tabuleiro, chamando a funcao de volta ao terminar.
  void EscolheArquivoSalvarTabuleiro(
    std::function<void(const std::string& nome)> funcao_volta) override { funcao_volta(""); }

  // Mostra o dialogo para escolher um modelo de entidade.
  void EscolheModeloEntidade(
    const MenuModelos& modelos,
    std::function<void(const std::string& nome)> funcao_volta) override { funcao_volta(""); }

  // Mostra o dialogo de selecao de cor. A funcao de volta recebe se houve selecao de cor e caso positivo,
  // os componentes rgba.
  void EscolheCor(
    float r, float g, float b, float a,
    std::function<void(bool, float, float, float, float)> funcao_volta) override {
    funcao_volta(true, r, g, b, a);
  }

 private:
  static constexpr int kWindowWidth = 1280, kWindowHeight = 960;
};

}  // namespace ifg::imgui