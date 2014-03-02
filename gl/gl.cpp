#include "gl/gl.h"

namespace gl {

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
