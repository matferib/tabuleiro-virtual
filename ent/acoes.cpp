#include <algorithm>
#include <cmath>
#include <functional>

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
#include "tex/texturas.h"

namespace ent {

namespace {

using std::placeholders::_1;

void MudaCorProto(const Cor& cor) {
  const GLfloat corgl[3] = { cor.r(), cor.g(), cor.b() };
  MudaCor(corgl);
}
void MudaCorProtoAlfa(const Cor& cor) {
  const GLfloat corgl[4] = { cor.r(), cor.g(), cor.b(), cor.a() };
  MudaCorAlfa(corgl);
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
    case ACAO_GEO_CILINDRO:
      gl::CilindroSolido(1.0f  /*raio*/, 2.0f  /*altura*/, 10  /*divisoes base*/, 3  /*divisoes altura*/);
      return;
    case ACAO_GEO_ESFERA:
    default:
      gl::EsferaSolida(1.0f  /*raio*/, 10  /*fatias*/, 10  /*tocos*/);
      return;
  }
}

// Verifica se a coordenada passou do ponto de destino.
bool Passou(float antes, float depois, float destino) {
  return (antes < destino) ? depois > destino : depois < destino;
}

// Retorna depois se a coordenada nao passou de destino, caso contrario retorna destino.
float ArrumaSePassou(float antes, float depois, float destino) {
  return Passou(antes, depois, destino) ? destino : depois;
}

// Ação mais básica: uma sinalizacao no tabuleiro.
class AcaoSinalizacao : public Acao {
 public:
  constexpr static float TAMANHO_MAXIMO = TAMANHO_LADO_QUADRADO * 2.0f;
  AcaoSinalizacao(const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas) :
      Acao(acao_proto, tabuleiro, texturas), estado_(TAMANHO_MAXIMO) {
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

// Ação mais básica: uma sinalizacao no tabuleiro.
class AcaoPocao: public Acao {
 public:
  AcaoPocao(const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas) :
      Acao(acao_proto, tabuleiro, texturas) {
    bolhas_.push_back(std::move(gl::VboEsferaSolida(0.15f, 6, 6)));
    bolhas_.push_back(std::move(gl::VboEsferaSolida(0.1f, 6, 6)));
    bolhas_.push_back(std::move(gl::VboEsferaSolida(0.2f, 6, 6)));
    bolhas_[0].Translada(acao_proto_.pos_entidade().x(), acao_proto_.pos_entidade().y(), acao_proto_.pos_entidade().z());
    bolhas_[1].Translada(acao_proto_.pos_entidade().x() + 0.2f, acao_proto_.pos_entidade().y(), acao_proto_.pos_entidade().z());
    bolhas_[2].Translada(acao_proto_.pos_entidade().x(), acao_proto_.pos_entidade().y() + 0.3f, acao_proto_.pos_entidade().z());
    for (auto& b : bolhas_) {
      b.AtribuiCor(acao_proto_.cor().r(), acao_proto_.cor().g(), acao_proto_.cor().b(), 0.5f);
    }
    duracao_ms_ = 0;
    acao_proto_.mutable_cor()->set_a(0.5f);
  }

  void DesenhaTranslucidoSeNaoFinalizada(ParametrosDesenho* pd) const override {
    VLOG(3) << "Desenhando acao pocao";
    gl::DesenhaVbo(bolhas_[0]);
    if (duracao_ms_ > INTERVALO_MS) gl::DesenhaVbo(bolhas_[1]);
    if (duracao_ms_ > INTERVALO_MS * 2) gl::DesenhaVbo(bolhas_[2]);
  }

  void AtualizaAposAtraso(int intervalo_ms) override {
    if (duracao_ms_ == 0) {
      // Priemeiro frame.
      ++duracao_ms_;
      return;
    }
    VLOG(3) << "Atualizando acao pocao, duracao_ms: " << duracao_ms_;
    const float delta = (intervalo_ms * VELOCIDADE_BOLHA_M_S) / 1000.0f;
    bolhas_[0].Translada(0.0f, 0.0f, delta);
    if (duracao_ms_ > INTERVALO_MS) bolhas_[1].Translada(0.0f, 0.0f, delta);
    if (duracao_ms_ > INTERVALO_MS * 2) bolhas_[2].Translada(0.0f, 0.0f, delta);
    duracao_ms_ += intervalo_ms;
  }

  bool Finalizada() const override {
    return duracao_ms_ > MAX_DURACAO_MS;
  }

 private:
  static constexpr int MAX_DURACAO_MS = 3000;
  static constexpr float VELOCIDADE_BOLHA_M_S = 0.3f;
  static constexpr float INTERVALO_MS = 500;
  int duracao_ms_;
  std::vector<gl::VboNaoGravado> bolhas_;
};

class AcaoAgarrar : public Acao {
 public:
  AcaoAgarrar(const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas) :
      Acao(acao_proto, tabuleiro, texturas) {
    Entidade* entidade_destino = EntidadeDestino();
    Entidade* entidade_origem = EntidadeOrigem();
    if (entidade_origem == nullptr || entidade_destino == nullptr || entidade_destino->Proto().fixa())  {
      VLOG(1) << "Finalizando alvo, origem ou destino não existe ou eh fixo.";
      finalizada_ = true;
      return;
    }

    auto vo_vd = PosParaVector3(entidade_origem->Pos()) - PosParaVector3(entidade_destino->Pos());
    if (vo_vd.length() < 0.001) return;
    auto deslocamento = vo_vd.normalize() * TAMANHO_LADO_QUADRADO_2;
    dx_ = deslocamento.x;
    dy_ = deslocamento.y;
    dz_ = deslocamento.z;
  }

  void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) const override {
  }

  void AtualizaAposAtraso(int intervalo_ms) override {
    finalizada_ = !AtualizaAlvo(intervalo_ms);
  }

  bool Finalizada() const override {
    return finalizada_;
  }

 private:
  bool finalizada_ = false;
};

// Sobe um numero verde ou vermelho de acordo com o dano causado.
// TODO fonte maior?
class AcaoDeltaPontosVida : public Acao {
 public:
  AcaoDeltaPontosVida(const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas)
      : Acao(acao_proto, tabuleiro, texturas) {
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
    gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
    gl::MatrizEscopo salva_matriz;
    gl::Translada(pos_.x(), pos_.y(), pos_.z());
    if (acao_proto_.has_cor()) {
      const float cor[] = { acao_proto_.cor().r(), acao_proto_.cor().g(), acao_proto_.cor().b() };
      MudaCorAplicandoNevoa(cor, pd);
    } else if (acao_proto_.delta_pontos_vida() > 0) {
      MudaCorAplicandoNevoa(COR_VERDE, pd);
    } else if (acao_proto_.delta_pontos_vida() == 0) {
      MudaCorAplicandoNevoa(COR_BRANCA, pd);
    } else {
      MudaCorAplicandoNevoa(COR_VERMELHA, pd);
    }
    DesenhaStringDelta();
  }

