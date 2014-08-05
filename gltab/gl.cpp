#if !USAR_OPENGL_ES
#include "gltab/gl.h"

namespace gl {

void IniciaGl(int* argcp, char** argv) {
  glutInit(argcp, argv);
}
void FinalizaGl() {
}

// OpenGL normal.
void ConeSolido(GLfloat base, GLfloat altura, GLint num_fatias, GLint num_tocos) {
  glutSolidCone(base, altura, num_fatias, num_tocos);
}

void EsferaSolida(GLfloat raio, GLint num_fatias, GLint num_tocos) {
  glutSolidSphere(raio, num_fatias, num_tocos);
}

void CuboSolido(GLfloat tam_lado) {
  glutSolidCube(tam_lado);
}

namespace {

// Alinhamento pode ser < 0 esquerda, = 0 centralizado, > 0 direita.
void DesenhaStringAlinhado(const std::string& str, int alinhamento) {
  // Le o raster em coordenadas de janela.
  GLint raster_pos[4];
  glGetIntegerv(GL_CURRENT_RASTER_POSITION, raster_pos);
  // Le viewport.
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  int largura = viewport[2], altura = viewport[3];

  // Muda para projecao 2D.
  gl::MatrizEscopo salva_matriz_2(GL_PROJECTION);
  gl::CarregaIdentidade();
  gl::Ortogonal(0, largura, 0, altura, 0, 1);
  gl::MatrizEscopo salva_matriz_3(GL_MODELVIEW);
  gl::CarregaIdentidade();
  if (alinhamento < 0) {
    glRasterPos2i(raster_pos[0], raster_pos[1]);
  } else if (alinhamento == 0) {
    glRasterPos2i(raster_pos[0] - (str.size() / 2) * 8, raster_pos[1]);
  } else {
    glRasterPos2i(raster_pos[0] - (str.size() * 8), raster_pos[1]);
  }
  for (const char c : str) {
    gl::DesenhaCaractere(c);
  }
}

}  // namespace

void DesenhaString(const std::string& str) {
  DesenhaStringAlinhado(str, 0);
}

void DesenhaStringAlinhadoEsquerda(const std::string& str) {
  DesenhaStringAlinhado(str, -1);
}

void DesenhaStringAlinhadoDireita(const std::string& str) {
  DesenhaStringAlinhado(str, 1);
}

void CilindroSolido(GLfloat raio, GLfloat altura, GLint fatias, GLint tocos) {
  GLUquadric* cilindro = gluNewQuadric();
  gluQuadricOrientation(cilindro, GLU_OUTSIDE);
  gluQuadricNormals(cilindro, GLU_SMOOTH);
  gluQuadricDrawStyle(cilindro, GLU_FILL);
  gluCylinder(cilindro, raio, raio, altura, fatias, tocos);
  gluDeleteQuadric(cilindro);
}

void CilindroSolido(GLfloat raio_base, GLfloat raio_topo, GLfloat altura, GLint fatias, GLint tocos) {
  GLUquadric* cilindro = gluNewQuadric();
  gluQuadricOrientation(cilindro, GLU_OUTSIDE);
  gluQuadricNormals(cilindro, GLU_SMOOTH);
  gluQuadricDrawStyle(cilindro, GLU_FILL);
  gluCylinder(cilindro, raio_base, raio_topo, altura, fatias, tocos);
  gluDeleteQuadric(cilindro);
}

}  // namespace gl.

#else
// OpenGL ES.
// Varias funcoes copiadas do GLUES: https://code.google.com/p/glues/.

#include <cmath>
#include <unordered_map>
#include <utility>
#include <vector>
#include "gltab/gl.h"
#include "log/log.h"

