#ifndef ENT_TABULEIRO_INTERFACE_H
#define ENT_TABULEIRO_INTERFACE_H

#include <memory>
#include "ifg/interface.h"

namespace ent {

class ParametrosDesenho;
class Tabelas;

// Todo elemento desenhavel da interface herda.
class ElementoInterface {
 public:
  explicit ElementoInterface(ElementoInterface* pai = nullptr) : pai_(pai) {}
  virtual ~ElementoInterface() {}

  void DesenhaSeValido(ParametrosDesenho* pd) {
    if (Largura() > 0 && Altura() > 0) {
      Desenha(pd);
    }
  }

  ElementoInterface* Pai() { return pai_; }

  int X() const { return x_; }
  int Y() const { return y_; }
  int Largura() const { return largura_; }
  int Altura() const { return altura_; }

  virtual void Posiciona(int x, int y) {
    x_ = x; y_ = y;
  }
  void Dimensoes(int largura, int altura) {
    EscreveLargura(largura);
    EscreveAltura(altura);
  }
  virtual void EscreveAltura(int altura) {
    altura_ = altura;
  }
  virtual void EscreveLargura(int largura) {
    largura_ = largura;
  }

  // Processa o click, retornando true se o click foi consumido. Caso contrario, o pai pode processar.
  virtual bool Picking(int x, int y) { return false; }
  virtual bool Clicado(int x, int y) const {
    return (x >= X() && x < (X() + Largura())) && (y >= Y() && y < (Y() + Altura()));
  }

 protected:
  virtual void Desenha(ParametrosDesenho* pd) = 0;

 private:
  ElementoInterface* pai_ = nullptr;
  std::vector<std::unique_ptr<ElementoInterface>> filhos_;
  int x_ = 0;
  int y_ = 0;
  int largura_ = 0;
  int altura_ = 0;
};

class InterfaceGraficaOpengl : public ifg::InterfaceGrafica {
 public:
  InterfaceGraficaOpengl(
      const Tabelas& tabelas, ifg::TratadorTecladoMouse* teclado_mouse, ent::Tabuleiro* tabuleiro, ntf::CentralNotificacoes* central)
      : ifg::InterfaceGrafica(tabelas, teclado_mouse, tabuleiro, central) {}

  ~InterfaceGraficaOpengl() override {}

  void Desenha(ParametrosDesenho* pd);

  bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

  void Picking(int x, int y);

  void FechaElemento() {
    elemento_.reset();
  }

 protected:
  void EscolheItemLista(
      const std::string& titulo,
      const std::optional<std::string>& rotulo_ok,
      const std::vector<std::string>& lista,
      std::function<void(bool, int)> funcao_volta) override;

  void EscolheArquivoAbrirTabuleiro(
      const std::vector<std::string>& tab_estaticos,
      const std::vector<std::string>& tab_dinamicos,
      std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta) override;

  void EscolheArquivoSalvarTabuleiro(
      std::function<void(const std::string& nome)> funcao_volta) override;

  void EscolheArquivoAbrirImagem(
    const std::vector<std::string>& imagens,
    std::function<void(const std::string& nome)> funcao_volta) override;

 private:
  // So pode haver um elemento raiz por vez.
  std::unique_ptr<ElementoInterface> elemento_;
};

}  // namespace ent


#endif
