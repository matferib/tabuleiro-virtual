#include <cmath>
#include <cstring>
#include <string>
#include <vector>
#include "gltab/gl.h"
#include "log/log.h"

// Comum.
namespace gl {

bool ImprimeSeErro() {
  auto erro = glGetError();
  if (erro != GL_NO_ERROR) {
#if __APPLE__
    LOG(ERROR) << "OpenGL Erro: " << erro;
#else
    LOG(ERROR) << "OpenGL Erro: " << gluErrorString(erro);
#endif
    return true;
  }
  return false;
}
#define V_ERRO() do { if (ImprimeSeErro()) return; } while (0)

namespace interno {

const std::vector<std::string> QuebraString(const std::string& entrada, char caractere_quebra) {
  std::vector<std::string> ret;
  if (entrada.empty()) {
    return ret;
  }
  auto it_inicio = entrada.begin();
  auto it = it_inicio;
  while (it != entrada.end()) {
    if (*it == caractere_quebra) {
      ret.push_back(std::string(it_inicio, it));
      it_inicio = ++it;
    } else {
      ++it;
    }
  }
  ret.push_back(std::string(it_inicio, it));
  return ret;
}

// Implementado diferente em cada um.
void DesenhaStringAlinhado(const std::string& str, int alinhamento, bool inverte_vertical);

}  // interno

// Sao funcoes iguais dos dois lados que dependem de implementacoes diferentes.
void DesenhaString(const std::string& str, bool inverte_vertical) {
  interno::DesenhaStringAlinhado(str, 0, inverte_vertical);
}

void DesenhaStringAlinhadoEsquerda(const std::string& str, bool inverte_vertical) {
  interno::DesenhaStringAlinhado(str, -1, inverte_vertical);
}

void DesenhaStringAlinhadoDireita(const std::string& str, bool inverte_vertical) {
  interno::DesenhaStringAlinhado(str, 1, inverte_vertical);
}

const Vbo VboConeSolido(GLfloat base, GLfloat altura, GLint num_fatias, GLint num_tocos) {
  return VboTroncoConeSolido(base, 0.0f, altura, num_fatias, num_tocos);
}

const Vbo VboTroncoConeSolido(GLfloat raio_base, GLfloat raio_topo_original, GLfloat altura, GLint num_fatias, GLint num_tocos) {
  const int num_vertices_por_fatia = 4;
  const int num_vertices_por_toco = num_vertices_por_fatia * num_fatias;
  const int num_coordenadas_por_toco = num_vertices_por_toco * 3;
  const int num_coordenadas_total = num_coordenadas_por_toco * num_tocos;
  const int num_indices_por_fatia = 6;
  const int num_indices_por_toco = num_indices_por_fatia * num_fatias;
  const int num_indices_total = num_indices_por_toco * num_tocos;

  float coordenadas[num_coordenadas_total];
  float normais[num_coordenadas_total];
  unsigned short indices[num_indices_total];

  float h_delta = altura / num_tocos;
  float h_topo = 0;
  float delta_raio = (raio_base - raio_topo_original) / num_tocos;
  float raio_topo = raio_base;

  float angulo_fatia = (360.0f * GRAUS_PARA_RAD) / num_fatias;
  float cos_fatia = cosf(angulo_fatia);
  float sen_fatia = sinf(angulo_fatia);

  int i_coordenadas = 0;
  int i_indices = 0;
  int coordenada_inicial = 0;
  float v_base[2];
  float v_topo[2] = { 0.0f, raio_base };
  float beta_rad = atanf(altura / (raio_base - raio_topo_original));
  float alfa_rad = (M_PI / 2.0f) - beta_rad;
  float sen_alfa = sinf(alfa_rad);
  float cos_alfa = cosf(alfa_rad);
  for (int t = 1; t <= num_tocos; ++t) {
    float h_base = h_topo;
    h_topo += h_delta;
    // Novas alturas e base.
    v_base[0] = 0.0f;
    v_base[1] = raio_topo;
    raio_topo -= delta_raio;
    v_topo[0] = 0.0f;
    v_topo[1] = raio_topo;
    // Normal: TODO fazer direito.
    float v_normal[3];
    v_normal[0] = 0.0f;
    v_normal[1] = cos_alfa;
    v_normal[2] = sen_alfa;

    for (int f = 0; f < num_fatias; ++f) {
      // Cada faceta possui 4 vertices (anti horario).
      // V0 = vbase.
      coordenadas[i_coordenadas] = v_base[0];
      coordenadas[i_coordenadas + 1] = v_base[1];
      coordenadas[i_coordenadas + 2] = h_base;
      // V1 = vbase rodado.
      float v_base_0_rodado = v_base[0] * cos_fatia - v_base[1] * sen_fatia;
      float v_base_1_rodado = v_base[0] * sen_fatia + v_base[1] * cos_fatia;
      v_base[0] = v_base_0_rodado;
      v_base[1] = v_base_1_rodado;
      coordenadas[i_coordenadas + 3] = v_base[0];
      coordenadas[i_coordenadas + 4] = v_base[1];
      coordenadas[i_coordenadas + 5] = h_base;
      // v3 = vtopo.
      coordenadas[i_coordenadas + 9] = v_topo[0];
      coordenadas[i_coordenadas + 10] = v_topo[1];
      coordenadas[i_coordenadas + 11] = h_topo;
      // V2 = vtopo rodado.
      float v_topo_0_rodado = v_topo[0] * cos_fatia - v_topo[1] * sen_fatia;
      float v_topo_1_rodado = v_topo[0] * sen_fatia + v_topo[1] * cos_fatia;
      v_topo[0] = v_topo_0_rodado;
      v_topo[1] = v_topo_1_rodado;
      coordenadas[i_coordenadas + 6] = v_topo[0];
      coordenadas[i_coordenadas + 7] = v_topo[1];
      coordenadas[i_coordenadas + 8] = h_topo;

      // As normais.
      // Vn0.
      normais[i_coordenadas] = v_normal[0];
      normais[i_coordenadas + 1] = v_normal[1];
      normais[i_coordenadas + 2] = v_normal[2];
      // Vn3 = acima de vn0.
      normais[i_coordenadas + 9] = v_normal[0];
      normais[i_coordenadas + 10] = v_normal[1];
      normais[i_coordenadas + 11] = v_normal[2];
      // Vn1 = vn0 rodado.
      float v_normal_0_rodado = v_normal[0] * cos_fatia - v_normal[1] * sen_fatia;
      float v_normal_1_rodado = v_normal[0] * sen_fatia + v_normal[1] * cos_fatia;
      v_normal[0] = v_normal_0_rodado;
      v_normal[1] = v_normal_1_rodado;
      normais[i_coordenadas + 3] = v_normal[0];
      normais[i_coordenadas + 4] = v_normal[1];
      normais[i_coordenadas + 5] = v_normal[2];
      // Vn2 = acima de vn1.
      normais[i_coordenadas + 6] = v_normal[0];
      normais[i_coordenadas + 7] = v_normal[1];
      normais[i_coordenadas + 8] = v_normal[2];

      // Indices: V0, V1, V2, V0, V2, V3.
      indices[i_indices] = coordenada_inicial;
      indices[i_indices + 1] = coordenada_inicial + 1;
      indices[i_indices + 2] = coordenada_inicial + 2;
      indices[i_indices + 3] = coordenada_inicial;
      indices[i_indices + 4] = coordenada_inicial + 2;
      indices[i_indices + 5] = coordenada_inicial + 3;

      i_indices += 6;
      i_coordenadas += 12;
      coordenada_inicial += 4;
    }
  }

#if 0
  LOG(INFO) << "raio_base: " << raio_base;
  LOG(INFO) << "raio_topo: " << raio_topo_original;
  for (int i = 0; i < sizeof(indices) / sizeof(unsigned short); ++i) {
    LOG(INFO) << "indices[" << i << "]: " << indices[i];
  }
  for (int i = 0; i < sizeof(coordenadas) / sizeof(float); i += 3) {
    LOG(INFO) << "coordenadas[" << i / 3 << "]: "
              << coordenadas[i] << ", " << coordenadas[i + 1] << ", " << coordenadas[i + 2];
  }
#endif

  Vbo ret;
  ret.AtribuiCoordenadas(3, coordenadas, num_coordenadas_total);
  ret.AtribuiNormais(normais);
  ret.AtribuiIndices(indices, num_indices_total);
  return ret;
}

const Vbo VboEsferaSolida(GLfloat raio, GLint num_fatias, GLint num_tocos) {
  // Vertices.
  const int num_vertices_por_fatia = 4;
  const int num_vertices_por_toco = num_vertices_por_fatia * num_fatias;
  const int num_coordenadas_por_toco = num_vertices_por_toco * 3;
  const int num_indices_por_fatia = 6;
  const int num_indices_por_toco = num_indices_por_fatia * num_fatias;
  const int num_coordenadas_total = num_coordenadas_por_toco * num_tocos * 2;
  const int num_indices_total = num_indices_por_toco * num_tocos * 2;

  float angulo_h_rad = (90.0f * GRAUS_PARA_RAD) / num_tocos;
  float angulo_fatia = (360.0f * GRAUS_PARA_RAD) / num_fatias;
  float coordenadas[num_coordenadas_total];
  unsigned short indices[num_indices_total];
  float cos_fatia = cosf(angulo_fatia);
  float sen_fatia = sinf(angulo_fatia);

  float v_base[2];
  v_base[0] = 0;
  v_base[1] = 0;
  float v_topo[2];
  v_topo[0] = 0;
  v_topo[1] = raio;
  float h_topo = 0;

  int i_coordenadas = 0;
  int i_indices = 0;
  int coordenada_inicial = 0;

  float raio_topo = raio;
  for (int i = 1; i <= num_tocos; ++i) {
    float h_base = h_topo;
    // Novas alturas e base.
    float angulo_h_rad_vezes_i = angulo_h_rad * i;
    h_topo = sinf(angulo_h_rad_vezes_i) * raio;
    v_base[0] = 0.0f;
    v_base[1] = raio_topo;
    v_topo[0] = 0.0f;
    raio_topo = raio * cosf(angulo_h_rad_vezes_i);
    v_topo[1] = raio_topo;

    for (int i = 0; i < num_fatias; ++i) {
      // Cada faceta da esfera possui 4 vertices (anti horario). Cada vertices sera a propria normal.
      // V0 = vbase.
      coordenadas[i_coordenadas] = v_base[0];
      coordenadas[i_coordenadas + 1] = v_base[1];
      coordenadas[i_coordenadas + 2] = h_base;
      // V1 = vbase rodado.
      float v_base_0_rodado = v_base[0] * cos_fatia - v_base[1] * sen_fatia;
      float v_base_1_rodado = v_base[0] * sen_fatia + v_base[1] * cos_fatia;
      v_base[0] = v_base_0_rodado;
      v_base[1] = v_base_1_rodado;
      coordenadas[i_coordenadas + 3] = v_base[0];
      coordenadas[i_coordenadas + 4] = v_base[1];
      coordenadas[i_coordenadas + 5] = h_base;
      // v3 = vtopo.
      coordenadas[i_coordenadas + 9] = v_topo[0];
      coordenadas[i_coordenadas + 10] = v_topo[1];
      coordenadas[i_coordenadas + 11] = h_topo;
      // V2 = vtopo rodado.
      float v_topo_0_rodado = v_topo[0] * cos_fatia - v_topo[1] * sen_fatia;
      float v_topo_1_rodado = v_topo[0] * sen_fatia + v_topo[1] * cos_fatia;
      v_topo[0] = v_topo_0_rodado;
      v_topo[1] = v_topo_1_rodado;
      coordenadas[i_coordenadas + 6] = v_topo[0];
      coordenadas[i_coordenadas + 7] = v_topo[1];
      coordenadas[i_coordenadas + 8] = h_topo;

      // Simetria na parte de baixo.
      memcpy(coordenadas + i_coordenadas + 12, coordenadas + i_coordenadas, sizeof(float) * 12);
      for (int i = 2; i < 12 ; i += 3) {
        coordenadas[i_coordenadas + 12 + i] = -coordenadas[i_coordenadas + i];
      }

      // Indices: V0, V1, V2, V0, V2, V3.
      indices[i_indices] = coordenada_inicial;
      indices[i_indices + 1] = coordenada_inicial + 1;
      indices[i_indices + 2] = coordenada_inicial + 2;
      indices[i_indices + 3] = coordenada_inicial;
      indices[i_indices + 4] = coordenada_inicial + 2;
      indices[i_indices + 5] = coordenada_inicial + 3;
      indices[i_indices + 6] = coordenada_inicial + 4;
      indices[i_indices + 7] = coordenada_inicial + 7;
      indices[i_indices + 8] = coordenada_inicial + 6;
      indices[i_indices + 9] = coordenada_inicial + 4;
      indices[i_indices + 10] = coordenada_inicial + 6;
      indices[i_indices + 11] = coordenada_inicial + 5;

      i_indices += 12;
      i_coordenadas += 24;
      coordenada_inicial += 8;
    }
  }
  //LOG(INFO) << "raio: " << raio;
  //for (int i = 0; i < sizeof(indices) / sizeof(unsigned short); ++i) {
  //  LOG(INFO) << "indices[" << i << "]: " << indices[i];
  //}
  //for (int i = 0; i < sizeof(coordenadas) / sizeof(float); i += 3) {
  //  LOG(INFO) << "coordenadas[" << i / 3 << "]: "
  //            << coordenadas[i] << ", " << coordenadas[i + 1] << ", " << coordenadas[i + 2];
  //}

  Vbo ret;
  ret.AtribuiCoordenadas(3, coordenadas, num_coordenadas_total);
  ret.AtribuiNormais(coordenadas);  // TODO normalizar as normais. Por enquanto fica igual as coordenadas.
  ret.AtribuiIndices(indices, num_indices_total);
  return ret;
}

const Vbo VboCilindroSolido(GLfloat raio, GLfloat altura, GLint num_fatias, GLint num_tocos) {
  // Vertices.
  const int num_vertices_por_fatia = 4;
  const int num_vertices_por_toco = num_vertices_por_fatia * num_fatias;
  const int num_coordenadas_por_toco = num_vertices_por_toco * 3;
  const int num_indices_por_fatia = 6;
  const int num_indices_por_toco = num_indices_por_fatia * num_fatias;
  const int num_coordenadas_total = num_coordenadas_por_toco * num_tocos;
  const int num_indices_total = num_indices_por_toco * num_tocos;

  float angulo_fatia = (360.0f * GRAUS_PARA_RAD) / num_fatias;
  float coordenadas[num_coordenadas_total];
  float normais[num_coordenadas_total];
  unsigned short indices[num_indices_total];
  float cos_fatia = cosf(angulo_fatia);
  float sen_fatia = sinf(angulo_fatia);

  int i_normais = 0;
  float v_base[2];
  for (int toco = 1; toco <= num_tocos; ++toco) {
    v_base[0] = 0.0f;
    v_base[1] = raio;
    for (int i = 0; i < num_fatias; ++i) {
      // Cada faceta possui 4 vertices (anti horario).
      // V0 = vbase.
      normais[i_normais] = v_base[0];
      normais[i_normais + 1] = v_base[1];
      normais[i_normais + 2] = 0;
      // v3 = vbase topo.
      normais[i_normais + 9] = v_base[0];
      normais[i_normais + 10] = v_base[1];
      normais[i_normais + 11] = 0;
      // V1 = vbase rodado.
      float v_base_0_rodado = v_base[0] * cos_fatia - v_base[1] * sen_fatia;
      float v_base_1_rodado = v_base[0] * sen_fatia + v_base[1] * cos_fatia;
      v_base[0] = v_base_0_rodado;
      v_base[1] = v_base_1_rodado;
      normais[i_normais + 3] = v_base[0];
      normais[i_normais + 4] = v_base[1];
      normais[i_normais + 5] = 0;
      // V2 = vtopo rodado.
      normais[i_normais + 6] = v_base[0];
      normais[i_normais + 7] = v_base[1];
      normais[i_normais + 8] = 0;

      // Incrementa.
      i_normais += 12;
    }
  }

  float h_delta = altura / num_tocos;
  v_base[0] = 0;
  v_base[1] = raio;
  float h_topo = 0;

  int i_coordenadas = 0;
  int i_indices = 0;
  int coordenada_inicial = 0;
  for (int toco = 1; toco <= num_tocos; ++toco) {
    float h_base = h_topo;
    h_topo += h_delta;
    // Novas alturas e base.
    v_base[0] = 0.0f;
    v_base[1] = raio;

    for (int i = 0; i < num_fatias; ++i) {
      // Cada faceta possui 4 vertices (anti horario).
      // V0 = vbase.
      coordenadas[i_coordenadas] = v_base[0];
      coordenadas[i_coordenadas + 1] = v_base[1];
      coordenadas[i_coordenadas + 2] = h_base;
      // v3 = vbase topo.
      coordenadas[i_coordenadas + 9] = v_base[0];
      coordenadas[i_coordenadas + 10] = v_base[1];
      coordenadas[i_coordenadas + 11] = h_topo;
      // V1 = vbase rodado.
      float v_base_0_rodado = v_base[0] * cos_fatia - v_base[1] * sen_fatia;
      float v_base_1_rodado = v_base[0] * sen_fatia + v_base[1] * cos_fatia;
      v_base[0] = v_base_0_rodado;
      v_base[1] = v_base_1_rodado;
      coordenadas[i_coordenadas + 3] = v_base[0];
      coordenadas[i_coordenadas + 4] = v_base[1];
      coordenadas[i_coordenadas + 5] = h_base;
      // V2 = vtopo rodado.
      coordenadas[i_coordenadas + 6] = v_base[0];
      coordenadas[i_coordenadas + 7] = v_base[1];
      coordenadas[i_coordenadas + 8] = h_topo;

      // Indices: V0, V1, V2, V0, V2, V3.
      indices[i_indices] = coordenada_inicial;
      indices[i_indices + 1] = coordenada_inicial + 1;
      indices[i_indices + 2] = coordenada_inicial + 2;
      indices[i_indices + 3] = coordenada_inicial;
      indices[i_indices + 4] = coordenada_inicial + 2;
      indices[i_indices + 5] = coordenada_inicial + 3;

      i_indices += 6;
      i_coordenadas += 12;
      coordenada_inicial += 4;
    }
  }
  //LOG(INFO) << "raio: " << raio;
  //for (int i = 0; i < sizeof(indices) / sizeof(unsigned short); ++i) {
  //  LOG(INFO) << "indices[" << i << "]: " << indices[i];
  //}
  //for (int i = 0; i < sizeof(coordenadas) / sizeof(float); i += 3) {
  //  LOG(INFO) << "coordenadas[" << i / 3 << "]: "
  //            << coordenadas[i] << ", " << coordenadas[i + 1] << ", " << coordenadas[i + 2];
  //}

  Vbo ret;
  ret.AtribuiCoordenadas(3, coordenadas, num_coordenadas_total);
  ret.AtribuiNormais(normais);
  ret.AtribuiIndices(indices, num_indices_total);
  return ret;
}

const Vbo VboCuboSolido(GLfloat tam_lado) {
  GLfloat meio_lado = tam_lado / 2.0f;
  const unsigned short num_indices = 36;
  unsigned short indices[num_indices] = {
      0, 1, 2, 0, 2, 3,
      4, 5, 6, 4, 6, 7,
      8, 9, 10, 8, 10, 11,
      12, 13, 14, 12, 14, 15,
      16, 17, 18, 16, 18, 19,
      20, 21, 22, 20, 22, 23,
  };
  const unsigned short num_coordenadas = 6 * 3 * 4;
  const float normais[num_coordenadas] = {
    // sul.
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    // norte.
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    // oeste.
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    // leste.
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    // cima.
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    // baixo.
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
  };
  const float coordenadas[num_coordenadas] = {
    // sul: 0-3
    -meio_lado, -meio_lado, -meio_lado,
    meio_lado, -meio_lado, -meio_lado,
    meio_lado, -meio_lado, meio_lado,
    -meio_lado, -meio_lado, meio_lado,
    // norte: 4-7.
    -meio_lado, meio_lado, -meio_lado,
    -meio_lado, meio_lado, meio_lado,
    meio_lado, meio_lado, meio_lado,
    meio_lado, meio_lado, -meio_lado,
    // oeste: 8-11.
    -meio_lado, -meio_lado, -meio_lado,
    -meio_lado, -meio_lado, meio_lado,
    -meio_lado, meio_lado, meio_lado,
    -meio_lado, meio_lado, -meio_lado,
    // leste: 12-15.
    meio_lado, -meio_lado, -meio_lado,
    meio_lado, meio_lado, -meio_lado,
    meio_lado, meio_lado, meio_lado,
    meio_lado, -meio_lado, meio_lado,
    // cima: 16-19.
    -meio_lado, -meio_lado, meio_lado,
    meio_lado, -meio_lado, meio_lado,
    meio_lado, meio_lado, meio_lado,
    -meio_lado, meio_lado, meio_lado,
    // baixo: 20-23.
    -meio_lado, -meio_lado, -meio_lado,
    -meio_lado, meio_lado, -meio_lado,
    meio_lado, meio_lado, -meio_lado,
    meio_lado, -meio_lado, -meio_lado,
  };

  Vbo ret;
  ret.AtribuiCoordenadas(3, coordenadas, num_coordenadas);
  ret.AtribuiNormais(normais);
  ret.AtribuiIndices(indices, num_indices);
  return ret;
}

const Vbo VboPiramideSolida(GLfloat tam_lado, GLfloat altura) {
  const unsigned short indices[] = {
      0, 1, 2,  // sul
      3, 4, 5,  // leste
      6, 7, 8,  // norte
      9, 10, 11, // oeste
  };
  // Todas normais sao compostas pelo mesmo componente em direcoes diferentes por 90 graus.
  // Alfa eh o angulo da base com as paredes.
  // m = tam_lado / 2.0f.
  // t = lateral da piramide, tamanho: sqrt(m^2 + h^2).
  // cos(a) = m / t = componente eixo x-y.
  // sen(a) = h / t = componente z.
  const float m = tam_lado / 2.0f;
  const float h = altura;
  const float t = sqrt(m * m + h * h);
  const float cos_a = m / t;
  const float comp_xy = cos_a;
  const float sen_a = h / t;
  const float comp_z = sen_a;
  const float normais[] = {
    // Face sul
    0.0f, -comp_xy, comp_z,
    0.0f, -comp_xy, comp_z,
    0.0f, -comp_xy, comp_z,
    // Face leste.
    comp_xy, 0.0f, comp_z,
    comp_xy, 0.0f, comp_z,
    comp_xy, 0.0f, comp_z,
    // Face norte.
    0.0f, comp_xy, comp_z,
    0.0f, comp_xy, comp_z,
    0.0f, comp_xy, comp_z,
    // Face Oeste.
    -comp_xy, 0.0f, comp_z,
    -comp_xy, 0.0f, comp_z,
    -comp_xy, 0.0f, comp_z,
  };
  const float coordenadas[] = {
    // Topo.
    // Face sul.
    0.0f, 0.0f, altura,
    -m, -m, 0.0f,
    m, -m, 0.0f,
    // Face leste.
    0.0f, 0.0f, altura,
    m, -m, 0.0f,
    m, m, 0.0f,
    // Face norte.
    0.0f, 0.0f, altura,
    m, m, 0.0f,
    -m, m, 0.0f,
    // Face Oeste.
    0.0f, 0.0f, altura,
    -m, m, 0.0f,
    -m, -m, 0.0f,
  };
  Vbo vbo;
  vbo.AtribuiCoordenadas(3, coordenadas, sizeof(coordenadas) / sizeof(float));
  vbo.AtribuiIndices(indices, 12);
  vbo.AtribuiNormais(normais);
  return vbo;
}

const Vbo VboRetangulo(GLfloat tam_lado) {
  const unsigned short indices[] = { 0, 1, 2, 3 };
  float m = tam_lado / 2.0f;
  const float coordenadas[] = {
    -m, -m, 0.0f,
    m,  -m, 0.0f,
    m,  m,  0.0f,
    -m, m,  0.0f,
  };
  const float normais[] = {
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
  };
  const float coordenadas_texel[] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
  };
  Vbo vbo;
  vbo.AtribuiCoordenadas(3, coordenadas, sizeof(coordenadas) / sizeof(float));
  vbo.AtribuiNormais(normais);
  vbo.AtribuiTexturas(coordenadas_texel);
  vbo.AtribuiIndices(indices, 4);
  return vbo;
}

