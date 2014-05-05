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
  }, {
    'L', {
      Q(I(1, 1), I(3, 1), I(3, 9), I(1, 9)),
      Q(I(3, 1), I(9, 1), I(9, 3), I(3, 3)),
    },
  }, {
  }, {
    'l', {
      Q(I(4, 2), I(6, 2), I(6, 9), I(4, 9)),  // Meio
      Q(I(4, 1), I(7, 1), I(7, 2), I(4, 2)),  // S
    },
  }, {
    'e', {
      Q(I(1, 2), I(3, 1), I(3, 6), I(1, 5)),  // W
      Q(I(3, 1), I(7, 1), I(9, 2), I(3, 2)),  // S
      Q(I(3, 3), I(7, 3), I(9, 4), I(3, 4)),  // Meio
      Q(I(7, 4), I(9, 4), I(9, 5), I(7, 5)),  // E
      Q(I(3, 5), I(7, 5), I(7, 6), I(3, 6)),  // N
    },
  }, {
    'i', {
      Q(I(4, 1), I(6, 1), I(6, 6), I(4, 6)),
      Q(I(4, 7), I(6, 7), I(6, 8), I(4, 8)),
    },
  }, {
    'I', {
      Q(I(4, 8), I(6, 8), I(6, 9), I(4, 9)),  // N
      Q(I(4, 2), I(6, 2), I(6, 8), I(4, 8)),  // Meio
      Q(I(3, 8), I(7, 8), I(7, 9), I(3, 9)),  // S
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
      Q(I(4, 1), I(6, 1), I(6, 7), I(4, 7)),  // S-N
      Q(I(1, 7), I(9, 7), I(9, 9), I(1, 9)),  // W-E
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
    'P', {
      Q(I(1, 8), I(9, 8), I(8, 9), I(1, 9)),  // N
      Q(I(1, 1), I(3, 1), I(3, 8), I(1, 8)),  // W
      Q(I(7, 5), I(9, 5), I(9, 8), I(7, 8)),  // E
      Q(I(3, 4), I(8, 4), I(9, 5), I(3, 5)),  // Meio
    },
  }, {
    'o', {
      Q(I(1, 2), I(3, 2), I(3, 5), I(1, 5)),  // W
      Q(I(1, 2), I(3, 1), I(7, 1), I(9, 2)),  // S
      Q(I(7, 2), I(9, 2), I(9, 5), I(7, 5)),  // E
      Q(I(1, 5), I(9, 5), I(7, 6), I(3, 6)),  // N
    },
  }, {
    'r', {
      Q(I(1, 1), I(3, 1), I(3, 6), I(1, 6)),  // W
      Q(I(3, 4), I(6, 4), I(6, 6), I(5, 6)),  // meio
    },
  },

  //Q(I(), I(), I(), I(),),
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
