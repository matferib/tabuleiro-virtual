#if USAR_OPENGL_ES

// OpenGL ES.

#include <cmath>
#include <unordered_map>
#include <unordered_set>
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

// Quadrado de triangulos de coordenadas
#define Q(V1, V2, V3, V4) V1, V2, V3, V1, V3, V4

// Letras deslocadas em 3 pixels.
const std::unordered_set<char> g_caracteres_baixos = { 'g', 'j', 'p', 'q', 'y' };

// O atlas de caracteres.
const std::unordered_map<char, std::vector<unsigned short>> g_indices_caracteres = {
  {
    '?', {
      Q(I(1, 7), I(3, 7), I(3, 8), I(1, 8)),  // NW
      Q(I(1, 8), I(9, 8), I(7, 9), I(3, 9)),  // N
      Q(I(7, 6), I(9, 6), I(9, 8), I(7, 8)),  // NE
      Q(I(4, 5), I(6, 5), I(9, 6), I(7, 6)),  // Meio-E
      Q(I(4, 3), I(6, 3), I(6, 5), I(4, 5)),  // Meio
      Q(I(4, 1), I(6, 1), I(6, 2), I(4, 2)),  // S
    },
  }, {
    '-', {
      Q(I(1, 4), I(9, 4), I(9, 5), I(1, 5)),  // WE
    },
  }, {
    '=', {
      Q(I(1, 4), I(9, 4), I(9, 5), I(1, 5)),  // N
      Q(I(1, 2), I(9, 2), I(9, 3), I(1, 3)),  // S
    },
  }, {
    '+', {
      Q(I(1, 4), I(9, 4), I(9, 5), I(1, 5)),  // WE
      Q(I(4, 2), I(6, 2), I(6, 7), I(4, 7)),  // SN
    },
  }, {
    '^', {
      Q(I(1, 4), I(3, 4) ,I(6, 9), I(4, 9)) ,  // W
      Q(I(6, 9), I(4, 9), I(7, 4), I(9, 4)),   // E
    },
  }, {
    '<', {
      Q(I(1, 3), I(4, 3), I(4, 4), I(1, 4)),  // Meio
      Q(I(1, 3), I(9, 1), I(9, 2), I(4, 3)),  // S
      Q(I(1, 4), I(4, 4), I(9, 5), I(9, 6)),  // N
    },
  }, {
    '>', {
      Q(I(6, 3), I(9, 3), I(9, 4), I(6, 4)),  // Meio
      Q(I(1, 1), I(9, 3), I(6, 3), I(1, 2)),  // S
      Q(I(1, 5), I(6, 4), I(9, 4), I(1, 6)),  // N
    },
  }, {
    ':', {
      Q(I(3, 1), I(5, 1), I(5, 2), I(3, 2)),  // S
      Q(I(3, 4), I(5, 4), I(5, 5), I(3, 5)),  // N
    },
  }, {
    '.', {
      Q(I(3, 1), I(5, 1), I(5, 2), I(3, 2)),  // S
    },
  }, {
    '/', {
      Q(I(1, 1), I(3, 1), I(9, 9), I(7, 9)),
    },
  }, {
    '0', {
      Q(I(3, 8), I(7, 8), I(7, 9), I(3, 9)),  // N
      Q(I(1, 2), I(3, 1), I(3, 9), I(1, 8)),  // W
      Q(I(3, 1), I(7, 1), I(7, 2), I(3, 2)),  // S
      Q(I(7, 1), I(9, 2), I(9, 8), I(7, 9)),  // E
    },
  }, {
    '1', {
      Q(I(2, 1), I(8, 1), I(8, 2), I(2, 2)),  // Base.
      Q(I(4, 2), I(6, 2), I(6, 9), I(4, 9)),  // Corpo
      I(2, 7), I(4, 7), I(4, 9),  // Cabeca
    },
  }, {
    '2', {
      Q(I(1, 8), I(9, 8), I(7, 9), I(3, 9)),  // N.
      Q(I(1, 2), I(3, 2), I(9, 8), I(7, 8)),  // Meio.
      Q(I(1, 1), I(9, 1), I(9, 2), I(1, 2)),  // S.
    },
  }, {
    '3', {
      Q(I(1, 8), I(9, 8), I(9, 9), I(1, 9)),  // N.
      Q(I(4, 6), I(6, 6), I(9, 8), I(7, 8)),  // NE
      Q(I(4, 6), I(7, 5), I(9, 5), I(6, 6)),  // NEE.
      Q(I(7, 2), I(9, 2), I(9, 5), I(7, 5)),  // E.
      Q(I(1, 1), I(7, 1), I(9, 2), I(1, 2)),  // S.
    },
  }, {
    '4', {
      Q(I(1, 2), I(9, 2), I(9, 3), I(1, 3)), // S
      Q(I(6, 1), I(8, 1), I(8, 9), I(6, 9)), // E
      Q(I(1, 3), I(3, 3), I(6, 7), I(6, 9)), // W
    },
  }, {
    '5', {
      Q(I(1, 1), I(7, 1), I(9, 2), I(1, 2)),  // S
      Q(I(7, 2), I(9, 2), I(9, 5), I(7, 5)),  // E
      Q(I(1, 5), I(9, 5), I(7, 6), I(1, 6)),  // Meio
      Q(I(1, 6), I(3, 6), I(3, 8), I(1, 8)),  // W
      Q(I(1, 8), I(9, 8), I(9, 9), I(1, 9)),  // N
    },
  }, {
    '6', {
      Q(I(1, 6), I(3, 6), I(7, 9), I(5, 9)),  // N
      Q(I(1, 2), I(3, 1), I(3, 6), I(1, 6)),  // W
      Q(I(1, 2), I(3, 1), I(7, 1), I(9, 2)),  // S
      Q(I(7, 2), I(9, 2), I(9, 5), I(7, 5)),  // E
      Q(I(3, 5), I(9, 5), I(7, 6), I(3, 6)),  // Meio
    },
  }, {
    '7', {
      Q(I(1, 8), I(9, 8), I(9, 9), I(1, 9)),  // N
      Q(I(1, 1), I(3, 1), I(9, 8), I(7, 8)),  // S
    },
  }, {
    '8', {
      Q(I(4, 8), I(6, 8), I(6, 9), I(4, 9)),  // N
      Q(I(2, 6), I(4, 5), I(4, 9), I(2, 8)),  // NW
      Q(I(6, 5), I(8, 6), I(8, 8), I(6, 9)),  // NE
      Q(I(1, 5), I(9, 5), I(7, 6), I(3, 6)),  // Meio
      Q(I(7, 2), I(9, 2), I(9, 5), I(7, 5)),  // SE
      Q(I(1, 2), I(3, 2), I(3, 5), I(1, 5)),  // SW
      Q(I(1, 2), I(3, 1), I(7, 1), I(9, 2)),  // S
    },
  }, {
    '9', {
      Q(I(1, 5), I(3, 4), I(3, 9), I(1, 8)),  // W
      Q(I(3, 4), I(7, 4), I(7, 5), I(3, 5)),  // Meio
      Q(I(7, 2), I(9, 2), I(9, 8), I(7, 8)),  // E
      Q(I(3, 8), I(9, 8), I(7, 9), I(3, 9)),  // N
      Q(I(1, 2), I(3, 1), I(7, 1), I(9, 2)),  // S
    },
  }, {
    'A', {
      Q(I(1, 1), I(3, 1), I(6, 9), I(4, 9)), // W
      Q(I(7, 1), I(9, 1), I(6, 9), I(4, 9)), // E
      Q(I(3, 3), I(7, 3), I(7, 4), I(3, 4)), // Meio
    },
  }, {
    'a', {
      Q(I(1, 5), I(8, 5), I(7, 6), I(2, 6)),  // N
      Q(I(7, 2), I(8, 2), I(8, 5), I(7, 5)),  // E
      Q(I(1, 3), I(7, 3), I(7, 4), I(2, 4)),  // Meio
      Q(I(1, 2), I(2, 2), I(2, 3), I(1, 3)),  // SW
      Q(I(1, 2), I(2, 1), I(9, 1), I(8, 2)),  // S
    },
  }, {
    'B', {
      Q(I(1, 1), I(3, 1), I(3, 9), I(1, 9)),  // W
      Q(I(3, 1), I(7, 1), I(9, 2), I(3, 2)),  // S
      Q(I(3, 8), I(8, 8), I(7, 9), I(3, 9)),  // N
      Q(I(3, 5), I(7, 5), I(7, 6), I(3, 6)),  // Meio
      Q(I(6, 8), I(6, 5), I(8, 6), I(8, 8)),  // NE
      Q(I(7, 2), I(9, 2), I(9, 5), I(7, 6)),  // SE
    },
  }, {
    'b', {
      Q(I(1, 1), I(3, 1), I(3, 9), I(1, 9)),  // W
      Q(I(7, 1), I(9, 2), I(9, 5), I(7, 6)),  // E
      Q(I(3, 5), I(7, 5), I(7, 6), I(3, 6)),  // Meio
      Q(I(3, 1), I(7, 1), I(7, 2), I(3, 2)),  // S
    },
  }, {
    'C', {
      Q(I(1, 2), I(3, 1), I(3, 8), I(1, 8)),  // W
      Q(I(1, 8), I(9, 8), I(7, 9), I(3, 9)),  // N
      Q(I(1, 2), I(3, 1), I(7, 1), I(9, 2)),  // S
    },
  }, {
    'c', {
      Q(I(1, 2), I(3, 1), I(3, 6), I(1, 5)),  // W
      Q(I(1, 5), I(9, 5), I(7, 6), I(3, 6)),  // N
      Q(I(1, 2), I(3, 1), I(7, 1), I(9, 2)),  // S
    },
  }, {
    'd', {
      Q(I(1, 2), I(3, 1), I(7, 1), I(7, 2)),  // S
      Q(I(1, 2), I(3, 2), I(3, 5), I(1, 5)),  // W
      Q(I(1, 5), I(7, 5), I(7, 6), I(3, 6)),  // N
      Q(I(7, 1), I(9, 1), I(9, 9), I(7, 9)),  // E
    },
  }, {
    'E', {
      Q(I(1, 1), I(3, 1), I(3, 9), I(1, 9)),  // W
      Q(I(3, 1), I(9, 1), I(9, 2), I(3, 2)),  // S
      Q(I(3, 8), I(9, 8), I(9, 9), I(3, 9)),  // N
      Q(I(3, 5), I(7, 5), I(7, 6), I(3, 6)),  // Meio
    },
  }, {
    'e', {
      Q(I(1, 2), I(3, 1), I(3, 6), I(1, 5)),  // W
      Q(I(3, 1), I(7, 1), I(9, 2), I(3, 2)),  // S
      Q(I(3, 3), I(7, 3), I(9, 4), I(3, 4)),  // Meio
      Q(I(7, 4), I(9, 4), I(9, 5), I(7, 5)),  // E
      Q(I(3, 5), I(9, 5), I(7, 6), I(3, 6)),  // N
    },
  }, {
    'F', {
      Q(I(1, 1), I(3, 1), I(3, 9), I(1, 9)),  // W
      Q(I(3, 8), I(9, 8), I(9, 9), I(3, 9)),  // N
      Q(I(3, 5), I(7, 5), I(7, 6), I(3, 6)),  // Meio
    },
  }, {
    'f', {
      Q(I(3, 1), I(5, 1), I(5, 8), I(3, 8)),  // S-N
      Q(I(2, 5), I(7, 5), I(7, 6), I(2, 6)),  // Meio
      Q(I(3, 8), I(9, 8), I(7, 9), I(5, 9)),
    },
  }, {
    'G', {
      Q(I(1, 2), I(3, 1), I(3, 8), I(1, 8)),  // W
      Q(I(5, 4), I(9, 4), I(8, 5), I(5, 5)),  // NE
      Q(I(1, 8), I(9, 8), I(7, 9), I(3, 9)),  // N
      Q(I(7, 2), I(9, 2), I(9, 4), I(7, 4)),  // SE
      Q(I(1, 2), I(3, 1), I(7, 1), I(9, 2)),  // S
    },
  }, {
    'g', {
      Q(I(1, 5), I(3, 4), I(3, 9), I(1, 8)),  // W
      Q(I(1, 8), I(8, 8), I(6, 9), I(3, 9)),  // N
      Q(I(7, 1), I(9, 2), I(9, 9), I(7, 9)),  // E
      Q(I(3, 4), I(7, 4), I(7, 5), I(3, 5)),  // Meio
      Q(I(1, 2), I(3, 1), I(7, 1), I(9, 2)),  // S
    },
  }, {
    'H', {
      Q(I(1, 1), I(3, 1), I(3, 9), I(1, 9)),  // W
      Q(I(7, 1), I(9, 1), I(9, 9), I(7, 9)),  // E
      Q(I(3, 5), I(7, 5), I(7, 6), I(3, 6)),  // Meio
    },
  }, {
    'h', {
      Q(I(1, 1), I(3, 1), I(3, 9), I(1, 9)),  // W
      Q(I(7, 1), I(9, 1), I(9, 5), I(7, 6)),  // E
      Q(I(3, 5), I(9, 5), I(7, 6), I(3, 6)),  // Meio
    },
  }, {
    'I', {
      Q(I(2, 8), I(8, 8), I(8, 9), I(2, 9)),  // N
      Q(I(4, 2), I(6, 2), I(6, 8), I(4, 8)),  // Meio
      Q(I(2, 1), I(8, 1), I(8, 2), I(2, 2)),  // S
    },
  }, {
    'i', {
      Q(I(4, 1), I(6, 1), I(6, 6), I(4, 6)),
      Q(I(4, 7), I(6, 7), I(6, 8), I(4, 8)),
    },
  }, {
    'J', {
      Q(I(1, 2), I(3, 2), I(3, 3), I(1, 3)),  // W
      Q(I(1, 2), I(3, 1), I(7, 1), I(9, 2)),  // S
      Q(I(7, 2), I(9, 2), I(9, 9), I(7, 9)),  // E
      Q(I(6, 8), I(7, 8), I(7, 9), I(6, 9)),  // N
    },
  }, {
    'j', {
      Q(I(1, 2), I(2, 1), I(5, 1), I(6, 2)),  // S
      Q(I(4, 2), I(6, 2), I(6, 7), I(4, 7)),  // Meio
      Q(I(4, 8), I(6, 8), I(6, 9), I(4, 9)),  // N
    },
  }, {
    'k', {
      Q(I(1, 1), I(3, 1), I(3, 9), I(1, 9)),  // W
      Q(I(3, 4), I(9, 6), I(7, 6), I(1, 5)),  // NE
      Q(I(3, 3), I(7, 1), I(9, 1), I(3, 4)),  // SE
    },
  }, {
    'L', {
      Q(I(1, 1), I(3, 1), I(3, 9), I(1, 9)),  // W
      Q(I(3, 1), I(9, 1), I(9, 2), I(3, 2)),  // S
    },
  }, {
    'l', {
      Q(I(2, 8), I(4, 8), I(4, 9), I(2, 9)),  // N
      Q(I(4, 2), I(6, 2), I(6, 9), I(4, 9)),  // Meio
      Q(I(4, 1), I(8, 1), I(8, 2), I(4, 2)),  // S
    },
  }, {
    'm', {
      Q(I(1, 1), I(3, 1), I(3, 5), I(1, 5)),  // W
      Q(I(4, 1), I(6, 1), I(6, 5), I(4, 5)),  // Meio
      Q(I(7, 1), I(9, 1), I(9, 5), I(7, 5)),  // E
      Q(I(1, 5), I(9, 5), I(7, 6), I(1, 6)),  // N
    },
  }, {
    'n', {
      Q(I(1, 1), I(3, 1), I(3, 6), I(1, 6)),  // W
      Q(I(3, 5), I(9, 5), I(7, 6), I(3, 6)),  // N
      Q(I(7, 1), I(9, 1), I(9, 5), I(7, 5)),  // E
    },
  }, {
    'o', {
      Q(I(1, 2), I(3, 2), I(3, 5), I(1, 5)),  // W
      Q(I(1, 2), I(3, 1), I(7, 1), I(9, 2)),  // S
      Q(I(7, 2), I(9, 2), I(9, 5), I(7, 5)),  // E
      Q(I(1, 5), I(9, 5), I(7, 6), I(3, 6)),  // N
    },
  }, {
    'p', {
      Q(I(1, 8), I(9, 8), I(7, 9), I(1, 9)),  // N
      Q(I(1, 1), I(3, 1), I(3, 8), I(1, 8)),  // W
      Q(I(7, 5), I(9, 5), I(9, 8), I(7, 8)),  // E
      Q(I(3, 4), I(7, 4), I(9, 5), I(3, 5)),  // Meio
    },
  }, {
    'P', {
      Q(I(1, 8), I(9, 8), I(7, 9), I(1, 9)),  // N
      Q(I(1, 1), I(3, 1), I(3, 8), I(1, 8)),  // W
      Q(I(7, 5), I(9, 5), I(9, 8), I(7, 8)),  // E
      Q(I(3, 4), I(7, 4), I(9, 5), I(3, 5)),  // Meio
    },
  }, {
    'q', {
      Q(I(1, 5), I(3, 4), I(3, 9), I(1, 8)),  // W
      Q(I(3, 4), I(7, 4), I(7, 5), I(3, 5)),  // Meio
      Q(I(7, 1), I(9, 1), I(9, 9), I(7, 9)),  // E
      Q(I(3, 8), I(7, 8), I(7, 9), I(3, 9)),  // N
    },
  }, {
    'Q', {
      Q(I(3, 8), I(7, 8), I(7, 9), I(3, 9)),  // N
      Q(I(1, 2), I(3, 1), I(3, 9), I(1, 8)),  // W
      Q(I(3, 1), I(7, 1), I(7, 2), I(3, 2)),  // S
      Q(I(7, 1), I(9, 2), I(9, 8), I(7, 9)),  // E
      I(6, 1), I(7, 0), I(7, 1),  // perninha.
    },
  }, {
    'r', {
      Q(I(3, 1), I(5, 1), I(5, 6), I(3, 6)),  // W
      Q(I(5, 4), I(9, 4), I(9, 6), I(7, 6)),  // meio
    },
  }, {
    's', {
      Q(I(1, 5), I(9, 5), I(8, 6), I(2, 6)),  // N
      Q(I(1, 4), I(2, 4), I(2, 5), I(1, 5)),  // NW
      Q(I(1, 4), I(2, 3), I(9 ,3), I(8, 4)),  // Meio
      Q(I(8, 2), I(9, 2), I(9, 3), I(8, 3)),  // SE
      Q(I(1, 1), I(8, 1), I(9, 2), I(1, 2)),  // S
    },
  }, {
    't', {
      Q(I(4, 1), I(6, 1), I(6, 9), I(4, 9)),  // S-N
      Q(I(1, 6), I(9, 6), I(9, 7), I(1, 7)),  // W-E
    },
  }, {
    'T', {
      Q(I(4, 1), I(6, 1), I(6, 8), I(4, 8)),  // S-N
      Q(I(1, 8), I(9, 8), I(9, 9), I(1, 9)),  // W-E
    },
  }, {
    'u', {
      Q(I(1, 2), I(3, 2), I(3, 6), I(1, 6)),  // W
      Q(I(6, 2), I(8, 2), I(8, 6), I(6, 6)),  // E
      Q(I(1, 2), I(3, 1), I(9, 1), I(8, 2)),  // S
    },
  }, {
    'v', {
      Q(I(1, 6), I(4, 1), I(6, 1), I(3, 6)),  // W
      Q(I(4, 1), I(6, 1), I(9, 6), I(7, 6)),  // E
    },
  }, {
    'V', {
      Q(I(1, 9), I(4, 1), I(6, 1), I(3, 9)),  // W
      Q(I(4, 1), I(6, 1), I(9, 9), I(7, 9)),  // E
    },
  }, {
    'x', {
      Q(I(1, 1), I(3, 1), I(9, 6), I(7, 6)),  // SW-NE
      Q(I(7, 1), I(9, 1), I(3, 6), I(1, 6)),  // SE-NW
    },
  }, {
    'w', {
      Q(I(1, 2), I(3, 2), I(3, 6), I(1, 6)),  // W
      Q(I(4, 2), I(6, 2), I(6, 6), I(4, 6)),  // Meio
      Q(I(7, 2), I(9, 2), I(9, 6), I(7, 6)),  // E
      Q(I(1, 2), I(3, 1), I(7, 1), I(9, 2)),  // S
    },
  }, {
    'y', {
      Q(I(1, 1), I(3, 1), I(9, 9), I(7, 9)),  // SW-NE
      Q(I(1, 9), I(4, 4), I(5, 4), I(3, 9)),  // SE-NW
    },
  }, {
    'z', {
      Q(I(1, 1), I(9, 1), I(9, 2), I(1, 2)),  // S
      Q(I(1, 5), I(9, 5), I(9, 6), I(1, 6)),  // N
      Q(I(1, 1), I(9, 5), I(9, 6), I(1, 2)),  // SW-NE
    },
  },

  //Q(I(), I(), I(), I(),),
};