  void AtualizaAposAtraso(int intervalo_ms) override {
    if (faltam_ms_ == DURACAO_MS) {
      // Primeiro frame. Apenas posiciona na posicao inicial. Importante pos UI para nao pular o efeito.
      --faltam_ms_;
    } else {
      pos_.set_z(pos_.z() + intervalo_ms * MAX_DELTA_Z / DURACAO_MS);
      faltam_ms_ -= intervalo_ms;
    }
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
      gl::DesenhaString(StringSemUtf8(string_delta_));
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
  AcaoDispersao(const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas) : Acao(acao_proto, tabuleiro, texturas) {
    efeito_ = 0;
    efeito_maximo_ = TAMANHO_LADO_QUADRADO * (acao_proto.geometria() == ACAO_GEO_CONE ?
        acao_proto_.distancia_quadrados() : acao_proto_.raio_quadrados());
  }

  ~AcaoDispersao() {
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
    MudaCorProtoAlfa(acao_proto_.cor());
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
    gl::DesabilitaEscopo luz(GL_LIGHTING);
    DesenhaGeometriaAcao(acao_proto_.geometria());
  }

  void AtualizaAposAtraso(int intervalo_ms) override {
    if (efeito_ == 0.0f) {
      Entidade* entidade_origem = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
      const auto& pos_origem = (entidade_origem != nullptr) && (acao_proto_.geometria() == ACAO_GEO_CONE)
          ? entidade_origem->Pos() : acao_proto_.pos_tabuleiro();
      const Posicao& pos = acao_proto_.pos_tabuleiro();
      for (const auto& id_destino : acao_proto_.id_entidade_destino()) {
        auto* ed = tabuleiro_->BuscaEntidade(id_destino);
        if (ed == nullptr) {
          continue;
        }
        Vector3 v;
        v.x = ed->Pos().x() - pos_origem.x();
        v.y = ed->Pos().y() - pos_origem.y();
        v.z = ed->Pos().z() - pos_origem.z();
        if (fabs(v.length()) > 0.001f) {
          v.normalize() /= 10.0f;
          dx_ = v.x;
          dy_ = v.y;
          dz_ = v.z;
          AtualizaDirecaoQuedaAlvo(ed);
        }
      }
      if (entidade_origem != nullptr) {
        Vector3 v;
        v.x = pos.x() - entidade_origem->X();
        v.y = pos.y() - entidade_origem->Y();
        v.z = pos.z() - entidade_origem->Z();
        if (fabs(v.length()) > 0.001f) {
          v.normalize() /= 10.0f;
          dx_ = v.x;
          dy_ = v.y;
          dz_ = v.z;
          AtualizaRotacaoZFonte(entidade_origem);
        }
      }
    }
    efeito_ += efeito_maximo_ * static_cast<float>(intervalo_ms) / (acao_proto_.duracao_s() * 1000);
    if (Finalizada()) {
      AtualizaAlvo(intervalo_ms);
    }
  }

  bool Finalizada() const override {
    return efeito_ > efeito_maximo_;
  }

 private:
  float efeito_maximo_;
  float efeito_;
};

// Acao de dispersao, estilo bola de fogo.
class AcaoProjetilArea: public Acao {
 public:
  AcaoProjetilArea(const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas) : Acao(acao_proto, tabuleiro, texturas) {
    auto* entidade_origem = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    if (entidade_origem == nullptr) {
      VLOG(1) << "Finalizando projetil area, precisa de entidade origem.";
      estagio_ = FIM;
      return;
    }
    if (acao_proto_.id_entidade_destino_size() == 0) {
      VLOG(1) << "Finalizando projetil area, nao ha entidade destino.";
      estagio_ = FIM;
      return;
    }
    if (acao_proto_.id_entidade_origem() == acao_proto_.id_entidade_destino(0)) {
      VLOG(1) << "Finalizando projetil area, entidade origem == destino.";
      estagio_ = FIM;
      return;
    }
    pos_ = entidade_origem->PosicaoAcao();
    efeito_ = 0;
    efeito_maximo_ = TAMANHO_LADO_QUADRADO * (acao_proto.geometria() == ACAO_GEO_CONE ?
        acao_proto_.distancia_quadrados() : acao_proto_.raio_quadrados());
    estagio_ = INICIAL;
  }

