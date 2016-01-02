#ifndef IFG_INTERFACE_IO_H
#define IFG_INTERFACE_IO_H

#include "ifg/interface.h"

namespace ifg {

class InterfaceIos : public InterfaceGrafica {
 public:
  InterfaceIos(
      TratadorTecladoMouse* teclado_mouse, ent::Tabuleiro* tabuleiro, ntf::CentralNotificacoes* central)
      : InterfaceGrafica(teclado_mouse, tabuleiro, central) {}

 protected:
  void MostraMensagem(bool erro, const std::string& mensagem, std::function<void()> funcao_volta) override;

  void EscolheArquivoAbrirTabuleiro(
      const std::vector<std::string>& tab_estaticos,
      const std::vector<std::string>& tab_dinamicos,
      std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta) override;

  void EscolheArquivoSalvarTabuleiro(
      std::function<void(const std::string& nome)> funcao_volta) override;
};

}  // namespace ifg

#endif
