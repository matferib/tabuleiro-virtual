#include <string>
#include <utility>
#include <vector>

#include "ent/tabuleiro_interface.h"
#include "ent/constantes.h"
#include "ent/util.h"
#include "gltab/gl.h"
#include "gltab/gl_vbo.h"
#include "log/log.h"

namespace ent {

const int kPaddingPx = 2;

class ElementoRotulo : public ElementoInterface {
 public:
  ElementoRotulo(const std::string& rotulo, const float* cor, ElementoInterface* pai)
      : ElementoInterface(pai) {
    rotulo_ = StringSemUtf8(rotulo);
    CorParaProto(cor, &cor_);
    int fonte_x_int, fonte_y_int;
    gl::TamanhoFonteComEscala(&fonte_x_int, &fonte_y_int);
    Dimensoes(rotulo_.size() * fonte_x_int + kPaddingPx * 2, fonte_y_int + kPaddingPx * 2);
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

  void Rotulo(const std::string& rotulo) {
    rotulo_ = rotulo;
  } 

 protected:
  std::string rotulo_;
  Cor cor_;
};

class ElementoBotao : public ElementoRotulo {
 public:
  ElementoBotao(const std::string& rotulo, const float* cor, ElementoInterface* pai)
      : ElementoRotulo(rotulo, cor, pai) {
    int fonte_x_int, fonte_y_int;
    gl::TamanhoFonteComEscala(&fonte_x_int, &fonte_y_int);
    Dimensoes(rotulo_.size() * fonte_x_int + kPaddingPx * 2, fonte_y_int + kPaddingPx * 2);
  }

  ~ElementoBotao() {}

  void Desenha(ParametrosDesenho* pd) override {
    if (Largura() == 0 || Altura() == 0) {
      return;
    }
    MudaCor(COR_VERDE);  // teste
    gl::Retangulo(X(), Y(), X() + Largura(), Y() + Altura());
    ElementoRotulo::Desenha(pd);
  }
};

// Container de elementos.
class ElementoContainer : public ElementoInterface {
 public:
  ElementoContainer(int x, int y, int largura, int altura, ElementoInterface* pai) : ElementoInterface(pai) {
    Posicao(x, y);
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
    filhos_.push_back(std::move(std::unique_ptr<ElementoInterface>(filho)));
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
      filho->Altura(Altura() - (kPaddingPx * 2));
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
    filho->Largura(std::min(filho->Largura(), sobra));
    filho->Posicao(x - filho->Largura(), Y() + kPaddingPx);
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
      filho->Largura(Largura() - (kPaddingPx * 2));
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
    filho->Altura(std::min(filho->Altura(), sobra));
    filho->Posicao(X() + kPaddingPx, y - filho->Altura());
  }
};

// Barra horizontal de ok e cancela.
class ElementoBarraOkCancela : public ElementoContainerHorizontal {
 public:
  ElementoBarraOkCancela(int x, int y, int largura, int altura, ElementoInterface* pai)
      : ElementoContainerHorizontal(x, y, largura, altura, pai) {
    AdicionaFilho(new ElementoBotao("Cancela", COR_PRETA, this));
    AdicionaFilho(new ElementoBotao("Ok", COR_PRETA, this));
  }
  ~ElementoBarraOkCancela() {}
};

// Uma lista paginada de elementos.
class ElementoListaPaginada : public ElementoInterface {
 public:
  ElementoListaPaginada(const std::vector<std::string>& rotulos,
      int x, int y, int largura, int altura, ElementoInterface* pai)
      : ElementoInterface(pai), rotulos_(rotulos) {
    Posicao(x, y);
    Dimensoes(largura, altura);
    int fonte_x_int, fonte_y_int;
    gl::TamanhoFonteComEscala(&fonte_x_int, &fonte_y_int);
    rotulo_anterior_.reset(new ElementoRotulo("(anterior)", COR_BRANCA, this));
    rotulo_anterior_->Posicao(X() + kPaddingPx, Y() + Altura() - kPaddingPx - fonte_y_int);
    rotulo_proximo_.reset(new ElementoRotulo("(próximo)", COR_BRANCA, this));
    rotulo_proximo_->Posicao(X() + kPaddingPx, Y() + kPaddingPx);
    lista_.reset(new ElementoContainerVertical(
          X(), Y() + rotulo_proximo_->Altura(),
          Largura(), Altura() - rotulo_proximo_->Altura() - rotulo_anterior_->Altura(), this));
    std::unique_ptr<ElementoRotulo> rotulo(new ElementoRotulo("TEMP", COR_BRANCA, nullptr));
    elementos_por_pagina_ =
        (lista_->Altura() - rotulo_anterior_->Altura() - rotulo_proximo_->Altura()) / rotulo->Altura();
    if (elementos_por_pagina_ > 0) {
      num_paginas_ = rotulos.size() / elementos_por_pagina_;
      if (rotulos.size() % elementos_por_pagina_ > 0) {
        ++num_paginas_;
      }
    } else {
      num_paginas_ = 0;
    }
    for (unsigned int i = 0; i < elementos_por_pagina_; ++i) {
      auto* er = new ElementoRotulo("PLACEHOLDER", COR_BRANCA, this);
      elementos_rotulos_.push_back(er);
      lista_->AdicionaFilho(er, true  /*preenche*/);
    }
    AtualizaLista(0);
  }

