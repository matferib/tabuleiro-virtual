#include <algorithm>
#include <boost/filesystem.hpp>
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "arq/arquivo.h"
#include "ent/acoes.h"
#include "ent/acoes.pb.h"
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ent/tabuleiro.pb.h"
#include "ent/util.h"
#include "gltab/gl.h"
#include "log/log.h"
#include "net/util.h"  // hack to_string
#include "ntf/notificacao.pb.h"


namespace ent {

// Operacoes de picking neste modulo.
void Tabuleiro::BuscaHitMaisProximo(
    int x, int y, unsigned int* id, unsigned int* tipo_objeto, float* profundidade) {
  GLuint buffer_hits[100] = {0};
  GLuint numero_hits = 0;
  EncontraHits(x, y, &numero_hits, buffer_hits);
  // Cada hit ocupa pelo menos 4 inteiros do buffer. Na pratica, por causa da pilha vao ocupar ate mais.
  if (numero_hits > 25) {
    LOG(WARNING) << "Muitos hits para a posicao, tamanho de buffer de selecao invalido.";
    *tipo_objeto = 0;
    *id = 0;
    return;
  }

  // Busca o hit mais próximo em buffer_hits. Cada posicao do buffer (hit record):
  // - 0: pos_pilha de nomes (numero de nomes empilhados);
  // - 1: profundidade minima.
  // - 2: profundidade maxima.
  // - 3: nomes empilhados (1 para cada pos pilha).
  // Dado o hit mais proximo, retorna o identificador, a posicao da pilha e a
  // profundidade do objeto (normalizado 0..1.0).
  VLOG(2) << "numero de hits no buffer de picking: " << numero_hits;
  GLuint* ptr_hits = buffer_hits;
  // valores do hit mais proximo.
  GLuint menor_z = 0xFFFFFFFF;
  GLuint tipo_objeto_menor = 0;
  GLuint id_menor = 0;

  // Busca o hit mais proximo.
  for (GLuint i = 0; i < numero_hits; ++i) {
    GLuint pos_pilha_corrente = *ptr_hits;
    ++ptr_hits;
    if (pos_pilha_corrente != 2) {
      LOG(ERROR) << "Tamanho da pilha diferente de 2: " << pos_pilha_corrente;
      *tipo_objeto = 0;
      *id = 0;
      return;
    }
    GLuint z_corrente = *ptr_hits;
    ptr_hits += 2;  // pula maximo.
    // Tipo do objeto do hit.
    GLuint tipo_corrente = *ptr_hits;
    ++ptr_hits;
    // Id do objeto.
    GLuint id_corrente = *ptr_hits;
    ++ptr_hits;

    if (z_corrente <= menor_z) {
      VLOG(3) << "tipo_corrente: " << tipo_corrente
              << ", z_corrente: " << z_corrente
              << ", id_corrente: " << id_corrente;
      menor_z = z_corrente;
      tipo_objeto_menor = tipo_corrente;
      id_menor = id_corrente;
    } else {
      VLOG(3) << "Pulando objeto, tipo_corrente: " << tipo_corrente
              << ", z_corrente: " << z_corrente
              << ", id_corrente: " << id_corrente;
    }
  }
  *tipo_objeto = tipo_objeto_menor;
  *id = id_menor;
  float menor_profundidade = 0.0f;
  // Converte profundidade de inteiro para float.
  // No OpenGL ES a profundidade retornada vai ser sempre zero. Se nao houver hit, menor_z vai ser 0xFFFFFFFF
  // e a profundidade maxima sera retornada.
  menor_profundidade = static_cast<float>(menor_z) / static_cast<float>(0xFFFFFFFF);
  if (profundidade != nullptr) {
    *profundidade = menor_profundidade;
  }
  VLOG(1) << "Retornando menor profundidade: " << menor_profundidade
          << ", tipo_objeto: " << tipo_objeto_menor 
          << ", id: " << id_menor;
}

bool Tabuleiro::MousePara3d(int x, int y, float* x3d, float* y3d, float* z3d) {
  GLuint id;
  GLuint pos_pilha;
  float profundidade;
  BuscaHitMaisProximo(x, y, &id, &pos_pilha, &profundidade);
  if (profundidade == 1.0f) {
    return false;
  }
#if !USAR_OPENGL_ES
  return MousePara3dComProfundidade(x, y, profundidade, x3d, y3d, z3d);
#else
  return MousePara3dComId(x, y, id, pos_pilha, x3d, y3d, z3d);
#endif
}

bool Tabuleiro::MousePara3dTabuleiro(int x, int y, float* x3d, float* y3d, float* z3d) {
  // Intersecao de reta com plano z=0.
#if !USAR_OPENGL_ES
  GLdouble modelview[16], projection[16];
#else
  GLfloat modelview[16], projection[16];
#endif
  GLint viewport[4];
  gl::Le(GL_MODELVIEW_MATRIX, modelview);
  gl::Le(GL_PROJECTION_MATRIX, projection);
  gl::Le(GL_VIEWPORT, viewport);
  float p1x, p1y, p1z;
  gl::Desprojeta(x, y, -1.0f, modelview, projection, viewport, &p1x, &p1y, &p1z);
  float p2x, p2y, p2z;
  gl::Desprojeta(x, y, 1.0f, modelview, projection, viewport, &p2x, &p2y, &p2z);
  if (p2z - p1z == 0) {
    LOG(ERROR) << "Retornando lixo";
    return false;
  }
  float mult = (parametros_desenho_.offset_terreno() - p1z) / (p2z - p1z);
  *x3d = p1x + (p2x - p1x) * mult;
  *y3d = p1y + (p2y - p1y) * mult;
  *z3d = parametros_desenho_.offset_terreno();
  VLOG(2) << "Retornando tabuleiro: " << *x3d << ", " << *y3d << ", " << *z3d;
  return true;
}

#if !USAR_OPENGL_ES
bool Tabuleiro::MousePara3dComProfundidade(int x, int y, float profundidade, float* x3d, float* y3d, float* z3d) {
  GLdouble modelview[16], projection[16];
  GLint viewport[4];
  gl::Le(GL_MODELVIEW_MATRIX, modelview);
  gl::Le(GL_PROJECTION_MATRIX, projection);
  gl::Le(GL_VIEWPORT, viewport);
  if (!gl::Desprojeta(x, y, profundidade,
                      modelview, projection, viewport,
                      x3d, y3d, z3d)) {
    LOG(ERROR) << "Falha ao projetar x y no mundo 3d.";
    return false;
  }
  VLOG(2) << "Retornando: " << *x3d << " " << *y3d << " " << *z3d;
  return true;
}
#else
bool Tabuleiro::MousePara3dComId(int x, int y, unsigned int id, unsigned int pos_pilha, float* x3d, float* y3d, float* z3d) {
  // Busca mais detalhado.
  if (pos_pilha == 1) {
    MousePara3dTabuleiro(x, y, x3d, y3d, z3d);
  } else {
#if !USAR_OPENGL_ES
    GLdouble modelview[16], projection[16];
#else
    GLfloat modelview[16], projection[16];
#endif
    GLint viewport[4];
    gl::Le(GL_MODELVIEW_MATRIX, modelview);
    gl::Le(GL_PROJECTION_MATRIX, projection);
    gl::Le(GL_VIEWPORT, viewport);
    // Raio que sai do pixel.
    float p1x, p1y, p1z;
    gl::Desprojeta(x, y, -1.0f, modelview, projection, viewport, &p1x, &p1y, &p1z);
    if (camera_isometrica_) {
      *x3d = p1x;
      *y3d = p1y;
      *z3d = 0.0f;
      return true;
    }

    float p2x, p2y, p2z;
    gl::Desprojeta(x, y, 1.0f, modelview, projection, viewport, &p2x, &p2y, &p2z);
    if (p2z - p1z == 0) {
      LOG(ERROR) << "Retornando lixo";
      return false;
    }
    // Equacao parametrica do raio. Substituindo os p1* por x0, y0 e z0 e os p2* por x, y e z, temos:
    // x = x0 + at
    // y = y0 + bt
    // z = z0 + ct
    float a_raio = p2x - p1x;
    float b_raio = p2y - p1y;
    float c_raio = p2z - p1z;

    auto* e = BuscaEntidade(id);
    if (e == nullptr) {
      LOG(ERROR) << "Retornando lixo porque nao achei a entidade";
      return false;
    }
    // Cria um plano perpendicular a linha de visao para o objeto e com o plano XY.
    // Equacao do olho para o objeto. a_olho_obj * x + b_olho_obj = y.
    // Equacao da perdicular: a_perpendicular * x + b_perpendicular = y.
    //                         onde a_perpendicular = -1 / a_olho_obj.
    float a_perpendicular = (fabs(olho_.pos().x() -  e->X()) < 0.0001f) ?
        0.0f : (-1.0f / (olho_.pos().y() - e->Y()) / (olho_.pos().x() - e->X()));
    float b_perpendicular = e->Y() - e->X() * a_perpendicular;

    // Valor do t da intersecao.: onde a equacao perpendicular encontra com o plano.
    // (para simplicar nomenclatura, p = a_perpendicular, q = b_perpendicular, a = a_raio, b = b_raio).
    // (x0 = p1x, y0 = p1y, z0 = p1z).
    // y = y0 + bt = px + q;
    // t = (px + q - y0) / b. (1)
    // Como: x = x0 + at,
    // entao t = (x - x0) / a. (2)
    // Igualando (1) e (2):
    // (px + q - y0) / b = (x - x0) / a;
    // apx + aq - ay0 = bx - bx0;
    // apx - bx = ay0 - aq - bx0;
    // x (ap - b) = ay0 - aq - bx0;
    // Portanto, o x da intercessao eh (tanto faz, eh so multiplicar acima por -1 dos dois lados):
    // x = (ay0 - aq - bx0) / (ap - b) ou
    // x = (aq - ay0 + bx0) / (b - ap)
    if (fabs(b_raio - a_raio * a_perpendicular) < 0.0001f) {
      LOG(WARNING) << "Projecao praticamente perpendicular, retornando valores simples";
      *x3d = p1x;
      *y3d = p1y;
      *z3d = 0.0f;
      return true;
    }
    float x_inter = (a_raio * b_perpendicular - a_raio * p1y + b_raio * p1x) / (b_raio - a_raio * a_perpendicular);
    // Valor do t para interceptar o plano perpendicular
    float t_inter = (x_inter - p1x) / a_raio;
    // Outros valores da intersecao.
    float y_inter = p1y + b_raio * t_inter;
    float z_inter = p1z + c_raio * t_inter;

    *x3d = x_inter;
    *y3d = y_inter;
    *z3d = z_inter;
  }
  // Importante para operacoes no mesmo frame nao se confundirem.
  VLOG(2) << "Retornando: " << *x3d << " " << *y3d << " " << *z3d;
  return true;
}
#endif

}  // namespace ent
