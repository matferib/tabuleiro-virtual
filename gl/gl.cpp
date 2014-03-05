#include <cmath>
#include <unordered_map>
#include "gl/gl.h"
#include "log/log.h"

namespace gl {
#if !USAR_OPENGL_ES
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

#else

// OpenGL ES.
// Varias funcoes copiadas do GLUES: https://code.google.com/p/glues/.

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
  for (int i = 0; i < 4; ++i) {
    res[i] = vetor[0] * m[i] +
             vetor[1] * m[i + 4] +
             vetor[2] * m[i + 8] +
             vetor[3] * m[i + 12];
  }
  vetor[0] = res[0];
  vetor[1] = res[1];
  vetor[2] = res[2];
  vetor[3] = res[3];
}

struct ContextoInterno {
  // Mapeia um ID para a cor RGB em 22 bits (os dois mais significativos sao para a pilha).
  std::unordered_map<unsigned int, unsigned int> ids;
  unsigned int proximo_id;
  // O bit da pilha em dois bits (0-3).
  unsigned int bit_pilha;
  modo_renderizacao_e modo_renderizacao;
  GLuint* buffer_selecao;
  GLuint tam_buffer;
} g_contexto;

// Gera um proximo ID.
void MapeiaId(unsigned int id, GLubyte rgb[3]) {
  unsigned int id_mapeado = g_contexto.proximo_id | (g_contexto.bit_pilha << 22);
  g_contexto.ids.insert(std::make_pair(id_mapeado, id));
  if (g_contexto.proximo_id == ((1 << 22) - 1)) {
    LOG(ERROR) << "Limite de ids alcancado";
  } else {
    ++g_contexto.proximo_id;
  }
  rgb[0] = (id_mapeado & 0xFF);
  rgb[1] = ((id_mapeado >> 8) & 0xFF);
  rgb[2] = ((id_mapeado >> 16) & 0xFF);
}

}  // namespace

void IniciaGl(int* argcp, char** argv) {
  g_contexto.modo_renderizacao = MR_RENDER;
}