  ~ElementoListaPaginada() {}

  void Desenha(ParametrosDesenho* pd) {
    rotulo_anterior_->DesenhaSeValido(pd);
    if (num_paginas_ > 0) {
      lista_->DesenhaSeValido(pd);
    }
    rotulo_proximo_->DesenhaSeValido(pd);
  }

 private:
  void AtualizaLista(unsigned int pagina) {
    if (pagina >= num_paginas_) {
      return;
    }
    int inicio = pagina * elementos_por_pagina_;
    for (unsigned int i = inicio, j = 0; i < elementos_por_pagina_; ++i, ++j) {
      if (i >= rotulos_.size()) {
        break;
      }
      elementos_rotulos_[j]->Rotulo(rotulos_[i]);
    }
  }

  unsigned int pagina_corrente_ = 0;
  unsigned int num_paginas_ = 0;
  unsigned int elementos_por_pagina_ = 0;
  std::unique_ptr<ElementoRotulo> rotulo_anterior_;
  std::unique_ptr<ElementoRotulo> rotulo_proximo_;
  std::unique_ptr<ElementoContainerVertical> lista_;
  std::vector<std::string> rotulos_;
  std::vector<ElementoRotulo*> elementos_rotulos_;
};

class ElementoAbrirTabuleiro : public ElementoInterface {
 public:
  ElementoAbrirTabuleiro(const std::vector<std::string>& tab_estaticos,
                         const std::vector<std::string>& tab_dinamicos,
                         std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta)
    : tab_estaticos_(tab_estaticos),
      tab_dinamicos_(tab_dinamicos_),
      funcao_volta_(funcao_volta) {
    int fonte_x_int, fonte_y_int;
    gl::TamanhoFonteComEscala(&fonte_x_int, &fonte_y_int);
    GLint viewport[4];
    gl::Le(GL_VIEWPORT, viewport);
    GLint xc = viewport[2] / 2, yc = viewport[3] / 2;
    GLint largura = std::min(50 * fonte_x_int, static_cast<int>(viewport[2] * 0.8f));
    GLint altura = std::min(20 * fonte_y_int, static_cast<int>(viewport[3] * 0.8f));
    Posicao(xc - (largura / 2), yc - (altura / 2));
    Dimensoes(largura, altura);
    container_ok_cancela_.reset(
        new ElementoBarraOkCancela(X(), Y(), Largura(), static_cast<int>(fonte_y_int + 4 * kPaddingPx), this));
    std::vector<std::string> lista;
    lista.insert(lista.end(), tab_estaticos.begin(), tab_estaticos.end());
    lista.insert(lista.end(), tab_dinamicos.begin(), tab_dinamicos.end());
    lista_paginada_.reset(new ElementoListaPaginada(
          lista,
          X(), Y() + container_ok_cancela_->Altura(),
          Largura(), Altura() - container_ok_cancela_->Altura(), this));
  }
  ~ElementoAbrirTabuleiro() {}

  void Desenha(ParametrosDesenho* pd) override {
    MudaCor(COR_PRETA);
    gl::Retangulo(X(), Y(), X() + Largura(), Y() + Altura());
    lista_paginada_->Desenha(pd);
    container_ok_cancela_->Desenha(pd);
  }

 private:
  const std::vector<std::string>& tab_estaticos_;
  const std::vector<std::string>& tab_dinamicos_;
  std::unique_ptr<ElementoListaPaginada> lista_paginada_;
  std::unique_ptr<ElementoContainer> container_ok_cancela_;
  std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta_;
};

void InterfaceGraficaOpengl::EscolheArquivoAbrirTabuleiro(
    const std::vector<std::string>& tab_estaticos,
    const std::vector<std::string>& tab_dinamicos,
    std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta) {
  if (elemento_.get() != nullptr) {
    LOG(WARNING) << "So pode haver um elemento por vez.";
    return;
  }
  elemento_.reset(new ElementoAbrirTabuleiro(tab_estaticos, tab_dinamicos, funcao_volta));
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

// DESENHO
void InterfaceGraficaOpengl::Desenha(ParametrosDesenho* pd) {
  if (elemento_.get() == nullptr) {
    return;
  }

  gl::Desabilita(GL_LIGHTING);
  gl::Desabilita(GL_DEPTH_TEST);
  // Modo 2d: eixo com origem embaixo esquerda.
  gl::MatrizEscopo salva_matriz(GL_PROJECTION);
  gl::CarregaIdentidade(false);
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  if (pd->has_picking_x()) {
    gl::MatrizPicking(pd->picking_x(), pd->picking_y(), 1.0, 1.0, viewport);
  }
  gl::Ortogonal(0, viewport[2], 0, viewport[3], 0, 1);
  gl::MatrizEscopo salva_matriz_2(GL_MODELVIEW);
  gl::CarregaIdentidade();
  elemento_->DesenhaSeValido(pd);
  return;
}

}  // namespace ent
