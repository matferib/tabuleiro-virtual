#ifndef IFG_QT_INTERFACE_H
#define IFG_QT_INTERFACE_H

#include <QtWidgets/QWidget>
#include <memory>
#include <string>
#include <vector>
#include "ifg/interface.h"
#include "ifg/qt/principal.h"

namespace ent {
class Tabelas;
}  // namespace ent
namespace ifg {
namespace qt {

class ParametrosDesenho;

class InterfaceGraficaQt : public ifg::InterfaceGrafica {
 public:
  InterfaceGraficaQt(
      const ent::Tabelas& tabelas, Principal* pai, ifg::TratadorTecladoMouse* teclado_mouse, ent::Tabuleiro* tabuleiro,
      ntf::CentralNotificacoes* central)
      : ifg::InterfaceGrafica(tabelas, teclado_mouse, tabuleiro, central), pai_(pai) {}

  ~InterfaceGraficaQt() override {}

 protected:
  void MostraMensagem(bool erro, const std::string& mensagem, std::function<void()> funcao_volta) override;

  void EscolheItemLista(
      const std::string& titulo,
      const std::optional<std::string>& rotulo_ok,
      const std::vector<RotuloTipoTesouro>& lista,
      std::function<void(bool, int, std::optional<ent::TipoTesouro>)> funcao_volta) override;

  void EscolheItemsLista(
      const std::string& titulo,
      const std::vector<std::string>& lista,
      std::function<void(bool, std::vector<int>)> funcao_volta) override;

  void EscolheCor(
      TipoCor tc, std::optional<int> id_cenario, float r, float g, float b, float a,
      std::function<void(bool, TipoCor, std::optional<int>, float, float, float, float)> funcao_volta) override;

  void EscolheArquivoAbrirTabuleiro(
      const std::vector<std::string>& tab_estaticos,
      const std::vector<std::string>& tab_dinamicos,
      std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta) override;

  void EscolheArquivoAbrirImagem(
    const std::vector<std::string>& imagens_locais, const std::vector<std::string>& imagens_globais,
    std::function<void(const std::string& nome, arq::tipo_e)> funcao_volta) override;

  void EscolheArquivoSalvarTabuleiro(
      std::function<void(const std::string& nome)> funcao_volta) override;

  void EscolheModeloEntidade(const MenuModelos& modelos, std::function<void(const std::string& nome)> funcao_volta) override;

  void EscolheValorDadoForcado(const std::string& titulo, int nfaces, std::function<void(int)> funcao_volta) override;

 private:
  Principal* pai_ = nullptr;
};

}  // namespace qt
}  // namespace ent


#endif
