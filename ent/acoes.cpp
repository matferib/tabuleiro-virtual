#include <algorithm>
#include <cmath>

//#define VLOG_NIVEL 2
#include "ent/acoes.h"
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ent/util.h"
#include "gltab/gl.h"
#include "gltab/gl_vbo.h"
#include "matrix/vectors.h"
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
    case ACAO_GEO_CUBO:
      gl::CuboSolido(1.0f);
      return;
    case ACAO_GEO_CONE:
      gl::ConeSolido(0.5f  /*raio*/, 1.0f  /*altura*/, 10  /*divisoes base*/, 3  /*divisoes altura*/);
      return;
    case ACAO_GEO_ESFERA:
    default:
      gl::EsferaSolida(1.0f  /*raio*/, 10  /*fatias*/, 10  /*tocos*/);
      return;
  }
}

// Ação mais básica: uma sinalizacao no tabuleiro.
class AcaoSinalizacao : public Acao {
 public:
  constexpr static float TAMANHO_MAXIMO = TAMANHO_LADO_QUADRADO * 2.0f;
  AcaoSinalizacao(const AcaoProto& acao_proto) : Acao(acao_proto, nullptr), estado_(TAMANHO_MAXIMO) {
    if (!acao_proto_.has_pos_tabuleiro()) {
      estado_ = -1.0f;
    }
    if (!vbo_.Gravado()) {
      const float coordenadas[] = {
        // Primeiro triangulo.
        COS_30 * 0.3f, SEN_30 * 0.2f,
        1.0f, 0.0f,
        COS_60, SEN_60,
        // Segundo triangulo.
        -COS_30 * 0.3f, SEN_30 * 0.2f,
        -COS_60, SEN_60,
        -1.0f, 0.0f,
        // Terceiro triangulo.
        0.0f, -0.2f,
        -COS_60, -SEN_60,
        COS_60, -SEN_60
      };
      const unsigned short indices[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8,
      };
      gl::VboNaoGravado vbong("acao_sinalizacao");
      vbong.AtribuiIndices(indices, 9);
      vbong.AtribuiCoordenadas(2, coordenadas, 18);
      vbo_.Grava(vbong);
    }
  }

  void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) const override {
    gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
    gl::HabilitaEscopo normalizacao_escopo(GL_NORMALIZE);
    gl::Normal(0, 0, 1.0f);
    gl::HabilitaEscopo offset_escopo(GL_POLYGON_OFFSET_FILL);
    gl::DesvioProfundidade(-3.0, -30.0f);
    MudaCor(COR_BRANCA);

    const Posicao& pos = acao_proto_.pos_tabuleiro();
    {
      gl::MatrizEscopo salva_matriz;
      gl::Translada(pos.x(), pos.y(), pos.z());
      gl::Escala(estado_, estado_, 0.0f);
      gl::DesenhaVbo(vbo_, GL_TRIANGLES);
    }
  }

  void AtualizaAposAtraso(int intervalo_ms) override {
    const int DURACAO_SINALIZACAO_MS = 500;
    estado_ -= TAMANHO_MAXIMO * intervalo_ms / DURACAO_SINALIZACAO_MS;
  }

  bool Finalizada() const override {
    return estado_ < 0;
  }

 private:
  double estado_;
  static gl::VboGravado vbo_;
};
gl::VboGravado AcaoSinalizacao::vbo_;

