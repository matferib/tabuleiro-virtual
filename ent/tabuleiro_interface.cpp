#include <functional>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "ent/controle_virtual.pb.h"
#include "ent/tabuleiro_interface.h"
#include "ent/constantes.h"
#include "ent/entidade.pb.h"
#include "ent/util.h"
#include "ifg/modelos.pb.h"
#include "gltab/gl.h"
#include "gltab/gl_vbo.h"
#include "log/log.h"

namespace ent {

const int kPaddingPx = 2;

class ElementoRotulo : public ElementoInterface {
 public:
  ElementoRotulo(const std::string& rotulo, ElementoInterface* pai)
      : ElementoInterface(pai) {
    rotulo_ = StringSemUtf8(rotulo);
    Redimensiona();
  }

  ~ElementoRotulo() {}

  void Desenha(ParametrosDesenho* pd) override {
    int fonte_x_int, fonte_y_int;
    gl::TamanhoFonteComEscala(&fonte_x_int, &fonte_y_int);
    if (!pd->has_picking_x() && Altura() >= fonte_y_int) {
      int max_caracteres = Largura() / fonte_x_int;
      MudaCor(cor_);
      gl::PosicaoRaster(X() + kPaddingPx, Y() + kPaddingPx);
      gl::DesenhaStringAlinhadoEsquerda(rotulo_.substr(0, max_caracteres));
    }
  }

  void EscreveRotulo(const std::string& rotulo) {
    rotulo_ = StringSemUtf8(rotulo);
  }

  void CorRotulo(const float* cor) {
    CorParaProto(cor, &cor_);
  }

 private:
  void Redimensiona() {
    int fonte_x_int, fonte_y_int;
    gl::TamanhoFonteComEscala(&fonte_x_int, &fonte_y_int);
    Dimensoes(rotulo_.size() * fonte_x_int, fonte_y_int);
  }

 public:
  std::string rotulo_;
  Cor cor_;
};

class ElementoBotao : public ElementoInterface {
 public:
  ElementoBotao(
      const std::string& rotulo, std::function<void()> volta, ElementoInterface* pai)
      : rotulo_(rotulo, pai) {
    volta_ = volta;
    Dimensoes(rotulo_.Largura() + kPaddingPx * 2, rotulo_.Altura() + kPaddingPx * 2);
    rotulo_.Posiciona(kPaddingPx, kPaddingPx);
    rotulo_.CorRotulo(COR_PRETA);
    CorFundo(COR_VERDE);  // teste.
  }

  ~ElementoBotao() {}

  void Desenha(ParametrosDesenho* pd) override {
    if (ElementoInterface::Largura() == 0 || ElementoInterface::Altura() == 0) {
      return;
    }
    MudaCor(cor_fundo_);
    gl::Retangulo(X(), Y(), X() + ElementoInterface::Largura(), Y() + ElementoInterface::Altura());
    if (Largura() > rotulo_.Largura() && Altura() > rotulo_.Altura()) {
      rotulo_.DesenhaSeValido(pd);
    }
  }

  void CorFundo(const float* cor) {
    CorParaProto(cor, &cor_fundo_);
  }

  bool Picking(int x, int y) override {
    volta_();
    return true;
  }

  void EscreveRotulo(const std::string& rotulo) {
    rotulo_.EscreveRotulo(rotulo);
    ReajustaRotulo();
  }

  void CorRotulo(const float* cor) {
    rotulo_.CorRotulo(cor);
  }

  void Posiciona(int x, int y) override {
    ElementoInterface::Posiciona(x, y);
    ReajustaRotulo();
  }

  void EscreveAltura(int altura) override {
    ElementoInterface::EscreveAltura(altura);
    if (altura >= rotulo_.Altura()) {
      ReajustaRotulo();
    }
  }

  void EscreveLargura(int largura) override {
    ElementoInterface::EscreveLargura(largura);
    int largura_rotulo = rotulo_.Largura();
    if (largura >= largura_rotulo) {
      ReajustaRotulo();
    }
  }

 private:
  void ReajustaRotulo() {
    rotulo_.Posiciona(X() + kPaddingPx, Y() + kPaddingPx);
    rotulo_.EscreveLargura(Largura() - 2 * kPaddingPx);
    rotulo_.EscreveAltura(Altura() - 2 * kPaddingPx);
  }

  ElementoRotulo rotulo_;
  Cor cor_fundo_;
  std::function<void()> volta_;
};

// Container de elementos.
class ElementoContainer : public ElementoInterface {
 public:
  ElementoContainer(int x, int y, int largura, int altura, ElementoInterface* pai) : ElementoInterface(pai) {
    Posiciona(x, y);
    Dimensoes(largura, altura);
  }
  ~ElementoContainer() override {}

  void Desenha(ParametrosDesenho* pd) override {
    for (auto& filho : filhos_) {
      filho->DesenhaSeValido(pd);
    }
  }

  void AdicionaFilho(ElementoInterface* filho, bool preenche = false) {
    AjustaFilhoAntesInserir(filho, preenche);
    VLOG(1) << "Filho x: " << filho->X() << ", y: " << filho->Y()
            << ", l: " << filho->Largura() << ", a: " << filho->Altura();
    filhos_.push_back(std::unique_ptr<ElementoInterface>(filho));
  }

  bool Picking(int x, int y) override {
    for (auto& filho : filhos_) {
      if (filho->Clicado(x, y)) {
        return filho->Picking(x, y);
      }
    }
    return false;
  }

 protected:
  virtual void AjustaFilhoAntesInserir(ElementoInterface* filho, bool preenche) = 0;

  std::vector<std::unique_ptr<ElementoInterface>> filhos_;
};

// Preenche direita para esquerda.
class ElementoContainerHorizontal : public ElementoContainer {
 public:
  ElementoContainerHorizontal(int x, int y, int largura, int altura, ElementoInterface* pai)
      : ElementoContainer(x, y, largura, altura, pai) {}
  ~ElementoContainerHorizontal() override {}

