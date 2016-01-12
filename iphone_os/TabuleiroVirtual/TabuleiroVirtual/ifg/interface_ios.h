#ifndef IFG_INTERFACE_IO_H
#define IFG_INTERFACE_IO_H

#include "ifg/interface.h"

namespace ifg {

class InterfaceIos : public InterfaceGrafica {
 public:
  InterfaceIos(void* dados, TratadorTecladoMouse* teclado_mouse, ent::Tabuleiro* tabuleiro, ntf::CentralNotificacoes* central);
  ~InterfaceIos() override;

 protected:
  void MostraMensagem(bool erro, const std::string& mensagem, std::function<void()> funcao_volta) override;

  void EscolheArquivoAbrirTabuleiro(
      const std::vector<std::string>& tab_estaticos,
      const std::vector<std::string>& tab_dinamicos,
      std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta) override;

  void EscolheArquivoSalvarTabuleiro(
      std::function<void(const std::string& nome)> funcao_volta) override;
    
 private:
  struct Dados;
  Dados* dados_;
};

}  // namespace ifg

#endif
