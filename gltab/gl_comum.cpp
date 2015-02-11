#include <cmath>
#include <string>
#include <vector>
#include "gltab/gl.h"

// Comum.
namespace gl {
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

const Vbo RetornaConeSolido(GLfloat base, GLfloat altura, GLint num_fatias, GLint num_tocos) {
  return RetornaTroncoConeSolido(base, 0.0f, altura, num_fatias, num_tocos);
}

void ConeSolido(GLfloat base, GLfloat altura, GLint num_fatias, GLint num_tocos) {
  TroncoConeSolido(base, 0.0f, altura, num_fatias, num_tocos);
}

const Vbo RetornaTroncoConeSolido(GLfloat raio_base, GLfloat raio_topo_original, GLfloat altura, GLint num_fatias, GLint num_tocos) {
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
  ret.coordenadas.insert(ret.coordenadas.begin(), coordenadas, coordenadas + num_coordenadas_total);
  ret.normais.insert(ret.normais.begin(), normais, normais + num_coordenadas_total);
  ret.indices.insert(ret.indices.begin(), indices, indices + num_indices_total);
  return ret;
}

void TroncoConeSolido(GLfloat raio_base, GLfloat raio_topo, GLfloat altura, GLint num_fatias, GLint num_tocos) {
  const Vbo vbo(RetornaTroncoConeSolido(raio_base, raio_topo, altura, num_fatias, num_tocos));
  HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  HabilitaEstadoCliente(GL_NORMAL_ARRAY);
  PonteiroNormais(GL_FLOAT, &vbo.normais[0]);
  PonteiroVertices(3, GL_FLOAT, &vbo.coordenadas[0]);
  DesenhaElementos(GL_TRIANGLES, vbo.indices.size(), GL_UNSIGNED_SHORT, &vbo.indices[0]);
  DesabilitaEstadoCliente(GL_NORMAL_ARRAY);
  DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}

void EsferaSolida(GLfloat raio, GLint num_fatias, GLint num_tocos) {
  const Vbo vbo(RetornaEsferaSolida(raio, num_fatias, num_tocos));
  HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  HabilitaEstadoCliente(GL_NORMAL_ARRAY);
  PonteiroNormais(GL_FLOAT, &vbo.normais[0]);
  PonteiroVertices(3, GL_FLOAT, &vbo.coordenadas[0]);
  DesenhaElementos(GL_TRIANGLES, vbo.indices.size(), GL_UNSIGNED_SHORT, &vbo.indices[0]);
  DesabilitaEstadoCliente(GL_NORMAL_ARRAY);
  DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}

const Vbo RetornaEsferaSolida(GLfloat raio, GLint num_fatias, GLint num_tocos) {
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
  float normais[num_coordenadas_total];
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

  // TODO normais unitarias.
  memcpy(normais, coordenadas, sizeof(coordenadas));
  Vbo ret;
  ret.coordenadas.insert(ret.coordenadas.begin(), coordenadas, coordenadas + num_coordenadas_total);
  ret.normais.insert(ret.normais.begin(), normais, normais + num_coordenadas_total);
  ret.indices.insert(ret.indices.begin(), indices, indices + num_indices_total);
  return ret;
}


}  // namespace gl