 protected:
  void AjustaFilhoAntesInserir(ElementoInterface* filho, bool preenche) override {
    if (filho->Altura() > (Altura() - (kPaddingPx * 2)) || preenche) {
      filho->EscreveAltura(Altura() - (kPaddingPx * 2));
    }
    // Computa sobra largura.
    int sobra = Largura() - (kPaddingPx * 2);
    int x = X() + Largura() - kPaddingPx;
    for (const auto& f : filhos_) {
      sobra -= f->Largura();
      x -= (f->Largura() + kPaddingPx);
    }
    if (sobra < 0) {
      sobra = 0;
    }
    filho->EscreveLargura(std::min(filho->Largura(), sobra));
    filho->Posiciona(x - filho->Largura(), Y() + kPaddingPx);
  }
};

// Preenche de cima para baixo.
class ElementoContainerVertical : public ElementoContainer {
 public:
  ElementoContainerVertical(int x, int y, int largura, int altura, ElementoInterface* pai)
      : ElementoContainer(x, y, largura, altura, pai) {}
  ~ElementoContainerVertical() override {}

 protected:
  void AjustaFilhoAntesInserir(ElementoInterface* filho, bool preenche) override {
    if (filho->Largura() > (Largura() - (kPaddingPx * 2)) || preenche) {
      filho->EscreveLargura(Largura() - (kPaddingPx * 2));
    }
    // Computa sobra altura.
    int sobra = Altura() - (kPaddingPx * 2);
    int y = Y() + Altura() - kPaddingPx;
    for (const auto& f : filhos_) {
      sobra -= f->Altura();
      y -= (f->Altura() + kPaddingPx);
    }
    if (sobra < 0) {
      sobra = 0;
    }
    filho->EscreveAltura(std::min(filho->Altura(), sobra));
    filho->Posiciona(X() + kPaddingPx, y - filho->Altura());
  }
};

// Barra horizontal de ok e cancela.
class ElementoBarraOkCancela : public ElementoContainerHorizontal {
 public:
  ElementoBarraOkCancela(
      int x, int y, int largura, int altura, const std::optional<std::string>& rotulo_ok,
      std::function<void()> volta_ok, std::function<void()> volta_cancela, ElementoInterface* pai)
      : ElementoContainerHorizontal(x, y, largura, altura, pai) {
    auto* botao_cancela = new ElementoBotao("Cancela", volta_cancela, this);
    botao_cancela->CorRotulo(COR_VERMELHA);
    botao_cancela->CorFundo(COR_CINZA);
    AdicionaFilho(botao_cancela);
    auto* botao_ok = new ElementoBotao(rotulo_ok.has_value() ? *rotulo_ok : "Ok", volta_ok, this);
    botao_ok->CorRotulo(COR_VERDE);
    botao_ok->CorFundo(COR_CINZA);
    AdicionaFilho(botao_ok);
  }
  ~ElementoBarraOkCancela() {}

 private:
  std::function<void()> volta_ok_;
  std::function<void()> volta_cancela_;
};

// Uma lista paginada de elementos.
class ElementoListaPaginada : public ElementoInterface {
 public:
  ElementoListaPaginada(
      const std::vector<std::string>& rotulos,
      int x, int y, int largura, int altura, ElementoInterface* pai)
      : ElementoInterface(pai), rotulos_(rotulos) {
    Posiciona(x, y);
    Dimensoes(largura, altura);
    int fonte_x_int, fonte_y_int;
    gl::TamanhoFonteComEscala(&fonte_x_int, &fonte_y_int);
    std::function<void()> volta_anterior = [this] () {
      if (pagina_corrente_ > 0) {
        --pagina_corrente_;
        AtualizaLista();
      }
    };
    botao_anterior_.reset(new ElementoBotao("(anterior)", volta_anterior, this));
    botao_anterior_->CorRotulo(COR_BRANCA);
    botao_anterior_->CorFundo(COR_PRETA);
    botao_anterior_->Posiciona(X() + kPaddingPx, Y() + Altura() - botao_anterior_->Altura());
    std::function<void()> volta_proximo = [this] () {
      if ((pagina_corrente_ + 1) < num_paginas_) {
        ++pagina_corrente_;
        AtualizaLista();
      }
    };
    botao_proximo_.reset(new ElementoBotao("(próximo)", volta_proximo, this));
    botao_proximo_->Posiciona(X() + kPaddingPx, Y() + kPaddingPx);
    botao_proximo_->CorRotulo(COR_BRANCA);
    botao_proximo_->CorFundo(COR_PRETA);
    lista_.reset(new ElementoContainerVertical(
        X(), Y() + botao_proximo_->Altura(),
        Largura(), Altura() - botao_proximo_->Altura() - botao_anterior_->Altura(), this));
    std::unique_ptr<ElementoBotao> rotulo(new ElementoBotao("TEMP", std::function<void()>(), nullptr));
    elementos_por_pagina_ =
        (lista_->Altura() - botao_anterior_->Altura() - botao_proximo_->Altura()) / rotulo->Altura();
    if (elementos_por_pagina_ > 0) {
      num_paginas_ = rotulos.size() / elementos_por_pagina_;
      if (rotulos.size() % elementos_por_pagina_ > 0) {
        ++num_paginas_;
      }
    } else {
      num_paginas_ = 0;
    }
    for (unsigned int i = 0; i < elementos_por_pagina_; ++i) {
      std::function<void()> volta = [this, i] () {
        unsigned int candidato = pagina_corrente_ * elementos_por_pagina_ + i;
        if (candidato < rotulos_.size()) {
          item_selecionado_ = pagina_corrente_ * elementos_por_pagina_ + i;
          AtualizaLista();
        }
      };
      auto* er = new ElementoBotao("PLACEHOLDER", volta, this);
      er->CorRotulo(COR_BRANCA);
      er->CorFundo(COR_PRETA);
      elementos_rotulos_.push_back(er);
      lista_->AdicionaFilho(er, true  /*preenche*/);
    }
    item_selecionado_ = std::numeric_limits<unsigned int>::max();
    AtualizaLista();
  }

  ~ElementoListaPaginada() {}

  unsigned int ItemSelecionado() const {
    return item_selecionado_;
  }

  void Desenha(ParametrosDesenho* pd) override {
    botao_anterior_->DesenhaSeValido(pd);
    if (num_paginas_ > 0) {
      lista_->DesenhaSeValido(pd);
    }
    botao_proximo_->DesenhaSeValido(pd);
  }

