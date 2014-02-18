#include <cmath>
#if __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include "ent/constantes.h"
#include "ent/util.h"

namespace ent {

void MudaCor(const float* cor) {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cor);
  glColor3fv(cor);
}

void MudaCorAlfa(const float* cor) {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cor);
  glColor4fv(cor);
}

float VetorParaRotacaoGraus(float x, float y, float* tamanho) {
  float tam = sqrt(x * x + y * y);
  float angulo = acosf(x / tam) * RAD_PARA_GRAUS;
  if (tamanho != nullptr) {
    *tamanho = tam;
  }
  return (y >= 0 ? angulo : -angulo);
}

void DesenhaDisco(GLfloat raio, int num_faces) {
  glNormal3f(0, 0, 1.0f);
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(0.0, 0.0, 0.0);
  for (int i = 0; i <= num_faces; ++i) {
    float angulo = i * 2 * M_PI / num_faces;
    glVertex3f(cosf(angulo) * raio, sinf(angulo) * raio, 0.0f);
  }
  glEnd();
}

}  // namespace ent