// Sobe um numero verde ou vermelho de acordo com o dano causado.
// TODO fonte maior?
class AcaoDeltaPontosVida : public Acao {
 public:
  AcaoDeltaPontosVida(const AcaoProto& acao_proto, Tabuleiro* tabuleiro) : Acao(acao_proto, tabuleiro) {
    Entidade* entidade_destino = nullptr;
    if (!acao_proto_.has_pos_entidade()) {
      if (acao_proto_.id_entidade_destino_size() == 0 ||
          (entidade_destino = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_destino(0))) == nullptr) {
        faltam_ms_ = 0;
        VLOG(1) << "Finalizando delta_pontos_vida precisa de entidade destino: " << acao_proto_.ShortDebugString();
        return;
      }
      pos_ = entidade_destino->Pos();
      pos_.set_z(entidade_destino->ZOlho());
      VLOG(2) << "Acao usando entidade destino " << pos_.ShortDebugString();
    } else {
      pos_ = acao_proto_.pos_entidade();
      VLOG(2) << "Acao usando posicao entidade " << pos_.ShortDebugString();
    }
    faltam_ms_ = 0;
    // Monta a string de delta.
    if (acao_proto_.has_delta_pontos_vida()) {
      string_delta_ = "";
      if (acao_proto_.has_texto()) {
        string_delta_ = acao_proto_.texto() + "\n";
      }
      int delta = abs(acao_proto_.delta_pontos_vida());
      if (!acao_proto_.has_delta_pontos_vida()) {
        faltam_ms_ = 0;
        VLOG(1) << "Finalizando delta_pontos_vida, precisa de um delta.";
        return;
      }
      if (delta > 10000) {
        faltam_ms_ = 0;
        VLOG(1) << "Finalizando delta_pontos_vida, delta muito grande.";
        return;
      }
      if (delta == 0) {
        string_delta_ += "X";
      } else {
        char delta_texto[10] = {'\0'};
        snprintf(delta_texto, 9, "%d", delta);
        string_delta_ += delta_texto;
      }
    } else if (acao_proto_.has_texto()) {
      string_delta_ = acao_proto_.texto();
    } else {
      faltam_ms_ = 0;
      VLOG(1) << "Finalizando delta_pontos_vida, proto nao tem delta nem texto.";
      return;
    }
    VLOG(2) << "String delta: " << string_delta_;
    faltam_ms_ = DURACAO_MS;
  }

  void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) const override {
    if (!pd->desenha_detalhes()) {
      return;
    }
    gl::MatrizEscopo salva_matriz;
    gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
    gl::Translada(pos_.x(), pos_.y(), pos_.z());
    if (acao_proto_.delta_pontos_vida() > 0) {
      MudaCorAplicandoNevoa(COR_VERDE, pd);
    } else if (acao_proto_.delta_pontos_vida() == 0) {
      MudaCorAplicandoNevoa(COR_BRANCA, pd);
    } else {
      MudaCorAplicandoNevoa(COR_VERMELHA, pd);
    }
    DesenhaStringDelta();
  }

  void AtualizaAposAtraso(int intervalo_ms) override {
    pos_.set_z(pos_.z() + intervalo_ms * MAX_DELTA_Z / DURACAO_MS);
    faltam_ms_ -= intervalo_ms;
    if (faltam_ms_ <= 0) {
      VLOG(1) << "Finalizando delta_pontos_vida, MAX_ATUALIZACOES alcancado.";
    }
  }

  bool Finalizada() const override {
    return faltam_ms_ <= 0;  // 3s.
  }

 private:
  void DesenhaStringDelta() const {
    gl::DesabilitaEscopo salva_nevoa(GL_FOG);
    gl::DesabilitaEscopo salva_oclusao(gl::OclusaoLigada, gl::Oclusao);
    if (gl::PosicaoRaster(0.0f, 0.0f, 0.0f)) {
      gl::DesenhaString(string_delta_);
    }
  }

  constexpr static int DURACAO_MS = 3000;
  constexpr static float MAX_DELTA_Z = 2.0f;

  std::string string_delta_;
  Posicao pos_;
  int faltam_ms_;
};

// Acao de dispersao, estilo bola de fogo.
class AcaoDispersao : public Acao {
 public:
  AcaoDispersao(const AcaoProto& acao_proto, Tabuleiro* tabuleiro) : Acao(acao_proto, tabuleiro) {
    efeito_ = 0;
    efeito_maximo_ = TAMANHO_LADO_QUADRADO * (acao_proto.geometria() == ACAO_GEO_CONE ?
        acao_proto_.distancia_quadrados() : acao_proto_.raio_quadrados());
  }

  static Matrix4 MatrizCone(const AcaoProto& acao_proto, const Posicao& pos_origem, float distancia_m) {
    Vector3 v_origem(pos_origem.x(), pos_origem.y(), pos_origem.z());
    Vector3 v_destino(acao_proto.pos_tabuleiro().x(), acao_proto.pos_tabuleiro().y(), acao_proto.pos_tabuleiro().z());
    Vector3 diff = v_destino - v_origem;

    Matrix4 m_cone;
    m_cone.rotateY(90.0f);
    m_cone.rotateZ(180.0f);
    m_cone.translate(1.0f, 0.0f, 0.0f);
    m_cone.scale(distancia_m, distancia_m, 0.2f * distancia_m);
    m_cone = MatrizRotacao(diff) * m_cone;
    m_cone.translate(pos_origem.x(), pos_origem.y(), pos_origem.z());
    return m_cone;
  }