  bool Picking(int x, int y) override {
    if (botao_anterior_->Clicado(x, y)) {
      return botao_anterior_->Picking(x, y);
    } else if (botao_proximo_->Clicado(x, y)) {
      return botao_proximo_->Picking(x, y);
    } else if (lista_->Clicado(x, y)) {
      return lista_->Picking(x, y);
    }
    return false;
  }

 private:
  void AtualizaLista() {
    if (pagina_corrente_ >= num_paginas_) {
      return;
    }
    unsigned int inicio = pagina_corrente_ * elementos_por_pagina_;
    unsigned int indice_selecionado = std::numeric_limits<unsigned int>::max();
    if (item_selecionado_ >= inicio && item_selecionado_ < inicio + elementos_por_pagina_) {
      indice_selecionado = item_selecionado_ % elementos_por_pagina_;
    }
    for (unsigned int i = 0; i < elementos_por_pagina_; ++i) {
      if (i == indice_selecionado) {
        elementos_rotulos_[i]->CorFundo(COR_VERDE);
        elementos_rotulos_[i]->CorRotulo(COR_PRETA);
      } else {
        elementos_rotulos_[i]->CorFundo(COR_PRETA);
        elementos_rotulos_[i]->CorRotulo(COR_VERDE);
      }
      if ((inicio + i) >= rotulos_.size()) {
        elementos_rotulos_[i]->EscreveRotulo("");
      } else {
        elementos_rotulos_[i]->EscreveRotulo(rotulos_[inicio + i]);
      }
    }
  }

  unsigned int pagina_corrente_ = 0;
  unsigned int num_paginas_ = 0;
  unsigned int elementos_por_pagina_ = 0;
  unsigned int item_selecionado_ = 0;
  std::unique_ptr<ElementoBotao> botao_anterior_;
  std::unique_ptr<ElementoBotao> botao_proximo_;
  std::unique_ptr<ElementoContainerVertical> lista_;
  std::vector<std::string> rotulos_;
  std::vector<ElementoBotao*> elementos_rotulos_;
};

// Uma lista paginada de elementos com multiplos elementos selecionaveis.
class ElementoListaPaginadaMulti : public ElementoInterface {
 public:
  ElementoListaPaginadaMulti(
      const std::vector<std::string>& rotulos,
      int x, int y, int largura, int altura, ElementoInterface* pai)
      : ElementoInterface(pai), rotulos_(rotulos) {
    Posiciona(x, y);
    Dimensoes(largura, altura);
    int fonte_x_int, fonte_y_int;
    gl::TamanhoFonteComEscala(&fonte_x_int, &fonte_y_int);
    std::function<void()> volta_anterior = [this] () {
      if (pagina_corrente_ > 0) {
        --pagina_corrente_;
        AtualizaLista();
      }
    };
    botao_anterior_.reset(new ElementoBotao("(anterior)", volta_anterior, this));
    botao_anterior_->CorRotulo(COR_BRANCA);
    botao_anterior_->CorFundo(COR_PRETA);
    botao_anterior_->Posiciona(X() + kPaddingPx, Y() + Altura() - botao_anterior_->Altura());
    std::function<void()> volta_proximo = [this] () {
      if ((pagina_corrente_ + 1) < num_paginas_) {
        ++pagina_corrente_;
        AtualizaLista();
      }
    };
    botao_proximo_.reset(new ElementoBotao("(próximo)", volta_proximo, this));
    botao_proximo_->Posiciona(X() + kPaddingPx, Y() + kPaddingPx);
    botao_proximo_->CorRotulo(COR_BRANCA);
    botao_proximo_->CorFundo(COR_PRETA);
    lista_.reset(new ElementoContainerVertical(
        X(), Y() + botao_proximo_->Altura(),
        Largura(), Altura() - botao_proximo_->Altura() - botao_anterior_->Altura(), this));
    std::unique_ptr<ElementoBotao> rotulo(new ElementoBotao("TEMP", std::function<void()>(), nullptr));
    elementos_por_pagina_ =
        (lista_->Altura() - botao_anterior_->Altura() - botao_proximo_->Altura()) / rotulo->Altura();
    if (elementos_por_pagina_ > 0) {
      num_paginas_ = rotulos.size() / elementos_por_pagina_;
      if (rotulos.size() % elementos_por_pagina_ > 0) {
        ++num_paginas_;
      }
    } else {
      num_paginas_ = 0;
    }
    for (unsigned int i = 0; i < elementos_por_pagina_; ++i) {
      std::function<void()> volta = [this, i] () {
        unsigned int candidato = pagina_corrente_ * elementos_por_pagina_ + i;
        if (candidato < rotulos_.size()) {
          items_selecionados_.push_back(pagina_corrente_ * elementos_por_pagina_ + i);
          AtualizaLista();
        }
      };
      auto* er = new ElementoBotao("PLACEHOLDER", volta, this);
      er->CorRotulo(COR_BRANCA);
      er->CorFundo(COR_PRETA);
      elementos_rotulos_.push_back(er);
      lista_->AdicionaFilho(er, true  /*preenche*/);
    }
    items_selecionados_.clear();
    AtualizaLista();
  }

  ~ElementoListaPaginadaMulti() {}

  std::vector<unsigned int> ItemsSelecionados() const {
    return items_selecionados_;
  }

  void Desenha(ParametrosDesenho* pd) override {
    botao_anterior_->DesenhaSeValido(pd);
    if (num_paginas_ > 0) {
      lista_->DesenhaSeValido(pd);
    }
    botao_proximo_->DesenhaSeValido(pd);
  }

  bool Picking(int x, int y) override {
    if (botao_anterior_->Clicado(x, y)) {
      return botao_anterior_->Picking(x, y);
    } else if (botao_proximo_->Clicado(x, y)) {
      return botao_proximo_->Picking(x, y);
    } else if (lista_->Clicado(x, y)) {
      return lista_->Picking(x, y);
    }
    return false;
  }

