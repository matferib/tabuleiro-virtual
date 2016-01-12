#ifndef IFG_QT_INTERFACE_H
#define IFG_QT_INTERFACE_H

#include <QWidget>
#include <memory>
#include <string>
#include <vector>
#include "ifg/interface.h"

namespace ifg {
namespace qt {

class ParametrosDesenho;

class InterfaceGraficaQt : public ifg::InterfaceGrafica {
 public:
  InterfaceGraficaQt(
      QWidget* pai, ifg::TratadorTecladoMouse* teclado_mouse, ent::Tabuleiro* tabuleiro,
      ntf::CentralNotificacoes* central)
      : ifg::InterfaceGrafica(teclado_mouse, tabuleiro, central), pai_(pai) {}

  ~InterfaceGraficaQt() override {}

 protected:
  void MostraMensagem(bool erro, const std::string& mensagem, std::function<void()> funcao_volta) override;

  void EscolheArquivoAbrirTabuleiro(
      const std::vector<std::string>& tab_estaticos,
      const std::vector<std::string>& tab_dinamicos,
      std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta) override;

  void EscolheArquivoSalvarTabuleiro(
      std::function<void(const std::string& nome)> funcao_volta) override;

 private:
  QWidget* pai_ = nullptr;
};

}  // namespace qt
}  // namespace ent


#endif
