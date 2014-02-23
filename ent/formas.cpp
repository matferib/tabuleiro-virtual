#include <cmath>
#if __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif
#include "ent/constantes.h"
#include "ent/formas.h"
#include "ent/formas.pb.h"
#include "ent/util.h"
#include "log/log.h"

namespace ent {

Forma::Forma(const FormaProto& proto) : proto_(proto) {}
Forma::~Forma() {}

void Forma::Desenha(const ParametrosDesenho& pd) {
  glPushAttrib(GL_ENABLE_BIT);
  if (proto_.cor().a() < 1.0f) {
    glEnable(GL_BLEND);
  }
  glEnable(GL_NORMALIZE);
  glDisable(GL_CULL_FACE);
  MudaCor(proto_.cor());
  switch (proto_.tipo()) {
    case TF_RETANGULO: {
      glNormal3i(0, 0, 1);
      glRectf(proto_.inicio().x(), proto_.inicio().y(), proto_.fim().x(), proto_.fim().y());
    }
    break;
    case TF_ESFERA: {
      // Usar x como base para achatamento.
      float centro_x = (proto_.inicio().x() + proto_.fim().x()) / 2.0f;
      float centro_y = (proto_.inicio().y() + proto_.fim().y()) / 2.0f;
      float escala_x = fabs(proto_.inicio().x() - proto_.fim().x());
      float escala_y = fabs(proto_.inicio().y() - proto_.fim().y());
      glPushMatrix();
      glTranslatef(centro_x, centro_y, 0.0f);
      glScalef(escala_x, escala_y, std::min(escala_x, escala_y));
      glutSolidSphere(0.5f  /*raio*/, 20  /*ao redor*/, 20 /*vertical*/);
      glPopMatrix();
    }
    break;
    case TF_CIRCULO: {
      glNormal3i(0, 0, 1);
      // Usar x como base para achatamento.
      float centro_x = (proto_.inicio().x() + proto_.fim().x()) / 2.0f;
      float centro_y = (proto_.inicio().y() + proto_.fim().y()) / 2.0f;
      float escala_x = fabs(proto_.inicio().x() - proto_.fim().x());
      float escala_y = fabs(proto_.inicio().y() - proto_.fim().y());
      glPushMatrix();
      glTranslatef(centro_x, centro_y, 0.0f);
      glScalef(escala_x, escala_y, 1.0f);
      DesenhaDisco(0.5f, 12);
      glPopMatrix();
    }
    break;
    case TF_CUBO: {
      // Usar x como base para achatamento.
      float centro_x = (proto_.inicio().x() + proto_.fim().x()) / 2.0f;
      float centro_y = (proto_.inicio().y() + proto_.fim().y()) / 2.0f;
      float escala_x = fabs(proto_.inicio().x() - proto_.fim().x());
      float escala_y = fabs(proto_.inicio().y() - proto_.fim().y());
      glPushMatrix();
      glTranslatef(centro_x, centro_y, TAMANHO_LADO_QUADRADO_2);
      // Altura do cubo do lado do quadrado.
      glScalef(escala_x, escala_y, TAMANHO_LADO_QUADRADO);
      glutSolidCube(1.0f);
      glPopMatrix();
    }
    break;
    case TF_LIVRE: {
      glNormal3i(0, 0, 1);
      bool usa_stencil = pd.transparencias() && proto_.cor().a() < 1.0f;
      if (usa_stencil) {
        LigaStencil();
      }
      DesenhaLinha3d(proto_.ponto(), TAMANHO_LADO_QUADRADO / 2.0f);
      if (usa_stencil) {
        DesenhaStencil(proto_.cor());
      }
    }
    break;
    default:
      LOG(ERROR) << "Forma de desenho invalida";
  }
  glPopAttrib();
}

}  // namespace ent