 private:
  void AtualizaLista() {
    if (pagina_corrente_ >= num_paginas_) {
      return;
    }
    unsigned int inicio = pagina_corrente_ * elementos_por_pagina_;
    std::vector<unsigned int> indices_selecionados;
    for (unsigned int item_selecionado : items_selecionados_) {
      if (item_selecionado >= inicio && item_selecionado < inicio + elementos_por_pagina_) {
        indices_selecionados.push_back(item_selecionado % elementos_por_pagina_);
      }
    }
    for (unsigned int i = 0; i < elementos_por_pagina_; ++i) {
      if (c_any(indices_selecionados, i)) {
        elementos_rotulos_[i]->CorFundo(COR_VERDE);
        elementos_rotulos_[i]->CorRotulo(COR_PRETA);
      } else {
        elementos_rotulos_[i]->CorFundo(COR_PRETA);
        elementos_rotulos_[i]->CorRotulo(COR_VERDE);
      }
      if ((inicio + i) >= rotulos_.size()) {
        elementos_rotulos_[i]->EscreveRotulo("");
      } else {
        elementos_rotulos_[i]->EscreveRotulo(rotulos_[inicio + i]);
      }
    }
  }

  unsigned int pagina_corrente_ = 0;
  unsigned int num_paginas_ = 0;
  unsigned int elementos_por_pagina_ = 0;
  std::vector<unsigned int> items_selecionados_;
  std::unique_ptr<ElementoBotao> botao_anterior_;
  std::unique_ptr<ElementoBotao> botao_proximo_;
  std::unique_ptr<ElementoContainerVertical> lista_;
  std::vector<std::string> rotulos_;
  std::vector<ElementoBotao*> elementos_rotulos_;
};


// Elemento com uma lista paginada de items.
class ElementoItemLista : public ElementoInterface {
 public:
  ElementoItemLista(InterfaceGraficaOpengl* interface_grafica,
                    const std::optional<std::string>& rotulo_ok,
                    const std::vector<ifg::InterfaceGrafica::RotuloTipoTesouro>& lista,
                    std::function<void(bool, int, std::optional<ent::TipoTesouro>)> funcao_volta)
    : interface_grafica_(interface_grafica) {
    int fonte_x_int, fonte_y_int;
    gl::TamanhoFonteComEscala(&fonte_x_int, &fonte_y_int);
    GLint viewport[4];
    gl::Le(GL_VIEWPORT, viewport);
    GLint xc = viewport[2] / 2, yc = viewport[3] / 2;
    GLint largura = std::min(50 * fonte_x_int, static_cast<int>(viewport[2] * 0.8f));
    GLint altura = std::min(20 * fonte_y_int, static_cast<int>(viewport[3] * 0.8f));
    Posiciona(xc - (largura / 2), yc - (altura / 2));
    Dimensoes(largura, altura);
    std::function<void()> volta_cancela = [this, funcao_volta] () {
      funcao_volta(false, -1, std::nullopt);
      interface_grafica_->FechaElemento();
    };
    std::function<void()> volta_ok = [this, lista, funcao_volta] () {
      unsigned int indice = lista_paginada_->ItemSelecionado();
      funcao_volta(true, indice, lista[indice].tipo_tesouro);
      interface_grafica_->FechaElemento();
    };
    barra_ok_cancela_.reset(
        new ElementoBarraOkCancela(X(), Y(), Largura(), static_cast<int>(fonte_y_int + 4 * kPaddingPx), rotulo_ok, 
                                   volta_ok, volta_cancela, this));
    std::vector<std::string> lista_rotulos;
    for (const auto& [rotulo, tt] : lista) {
      lista_rotulos.push_back(rotulo);
    }
    lista_paginada_.reset(new ElementoListaPaginada(
          lista_rotulos,
          X(), Y() + barra_ok_cancela_->Altura(),
          Largura(), Altura() - barra_ok_cancela_->Altura(), this));
  }
  ~ElementoItemLista() {}

  void Desenha(ParametrosDesenho* pd) override {
    MudaCor(COR_PRETA);
    gl::Retangulo(X(), Y(), X() + Largura(), Y() + Altura());
    lista_paginada_->Desenha(pd);
    barra_ok_cancela_->Desenha(pd);
  }

  bool Picking(int x, int y) override {
    if (barra_ok_cancela_->Clicado(x, y)) {
      return barra_ok_cancela_->Picking(x, y);
    } else if (lista_paginada_->Clicado(x, y)) {
      return lista_paginada_->Picking(x, y);
    }
    return false;
  }

 private:
  InterfaceGraficaOpengl* interface_grafica_ = nullptr;
  std::unique_ptr<ElementoListaPaginada> lista_paginada_;
  std::unique_ptr<ElementoBarraOkCancela> barra_ok_cancela_;
};

// Elemento com uma lista paginada de items permitindo multiplas selecoes.
class ElementoItemListaMulti : public ElementoInterface {
 public:
  ElementoItemListaMulti(InterfaceGraficaOpengl* interface_grafica,
                         const std::optional<std::string>& rotulo_ok,
                         const std::vector<std::string>& lista,
                         std::function<void(bool, std::vector<int>)> funcao_volta)
    : interface_grafica_(interface_grafica) {
    int fonte_x_int, fonte_y_int;
    gl::TamanhoFonteComEscala(&fonte_x_int, &fonte_y_int);
    GLint viewport[4];
    gl::Le(GL_VIEWPORT, viewport);
    GLint xc = viewport[2] / 2, yc = viewport[3] / 2;
    GLint largura = std::min(50 * fonte_x_int, static_cast<int>(viewport[2] * 0.8f));
    GLint altura = std::min(20 * fonte_y_int, static_cast<int>(viewport[3] * 0.8f));
    Posiciona(xc - (largura / 2), yc - (altura / 2));
    Dimensoes(largura, altura);
    std::function<void()> volta_cancela = [this, funcao_volta] () {
      funcao_volta(false, {});
      interface_grafica_->FechaElemento();
    };
    std::function<void()> volta_ok = [this, funcao_volta] () {
      std::vector<int> v(lista_paginada_->ItemsSelecionados().begin(), lista_paginada_->ItemsSelecionados().end());
      funcao_volta(true, v);
      interface_grafica_->FechaElemento();
    };
    barra_ok_cancela_.reset(
        new ElementoBarraOkCancela(X(), Y(), Largura(), static_cast<int>(fonte_y_int + 4 * kPaddingPx), rotulo_ok, 
                                   volta_ok, volta_cancela, this));
    lista_paginada_.reset(new ElementoListaPaginadaMulti(
          lista,
          X(), Y() + barra_ok_cancela_->Altura(),
          Largura(), Altura() - barra_ok_cancela_->Altura(), this));
  }
  ~ElementoItemListaMulti() {}

