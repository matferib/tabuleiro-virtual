#ifndef GLTAB_GL_H
#define GLTAB_GL_H

#include <cmath>
#include <string>
#include <stdexcept>
#include <vector>
#if USAR_OPENGL_ES && !BENCHMARK
#if __APPLE__
  #include "TargetConditionals.h"
  #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
    // iOS device
    #include <OpenGLES/ES1/gl.h>
    #include <OpenGLES/ES1/glext.h>
  #elif TARGET_OS_MAC
    // Other kinds of Mac OS
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
  #endif
#else
#include <GLES/gl.h>
#include <GLES/glext.h>
//#include <GLES/egl.h>  Da problema com o simbolo None definido no X11/X.h, uma enum do Qt em qstyleoption.h usa None tambem.
#include <GLES/glplatform.h>
#endif
#elif __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#elif WIN32
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define RAD_PARA_GRAUS (180.0f / M_PI)
#define GRAUS_PARA_RAD (M_PI / 180.0f)

namespace gl {

// Inicializacao e finalizacao da parte grafica. Inicializacao lanca excecao std::logic_error em caso de erro.
void IniciaGl(int* argcp, char** argv);
void FinalizaGl();

class Contexto {
 public:
  Contexto(int* argcp, char** argv) { IniciaGl(argcp, argv); }
  ~Contexto() { FinalizaGl(); }
};

/** Salva a matriz corrente durante escopo da classe. Ou muda o modo de matriz e a salva, retornando ao modo anterior ao fim do escopo. */
class MatrizEscopo {
 public:
  /** Salva a matriz corrente pelo escopo. */
  MatrizEscopo() : modo_anterior_(GL_INVALID_ENUM), modo_(GL_INVALID_ENUM) { glPushMatrix(); }

  /** Muda matriz para matriz do modo e salva pelo escopo. Ao terminar, retorna para o modo anterior a chamada. */
  explicit MatrizEscopo(GLenum modo) : modo_(modo) {
    glGetIntegerv(GL_MATRIX_MODE, (GLint*)&modo_anterior_);
    glMatrixMode(modo_);
    glPushMatrix();
  }

  /** Restaura matriz anterior ao escopo para o modo escolhido. */
  ~MatrizEscopo() {
    if (modo_ != GL_INVALID_ENUM) {
      glMatrixMode(modo_);
    }
    glPopMatrix();
    if (modo_anterior_ != GL_INVALID_ENUM) {
      glMatrixMode(modo_anterior_);
    }
  }