  void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) const override {
    //gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
    MudaCorProto(acao_proto_.cor());
    gl::MatrizEscopo salva_matriz;
    const Posicao& pos_tabuleiro = acao_proto_.pos_tabuleiro();
    if (acao_proto_.geometria() == ACAO_GEO_CONE) {
      Entidade* entidade_origem = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
      if (entidade_origem == nullptr) {
        return;
      }
      // Posicao da acao eh a ponta do cone. Computa tudo considerando nivel do solo, depois faz translacao pro nivel da acao.
      const auto& pos_origem = entidade_origem->PosicaoAcao();
      Vector3 v_origem(pos_origem.x(), pos_origem.y(), pos_origem.z());
      Vector3 v_destino(pos_tabuleiro.x(), pos_tabuleiro.y(), pos_tabuleiro.z());
      gl::MultiplicaMatriz(MatrizCone(acao_proto_, pos_origem, efeito_).get());
    } else {
      const Posicao& pos = acao_proto_.has_pos_entidade() ? acao_proto_.pos_entidade() : pos_tabuleiro;
      gl::Translada(pos.x(), pos.y(), pos.z());
      gl::Escala(efeito_, efeito_, efeito_);
    }
    DesenhaGeometriaAcao(acao_proto_.geometria());
  }

  void AtualizaAposAtraso(int intervalo_ms) override {
    if (efeito_ == 0.0f) {
      Entidade* entidade_origem = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
      const Posicao& pos = acao_proto_.pos_tabuleiro();
      if (entidade_origem != nullptr) {
        for (const auto& id_destino : acao_proto_.id_entidade_destino()) {
          auto* ed = tabuleiro_->BuscaEntidade(id_destino);
          if (ed == nullptr) {
            continue;
          }
          Vector3 v;
          v.x = ed->Pos().x() - entidade_origem->X();
          v.y = ed->Pos().y() - entidade_origem->Y();
          v.z = ed->Pos().z() - entidade_origem->Z();
          v.normalize() /= 10.0f;
          dx_ = v.x;
          dy_ = v.y;
          dz_ = v.z;
          AtualizaDirecaoQuedaAlvo(ed);
        }
        Vector3 v;
        v.x = pos.x() - entidade_origem->X();
        v.y = pos.y() - entidade_origem->Y();
        v.z = pos.z() - entidade_origem->Z();
        v.normalize() /= 10.0f;
        dx_ = v.x;
        dy_ = v.y;
        dz_ = v.z;
        AtualizaRotacaoZFonte(entidade_origem);
      }
    }
    efeito_ += efeito_maximo_ * static_cast<float>(intervalo_ms) / DURACAO_MS;
  }

  bool Finalizada() const override {
    return efeito_ > efeito_maximo_;
  }

 private:
  constexpr static int DURACAO_MS = 500;
  float efeito_maximo_;
  float efeito_;
};

// Uma acao de projetil, tipo flecha ou missil magico.
class AcaoProjetil : public Acao {
 public:
  AcaoProjetil(const AcaoProto& acao_proto, Tabuleiro* tabuleiro) : Acao(acao_proto, tabuleiro) {
    estagio_ = INICIAL;
    auto* entidade_origem = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    if (entidade_origem == nullptr) {
      VLOG(1) << "Finalizando projetil, precisa de entidade origem.";
      estagio_ = FIM;
      return;
    }
    if (acao_proto_.id_entidade_destino_size() == 0) {
      VLOG(1) << "Finalizando projetil, nao ha entidade destino.";
      estagio_ = FIM;
      return;
    }
    if (acao_proto_.id_entidade_origem() == acao_proto_.id_entidade_destino(0)) {
      VLOG(1) << "Finalizando projetil, entidade origem == destino.";
      estagio_ = FIM;
      return;
    }
    pos_ = entidade_origem->PosicaoAcao();
  }

  void AtualizaAposAtraso(int intervalo_ms) override {
    if (estagio_ == INICIAL) {
      estagio_ = VOO;
      AtualizaVoo(intervalo_ms);
      // Atualiza depois, para ter dx, e dy.
      AtualizaRotacaoZFonte(tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem()));
    } else if (estagio_ == VOO) {
      AtualizaVoo(intervalo_ms);
    } else if (estagio_ == ATINGIU_ALVO) {
      if (!AtualizaAlvo(intervalo_ms)) {
        VLOG(1) << "Terminando acao projetil, alvo atualizado.";
        estagio_ = FIM;
      }
    } else {
      return;
    }
  }

  void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) const override {
    if (estagio_ == ATINGIU_ALVO) {
      return;
    }
    // TODO desenha impacto.
    gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
    gl::MatrizEscopo salva_matriz;
    MudaCorProto(acao_proto_.cor());
    gl::Translada(pos_.x(), pos_.y(), pos_.z());
    // Roda pro vetor de direcao.
    gl::Roda(VetorParaRotacaoGraus(dx_, dy_), 0, 0, 1.0f);
    gl::Escala(acao_proto_.escala().x(), acao_proto_.escala().y(), acao_proto_.escala().z());
    DesenhaGeometriaAcao(acao_proto_.has_geometria() ? acao_proto_.geometria() : ACAO_GEO_ESFERA);
  }

  bool Finalizada() const override {
    return estagio_ == FIM;
  }

 private:
  void AtualizaVoo(int intervalo_ms) {
    Entidade* entidade_destino = nullptr;
    if ((entidade_destino = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_destino(0))) == nullptr) {
      VLOG(1) << "Finalizando projetil, destino não existe.";
      estagio_ = FIM;
      return;
    }
    const auto& pos_destino = entidade_destino->PosicaoAcao();
    // Recalcula vetor.
    dx_ = pos_destino.x() - pos_.x();
    dy_ = pos_destino.y() - pos_.y();
    dz_ = pos_destino.z() - pos_.z();
    AtualizaVelocidade(intervalo_ms);
    VLOG(1) << "Velocidade: " << velocidade_m_ms_;
    Vector3 v(dx_, dy_, dz_);
    Vector3 vn(v.normalize());
    v *= (velocidade_m_ms_ * intervalo_ms);
    // Posicao antes.
    float xa = pos_.x();
    float ya = pos_.y();
    float za = pos_.z();
    // Antes, depois, destino.
    pos_.set_x(ArrumaSePassou(xa, xa + v.x, pos_destino.x()));
    pos_.set_y(ArrumaSePassou(ya, ya + v.y, pos_destino.y()));
    pos_.set_z(ArrumaSePassou(za, za + v.z, pos_destino.z()));
    // Deslocamento do alvo.
    vn /= 2.0f;  // meio metro de deslocamento.
    dx_ = vn.x;
    dy_ = vn.y;
    dz_ = vn.z;
    if (pos_.x() == pos_destino.x() &&
        pos_.y() == pos_destino.y() &&
        pos_.z() == pos_destino.z()) {
      VLOG(1) << "Projetil atingiu alvo.";
      estagio_ = ATINGIU_ALVO;
      return;
    }
  }

  // Verifica se a coordenada passou do ponto de destino.
  static bool Passou(float antes, float depois, float destino) {
    return (antes < destino) ? depois > destino : depois < destino;
  }

  // Retorna depois se a coordenada nao passou de destino, caso contrario retorna destino.
  static float ArrumaSePassou(float antes, float depois, float destino) {
    return Passou(antes, depois, destino) ? destino : depois;
  }

  enum estagio_e {
    INICIAL = 0,
    VOO,
    ATINGIU_ALVO,
    FIM
  } estagio_;
  Posicao pos_;
};