  void Desenha(ParametrosDesenho* pd) override {
    MudaCor(COR_PRETA);
    gl::Retangulo(X(), Y(), X() + Largura(), Y() + Altura());
    lista_paginada_->Desenha(pd);
    barra_ok_cancela_->Desenha(pd);
  }

  bool Picking(int x, int y) override {
    if (barra_ok_cancela_->Clicado(x, y)) {
      return barra_ok_cancela_->Picking(x, y);
    } else if (lista_paginada_->Clicado(x, y)) {
      return lista_paginada_->Picking(x, y);
    }
    return false;
  }

 private:
  InterfaceGraficaOpengl* interface_grafica_ = nullptr;
  std::unique_ptr<ElementoListaPaginadaMulti> lista_paginada_;
  std::unique_ptr<ElementoBarraOkCancela> barra_ok_cancela_;
};



class ElementoAbrirTabuleiro : public ElementoInterface {
 public:
  ElementoAbrirTabuleiro(InterfaceGraficaOpengl* interface_grafica,
                         const std::vector<std::string>& tab_estaticos,
                         const std::vector<std::string>& tab_dinamicos,
                         std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta)
    : interface_grafica_(interface_grafica),
      tab_estaticos_(tab_estaticos),
      tab_dinamicos_(tab_dinamicos),
      funcao_volta_(funcao_volta) {
    int fonte_x_int, fonte_y_int;
    gl::TamanhoFonteComEscala(&fonte_x_int, &fonte_y_int);
    GLint viewport[4];
    gl::Le(GL_VIEWPORT, viewport);
    GLint xc = viewport[2] / 2, yc = viewport[3] / 2;
    GLint largura = std::min(50 * fonte_x_int, static_cast<int>(viewport[2] * 0.8f));
    GLint altura = std::min(20 * fonte_y_int, static_cast<int>(viewport[3] * 0.8f));
    Posiciona(xc - (largura / 2), yc - (altura / 2));
    Dimensoes(largura, altura);
    std::function<void()> volta_cancela = [this, funcao_volta] () {
      funcao_volta("", arq::TIPO_TABULEIRO_ESTATICO);
      interface_grafica_->FechaElemento();
    };
    std::function<void()> volta_ok = [this, funcao_volta] () {
      unsigned int indice = lista_paginada_->ItemSelecionado();
      if (indice < tab_estaticos_.size()) {
        //LOG(INFO) << "1: " << indice << ", " << tab_estaticos_[indice];
        funcao_volta(tab_estaticos_[indice], arq::TIPO_TABULEIRO_ESTATICO);
        interface_grafica_->FechaElemento();
      } else if ((indice - tab_estaticos_.size()) < tab_dinamicos_.size()) {
        unsigned int real = indice - tab_estaticos_.size();
        //LOG(INFO) << "2: " << real << ", " << tab_dinamicos_[real];;
        funcao_volta(tab_dinamicos_[real], arq::TIPO_TABULEIRO);
        interface_grafica_->FechaElemento();
      }
    };
    barra_ok_cancela_.reset(
        new ElementoBarraOkCancela(X(), Y(), Largura(), static_cast<int>(fonte_y_int + 4 * kPaddingPx), std::nullopt,
                                   volta_ok, volta_cancela, this));
    std::vector<std::string> lista;
    lista.insert(lista.end(), tab_estaticos.begin(), tab_estaticos.end());
    lista.insert(lista.end(), tab_dinamicos.begin(), tab_dinamicos.end());
    lista_paginada_.reset(new ElementoListaPaginada(
          lista,
          X(), Y() + barra_ok_cancela_->Altura(),
          Largura(), Altura() - barra_ok_cancela_->Altura(), this));
  }
  ~ElementoAbrirTabuleiro() {}

  void Desenha(ParametrosDesenho* pd) override {
    MudaCor(COR_PRETA);
    gl::Retangulo(X(), Y(), X() + Largura(), Y() + Altura());
    lista_paginada_->Desenha(pd);
    barra_ok_cancela_->Desenha(pd);
  }

  bool Picking(int x, int y) override {
    if (barra_ok_cancela_->Clicado(x, y)) {
      return barra_ok_cancela_->Picking(x, y);
    } else if (lista_paginada_->Clicado(x, y)) {
      return lista_paginada_->Picking(x, y);
    }
    return false;
  }

 private:
  InterfaceGraficaOpengl* interface_grafica_ = nullptr;
  const std::vector<std::string> tab_estaticos_;
  const std::vector<std::string> tab_dinamicos_;
  std::unique_ptr<ElementoListaPaginada> lista_paginada_;
  std::unique_ptr<ElementoBarraOkCancela> barra_ok_cancela_;
  std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta_;
};

class ElementoSalvarTabuleiro : public ElementoInterface {
 public:
  ElementoSalvarTabuleiro(InterfaceGraficaOpengl* interface_grafica,
                         std::function<void(const std::string& nome)> funcao_volta)
    : interface_grafica_(interface_grafica),
      funcao_volta_(funcao_volta) {
    int fonte_x_int, fonte_y_int;
    gl::TamanhoFonteComEscala(&fonte_x_int, &fonte_y_int);
    GLint viewport[4];
    gl::Le(GL_VIEWPORT, viewport);
    GLint xc = viewport[2] / 2, yc = viewport[3] / 2;
    GLint largura = std::min(50 * fonte_x_int, static_cast<int>(viewport[2] * 0.8f));
    GLint altura = std::min(fonte_y_int * 3, static_cast<int>(viewport[3] * 0.5f));
    Posiciona(xc - (largura / 2), yc - (altura / 2));
    Dimensoes(largura, altura);
    std::function<void()> volta_cancela = [this, funcao_volta] () {
      funcao_volta("");
      interface_grafica_->FechaElemento();
    };
    std::function<void()> volta_ok = [this, funcao_volta] () {
      // TODO
      funcao_volta("bla.binproto");
      interface_grafica_->FechaElemento();
    };
    barra_ok_cancela_.reset(
        new ElementoBarraOkCancela(X(), Y(), Largura(), static_cast<int>(fonte_y_int + 4 * kPaddingPx), std::nullopt,
                                   volta_ok, volta_cancela, this));
  }
  ~ElementoSalvarTabuleiro() {}

