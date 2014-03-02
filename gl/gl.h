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
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
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

/** Salva os atributos durante o escopo da variavel, restaurando no fim. */
class AtributosEscopo {
 public:
  AtributosEscopo(GLbitfield mascara) { glPushAttrib(mascara); }
  ~AtributosEscopo() { glPopAttrib(); }
};

/** Empilha o nome no inicio do escopo e desempilha no final. */
class NomesEscopo {
 public:
  NomesEscopo(GLuint nome) { glPushName(nome); }
  ~NomesEscopo() { glPopName(); }
};

/** Funcoes gerais. */
inline void Le(GLenum nome_parametro, GLint* valor) { glGetIntegerv(nome_parametro, valor); }
inline void Le(GLenum nome_parametro, GLfloat* valor) { glGetFloatv(nome_parametro, valor); }
inline void Le(GLenum nome_parametro, GLdouble* valor) { glGetDoublev(nome_parametro, valor); }
inline void Habilita(GLenum cap) { glEnable(cap); }
inline void Desabilita(GLenum cap) { glDisable(cap); }
inline void DesvioProfundidade(GLfloat fator, GLfloat unidades) { glPolygonOffset(fator, unidades);  }
inline void CarregaIdentidade() { glLoadIdentity(); }
inline void MultiplicaMatriz(const GLfloat* matriz) { glMultMatrixf(matriz); }
inline void ModoMatriz(GLenum modo) { glMatrixMode(modo); }
inline void EmpilhaAtributo(GLbitfield mascara) { glPushAttrib(mascara); }
inline void DesempilhaAtributo() { glPopAttrib(); }

/** Funcoes de nomes. */
inline void EmpilhaNome(GLuint nome) { glPushName(nome); }
inline void CarregaNome(GLuint nome) { glLoadName(nome); }
inline void DesempilhaNome() { glPopName(); }

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

}  // namespace gl


#endif  // GL_GL_H
