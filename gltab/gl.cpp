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

static void __gluMultMatricesf(const GLfloat a[16], const GLfloat b[16],
                               GLfloat r[16])
{
    int i, j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            r[i*4+j] = a[i*4+0]*b[0*4+j] +
                       a[i*4+1]*b[1*4+j] +
                       a[i*4+2]*b[2*4+j] +
                       a[i*4+3]*b[3*4+j];
        }
    }
}

struct ContextoInterno {
  // Mapeia um ID para a cor RGB em 22 bits (os dois mais significativos sao para a pilha).
  std::unordered_map<unsigned int, unsigned int> ids;
  unsigned int proximo_id = 0;
  // O bit da pilha em dois bits (valor de 0 a 3).
  unsigned int bit_pilha = 0;
  modo_renderizacao_e modo_renderizacao = MR_RENDER;
  GLuint* buffer_selecao = nullptr;
  GLuint tam_buffer = 0;
};

ContextoInterno* g_contexto = nullptr;

// Gera um proximo ID.
void MapeiaId(unsigned int id, GLubyte rgb[3]) {
  unsigned int id_mapeado = g_contexto->proximo_id | (g_contexto->bit_pilha << 22);
  g_contexto->ids.insert(std::make_pair(id_mapeado, id));
  if (g_contexto->proximo_id == ((1 << 22) - 1)) {
    LOG(ERROR) << "Limite de ids alcancado";
  } else {
    ++g_contexto->proximo_id;
  }
  rgb[0] = (id_mapeado & 0xFF);
  rgb[1] = ((id_mapeado >> 8) & 0xFF);
  rgb[2] = ((id_mapeado >> 16) & 0xFF);
}

}  // namespace

void IniciaGl(int* argcp, char** argv) {
  g_contexto = new ContextoInterno;
}

void FinalizaGl() {
  delete g_contexto;
}

void Retangulo(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
  const unsigned short indices[] = { 0, 1, 2, 3, 4, 5, 6, 7, };
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
  CilindroSolido(base, 0.0f, altura, num_fatias, num_tocos);
}

void EsferaSolida(GLfloat raio, GLint num_fatias, GLint num_tocos) {
  // Desenhar a esfera baseada em cilindros.
  float angulo_rad = (90.0f * GRAUS_PARA_RAD) / num_tocos;
  GLfloat raio_base = raio;
  GLfloat raio_topo;

  for (int i = 0; i < num_tocos; ++i) {
    raio_topo = raio * cosf(angulo_rad * (i + 1));
    GLfloat h_base = raio * sinf(angulo_rad * i);
    GLfloat h_topo = raio * sinf(angulo_rad * (i + 1));
    GLfloat h_delta = h_topo - h_base;
    // Desenha cilindro de cima e de baixo.
    {
      MatrizEscopo salva_matriz;
      glTranslatef(0.0f, 0.0f, h_base);
      CilindroSolido(raio_base, raio_topo, h_delta, num_fatias, 1);
    }
    {
      MatrizEscopo salva_matriz;
      glTranslatef(0.0f, 0.0f, -h_topo);
      CilindroSolido(raio_topo, raio_base, h_delta, num_fatias, 1);
    }
    raio_base = raio_topo;
  }
}

