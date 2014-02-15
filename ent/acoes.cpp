#include <algorithm>
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

void MudaCorProto(const Cor& cor) {
  const GLfloat corgl[3] = { cor.r(), cor.g(), cor.b() };
  MudaCor(corgl);
}

// Geometria deve ser do tipo GeometriaAcao. O tamanho sera unitario na unidade da geometria (ou seja, raio para esfera,
// lado para cubo).
void DesenhaGeometriaAcao(int geometria) {
  switch (geometria) {
    case ACAO_GEO_ESFERA:
      glutSolidSphere(1.0f, 10, 10);
      return;
    case ACAO_GEO_CUBO:
      glutSolidCube(1.0f);
      return;
    default:
      return;
  }
}

// Retorna o angulo de rotacao do vetor.
double VetorParaRotacaoGraus(double x, double y) {
  double tam = sqrt(x * x + y * y);
  double angulo = acos(x / tam) * RAD_PARA_GRAUS;
  return (y >= 0 ? angulo : -angulo);
}

// Ação mais básica: uma sinalizacao no tabuleiro.
class AcaoSinalizacao : public Acao {
 public:
  AcaoSinalizacao(const AcaoProto& acao_proto) : Acao(acao_proto, nullptr), estado_(TAMANHO_LADO_QUADRADO * 2.0f) {
    if (!acao_proto_.has_pos_tabuleiro()) {
      estado_ = -1.0f;
    }
  }

  void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) override {
    glPushAttrib(GL_LIGHTING_BIT);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, COR_BRANCA);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glEnable(GL_NORMALIZE);
    glNormal3f(0, 0, 1.0f);
    MudaCor(COR_BRANCA);
    glPolygonOffset(-0.08f, -0.08f);

    const Posicao& pos = acao_proto_.pos_tabuleiro();
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
    glPopAttrib();
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

// Sobe um numero verde ou vermelho de acordo com o dano causado.
// TODO: centralizar o texto
// TODO fonte maior?
class AcaoDeltaPontosVida : public Acao {
 public:
  AcaoDeltaPontosVida(const AcaoProto& acao_proto, Tabuleiro* tabuleiro) : Acao(acao_proto, tabuleiro) {
    Entidade* entidade_destino = nullptr;
    if (!acao_proto_.has_id_entidade_destino() || 
        (entidade_destino = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_destino())) == nullptr) {
      contador_atualizacoes_ = MAX_ATUALIZACOES;
      VLOG(1) << "Finalizando delta_pontos_vida precisa de entidade destino: " << acao_proto_.ShortDebugString();
      return;
    }
    pos_ = entidade_destino->PosicaoAcao();
    contador_atualizacoes_ = 0;
    // Monta a string de delta.
    int delta = abs(acao_proto_.delta_pontos_vida());
    if (delta == 0) {
      contador_atualizacoes_ = MAX_ATUALIZACOES;
      VLOG(1) << "Finalizando delta_pontos_vida, precisa de um delta.";
      return;
    }
    if (delta > 10000) {
      contador_atualizacoes_ = MAX_ATUALIZACOES;
      VLOG(1) << "Finalizando delta_pontos_vida, delta muito grande.";
      return;
    }
    while (delta != 0) {
      int d = delta % 10;
      string_delta_.insert(string_delta_.end(), static_cast<char>(d + '0'));
      delta /= 10;
    }
    std::reverse(string_delta_.begin(), string_delta_.end());
    VLOG(2) << "String delta: " << string_delta_;
  }

  void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) override {
    glPushAttrib(GL_LIGHTING_BIT);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, COR_BRANCA);
    glPushMatrix();
    MudaCor(acao_proto_.delta_pontos_vida() > 0 ? COR_VERDE : COR_VERMELHA);
    DesenhaStringTempo(string_delta_);
    glPopMatrix();
    glPopAttrib();
  }

  void Atualiza() {
    pos_.set_z(pos_.z() + 0.02f);
    ++contador_atualizacoes_;
    if (contador_atualizacoes_ == MAX_ATUALIZACOES) {
      VLOG(1) << "Finalizando delta_pontos_vida, MAX_ATUALIZACOES alcancado.";
    }
  }

  bool Finalizada() const override {
    return contador_atualizacoes_ >= MAX_ATUALIZACOES;  // 3s.
  }

 private:
  void DesenhaStringTempo(const std::string& tempo) {
    glRasterPos3f(pos_.x(), pos_.y(), pos_.z());
    for (const char c : tempo) {
      glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
    }
  }

  const int MAX_ATUALIZACOES = 100;

  std::string string_delta_;
  Posicao pos_;
  int contador_atualizacoes_;
};

