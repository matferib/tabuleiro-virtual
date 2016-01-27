#ifndef ENT_TABULEIRO_TERRENO_H
#define ENT_TABULEIRO_TERRENO_H

#include <cstdlib>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "ent/constantes.h"
#include "matrix/matrices.h"

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
  // Constroi um terreno flat, com o numero de quadrados passado. O numero de pontos deve incluir a coluna final
  // leste e a linha final norte, para cobrir o tabuleiro todo.
  Terreno(int num_x_quad, int num_y_quad, bool ladrilho, const std::vector<double>& pontos) {
    ladrilho_ = ladrilho;
    if (pontos.size() != size_t((num_y_quad + 1) * (num_x_quad + 1))) {
      throw std::logic_error("Numero de pontos invalido.");
    }
    if (!ladrilho_) {
      inc_s_ = (1.0f / num_x_quad);
      inc_t_ = (1.0f / num_y_quad);
    }
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

  // Retorna o indice de um ponto no tabuleiro.
  static int IndicePonto(int x_quad, int y_quad, int num_x_quad) {
    int num_x = num_x_quad + 1;  // borda.
    return y_quad * num_x + x_quad;
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
    dp.z = altura;
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

  // Percorre os pontos, inserindo os dados de cada um.
  void CriaPontos(int num_x_quad, int num_y_quad, const std::vector<double>& pontos) {
    int num_x = num_x_quad + 1;
    int num_y = num_y_quad + 1;
    if (pontos.size() != size_t(num_x * num_y)) {
      throw std::logic_error("Numero de pontos invalido na criacao");
    }
    if (ladrilho_) {
      // So vai ate a penultima coluna, que ja cria a ultima.
      for (int ytab = 0; ytab < (num_y - 1); ++ytab) {
        for (int xtab = 0; xtab < (num_x - 1); ++xtab) {
          double altura_x0y0 = pontos[ytab       * num_x + xtab];
          double altura_x1y0 = pontos[ytab       * num_x + (xtab + 1)];
          double altura_x1y1 = pontos[(ytab + 1) * num_x + (xtab + 1)];
          double altura_x0y1 = pontos[(ytab + 1) * num_x + xtab];
          InserePonto(xtab,     ytab,     altura_x0y0, true,  true);
          InserePonto(xtab + 1, ytab,     altura_x1y0, false, true);
          InserePonto(xtab + 1, ytab + 1, altura_x1y1, false, false);
          InserePonto(xtab,     ytab + 1, altura_x0y1, true,  false);
        }
      }
    } else {
      // Preenche todas.
      for (int ytab = 0; ytab < num_y; ++ytab) {
        for (int xtab = 0; xtab < num_x; ++xtab) {
          InserePonto(xtab, ytab, pontos[ytab * num_x + xtab], true, true);
        }
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
        if (ladrilho_) {
          std::vector<XYQuad> xys = {
            { xtab, ytab, true, true },
            { xtab, ytab, false, true },
            { xtab, ytab, true, false },
            { xtab, ytab, false, false },
          };
          for (const auto& xy : xys) {
            auto it = mapa_pontos_.find(xy);
            // Nem todas combinacoes existem.
            if (it != mapa_pontos_.end()) {
              it->second.nx = n.x;
              it->second.ny = n.y;
              it->second.nz = n.z;
            }
          }
        } else {
          auto it = mapa_pontos_.find({xtab, ytab, true, true});
          it->second.nx = n.x;
          it->second.ny = n.y;
          it->second.nz = n.z;
        }
      }
    }
  }

  void CriaMalha(int num_x_quad, int num_y_quad) {
    int num_x = num_x_quad + 1;
    int num_y = num_y_quad + 1;
    for (int ytab = 0; ytab < (num_y - 1); ++ytab) {
      for (int xtab = 0; xtab < (num_x - 1); ++xtab) {
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

  int IndicePonto(int x_quad, int y_quad, bool xorigem, bool yorigem) const {
    XYQuad xy = { x_quad, y_quad, xorigem, yorigem };
    auto it = mapa_pontos_.find(xy);
    if (it == mapa_pontos_.end()) {
      throw std::logic_error("Ponto nao existe.");
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
    srandom(x_quad + (y_quad << 10));
    int mod = random() % 100;
    int mod_2 = mod / 2;
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
  bool ladrilho_ = false;
  std::map<XYQuad, DadosPonto> mapa_pontos_;  // mapa de pontos, mapeia id quadrado para os dados do ponto.
  std::vector<DadosPonto*> pontos_;  // pontos de forma sequencia (como ficarao na saida).
  std::vector<unsigned short> indices_;  // aponta para pontos_.
  // textura.
  float inc_s_ = 0.0f;
  float inc_t_ = 0.0f;
};

}  // namespace ent

#endif