// Acao de raio.
class AcaoRaio : public Acao {
 public:
  AcaoRaio(const AcaoProto& acao_proto, Tabuleiro* tabuleiro) : Acao(acao_proto, tabuleiro) {
    duracao_ = acao_proto.duracao_s();
    if (!acao_proto_.has_id_entidade_origem()) {
      duracao_ = 0.0f;
      VLOG(1) << "Acao raio requer id origem.";
      return;
    }
    if (acao_proto_.id_entidade_destino_size() == 0 && !acao_proto_.has_pos_tabuleiro()) {
      duracao_ = 0.0f;
      VLOG(1) << "Acao raio requer id destino ou posicao destino.";
      return;
    }

    if (!acao_proto_.efeito_area() &&
        acao_proto_.id_entidade_destino_size() > 0 && acao_proto_.id_entidade_origem() == acao_proto_.id_entidade_destino(0)) {
      duracao_ = 0.0f;
      VLOG(1) << "Acao raio requer origem e destino diferentes.";
      return;
    }
  }

  void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) const override {
    auto* eo = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    if (eo == nullptr) {
      return;
    }
    const Posicao& pos_o = eo->PosicaoAcao();
    Posicao pos_d = acao_proto_.pos_tabuleiro();
    if (!acao_proto_.efeito_area() && acao_proto_.id_entidade_destino_size() > 0) {
      auto* ed = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_destino(0));
      if (ed == nullptr) {
        return;
      }
      pos_d = ed->PosicaoAcao();
    } else {
      // Poe na mesma altura da origem.
      //pos_d.set_z(pos_o.z());
    }
    MudaCorProto(acao_proto_.cor());
    gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
    float dx = pos_d.x() - pos_o.x();
    float dy = pos_d.y() - pos_o.y();
    float dz = pos_d.z() - pos_o.z();
    float tam;
    gl::Translada(pos_o.x(), pos_o.y(), pos_o.z());
    gl::Roda(VetorParaRotacaoGraus(dx, dy, &tam), 0.0f,  0.0f, 1.0f);
    if (acao_proto_.has_distancia_quadrados()) {
      tam = acao_proto_.distancia_quadrados() * TAMANHO_LADO_QUADRADO;
    }
    float tam2 = 0;
    //LOG(INFO) << "ang: " << VetorParaRotacaoGraus(dz, tam, &tam2) << ", tam2: " << tam2 << ", pos_d.z(): " << pos_d.z();
    gl::Roda(VetorParaRotacaoGraus(dz, tam, &tam2), 0.0f, 1.0f, 0.0f);
    gl::ConeSolido(0.2f, tam2, 6  /*fatias*/, 1  /*tocos*/);
  }

  void AtualizaAposAtraso(int intervalo_ms) override {
    auto* eo = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    if (eo == nullptr) {
      VLOG(1) << "Terminando acao pois origem nao existe mais.";
      duracao_ = 0.0f;
      return;
    }
    if (duracao_ == acao_proto_.duracao_s()) {
      for (unsigned int id_destino : acao_proto_.id_entidade_destino()) {
        auto* ed = tabuleiro_->BuscaEntidade(id_destino);
        if (ed == nullptr) {
          continue;
        }
        const Posicao& pos_o = eo->PosicaoAcao();
        const Posicao& pos_d = ed->Pos();
        float dx = pos_d.x() - pos_o.x();
        float dy = pos_d.y() - pos_o.y();
        ed->AtualizaDirecaoDeQueda(dx, dy, 0);
      }
    }
    if (duracao_ > 0.0f) {
      duracao_ -= (intervalo_ms / 1000.0f);
    }
    if (duracao_ <= 0.0f) {
      VLOG(1) << "Finalizando raio, duracao acabou";
    }
  }

  bool Finalizada() const override {
    return duracao_ <= 0.0f;
  }

 private:
  float duracao_;
};