  void Desenha(ParametrosDesenho* pd) override {
    MudaCor(COR_PRETA);
    gl::Retangulo(X(), Y(), X() + Largura(), Y() + Altura());
    barra_ok_cancela_->Desenha(pd);
  }

  bool Picking(int x, int y) override {
    if (barra_ok_cancela_->Clicado(x, y)) {
      return barra_ok_cancela_->Picking(x, y);
    }
    return false;
  }

 private:
  InterfaceGraficaOpengl* interface_grafica_ = nullptr;
  std::unique_ptr<ElementoBarraOkCancela> barra_ok_cancela_;
  std::function<void(const std::string& nome)> funcao_volta_;
};

class ElementoAbrirImagem : public ElementoInterface {
 public:
  ElementoAbrirImagem(InterfaceGraficaOpengl* interface_grafica,
                      const std::vector<std::string>& texs_locais, const std::vector<std::string>& texs_globais,
                      std::function<void(const std::string& nome, arq::tipo_e)> funcao_volta)
    : interface_grafica_(interface_grafica),
      texs_locais_(texs_locais),
      texs_globais_(texs_globais),
      funcao_volta_(funcao_volta) {
    int fonte_x_int, fonte_y_int;
    gl::TamanhoFonteComEscala(&fonte_x_int, &fonte_y_int);
    GLint viewport[4];
    gl::Le(GL_VIEWPORT, viewport);
    GLint xc = viewport[2] / 2, yc = viewport[3] / 2;
    GLint largura = std::min(50 * fonte_x_int, static_cast<int>(viewport[2] * 0.8f));
    GLint altura = std::min(20 * fonte_y_int, static_cast<int>(viewport[3] * 0.8f));
    Posiciona(xc - (largura / 2), yc - (altura / 2));
    Dimensoes(largura, altura);
    std::function<void()> volta_cancela = [this, funcao_volta] () {
      funcao_volta("", arq::TIPO_TEXTURA);
      interface_grafica_->FechaElemento();
    };
    std::function<void()> volta_ok = [this, funcao_volta] () {
      unsigned int indice = lista_paginada_->ItemSelecionado();
      //LOG(INFO) << "1: " << indice << ", " << tab_estaticos_[indice];
      if (indice < texs_locais_.size()) {
        funcao_volta(texs_locais_[indice], arq::TIPO_TEXTURA_LOCAL);
      } else {
        funcao_volta(texs_globais_[indice - texs_locais_.size()], arq::TIPO_TEXTURA);
      }
      interface_grafica_->FechaElemento();
    };
    barra_ok_cancela_.reset(
        new ElementoBarraOkCancela(X(), Y(), Largura(), static_cast<int>(fonte_y_int + 4 * kPaddingPx), std::nullopt,
                                   volta_ok, volta_cancela, this));
    std::vector<std::string> lista;
    lista.insert(lista.end(), texs_locais_.begin(), texs_locais_.end());
    lista.insert(lista.end(), texs_globais_.begin(), texs_globais_.end());
    lista_paginada_.reset(new ElementoListaPaginada(
          lista,
          X(), Y() + barra_ok_cancela_->Altura(),
          Largura(), Altura() - barra_ok_cancela_->Altura(), this));
  }
  ~ElementoAbrirImagem() {}

  void Desenha(ParametrosDesenho* pd) override {
    MudaCor(COR_PRETA);
    gl::Retangulo(X(), Y(), X() + Largura(), Y() + Altura());
    lista_paginada_->Desenha(pd);
    barra_ok_cancela_->Desenha(pd);
  }

  bool Picking(int x, int y) override {
    if (barra_ok_cancela_->Clicado(x, y)) {
      return barra_ok_cancela_->Picking(x, y);
    } else if (lista_paginada_->Clicado(x, y)) {
      return lista_paginada_->Picking(x, y);
    }
    return false;
  }

 private:
  InterfaceGraficaOpengl* interface_grafica_ = nullptr;
  const std::vector<std::string> texs_locais_;
  const std::vector<std::string> texs_globais_;
  std::unique_ptr<ElementoListaPaginada> lista_paginada_;
  std::unique_ptr<ElementoBarraOkCancela> barra_ok_cancela_;
  std::function<void(const std::string& nome, arq::tipo_e)> funcao_volta_;
};

#if 0
class ElementoInputInt : public ElementoInterface {
 public:
  ElementoInputInt(const std::string& rotulo, int min, int max, int x, int y, int largura, int altura, ElementoInterface* pai)
      : ElementoInterface(pai) {
    Posiciona(x, y);
    Dimensoes(largura, altura);
    int fonte_x_int, fonte_y_int;
    gl::TamanhoFonteComEscala(&fonte_x_int, &fonte_y_int);
  }

  void Desenha(ParametrosDesenho* pd) override {
    if (ElementoInterface::Largura() == 0 || ElementoInterface::Altura() == 0) {
      return;
    }
    MudaCor(cor_fundo_);
    gl::Retangulo(X(), Y(), X() + ElementoInterface::Largura(), Y() + ElementoInterface::Altura());
    if (Largura() > rotulo_.Largura() && Altura() > rotulo_.Altura()) {
      rotulo_.DesenhaSeValido(pd);
    }
  }

  bool Picking(int x, int y) override {
    return true;
  }