 private:
  GLenum modo_anterior_;
  // O valor GL_INVALID_ENUM indica que nao eh para restaurar a matriz.
  GLenum modo_;
};

#if !USAR_OPENGL_ES
/* glPush/PopAttrib bits */
enum bits_atributos_e {
  BIT_POLIGONO = GL_POLYGON_BIT,
  BIT_LUZ = GL_LIGHTING_BIT,
  BIT_PROFUNDIDADE = GL_DEPTH_BUFFER_BIT,
  BIT_STENCIL = GL_STENCIL_BUFFER_BIT,
  BIT_HABILITAR = GL_ENABLE_BIT,
  BIT_COR = GL_COLOR_BUFFER_BIT,
};
/** Salva os atributos durante o escopo da variavel, restaurando no fim. */
class AtributosEscopo {
 public:
  AtributosEscopo(unsigned int mascara) { glPushAttrib(mascara); }
  ~AtributosEscopo() { glPopAttrib(); }
};
#endif

/** Funcao especial para depuracao. */
#if !USAR_OPENGL_ES
inline void InicioCena() {}
#else
void InicioCena();
#endif

/** Funcoes gerais. */
inline bool EstaHabilitado(GLenum nome_parametro) { return glIsEnabled(nome_parametro); }
inline void Le(GLenum nome_parametro, GLint* valor) { glGetIntegerv(nome_parametro, valor); }
inline void Le(GLenum nome_parametro, GLfloat* valor) { glGetFloatv(nome_parametro, valor); }
inline void Le(GLenum nome_parametro, GLboolean* valor) { glGetBooleanv(nome_parametro, valor); }
#if !USAR_OPENGL_ES
inline void Le(GLenum nome_parametro, GLdouble* valor) { glGetDoublev(nome_parametro, valor); }
#endif
void Habilita(GLenum cap);
void Desabilita(GLenum cap);
inline void HabilitaEstadoCliente(GLenum cap) { glEnableClientState(cap); }
inline void DesabilitaEstadoCliente(GLenum cap) { glDisableClientState(cap); }
inline void DesvioProfundidade(GLfloat fator, GLfloat unidades) { glPolygonOffset(fator, unidades);  }
inline void CarregaIdentidade() { glLoadIdentity(); }
inline void MultiplicaMatriz(const GLfloat* matriz) { glMultMatrixf(matriz); }
inline void ModoMatriz(GLenum modo) { glMatrixMode(modo); }
#if !USAR_OPENGL_ES
inline void EmpilhaAtributo(GLbitfield mascara) { glPushAttrib(mascara); }
inline void DesempilhaAtributo() { glPopAttrib(); }
#else
#endif
inline void FaceNula(GLenum modo) { glCullFace(modo); }
inline void FuncaoMistura(GLenum fator_s, GLenum fator_d) { glBlendFunc(fator_s, fator_d); }
inline void Viewport(GLint x, GLint y, GLsizei largura, GLsizei altura) { glViewport(x, y, largura, altura); }

// Texturas.
inline void GeraTexturas(GLsizei n, GLuint* texturas) { glGenTextures(n, texturas); }
inline void ApagaTexturas(GLsizei n, const GLuint* texturas) { glDeleteTextures(n, texturas); }
inline void LigacaoComTextura(GLenum alvo, GLuint textura) { glBindTexture(alvo, textura); }
inline void ParametroTextura(GLenum alvo, GLenum nome_param, GLint valor_param) { glTexParameteri(alvo, nome_param, valor_param); }
inline void ImagemTextura2d(
    GLenum alvo, GLint nivel, GLint formato_interno, GLsizei largura, GLsizei altura, GLint borda, 
    GLenum formato, GLenum tipo, const GLvoid* dados) {
  glTexImage2D(alvo, nivel, formato_interno, largura, altura, borda, formato, tipo, dados);
}

// Funcoes OpenGL 1.2 e acima.
#if WIN32
void GeraBuffers(GLsizei n, GLuint* buffers);
void LigacaoComBuffer(GLenum target, GLuint buffer);
void ApagaBuffers(GLsizei n, const GLuint* buffers);
void BufferizaDados(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
#else
inline void GeraBuffers(GLsizei n, GLuint* buffers) { glGenBuffers(n, buffers); }
inline void LigacaoComBuffer(GLenum target, GLuint buffer) { glBindBuffer(target, buffer); }
inline void ApagaBuffers(GLsizei n, const GLuint* buffers) { glDeleteBuffers(n, buffers); }
inline void BufferizaDados(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) { glBufferData(target, size, data, usage); }
#endif


/** Desenha elementos e afins. */
inline void DesenhaElementos(GLenum modo, GLsizei num_vertices, GLenum tipo, const GLvoid* indices) {
  glDrawElements(modo, num_vertices, tipo, indices);
}
inline void PonteiroVertices(GLint vertices_por_coordenada, GLenum tipo, const GLvoid* vertices) {
  glVertexPointer(vertices_por_coordenada, tipo, 0, vertices);
}
inline void PonteiroVertices(GLint vertices_por_coordenada, GLenum tipo, GLsizei passo, const GLvoid* vertices) {
  glVertexPointer(vertices_por_coordenada, tipo, passo, vertices);
}
inline void PonteiroVerticesTexturas(GLint vertices_por_coordenada, GLenum tipo, const GLvoid* vertices) {
  glTexCoordPointer(vertices_por_coordenada, tipo, 0, vertices);
}
inline void PonteiroVerticesTexturas(GLint vertices_por_coordenada, GLenum tipo, GLsizei passo, const GLvoid* vertices) {
  glTexCoordPointer(vertices_por_coordenada, tipo, passo, vertices);
}
inline void PonteiroNormais(GLenum tipo, const GLvoid* normais) { glNormalPointer(tipo, 0, normais);  }
inline void PonteiroNormais(GLenum tipo, GLsizei passo, const GLvoid* normais) { glNormalPointer(tipo, passo, normais);  }
inline void PonteiroCores(GLint num_componentes, GLsizei passo, const GLvoid* cores) { glColorPointer(num_componentes, GL_FLOAT, passo, cores); }
#if USAR_OPENGL_ES
void Retangulo(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
#else
inline void Retangulo(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
  glRectf(x1, y1, x2, y2);
}
#endif

/** Funcoes de nomes. */
#if !USAR_OPENGL_ES
inline void IniciaNomes() { glInitNames(); }
inline void EmpilhaNome(GLuint nome) { glPushName(nome); }
inline void CarregaNome(GLuint nome) { glLoadName(nome); }
inline void DesempilhaNome() { glPopName(); }
#else
void IniciaNomes();
void EmpilhaNome(GLuint nome);
void CarregaNome(GLuint nome);
void DesempilhaNome();
#endif

/** Configura o tipo de objeto para o escopo, retornando ao sair. */
class TipoEscopo {
 public:
  TipoEscopo(GLuint tipo, GLuint tipo_anterior = -1) {
    tipo_anterior_ = tipo_anterior;
    if (tipo_anterior != (GLuint)-1) {
      DesempilhaNome();
      DesempilhaNome();
    }
    EmpilhaNome(tipo);
    EmpilhaNome(tipo);  // no openglES vai ser util.
  }
  ~TipoEscopo() {
    DesempilhaNome();
    DesempilhaNome();
    if (tipo_anterior_ != (GLuint)-1) {
      EmpilhaNome(tipo_anterior_);
      EmpilhaNome(tipo_anterior_);
    }
  }

 private:
  GLuint tipo_anterior_;
};

/** Funcoes de escala, translacao e rotacao. */
inline void Escala(GLfloat x, GLfloat y, GLfloat z) { glScalef(x, y, z); }
inline void Translada(GLfloat x, GLfloat y, GLfloat z) { glTranslatef(x, y, z); }
inline void Roda(GLfloat angulo_graus, GLfloat x, GLfloat y, GLfloat z) { glRotatef(angulo_graus, x, y, z); }

/** Funcoes de iluminacao. */
inline void Luz(GLenum luz, GLenum nome_param, GLfloat param) { glLightf(luz, nome_param, param); }
inline void Luz(GLenum luz, GLenum nome_param, const GLfloat* params) { glLightfv(luz, nome_param, params); }
inline void ModeloLuz(GLenum nome_param, const GLfloat* params) { glLightModelfv(nome_param, params); }

/** Funcoes de nevoa. */
inline void Nevoa(GLenum param, GLfloat valor) { glFogf(param, valor); }
inline void Nevoa(GLenum param, const GLfloat* valor) { glFogfv(param, valor); }
inline void ModoNevoa(GLint modo) { glFogf(GL_FOG_MODE, modo); }

/** Funcoes de normais. */
inline void Normal(GLfloat x, GLfloat y, GLfloat z) { glNormal3f(x, y, z); }

/** Raster. */
#if !USAR_OPENGL_ES
inline void PosicaoRaster(GLfloat x, GLfloat y, GLfloat z) { glRasterPos3f(x, y, z); }
inline void PosicaoRaster(GLint x, GLint y) { glRasterPos2i(x, y); }
inline void DesenhaCaractere(char c) { glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c); }
/** Retorna o tamanho da fonte. Sempre fixo. */
inline void TamanhoFonte(int* largura, int* altura) { *largura = 8; *altura = 13; }
inline void TamanhoFonte(int largura_vp, int altura_vp, int* largura, int* altura) { *largura = 8; *altura = 13; }
#else
void PosicaoRaster(GLfloat x, GLfloat y, GLfloat z);
void PosicaoRaster(GLint x, GLint y);
void DesenhaCaractere(char c);
/** Retorna o tamanho da fonte. Variavel de acordo com viewport. Segunda versao eh um pouco mais barata. */
void TamanhoFonte(int* largura, int* altura);
void TamanhoFonte(int largura_vp, int altura_vp, int* largura, int* altura);
#endif
// Desenha a string str centralizada no ponto do raster.
// Se inverte_vertical for verdadeiro, linhas irao para cima ao inves de ir
// para baixo.
void DesenhaString(const std::string& str, bool inverte_vertical = false);
// Desenha a string str alinhada ao raster.
void DesenhaStringAlinhadoEsquerda(const std::string& str, bool inverte_vertical = false);
void DesenhaStringAlinhadoDireita(const std::string& str, bool inverte = false);

/** Matriz de olho e perspectiva e picking. */
#if !USAR_OPENGL_ES
inline void Perspectiva(GLdouble angulo_y, GLdouble aspecto, GLdouble z_perto, GLdouble z_longe) {
  gluPerspective(angulo_y, aspecto, z_perto, z_longe);
}
inline void OlharPara(GLdouble olho_x, GLdouble olho_y, GLdouble olho_z,
               GLdouble centro_x, GLdouble centro_y, GLdouble centro_z,
               GLdouble cima_x, GLdouble cima_y, GLdouble cima_z) {
  gluLookAt(olho_x, olho_y, olho_z, centro_x, centro_y, centro_z, cima_x, cima_y, cima_z);
}
inline void Ortogonal(GLdouble esquerda, GLdouble direita, GLdouble baixo, GLdouble cima, GLdouble proximo, GLdouble distante) {
  glOrtho(esquerda, direita, baixo, cima, proximo, distante);
}
inline GLint Desprojeta(GLdouble x_janela, GLdouble y_janela, GLdouble profundidade_3d,
                        const GLdouble* model, const GLdouble* proj, const GLint* view,
                        GLfloat* x3d, GLfloat* y3d, GLfloat* z3d) {
  double x3dd, y3dd, z3dd;
  GLint ret = gluUnProject(x_janela, y_janela, profundidade_3d, model, proj, view, &x3dd, &y3dd, &z3dd);
  *x3d = x3dd; *y3d = y3dd; *z3d = z3dd;
  return ret;
}
inline void MatrizPicking(GLdouble x, GLdouble y, GLdouble delta_x, GLdouble delta_y, GLint *viewport) {
  gluPickMatrix(x, y, delta_x, delta_y, viewport);
}
#else
void Perspectiva(float angulo_y, float aspecto, float z_perto, float z_longe);
void OlharPara(float olho_x, float olho_y, float olho_z,
               float centro_x, float centro_y, float centro_z,
               float cima_x, float cima_y, float cima_z);
inline void Ortogonal(float esquerda, float direita, float baixo, float cima, float proximo, float distante) {
  // glOrthof ta bugada no linux.
  //glOrthof(esquerda, direita, baixo, cima, proximo, distante);
  float tx = - ((direita + esquerda) / (direita - esquerda));
  float ty = - ((cima + baixo) / (cima - baixo));
  float tz = - ((distante + proximo) / (distante - proximo));
  GLfloat m[16];
  m[0] = 2.0f / (direita - esquerda); m[4] = 0; m[8] = 0; m[12] = tx;
  m[1] = 0; m[5] = 2.0f / (cima - baixo); m[9] = 0; m[13] = ty;
  m[2] = 0; m[6] = 0; m[10] = -2.0f / (distante - proximo); m[14] = tz;
  m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 1;
  glMultMatrixf(m);
}
GLint Desprojeta(float x_janela, float y_janela, float profundidade_3d,
                 const float* model, const float* proj, const GLint* view,
                 GLfloat* x3d, float* y3d, float* z3d);
void MatrizPicking(float x, float y, float delta_x, float delta_y, GLint *viewport);
#endif

/** Picking. */
#if !USAR_OPENGL_ES
enum modo_renderizacao_e {
  MR_RENDER = GL_RENDER,
  MR_SELECT = GL_SELECT
};
inline GLint ModoRenderizacao(modo_renderizacao_e modo) { return glRenderMode(modo); }
inline void BufferSelecao(GLsizei tam_buffer, GLuint* buffer) { glSelectBuffer(tam_buffer, buffer); }
#else
enum modo_renderizacao_e {
  MR_RENDER = 0x1C00,
  MR_SELECT = 0x1C02
};
/* Render Mode */
GLint ModoRenderizacao(modo_renderizacao_e modo);
void BufferSelecao(GLsizei tam_buffer, GLuint* buffer);
#endif

/** Mudanca de cor. */
#if !USAR_OPENGL_ES
inline void MudaCor(float r, float g, float b, float a) {
  //GLfloat cor[4] = { r, g, b, a };
  //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cor);
  glColor4f(r, g, b, a);
}
#else
void MudaCor(float r, float g, float b, float a);
#endif
inline void MascaraCor(GLboolean mascara) { glColorMask(mascara, mascara, mascara, mascara); }

inline void CorLimpeza(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { glClearColor(r, g, b, a); }
#if !USAR_OPENGL_ES
inline void Limpa(GLbitfield mascara) { glClear(mascara); }
#else
void Limpa(GLbitfield mascara);
#endif

inline void MascaraProfundidade(GLboolean valor) { glDepthMask(valor); }
class DesligaEscritaProfundidadeEscopo {
 public:
  DesligaEscritaProfundidadeEscopo() {
    // Nao funciona com glIsEnabled.
    Le(GL_DEPTH_WRITEMASK, &valor_anterior_);
    MascaraProfundidade(false);
  }
  ~DesligaEscritaProfundidadeEscopo() { MascaraProfundidade(valor_anterior_); }

 private:
  GLboolean valor_anterior_;
};

/** Habilita uma caracteristica pelo escopo. Ver glEnable. */
class HabilitaEscopo {
 public:
  HabilitaEscopo(GLenum cap) : cap_(cap) { Habilita(cap_); }
  ~HabilitaEscopo() { Desabilita(cap_); }
 private:
  GLenum cap_;
};

/** Desabilita uma caracteristica pelo escopo. Ver glDisable. */
class DesabilitaEscopo {
 public:
  DesabilitaEscopo(GLenum cap) : cap_(cap) {
    //Le(cap, &valor_anterior_);
    valor_anterior_ = EstaHabilitado(cap);
    Desabilita(cap_);
  }
  ~DesabilitaEscopo() {
    if (valor_anterior_) {
      Habilita(cap_);
    }
  }
 private:
  GLenum cap_;
  GLboolean valor_anterior_;
};

class ModeloLuzEscopo {
 public:
  ModeloLuzEscopo(const GLfloat* luz) {
      glGetFloatv(GL_LIGHT_MODEL_AMBIENT, luz_antes);
      glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luz);
  }
  ~ModeloLuzEscopo() {
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luz_antes);
  }

 private:
  GLfloat luz_antes[4];
};


/** Stencil. */
inline void FuncaoStencil(GLenum func, GLint ref, GLuint mascara) { glStencilFunc(func, ref, mascara); }

inline void OperacaoStencil(GLenum falha_stencil, GLenum falha_profundidade, GLenum sucesso) {
  glStencilOp(falha_stencil, falha_profundidade, sucesso);
}

#if USAR_SHADER
/** Uniforms. */
GLint Uniforme(const char* id);
#endif

/** debugging. */
void AlternaModoDebug();

// Namespace para utilidades internas, nem deveria estar aqui.
namespace interno {
// Quebra uma string em varias.
const std::vector<std::string> QuebraString(const std::string& entrada, char caractere_quebra);
}  // namespace internal

}  // namespace gl


#endif  // GL_GL_H
