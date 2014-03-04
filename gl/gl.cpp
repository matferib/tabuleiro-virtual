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

void CilindroSolido(GLfloat raio_base, GLfloat raio_topo, GLfloat altura, GLint fatias, GLint tocos) {
#if USAR_OPENGL_ES
#else
  GLUquadric* cilindro = gluNewQuadric();
  gluQuadricOrientation(cilindro, GLU_OUTSIDE);
  gluQuadricNormals(cilindro, GLU_SMOOTH);
  gluQuadricDrawStyle(cilindro, GLU_FILL);
  gluCylinder(cilindro, raio_base, raio_topo, altura, fatias, tocos);
  gluDeleteQuadric(cilindro);
#endif
}

#if USAR_OPENGL_ES
void Perspectiva(GLdouble angulo_y, GLdouble aspecto, GLdouble z_perto, GLdouble z_longe) {
  // TODO
}
void OlharPara(GLdouble olho_x, GLdouble olho_y, GLdouble olho_z,
               GLdouble centro_x, GLdouble centro_y, GLdouble centro_z,
               GLdouble cima_x, GLdouble cima_y, GLdouble cima_z) {
  // TODO
}
GLint Desprojeta(GLdouble x_janela, GLdouble y_janela, GLdouble profundidade_3d,
                 const GLdouble* model, const GLdouble* proj, const GLint* view,
                 GLdouble* x3d, GLdouble* y3d, GLdouble* z3d) {
  // TODO
}

#endif

}  // namespace gl
