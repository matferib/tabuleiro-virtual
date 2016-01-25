#ifndef ENT_TABULEIRO_TERRENO_H
#define ENT_TABULEIRO_TERRENO_H

#include <map>
#include <stdexcept>
#include <vector>

#include "ent/constantes.h"

namespace ent {

// Coordenada baseada em quadrados. 0,0 esquerda embaixo. 1,1 direita em cima.
struct XYQuad {
  int xquad;
  int yquad;

  bool operator<(const XYQuad& rhs) const {
    return xquad < rhs.yquad || (xquad == rhs.xquad && yquad < rhs.yquad);
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
  Terreno(int num_x_quad, int num_y_quad) {
    delta_x_ = - ((num_x_quad * TAMANHO_LADO_QUADRADO) / 2.0f);
    delta_y_ = - ((num_y_quad * TAMANHO_LADO_QUADRADO) / 2.0f);
    inc_s_ = (1.0f / num_x_quad);
    inc_t_ = (1.0f / num_y_quad);
    for (int ytab = 0; ytab < num_y_quad; ++ytab) {
      for (int xtab = 0; xtab < num_x_quad; ++xtab) {
        if (PontoExiste(xtab, ytab)) {
          indices_.push_back(IndicePonto(xtab, ytab));
        } else {
          InserePonto(xtab, ytab);
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
  bool PontoExiste(int x_quad, int y_quad) const {
    return mapa_pontos_.find({x_quad, y_quad}) != mapa_pontos_.end();
  }

  void InserePonto(int x_quad, int y_quad) {
    DadosPonto dp;
    dp.x = ConverteXQuad(x_quad);
    dp.y = ConverteYQuad(y_quad);
    dp.z = Altura(x_quad, y_quad);
    dp.nx = 0.0f;
    dp.ny = 0.0f;
    dp.nz = 1.0f;
    dp.s = x_quad * inc_s_;
    dp.t = 1.0f - (y_quad * inc_t_);
    dp.indice = pontos_.size();
    indices_.push_back(dp.indice);
    XYQuad xy = { x_quad, y_quad };
    mapa_pontos_[xy] = std::move(dp);
    pontos_.push_back(&mapa_pontos_[xy]);
  }

  int IndicePonto(int x_quad, int y_quad) const {
    XYQuad xy = { x_quad, y_quad };
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
