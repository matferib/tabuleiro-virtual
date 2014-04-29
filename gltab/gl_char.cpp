#if USAR_OPENGL_ES

// OpenGL ES.

#include <cmath>
#include <unordered_map>
#include <utility>
#include <vector>
#include "gltab/gl.h"
#include "log/log.h"

namespace gl {

// Atlas de caracteres. Aqui estao vertices usados para construi-los. Cada caractere esta em um espaco
// de 1.0 x 1.0 dividido em pontos em ordem crescente de 0.0, 0.1, 0.2 ... 1.0 de baixo para cima, depois
// da esquerda para a direita.
const int kResX = 10;
const int kResY = 10;

namespace  {

// Retorna o indice do vertice coordenada x, y.
unsigned short I(int x, int y) {
  return x * (kResY + 1) + y;
}

}  // namespace

// As coordenadas do quadrado 1x1 com resolucao 0.1.
const std::vector<float> g_vertices_caracteres = []() {
  std::vector<float> coordenadas;
  for (int l = 0; l <= kResX; ++l) {
    float x = 0.1f * static_cast<float>(l);
    for (int a = 0; a <= kResY; ++a) {
      float y = 0.1f * static_cast<float>(a);
      coordenadas.push_back(x);
      coordenadas.push_back(y);
    }
  }
  return coordenadas;
}();

// Quadrado de triangulos de coordendadas
#define Q(V1, V2, V3, V4) V1, V2, V3, V1, V3, V4

// O atlas de caracteres.
const std::unordered_map<char, std::vector<unsigned short>> g_indices_caracteres = {
  {
    '0', {
      Q(I(3, 1), I(4, 3), I(3, 4), I(1, 3)),  // SW
      Q(I(3, 1), I(7, 1), I(6, 3), I(4, 3)),  // S
      Q(I(6, 3), I(7, 1), I(9, 3), I(7, 4)),  // SE
      Q(I(7, 4), I(9, 3), I(9, 7), I(7, 6)),  // E
      Q(I(7, 6), I(9, 7), I(7, 9), I(6, 7)),  // NE
      Q(I(4, 7), I(6, 7), I(7, 9), I(3, 9)),  // N
      Q(I(3, 6), I(4, 7), I(3, 9), I(1, 7)),  // NW
      Q(I(1, 3), I(3, 4), I(3, 6), I(1, 7)),  // W
    },
  }, {
    '1', {
      Q(I(1, 1), I(9, 1), I(9, 3), I(1, 3)),  // Base.
      Q(I(4, 3), I(6, 3), I(6, 9), I(4, 9)),  // Corpo
      I(1, 7), I(4, 7), I(4, 9),  // Cabeca
    },
  }, {
    '2', {
      Q(I(1, 1), I(8, 1), I(7, 3), I(1, 3)),  // S
      Q(I(8, 1), I(9, 2), I(8, 4), I(7, 3)),  // SE
      Q(I(1, 3), I(5, 3), I(9, 6), I(7, 7)),  // Corpo
      Q(I(7, 7), I(9, 6), I(9, 8), I(8, 9)),  // NE
      Q(I(1, 7), I(7, 7), I(8, 9), I(3, 9)),  // N
    },
  }, {
    '3', {
      Q(I(2, 1), I(8, 1), I(7, 3), I(1, 3)),  // S
      Q(I(7, 3), I(8, 1), I(9, 2), I(9, 5)), I(7, 3), I(9, 5), I(7, 5),  // SE
      Q(I(7, 5), I(9, 5), I(7, 6), I(5, 6)),  // E
      Q(I(5, 6), I(7, 6), I(9, 9), I(7, 9)),  // NE
      Q(I(1, 7), I(7, 7), I(9, 9), I(1, 9)),  // N
    },
  }, {
    '4', {
      Q(I(1, 2), I(9, 2), I(9, 4), I(1, 4)),
      Q(I(6, 1), I(8, 1), I(8, 9), I(6, 9)),
      Q(I(1, 4), I(3, 4), I(6, 7), I(6, 9)),
    },
  }, {
    '5', {
      Q(I(1, 7), I(9, 7), I(9, 9), I(1, 9)),
      Q(I(1, 4), I(3, 4), I(3, 7), I(1, 7)),
      Q(I(3, 4), I(9, 4), I(8, 6), I(3, 6)),
      Q(I(7, 3), I(9, 3), I(9, 4), I(7, 4)),
      Q(I(1, 1), I(8, 1), I(9, 3), I(1, 3)),
    },
  }, {
    '6', {
      Q(I(1, 7), I(9, 7), I(7, 9), I(3, 9)),  // N
      Q(I(1, 3), I(3, 1), I(3, 7), I(1, 7)),  // W
      Q(I(3, 1), I(7, 1), I(9, 3), I(3, 3)),  // S
      Q(I(7, 3), I(9, 3), I(9, 4), I(7, 4)),  // E
      Q(I(3, 4), I(9, 4), I(7, 6), I(3, 6)),  // Meio
    },
  }, {
    '7', {
      Q(I(1, 7), I(7, 7), I(7, 9), I(1, 9)),  // N
      Q(I(1, 1), I(3, 1), I(9, 9), I(7, 9)),  // S
    },
  }, {
    '8', {
      Q(I(2, 8), I(8, 8), I(7, 9), I(3, 9)),  // N
      Q(I(2, 7), I(4, 7), I(4, 8), I(2, 8)),  // NWW
      Q(I(6, 7), I(8, 7), I(8, 8), I(6, 8)),  // NEE
      Q(I(3, 6), I(7, 6), I(8, 7), I(2, 7)),  // Meio
      Q(I(4, 5), I(6, 5), I(7, 6), I(3, 6)),  // Logo abaixo. 
      Q(I(1, 4), I(3, 4), I(4, 5), I(3, 6)), I(1, 2), I(3, 4), I(1, 4),  // SWW
      Q(I(7, 4), I(9, 4), I(7, 6), I(6, 5)), I(7, 4), I(9, 2), I(9, 4),  // SEE
      Q(I(1, 2), I(3, 1), I(4, 3), I(3, 4)),  // SW
      Q(I(6, 3), I(7, 1), I(9, 2), I(7, 4)),  // SE
      Q(I(3, 1), I(7, 1), I(6, 3), I(4, 3)),  // S
    },
  }, {
    '9', {
      Q(I(1, 3), I(3, 1), I(7, 1), I(9, 3)),
      Q(I(7, 3), I(9, 3), I(9, 7), I(7, 9)),
      Q(I(1, 7), I(7, 7), I(7, 9), I(3, 9)),
      Q(I(1, 6), I(3, 4), I(3, 7), I(1, 7)),
      Q(I(3, 4), I(7, 4), I(7, 6), I(3, 6)),
    },
  },
};

void DesenhaCaractere(char c) {
  const auto& caractere_it = g_indices_caracteres.find(c);
  if (caractere_it == g_indices_caracteres.end()) {
    return;
  }
  gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  gl::PonteiroVertices(2, GL_FLOAT, &g_vertices_caracteres[0]);
  gl::DesenhaElementos(GL_TRIANGLES, caractere_it->second.size(), GL_UNSIGNED_SHORT, &caractere_it->second[0]);
  gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}

}

#endif