void FinalizaGl() {
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

void CuboSolido(GLfloat tam_lado) {
  unsigned short indices[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
  GLfloat tam_lado_2 = tam_lado / 2.0f;
  const float vertices_sul[] = {
    -tam_lado_2, -tam_lado_2, -tam_lado_2,
    tam_lado_2, -tam_lado_2, -tam_lado_2,
    tam_lado_2, -tam_lado_2, tam_lado_2,
    -tam_lado_2, -tam_lado_2, tam_lado_2
  };
  const float vertices_norte[] = {
    -tam_lado_2, tam_lado_2, -tam_lado_2,
    -tam_lado_2, tam_lado_2, tam_lado_2,
    tam_lado_2, tam_lado_2, tam_lado_2,
    tam_lado_2, tam_lado_2, -tam_lado_2,
  };
  const float vertices_oeste[] = {
    -tam_lado_2, -tam_lado_2, -tam_lado_2,
    -tam_lado_2, -tam_lado_2, tam_lado_2,
    -tam_lado_2, tam_lado_2, tam_lado_2,
    -tam_lado_2, tam_lado_2, -tam_lado_2,
  };
  const float vertices_leste[] = {
    tam_lado_2, -tam_lado_2, -tam_lado_2,
    tam_lado_2, tam_lado_2, -tam_lado_2,
    tam_lado_2, tam_lado_2, tam_lado_2,
    tam_lado_2, -tam_lado_2, tam_lado_2,
  };
  const float vertices_cima[] = {
    -tam_lado_2, -tam_lado_2, tam_lado_2,
    tam_lado_2, -tam_lado_2, tam_lado_2,
    tam_lado_2, tam_lado_2, tam_lado_2,
    -tam_lado_2, tam_lado_2, tam_lado_2,
  };
  const float vertices_baixo[] = {
    -tam_lado_2, -tam_lado_2, -tam_lado_2,
    -tam_lado_2, tam_lado_2, -tam_lado_2,
    tam_lado_2, tam_lado_2, -tam_lado_2,
    tam_lado_2, -tam_lado_2, -tam_lado_2,
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

void CilindroSolido(GLfloat raio_base, GLfloat raio_topo, GLfloat altura, GLint fatias, GLint tocos) {
  // TODO Conferir as normais, porque a esfera parece estar errada.
  gl::MatrizEscopo salva_matriz;
  float angulo_rotacao_graus = 360.0f / fatias;
  unsigned short indices[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
  GLfloat tam_lado_base_2 = raio_base * sinf(angulo_rotacao_graus * GRAUS_PARA_RAD / 2.0f);
  GLfloat tam_lado_topo_2 = raio_topo * sinf(angulo_rotacao_graus * GRAUS_PARA_RAD / 2.0f);
  GLfloat tam_y_base = raio_base * cosf(angulo_rotacao_graus * GRAUS_PARA_RAD / 2.0f);
  GLfloat tam_y_topo = raio_topo * cosf(angulo_rotacao_graus * GRAUS_PARA_RAD / 2.0f);
  GLfloat vetor_x[3] = { 1.0f, 0.0f, 0.0f };
  GLfloat vetor_cima[3] = { 0.0f, -(raio_base - raio_topo), altura };
  GLfloat vetor_normal[3];
  // Gera a normal FLAT.
  ProdutoVetorial(vetor_x, vetor_cima, vetor_normal);
  Normaliza(vetor_normal);
  // Gera a normal SMOOTH.
  GLfloat matriz_rotacao[16];
  GLfloat vetor_normal_oeste[3] = { vetor_normal[0], vetor_normal[1], vetor_normal[2] };
  MatrizRotacaoZ(-angulo_rotacao_graus / 2.0f, matriz_rotacao);
  MultiplicaMatrizVetor(matriz_rotacao, vetor_normal_oeste);
  GLfloat vetor_normal_leste[3] = { vetor_normal[0], vetor_normal[1], vetor_normal[2] };
  MatrizRotacaoZ(angulo_rotacao_graus / 2.0f, matriz_rotacao);
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

  gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  gl::HabilitaEstadoCliente(GL_NORMAL_ARRAY);
  for (int i = 0; i < fatias; ++i) {
    glNormal3f(vetor_normal[0], vetor_normal[1], vetor_normal[2]);
    gl::PonteiroNormais(GL_FLOAT, vertices_normais);
    gl::PonteiroVertices(3, GL_FLOAT, vertices_sul);
    gl::DesenhaElementos(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, indices);
    glRotatef(angulo_rotacao_graus, 0.0f, 0.0f, 1.0f);
  }
  gl::DesabilitaEstadoCliente(GL_NORMAL_ARRAY);
  gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
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

GLint Desprojeta(float x_janela, float y_janela, float profundidade_3d,
                 const float* model, const float* proj, const GLint* view,
                 float* x3d, float* y3d, float* z3d) {
  // TODO
  return 0;
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
  if (g_contexto.modo_renderizacao == modo) {
    return 0;
  }
  g_contexto.modo_renderizacao = modo;
  switch (modo) { 
    case MR_SELECT:
      return 0;
    case MR_RENDER: {
      glFlush();
      glFinish();
      GLubyte pixel[4] = { 0 };
      glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
      LOG(INFO) << "Pixel: " << (void*)pixel[0] << " " << (void*)pixel[1] << " " << (void*)pixel[2] << " " << (void*)pixel[3];;
      unsigned int id_mapeado = pixel[0] | (pixel[1] << 8) | (pixel[2] << 16);
      LOG(INFO) << "Id mapeado: " << (void*)id_mapeado;
      unsigned int pos_pilha = id_mapeado >> 22;
      LOG(INFO) << "Pos pilha: " << pos_pilha;
      if (pos_pilha == 0) {
        LOG(ERROR) << "Pos pilha = 0";
        return 0;
      }
      auto it = g_contexto.ids.find(id_mapeado);
      if (it == g_contexto.ids.end()) {
        LOG(ERROR) << "ERRO nao encontrei o id.";
        return 0;
      }
      unsigned int id_original = it->second;
      LOG(INFO) << "Id original: " << id_original;
      GLuint* ptr = g_contexto.buffer_selecao;
      ptr[0] = pos_pilha;
      ptr[1] = 0.0f;  // zmin.
      ptr[2] = 0.0f;  // zmax
      for (unsigned int i = 0; i < pos_pilha; ++i) {
        ptr[3 + i] = id_original;
      }
      g_contexto.buffer_selecao = nullptr;
      g_contexto.tam_buffer = 0;
      return 1;  // Numero de hits: so pode ser 0 ou 1.
    } 
    default:
      return 0;
  }
}

void BufferSelecao(GLsizei tam_buffer, GLuint* buffer) {
  g_contexto.buffer_selecao = buffer;
  g_contexto.tam_buffer = tam_buffer;
}

// Nomes
void IniciaNomes() {
  g_contexto.proximo_id = 0;
  g_contexto.bit_pilha = 0;
  g_contexto.ids.clear();
}

void EmpilhaNome(GLuint id) {
  if (g_contexto.bit_pilha == 3) {
    LOG(ERROR) << "Bit da pilha passou do limite superior.";
    return;
  }
  ++g_contexto.bit_pilha;
}

void CarregaNome(GLuint id) {
  GLubyte rgb[3];
  MapeiaId(id, rgb);
  // Muda a cor para a mapeada.
  glColor4ub(rgb[0], rgb[1], rgb[2], 1.0f);
}

void DesempilhaNome() {
  if (g_contexto.bit_pilha == 0) {
    LOG(ERROR) << "Bit da pilha passou do limite inferior.";
    return;
  }
  --g_contexto.bit_pilha;
}

void MudaCor(float r, float g, float b, float a) {
  if (g_contexto.modo_renderizacao != MR_RENDER) {
    // So muda no modo de renderizacao pra nao estragar o picking por cor.
    return;
  }
  GLfloat cor[4] = { r, g, b, a };
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cor);
  glColor4f(r, g, b, a);
}

#endif

}  // namespace gl
