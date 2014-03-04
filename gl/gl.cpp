#include "gl/gl.h"

namespace gl {

#if USAR_OPENGL_ES
inline void Retangulo(GLfloat x1, GLFloat y1, GLfloat x2, GLfloat y2) {
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
#endif

void ConeSolido(GLfloat base, GLfloat altura, GLint num_fatias, GLint num_tocos) {
#if USAR_OPENGL_ES
#else
  glutSolidCone(base, altura, num_fatias, num_tocos);
#endif
}

void EsferaSolida(GLfloat raio, GLint num_fatias, GLint num_tocos) {
#if USAR_OPENGL_ES
#else
  glutSolidSphere(raio, num_fatias, num_tocos);
#endif
}

void CuboSolido(GLfloat tam_lado) {
#if USAR_OPENGL_ES
#else
  glutSolidCube(tam_lado);
#endif
}

}  // namespace gl