namespace gl {

#define __glPi 3.14159265358979323846
namespace {

void PreencheIdentidade(GLfloat m[16]) {
  m[0+4*0] = 1; m[0+4*1] = 0; m[0+4*2] = 0; m[0+4*3] = 0;
  m[1+4*0] = 0; m[1+4*1] = 1; m[1+4*2] = 0; m[1+4*3] = 0;
  m[2+4*0] = 0; m[2+4*1] = 0; m[2+4*2] = 1; m[2+4*3] = 0;
  m[3+4*0] = 0; m[3+4*1] = 0; m[3+4*2] = 0; m[3+4*3] = 1;
}

void Normaliza(GLfloat v[3]) {
  GLfloat r = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  if (r == 0.0f) {
    return;
  }
  v[0] /= r;
  v[1] /= r;
  v[2] /= r;
}

void ProdutoVetorial(GLfloat v1[3], GLfloat v2[3], GLfloat result[3]) {
  result[0] = v1[1] * v2[2] - v1[2] * v2[1];
  result[1] = v1[2] * v2[0] - v1[0] * v2[2];
  result[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

/** Matriz de rotacao baseada no eixo X. */
void MatrizRotacaoX(GLfloat angulo, GLfloat m[16]) {
  float s = sinf(angulo * GRAUS_PARA_RAD);
  float c = cosf(angulo * GRAUS_PARA_RAD);
  m[0+4*0] = 1; m[0+4*1] = 0; m[0+4*2] = 0;  m[0+4*3] = 0;
  m[1+4*0] = 0; m[1+4*1] = c; m[1+4*2] = -s; m[1+4*3] = 0;
  m[2+4*0] = 0; m[2+4*1] = s; m[2+4*2] = c;  m[2+4*3] = 0;
  m[3+4*0] = 0; m[3+4*1] = 0; m[3+4*2] = 0;  m[3+4*3] = 1;
}

/** Matriz de rotacao baseada no eixo X. */
void MatrizRotacaoY(GLfloat angulo, GLfloat m[16]) {
  float s = sinf(angulo * GRAUS_PARA_RAD);
  float c = cosf(angulo * GRAUS_PARA_RAD);
  m[0+4*0] = c;  m[0+4*1] = 0; m[0+4*2] = 0;  m[0+4*3] = 0;
  m[1+4*0] = 0;  m[1+4*1] = 1; m[1+4*2] = -s; m[1+4*3] = 0;
  m[2+4*0] = -s; m[2+4*1] = 0; m[2+4*2] = c;  m[2+4*3] = 0;
  m[3+4*0] = 0;  m[3+4*1] = 0; m[3+4*2] = 0;  m[3+4*3] = 1;
}

/** Matriz de rotacao em Z. */
void MatrizRotacaoZ(GLfloat angulo, GLfloat m[16]) {
  float s = sinf(angulo * GRAUS_PARA_RAD);
  float c = cosf(angulo * GRAUS_PARA_RAD);
  m[0+4*0] = c; m[0+4*1] = -s; m[0+4*2] = 0; m[0+4*3] = 0;
  m[1+4*0] = s; m[1+4*1] = c;  m[1+4*2] = 0; m[1+4*3] = 0;
  m[2+4*0] = 0; m[2+4*1] = 0;  m[2+4*2] = 1; m[2+4*3] = 0;
  m[3+4*0] = 0; m[3+4*1] = 0;  m[3+4*2] = 0; m[3+4*3] = 1;
}

/* Multiplica a matriz openGL matriz pelo vetor. A matriz OpenGL tem formato col x linha (column major), portanto,
* ao inves de multiplicar matriz (4x4) pelo vetor (4x1), fazemos a inversao: vetor (1x4) pela matriz (4x4).
*/
void MultiplicaMatrizVetor(const float m[16], float vetor[4]) {
  GLfloat res[4];
  res[0] = vetor[0] * m[0] + vetor[1] * m[4] + vetor[2] * m[8]  + vetor[3] * m[12];
  res[1] = vetor[0] * m[1] + vetor[1] * m[5] + vetor[2] * m[9]  + vetor[3] * m[13];
  res[2] = vetor[0] * m[2] + vetor[1] * m[6] + vetor[2] * m[10] + vetor[3] * m[14];
  res[3] = vetor[0] * m[3] + vetor[1] * m[7] + vetor[2] * m[11] + vetor[3] * m[15];
  memcpy(vetor, res, sizeof(res));
}

/*
** Invert 4x4 matrix.
** Contributed by David Moore (See Mesa bug #6748)
*/
static int __gluInvertMatrixf(const GLfloat m[16], GLfloat invOut[16])
{
    GLfloat inv[16], det;
    int i;

    inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
             + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
    inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
             - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
    inv[8] =   m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
             + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
    inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
             - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
    inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
             - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
             + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    inv[9] =  -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
             - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    inv[13] =  m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
             + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
    inv[2] =   m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
             + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
    inv[6] =  -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
             - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
    inv[10] =  m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
             + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
    inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
             - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
    inv[3] =  -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
             - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
    inv[7] =   m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
             + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
    inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
             - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
    inv[15] =  m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
             + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

    det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
    if (det == 0)
        return GL_FALSE;

    det=1.0f/det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return GL_TRUE;
}

static void __gluMultMatrixVecf(const GLfloat matrix[16], const GLfloat in[4],
                                GLfloat out[4])
{
    int i;

    for (i=0; i<4; i++)
    {
        out[i] = in[0] * matrix[0*4+i] +
                 in[1] * matrix[1*4+i] +
                 in[2] * matrix[2*4+i] +
                 in[3] * matrix[3*4+i];
    }
}

static void __gluMultMatricesf(const GLfloat a[16], const GLfloat b[16], GLfloat r[16]) {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            r[i*4+j] = a[i*4+0]*b[0*4+j] +
                       a[i*4+1]*b[1*4+j] +
                       a[i*4+2]*b[2*4+j] +
                       a[i*4+3]*b[3*4+j];
        }
    }
}

GLint gluProject(
    GLfloat objx,
    GLfloat objy,
    GLfloat objz,
    const GLfloat modelMatrix[16],
    const GLfloat projMatrix[16],
    const GLint viewport[4],
    GLfloat* winx,
    GLfloat* winy,
    GLfloat* winz) {
  GLfloat in[4];
  GLfloat out[4];

  in[0]=objx;
  in[1]=objy;
  in[2]=objz;
  in[3]=1.0;
  __gluMultMatrixVecf(modelMatrix, in, out);
  __gluMultMatrixVecf(projMatrix, out, in);

  /*
  int max;
  gl::Le(GL_MAX_PROJECTION_STACK_DEPTH, &max);
  LOG(INFO) << "Maximo pilha PJ: " << max;
  gl::Le(GL_MAX_MODELVIEW_STACK_DEPTH, &max);
  LOG(INFO) << "Maximo pilha MV: " << max;
  for (int i = 0; i < 16; ++i) {
    LOG(INFO) << "proj[" << i << "]: " << projMatrix[i];
  }
  for (int i = 0; i < 16; ++i) {
    LOG(INFO) << "mv[" << i << "]: " << modelMatrix[i];
  }
  */
  if (in[3] == 0.0) {
    LOG(ERROR) << "Projecao falhou";
    //int max;
    //gl::Le(GL_MAX_PROJECTION_STACK_DEPTH, &max);
    //LOG(INFO) << "Maximo pilha PJ: " << max;
    //gl::Le(GL_MAX_MODELVIEW_STACK_DEPTH, &max);
    //LOG(INFO) << "Maximo pilha MV: " << max;
    return GL_FALSE;
  }
  /*
  LOG(INFO) << "Projecao ok";
  */

  in[0] /= in[3];
  in[1] /= in[3];
  in[2] /= in[3];
  /* Map x, y and z to range 0-1 */
  in[0]= in[0] * 0.5f + 0.5f;
  in[1]= in[1] * 0.5f + 0.5f;
  in[2]= in[2] * 0.5f + 0.5f;

  /* Map x,y to viewport */
  in[0] = in[0] * viewport[2] + viewport[0];
  in[1] = in[1] * viewport[3] + viewport[1];

  *winx = in[0];
  *winy = in[1];
  *winz = in[2];
  return GL_TRUE;
}
struct ContextoInterno {
  // Mapeia um ID para a cor RGB em 21 bits (os dois mais significativos sao para a pilha).
  std::unordered_map<unsigned int, unsigned int> ids;
  unsigned int proximo_id = 0;
  // O bit da pilha em tres bits (valor de [0 a 7]).
  unsigned int bit_pilha = 0;
  modo_renderizacao_e modo_renderizacao = MR_RENDER;
  bool depurar_selecao_por_cor = false;  // Mudar para true para depurar selecao por cor.
  GLuint* buffer_selecao = nullptr;
  GLuint tam_buffer = 0;
  float raster_x = 0.0f;
  float raster_y = 0.0f;
  int max_pilha_mv = 0.0f;
  int max_pilha_pj = 0.0f;