  ~AcaoProjetilArea() {
  }

  void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) const override {
    MudaCorProtoAlfa(acao_proto_.cor());
    gl::MatrizEscopo salva_matriz;

    switch (estagio_) {
      case INICIAL: {
      }
      break;
      case VOO: {
        gl::MatrizEscopo salva_matriz;
        MudaCorProto(acao_proto_.cor());
        gl::Translada(pos_.x(), pos_.y(), pos_.z());
        gl::Escala(acao_proto_.escala().x(), acao_proto_.escala().y(), acao_proto_.escala().z());
        DesenhaGeometriaAcao(ACAO_GEO_ESFERA);
      }
      break;
      case ATINGIU_ALVO: {
        const Posicao& pos_tabuleiro = acao_proto_.pos_tabuleiro();
        const Posicao& pos = acao_proto_.has_pos_entidade() ? acao_proto_.pos_entidade() : pos_tabuleiro;
        gl::Translada(pos.x(), pos.y(), pos.z());
        gl::Escala(efeito_, efeito_, efeito_);
        gl::DesabilitaEscopo luz(GL_LIGHTING);
        DesenhaGeometriaAcao(ACAO_GEO_ESFERA);
      }
      break;
      default: ;
    }
  }

  void AtualizaAposAtraso(int intervalo_ms) override {
    switch (estagio_) {
      case INICIAL:
        estagio_ = VOO;
        AtualizaVoo(intervalo_ms);
        break;
      case VOO:
        AtualizaVoo(intervalo_ms);
        break;
      case ATINGIU_ALVO: {
        AtualizaDispersao(intervalo_ms);
        break;
      }
      default: ;

    }
  }

  bool Finalizada() const override {
    return estagio_ == FIM;
  }

