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
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"
#include "log/log.h"

namespace ent {

namespace {

const double SEN_60 = sin(M_PI / 3.0);
const double SEN_30 = sin(M_PI / 6.0);
const double COS_60 = cos(M_PI / 3.0);
const double COS_30 = cos(M_PI / 6.0);

// TODO mudar para constantes.
GLfloat BRANCO[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat AZUL[] = { 0.0f, 0.0f, 1.0f, 1.0f };

/** Altera a cor correnta para cor. */
void MudaCor(GLfloat* cor) {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cor);
  glColor3fv(cor);
}

// Ação mais básica: uma sinalizacao no tabuleiro.
class AcaoSinalizacao : public Acao {
 public:
  AcaoSinalizacao(const AcaoProto& acao_proto) : Acao(acao_proto, nullptr), estado_(TAMANHO_LADO_QUADRADO * 2.0f) {}

  void Desenha(ParametrosDesenho* pd) override {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glEnable(GL_NORMALIZE);
    glNormal3f(0, 0, 1.0f);
    MudaCor(BRANCO);
    glPolygonOffset(-0.06f, -0.06f);

    const Posicao& pos = acao_proto_.pos_destino();
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

    glDisable(GL_NORMALIZE);
    glDisable(GL_POLYGON_OFFSET_FILL);
  }

  void Atualiza() {
    estado_ -= 0.05f;
  }

  bool Finalizada() const override {
    return estado_ < 0;
  }

 private:
  double estado_;
};

// Uma acao de missil magico vai da origem ate o alvo de forma meio aleatoria.
class AcaoMissilMagico : public Acao {
 public:
  AcaoMissilMagico(const AcaoProto& acao_proto, Tabuleiro* tabuleiro) : Acao(acao_proto, tabuleiro) {
    finalizada_ = false;
    auto* entidade_origem = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    if (entidade_origem == nullptr) {
      VLOG(1) << "Finalizando missil magico, precisa de entidade origem.";
      finalizada_ = true;
      return;
    }
    pos_ = entidade_origem->PosicaoAcao();
  }

  void Desenha(ParametrosDesenho* pd) override {
    if (finalizada_) {
      return;
    }

    glPushAttrib(GL_LIGHTING_BIT);
    // Luz da camera apontando para a bola.
    const Posicao& pos_olho = pd->pos_olho();
    glLightfv(GL_LIGHT0, GL_DIFFUSE, BRANCO);
    GLfloat pos_luz[] = { pos_olho.x() - pos_.x(), pos_olho.y() - pos_.y(), pos_olho.z() - pos_.z(), 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, pos_luz);

    MudaCor(AZUL);
    glPushMatrix();
    glTranslated(pos_.x(), pos_.y(), pos_.z());
    glutSolidSphere(TAMANHO_LADO_QUADRADO_2 / 4, 5, 5);
    glPopMatrix();
    glPopAttrib();
  }

  void Atualiza() override {
    if (finalizada_) {
      return;
    }
    Entidade* entidade_destino = nullptr;
    if (!acao_proto_.has_id_entidade_destino() ||
        (entidade_destino = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_destino())) == nullptr) {
      VLOG(1) << "Finalizando missil magico, destino não existe.";
      finalizada_ = true;
      return;
    }
    const auto& pos_destino = entidade_destino->PosicaoAcao();
    double dx = pos_destino.x() - pos_.x();
    double dy = pos_destino.y() - pos_.y();
    double dz = pos_destino.z() - pos_.z();
    VLOG(2) << "Acao pos: " << pos_.ShortDebugString() 
            << ", pos_destino: " << pos_destino.ShortDebugString()
            << ", dx: " << dx << ", dy: " << dy << ", dz: " << dz;
    if (fabs(dx) < TAMANHO_LADO_QUADRADO_10 &&
        fabs(dy) < TAMANHO_LADO_QUADRADO_10 &&
        fabs(dz) < TAMANHO_LADO_QUADRADO_10) {
      VLOG(1) << "Finalizando missil magico, chegou ao destino.";
      finalizada_ = true;
      return;
    }
    pos_.set_x(pos_.x() + dx / 20.0);
    pos_.set_y(pos_.y() + dy / 20.0);
    pos_.set_z(pos_.z() + dz / 20.0);
  }

  bool Finalizada() const override {
    return finalizada_;
  }

 private:
  bool finalizada_;
  double distancia_;
  Posicao pos_;
};

}  // namespace

Acao* NovaAcao(const AcaoProto& acao_proto, Tabuleiro* tabuleiro) {
  const std::string& id_acao = acao_proto.id();
  if (id_acao == ACAO_SINALIZACAO) {
    return new AcaoSinalizacao(acao_proto);
  } else if (id_acao == ACAO_MISSIL_MAGICO) {
    return new AcaoMissilMagico(acao_proto, tabuleiro);
  }
  return nullptr;
}

}  // namespace ent