namespace {

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

void CuboSolido(GLfloat tam_lado) {
  gl::MatrizEscopo salva_matriz;
  gl::Escala(tam_lado, tam_lado, tam_lado);
  CuboSolidoUnitario();
}

void CilindroSolido(GLfloat raio_base, GLfloat raio_topo, GLfloat altura, GLint fatias, GLint tocos) {
#if 0
  gl::MatrizEscopo salva_matriz;
  // Matriz eh column major, ou seja, esta invertida.
  GLfloat matriz_transformacao[] = {
    0.0f,                    0.0f, 0.0f, 1.0f,
    0.0f,                    1.0f, 0.0f, 0.0f,
    (raio_topo / raio_base), 0.0f, altura, 0.0f,
    1.0f,                    0.0f, 0.0f, 1.0f,
  };
  gl::MultiplicaMatriz(matriz_transformacao);

  float angulo_rotacao_graus = 360.0f / fatias;
  unsigned short indices[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
  GLfloat tam_lado_base_2 = sinf(angulo_rotacao_graus * GRAUS_PARA_RAD / 2.0f);
  GLfloat tam_lado_topo_2 = sinf(angulo_rotacao_graus * GRAUS_PARA_RAD / 2.0f);
  GLfloat tam_y_base = cosf(angulo_rotacao_graus * GRAUS_PARA_RAD / 2.0f);
  GLfloat tam_y_topo = cosf(angulo_rotacao_graus * GRAUS_PARA_RAD / 2.0f);
  GLfloat vetor_x[3] = { 1.0f, 0.0f, 0.0f };
  GLfloat vetor_cima[3] = { 0.0f, 0.0f, 1.0f};
  GLfloat vetor_normal[3] = { 0.0f, -1.0f, 0.0f };
  const float vertices_sul[] = {
    -tam_lado_base_2, -tam_y_base, 0.0f,
    tam_lado_base_2, -tam_y_base, 0.0f,
    tam_lado_topo_2, -tam_y_topo, altura,
    -tam_lado_topo_2, -tam_y_topo, altura
  };

  HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  //HabilitaEstadoCliente(GL_NORMAL_ARRAY);
  for (int i = 0; i < fatias; ++i) {
    Normal(vetor_normal[0], vetor_normal[1], vetor_normal[2]);
    //PonteiroNormais(GL_FLOAT, vertices_normais);
    PonteiroVertices(3, GL_FLOAT, vertices_sul);
    DesenhaElementos(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, indices);
    Roda(angulo_rotacao_graus, 0.0f, 0.0f, 1.0f);
  }
  //DesabilitaEstadoCliente(GL_NORMAL_ARRAY);
  DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
#else
  // Versao antiga.
  gl::MatrizEscopo salva_matriz;
  float angulo_rotacao_graus = 360.0f / fatias;
  float angulo_rotacao_graus_2 = angulo_rotacao_graus / 2.0f;
  unsigned short indices[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
  float seno_angulo_rotacao_2 = sinf(angulo_rotacao_graus_2 * GRAUS_PARA_RAD);
  GLfloat tam_lado_base_2 = raio_base * seno_angulo_rotacao_2;
  GLfloat tam_lado_topo_2 = raio_topo * seno_angulo_rotacao_2;
  float cos_angulo_rotacao_2 = cosf(angulo_rotacao_graus_2 * GRAUS_PARA_RAD);
  GLfloat tam_y_base = raio_base * cos_angulo_rotacao_2;
  GLfloat tam_y_topo = raio_topo * cos_angulo_rotacao_2;
  GLfloat vetor_x[3] = { 1.0f, 0.0f, 0.0f };
  GLfloat vetor_cima[3] = { 0.0f, raio_base - raio_topo, altura };
  GLfloat vetor_normal[3];
  // Gera a normal FLAT.
  ProdutoVetorial(vetor_x, vetor_cima, vetor_normal);
  Normaliza(vetor_normal);
  // Gera a normal SMOOTH.
  GLfloat matriz_rotacao[16];
  GLfloat vetor_normal_oeste[3] = { vetor_normal[0], vetor_normal[1], vetor_normal[2] };
  MatrizRotacaoZ(-angulo_rotacao_graus_2, matriz_rotacao);
  MultiplicaMatrizVetor(matriz_rotacao, vetor_normal_oeste);
  GLfloat vetor_normal_leste[3] = { vetor_normal[0], vetor_normal[1], vetor_normal[2] };
  MatrizRotacaoZ(angulo_rotacao_graus_2, matriz_rotacao);
  MultiplicaMatrizVetor(matriz_rotacao, vetor_normal_leste);
  const float vertices_normais[12] = {
    vetor_normal_oeste[0], vetor_normal_oeste[1], vetor_normal_oeste[2],
    vetor_normal_leste[0], vetor_normal_leste[1], vetor_normal_leste[2],
    vetor_normal_leste[0], vetor_normal_leste[1], vetor_normal_leste[2],
    vetor_normal_oeste[0], vetor_normal_oeste[1], vetor_normal_oeste[2],
  };
  const float vertices_sul[] = {
    -tam_lado_base_2, -tam_y_base, 0.0f,
    tam_lado_base_2, -tam_y_base, 0.0f,
    tam_lado_topo_2, -tam_y_topo, altura,
    -tam_lado_topo_2, -tam_y_topo, altura
  };

  HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  HabilitaEstadoCliente(GL_NORMAL_ARRAY);
  for (int i = 0; i < fatias; ++i) {
    Normal(vetor_normal[0], vetor_normal[1], vetor_normal[2]);
    PonteiroNormais(GL_FLOAT, vertices_normais);
    PonteiroVertices(3, GL_FLOAT, vertices_sul);
    DesenhaElementos(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, indices);
    Roda(angulo_rotacao_graus, 0.0f, 0.0f, 1.0f);
  }
  DesabilitaEstadoCliente(GL_NORMAL_ARRAY);
  DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
#endif
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
      g_contexto->proximo_id = 0;
      g_contexto->bit_pilha = 0;
      g_contexto->ids.clear();
      return 0;
    case MR_RENDER: {
      //glFlush();
      //glFinish();
      GLubyte pixel[4] = { 0 };
      glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
      VLOG(2) << "Erro pos glReadPixels: " << glGetError();
      // Usar void* para imprimir em hexa.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
      VLOG(2) << "Pixel: " << (void*)pixel[0] << " " << (void*)pixel[1] << " " << (void*)pixel[2] << " " << (void*)pixel[3];;
      unsigned int id_mapeado = pixel[0] | (pixel[1] << 8) | (pixel[2] << 16);
      VLOG(2) << "Id mapeado: " << (void*)id_mapeado;
      unsigned int pos_pilha = id_mapeado >> 22;
      VLOG(2) << "Pos pilha: " << pos_pilha;
      if (pos_pilha == 0 || pos_pilha > 3) {
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
      VLOG(2) << "Id original: " << id_original;
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
  if (g_contexto->modo_renderizacao != MR_SELECT) {
    // So muda no modo de selecao.
    return;
  }
  if (g_contexto->bit_pilha == 3) {
    LOG(ERROR) << "Bit da pilha passou do limite superior.";
    return;
  }
  ++g_contexto->bit_pilha;
}

void CarregaNome(GLuint id) {
  if (g_contexto->modo_renderizacao != MR_SELECT) {
    // So muda no modo de selecao.
    return;
  }
  GLubyte rgb[3];
  MapeiaId(id, rgb);
  // Muda a cor para a mapeada.
  glColor4ub(rgb[0], rgb[1], rgb[2], 1.0f);
}

void DesempilhaNome() {
  if (g_contexto->modo_renderizacao != MR_SELECT) {
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
  if (g_contexto->modo_renderizacao != MR_RENDER) {
    // So muda no modo de renderizacao pra nao estragar o picking por cor.
    return;
  }
  GLfloat cor[4] = { r, g, b, a };
  // Segundo manual do OpenGL ES, nao se pode definir o material separadamente por face.
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cor);
  glColor4f(r, g, b, a);
}

void Limpa(GLbitfield mascara) {
  if (g_contexto->modo_renderizacao == MR_SELECT) {
    if ((mascara & GL_COLOR_BUFFER_BIT) != 0) {
      // Preto nao eh valido no color picking.
      glClearColor(0, 0, 0, 1.0f);
    }
  }
  glClear(mascara);
}

}  // namespace gl

#endif
