#include <cmath>
#include "ent/constantes.h"

// Constantes que precisam de definicao.

namespace ent {

const double SEN_60 = sin(M_PI / 3.0);
const double SEN_30 = sin(M_PI / 6.0);
const double COS_60 = cos(M_PI / 3.0);
const double COS_30 = cos(M_PI / 6.0);

const GLfloat COR_BRANCA[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat COR_PRETA[]    = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat COR_VERMELHA[] = { 1.0f, 0.0f, 0.0f, 1.0f };
const GLfloat COR_VERDE[]    = { 0.0f, 1.0f, 0.0f, 1.0f };
const GLfloat COR_AZUL[]     = { 0.0f, 0.0f, 1.0f, 1.0f };
const GLfloat COR_AMARELA[]  = { 1.0f, 1.0f, 0.0f, 1.0f };

void MudaCor(const GLfloat* cor) {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cor);
  glColor3fv(cor);
}

void MudaCorAlfa(const GLfloat* cor) {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cor);
  glColor4fv(cor);
}

}  // namespace ent