 private:
  void AtualizaDispersao(int intervalo_ms) {
    if (efeito_ == 0.0f) {
      Entidade* entidade_origem = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
      const auto& pos_origem = (entidade_origem != nullptr) && (acao_proto_.geometria() == ACAO_GEO_CONE)
        ? entidade_origem->Pos() : acao_proto_.pos_tabuleiro();
      const Posicao& pos = acao_proto_.pos_tabuleiro();
      for (const auto& id_destino : acao_proto_.id_entidade_destino()) {
        auto* ed = tabuleiro_->BuscaEntidade(id_destino);
        if (ed == nullptr) {
          continue;
        }
        Vector3 v;
        v.x = ed->Pos().x() - pos_origem.x();
        v.y = ed->Pos().y() - pos_origem.y();
        v.z = ed->Pos().z() - pos_origem.z();
        if (fabs(v.length()) > 0.001f) {
          v.normalize() /= 10.0f;
          dx_ = v.x;
          dy_ = v.y;
          dz_ = v.z;
          AtualizaDirecaoQuedaAlvo(ed);
        }
      }
      if (entidade_origem != nullptr) {
        Vector3 v;
        v.x = pos.x() - entidade_origem->X();
        v.y = pos.y() - entidade_origem->Y();
        v.z = pos.z() - entidade_origem->Z();
        if (fabs(v.length()) > 0.001f) {
          v.normalize() /= 10.0f;
          dx_ = v.x;
          dy_ = v.y;
          dz_ = v.z;
          AtualizaRotacaoZFonte(entidade_origem);
        }
      }
    }
    efeito_ += efeito_maximo_ * static_cast<float>(intervalo_ms) / (acao_proto_.duracao_s() * 1000);
    if (efeito_ > efeito_maximo_) estagio_ = FIM;
  }

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

  enum estagio_e {
    INICIAL = 0,
    VOO,
    ATINGIU_ALVO,
    FIM
  } estagio_;
  Posicao pos_;

  float efeito_maximo_;
  float efeito_;
};

// Uma acao de projetil, tipo flecha ou missil magico.
class AcaoProjetil : public Acao {
 public:
  AcaoProjetil(const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas) : Acao(acao_proto, tabuleiro, texturas) {
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
    }
  }