void GravaVbo(Vbo* vbo) {
  // Gera o buffer.
  gl::GeraBuffers(1, &vbo->nome_coordenadas);
  V_ERRO();
  // Associa coordenadas com ARRAY_BUFFER.
  gl::LigacaoComBuffer(GL_ARRAY_BUFFER, vbo->nome_coordenadas);
  V_ERRO();
  vbo->GeraBufferUnico();
  gl::BufferizaDados(GL_ARRAY_BUFFER,
                     sizeof(GL_FLOAT) * vbo->buffer_unico().size(),
                     vbo->buffer_unico().data(),
                     GL_STATIC_DRAW);
  V_ERRO();
  // Buffer de indices.
  gl::GeraBuffers(1, &vbo->nome_indices);
  V_ERRO();
  gl::LigacaoComBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo->nome_indices);
  V_ERRO();
  gl::BufferizaDados(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * vbo->indices().size(), vbo->indices().data(), GL_STATIC_DRAW);
  V_ERRO();
}

void DesgravaVbo(Vbo* vbo) {
  gl::ApagaBuffers(1, &vbo->nome_coordenadas);
  gl::ApagaBuffers(1, &vbo->nome_indices);
  vbo->ApagaBufferUnico();
}

void DesenhaVbo(const Vbo& vbo, GLenum modo) {
  // Os casts de char* 0 sao para evitar warning de conversao de short pra void*.
  gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  gl::LigacaoComBuffer(GL_ARRAY_BUFFER, vbo.nome_coordenadas);
  if (vbo.tem_normais()) {
    gl::HabilitaEstadoCliente(GL_NORMAL_ARRAY);
    gl::PonteiroNormais(GL_FLOAT, static_cast<char*>(0) + vbo.DeslocamentoNormais());
  }
  if (vbo.tem_texturas()) {
    gl::HabilitaEstadoCliente(GL_TEXTURE_COORD_ARRAY);
    gl::PonteiroVerticesTexturas(2, GL_FLOAT, 0, static_cast<char*>(0) + vbo.DeslocamentoTexturas());
  }

  gl::PonteiroVertices(3, GL_FLOAT, 0, (void*)0);
  gl::LigacaoComBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.nome_indices);
  gl::DesenhaElementos(modo, vbo.NumVertices(), GL_UNSIGNED_SHORT, (void*)0);

  gl::LigacaoComBuffer(GL_ARRAY_BUFFER, 0);
  gl::LigacaoComBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  gl::DesabilitaEstadoCliente(GL_NORMAL_ARRAY);
  gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}

void DesenhaVboNaoGravado(const Vbo& vbo, GLenum modo) {
  HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  if (vbo.tem_normais()) {
    HabilitaEstadoCliente(GL_NORMAL_ARRAY);
    PonteiroNormais(GL_FLOAT, vbo.normais().data());
  }
  if (vbo.tem_texturas()) {
    gl::HabilitaEstadoCliente(GL_TEXTURE_COORD_ARRAY);
    gl::PonteiroVerticesTexturas(2, GL_FLOAT, 0, vbo.texturas().data());
  }
  PonteiroVertices(vbo.num_dimensoes(), GL_FLOAT, vbo.coordenadas().data());
  DesenhaElementos(modo, vbo.indices().size(), GL_UNSIGNED_SHORT, vbo.indices().data());
  DesabilitaEstadoCliente(GL_NORMAL_ARRAY);
  DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
  DesabilitaEstadoCliente(GL_TEXTURE_COORD_ARRAY);
}

}  // namespace gl
