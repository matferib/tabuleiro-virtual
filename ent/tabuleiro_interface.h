#ifndef ENT_TABULEIRO_INTERFACE_H
#define ENT_TABULEIRO_INTERFACE_H

#include <memory>
#include "ifg/interface.h"

namespace ent {

class ParametrosDesenho;

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

  void Posicao(int x, int y) {
    x_ = x; y_ = y;
  }
  void Dimensoes(int largura, int altura) {
    largura_ = largura; altura_ = altura;
  }
  void Altura(int altura) {
    altura_ = altura;
  }
  void Largura(int largura) {
    largura_ = largura;
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
  InterfaceGraficaOpengl(ntf::CentralNotificacoes* central) :
    ifg::InterfaceGrafica(central) {}

  ~InterfaceGraficaOpengl() override {}

  void Desenha(ParametrosDesenho* pd);

  bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

 protected:
  void EscolheArquivoAbrirTabuleiro(
      const std::vector<std::string>& tab_estaticos,
      const std::vector<std::string>& tab_dinamicos,
      std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta) override;

 private:
  // So pode haver um elemento raiz por vez.
  std::unique_ptr<ElementoInterface> elemento_;
};

}  // namespace ent


#endif