// Acao ACAO_CORPO_A_CORPO.
class AcaoCorpoCorpo : public Acao {
 public:
  AcaoCorpoCorpo(const AcaoProto& acao_proto, Tabuleiro* tabuleiro) : Acao(acao_proto, tabuleiro) {
    rotacao_graus_ = 0.0f;
    if (!acao_proto_.has_id_entidade_origem()) {
      VLOG(1) << "Acao corpo a corpo requer id origem.";
      finalizado_ = true;
      return;
    }
    if (acao_proto_.id_entidade_destino_size() == 0) {
      VLOG(1) << "Acao corpo a corpo requer id destino.";
      finalizado_ = true;
      return;
    }
    if (acao_proto_.id_entidade_origem() == acao_proto_.id_entidade_destino(0)) {
      VLOG(1) << "Acao corpo a corpo requer origem e destino diferentes.";
      finalizado_ = true;
      return;
    }
    AtualizaDeltas();
    finalizado_ = false;
  }

  void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) const override {
    auto* eo = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    if (eo == nullptr) {
      return;
    }
    const Posicao& pos_o = eo->PosicaoAcao();
    MudaCorProto(acao_proto_.cor());
    gl::DesabilitaEscopo cull_escopo(GL_CULL_FACE);
    gl::Translada(pos_o.x(), pos_o.y(), pos_o.z());
    gl::Roda(direcao_graus_, 0.0f,  0.0f, 1.0f);
    gl::Roda(90.0f, 1.0f, 0.0f,  0.0f);  // o triangulo eh no Y, entao traz ele pro Z.
    gl::Roda(rotacao_graus_, 0.0f, 0.0f, -1.0f);
    gl::Escala(0.2f, distancia_ + TAMANHO_LADO_QUADRADO_2, 0.2f);
    gl::Triangulo(1.0f);
  }

  void AtualizaAposAtraso(int intervalo_ms) override {
    auto* eo = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    if (eo == nullptr) {
      VLOG(1) << "Terminando acao corpo a corpo: origem nao existe mais.";
      finalizado_ = true;
      return;
    }
    if (rotacao_graus_ == 0.0f) {
      AtualizaRotacaoZFonte(eo);
    }

    // TODO desenhar o impacto.
    // Os parametros iniciais sao mantidos, so a rotacao do corte eh alterada.
    if (rotacao_graus_ < 180.0f) {
      rotacao_graus_ += intervalo_ms * 180.0f / DURACAO_MS;
    }
    bool terminou_alvo = false;
    if (rotacao_graus_ >= 90.0f) {
      terminou_alvo = !AtualizaAlvo(intervalo_ms);
    }
    if (rotacao_graus_ >= 180.0f && terminou_alvo) {
      VLOG(1) << "Finalizando corpo a corpo";
      finalizado_ = true;
    }
  }

  bool Finalizada() const override {
    return finalizado_;
  }

 private:
  // Atualiza a direcao.
  void AtualizaDeltas() {
    auto* eo = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    auto* ed = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_destino(0));
    if (eo == nullptr || ed == nullptr) {
      VLOG(1) << "Terminando acao corpo a corpo: origem ou destino nao existe mais.";
      finalizado_ = true;
      return;
    }
    const Posicao& pos_o = eo->PosicaoAcao();
    const Posicao& pos_d = ed->PosicaoAcao();
    dx_ = pos_d.x() - pos_o.x();
    dy_ = pos_d.y() - pos_o.y();
    dz_ = pos_d.z() - pos_o.z();
    direcao_graus_ = VetorParaRotacaoGraus(dx_, dy_);
    Vector3 v(dx_, dy_, dz_);
    distancia_ = v.length();
    // Ajusta os deltas para nao ficar deslocamento gigante.
    //LOG(INFO) << "v.length1: " << distancia_;
    v /= 3.0f;  // 1/3 da distancia como delta.
    //LOG(INFO) << "v.length2: " << v.length();
    float tam = std::max(std::min(v.length(), TAMANHO_LADO_QUADRADO * 2.0f), 0.1f);  // min 0.1, maximo 2 quadrados (3m).
    //LOG(INFO) << "Tam: " << tam;
    v.normalize();
    v *= tam;
    dx_ = v.x;
    dy_ = v.y;
    dz_ = v.z;
  }
  float distancia_;
  float direcao_graus_;
  float rotacao_graus_;
  bool finalizado_;
  constexpr static int DURACAO_MS = 180;
};