 private:
  int min_ = 0;
  int max_ = 0;
  ElementoRotulo rotulo_;
};
#endif

class ElementoRangeNumero : public ElementoInterface {
 public:
  ElementoRangeNumero(InterfaceGraficaOpengl* interface_grafica,
                      int min, int max,
                      std::function<void(int valor)> funcao_volta)
    : interface_grafica_(interface_grafica),
      funcao_volta_(funcao_volta) {
    int fonte_x_int, fonte_y_int;
    gl::TamanhoFonteComEscala(&fonte_x_int, &fonte_y_int);
    GLint viewport[4];
    gl::Le(GL_VIEWPORT, viewport);
    GLint xc = viewport[2] / 2, yc = viewport[3] / 2;
    GLint largura = std::min(50 * fonte_x_int, static_cast<int>(viewport[2] * 0.8f));
    GLint altura = std::min(20 * fonte_y_int, static_cast<int>(viewport[3] * 0.8f));
    Posiciona(xc - (largura / 2), yc - (altura / 2));
    Dimensoes(largura, altura);
    std::function<void()> volta_cancela = [this, funcao_volta] () {
      funcao_volta(0);
      interface_grafica_->FechaElemento();
    };
    std::function<void()> volta_ok = [this, funcao_volta] () {
      // TODO.
      funcao_volta(0);
      interface_grafica_->FechaElemento();
    };
    barra_ok_cancela_.reset(
        new ElementoBarraOkCancela(X(), Y(), Largura(), static_cast<int>(fonte_y_int + 4 * kPaddingPx), std::nullopt,
                                   volta_ok, volta_cancela, this));
  }
  ~ElementoRangeNumero() {}

  void Desenha(ParametrosDesenho* pd) override {
    MudaCor(COR_PRETA);
    gl::Retangulo(X(), Y(), X() + Largura(), Y() + Altura());
    barra_ok_cancela_->Desenha(pd);
  }

  bool Picking(int x, int y) override {
    if (barra_ok_cancela_->Clicado(x, y)) {
      return barra_ok_cancela_->Picking(x, y);
    }
    return false;
  }

 private:
  InterfaceGraficaOpengl* interface_grafica_ = nullptr;
  std::unique_ptr<ElementoBarraOkCancela> barra_ok_cancela_;
  std::function<void(int)> funcao_volta_;
};

class ElementoModeloEntidade : public ElementoInterface {
 public:
  ElementoModeloEntidade(InterfaceGraficaOpengl* interface_grafica,
                         const ifg::MenuModelos& mm,
                         std::function<void(const std::string& nome)> funcao_volta)
    : interface_grafica_(interface_grafica),
      funcao_volta_(funcao_volta) {
    int fonte_x_int, fonte_y_int;
    gl::TamanhoFonteComEscala(&fonte_x_int, &fonte_y_int);
    GLint viewport[4];
    gl::Le(GL_VIEWPORT, viewport);
    GLint xc = viewport[2] / 2, yc = viewport[3] / 2;
    GLint largura = std::min(50 * fonte_x_int, static_cast<int>(viewport[2] * 0.8f));
    GLint altura = std::min(20 * fonte_y_int, static_cast<int>(viewport[3] * 0.8f));
    Posiciona(xc - (largura / 2), yc - (altura / 2));
    Dimensoes(largura, altura);
    std::function<void()> volta_cancela = [this, funcao_volta] () {
      funcao_volta("");
      interface_grafica_->FechaElemento();
    };
    PreencheModelos(mm, modelos_);
    std::function<void()> volta_ok = [this, funcao_volta] () {
      unsigned int indice = lista_paginada_->ItemSelecionado();
      //LOG(INFO) << "1: " << indice << ", " << tab_estaticos_[indice];
      funcao_volta(modelos_[indice]);
      interface_grafica_->FechaElemento();
    };
    barra_ok_cancela_.reset(
        new ElementoBarraOkCancela(X(), Y(), Largura(), static_cast<int>(fonte_y_int + 4 * kPaddingPx), std::nullopt,
                                   volta_ok, volta_cancela, this));
    std::vector<std::string> lista;
    lista.insert(lista.end(), modelos_.begin(), modelos_.end());
    lista_paginada_.reset(new ElementoListaPaginada(
          lista,
          X(), Y() + barra_ok_cancela_->Altura(),
          Largura(), Altura() - barra_ok_cancela_->Altura(), this));
  }
  ~ElementoModeloEntidade() {}

  static void PreencheModelos(const ifg::MenuModelos& mm, std::vector<std::string>& modelos) {
    for (const auto& item : mm.item_menu()) {
      modelos.push_back(item.id());
    }
    for (const auto& mm : mm.sub_menu()) {
      PreencheModelos(mm, modelos);
    }
  }

  void Desenha(ParametrosDesenho* pd) override {
    MudaCor(COR_PRETA);
    gl::Retangulo(X(), Y(), X() + Largura(), Y() + Altura());
    lista_paginada_->Desenha(pd);
    barra_ok_cancela_->Desenha(pd);
  }

  bool Picking(int x, int y) override {
    if (barra_ok_cancela_->Clicado(x, y)) {
      return barra_ok_cancela_->Picking(x, y);
    } else if (lista_paginada_->Clicado(x, y)) {
      return lista_paginada_->Picking(x, y);
    }
    return false;
  }

 private:
  InterfaceGraficaOpengl* interface_grafica_ = nullptr;
  std::vector<std::string> modelos_;
  std::unique_ptr<ElementoListaPaginada> lista_paginada_;
  std::unique_ptr<ElementoBarraOkCancela> barra_ok_cancela_;
  std::function<void(const std::string& nome)> funcao_volta_;
};


class ElementoMostraMensagem : public ElementoInterface {
 public:
  ElementoMostraMensagem(
      InterfaceGraficaOpengl* interface_grafica,
      bool erro, const std::string& mensagem,
      std::function<void()> funcao_volta)
    : interface_grafica_(interface_grafica) {
    int fonte_x_int, fonte_y_int;
    gl::TamanhoFonteComEscala(&fonte_x_int, &fonte_y_int);
    GLint viewport[4];
    gl::Le(GL_VIEWPORT, viewport);
    GLint xc = viewport[2] / 2, yc = viewport[3] / 2;
    GLint largura = std::min(50 * fonte_x_int, static_cast<int>(viewport[2] * 0.8f));
    GLint altura = std::min(20 * fonte_y_int, static_cast<int>(viewport[3] * 0.8f));
    Posiciona(xc - (largura / 2), yc - (altura / 2));
    Dimensoes(largura, altura);
    rotulo_.reset(new ElementoRotulo(mensagem, this));
    std::function<void()> volta_ok = [this, funcao_volta] () {
      funcao_volta();
      interface_grafica_->FechaElemento();
    };
    botao_ok_.reset(new ElementoBotao("OK", volta_ok, this));
  }
  ~ElementoMostraMensagem() {}

