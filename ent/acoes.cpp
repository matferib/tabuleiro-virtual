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
    contador_frames_ = 0;
    dx_ = dy_ = dz_ = 0;
    velocidade_ = 0;
    estagio_ = INICIAL;
    auto* entidade_origem = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    if (entidade_origem == nullptr) {
      VLOG(1) << "Finalizando missil magico, precisa de entidade origem.";
      estagio_ = FIM;
      return;
    }
    pos_ = entidade_origem->PosicaoAcao();
    delta_alvo_ = 0;
  }

  void Atualiza() override {
    if (estagio_ == INICIAL) {
      AtualizaInicial();
    } else if (estagio_ == ATINGIU_ALVO) {
      AtualizaAlvo();
    } else {
      return;
    }
  }

  void Desenha(ParametrosDesenho* pd) override {
    if (estagio_ != INICIAL) {
      // TODO desenhar impacto.
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

  bool Finalizada() const override {
    return estagio_ == FIM;
  }

 private:
  void AtualizaInicial() {
    // Atualiza destino a cada 50ms.
    Entidade* entidade_destino = nullptr;
    if (!acao_proto_.has_id_entidade_destino() ||
        (entidade_destino = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_destino())) == nullptr) {
      VLOG(1) << "Finalizando missil magico, destino não existe.";
      estagio_ = FIM;
      return;
    }
    const auto& pos_destino = entidade_destino->PosicaoAcao();
    if (--contador_frames_ <= 0) {
      // Recalcula vetor.
      contador_frames_ = 5;
      // TODO adicionar um componente erratico.
      dx_ = pos_destino.x() - pos_.x();
      dy_ = pos_destino.y() - pos_.y();
      dz_ = pos_destino.z() - pos_.z();
      double tamanho = sqrt(dx_ * dx_ + dy_ * dy_ + dz_ * dz_);
      // Normalizacao e velocidade crescendo quadraticamente (inicio lento depois acelera).
      const double vel_tam = velocidade_ * velocidade_ / tamanho;
      dx_ *= vel_tam;
      dy_ *= vel_tam;
      dz_ *= vel_tam;
      velocidade_ += 0.02f;
    }

    double xa = pos_.x();
    double ya = pos_.y();
    double za = pos_.z();
    pos_.set_x(ArrumaSePassou(xa, pos_.x() + dx_, pos_destino.x()));
    pos_.set_y(ArrumaSePassou(ya, pos_.y() + dy_, pos_destino.y()));
    pos_.set_z(ArrumaSePassou(za, pos_.z() + dz_, pos_destino.z()));

    if (pos_.x() == pos_destino.x() &&
        pos_.y() == pos_destino.y() &&
        pos_.z() == pos_destino.z()) {
      VLOG(1) << "Missil magico atingiu alvo.";
      estagio_ = ATINGIU_ALVO;
      return;
    }
  }

  void AtualizaAlvo() {
    Entidade* entidade_destino = nullptr;
    if (!acao_proto_.has_id_entidade_destino() ||
        (entidade_destino = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_destino())) == nullptr) {
      VLOG(1) << "Finalizando missil magico, destino não existe.";
      estagio_ = FIM;
      return;
    }
    // Move o alvo na direcao do impacto e volta.
    if (delta_alvo_ >= M_PI) {
      estagio_ = FIM;
    }
    double cos_delta_alvo = cosf(delta_alvo_) * TAMANHO_LADO_QUADRADO_2;
    double dx_alvo = dx_ * cos_delta_alvo;
    double dy_alvo = dy_ * cos_delta_alvo;
    double dz_alvo = dz_ * cos_delta_alvo;
    entidade_destino->MoveDelta(dx_alvo, dy_alvo, dz_alvo);
    delta_alvo_ += 0.5; 
  }

  // Verifica se a coordenada passou do ponto de destino.
  static bool Passou(double antes, double depois, double destino) {
    return (antes < destino) ? depois > destino : depois < destino;
  }

  // Retorna depois se a coordenada nao passou de destino, caso contrario retorna destino.
  static double ArrumaSePassou(double antes, double depois, double destino) {
    return Passou(antes, depois, destino) ? destino : depois;
  }

  int contador_frames_;
  enum estagio_e {
    INICIAL = 0,
    ATINGIU_ALVO,
    FIM
  } estagio_;
  double velocidade_;
  double dx_, dy_, dz_;
  double delta_alvo_;
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