// GLUTES font data.
static const GLubyte Fixed8x13_Character_032[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* blank */
static const GLubyte Fixed8x13_Character_097[] = {  8,  0,  0,116,140,132,124,  4,120,  0,  0,  0,  0,  0}; /* "a" */
static const GLubyte Fixed8x13_Character_098[] = {  8,  0,  0,184,196,132,132,196,184,128,128,128,  0,  0};
static const GLubyte Fixed8x13_Character_099[] = {  8,  0,  0,120,132,128,128,132,120,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_100[] = {  8,  0,  0,116,140,132,132,140,116,  4,  4,  4,  0,  0};
static const GLubyte Fixed8x13_Character_101[] = {  8,  0,  0,120,132,128,252,132,120,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_102[] = {  8,  0,  0, 64, 64, 64, 64,248, 64, 64, 68, 56,  0,  0};
static const GLubyte Fixed8x13_Character_103[] = {  8,120,132,120,128,112,136,136,116,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_104[] = {  8,  0,  0,132,132,132,132,196,184,128,128,128,  0,  0};
static const GLubyte Fixed8x13_Character_105[] = {  8,  0,  0,248, 32, 32, 32, 32, 96,  0, 32,  0,  0,  0};
static const GLubyte Fixed8x13_Character_106[] = {  8,112,136,136,  8,  8,  8,  8, 24,  0,  8,  0,  0,  0};
static const GLubyte Fixed8x13_Character_107[] = {  8,  0,  0,132,136,144,224,144,136,128,128,128,  0,  0};
static const GLubyte Fixed8x13_Character_108[] = {  8,  0,  0,248, 32, 32, 32, 32, 32, 32, 32, 96,  0,  0};
static const GLubyte Fixed8x13_Character_109[] = {  8,  0,  0,130,146,146,146,146,236,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_110[] = {  8,  0,  0,132,132,132,132,196,184,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_111[] = {  8,  0,  0,120,132,132,132,132,120,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_112[] = {  8,128,128,128,184,196,132,196,184,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_113[] = {  8,  4,  4,  4,116,140,132,140,116,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_114[] = {  8,  0,  0, 64, 64, 64, 64, 68,184,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_115[] = {  8,  0,  0,120,132, 24, 96,132,120,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_116[] = {  8,  0,  0, 56, 68, 64, 64, 64,248, 64, 64,  0,  0,  0};
static const GLubyte Fixed8x13_Character_117[] = {  8,  0,  0,116,136,136,136,136,136,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_119[] = {  8,  0,  0, 68,170,146,146,130,130,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_118[] = {  8,  0,  0, 32, 80, 80,136,136,136,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_120[] = {  8,  0,  0,132, 72, 48, 48, 72,132,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_121[] = {  8,120,132,  4,116,140,132,132,132,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_122[] = {  8,  0,  0,252, 64, 32, 16,  8,252,  0,  0,  0,  0,  0}; /* "z" */
static const GLubyte Fixed8x13_Character_065[] = {  8,  0,  0,132,132,132,252,132,132,132, 72, 48,  0,  0}; /* "A" */
static const GLubyte Fixed8x13_Character_066[] = {  8,  0,  0,252, 66, 66, 66,124, 66, 66, 66,252,  0,  0};
static const GLubyte Fixed8x13_Character_067[] = {  8,  0,  0,120,132,128,128,128,128,128,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_068[] = {  8,  0,  0,252, 66, 66, 66, 66, 66, 66, 66,252,  0,  0};
static const GLubyte Fixed8x13_Character_069[] = {  8,  0,  0,252,128,128,128,240,128,128,128,252,  0,  0};
static const GLubyte Fixed8x13_Character_070[] = {  8,  0,  0,128,128,128,128,240,128,128,128,252,  0,  0};
static const GLubyte Fixed8x13_Character_071[] = {  8,  0,  0,116,140,132,156,128,128,128,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_072[] = {  8,  0,  0,132,132,132,132,252,132,132,132,132,  0,  0};
static const GLubyte Fixed8x13_Character_073[] = {  8,  0,  0,248, 32, 32, 32, 32, 32, 32, 32,248,  0,  0};
static const GLubyte Fixed8x13_Character_074[] = {  8,  0,  0,112,136,  8,  8,  8,  8,  8,  8, 60,  0,  0};
static const GLubyte Fixed8x13_Character_075[] = {  8,  0,  0,132,136,144,160,192,160,144,136,132,  0,  0};
static const GLubyte Fixed8x13_Character_076[] = {  8,  0,  0,252,128,128,128,128,128,128,128,128,  0,  0};
static const GLubyte Fixed8x13_Character_077[] = {  8,  0,  0,130,130,130,146,146,170,198,130,130,  0,  0};
static const GLubyte Fixed8x13_Character_078[] = {  8,  0,  0,132,132,132,140,148,164,196,132,132,  0,  0};
static const GLubyte Fixed8x13_Character_079[] = {  8,  0,  0,120,132,132,132,132,132,132,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_080[] = {  8,  0,  0,128,128,128,128,248,132,132,132,248,  0,  0};
static const GLubyte Fixed8x13_Character_081[] = {  8,  0,  4,120,148,164,132,132,132,132,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_082[] = {  8,  0,  0,132,136,144,160,248,132,132,132,248,  0,  0};
static const GLubyte Fixed8x13_Character_083[] = {  8,  0,  0,120,132,  4,  4,120,128,128,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_084[] = {  8,  0,  0, 16, 16, 16, 16, 16, 16, 16, 16,254,  0,  0};
static const GLubyte Fixed8x13_Character_085[] = {  8,  0,  0,120,132,132,132,132,132,132,132,132,  0,  0};
static const GLubyte Fixed8x13_Character_087[] = {  8,  0,  0, 68,170,146,146,146,130,130,130,130,  0,  0};
static const GLubyte Fixed8x13_Character_086[] = {  8,  0,  0, 16, 40, 40, 40, 68, 68, 68,130,130,  0,  0};
static const GLubyte Fixed8x13_Character_088[] = {  8,  0,  0,130,130, 68, 40, 16, 40, 68,130,130,  0,  0};
static const GLubyte Fixed8x13_Character_089[] = {  8,  0,  0, 16, 16, 16, 16, 16, 40, 68,130,130,  0,  0};
static const GLubyte Fixed8x13_Character_090[] = {  8,  0,  0,252,128,128, 64, 32, 16,  8,  4,252,  0,  0}; /* "Z" */
static const GLubyte Fixed8x13_Character_048[] = {  8,  0,  0, 48, 72,132,132,132,132,132, 72, 48,  0,  0}; /* "0" */
static const GLubyte Fixed8x13_Character_049[] = {  8,  0,  0,248, 32, 32, 32, 32, 32,160, 96, 32,  0,  0};
static const GLubyte Fixed8x13_Character_050[] = {  8,  0,  0,252,128, 64, 48,  8,  4,132,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_051[] = {  8,  0,  0,120,132,  4,  4, 56, 16,  8,  4,252,  0,  0};
static const GLubyte Fixed8x13_Character_052[] = {  8,  0,  0,  8,  8,252,136,136, 72, 40, 24,  8,  0,  0};
static const GLubyte Fixed8x13_Character_053[] = {  8,  0,  0,120,132,  4,  4,196,184,128,128,252,  0,  0};
static const GLubyte Fixed8x13_Character_054[] = {  8,  0,  0,120,132,132,196,184,128,128, 64, 56,  0,  0};
static const GLubyte Fixed8x13_Character_055[] = {  8,  0,  0, 64, 64, 32, 32, 16, 16,  8,  4,252,  0,  0};
static const GLubyte Fixed8x13_Character_056[] = {  8,  0,  0,120,132,132,132,120,132,132,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_057[] = {  8,  0,  0,112,  8,  4,  4,116,140,132,132,120,  0,  0}; /* "9" */
static const GLubyte Fixed8x13_Character_096[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0, 16, 96,224,  0,  0}; /* "`" */
static const GLubyte Fixed8x13_Character_126[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0,144,168, 72,  0,  0}; /* "~" */
static const GLubyte Fixed8x13_Character_033[] = {  8,  0,  0,128,  0,128,128,128,128,128,128,128,  0,  0}; /* "!" */
static const GLubyte Fixed8x13_Character_064[] = {  8,  0,  0,120,128,148,172,164,156,132,132,120,  0,  0}; /* "@" */
static const GLubyte Fixed8x13_Character_035[] = {  8,  0,  0,  0, 72, 72,252, 72,252, 72, 72,  0,  0,  0}; /* "#" */
static const GLubyte Fixed8x13_Character_036[] = {  8,  0,  0,  0, 32,240, 40,112,160,120, 32,  0,  0,  0}; /* "$" */
static const GLubyte Fixed8x13_Character_037[] = {  8,  0,  0,136, 84, 72, 32, 16, 16, 72,164, 68,  0,  0}; /* "%" */
static const GLubyte Fixed8x13_Character_094[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0,136, 80, 32,  0,  0}; /* "^" */
static const GLubyte Fixed8x13_Character_038[] = {  8,  0,  0,116,136,148, 96,144,144, 96,  0,  0,  0,  0}; /* "&" */
static const GLubyte Fixed8x13_Character_042[] = {  8,  0,  0,  0,  0, 72, 48,252, 48, 72,  0,  0,  0,  0}; /* "*" */
static const GLubyte Fixed8x13_Character_040[] = {  8,  0,  0, 32, 64, 64,128,128,128, 64, 64, 32,  0,  0}; /* "(" */
static const GLubyte Fixed8x13_Character_041[] = {  8,  0,  0,128, 64, 64, 32, 32, 32, 64, 64,128,  0,  0}; /* ")" */
static const GLubyte Fixed8x13_Character_045[] = {  8,  0,  0,  0,  0,  0,  0,252,  0,  0,  0,  0,  0,  0}; /* "-" */
static const GLubyte Fixed8x13_Character_095[] = {  8,  0,254,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "_" */
static const GLubyte Fixed8x13_Character_061[] = {  8,  0,  0,  0,  0,252,  0,  0,252,  0,  0,  0,  0,  0}; /* "=" */
static const GLubyte Fixed8x13_Character_043[] = {  8,  0,  0,  0,  0, 32, 32,248, 32, 32,  0,  0,  0,  0}; /* "+" */
static const GLubyte Fixed8x13_Character_091[] = {  8,  0,  0,240,128,128,128,128,128,128,128,240,  0,  0}; /* "[" */
static const GLubyte Fixed8x13_Character_123[] = {  8,  0,  0, 56, 64, 64, 32,192, 32, 64, 64, 56,  0,  0}; /* "{" */
static const GLubyte Fixed8x13_Character_125[] = {  8,  0,  0,224, 16, 16, 32, 24, 32, 16, 16,224,  0,  0}; /* "}" */
static const GLubyte Fixed8x13_Character_093[] = {  8,  0,  0,240, 16, 16, 16, 16, 16, 16, 16,240,  0,  0}; /* "]" */
static const GLubyte Fixed8x13_Character_059[] = {  8,  0,128, 96,112,  0,  0, 32,112, 32,  0,  0,  0,  0}; /* ";" */
static const GLubyte Fixed8x13_Character_058[] = {  8,  0, 64,224, 64,  0,  0, 64,224, 64,  0,  0,  0,  0}; /* ":" */
static const GLubyte Fixed8x13_Character_044[] = {  8,  0,128, 96,112,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "," */
static const GLubyte Fixed8x13_Character_046[] = {  8,  0, 64,224, 64,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "." */
static const GLubyte Fixed8x13_Character_060[] = {  8,  0,  0,  8, 16, 32, 64,128, 64, 32, 16,  8,  0,  0}; /* "<" */
static const GLubyte Fixed8x13_Character_062[] = {  8,  0,  0,128, 64, 32, 16,  8, 16, 32, 64,128,  0,  0}; /* ">" */
static const GLubyte Fixed8x13_Character_047[] = {  8,  0,  0,128,128, 64, 32, 16,  8,  4,  2,  2,  0,  0}; /* "/" */
static const GLubyte Fixed8x13_Character_063[] = {  8,  0,  0, 16,  0, 16, 16,  8,  4,132,132,120,  0,  0}; /* "?" */
static const GLubyte Fixed8x13_Character_092[] = {  8,  0,  0,  2,  2,  4,  8, 16, 32, 64,128,128,  0,  0}; /* "\" */
static const GLubyte Fixed8x13_Character_034[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0,144,144,144,  0,  0}; /* """ */

/* Missing Characters filled in by John Fay by hand ... */
static const GLubyte Fixed8x13_Character_039[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0, 32, 32, 32,  0,  0}; /* """ */
static const GLubyte Fixed8x13_Character_124[] = {  8, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,  0,  0}; /* """ */


/* The font characters mapping: */
static const GLubyte* Fixed8x13_Character_Map[] = {Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_032,Fixed8x13_Character_033,Fixed8x13_Character_034,Fixed8x13_Character_035,Fixed8x13_Character_036,Fixed8x13_Character_037,Fixed8x13_Character_038,Fixed8x13_Character_039,Fixed8x13_Character_040,
   Fixed8x13_Character_041,Fixed8x13_Character_042,Fixed8x13_Character_043,Fixed8x13_Character_044,Fixed8x13_Character_045,Fixed8x13_Character_046,Fixed8x13_Character_047,Fixed8x13_Character_048,Fixed8x13_Character_049,Fixed8x13_Character_050,Fixed8x13_Character_051,Fixed8x13_Character_052,Fixed8x13_Character_053,Fixed8x13_Character_054,Fixed8x13_Character_055,Fixed8x13_Character_056,Fixed8x13_Character_057,Fixed8x13_Character_058,Fixed8x13_Character_059,Fixed8x13_Character_060,Fixed8x13_Character_061,Fixed8x13_Character_062,Fixed8x13_Character_063,Fixed8x13_Character_064,Fixed8x13_Character_065,Fixed8x13_Character_066,Fixed8x13_Character_067,Fixed8x13_Character_068,Fixed8x13_Character_069,Fixed8x13_Character_070,Fixed8x13_Character_071,Fixed8x13_Character_072,Fixed8x13_Character_073,Fixed8x13_Character_074,Fixed8x13_Character_075,Fixed8x13_Character_076,Fixed8x13_Character_077,Fixed8x13_Character_078,Fixed8x13_Character_079,Fixed8x13_Character_080,Fixed8x13_Character_081,Fixed8x13_Character_082,
   Fixed8x13_Character_083,Fixed8x13_Character_084,Fixed8x13_Character_085,Fixed8x13_Character_086,Fixed8x13_Character_087,Fixed8x13_Character_088,Fixed8x13_Character_089,Fixed8x13_Character_090,Fixed8x13_Character_091,Fixed8x13_Character_092,Fixed8x13_Character_093,Fixed8x13_Character_094,Fixed8x13_Character_095,Fixed8x13_Character_096,Fixed8x13_Character_097,Fixed8x13_Character_098,Fixed8x13_Character_099,Fixed8x13_Character_100,Fixed8x13_Character_101,Fixed8x13_Character_102,Fixed8x13_Character_103,Fixed8x13_Character_104,Fixed8x13_Character_105,Fixed8x13_Character_106,Fixed8x13_Character_107,Fixed8x13_Character_108,Fixed8x13_Character_109,Fixed8x13_Character_110,Fixed8x13_Character_111,Fixed8x13_Character_112,Fixed8x13_Character_113,Fixed8x13_Character_114,Fixed8x13_Character_115,Fixed8x13_Character_116,Fixed8x13_Character_117,Fixed8x13_Character_118,Fixed8x13_Character_119,Fixed8x13_Character_120,Fixed8x13_Character_121,Fixed8x13_Character_122,Fixed8x13_Character_123,Fixed8x13_Character_124,
   Fixed8x13_Character_125,Fixed8x13_Character_126,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,
   Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,
   Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,
   Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,NULL};
#if 0
void DesenhaCaractere(char c) {
  const auto& caractere_it = g_indices_caracteres.find(c);
  if (caractere_it == g_indices_caracteres.end()) {
    return;
  }
  gl::MatrizEscopo salva_matriz(false);
  if (g_caracteres_baixos.find(caractere_it->first) != g_caracteres_baixos.end()) {
    gl::Translada(0.0f, -0.3f, 0.0f, false);
  }
  gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  gl::PonteiroVertices(2, GL_FLOAT, &g_vertices_caracteres[0]);
  gl::DesenhaElementos(GL_TRIANGLES, caractere_it->second.size(), GL_UNSIGNED_SHORT, &caractere_it->second[0]);
  gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}
#else
int BitsToIndexedShorts(const GLubyte *bits, int ssx, int ssy, GLshort *points, int startx, int starty)
{
	int ssxsy = ssx * ssy; // total bit #
	int x = 0;
	int bit = 7, count = 0;

	while(ssxsy--)
	{
		if(bits[0] & (1 << bit))
		{
			points[0] = startx + x;
			points[1] = starty;
			count++;
			points += 2;
		}
		if(--bit == -1)
		{
			bit = 7;
			bits++;
		}
		if(++x >= ssx)
		{
			x = 0;
			starty++;
		}
	}
	return count;
}

void __glutBitmapCharacter(int character) {
	int i, nbpoints;
	GLushort indices[64*64];

  if (!(character >= 1)&&(character < 256)) return;

  const GLubyte* face = Fixed8x13_Character_Map[character - 1];

  GLshort* points = (GLshort*)malloc(13 * face[0] * 2 * sizeof(GLshort));
  nbpoints = BitsToIndexedShorts(face + 1, face[0], 13, points, 0, 0);

  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_NOTEQUAL, 0);

  for (i = 0; i < nbpoints; i++) indices[i] = i;

  gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  gl::PonteiroVertices(2, GL_SHORT, 0, points);
  gl::DesenhaElementos(GL_POINTS, nbpoints, GL_UNSIGNED_SHORT, indices);
  gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
  glDisable(GL_ALPHA_TEST);
  free(points);
}

void DesenhaCaractere(char c) {
  __glutBitmapCharacter((int)c);
}

#endif

}

#endif