  inline bool UsarSelecaoPorCor() const {
    return depurar_selecao_por_cor || modo_renderizacao == MR_SELECT;
  }
};

ContextoInterno* g_contexto = nullptr;

// Gera um proximo ID.
void MapeiaId(unsigned int id, GLubyte rgb[3]) {
  unsigned int id_mapeado = g_contexto->proximo_id | (g_contexto->bit_pilha << 21);
  g_contexto->ids.insert(std::make_pair(id_mapeado, id));
  if (g_contexto->proximo_id == ((1 << 21) - 1)) {
    LOG(ERROR) << "Limite de ids alcancado";
  } else {
    if (g_contexto->depurar_selecao_por_cor) {
      // Mais facil de ver.
      g_contexto->proximo_id += 5;
    } else {
      ++g_contexto->proximo_id;
    }
  }
  rgb[0] = (id_mapeado & 0xFF);
  rgb[1] = ((id_mapeado >> 8) & 0xFF);
  rgb[2] = ((id_mapeado >> 16) & 0xFF);
}

// Cubo de tamanho 1.
void CuboSolidoUnitario() {
  unsigned short indices[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
  const float vertices_sul[] = {
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f
  };
  const float vertices_norte[] = {
    -0.5f, 0.5f, -0.5f,
    -0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, -0.5f,
  };
  const float vertices_oeste[] = {
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, -0.5f,
  };
  const float vertices_leste[] = {
    0.5f, -0.5f, -0.5f,
    0.5f, 0.5f, -0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,
  };
  const float vertices_cima[] = {
    -0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f,
  };
  const float vertices_baixo[] = {
    -0.5f, -0.5f, -0.5f,
    -0.5f, 0.5f, -0.5f,
    0.5f, 0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
  };

  // TODO fazer tudo num desenho so.
  gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  glNormal3f(0.0f, -1.0f, 0.0f);
  gl::PonteiroVertices(3, GL_FLOAT, vertices_sul);
  gl::DesenhaElementos(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, indices);
  glNormal3f(0.0f, 1.0f, 0.0f);
  gl::PonteiroVertices(3, GL_FLOAT, vertices_norte);
  gl::DesenhaElementos(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, indices);
  glNormal3f(-1.0f, 0.0f, 0.0f);
  gl::PonteiroVertices(3, GL_FLOAT, vertices_oeste);
  gl::DesenhaElementos(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, indices);
  glNormal3f(1.0f, 0.0f, 0.0f);
  gl::PonteiroVertices(3, GL_FLOAT, vertices_leste);
  gl::DesenhaElementos(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, indices);
  glNormal3f(0.0f, 0.0f, 1.0f);
  gl::PonteiroVertices(3, GL_FLOAT, vertices_cima);
  gl::DesenhaElementos(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, indices);
  glNormal3f(0.0f, 0.0f, -1.0f);
  gl::PonteiroVertices(3, GL_FLOAT, vertices_baixo);
  gl::DesenhaElementos(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, indices);
  gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}

}  // namespace

void IniciaGl(int* argcp, char** argv) {
  g_contexto = new ContextoInterno;
  gl::Le(GL_MAX_PROJECTION_STACK_DEPTH, &g_contexto->max_pilha_pj);
  gl::Le(GL_MAX_MODELVIEW_STACK_DEPTH, &g_contexto->max_pilha_mv);
  LOG(INFO) << "Max pilha mv: " << g_contexto->max_pilha_mv;
  LOG(INFO) << "Max pilha pj: " << g_contexto->max_pilha_pj;
}

void FinalizaGl() {
  delete g_contexto;
}

void InicioCena() {
  if (g_contexto->UsarSelecaoPorCor()) {
    g_contexto->proximo_id = 0;
    g_contexto->bit_pilha = 0;
    g_contexto->ids.clear();
  }
}

void Habilita(GLenum cap) {
  if (g_contexto->UsarSelecaoPorCor() && (cap == GL_LIGHTING)) {
    // Sem luz no modo de selecao.
    glDisable(cap);
  } else {
    glEnable(cap);
  }
}

void Retangulo(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
  const unsigned short indices[] = { 0, 1, 2, 3 };
  const float vertices[] = {
    x1, y1,
    x2, y1,
    x2, y2,
    x1, y2,
  };
  gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  gl::PonteiroVertices(2, GL_FLOAT, vertices);
  gl::DesenhaElementos(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, indices);
  gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}

void ConeSolido(GLfloat base, GLfloat altura, GLint num_fatias, GLint num_tocos) {
  TroncoConeSolido(base, 0.0f, altura, num_fatias, num_tocos);
}

void EsferaSolida(GLfloat raio, GLint num_fatias, GLint num_tocos) {
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

  HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  HabilitaEstadoCliente(GL_NORMAL_ARRAY);
  PonteiroNormais(GL_FLOAT, normais);
  PonteiroVertices(3, GL_FLOAT, coordenadas);
  DesenhaElementos(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned short), GL_UNSIGNED_SHORT, indices);
  DesabilitaEstadoCliente(GL_NORMAL_ARRAY);
  DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}

void CuboSolido(GLfloat tam_lado) {
  gl::MatrizEscopo salva_matriz;
  gl::Escala(tam_lado, tam_lado, tam_lado);
  CuboSolidoUnitario();
}

void CilindroSolido(GLfloat raio, GLfloat altura, GLint num_fatias, GLint num_tocos) {
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

  float h_delta = altura / num_tocos;
  float v_base[2];
  v_base[0] = 0;
  v_base[1] = raio;
  float h_topo = 0;

  int i_coordenadas = 0;
  int i_indices = 0;
  int coordenada_inicial = 0;

  for (int i = 1; i <= num_tocos; ++i) {
    float h_base = h_topo;
    h_topo += h_delta;
    // Novas alturas e base.
    v_base[0] = 0.0f;
    v_base[1] = raio;

    for (int i = 0; i < num_fatias; ++i) {
      // Cada faceta da esfera possui 4 vertices (anti horario). Cada vertices sera a propria normal.
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

  // TODO normais unitarias.
  memcpy(normais, coordenadas, sizeof(coordenadas));

  HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  HabilitaEstadoCliente(GL_NORMAL_ARRAY);
  PonteiroNormais(GL_FLOAT, normais);
  PonteiroVertices(3, GL_FLOAT, coordenadas);
  DesenhaElementos(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned short), GL_UNSIGNED_SHORT, indices);
  DesabilitaEstadoCliente(GL_NORMAL_ARRAY);
  DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}

void TroncoConeSolido(GLfloat raio_base, GLfloat raio_topo_original, GLfloat altura, GLint num_fatias, GLint num_tocos) {
  if (raio_base == raio_topo_original) {
    CilindroSolido(raio_base, altura, num_fatias, num_tocos);
    return;
  }
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

  HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  HabilitaEstadoCliente(GL_NORMAL_ARRAY);
  PonteiroNormais(GL_FLOAT, normais);
  PonteiroVertices(3, GL_FLOAT, coordenadas);
  DesenhaElementos(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned short), GL_UNSIGNED_SHORT, indices);
  DesabilitaEstadoCliente(GL_NORMAL_ARRAY);
  DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}

void Perspectiva(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar) {
  // Copiado do glues.
  GLfloat m[4][4];
  GLfloat sine, cotangent, deltaZ;
  GLfloat radians=(GLfloat)(fovy/2.0f*__glPi/180.0f);

  deltaZ=zFar-zNear;
  sine=(GLfloat)sinf(radians);
  if ((deltaZ==0.0f) || (sine==0.0f) || (aspect==0.0f))
  {
      return;
  }
  cotangent=(GLfloat)(cos(radians)/sine);

  PreencheIdentidade(&m[0][0]);
  m[0][0] = cotangent / aspect;
  m[1][1] = cotangent;
  m[2][2] = -(zFar + zNear) / deltaZ;
  m[2][3] = -1.0f;
  m[3][2] = -2.0f * zNear * zFar / deltaZ;
  m[3][3] = 0;
  glMultMatrixf(&m[0][0]);
}

void OlharPara(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx,
               GLfloat centery, GLfloat centerz,
               GLfloat upx, GLfloat upy, GLfloat upz) {
  GLfloat forward[3], side[3], up[3];
  GLfloat m[4][4];

  forward[0] = centerx - eyex;
  forward[1] = centery - eyey;
  forward[2] = centerz - eyez;

  up[0] = upx;
  up[1] = upy;
  up[2] = upz;

  Normaliza(forward);

  /* Side = forward x up */
  ProdutoVetorial(forward, up, side);
  Normaliza(side);

  /* Recompute up as: up = side x forward */
  ProdutoVetorial(side, forward, up);

  PreencheIdentidade(&m[0][0]);
  m[0][0] = side[0];
  m[1][0] = side[1];
  m[2][0] = side[2];

  m[0][1] = up[0];
  m[1][1] = up[1];
  m[2][1] = up[2];

  m[0][2] = -forward[0];
  m[1][2] = -forward[1];
  m[2][2] = -forward[2];

  glMultMatrixf(&m[0][0]);
  glTranslatef(-eyex, -eyey, -eyez);
}

GLint Desprojeta(GLfloat winx, GLfloat winy, GLfloat winz,
                 const GLfloat modelMatrix[16],
                 const GLfloat projMatrix[16],
                 const GLint viewport[4],
                 GLfloat* objx, GLfloat* objy, GLfloat* objz) {
  GLfloat finalMatrix[16];
  GLfloat in[4];
  GLfloat out[4];

  __gluMultMatricesf(modelMatrix, projMatrix, finalMatrix);
  if (!__gluInvertMatrixf(finalMatrix, finalMatrix))
  {
      return(GL_FALSE);
  }

  in[0]=winx;
  in[1]=winy;
  in[2]=winz;
  in[3]=1.0;

  /* Map x and y from window coordinates */
  in[0] = (in[0] - viewport[0]) / viewport[2];
  in[1] = (in[1] - viewport[1]) / viewport[3];

  /* Map to range -1 to 1 */
  in[0] = in[0] * 2 - 1;
  in[1] = in[1] * 2 - 1;
  in[2] = in[2] * 2 - 1;

  __gluMultMatrixVecf(finalMatrix, in, out);
  if (out[3] == 0.0)
  {
      return(GL_FALSE);
  }

  out[0] /= out[3];
  out[1] /= out[3];
  out[2] /= out[3];
  *objx = out[0];
  *objy = out[1];
  *objz = out[2];

  return(GL_TRUE);
}

void MatrizPicking(float x, float y, float delta_x, float delta_y, GLint *viewport) {
  if (delta_x <= 0 || delta_y <= 0) {
      return;
  }

  /* Translate and scale the picked region to the entire window */
  glTranslatef((viewport[2] - 2 * (x - viewport[0])) / delta_x,
               (viewport[3] - 2 * (y - viewport[1])) / delta_y, 0);
  glScalef(viewport[2] / delta_x, viewport[3] / delta_y, 1.0);
}

GLint ModoRenderizacao(modo_renderizacao_e modo) {
  if (g_contexto->modo_renderizacao == modo) {
    VLOG(1) << "Nao houve mudanca no modo de renderizacao";
    return 0;
  }
  g_contexto->modo_renderizacao = modo;
  switch (modo) {
    case MR_SELECT:
      return 0;
    case MR_RENDER: {
      glFlush();
      glFinish();
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      GLubyte pixel[4] = { 0 };
      glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
      int erro = glGetError();
      if (erro != 0) {
        VLOG(1) << "Erro pos glReadPixels: " << erro;
      }
      // Usar void* para imprimir em hexa.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
      VLOG(2) << "Pixel: " << (void*)pixel[0] << " " << (void*)pixel[1] << " " << (void*)pixel[2] << " " << (void*)pixel[3];
      unsigned int id_mapeado = pixel[0] | (pixel[1] << 8) | (pixel[2] << 16);
      VLOG(1) << "Id mapeado: " << (void*)id_mapeado;
      unsigned int pos_pilha = id_mapeado >> 21;
      VLOG(1) << "Pos pilha: " << pos_pilha;
      if (pos_pilha == 0 || pos_pilha > 7) {
        LOG(ERROR) << "Pos pilha invalido: " << pos_pilha;
        return 0;
      }
      auto it = g_contexto->ids.find(id_mapeado);
      if (it == g_contexto->ids.end()) {
        LOG(ERROR) << "Id nao mapeado: " << (void*)id_mapeado;
        return 0;
      }
#pragma GCC diagnostic pop
      unsigned int id_original = it->second;
      VLOG(1) << "Id original: " << id_original;
      GLuint* ptr = g_contexto->buffer_selecao;
      ptr[0] = pos_pilha;
      ptr[1] = 0;  // zmin.
      ptr[2] = 0;  // zmax
      for (unsigned int i = 0; i < pos_pilha; ++i) {
        ptr[3 + i] = id_original;
      }
      g_contexto->buffer_selecao = nullptr;
      g_contexto->tam_buffer = 0;
      return 1;  // Numero de hits: so pode ser 0 ou 1.
    }
    default:
      return 0;
  }
}

void BufferSelecao(GLsizei tam_buffer, GLuint* buffer) {
  g_contexto->buffer_selecao = buffer;
  g_contexto->tam_buffer = tam_buffer;
}

// Nomes
void IniciaNomes() {
}

void EmpilhaNome(GLuint id) {
  if (!g_contexto->UsarSelecaoPorCor()) {
    // So muda no modo de selecao.
    return;
  }
  if (g_contexto->bit_pilha == 7) {
    LOG(ERROR) << "Bit da pilha passou do limite superior.";
    return;
  }
  ++g_contexto->bit_pilha;
  VLOG(1) << "bit pilha: " << g_contexto->bit_pilha;
}

void CarregaNome(GLuint id) {
  if (!g_contexto->UsarSelecaoPorCor()) {
    // So muda no modo de selecao.
    return;
  }
  GLubyte rgb[3];
  MapeiaId(id, rgb);
  VLOG(2) << "Mapeando " << id << " para " << (int)rgb[0] << ", " << (int)rgb[1] << ", " << (int)rgb[2];
  // Muda a cor para a mapeada.
  glColor4ub(rgb[0], rgb[1], rgb[2], 255);
}

void DesempilhaNome() {
  if (!g_contexto->UsarSelecaoPorCor()) {
    // So muda no modo de selecao.
    return;
  }
  if (g_contexto->bit_pilha == 0) {
    LOG(ERROR) << "Bit da pilha passou do limite inferior.";
    return;
  }
  --g_contexto->bit_pilha;
}

void MudaCor(float r, float g, float b, float a) {
  if (g_contexto->UsarSelecaoPorCor()) {
    // So muda no modo de renderizacao pra nao estragar o picking por cor.
    return;
  }
  GLfloat cor[4] = { r, g, b, a };
  // Segundo manual do OpenGL ES, nao se pode definir o material separadamente por face.
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cor);
  glColor4f(r, g, b, a);
}