// Acao de dispersao, estilo bola de fogo.
class AcaoDispersao : public Acao {
 public:
  AcaoDispersao(const AcaoProto& acao_proto) : Acao(acao_proto, nullptr) {
    raio_ = 0;
    raio_maximo_ = acao_proto_.raio_area() * TAMANHO_LADO_QUADRADO;
  }

  void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) override {
    glPushAttrib(GL_LIGHTING_BIT);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, COR_BRANCA);
    MudaCorProto(acao_proto_.cor());
    glPushMatrix();
    const Posicao& pos = acao_proto_.pos_tabuleiro();
    glTranslated(pos.x(), pos.y(), pos.z());
    glScalef(raio_, raio_, raio_);
    DesenhaGeometriaAcao(acao_proto_.geometria());
    glPopMatrix();
    glPopAttrib();
  }

  void Atualiza() {
    raio_ += 0.2;
  }

  bool Finalizada() const override {
    return raio_ > raio_maximo_;
  }

 private:
  double raio_maximo_;
  double raio_;
};

// Uma acao de projetil, tipo flecha ou missil magico.
class AcaoProjetil : public Acao {
 public:
  AcaoProjetil(const AcaoProto& acao_proto, Tabuleiro* tabuleiro) : Acao(acao_proto, tabuleiro) {
    dx_ = dy_ = dz_ = 0;
    velocidade_ = acao_proto_.velocidade().inicial();
    estagio_ = INICIAL;
    auto* entidade_origem = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    if (entidade_origem == nullptr) {
      VLOG(1) << "Finalizando projetil, precisa de entidade origem.";
      estagio_ = FIM;
      return;
    }
    if (acao_proto_.id_entidade_origem() == acao_proto_.id_entidade_destino()) {
      VLOG(1) << "Finalizando projetil, entidade origem == destino.";
      estagio_ = FIM;
      return;
    }
    pos_ = entidade_origem->PosicaoAcao();
    delta_alvo_ = 0;
    dx_total_ = dy_total_ = dz_total_ = 0;
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

  void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) override {
    if (estagio_ == ATINGIU_ALVO) {
      return;
    }
    // TODO desenha impacto.
    glPushAttrib(GL_LIGHTING_BIT);
    // Luz da camera apontando para a bola.
    const Posicao& pos_olho = pd->pos_olho();
    glLightfv(GL_LIGHT0, GL_DIFFUSE, COR_BRANCA);
    GLfloat pos_luz[] = { pos_olho.x() - pos_.x(), pos_olho.y() - pos_.y(), pos_olho.z() - pos_.z(), 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, pos_luz);

    MudaCorProto(acao_proto_.cor());
    glPushMatrix();
    glTranslated(pos_.x(), pos_.y(), pos_.z());
    // Roda pro vetor de direcao.
    glRotatef(VetorParaRotacaoGraus(dx_, dy_), 0, 0, 1.0f);
    glScalef(acao_proto_.escala().x(), acao_proto_.escala().y(), acao_proto_.escala().z());
    DesenhaGeometriaAcao(acao_proto_.has_geometria() ? acao_proto_.geometria() : ACAO_GEO_ESFERA);
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
      VLOG(1) << "Finalizando projetil, destino não existe.";
      estagio_ = FIM;
      return;
    }
    const auto& pos_destino = entidade_destino->PosicaoAcao();
    // Recalcula vetor.
    // TODO adicionar um componente erratico.
    dx_ = pos_destino.x() - pos_.x();
    dy_ = pos_destino.y() - pos_.y();
    dz_ = pos_destino.z() - pos_.z();
    double tamanho = sqrt(dx_ * dx_ + dy_ * dy_ + dz_ * dz_);
    if (tamanho == 0) {
      VLOG(1) << "Projetil atingiu alvo.";
      estagio_ = ATINGIU_ALVO;
      return;
    }
    AtualizaVelocidade();
    const double vel_tam = velocidade_ / tamanho;
    dx_ *= vel_tam;
    dy_ *= vel_tam;
    dz_ *= vel_tam;
    VLOG(4) << "vel_tam: " << vel_tam << ", vel: " << velocidade_ << ", tamanho: " << tamanho 
            << ", dx: " << dx_ << ", dy: " << dy_ << ", dz: " << dz_;