  void Desenha(ParametrosDesenho* pd) override {
    MudaCor(COR_PRETA);
    gl::Retangulo(X(), Y(), X() + Largura(), Y() + Altura());
    rotulo_->Desenha(pd);
    botao_ok_->Desenha(pd);
  }

  bool Picking(int x, int y) override {
    if (botao_ok_->Clicado(x, y)) {
      return botao_ok_->Picking(x, y);
    }
    return false;
  }

 private:
  InterfaceGraficaOpengl* interface_grafica_ = nullptr;
  std::unique_ptr<ElementoRotulo> rotulo_;
  std::unique_ptr<ElementoBotao> botao_ok_;
};

//-------------------------
// Interface Grafica OpenGL
//-------------------------
void InterfaceGraficaOpengl::EscolheItemLista(
    const std::string& titulo,
    const std::optional<std::string>& rotulo_ok,
    const std::vector<RotuloTipoTesouro>& lista,
    std::function<void(bool, int, std::optional<ent::TipoTesouro>)> funcao_volta) {
  if (elemento_.get() != nullptr) {
    LOG(WARNING) << "So pode haver um elemento por vez.";
    return;
  }
  elemento_.reset(new ElementoItemLista(this, rotulo_ok, lista, funcao_volta));
}

void InterfaceGraficaOpengl::EscolheItemsLista(
    const std::string& titulo,
    const std::vector<std::string>& lista,
    std::function<void(bool, std::vector<int>)> funcao_volta) {
  if (elemento_.get() != nullptr) {
    LOG(WARNING) << "So pode haver um elemento por vez.";
    return;
  }
  elemento_.reset(new ElementoItemListaMulti(this, "Escolha um ou mais", lista, funcao_volta));
}

void InterfaceGraficaOpengl::EscolheArquivoAbrirTabuleiro(
    const std::vector<std::string>& tab_estaticos,
    const std::vector<std::string>& tab_dinamicos,
    std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta) {
  if (elemento_.get() != nullptr) {
    LOG(WARNING) << "So pode haver um elemento por vez.";
    return;
  }
  elemento_.reset(new ElementoAbrirTabuleiro(this, tab_estaticos, tab_dinamicos, funcao_volta));
}

void InterfaceGraficaOpengl::EscolheArquivoSalvarTabuleiro(
    std::function<void(const std::string& nome)> funcao_volta) {
  if (elemento_.get() != nullptr) {
    LOG(WARNING) << "So pode haver um elemento por vez.";
    return;
  }
  elemento_.reset(new ElementoSalvarTabuleiro(this, funcao_volta));
}

void InterfaceGraficaOpengl::EscolheArquivoAbrirImagem(
    const std::vector<std::string>& imagens_locais, const std::vector<std::string>& imagens_globais,
    std::function<void(const std::string& nome, arq::tipo_e)> funcao_volta) {
  if (elemento_.get() != nullptr) {
    LOG(WARNING) << "So pode haver um elemento por vez.";
    return;
  }
  elemento_.reset(new ElementoAbrirImagem(this, imagens_locais, imagens_globais, funcao_volta));
}

void InterfaceGraficaOpengl::EscolheValorDadoForcado(
    const std::string& titulo, int nfaces, std::function<void(int)> funcao_volta) {
  if (elemento_.get() != nullptr) {
    LOG(WARNING) << "So pode haver um elemento por vez.";
    return;
  }
  elemento_.reset(new ElementoRangeNumero(this, /*min=*/1, /*max=*/nfaces, funcao_volta));
}

void InterfaceGraficaOpengl::EscolheModeloEntidade(
    const ifg::MenuModelos& modelos,
    std::function<void(const std::string& nome)> funcao_volta) {
  if (elemento_.get() != nullptr) {
    LOG(WARNING) << "So pode haver um elemento por vez.";
    return;
  }
  elemento_.reset(new ElementoModeloEntidade(this, modelos, funcao_volta));
}


void InterfaceGraficaOpengl::MostraMensagem(bool erro, const std::string& mensagem, std::function<void()> funcao_volta) {
  if (elemento_.get() == nullptr) {
    return;
  }
  elemento_.reset(new ElementoMostraMensagem(this, erro, mensagem, funcao_volta));
}

bool InterfaceGraficaOpengl::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_TEMPORIZADOR:
      break;
    default:
      ;
  }
  return ifg::InterfaceGrafica::TrataNotificacao(notificacao);
}

void InterfaceGraficaOpengl::Desenha(ParametrosDesenho* pd) {
  if (elemento_.get() == nullptr) {
    return;
  }

  gl::Desabilita(GL_LIGHTING);
  gl::Desabilita(GL_DEPTH_TEST);
  gl::CarregaNome(CONTROLE_INTERFACE_GRAFICA);
  // Modo 2d: eixo com origem embaixo esquerda.
  gl::MatrizEscopo salva_matriz(gl::MATRIZ_PROJECAO);
  gl::CarregaIdentidade();
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  if (pd->has_picking_x()) {
    gl::MatrizPicking(pd->picking_x(), pd->picking_y(), 1.0, 1.0, viewport);
  }
  gl::Ortogonal(0, viewport[2], 0, viewport[3], 0, 1);
  gl::AtualizaMatrizes();
  gl::MatrizEscopo salva_matriz_camera(gl::MATRIZ_CAMERA);
  gl::CarregaIdentidade();
  gl::AtualizaMatrizes();
  gl::MatrizEscopo salva_matriz_modelagem(gl::MATRIZ_MODELAGEM);
  gl::CarregaIdentidade();
  elemento_->DesenhaSeValido(pd);
  return;
}

void InterfaceGraficaOpengl::Picking(int x, int y) {
  if (elemento_.get() == nullptr) {
    return;
  }
  elemento_->Picking(x, y);
}

}  // namespace ent