void Limpa(GLbitfield mascara) {
  if (g_contexto->UsarSelecaoPorCor()) {
    if ((mascara & GL_COLOR_BUFFER_BIT) != 0) {
      // Preto nao eh valido no color picking.
      glClearColor(0, 0, 0, 1.0f);
    }
  }
  glClear(mascara);
}

void TamanhoFonte(int largura_viewport, int altura_viewport, int* largura_fonte, int* altura) {
  unsigned int media_tela = (largura_viewport + altura_viewport) / 2;
  *largura_fonte = media_tela / 64;
  *altura = static_cast<int>(*largura_fonte * (13.0f / 8.0f));
}

void TamanhoFonte(int* largura, int* altura) {
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  TamanhoFonte(viewport[2], viewport[3], largura, altura);
}

namespace {

// Alinhamento pode ser < 0 esquerda, = 0 centralizado, > 0 direita.
void DesenhaStringAlinhado(const std::string& str, int alinhamento) {
  gl::DesabilitaEscopo profundidade_escopo(GL_DEPTH_TEST);
  gl::DesligaTesteProfundidadeEscopo mascara_escopo;
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);

  gl::MatrizEscopo salva_matriz(GL_PROJECTION);
  gl::CarregaIdentidade();
  gl::Ortogonal(0.0f, viewport[2], 0.0f, viewport[3], 0.0f, 1.0f);
  gl::MatrizEscopo salva_matriz_proj(GL_MODELVIEW);
  gl::CarregaIdentidade();

