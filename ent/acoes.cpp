#if __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include "ent/acoes.h"
#include "ent/constantes.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

namespace ent {

namespace {

const double SEN_60 = sin(M_PI / 3.0);
const double SEN_30 = sin(M_PI / 6.0);
const double COS_60 = cos(M_PI / 3.0);
const double COS_30 = cos(M_PI / 6.0);

// TODO mudar para constantes.
GLfloat BRANCO[] = { 1.0f, 1.0f, 1.0f, 1.0f };

/** Altera a cor correnta para cor. */
void MudaCor(GLfloat* cor) {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cor);
  glColor3fv(cor);
}

class AcaoSinalizacao : public Acao {
 public:
  AcaoSinalizacao(const AcaoProto& acao_proto) : Acao(acao_proto), estado_(TAMANHO_LADO_QUADRADO * 2.0f) {}

  void Desenha(ParametrosDesenho* pd) override {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glEnable(GL_NORMALIZE);
    glNormal3f(0, 0, 1.0f);
    MudaCor(BRANCO);
    glPolygonOffset(-0.06f, -0.06f);

    const Posicao& pos = acao_proto_.pos();
    glPushMatrix();
    glTranslated(pos.x(), pos.y(), pos.z());
    glScaled(estado_, estado_, 0.0f);
    glBegin(GL_TRIANGLES);
    // Primeiro triangulo.
    glVertex2d(COS_30 * 0.3, SEN_30 * 0.2);
    glVertex2i(1, 0);
    glVertex2d(COS_60, SEN_60);
    // Segundo triangulo.
    glVertex2d(-COS_30 * 0.3, SEN_30 * 0.2);
    glVertex2d(-COS_60, SEN_60);
    glVertex2i(-1, 0);
    // Terceiro triangulo.
    glVertex2d(0.0, -0.2);
    glVertex2d(-COS_60, -SEN_60);
    glVertex2d(COS_60, -SEN_60);
    glEnd();
    glPopMatrix();
    estado_ -= 0.05f;

    glDisable(GL_NORMALIZE);
    glDisable(GL_POLYGON_OFFSET_FILL);
  }

  bool Finalizada() const override {
    return estado_ < 0;
  }

 private:
  double estado_;
};

}  // namespace

Acao* NovaAcao(const AcaoProto& acao_proto) {
  const std::string& id_acao = acao_proto.id();
  if (id_acao == ACAO_SINALIZACAO) {
    return new AcaoSinalizacao(acao_proto);
  }
  return nullptr;
}

}  // namespace ent