// Acao de feitico de toque.
class AcaoFeiticoToque : public Acao {
 public:
  AcaoFeiticoToque(const AcaoProto& acao_proto, Tabuleiro* tabuleiro) : Acao(acao_proto, tabuleiro) {
    if (!acao_proto_.has_id_entidade_origem() || acao_proto_.id_entidade_destino_size() == 0) {
      VLOG(1) << "Acao de feitico de toque requer origem e destino";
      terminado_ = true;
      return;
    }
    terminado_ = false;
    desenhando_origem_ = true;
    raio_ = 1.0f;
  }

  void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) const override {
    auto* e = tabuleiro_->BuscaEntidade(desenhando_origem_ ? acao_proto_.id_entidade_origem() : acao_proto_.id_entidade_destino(0));
    if (e == nullptr) {
      return;
    }
    const Posicao& pos = e->PosicaoAcao();
    MudaCorProto(acao_proto_.cor());
    gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
    gl::Translada(pos.x() + acao_proto_.translacao().x(),
                  pos.y() + acao_proto_.translacao().y(),
                  pos.z() + acao_proto_.translacao().z());
    gl::Escala(acao_proto_.escala().x() * raio_, acao_proto_.escala().y() * raio_, acao_proto_.escala().z() * raio_);
    DesenhaGeometriaAcao(acao_proto_.geometria());
  }

  void AtualizaAposAtraso(int intervalo_ms) override {
    auto* e = tabuleiro_->BuscaEntidade(desenhando_origem_ ? acao_proto_.id_entidade_origem() : acao_proto_.id_entidade_destino(0));
    if (e == nullptr) {
      VLOG(1) << "Terminando acao feitico: origem ou destino nao existe mais.";
      terminado_ = true;
      return;
    }
    const float DELTA_RAIO = static_cast<float>(intervalo_ms) / DURACAO_MS;
    if (desenhando_origem_) {
      raio_ -= DELTA_RAIO;
      if (raio_ <= 0) {
        desenhando_origem_ = false;
      }
    } else {
      raio_ += DELTA_RAIO;
      if (raio_ >= 1.0f) {
        terminado_ = true;
      }
    }
  }

  bool Finalizada() const override {
    return terminado_;
  }

 private:
  constexpr static int DURACAO_MS = 480;
  bool desenhando_origem_;
  float raio_;
  bool terminado_;
};

}  // namespace

// Acao.
Acao::Acao(const AcaoProto& acao_proto, Tabuleiro* tabuleiro)
    : acao_proto_(acao_proto), tabuleiro_(tabuleiro) {
  velocidade_m_ms_ = acao_proto.velocidade().inicial_m_s() / 1000.0f;
  aceleracao_m_ms_2_ = acao_proto.velocidade().aceleracao_m_s_2() / 1000.0f;
  dx_ = dy_ = dz_ = 0;
  dx_total_ = dy_total_ = dz_total_ = 0;
  disco_alvo_rad_ = 0;
  atraso_s_ = acao_proto_.atraso_s();
}

void Acao::Atualiza(int intervalo_ms) {
  if (atraso_s_ > 0) {
    atraso_s_ -= (intervalo_ms / 1000.0f);
    return;
  }
  AtualizaAposAtraso(intervalo_ms);
}

int Acao::IdCenario() const {
  if (acao_proto_.has_id_entidade_origem()) {
    auto* entidade_origem = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    if (entidade_origem != nullptr) {
      return entidade_origem->IdCenario();
    } else {
      LOG(WARNING) << "Cenario invalido para acao";
      return CENARIO_INVALIDO;
    }
  } else if (acao_proto_.has_pos_entidade()) {
    return acao_proto_.pos_entidade().id_cenario();
  } else if (acao_proto_.id_entidade_destino_size() > 0) {
    auto* entidade_destino = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_destino(0));
    if (entidade_destino != nullptr) {
      return entidade_destino->IdCenario();
    } else {
      return CENARIO_INVALIDO;
    }
  }
  return CENARIO_INVALIDO;
}

void Acao::Desenha(ParametrosDesenho* pd) const {
  if (atraso_s_ > 0) {
    return;
  }
  if (Finalizada()) {
    return;
  }
  gl::MatrizEscopo salva_matriz(GL_MODELVIEW);
  DesenhaSeNaoFinalizada(pd);
}

void Acao::DesenhaTranslucido(ParametrosDesenho* pd) const {
  if (Finalizada()) {
    return;
  }
  gl::MatrizEscopo salva_matriz(GL_MODELVIEW);
  DesenhaTranslucidoSeNaoFinalizada(pd);
}

