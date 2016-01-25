#ifndef ENT_TABULEIRO_TERRENO_H
#define ENT_TABULEIRO_TERRENO_H

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "ent/constantes.h"

namespace ent {

// Coordenada baseada em quadrados. 0,0 esquerda embaixo. 1,1 direita em cima.
struct XYQuad {
  int xquad;
  int yquad;
  bool xorigem;
  bool yorigem;

  std::string ParaString() {
    return std::string("(") + std::to_string(xquad) + ", " + std::to_string(yquad) + "; " +
            std::to_string(xorigem) + ", " + std::to_string(yorigem) + ")";
  }
  bool operator<(const XYQuad& rhs) const {
    if (xquad < rhs.xquad) {
      return true;
    } else if (xquad > rhs.xquad) {
      return false;
    } else if (yquad < rhs.yquad) {
      return true;
    } else if (yquad > rhs.yquad) {
      return false;
    } else if (xorigem && !rhs.xorigem) {
      return true;
    } else if (!xorigem && rhs.xorigem) {
      return false;
    } else if (yorigem && !rhs.yorigem) {
      return true;
    } else if (!yorigem && rhs.yorigem) {
      return false;
    } else {
      // Tudo igual.
      return false;
    }
  }
};

// Cada XYquad tem seus dados.
struct DadosPonto {
  int indice;
  float x;
  float y;
  float z;
  // normais.
  float nx;
  float ny;
  float nz;
  // Textura.
  float s;
  float t;
};

class Terreno {
 public:
  // Constroi um terreno flat, com o numero de quadrados passado.
  Terreno(int num_x_quad, int num_y_quad, bool ladrilho) {
    ladrilho_ = ladrilho;
    if (!ladrilho_) {
      inc_s_ = (1.0f / num_x_quad);
      inc_t_ = (1.0f / num_y_quad);
    }
    // Cria os pontos (os do final sao para fechar o quadrado).
    if (ladrilho_) {
      for (int ytab = 0; ytab < num_y_quad; ++ytab) {
        for (int xtab = 0; xtab < num_x_quad; ++xtab) {
          InserePonto(xtab,     ytab,     true,  true);
          InserePonto(xtab + 1, ytab,     false, true);
          InserePonto(xtab + 1, ytab + 1, false, false);
          InserePonto(xtab,     ytab + 1, true,  false);
        }
      }
    } else {
      // Sem ladrilho precisa criar a ultima fileira pra fechar o quadrado.
      for (int ytab = 0; ytab <= num_y_quad; ++ytab) {
        for (int xtab = 0; xtab <= num_x_quad; ++xtab) {
          InserePonto(xtab, ytab, true, true);
        }
      }
    }
    // Cria os triangulos.
    for (int ytab = 0; ytab < num_y_quad; ++ytab) {
      for (int xtab = 0; xtab < num_x_quad; ++xtab) {
        if (ladrilho_) {
          indices_.push_back(IndicePonto(xtab, ytab, true, true));
          indices_.push_back(IndicePonto(xtab + 1, ytab, false, true));
          indices_.push_back(IndicePonto(xtab + 1, ytab + 1, false, false));
          indices_.push_back(IndicePonto(xtab, ytab, true, true));
          indices_.push_back(IndicePonto(xtab + 1, ytab + 1, false, false));
          indices_.push_back(IndicePonto(xtab, ytab + 1, true, false));
        } else {
          indices_.push_back(IndicePonto(xtab, ytab, true, true));
          indices_.push_back(IndicePonto(xtab + 1, ytab, true, true));
          indices_.push_back(IndicePonto(xtab + 1, ytab + 1, true, true));
          indices_.push_back(IndicePonto(xtab, ytab, true, true));
          indices_.push_back(IndicePonto(xtab + 1, ytab + 1, true, true));
          indices_.push_back(IndicePonto(xtab, ytab + 1, true, true));
        }
      }
    }
  }

  void Preenche(std::vector<unsigned short>* indices,
                std::vector<float>* coordenadas,
                std::vector<float>* normais,
                std::vector<float>* texturas) const {
    indices->insert(indices->end(), indices_.cbegin(), indices_.cend());
    for (const auto* const ponto : pontos_) {
      coordenadas->push_back(ponto->x);
      coordenadas->push_back(ponto->y);
      coordenadas->push_back(ponto->z);
      normais->push_back(ponto->nx);
      normais->push_back(ponto->ny);
      normais->push_back(ponto->nz);
      texturas->push_back(ponto->s);
      texturas->push_back(ponto->t);
    }
  }

 private:
  bool PontoExiste(int x_quad, int y_quad, bool xorigem, bool yorigem) const {
    return mapa_pontos_.find({x_quad, y_quad, xorigem, yorigem}) != mapa_pontos_.end();
  }

  void InserePonto(int x_quad, int y_quad, bool xorigem, bool yorigem) {
    XYQuad xy = {x_quad, y_quad, xorigem, yorigem};
    if (PontoExiste(x_quad, y_quad, xorigem, yorigem)) {
      throw std::logic_error(std::string("Ponto ja existe: ") + xy.ParaString());
    }
    DadosPonto dp;
    dp.x = ConverteXQuad(x_quad);
    dp.y = ConverteYQuad(y_quad);
    dp.z = Altura(x_quad, y_quad);
    dp.nx = 0.0f;
    dp.ny = 0.0f;
    dp.nz = 1.0f;
    if (ladrilho_) {
      dp.s = xorigem ? 0.0f : 1.0f;
      dp.t = yorigem ? 1.0f : 0.0f;
    } else {
      dp.s = x_quad * inc_s_;
      dp.t = 1.0f - (y_quad * inc_t_);
    }
    dp.indice = pontos_.size();
    mapa_pontos_[xy] = std::move(dp);
    pontos_.push_back(&mapa_pontos_[xy]);
  }

  int IndicePonto(int x_quad, int y_quad, bool xorigem, bool yorigem) const {
    XYQuad xy = { x_quad, y_quad, xorigem, yorigem };
    auto it = mapa_pontos_.find(xy);
    if (it == mapa_pontos_.end()) {
      throw std::logic_error("Ponto nao existe.");
    }
    return it->second.indice; 
  }

 private:
  float ConverteXQuad(int x_quad) const {
    return delta_x_ + x_quad * TAMANHO_LADO_QUADRADO;
  }

  float ConverteYQuad(int y_quad) const {
    return delta_y_ + y_quad * TAMANHO_LADO_QUADRADO;
  }

  float Altura(int x_quad, int y_quad) const {
    return 0.0f;
  }

  bool ladrilho_ = false;
  std::map<XYQuad, DadosPonto> mapa_pontos_;
  std::vector<DadosPonto*> pontos_;
  std::vector<unsigned short> indices_; 
  float delta_x_ = 0;
  float delta_y_ = 0;
  // textura.
  float inc_s_ = 0.0f;
  float inc_t_ = 0.0f;
};

}  // namespace ent

#endif