    double xa = pos_.x();
    double ya = pos_.y();
    double za = pos_.z();
    pos_.set_x(ArrumaSePassou(xa, xa + dx_, pos_destino.x()));
    pos_.set_y(ArrumaSePassou(ya, ya + dy_, pos_destino.y()));
    pos_.set_z(ArrumaSePassou(za, za + dz_, pos_destino.z()));

    if (pos_.x() == pos_destino.x() &&
        pos_.y() == pos_destino.y() &&
        pos_.z() == pos_destino.z()) {
      VLOG(1) << "Projetil atingiu alvo.";
      estagio_ = ATINGIU_ALVO;
      return;
    }
  }

  void AtualizaAlvo() {
    Entidade* entidade_destino = nullptr;
    if (!acao_proto_.has_id_entidade_destino() ||
        (entidade_destino = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_destino())) == nullptr) {
      VLOG(1) << "Finalizando projetil, destino não existe.";
      estagio_ = FIM;
      return;
    }
    // Move o alvo na direcao do impacto e volta.
    if (delta_alvo_ >= M_PI) {
      // Tira o residuo.
      entidade_destino->MoveDelta(-dx_total_, -dy_total_, -dz_total_);
      estagio_ = FIM;
      return;
    }
    double cos_delta_alvo = cosf(delta_alvo_) * TAMANHO_LADO_QUADRADO_2;
    double dx_alvo = dx_ * cos_delta_alvo;
    double dy_alvo = dy_ * cos_delta_alvo;
    double dz_alvo = dz_ * cos_delta_alvo;
    dx_total_ += dx_alvo;
    dy_total_ += dy_alvo;
    dz_total_ += dz_alvo;
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

  enum estagio_e {
    INICIAL = 0,
    ATINGIU_ALVO,
    FIM
  } estagio_;
  double delta_tempo_;
  double dx_, dy_, dz_;
  double delta_alvo_;
  // Para controle de quanto o alvo se moveu.
  double dx_total_, dy_total_, dz_total_;
  Posicao pos_;
};

}  // namespace

// Acao.
void Acao::Desenha(ParametrosDesenho* pd) {
  if (Finalizada()) {
    return;
  }
  glPushMatrix();
  DesenhaSeNaoFinalizada(pd);
  glPopMatrix();
}

void Acao::DesenhaTranslucido(ParametrosDesenho* pd) {
  if (Finalizada()) {
    return;
  }
  glPushMatrix();
  DesenhaTranslucidoSeNaoFinalizada(pd);
  glPopMatrix();
}

void Acao::AtualizaVelocidade() {
  ++delta_tempo_;
  int tipo_aceleracao = acao_proto_.velocidade().tipo_aceleracao();
  switch (tipo_aceleracao) {
    case ACAO_ACEL_ZERO:
      return;
    case ACAO_ACEL_CONSTANTE:
      velocidade_ += acao_proto_.velocidade().delta_velocidade();
      return;
    case ACAO_ACEL_QUADRATICA:
      velocidade_ += acao_proto_.velocidade().delta_velocidade() * (pow(delta_tempo_, 2) - pow(delta_tempo_ - 1, 2) );
      return;
    default:
      LOG(WARNING) << "Tipo de aceleracao invalida.";
      return;
  }
}

Acao* NovaAcao(const AcaoProto& acao_proto, Tabuleiro* tabuleiro) {
  switch (acao_proto.tipo()) {
    case ACAO_SINALIZACAO:
      return new AcaoSinalizacao(acao_proto);
    case ACAO_PROJETIL:
      return new AcaoProjetil(acao_proto, tabuleiro);
    case ACAO_DISPERSAO:
      return new AcaoDispersao(acao_proto);
    case ACAO_DELTA_PONTOS_VIDA:
      return new AcaoDeltaPontosVida(acao_proto, tabuleiro);
    default:
      LOG(ERROR) << "Acao invalida: " << acao_proto.ShortDebugString();
      return nullptr;
  }
}

}  // namespace ent