  int largura_fonte;
  int altura_fonte;
  TamanhoFonte(&largura_fonte, &altura_fonte);

  float x2d = g_contexto->raster_x;
  float y2d = g_contexto->raster_y;
  gl::Translada(x2d, y2d, 0.0f);

  //LOG(INFO) << "x2d: " << x2d << " y2d: " << y2d;
  gl::Escala(largura_fonte, altura_fonte, 1.0f);
  if (alinhamento < 0) {
  } else if (alinhamento == 0) {
    gl::Translada(-static_cast<float>(str.size()) / 2.0f, 0.0f, 0.0f);
  } else {
    gl::Translada(-static_cast<float>(str.size()), 0.0f, 0.0f);
  }
  for (const char c : str) {
    gl::DesenhaCaractere(c);
    gl::Translada(1.0f, 0.0f, 0.0f);
  }
}

}  // namespace

void DesenhaString(const std::string& str) {
  DesenhaStringAlinhado(str, 0);
}

void DesenhaStringAlinhadoEsquerda(const std::string& str) {
  DesenhaStringAlinhado(str, -1);
}

void DesenhaStringAlinhadoDireita(const std::string& str) {
  DesenhaStringAlinhado(str, 1);
}

void PosicaoRaster(GLfloat x, GLfloat y, GLfloat z) {
  float matriz_mv[16];
  float matriz_pr[16];
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  gl::Le(GL_MODELVIEW_MATRIX, matriz_mv);
  gl::Le(GL_PROJECTION_MATRIX, matriz_pr);
  float x2d, y2d, z2d;
  if (!gluProject(x, y, z, matriz_mv, matriz_pr, viewport, &x2d, &y2d, &z2d)) {
    return;
  }
  g_contexto->raster_x = x2d;
  g_contexto->raster_y = y2d;
  //LOG(INFO) << "raster_x: " << x2d << ", raster_y: " << y2d;
}

void PosicaoRaster(GLint x, GLint y) {
  PosicaoRaster(static_cast<float>(x), static_cast<float>(y), 0.0f);
}

void AlternaModoDebug() {
  g_contexto->depurar_selecao_por_cor = !g_contexto->depurar_selecao_por_cor;
}

}  // namespace gl

#endif