void Acao::AtualizaVelocidade(int intervalo_ms) {
  VLOG(1) << "velocidade_m_ms antes: " << velocidade_m_ms_
          << ", aceleracao_m_ms_2_ antes: " << aceleracao_m_ms_2_;
  int tipo_aceleracao = acao_proto_.velocidade().tipo_aceleracao();
  switch (tipo_aceleracao) {
    case ACAO_ACEL_ZERO:
      break;
    case ACAO_ACEL_CONSTANTE:
      velocidade_m_ms_ += intervalo_ms * aceleracao_m_ms_2_;
      break;
    case ACAO_ACEL_LINEAR:
      aceleracao_m_ms_2_ += intervalo_ms * (acao_proto_.velocidade().delta_aceleracao_m_s_3() / 1000.0f);
      velocidade_m_ms_ += intervalo_ms * aceleracao_m_ms_2_;
      break;
    default:
      LOG(WARNING) << "Tipo de aceleracao invalida.";
      break;
  }
  float vmax_m_ms = acao_proto_.velocidade().maxima_m_s() / 1000.0f;
  if (velocidade_m_ms_ > vmax_m_ms) {
    velocidade_m_ms_ = vmax_m_ms;
  }
  VLOG(1) << "velocidade_m_ms depois: " << velocidade_m_ms_
          << ", aceleracao_m_ms_2_ depois: " << aceleracao_m_ms_2_;
}

void Acao::AtualizaRotacaoZFonte(Entidade* entidade) {
  if (entidade == nullptr) {
    return;
  }
  Posicao vr;
  vr.set_x(dx_);
  vr.set_y(dy_);
  entidade->AlteraRotacaoZGraus(VetorParaRotacaoGraus(vr));
}

void Acao::AtualizaDirecaoQuedaAlvo(Entidade* entidade) {
  if (entidade == nullptr) {
    return;
  }
  entidade->AtualizaDirecaoDeQueda(dx_, dy_, dz_);
}

void Acao::AtualizaRotacaoZFonteRelativoTabuleiro(Entidade* entidade) {
  if (entidade == nullptr) {
    return;
  }
  Posicao v;
  const Posicao& pos_tabuleiro = acao_proto_.pos_tabuleiro();
  v.set_x(pos_tabuleiro.x() - entidade->X());
  v.set_y(pos_tabuleiro.y() - entidade->Y());
  entidade->AlteraRotacaoZGraus(VetorParaRotacaoGraus(v));
}

void Acao::AtualizaDirecaoQuedaAlvoRelativoTabuleiro(Entidade* entidade) {
  if (entidade == nullptr) {
    return;
  }
  Vector3 v;
  const Posicao& pos_tabuleiro = acao_proto_.pos_tabuleiro();
  v.x = entidade->X() - pos_tabuleiro.x();
  v.y = entidade->Y() - pos_tabuleiro.y();
  v.z = entidade->Z() - pos_tabuleiro.z();
  v.normalize();
  entidade->AtualizaDirecaoDeQueda(v.x, v.y, v.z);
}

bool Acao::AtualizaAlvo(int intervalo_ms) {
  Entidade* entidade_destino = nullptr;
  if (acao_proto_.id_entidade_destino_size() == 0 ||
      (entidade_destino = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_destino(0))) == nullptr) {
    VLOG(1) << "Finalizando alvo, destino não existe.";
    return false;
  }
  if (entidade_destino->Proto().fixa()) {
    VLOG(1) << "Finalizando alvo fixo.";
    dx_total_ = dy_total_ = dz_total_ = 0;
    return false;
  }

  // Move o alvo na direcao do impacto e volta se nao estiver caido.
  if (disco_alvo_rad_ >= (M_PI / 2.0f) && entidade_destino->Proto().morta()) {
    VLOG(1) << "Finalizando alvo, entidade morta nao precisa voltar.";
    dx_total_ = dy_total_ = dz_total_ = 0;
    return false;
  }
  if (disco_alvo_rad_ >= M_PI) {
    VLOG(1) << "Finalizando alvo, arco terminou.";
    MoveDeltaRespeitandoChao(-dx_total_, -dy_total_, -dz_total_, *tabuleiro_, entidade_destino);
    dx_total_ = dy_total_ = dz_total_ = 0;
    return false;
  }
  const int DURACAO_ATUALIZACAO_ALVO_MS = 100;
  // dt representa a fracao do arco ate 90 graus que o alvo andou. Os outros 90 sao da volta.
  const float dt = std::min((static_cast<float>(intervalo_ms * 2.0f) / DURACAO_ATUALIZACAO_ALVO_MS), 1.0f);
  float cos_delta_alvo = cosf(disco_alvo_rad_) * dt;
  float dx_alvo = dx_ * cos_delta_alvo;
  float dy_alvo = dy_ * cos_delta_alvo;
  float dz_alvo = dz_ * cos_delta_alvo;
  float x_antes = entidade_destino->X();
  float y_antes = entidade_destino->Y();
  float z_antes = entidade_destino->Z();
  MoveDeltaRespeitandoChao(dx_alvo, dy_alvo, dz_alvo, *tabuleiro_, entidade_destino);
  dx_total_ += entidade_destino->X() - x_antes;
  dy_total_ += entidade_destino->Y() - y_antes;
  dz_total_ += entidade_destino->Z() - z_antes;
  VLOG(1) << "Atualizando alvo: intervalo_ms: " << intervalo_ms << ", dt; " << dt
          << ", dx_total: " << dx_total_ << ", dy_total: " << dy_total_ << ", dz_total: " << dz_total_;
  if (disco_alvo_rad_ == 0.0f && estado_alvo_ == ALVO_NAO_ATINGIDO) {
    AtualizaDirecaoQuedaAlvo(entidade_destino);
    estado_alvo_ = ALVO_A_SER_ATINGIDO;
  }
  disco_alvo_rad_ += dt * M_PI / 2.0f;
  return true;
}

