#ifndef ENT_TABULEIRO_TERRENO_H
#define ENT_TABULEIRO_TERRENO_H

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "ent/constantes.h"
#include "log/log.h"
#include "matrix/matrices.h"
#include "net/util.h"

namespace ent {

// Coordenada baseada em quadrados. 0,0 esquerda embaixo. 1,1 direita em cima.
struct XYQuad {
  int xquad;
  int yquad;
  bool xorigem;
  bool yorigem;

  std::string ParaString() {
    return std::string("(") + net::to_string(xquad) + ", " + net::to_string(yquad) + "; " +
            net::to_string(xorigem) + ", " + net::to_string(yorigem) + ")";
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

// Wrapper sobre vector, repeater_field etc.
template <class Container>
class Wrapper {
 public:
  Wrapper(const Container& c) : c_(c) {}
  Wrapper(const Wrapper& rhs) : c_(rhs.c_) {}
  size_t size() const { return c_.size(); }
  bool empty() const { return c_.empty(); }

  typename Container::value_type operator[] (size_t indice) const {
    if (c_.empty()) {
      return 0.0f;
    }
    return c_.data()[indice];
  }

 private:
  Wrapper();
  const Container& c_;
};

class Terreno {
 public:
  // Constroi um terreno flat, com o numero de quadrados passado. O numero de pontos deve incluir a coluna final
  // leste e a linha final norte, para cobrir o tabuleiro todo.
  template <class Container>
  Terreno(int num_x_quad, int num_y_quad, const Wrapper<Container> pontos) {
    if (!pontos.empty() && pontos.size() != size_t((num_y_quad + 1) * (num_x_quad + 1))) {
      throw std::logic_error("Numero de pontos invalido.");
    }
    inc_s_ = (1.0f / num_x_quad);
    inc_t_ = (1.0f / num_y_quad);
    CriaPontos(num_x_quad, num_y_quad, pontos);
    ComputaNormais(num_x_quad, num_y_quad);
    CriaMalha(num_x_quad, num_y_quad);
  }

  // Retorna um conjunto de pontos aleatorios para o tamanho passado. Note que o vetor retornado tera um ponto a mais
  // para cada linha, e uma linha a mais no norte.
  static std::vector<double> CriaPontosAleatorios(int num_x_quad, int num_y_quad) {
    std::vector<double> pontos;
    int num_x = num_x_quad + 1;
    int num_y = num_y_quad + 1;
    pontos.reserve(num_x * num_y);
    for (int ytab = 0; ytab < num_y; ++ytab) {
      for (int xtab = 0; xtab < num_x; ++xtab) {
        pontos.push_back(GeraAltura(xtab, ytab));
      }
    }
    return pontos;
  }

  // Preenche os vetores de coordenadas dos pontos para OpenGL.
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

  // Retorna o indice de um ponto no tabuleiro no proto ponto_terreno.
  static int IndicePontoTabuleiro(int x_quad, int y_quad, int num_x_quad) {
    int num_x = num_x_quad + 1;  // borda.
    return y_quad * num_x + x_quad;
  }

  /** Retorna a altura de um ponto do tabuleiro, sem interpolar. O ponto eh dado por um x y de quadrado. */
  static float AlturaPonto(
      int x_quad, int y_quad, int num_x_quad, int num_y_quad, const double* pontos) {
    if (x_quad > num_x_quad || y_quad > num_y_quad) {
      throw std::logic_error("ponto invalido para altura");
    }
    return static_cast<float>(pontos[y_quad * (num_x_quad + 1) + x_quad]);
  }

  /** Retorna a altura do chao em determinado ponto do terreno. Retorna 0 se ponto for invalido. */
  static float ZChao(float x, float y, int num_x_quad, int num_y_quad, const double* pontos) {
    // Limites.
    float lim_x = num_x_quad * TAMANHO_LADO_QUADRADO_2;
    float lim_y = num_y_quad * TAMANHO_LADO_QUADRADO_2;
    if (fabs(x) >= lim_x || fabs(y) >= lim_y) {
      return 0.0f;
    }
    try {
      // Poe x e y com origem em 0,0.
      x += lim_x;
      y += lim_y;
      int x_quad0 = static_cast<int>(x / TAMANHO_LADO_QUADRADO);
      int x_quad1 = static_cast<int>(std::min<int>(x_quad0 + 1, num_x_quad));
      int y_quad0 = static_cast<int>(y / TAMANHO_LADO_QUADRADO);
      int y_quad1 = std::min<int>(y_quad0 + 1, num_y_quad);
      float zx0y0 = AlturaPonto(x_quad0, y_quad0, num_x_quad, num_y_quad, pontos);
      float zx1y0 = AlturaPonto(x_quad1, y_quad0, num_x_quad, num_y_quad, pontos);
      float zx0y1 = AlturaPonto(x_quad0, y_quad1, num_x_quad, num_y_quad, pontos);
      float zx1y1 = AlturaPonto(x_quad1, y_quad1, num_x_quad, num_y_quad, pontos);

      float dx = fmod(x, TAMANHO_LADO_QUADRADO) / TAMANHO_LADO_QUADRADO;
      float dy = fmod(y, TAMANHO_LADO_QUADRADO) / TAMANHO_LADO_QUADRADO;
      //LOG(INFO) << "zx0y0: " << zx0y0 << ", zx1y0: " << zx1y0 << ", zx0y1: " << zx0y1 << ", zx1y1: " << zx1y1
      //  << ", dx: " << dx << ", dy: " << dy;
      float z_sul   = (zx1y0 * dx) + (zx0y0 * (1.0f - dx));
      float z_norte = (zx1y1 * dx) + (zx0y1 * (1.0f - dx));
      float z_norte_sul = (z_norte * dy) + (z_sul * (1.0f - dy));
      return z_norte_sul;
    } catch (const std::logic_error& e) {
      LOG(ERROR) << e.what();
      return 0.0f;
    }
  }

  static bool Inverte(int x_quad, int y_quad) {
    return ((x_quad + y_quad) % 2) != 0;
  }

 private:
  // Verifica se um ponto existe.
  bool PontoExiste(int x_quad, int y_quad, bool xorigem, bool yorigem) const {
    return mapa_pontos_.find({x_quad, y_quad, xorigem, yorigem}) != mapa_pontos_.end();
  }

  // Insere os dados de um ponto na base de dados.
  void InserePonto(int x_quad, int y_quad, double altura, bool xorigem, bool yorigem) {
    XYQuad xy = { x_quad, y_quad, xorigem, yorigem };
    if (PontoExiste(x_quad, y_quad, xorigem, yorigem)) {
      throw std::logic_error(std::string("Ponto ja existe: ") + xy.ParaString());
    }
    DadosPonto dp;
    dp.x = ConverteXQuad(x_quad);
    dp.y = ConverteYQuad(y_quad);
    dp.z = static_cast<float>(altura);
    dp.s = x_quad * inc_s_;
    dp.t = 1.0f - (y_quad * inc_t_);
    dp.indice = static_cast<int>(pontos_.size());
    mapa_pontos_[xy] = std::move(dp);
    pontos_.push_back(&mapa_pontos_[xy]);
  }

  // Percorre os pontos, inserindo os dados de cada um.
  template <class Container>
  void CriaPontos(int num_x_quad, int num_y_quad, const Wrapper<Container> pontos) {
    int num_x = num_x_quad + 1;
    int num_y = num_y_quad + 1;
    VLOG(1) << "CriaPontos " << num_x << ", " << num_y;
    if (!pontos.empty() && pontos.size() != size_t(num_x * num_y)) {
      throw std::logic_error("Numero de pontos invalido na criacao");
    }
    // Preenche todas.
    for (int ytab = 0; ytab < num_y; ++ytab) {
      for (int xtab = 0; xtab < num_x; ++xtab) {
        InserePonto(xtab, ytab, pontos[ytab * num_x + xtab], true, true);
      }
    }
  }

  void ComputaNormais(int num_x_quad, int num_y_quad) {
    int num_x = num_x_quad + 1;
    int num_y = num_y_quad + 1;
    for (int ytab = 0; ytab < num_y; ++ytab) {
      for (int xtab = 0; xtab < num_x; ++xtab) {
        float esq = mapa_pontos_.find({std::max(xtab - 1, 0), ytab, true, true})->second.z;
        float dir = mapa_pontos_.find({std::min(xtab + 1, num_x - 1), ytab, true, true})->second.z;
        float baixo = mapa_pontos_.find({xtab, std::max(ytab - 1, 0), true, true})->second.z;
        float cima = mapa_pontos_.find({xtab, std::min(ytab + 1, num_y - 1), true, true})->second.z;
        Vector3 n(esq - dir, baixo - cima, 2.0f);
        n.normalize();
        auto it = mapa_pontos_.find({xtab, ytab, true, true});
        it->second.nx = n.x;
        it->second.ny = n.y;
        it->second.nz = n.z;
      }
    }
  }

  void CriaMalha(int num_x_quad, int num_y_quad) {
    int num_x = num_x_quad + 1;
    int num_y = num_y_quad + 1;
    VLOG(1) << "Criando malha " << num_x << ", " << num_y;
    for (int ytab = 0; ytab < (num_y - 1); ++ytab) {
      for (int xtab = 0; xtab < (num_x - 1); ++xtab) {
        VLOG(2) << "Criando quadrado " << xtab << ", " << ytab;
        bool b0 = true;
        bool b1 = true;
        int x0y0 = IndicePonto(xtab,     ytab,     b0, b0);
        int x1y0 = IndicePonto(xtab + 1, ytab,     b1, b0);
        int x1y1 = IndicePonto(xtab + 1, ytab + 1, b1, b1);
        int x0y1 = IndicePonto(xtab    , ytab + 1, b0, b1);
        indices_.push_back(x0y0);
        indices_.push_back(x1y0);
        if (Inverte(xtab, ytab)) {
          indices_.push_back(x0y1);
          indices_.push_back(x1y0);
        } else {
          indices_.push_back(x1y1);
          indices_.push_back(x0y0);
        }
        indices_.push_back(x1y1);
        indices_.push_back(x0y1);
      }
    }
  }

  int IndicePonto(int x_quad, int y_quad, bool xorigem, bool yorigem) const {
    XYQuad xy = { x_quad, y_quad, xorigem, yorigem };
    auto it = mapa_pontos_.find(xy);
    if (it == mapa_pontos_.end()) {
      LOG(ERROR) << "ponto " << x_quad << ", " << y_quad << ", " << xorigem << ", " << yorigem << " nao existe";
      throw std::logic_error(std::string("Ponto nao existe: ") + net::to_string(x_quad) + " " + net::to_string(y_quad));
    }
    return it->second.indice;
  }

  // Converte um x_quad em uma coordenada.
  float ConverteXQuad(int x_quad) const {
    return x_quad * TAMANHO_LADO_QUADRADO;
  }

  // Converte um y_quad em uma coordenada.
  float ConverteYQuad(int y_quad) const {
    return y_quad * TAMANHO_LADO_QUADRADO;
  }

  // Gera altura aleatoria de um ponto.
  static float GeraAltura(int x_quad, int y_quad) {
    std::minstd_rand motor_aleatorio(x_quad + (y_quad << 10));
    int mod = motor_aleatorio() % 100;
    int mod_2 = mod % 50;
    float res = 0.0f;
    if (mod_2 < 25) {
      res = 0.0f;
    } else if (mod_2 < 37) {
      res = 0.25;;
    } else if (mod_2 < 43) {
      res = 0.5;
    } else if (mod_2 < 46) {
      res = 0.75;
    } else if (mod_2 < 48) {
      res = 1.0f;
    } else {
      res = 1.5;
    }
    return mod >= 50 ? -res : res;
  }

 private:
  std::map<XYQuad, DadosPonto> mapa_pontos_;  // mapa de pontos, mapeia id quadrado para os dados do ponto.
  std::vector<DadosPonto*> pontos_;  // pontos de forma sequencia (como ficarao na saida).
  std::vector<unsigned short> indices_;  // aponta para pontos_.
  // textura.
  float inc_s_ = 0.0f;
  float inc_t_ = 0.0f;
};

}  // namespace ent

#endif
