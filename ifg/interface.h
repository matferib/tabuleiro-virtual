#ifndef IFG_INTERFACE_H
#define IFG_INTERFACE_H

#include <functional>
#include <string>

#include "arq/arquivo.h"
#include "ntf/notificacao.h"

namespace ent {
class Tabuleiro;
}  // namespace ent
namespace ifg {

class MenuModelos;
class TratadorTecladoMouse;

// Funcao auxiliar para misturar protos de menu de tipos diferentes (srd e nao srd).
void MisturaProtosMenu(const MenuModelos& entrada, MenuModelos* saida); 

class InterfaceGrafica : public ntf::Receptor {
 public:
  InterfaceGrafica(
      TratadorTecladoMouse* teclado_mouse, ent::Tabuleiro* tabuleiro, ntf::CentralNotificacoes* central)
      : teclado_mouse_(teclado_mouse), tabuleiro_(tabuleiro), central_(central) {
    central_->RegistraReceptor(this);
  }

  virtual ~InterfaceGrafica() {
    central_->DesregistraReceptor(this);
  }

  bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

 protected:
  // Mostra um dialogo de erro ou informacao.
  virtual void MostraMensagem(bool erro, const std::string& mensagem, std::function<void()> funcao_volta) = 0;

  // Mostra dialogo para escolher um item entre tab_estaticos e tab_dinamicos, chamando
  // a funcao de volta ao terminar.
  virtual void EscolheArquivoAbrirTabuleiro(
      const std::vector<std::string>& tab_estaticos,
      const std::vector<std::string>& tab_dinamicos,
      std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta) = 0;

  // Mostra dialogo para salvar tabuleiro, chamando a funcao de volta ao terminar.
  virtual void EscolheArquivoSalvarTabuleiro(
      std::function<void(const std::string& nome)> funcao_volta) = 0;

  // Mostra o dialogo para escolher um modelo de entidade.
  virtual void EscolheModeloEntidade(
      const MenuModelos& modelos,
      std::function<void(const std::string& nome)> funcao_volta) = 0;

 protected:
  TratadorTecladoMouse* teclado_mouse_ = nullptr;
  ent::Tabuleiro* tabuleiro_ = nullptr;
  ntf::CentralNotificacoes* central_ = nullptr;

 private:
  void TrataMostraMensagem(bool erro, const std::string& mensagem);

  void TrataAbrirTabuleiro(const ntf::Notificacao& notificacao);
  void VoltaAbrirTabuleiro(
    bool manter_entidades, const std::string& nome, arq::tipo_e tipo);

  void TrataSalvarTabuleiro(const ntf::Notificacao& notificacao);
  void VoltaSalvarTabuleiro(const std::string& nome);

  void TrataEscolherModeloEntidade(const ntf::Notificacao& notificacao);
  void VoltaEscolherModeloEntidade(const std::string& nome);
};

}  // namespace ifg

#endif