  void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) const override {
    if (estagio_ == ATINGIU_ALVO) {
      return;
    }
    // TODO desenha impacto.
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
  AcaoRaio(const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas) : Acao(acao_proto, tabuleiro, texturas) {
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
    Vector3 vo = PosParaVector3(pos_o);
    Vector3 vd = PosParaVector3(pos_d);
    Vector3 vovd = vd - vo;

    float dx = pos_d.x() - pos_o.x();
    float dy = pos_d.y() - pos_o.y();
    float dz = pos_d.z() - pos_o.z();
    // Roda para a direcao do alvo, deixando alinhado com o eixo X.
    gl::Translada(pos_o.x(), pos_o.y(), pos_o.z());
    float dxy;
    gl::Roda(VetorParaRotacaoGraus(dx, dy, &dxy), 0.0f,  0.0f, 1.0f);
    gl::Roda(VetorParaRotacaoGraus(dxy, dz), 0.0f, -1.0f, 0.0f);
    float tam = acao_proto_.has_distancia_quadrados() ? acao_proto_.distancia_quadrados() * TAMANHO_LADO_QUADRADO : vovd.length();
    if (!acao_proto_.has_info_textura() || !acao_proto_.efeito_area()) {
      //LOG(INFO) << "ang: " << VetorParaRotacaoGraus(dz, tam, &tam2) << ", tam2: " << tam2 << ", pos_d.z(): " << pos_d.z();
      gl::Roda(90.0f, 0.0f, 1.0f, 0.0f);
      gl::ConeSolido(0.2f, tam, 6  /*fatias*/, 1  /*tocos*/);
    } else {
      gl::Translada(tam / 2.0f, 0.0f, 0.0f);
      gl::Escala(tam, TAMANHO_LADO_QUADRADO, 0.1);
      gl::CuboUnitario();
    }
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
      AtualizaAlvo(intervalo_ms);
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
  AcaoCorpoCorpo(const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas) : Acao(acao_proto, tabuleiro, texturas) {
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
  AcaoFeiticoToque(const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas) : Acao(acao_proto, tabuleiro, texturas) {
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
        AtualizaAlvo(intervalo_ms);
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
Acao::Acao(const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas)
    : acao_proto_(acao_proto), tabuleiro_(tabuleiro), texturas_(texturas) {
  velocidade_m_ms_ = acao_proto.velocidade().inicial_m_s() / 1000.0f;
  aceleracao_m_ms_2_ = acao_proto.velocidade().aceleracao_m_s_2() / 1000.0f;
  dx_ = dy_ = dz_ = 0;
  dx_total_ = dy_total_ = dz_total_ = 0;
  disco_alvo_rad_ = 0;
  atraso_s_ = acao_proto_.atraso_s();
  if (!acao_proto_.info_textura().id().empty()) {
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_CARREGAR_TEXTURA);
    n.add_info_textura()->set_id(acao_proto_.info_textura().id());
    texturas_->TrataNotificacao(n);
  }
}

Acao::~Acao() {
  if (!acao_proto_.info_textura().id().empty()) {
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_DESCARREGAR_TEXTURA);
    n.add_info_textura()->set_id(acao_proto_.info_textura().id());
    texturas_->TrataNotificacao(n);
  }
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
  } else if (acao_proto_.has_pos_tabuleiro()) {
    return acao_proto_.pos_tabuleiro().id_cenario();
  }
  return CENARIO_INVALIDO;
}

void Acao::DesenhaComum(ParametrosDesenho* pd, std::function<void(ParametrosDesenho*)> f_desenho) const {
  if (atraso_s_ > 0 || Finalizada()) {
    return;
  }
  if (IdCenario() != pd->pos_olho().id_cenario()) {
    VLOG(2) << "Nao desenhando acao pois id cenario eh diferente";
    return;
  }
  gl::MatrizEscopo salva_matriz(GL_MODELVIEW);
  GLuint id_textura = acao_proto_.info_textura().id().empty() ? GL_INVALID_VALUE : texturas_->Textura(acao_proto_.info_textura().id());
  if (id_textura != GL_INVALID_VALUE) {
    gl::Habilita(GL_TEXTURE_2D);
    gl::LigacaoComTextura(GL_TEXTURE_2D, id_textura);
  }
  std::unique_ptr<gl::DesabilitaEscopo> luz_escopo(acao_proto_.ignora_luz() ? new gl::DesabilitaEscopo(GL_LIGHTING): nullptr);
  std::unique_ptr<gl::DesabilitaEscopo> cull_escopo(acao_proto_.dois_lados() ? new gl::DesabilitaEscopo(GL_CULL_FACE): nullptr);
  f_desenho(pd);
  gl::Desabilita(GL_TEXTURE_2D);
}

void Acao::Desenha(ParametrosDesenho* pd) const {
  if (acao_proto_.cor().a() < 1.0f) {
    return;
  }
  DesenhaComum(pd, std::bind(&Acao::DesenhaSeNaoFinalizada, this, _1));
}

void Acao::DesenhaTranslucido(ParametrosDesenho* pd) const {
  if (acao_proto_.cor().a() >= 1.0f) {
    return;
  }
  DesenhaComum(pd, std::bind(&Acao::DesenhaTranslucidoSeNaoFinalizada, this, _1));
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
  if (acao_proto_.consequencia() == TC_DESLOCA_ALVO) {
    if (!acao_proto_.bem_sucedida()) {
      VLOG(1) << "Finalizando alvo, nao foi bem sucedida.";
      dx_total_ = dy_total_ = dz_total_ = 0;
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
    const float DURACAO_ATUALIZACAO_ALVO_MS = 100.0f;
    // dt representa a fracao do arco ate 90 graus que o alvo andou. Os outros 90 sao da volta.
    const float dt = std::min(((intervalo_ms * 2.0f) / DURACAO_ATUALIZACAO_ALVO_MS), 1.0f);
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
    // Nao terminou de atualizar.
    return true;
  } else if (acao_proto_.consequencia() == TC_INFLAMA_ALVO) {
    if (!acao_proto_.bem_sucedida()) {
      VLOG(1) << "Finalizando alvo, nao foi bem sucedida.";
      dx_total_ = dy_total_ = dz_total_ = 0;
      return false;
    }
    for (auto id : acao_proto_.id_entidade_destino()) {
      entidade_destino = tabuleiro_->BuscaEntidade(id);
      if (entidade_destino == nullptr) {
        continue;
      }
      EntidadeProto parcial;
      parcial.set_fumegando(true);
      entidade_destino->AtualizaParcial(parcial);
    }
    return false;
  } else if (acao_proto_.consequencia() == TC_AGARRA_ALVO) {
    Entidade* entidade_origem = EntidadeOrigem();
    Entidade* entidade_destino = EntidadeDestino();
    if (entidade_origem == nullptr || entidade_destino == nullptr) {
      VLOG(1) << "Finalizando alvo, origem ou destino não existe.";
      return false;
    }
    if (AgarradoA(entidade_origem->Id(), entidade_destino->Proto())) {
      VLOG(1) << "Finalizando alvo, esta agarrando.";
      return false;
    }
    if (fabs(dx_total_) >= fabs(dx_) && fabs(dy_total_) >= fabs(dy_) && fabs(dz_total_) >= fabs(dz_)) {
      if (acao_proto_.bem_sucedida()) disco_alvo_rad_ = 1.0f;
      if (disco_alvo_rad_ > 0.0f) {
        VLOG(1) << "Finalizando alvo apos movimento.";
        if (acao_proto_.bem_sucedida()) {
          {
            EntidadeProto parcial;
            *parcial.mutable_agarrado_a() = entidade_origem->Proto().agarrado_a();
            parcial.add_agarrado_a(entidade_destino->Id());
            entidade_origem->AtualizaParcial(parcial);
          }
          {
            EntidadeProto parcial;
            *parcial.mutable_agarrado_a() = entidade_destino->Proto().agarrado_a();
            parcial.add_agarrado_a(entidade_origem->Id());
            entidade_destino->AtualizaParcial(parcial);
          }
        }
        return false;
      } else {
        dx_ = -dx_; dx_total_ = 0;
        dy_ = -dy_; dy_total_ = 0;
        dz_ = -dz_; dz_total_ = 0;
        disco_alvo_rad_ = 1.0f;
      }
    }

    const float DURACAO_ATUALIZACAO_ALVO_MS = 100.0f;
    const float fracao = intervalo_ms / DURACAO_ATUALIZACAO_ALVO_MS;
    if (fracao < 0.00001f) return false;
    float dx_alvo = dx_ * fracao;
    float dy_alvo = dy_ * fracao;
    float dz_alvo = dz_ * fracao;
    dx_total_ += dx_alvo;
    dy_total_ += dy_alvo;
    dz_total_ += dz_alvo;
    VLOG(2) << "dx_alvo " << dx_alvo << ", dy_alvo " << dy_alvo << ", dz_alvo " << dz_alvo << ", fracao: " << fracao;
    MoveDeltaRespeitandoChao(dx_alvo, dy_alvo, dz_alvo, *tabuleiro_, entidade_destino);
    return true;
  }
  return false;
}

// static
Posicao Acao::AjustaPonto(
    const Posicao& pos_ponto, float multiplicador_tamanho, const Posicao& pos_origem, const AcaoProto& acao_proto) {
  if (multiplicador_tamanho < 1.0f) {
    // Muito pequeno para se preocupar.
    return pos_ponto;
  }
  float correcao = multiplicador_tamanho * TAMANHO_LADO_QUADRADO_2;
  VLOG(1) << "Ajustando ponto: " << pos_ponto.ShortDebugString()
          << ", origem: " << pos_origem.ShortDebugString()
          << ", pos_tabuleiro: " << acao_proto.pos_tabuleiro().ShortDebugString()
          << ", correcao maxima: " << correcao;

  switch (acao_proto.tipo()) {
    case ACAO_DISPERSAO:
      switch (acao_proto.geometria()) {
        case ACAO_GEO_ESFERA:
        case ACAO_GEO_CILINDRO:
        case ACAO_GEO_CONE: {
          Vector3 v_objeto(
              PosParaVector3(pos_ponto) -
              PosParaVector3(
                  acao_proto.geometria() == ACAO_GEO_CONE ? pos_origem : acao_proto.pos_tabuleiro()));
          const float v_objeto_length = v_objeto.length();
          if (v_objeto_length < correcao) {
            correcao = v_objeto.length();
          }
          if (fabs(v_objeto_length) > 0.01) {
            v_objeto.normalize();
            v_objeto *= correcao;
          }
          Posicao pos_corrigida(pos_ponto);
          pos_corrigida.set_x(pos_ponto.x() - v_objeto.x);
          pos_corrigida.set_y(pos_ponto.y() - v_objeto.y);
          pos_corrigida.set_z(pos_ponto.z() - v_objeto.z);
          return pos_corrigida;
        }
        break;
        default:
          return pos_ponto;
      }
      break;
    case ACAO_RAIO: {
      if (!acao_proto.efeito_area()) {
        break;;
      }
      // Para achar a direcao que aponta para o raio, temos:
      //   Raio = R
      //      |\    .
      //      | \   .
      //   H1 |__\  Objeto = Ob
      //      |  /  .
      //      | /   .
      //      |/    .
      //   Origem = Or
      // O vetor de interesse é o traço do meio, chamemos de Q.
      Vector3 v_origem(PosParaVector3(pos_origem));
      Vector3 v_raio(PosParaVector3(acao_proto.pos_tabuleiro()) - v_origem);
      Vector3 v_objeto(PosParaVector3(pos_ponto) - v_origem);

      // Ob + Q = H1, portanto: Q = H1 - Ob.
      // Para achar H1, usa-se seu tamanho: cos(a) = |H1| / |Ob|, portanto:
      // |H1| = cos(a) * |Ob|.
      // Para encontrar cos(a), R dot Ob = |R|.|Ob|.cos(a), portanto:
      // cos(a) = R dot Ob / (|R|.|Ob|)
      const float v_raio_length = v_raio.length();
      const float v_objeto_length = v_objeto.length();
      if (fabs(v_objeto_length) < 0.01) {
        VLOG(1) << "Diferenca muito pequena, retornando proprio ponto";
        return pos_ponto;
      }
      const float cos_a = (v_raio_length < 0.01 || v_objeto_length < 0.01)
          ? 1.0f : v_raio.dot(v_objeto) / (v_raio_length * v_objeto_length);
      if (fabs(cos_a) > 0.99) {
        VLOG(1) << "cos(a) ~= 1.0f";
        // Objetos com mesma direcao. So aponta na direcao contraria.
        if (correcao > v_objeto_length) {
          correcao = v_objeto_length;
        }
        v_objeto.normalize();
        v_objeto *= -correcao;
        Posicao pos_corrigida;
        pos_corrigida.set_x(pos_ponto.x() + v_objeto.x);
        pos_corrigida.set_y(pos_ponto.y() + v_objeto.y);
        pos_corrigida.set_z(pos_ponto.z() + v_objeto.z);
        VLOG(1) << "Retornando: " << pos_corrigida.ShortDebugString();
        return pos_corrigida;
      }
      VLOG(1) << "cos(a) = " << cos_a;
      float h1 = cos_a * v_objeto_length;
      // H1 = norm(R) * |H1|.
      Vector3 v_h1 = v_raio.normalize() * h1;
      // Ob + Q = H1, portanto: Q = H1 - Ob.
      Vector3 v_q = v_h1 - v_objeto;
      const float v_q_length = v_q.length();
      if (correcao > v_q_length) {
        correcao = v_q_length;
      }
      if (v_q_length > 0.01f) {
        v_q.normalize();
      }
      v_q *= correcao;
      Posicao pos_corrigida(pos_ponto);
      pos_corrigida.set_x(pos_ponto.x() + v_q.x);
      pos_corrigida.set_y(pos_ponto.y() + v_q.y);
      pos_corrigida.set_z(pos_ponto.z() + v_q.z);
      VLOG(1) << "Retornando: " << pos_corrigida.ShortDebugString();
      return pos_corrigida;
    }
    break;
    default: ;
  }
  VLOG(1) << "Retornando proprio ponto.";
  return pos_ponto;
}

Entidade* Acao::EntidadeOrigem() {
  return tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
}

Entidade* Acao::EntidadeDestino() {
  if (acao_proto_.id_entidade_destino_size() != 1) return nullptr;
  return tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_destino(0));
}


// static
bool Acao::PontoAfetadoPorAcao(const Posicao& pos_ponto, const Posicao& pos_origem, const AcaoProto& acao_proto, bool ponto_eh_origem) {
  switch (acao_proto.tipo()) {
    case ACAO_DISPERSAO:
      switch (acao_proto.geometria()) {
        case ACAO_GEO_ESFERA: {
          const float dq =
            DistanciaQuadrado(
                pos_ponto, acao_proto.pos_tabuleiro());
          const float distancia_maxima = powf(acao_proto.raio_quadrados() * TAMANHO_LADO_QUADRADO, 2);
          VLOG(1) << "Distancia quadrado: " << dq << ", maximo: " << distancia_maxima;
          return dq <= distancia_maxima;
        }
        case ACAO_GEO_CILINDRO: {
          // Corte rapido por z. cilindro tem duas vezes o tamanho do raio.
          const float altura = acao_proto.raio_quadrados() * TAMANHO_LADO_QUADRADO * 2;
          const auto& pos_tabuleiro = acao_proto.pos_tabuleiro();
          if (pos_ponto.z() < pos_tabuleiro.z() ||
              pos_ponto.z() > (pos_tabuleiro.z() + altura)) {
            return false;
          }
          const float raio = acao_proto.raio_quadrados() * TAMANHO_LADO_QUADRADO;
          const float diff_x = pos_ponto.x() - pos_tabuleiro.x();
          const float diff_y = pos_ponto.y() - pos_tabuleiro.y();
          if (fabs(diff_x) > raio || fabs(diff_y) > raio) {
            return false;
          }
          // A entidade está dentro do cubo. Agora ve se esta dentro do circulo.
          float distancia_quadrado = powf(diff_x, 2) * pow(diff_y, 2);
          return distancia_quadrado <= raio * raio;
        }
        case ACAO_GEO_CONE: {
          if (ponto_eh_origem) {
            return false;
          }
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
      if (!acao_proto.efeito_area() || ponto_eh_origem) {
        return false;
      }
      Vector3 v_origem(PosParaVector3(pos_origem));
      Vector3 v_objeto(PosParaVector3(pos_ponto) - v_origem);
      const float v_objeto_length = v_objeto.length();
      if (v_objeto_length < 0.01f) {
        VLOG(1) << "Diferença muito pequena, retornando true";
        return true;
      }
      if (v_objeto_length > (acao_proto.distancia_quadrados() * TAMANHO_LADO_QUADRADO)) {
        VLOG(1) << "Objeto fora de alcance, retornando false";
        return false;
      }
      Vector3 v_raio(PosParaVector3(acao_proto.pos_tabuleiro()) - v_origem);
      const float v_raio_length = v_raio.length();
      if (v_raio_length < 0.01f) {
        LOG(WARNING) << "Raio sem direcao: " << acao_proto.ShortDebugString();
        return false;
      }
      v_raio.normalize();
      v_objeto.normalize();
      // Angulo entre os vetores.
      const float angulo_rad = acosf(v_raio.dot(v_objeto));
      const float angulo = angulo_rad * RAD_PARA_GRAUS;
      if (angulo > 90.0f) {
        // Objeto atras do raio.
        VLOG(1) << "angulo maior que 90.0f: " << angulo << ", retornando false";
        return false;
      }
      const float distancia_para_raio = sinf(angulo_rad) * v_objeto_length;
      VLOG(1) << "angulo: " << angulo << ", distancia_para_raio: " << distancia_para_raio;
      // 10% de tolerancia, porque criaturas grandes tende a errar quando o clique nao eh no centro.
      return distancia_para_raio < (TAMANHO_LADO_QUADRADO_2 * 1.10f);
    }
    break;
    default:
      return false;
  }
  return false;
}

Acao* NovaAcao(const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas) {
  VLOG(1) << "NovaAcao: " << acao_proto.DebugString();
  switch (acao_proto.tipo()) {
    case ACAO_SINALIZACAO:
      return new AcaoSinalizacao(acao_proto, tabuleiro, texturas);
    case ACAO_PROJETIL:
      return new AcaoProjetil(acao_proto, tabuleiro, texturas);
    case ACAO_DISPERSAO:
      return new AcaoDispersao(acao_proto, tabuleiro, texturas);
    case ACAO_DELTA_PONTOS_VIDA:
      return new AcaoDeltaPontosVida(acao_proto, tabuleiro, texturas);
    case ACAO_RAIO:
      return new AcaoRaio(acao_proto, tabuleiro, texturas);
    case ACAO_CORPO_A_CORPO:
      return new AcaoCorpoCorpo(acao_proto, tabuleiro, texturas);
    case ACAO_FEITICO_TOQUE:
      return new AcaoFeiticoToque(acao_proto, tabuleiro, texturas);
    case ACAO_AGARRAR:
      return new AcaoAgarrar(acao_proto, tabuleiro, texturas);
    case ACAO_POCAO:
      return new AcaoPocao(acao_proto, tabuleiro, texturas);
    case ACAO_PROJETIL_AREA:
      return new AcaoProjetilArea(acao_proto, tabuleiro, texturas);
    default:
      LOG(ERROR) << "Acao invalida: " << acao_proto.ShortDebugString();
      return nullptr;
  }
}

}  // namespace ent
