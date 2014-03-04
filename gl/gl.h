#ifndef GL_GL_H
#define GL_GL_H

#if __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#elif WIN32
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>
#elif USAR_OPENGL_ES
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES/egl.h>
#include <GLES/glplatform.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#if USAR_OPENGL_ES
// OpenGL ES nao tem double.
typedef GLfloat GLdouble;
#endif

namespace gl {

/** Salva a matriz corrente durante escopo da classe. Ou muda o modo de matriz e a salva, retornando ao modo anterior ao fim do escopo. */
class MatrizEscopo {
 public:
  /** Salva a matriz corrente pelo escopo. */
  MatrizEscopo() : modo_anterior_(GL_INVALID_ENUM), modo_(GL_INVALID_ENUM) { glPushMatrix(); }

  /** Muda matriz para matriz do modo e a salva pelo escopo. */
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

#if USAR_OPENGL_ES
/* glPush/PopAttrib bits */
#define GL_CURRENT_BIT				0x00000001
#define GL_POINT_BIT				0x00000002
#define GL_LINE_BIT				0x00000004
#define GL_POLYGON_BIT				0x00000008
#define GL_POLYGON_STIPPLE_BIT			0x00000010
#define GL_PIXEL_MODE_BIT			0x00000020
#define GL_LIGHTING_BIT				0x00000040
#define GL_FOG_BIT				0x00000080
#define GL_DEPTH_BUFFER_BIT			0x00000100
#define GL_ACCUM_BUFFER_BIT			0x00000200
#define GL_STENCIL_BUFFER_BIT			0x00000400
#define GL_VIEWPORT_BIT				0x00000800
#define GL_TRANSFORM_BIT			0x00001000
#define GL_ENABLE_BIT				0x00002000
#define GL_COLOR_BUFFER_BIT			0x00004000
#define GL_HINT_BIT				0x00008000
#define GL_EVAL_BIT				0x00010000
#define GL_LIST_BIT				0x00020000
#define GL_TEXTURE_BIT				0x00040000
#define GL_SCISSOR_BIT				0x00080000
#define GL_ALL_ATTRIB_BITS			0x000FFFFF
#endif
/** Salva os atributos durante o escopo da variavel, restaurando no fim. */
class AtributosEscopo {
 public:
#if !USAR_OPENGL_ES
  AtributosEscopo(GLbitfield mascara) { glPushAttrib(mascara); }
  ~AtributosEscopo() { glPopAttrib(); }
#else
  AtributosEscopo(GLbitfield mascara) { /* TODO */ }
  ~AtributosEscopo() { /* TODO */ }
#endif
};

/** Empilha o nome no inicio do escopo e desempilha no final. */
class NomesEscopo {
 public:
#if !USAR_OPENGL_ES
  NomesEscopo(GLuint nome) { glPushName(nome); }
  ~NomesEscopo() { glPopName(); }
#else
  NomesEscopo(GLuint nome) { /* TODO */ }
  ~NomesEscopo() { /* TODO */ }
#endif
};

/** Funcoes gerais. */
inline void Le(GLenum nome_parametro, GLint* valor) { glGetIntegerv(nome_parametro, valor); }
inline void Le(GLenum nome_parametro, GLfloat* valor) { glGetFloatv(nome_parametro, valor); }
#if !USAR_OPENGL_ES
inline void Le(GLenum nome_parametro, GLdouble* valor) { glGetDoublev(nome_parametro, valor); }
#endif
inline void Habilita(GLenum cap) { glEnable(cap); }
inline void Desabilita(GLenum cap) { glDisable(cap); }
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
inline void EmpilhaAtributo(GLbitfield mascara) { /* TODO */ }
inline void DesempilhaAtributo() { /* TODO */ }
#endif

/** Desenha elementos e afins. */
inline void DesenhaElementos(GLenum modo, GLsizei num_vertices, GLenum tipo, const GLvoid* indices) {
  glDrawElements(modo, num_vertices, tipo, indices);
}
inline void PonteiroVertices(GLint vertices_por_coordenada, GLenum tipo, const GLvoid* vertices) {
  glVertexPointer(vertices_por_coordenada, tipo, 0, vertices);
}
inline void PonteiroVerticesTexturas(GLint vertices_por_coordenada, GLenum tipo, const GLvoid* vertices) {
  glTexCoordPointer(vertices_por_coordenada, tipo, 0, vertices);
}
inline void PonteiroNormais(GLenum tipo, const GLvoid* normais) { glNormalPointer(tipo, 0, normais);  }
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
inline void IniciaNomes() { /* TODO */ }
inline void EmpilhaNome(GLuint nome) { /* TODO */ }
inline void CarregaNome(GLuint nome) { /* TODO */ }
inline void DesempilhaNome() { /* TODO */ }
#endif

/** Funcoes de escala, translacao e rotacao. */
inline void Escala(GLfloat x, GLfloat y, GLfloat z) { glScalef(x, y, z); }
inline void Translada(GLfloat x, GLfloat y, GLfloat z) { glTranslatef(x, y, z); }
inline void Roda(GLfloat angulo_graus, GLfloat x, GLfloat y, GLfloat z) { glRotatef(angulo_graus, x, y, z); }

/** Funcoes de iluminacao. */
inline void Luz(GLenum luz, GLenum nome_param, GLfloat param) { glLightf(luz, nome_param, param); }
inline void Luz(GLenum luz, GLenum nome_param, const GLfloat* params) { glLightfv(luz, nome_param, params); }
inline void ModeloLuz(GLenum nome_param, const GLfloat* params) { glLightModelfv(nome_param, params); }

/** Funcoes de normais. */
inline void Normal(GLfloat x, GLfloat y, GLfloat z) { glNormal3f(x, y, z); }

/** Objetos GLU e GLUT. */
void CilindroSolido(GLfloat raio_base, GLfloat raio_topo, GLfloat altura, GLint fatias, GLint tocos);
void ConeSolido(GLfloat base, GLfloat altura, GLint num_fatias, GLint num_tocos);
void EsferaSolida(GLfloat raio, GLint num_fatias, GLint num_tocos);
void CuboSolido(GLfloat tam_lado);

/** Raster. */
#if !USAR_OPENGL_ES
inline void PosicaoRaster(GLfloat x, GLfloat y, GLfloat z) { glRasterPos3f(x, y, z); }
inline void PosicaoRaster(GLint x, GLint y) { glRasterPos2i(x, y); }
inline void DesenhaCaractere(char c) { glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c); }
#else
inline void PosicaoRaster(GLfloat x, GLfloat y, GLfloat z) { /* TODO */ }
inline void PosicaoRaster(GLint x, GLint y) { /* TODO */ }
inline void DesenhaCaractere(char c) { /* TODO */ }
#endif

/** Matriz de olho e perspectiva. */
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
                        GLdouble* x3d, GLdouble* y3d, GLdouble* z3d) {
  return gluUnProject(x_janela, y_janela, profundidade_3d, model, proj, view, x3d, y3d, z3d);
}
#else
void Perspectiva(GLdouble angulo_y, GLdouble aspecto, GLdouble z_perto, GLdouble z_longe);
void OlharPara(GLdouble olho_x, GLdouble olho_y, GLdouble olho_z,
               GLdouble centro_x, GLdouble centro_y, GLdouble centro_z,
               GLdouble cima_x, GLdouble cima_y, GLdouble cima_z);
inline void Ortogonal(GLdouble esquerda, GLdouble direita, GLdouble baixo, GLdouble cima, GLdouble proximo, GLdouble distante) {
  glOrthof(esquerda, direita, baixo, cima, proximo, distante);
}
GLint Desprojeta(GLdouble x_janela, GLdouble y_janela, GLdouble profundidade_3d,
                 const GLdouble* model, const GLdouble* proj, const GLint* view,
                 GLdouble* x3d, GLdouble* y3d, GLdouble* z3d);
#endif

}  // namespace gl


#endif  // GL_GL_H
