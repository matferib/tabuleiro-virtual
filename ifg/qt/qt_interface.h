#ifndef IFG_QT_INTERFACE_H
#define IFG_QT_INTERFACE_H

#include <QWidget>
#include <memory>
#include <string>
#include <vector>
#include "ifg/interface.h"

namespace ent {
class Tabelas;
}  // namespace ent
namespace ifg {
namespace qt {

class ParametrosDesenho;

class InterfaceGraficaQt : public ifg::InterfaceGrafica {
 public:
  InterfaceGraficaQt(
      const ent::Tabelas& tabelas, QWidget* pai, ifg::TratadorTecladoMouse* teclado_mouse, ent::Tabuleiro* tabuleiro,
      ntf::CentralNotificacoes* central)
      : ifg::InterfaceGrafica(tabelas, teclado_mouse, tabuleiro, central), pai_(pai) {}

  ~InterfaceGraficaQt() override {}

 protected:
  void MostraMensagem(bool erro, const std::string& mensagem, std::function<void()> funcao_volta) override;

  void EscolheItemLista(
      const std::string& titulo,
      const std::vector<std::string>& lista,
      std::function<void(bool, int)> funcao_volta) override;

  void EscolheCor(
      float r, float g, float b, float a,
      std::function<void(bool, float, float, float, float)> funcao_volta) override;

  void EscolheArquivoAbrirTabuleiro(
      const std::vector<std::string>& tab_estaticos,
      const std::vector<std::string>& tab_dinamicos,
      std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta) override;

  void EscolheArquivoSalvarTabuleiro(
      std::function<void(const std::string& nome)> funcao_volta) override;

  void EscolheModeloEntidade(const MenuModelos& modelos, std::function<void(const std::string& nome)> funcao_volta) override;

  void EscolheVersaoTabuleiro(std::function<void(int versao)> funcao_volta) override;

 private:
  QWidget* pai_ = nullptr;
};

}  // namespace qt
}  // namespace ent


#endif