// static
bool Acao::PontoAfetadoPorAcao(const Posicao& pos_ponto, const Posicao& pos_origem, const AcaoProto& acao_proto) {
  switch (acao_proto.tipo()) {
    case ACAO_DISPERSAO:
      switch (acao_proto.geometria()) {
        case ACAO_GEO_ESFERA:
          return DistanciaQuadrado(pos_ponto, acao_proto.pos_tabuleiro()) <= powf(acao_proto.raio_quadrados() * TAMANHO_LADO_QUADRADO, 2);
        case ACAO_GEO_CONE: {
          // Vetor do ponto com relacao a origem.
          Vector3 v_origem(pos_origem.x(), pos_origem.y(), pos_origem.z());
          Vector3 v_destino(Vector3(pos_ponto.x(), pos_ponto.y(), pos_ponto.z()) - v_origem);
          float distancia = v_destino.length();
          if (distancia == 0.0f || distancia > (acao_proto.distancia_quadrados() * TAMANHO_LADO_QUADRADO)) {
            return false;
          }
          Vector3 direcao_cone(Vector3(acao_proto.pos_tabuleiro().x(), acao_proto.pos_tabuleiro().y(), acao_proto.pos_tabuleiro().z())  - v_origem);
          if (direcao_cone == Vector3()) {
            LOG(WARNING) << "Cone sem direcao: " << acao_proto.ShortDebugString();
            return false;
          }
          direcao_cone.normalize();
          v_destino.normalize();
          // Angulo entre os vetores.
          float angulo = acosf(direcao_cone.dot(v_destino)) * RAD_PARA_GRAUS;
          static float angulo_cone = atanf(0.5f) * RAD_PARA_GRAUS;
          return (angulo < angulo_cone);  // esse eh +- o angulo do cone.
        }
        break;
        default:
          return false;
      }
      break;
    case ACAO_RAIO: {
      if (!acao_proto.efeito_area()) {
        return false;
      }
      Vector3 v_origem(pos_origem.x(), pos_origem.y(), pos_origem.z());
      Vector3 v_destino(Vector3(pos_ponto.x(), pos_ponto.y(), pos_ponto.z()) - v_origem);
      float distancia = v_destino.length();
      LOG(INFO) << "distancia: " << distancia;
      if (distancia == 0.0f || distancia > (acao_proto.distancia_quadrados() * TAMANHO_LADO_QUADRADO)) {
        return false;
      }
      Vector3 direcao_raio(Vector3(acao_proto.pos_tabuleiro().x(), acao_proto.pos_tabuleiro().y(), acao_proto.pos_tabuleiro().z())  - v_origem);
      if (direcao_raio == Vector3()) {
        LOG(WARNING) << "Raio sem direcao: " << acao_proto.ShortDebugString();
        return false;
      }
      direcao_raio.normalize();
      v_destino.normalize();
      // Angulo entre os vetores.
      float angulo_rad = acosf(direcao_raio.dot(v_destino));
      float angulo = angulo_rad * RAD_PARA_GRAUS;
      if (angulo > 90.0f) {
        //LOG(INFO) << "angulo maior que 90.0f: " << angulo; 
        return false;
      }
      float distancia_para_raio = sinf(angulo_rad) * distancia;
      //LOG(INFO) << "angulo: " << angulo << ", distancia_para_raio: " << distancia_para_raio;
      return distancia_para_raio < TAMANHO_LADO_QUADRADO_2;
    }
    break;
    default:
      return false;
  }
  return false;
}

Acao* NovaAcao(const AcaoProto& acao_proto, Tabuleiro* tabuleiro) {
  switch (acao_proto.tipo()) {
    case ACAO_SINALIZACAO:
      return new AcaoSinalizacao(acao_proto);
    case ACAO_PROJETIL:
      return new AcaoProjetil(acao_proto, tabuleiro);
    case ACAO_DISPERSAO:
      return new AcaoDispersao(acao_proto, tabuleiro);
    case ACAO_DELTA_PONTOS_VIDA:
      return new AcaoDeltaPontosVida(acao_proto, tabuleiro);
    case ACAO_RAIO:
      return new AcaoRaio(acao_proto, tabuleiro);
    case ACAO_CORPO_A_CORPO:
      return new AcaoCorpoCorpo(acao_proto, tabuleiro);
    case ACAO_FEITICO_TOQUE:
      return new AcaoFeiticoToque(acao_proto, tabuleiro);
    default:
      LOG(ERROR) << "Acao invalida: " << acao_proto.ShortDebugString();
      return nullptr;
  }
}

}  // namespace ent
