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
#include "goog/stringprintf.h"
#include "matrix/vectors.h"
#include "m3d/m3d.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"
#include "log/log.h"
#include "som/som.h"
#include "tex/texturas.h"

namespace ent {

namespace {

using std::placeholders::_1;
using google::protobuf::StringPrintf;

void MudaCorProto(const Cor& cor) {
  const GLfloat corgl[3] = { cor.r(), cor.g(), cor.b() };
  MudaCor(corgl);
}
void MudaCorProtoAlfa(const Cor& cor) {
  const GLfloat corgl[4] = { cor.r(), cor.g(), cor.b(), cor.a() };
  MudaCorAlfa(corgl);
}

// Util para buscar id do primeiro destino.
bool TemAlgumDestino(const AcaoProto& acao_proto) {
  return acao_proto.por_entidade().empty() || !acao_proto.por_entidade(0).has_id() ? false : true;
}

// Retorna id do primeiro destino.
unsigned int IdPrimeiroDestino(const AcaoProto& acao_proto) {
  return TemAlgumDestino(acao_proto) ? acao_proto.por_entidade(0).id() : Entidade::IdInvalido;
}

// Busca o primeiro destino.
Entidade* BuscaPrimeiraEntidadeDestino(const AcaoProto& acao_proto, Tabuleiro* tabuleiro) {
  return TemAlgumDestino(acao_proto) ? tabuleiro->BuscaEntidade(IdPrimeiroDestino(acao_proto)) : nullptr;
}

bool TemTextoAcao(const AcaoProto& acao_proto) {
  if (TemAlgumDestino(acao_proto) && acao_proto.por_entidade(0).has_texto()) {
    return true;
  }
  return acao_proto.has_texto();
}

bool TemDeltaAcao(const AcaoProto& acao_proto) {
  if (TemAlgumDestino(acao_proto) && acao_proto.por_entidade(0).has_delta()) {
    return true;
  }
  return acao_proto.has_delta_pontos_vida();
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
  AcaoSinalizacao(const Tabelas& tabelas, const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas, const m3d::Modelos3d* modelos3d, ntf::CentralNotificacoes* central) :
      Acao(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central), estado_(TAMANHO_MAXIMO) {
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

  void AtualizaAposAtraso(int intervalo_ms, const Olho& camera) override {
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
  AcaoPocao(const Tabelas& tabelas, const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas, const m3d::Modelos3d* modelos3d, ntf::CentralNotificacoes* central) :
      Acao(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central) {
    bolhas_.emplace_back(gl::VboEsferaSolida(0.15f, 6, 6));
    bolhas_.emplace_back(gl::VboEsferaSolida(0.1f, 6, 6));
    bolhas_.emplace_back(gl::VboEsferaSolida(0.2f, 6, 6));
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

  void AtualizaAposAtraso(int intervalo_ms, const Olho& camera) override {
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
  AcaoAgarrar(const Tabelas& tabelas, const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas, const m3d::Modelos3d* modelos3d, ntf::CentralNotificacoes* central) :
      Acao(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central) {
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

  void AtualizaAposAtraso(int intervalo_ms, const Olho& camera) override {
    finalizada_ = !AtualizaAlvo(intervalo_ms);
  }

  bool Finalizada() const override {
    return finalizada_;
  }

 private:
  bool finalizada_ = false;
};

// Antigamente: sobe um numero verde ou vermelho de acordo com o dano causado.
// Hoje serve para texto tb.
// Renomear para AcaoTexto.
// TODO fonte maior?
class AcaoDeltaPontosVida : public Acao {
 public:
  AcaoDeltaPontosVida(const Tabelas& tabelas, const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas, const m3d::Modelos3d* modelos3d, ntf::CentralNotificacoes* central)
      : Acao(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central) {
    Entidade* entidade_destino = BuscaPrimeiraEntidadeDestino(acao_proto, tabuleiro);
    if (!acao_proto_.has_pos_entidade()) {
      if (entidade_destino == nullptr) {
        faltam_ms_ = 0;
        VLOG(1) << "Finalizando delta_pontos_vida precisa de entidade destino: " << acao_proto_.ShortDebugString();
        return;
      }
      if (tabuleiro_ == nullptr) {
        faltam_ms_ = 0;
        LOG(ERROR) << "Tabuleiro é nullptr!";
        return;
      }
      const auto* entidade_primeira_pessoa = tabuleiro_->EntidadePrimeiraPessoa();
      if (entidade_primeira_pessoa == nullptr || entidade_primeira_pessoa->Id() != entidade_destino->Id()) {
        pos_ = entidade_destino->Pos();
        // ficar mais alto.
        pos_.set_z(entidade_destino->ZOlho() + entidade_destino->AlturaOlho());
        VLOG(1) << "Acao usando entidade destino " << pos_.ShortDebugString();
      } else {
        // Entidade destino é a mesma da primeira pessoa. Vamos colocar a acao
        // no meio da tela.
        int largura, altura;
        std::tie(largura, altura) = tabuleiro_->LarguraAlturaViewport();
        pos_2d_.set_x(largura / 2);
        pos_2d_.set_y(altura / 2);
        max_delta_y_ = altura / 10;
        VLOG(1) << "Acao posicao primeira pessoa";
      }
    } else {
      const auto* entidade_primeira_pessoa = tabuleiro_->EntidadePrimeiraPessoa();
      if (entidade_primeira_pessoa == nullptr || entidade_destino == nullptr ||
          entidade_primeira_pessoa->Id() != entidade_destino->Id()) {
        pos_ = acao_proto_.pos_entidade();
        VLOG(1) << "Acao usando posicao entidade " << pos_.ShortDebugString();
      } else {
        // Entidade destino é a mesma da primeira pessoa. Vamos colocar a acao
        // no meio da tela.
        int largura, altura;
        std::tie(largura, altura) = tabuleiro_->LarguraAlturaViewport();
        pos_2d_.set_x(largura / 2);
        pos_2d_.set_y(altura / 2);
        max_delta_y_ = altura / 10;
        VLOG(1) << "Acao posicao primeira pessoa";
      }
    }
    faltam_ms_ = 0;
    // Monta a string de delta.
    if (TemDeltaAcao(acao_proto_)) {
      if (TemTextoAcao(acao_proto_)) {
        string_texto_ = StringPrintf("\n%s", TextoAcao(acao_proto_).c_str());
      }
      delta_acao_ = DeltaAcao(acao_proto_);
      const int delta_abs = abs(delta_acao_);
      if (delta_abs > 10000) {
        faltam_ms_ = 0;
        VLOG(1) << "Finalizando delta_pontos_vida, delta muito grande.";
        return;
      }
      string_delta_ = delta_abs != 0
        ? StringPrintf("%d", delta_abs)
        : (string_texto_.empty() ? "X" : "");
    } else if (TemTextoAcao(acao_proto_)) {
      string_texto_ = TextoAcao(acao_proto_);
    } else {
      faltam_ms_ = 0;
      VLOG(1) << "Finalizando delta_pontos_vida, proto nao tem delta nem texto.";
      return;
    }
    VLOG(2) << "String delta: " << string_delta_ << ", string texto: " << string_texto_;
    num_linhas_ = 1 + std::count(string_texto_.begin(), string_texto_.end(), '\n');
    duracao_total_ms_ = std::max<int>(5000, acao_proto_.has_duracao_s() ? acao_proto_.duracao_s() * 1000 : DURACAO_UMA_LINHA_MS * num_linhas_);
    faltam_ms_ = duracao_total_ms_;
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
    } else if (delta_acao_ > 0) {
      MudaCorAplicandoNevoa(COR_VERDE, pd);
    } else if (delta_acao_ == 0) {
      MudaCorAplicandoNevoa(COR_BRANCA, pd);
    } else {
      MudaCorAplicandoNevoa(COR_VERMELHA, pd);
    }
    if (!string_delta_.empty()) {
      DesenhaStringDelta();
    }

    if (!string_texto_.empty()) {
      if (acao_proto_.has_cor()) {
        const float cor[] = { acao_proto_.cor().r(), acao_proto_.cor().g(), acao_proto_.cor().b() };
        MudaCorAplicandoNevoa(cor, pd);
      } else {
        MudaCorAplicandoNevoa(COR_AMARELA, pd);
      }
      DesenhaStringTexto();
    }
  }

  void AtualizaAposAtraso(int intervalo_ms, const Olho& camera) override {
    if (faltam_ms_ == duracao_total_ms_) {
      // Primeiro frame. Apenas posiciona na posicao inicial. Importante pos UI para nao pular o efeito.
      --faltam_ms_;
    } else {
      if (pos_2d_.has_y()) {
        pos_2d_.set_y(pos_2d_.y() + (static_cast<float>(intervalo_ms) * max_delta_y_) / duracao_total_ms_);
      } else {
        pos_.set_z(pos_.z() + (static_cast<float>(intervalo_ms) * VELOCIDADE_ROLAGEM_M_POR_MS));
      }
      faltam_ms_ -= intervalo_ms;
    }
    if (faltam_ms_ <= 0) {
      VLOG(1) << "Finalizando delta_pontos_vida, MAX_ATUALIZACOES alcancado.";
    }
  }

  bool Finalizada() const override {
    return faltam_ms_ <= 0;
  }

 private:
  void DesenhaStringDelta() const {
    gl::DesabilitaEscopo salva_nevoa(GL_FOG);
    gl::DesabilitaEscopo salva_oclusao(gl::OclusaoLigada, gl::Oclusao);
    if (pos_2d_.has_x()) {
      if (gl::PosicaoRasterAbsoluta(pos_2d_.x(), pos_2d_.y())) {
        gl::DesenhaString(StringSemUtf8(string_delta_));
      }
    } else {
      if (gl::PosicaoRaster(0.0f, 0.0f, 0.0f)) {
        gl::DesenhaString(StringSemUtf8(string_delta_));
      }
    }
  }

  void DesenhaStringTexto() const {
    gl::DesabilitaEscopo salva_nevoa(GL_FOG);
    gl::DesabilitaEscopo salva_oclusao(gl::OclusaoLigada, gl::Oclusao);
    if (pos_2d_.has_x()) {
      if (gl::PosicaoRasterAbsoluta(pos_2d_.x(), pos_2d_.y())) {
        gl::DesenhaString(StringSemUtf8(string_texto_));
      }
    } else {
      if (gl::PosicaoRaster(0.0f, 0.0f, 0.0f)) {
        gl::DesenhaString(StringSemUtf8(string_texto_), /*inverte_vertical=*/true);
      }
    }
  }

  constexpr static int DURACAO_UMA_LINHA_MS = 2000;
  constexpr static float VELOCIDADE_ROLAGEM_M_POR_MS = 0.0002f;

  int delta_acao_ = 0;
  std::string string_texto_;
  std::string string_delta_;
  int duracao_total_ms_ = 0;
  Posicao pos_;
  Posicao pos_2d_;
  int faltam_ms_;
  int max_delta_y_ = 0;
  int num_linhas_ = 0;
};

// Acao de dispersao, estilo bola de fogo.
class AcaoDispersao : public Acao {
 public:
  AcaoDispersao(
      const Tabelas& tabelas, const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas, const m3d::Modelos3d* modelos3d,
      ntf::CentralNotificacoes* central)
      : Acao(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central) {
    efeito_ = 0;
    efeito_maximo_ = TAMANHO_LADO_QUADRADO *
        (acao_proto.geometria() == ACAO_GEO_CONE ? acao_proto_.distancia_quadrados() : acao_proto_.raio_quadrados());
    duracao_s_ = acao_proto.has_duracao_s() ? acao_proto.duracao_s() : 0.35f;
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
    DesenhaGeometriaAcao();
  }

  void AtualizaAposAtraso(int intervalo_ms, const Olho& camera) override {
    if (efeito_ == 0.0f) {
      TocaSomSucessoOuFracasso(camera);
      Entidade* entidade_origem = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
      const auto& pos_origem = (entidade_origem != nullptr) && (acao_proto_.geometria() == ACAO_GEO_CONE)
          ? entidade_origem->Pos() : acao_proto_.pos_tabuleiro();
      const Posicao& pos = acao_proto_.pos_tabuleiro();
      for (const auto& por_entidade : acao_proto_.por_entidade()) {
        const auto id_destino = por_entidade.id();
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
    efeito_ += efeito_maximo_ * static_cast<float>(intervalo_ms) / (duracao_s_ * 1000);
    if (Finalizada()) {
      AtualizaAlvo(intervalo_ms);
    }
  }

  bool Finalizada() const override {
    return efeito_ > efeito_maximo_;
  }

 private:
  float efeito_maximo_ = 0.0f;
  float efeito_ = 0.0f;
  float duracao_s_ = 0.0f;
};

// Acao de dispersao, estilo bola de fogo.
class AcaoProjetilArea: public Acao {
 public:
  AcaoProjetilArea(const Tabelas& tabelas, const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas, const m3d::Modelos3d* modelos3d, ntf::CentralNotificacoes* central)
      : Acao(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central) {
    auto* entidade_origem = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    if (entidade_origem == nullptr) {
      VLOG(1) << "Finalizando projetil area, precisa de entidade origem.";
      estagio_ = FIM;
      return;
    }
    pos_ = entidade_origem->PosicaoAcao();
    pos_impacto_ = acao_proto.pos_tabuleiro();
    efeito_q_ = 0;
    efeito_maximo_q_ = TAMANHO_LADO_QUADRADO * acao_proto_.raio_quadrados();
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
        DesenhaGeometriaAcao();
      }
      break;
      case ATINGIU_ALVO: {
        const Posicao& pos_tabuleiro = acao_proto_.pos_tabuleiro();
        const Posicao& pos = acao_proto_.has_pos_entidade() ? acao_proto_.pos_entidade() : pos_tabuleiro;
        gl::Translada(pos.x(), pos.y(), pos.z());
        gl::Escala(efeito_q_, efeito_q_, efeito_q_);
        gl::DesabilitaEscopo luz(GL_LIGHTING);
        DesenhaGeometriaAcao();
      }
      break;
      default: ;
    }
  }

  void AtualizaAposAtraso(int intervalo_ms, const Olho& camera) override {
    switch (estagio_) {
      case INICIAL:
        AtualizaInicial(intervalo_ms);
        break;
      case VOO:
        AtualizaVoo(intervalo_ms, camera);
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
  void AtualizaInicial(int intervalo_ms) {
    VLOG(1) << "Atualizando inicial";
    Entidade* entidade_origem = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    const auto& pos_origem = acao_proto_.pos_tabuleiro();
    for (const auto& por_entidade : acao_proto_.por_entidade()) {
      const auto id_destino = por_entidade.id();
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
      v.x = pos_impacto_.x() - entidade_origem->X();
      v.y = pos_impacto_.y() - entidade_origem->Y();
      v.z = pos_impacto_.z() - entidade_origem->Z();
      if (fabs(v.length()) > 0.001f) {
        v.normalize() /= 10.0f;
        dx_ = v.x;
        dy_ = v.y;
        dz_ = v.z;
        AtualizaRotacaoZFonte(entidade_origem);
      }
    }
    estagio_ = VOO;
  }

  void AtualizaDispersao(int intervalo_ms) {
    efeito_q_ += std::max(0.1f, efeito_maximo_q_ * static_cast<float>(intervalo_ms) / (acao_proto_.duracao_s() * 1000));
    if (efeito_q_ > efeito_maximo_q_) estagio_ = FIM;
    VLOG(1) << "Atualizando dispersao: efeito_q: " << efeito_q_ << ", maximo: " << efeito_maximo_q_;
  }

  void AtualizaVoo(int intervalo_ms, const Olho& camera) {
    VLOG(1) << "Atualizando voo";
    // Recalcula vetor.
    dx_ = pos_impacto_.x() - pos_.x();
    dy_ = pos_impacto_.y() - pos_.y();
    dz_ = pos_impacto_.z() - pos_.z();
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
    pos_.set_x(ArrumaSePassou(xa, xa + v.x, pos_impacto_.x()));
    pos_.set_y(ArrumaSePassou(ya, ya + v.y, pos_impacto_.y()));
    pos_.set_z(ArrumaSePassou(za, za + v.z, pos_impacto_.z()));
    // Deslocamento do alvo.
    vn /= 2.0f;  // meio metro de deslocamento.
    dx_ = vn.x;
    dy_ = vn.y;
    dz_ = vn.z;
    if (pos_.x() == pos_impacto_.x() &&
        pos_.y() == pos_impacto_.y() &&
        pos_.z() == pos_impacto_.z()) {
      VLOG(1) << "Projetil atingiu alvo.";
      estagio_ = ATINGIU_ALVO;
      TocaSomSucessoOuFracasso(camera);
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
  Posicao pos_impacto_;

  // Tamanho do efeito em quadrados.
  float efeito_maximo_q_;
  float efeito_q_;
};

// Uma acao de projetil, tipo flecha ou missil magico.
class AcaoProjetil : public Acao {
 public:
  AcaoProjetil(const Tabelas& tabelas, const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas, const m3d::Modelos3d* modelos3d, ntf::CentralNotificacoes* central)
      : Acao(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central) {
    estagio_ = INICIAL;
    auto* entidade_origem = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    if (entidade_origem == nullptr) {
      VLOG(1) << "Finalizando projetil, precisa de entidade origem.";
      estagio_ = FIM;
      return;
    }
    if (acao_proto_.por_entidade().empty()) {
      VLOG(1) << "Finalizando projetil, nao ha entidade destino.";
      estagio_ = FIM;
      return;
    }
    if (acao_proto_.id_entidade_origem() == acao_proto_.por_entidade(0).id()) {
      VLOG(1) << "Finalizando projetil, entidade origem == destino.";
      estagio_ = FIM;
      return;
    }
    pos_ = entidade_origem->PosicaoAcao();
  }

  void AtualizaAposAtraso(int intervalo_ms, const Olho& camera) override {
    if (estagio_ == INICIAL) {
      estagio_ = VOO;
      AtualizaVoo(intervalo_ms);
      // Atualiza depois, para ter dx, e dy.
      AtualizaRotacaoZFonte(tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem()));
      AtualizaLuzOrigem(intervalo_ms);
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
    DesenhaGeometriaAcao();
  }

  bool Finalizada() const override {
    return estagio_ == FIM;
  }

 private:
  void AtualizaVoo(int intervalo_ms) {
    Entidade* entidade_destino = BuscaPrimeiraEntidadeDestino(acao_proto_, tabuleiro_);
    if (entidade_destino == nullptr) {
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
  AcaoRaio(const Tabelas& tabelas, const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas, const m3d::Modelos3d* modelos3d, ntf::CentralNotificacoes* central)
      : Acao(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central) {
    duracao_ = acao_proto.has_duracao_s() ? acao_proto.duracao_s() : 0.5f;
    if (!acao_proto_.has_id_entidade_origem()) {
      duracao_ = 0.0f;
      VLOG(1) << "Acao raio requer id origem.";
      return;
    }
    if (acao_proto_.por_entidade().empty() && !acao_proto_.has_pos_tabuleiro()) {
      duracao_ = 0.0f;
      VLOG(1) << "Acao raio requer id destino ou posicao destino.";
      return;
    }

    if (!acao_proto_.efeito_area() &&
        !acao_proto_.por_entidade().empty() && acao_proto_.id_entidade_origem() == acao_proto_.por_entidade(0).id()) {
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
    if (!acao_proto_.efeito_area() && !acao_proto_.por_entidade().empty()) {
      auto* ed = tabuleiro_->BuscaEntidade(acao_proto_.por_entidade(0).id());
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

  void AtualizaAposAtraso(int intervalo_ms, const Olho& camera) override {
    auto* eo = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    if (eo == nullptr) {
      VLOG(1) << "Terminando acao pois origem nao existe mais.";
      duracao_ = 0.0f;
      return;
    }
    if (duracao_ == acao_proto_.duracao_s()) {
      for (const auto& por_entidade : acao_proto_.por_entidade()) {
        const auto id_destino = por_entidade.id();
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
  AcaoCorpoCorpo(
      const Tabelas& tabelas, const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas, const m3d::Modelos3d* modelos3d,
      ntf::CentralNotificacoes* central)
      : Acao(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central) {
    if (!acao_proto_.has_id_entidade_origem()) {
      VLOG(1) << "Acao corpo a corpo requer id origem.";
      finalizado_ = true;
      return;
    }
    if (!TemAlgumDestino(acao_proto_)) {
      VLOG(1) << "Acao corpo a corpo requer id destino.";
      finalizado_ = true;
      return;
    }
    if (acao_proto_.id_entidade_origem() == IdPrimeiroDestino(acao_proto_)) {
      VLOG(1) << "Acao corpo a corpo requer origem e destino diferentes.";
      finalizado_ = true;
      return;
    }
    auto* eo = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    const DadosAtaque* da = eo != nullptr && acao_proto_.has_grupo_ataque() && acao_proto_.has_indice_ataque()
        ? eo->DadoAtaque(acao_proto_.grupo_ataque(), acao_proto_.indice_ataque())
        : eo->DadoCorrente(/*ignora_ataques_na_rodada=*/true);
    if (da != nullptr) {
      dados_ataque_ = *da;
    }
    duracao_ms_ = acao_proto.has_duracao_s() ? acao_proto.duracao_s() * 1000 : DURACAO_MS;

    AtualizaDeltas();
    finalizado_ = false;
  }

  void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) const override {
    auto* eo = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    if (eo == nullptr) {
      return;
    }
    const auto& arma_tabelada = tabelas_.Arma(dados_ataque_.id_arma());
    if (arma_tabelada.info_modelo_3d().has_id()) {
      Matrix4 matriz;
      matriz.rotateY(rotacao_graus_);
      matriz.translate(translacao_m_, 0.0f, 0.0f);
      if (dados_ataque_.empunhadura() != EA_MAO_RUIM) {
        eo->AtualizaMatrizAcaoPrincipal(matriz);
      } else {
        eo->AtualizaMatrizAcaoSecundaria(matriz);
      }
    } else {
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
  }

  void AtualizaAposAtraso(int intervalo_ms, const Olho& camera) override {
    auto* eo = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    if (eo == nullptr) {
      VLOG(1) << "Terminando acao corpo a corpo: origem nao existe mais.";
      finalizado_ = true;
      return;
    }
    if (progresso_ == 0.0f) {
      AtualizaRotacaoZFonte(eo);
    }
    bool tocou_som = (progresso_ >= 0.5f);

    // TODO desenhar o impacto.
    // Os parametros iniciais sao mantidos, so a rotacao do corte eh alterada.
    if (progresso_ < 1.0f) {
      float incremento = std::max(0.01f, static_cast<float>(intervalo_ms) / duracao_ms_);
      progresso_ = std::min<float>(1.0f, progresso_ + incremento);
    }

    const auto& arma_tabelada = tabelas_.Arma(dados_ataque_.id_arma());
    if (arma_tabelada.tipo_dano().size() == 1 && arma_tabelada.tipo_dano(0) == TD_PERFURANTE) {
      rotacao_graus_ = 90.0f;
      if (progresso_ <= 0.5f) {
        translacao_m_ = progresso_ * TAMANHO_LADO_QUADRADO;
      } else {
        translacao_m_ = (1.0f - progresso_) * TAMANHO_LADO_QUADRADO;
      }
    } else {
      rotacao_graus_ = progresso_ * 180.0f;
      translacao_m_ = 0.0f;
    }

    bool terminou_alvo = false;
    if (progresso_ >= 0.5f) {
      if (!tocou_som) {
        TocaSomSucessoOuFracasso(camera);
      }
      terminou_alvo = !AtualizaAlvo(intervalo_ms);
    }
    if (progresso_ >= 1.0f && terminou_alvo) {
      VLOG(1) << "Finalizando corpo a corpo";
      finalizado_ = true;
      if (dados_ataque_.empunhadura() != EA_MAO_RUIM) {
        eo->AtualizaMatrizAcaoPrincipal(Matrix4());
      } else {
        eo->AtualizaMatrizAcaoSecundaria(Matrix4());
      }
    }
  }

  bool Finalizada() const override {
    return finalizado_;
  }

 private:
  // Atualiza a direcao.
  void AtualizaDeltas() {
    auto* eo = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem());
    auto* ed = BuscaPrimeiraEntidadeDestino(acao_proto_, tabuleiro_);
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
  float rotacao_graus_ = 0.0f;
  float progresso_ = 0.0f;  // de 0.0 a 1.0.
  float translacao_m_ = 0.0f;
  bool finalizado_;
  DadosAtaque dados_ataque_;
  int duracao_ms_;
  constexpr static int DURACAO_MS = 180;
};


// Acao de feitico.
class AcaoFeitico : public Acao {
 public:
  AcaoFeitico(const Tabelas& tabelas, const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas, const m3d::Modelos3d* modelos3d, ntf::CentralNotificacoes* central)
      : Acao(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central) {
    if (!acao_proto_.has_id_entidade_origem() || acao_proto_.por_entidade().empty()) {
      LOG(ERROR) << "Acao de feitico de toque requer origem e destino, tem origem? "
        << acao_proto_.has_id_entidade_origem() << ", destino vazio? " << acao_proto_.por_entidade().empty();
      terminado_ = true;
      return;
    }
    if (acao_proto.tipo() == ACAO_FEITICO_PESSOAL && acao_proto_.id_entidade_origem() != acao_proto_.por_entidade(0).id()) {
      LOG(ERROR) << "Acao de feitico de toque pessoal requer origem e destino iguais";
      terminado_ = true;
      return;
    }
    terminado_ = false;
    desenhando_origem_ = true;
    raio_ = 1.0f;
  }

  void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) const override {
    auto* e = tabuleiro_->BuscaEntidade(desenhando_origem_ ? acao_proto_.id_entidade_origem() : acao_proto_.por_entidade(0).id());
    if (e == nullptr) {
      return;
    }
    const Posicao& pos = e->PosicaoAcao();
    MudaCorProto(acao_proto_.cor());
    gl::Translada(pos.x() + acao_proto_.translacao().x(),
                  pos.y() + acao_proto_.translacao().y(),
                  pos.z() + acao_proto_.translacao().z());
    gl::Escala(acao_proto_.escala().x() * raio_, acao_proto_.escala().y() * raio_, acao_proto_.escala().z() * raio_);
    DesenhaGeometriaAcao();
  }

  void AtualizaAposAtraso(int intervalo_ms, const Olho& camera) override {
    auto* e = tabuleiro_->BuscaEntidade(desenhando_origem_ ? acao_proto_.id_entidade_origem() : acao_proto_.por_entidade(0).id());
    if (e == nullptr) {
      LOG(ERROR) << "Terminando acao feitico: origem ou destino nao existe mais.";
      terminado_ = true;
      return;
    }
    const float DELTA_RAIO = static_cast<float>(intervalo_ms) / DURACAO_MS;
    if (desenhando_origem_) {
      raio_ -= DELTA_RAIO;
      if (raio_ <= 0) {
        desenhando_origem_ = false;
        AtualizaAlvo(intervalo_ms);
        raio_ = 0.0f;
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
  constexpr static int DURACAO_MS = 300;
  bool desenhando_origem_;
  float raio_;
  bool terminado_;
};

// Nao faz nada.
class AcaoCriacaoEntidade : public Acao {
 public:
  AcaoCriacaoEntidade(const Tabelas& tabelas, const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas, const m3d::Modelos3d* modelos3d, ntf::CentralNotificacoes* central)
      : Acao(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central) {
  }

  void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) const override {
  }

  void AtualizaAposAtraso(int intervalo_ms, const Olho& camera) override {
  }

  bool Finalizada() const override {
    return true;
  }

 private:
};

}  // namespace

// Acao.
Acao::Acao(const Tabelas& tabelas, const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas, const m3d::Modelos3d* modelos3d, ntf::CentralNotificacoes* central)
    : tabelas_(tabelas), acao_proto_(acao_proto), tabuleiro_(tabuleiro), texturas_(texturas), m3d_(modelos3d), central_(central) {
  velocidade_m_ms_ = acao_proto.velocidade().inicial_m_s() / 1000.0f;
  aceleracao_m_ms_2_ = acao_proto.velocidade().aceleracao_m_s_2() / 1000.0f;
  dx_ = dy_ = dz_ = 0;
  dx_total_ = dy_total_ = dz_total_ = 0;
  disco_alvo_rad_ = 0;
  atraso_s_ = acao_proto_.atraso_s();
  if (central_ != nullptr && !acao_proto_.info_textura().id().empty()) {
    auto n = ntf::NovaNotificacao(ntf::TN_CARREGAR_TEXTURA);
    n->add_info_textura()->set_id(acao_proto_.info_textura().id());
    central_->AdicionaNotificacao(n.release());
  }
  if (central_ != nullptr && !acao_proto_.modelo_3d().id().empty()) {
    auto nl = ntf::NovaNotificacao(ntf::TN_CARREGAR_MODELO_3D);
    nl->mutable_entidade()->mutable_modelo_3d()->set_id(acao_proto_.modelo_3d().id());
    central_->AdicionaNotificacao(nl.release());
  }
}

Acao::~Acao() {
  if (central_ != nullptr && !acao_proto_.modelo_3d().id().empty()) {
    auto nl = ntf::NovaNotificacao(ntf::TN_DESCARREGAR_MODELO_3D);
    nl->mutable_entidade()->mutable_modelo_3d()->set_id(acao_proto_.modelo_3d().id());
    central_->AdicionaNotificacao(nl.release());
  }
  if (central_ != nullptr && !acao_proto_.info_textura().id().empty()) {
    auto n = ntf::NovaNotificacao(ntf::TN_DESCARREGAR_TEXTURA);
    n->add_info_textura()->set_id(acao_proto_.info_textura().id());
    central_->AdicionaNotificacao(n.release());
  }
}

void Acao::Atualiza(int intervalo_ms, const Olho& camera) {
  if (atraso_s_ > 0) {
    atraso_s_ -= (intervalo_ms / 1000.0f);
    return;
  }
  if (!tocou_som_inicial_) {
    TocaSomInicial(camera);
    tocou_som_inicial_ = true;
  }
  AtualizaAposAtraso(intervalo_ms, camera);
}

void Acao::TocaSomInicial(const Olho& camera) const {
  if (camera.pos().id_cenario() == CENARIO_INVALIDO && camera.pos().id_cenario() != IdCenario()) return;
  if (!acao_proto_.som_inicial().empty()) {
    som::Toca(acao_proto_.som_inicial());
  }
}

void Acao::TocaSomSucessoOuFracasso(const Olho& camera) const {
  if (camera.pos().id_cenario() == CENARIO_INVALIDO && camera.pos().id_cenario() != IdCenario()) return;
  if (acao_proto_.bem_sucedida()) {
    if (!acao_proto_.som_sucesso().empty()) {
      som::Toca(acao_proto_.som_sucesso());
    }
  } else {
    if (!acao_proto_.som_fracasso().empty()) {
      som::Toca(acao_proto_.som_fracasso());
    }
  }
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
  } else if (!acao_proto_.por_entidade().empty()) {
    auto* entidade_destino = tabuleiro_->BuscaEntidade(acao_proto_.por_entidade(0).id());
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
    VLOG(1) << "Nao desenhando acao pois id cenario: " << IdCenario()
            << " vs olho id: " << pd->pos_olho().id_cenario();
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

// Retorna false se finalizada.
bool Acao::AtualizaAlvo(int intervalo_ms) {
  if (acao_proto_.consequencia() == TC_REDUZ_LUZ_ALVO) {
    if (!acao_proto_.bem_sucedida()) {
      return false;
    }
    for (const auto& por_entidade : acao_proto_.por_entidade()) {
      const auto id = por_entidade.id();
      auto* entidade_destino = tabuleiro_->BuscaEntidade(id);
      if (entidade_destino == nullptr) continue;
      EntidadeProto parcial;
      parcial.mutable_luz()->set_raio_m(
          entidade_destino->RaioLuzMetros() * acao_proto_.reducao_luz());
      entidade_destino->AtualizaParcial(parcial);
    }
    return false;
  }
  if (acao_proto_.consequencia() == TC_DESLOCA_ALVO) {
    auto* entidade_destino = BuscaPrimeiraEntidadeDestino(acao_proto_, tabuleiro_);
    if (entidade_destino == nullptr) {
      VLOG(1) << "Finalizando alvo, destino nao existe.";
      return false;
    }
    if (!entidade_destino->PodeSerAfetadoPorAcoes()) {
      VLOG(1) << "Finalizando alvo, destino nao pode ser afetado por acoes.";
      return false;
    }
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
    for (const auto& por_entidade : acao_proto_.por_entidade()) {
      if (por_entidade.delta() == 0 && !por_entidade.forca_consequencia()) continue;
      const auto id = por_entidade.id();
      auto* entidade_destino = tabuleiro_->BuscaEntidade(id);
      if (entidade_destino == nullptr) {
        continue;
      }
      entidade_destino->AtivaFumegando(/*duracao_ms*/5000);
    }
    return false;
  } else if (acao_proto_.consequencia() == TC_DERRUBA_ALVO) {
    if (!acao_proto_.bem_sucedida()) {
      VLOG(1) << "Finalizando alvo, nao foi bem sucedida.";
      dx_total_ = dy_total_ = dz_total_ = 0;
      return false;
    }
    for (const auto& por_entidade : acao_proto_.por_entidade()) {
      if (por_entidade.delta() == 0 && !por_entidade.forca_consequencia()) continue;
      const auto id = por_entidade.id();
      auto* entidade_destino = tabuleiro_->BuscaEntidade(id);
      if (entidade_destino == nullptr) {
        LOG(ERROR) << "entidade destino invalida, id: " << id;
        continue;
      }
      if (!entidade_destino->PodeSerAfetadoPorAcoes()) {
        VLOG(1) << "Finalizando alvo, destino nao pode ser afetado por acoes.";
        return false;
      }
      AtualizaDirecaoQuedaAlvo(entidade_destino);
      EntidadeProto parcial;
      parcial.set_caida(true);
      entidade_destino->AtualizaParcial(parcial);
    }
    return false;
  } else if (acao_proto_.consequencia() == TC_AGARRA_ALVO) {
    Entidade* entidade_destino = EntidadeDestino();
    Entidade* entidade_origem = EntidadeOrigem();
    if (entidade_origem == nullptr || entidade_destino == nullptr) {
      VLOG(1) << "Finalizando alvo, origem ou destino não existe.";
      return false;
    }
    if (!entidade_destino->PodeSerAfetadoPorAcoes()) {
      VLOG(1) << "Finalizando alvo, destino nao pode ser afetado por acoes.";
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

void Acao::AtualizaLuzOrigem(int intervalo_ms) {
  if (acao_proto_.consequencia_origem() != TC_ILUMINA_ALVO) return;
  Entidade* entidade_origem = nullptr;
  if (!acao_proto_.has_id_entidade_origem() ||
      (entidade_origem = tabuleiro_->BuscaEntidade(acao_proto_.id_entidade_origem())) == nullptr) {
    VLOG(1) << "Luz origem nao sera ligada, origem não existe.";
    return;
  }
  if (acao_proto_.has_luz_origem()) {
    entidade_origem->AtivaLuzAcao(acao_proto_.luz_origem());
  } else {
    IluminacaoPontual luz;
    auto* cor = luz.mutable_cor();
    cor->set_r(1);
    cor->set_g(1);
    cor->set_b(1);
    luz.set_raio_m(1.5f);
    luz.set_duracao_ms(500);
    entidade_origem->AtivaLuzAcao(luz);
  }
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
    case ACAO_EXPULSAR_FASCINAR_MORTOS_VIVOS:
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
  return BuscaPrimeiraEntidadeDestino(acao_proto_, tabuleiro_);
}

// O tamanho sera unitario na unidade da geometria (ou seja, raio para esfera, lado para cubo).
void Acao::DesenhaGeometriaAcao() const {
  switch (acao_proto_.geometria()) {
    case ACAO_GEO_CUBO:
      gl::CuboSolido(1.0f);
      return;
    case ACAO_GEO_CONE:
      gl::ConeSolido(0.5f  /*raio*/, 1.0f  /*altura*/, 10  /*divisoes base*/, 3  /*divisoes altura*/);
      return;
    case ACAO_GEO_CILINDRO:
      gl::CilindroSolido(1.0f  /*raio*/, 2.0f  /*altura*/, 10  /*divisoes base*/, 3  /*divisoes altura*/);
      return;
    case ACAO_GEO_MODELO_3D: {
      const auto* modelo = m3d_->Modelo(acao_proto_.modelo_3d().id());
      if (modelo != nullptr) {
        modelo->vbos_gravados.Desenha();
      }
      return;
    }
    case ACAO_GEO_ESFERA:
    default:
      gl::EsferaSolida(1.0f  /*raio*/, 10  /*fatias*/, 10  /*tocos*/);
      return;
  }
}

// static
bool Acao::PontoAfetadoPorAcao(const Posicao& pos_ponto, const Posicao& pos_origem, const AcaoProto& acao_proto, bool ponto_eh_origem) {
  switch (acao_proto.tipo()) {
    case ACAO_PROJETIL_AREA: {
      const float dq_m2 =
        DistanciaEmMetrosAoQuadrado(
            pos_ponto, acao_proto.pos_tabuleiro());
      // Admite uma certa tolerancia, porque o raio normalmente eh pequeno.
      const float distancia_maxima_m2 = powf(acao_proto.raio_quadrados() * TAMANHO_LADO_QUADRADO * 1.10f, 2);
      VLOG(1) << "Distancia ao quadrado: " << dq_m2 << ", maximo: " << distancia_maxima_m2;
      return dq_m2 <= distancia_maxima_m2;
    }
    case ACAO_DISPERSAO:
    case ACAO_EXPULSAR_FASCINAR_MORTOS_VIVOS:
      switch (acao_proto.geometria()) {
        case ACAO_GEO_ESFERA: {
          const float dq =
            DistanciaEmMetrosAoQuadrado(
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

Acao* NovaAcao(const Tabelas& tabelas, const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas, const m3d::Modelos3d* modelos3d, ntf::CentralNotificacoes* central) {
  VLOG(1) << "NovaAcao: " << acao_proto.DebugString();
  switch (acao_proto.tipo()) {
    case ACAO_SINALIZACAO:
      return new AcaoSinalizacao(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central);
    case ACAO_PROJETIL:
      return new AcaoProjetil(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central);
    case ACAO_EXPULSAR_FASCINAR_MORTOS_VIVOS:
    case ACAO_DISPERSAO:
      return new AcaoDispersao(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central);
    case ACAO_DELTA_PONTOS_VIDA:
      return new AcaoDeltaPontosVida(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central);
    case ACAO_RAIO:
      return new AcaoRaio(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central);
    case ACAO_CORPO_A_CORPO:
      return new AcaoCorpoCorpo(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central);
    case ACAO_FEITICO_TOQUE:
    case ACAO_FEITICO_PESSOAL:
      return new AcaoFeitico(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central);
    case ACAO_AGARRAR:
      return new AcaoAgarrar(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central);
    case ACAO_POCAO:
      return new AcaoPocao(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central);
    case ACAO_PROJETIL_AREA:
      return new AcaoProjetilArea(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central);
    case ACAO_CRIACAO_ENTIDADE:
      return new AcaoCriacaoEntidade(tabelas, acao_proto, tabuleiro, texturas, modelos3d, central);
    default:
      LOG(ERROR) << "Acao invalida: " << acao_proto.ShortDebugString();
      return nullptr;
  }
}

const std::string& TextoAcao(const AcaoProto& acao_proto) {
  if (TemAlgumDestino(acao_proto) && acao_proto.por_entidade(0).has_texto()) {
    return acao_proto.por_entidade(0).texto();
  }
  return acao_proto.texto();
}

int DeltaAcao(const AcaoProto& acao_proto) {
  if (TemAlgumDestino(acao_proto) && acao_proto.por_entidade(0).has_delta()) {
    return acao_proto.por_entidade(0).delta();
  }
  return acao_proto.delta_pontos_vida();
}

void CombinaEfeitos(AcaoProto* acao) {
  std::set<int, std::greater<int>> a_remover;
  std::unordered_map<int, AcaoProto::EfeitoAdicional*> efeito_por_id;
  for (int i = 0; i < acao->efeitos_adicionais_size(); ++i) {
    auto* ea = acao->mutable_efeitos_adicionais(i);
    if (ea->has_combinar_com() || ea->has_combinar_com_efeito()) {
      // Vou ignorar o valor e simplesmente pegar pelo id do efeito se houver.
      auto it = efeito_por_id.find(ea->has_combinar_com_efeito() ? ea->combinar_com_efeito() : ea->efeito());
      if (it != efeito_por_id.end()) {
        it->second->MergeFrom(*ea);
        it->second->clear_combinar_com();
        it->second->clear_combinar_com_efeito();
        VLOG(1) << "combinado por id: " << acao->efeitos_adicionais(ea->combinar_com()).DebugString();
      } else if (ea->combinar_com() >= 0 && ea->combinar_com() < acao->efeitos_adicionais_size() && i != ea->combinar_com()) {
        acao->mutable_efeitos_adicionais(ea->combinar_com())->MergeFrom(*ea);
        acao->mutable_efeitos_adicionais(ea->combinar_com())->clear_combinar_com();
        acao->mutable_efeitos_adicionais(ea->combinar_com())->clear_combinar_com_efeito();
        LOG(WARNING) << "combinado por posicao: " << acao->efeitos_adicionais(ea->combinar_com()).DebugString();
      } else {
        LOG(ERROR) << "Combina com invalido: " << ea->combinar_com()
                   << ", i: " << i << ", tamanho: " << acao->efeitos_adicionais_size();
      }
      a_remover.insert(i);
    } else if (ea->has_efeito()) {
      efeito_por_id[ea->efeito()] = ea;
    }
  }
  for (int i : a_remover) {
    acao->mutable_efeitos_adicionais()->DeleteSubrange(i, 1);
  }
}

bool EfeitoArea(const AcaoProto& acao_proto) {
  return acao_proto.efeito_area() || acao_proto.tipo() == ACAO_DISPERSAO;
}

const std::vector<unsigned int> EntidadesAfetadasPorAcao(
    const AcaoProto& acao, const Entidade* entidade_origem, const std::vector<const Entidade*>& entidades_cenario) {
  VLOG(1) << "entidades_cenario: " << entidades_cenario.size();
  std::vector<const Entidade*> entidades_ordenadas;
  if (acao.mais_fracos_primeiro()) {
    entidades_ordenadas = entidades_cenario;
    std::sort(entidades_ordenadas.begin(), entidades_ordenadas.end(), [](const Entidade* lhs, const Entidade* rhs) {
        if (lhs->NivelPersonagem() < rhs->NivelPersonagem()) return true;
        if (rhs->NivelPersonagem() < lhs->NivelPersonagem()) return false;
        return lhs->Id() < rhs->Id();
    });
  }
  VLOG(1) << "entidades_ordenadas: " << entidades_ordenadas.size();
  Posicao pos_origem;
  if (entidade_origem != nullptr) {
    pos_origem = entidade_origem->PosicaoAcao();
  }
  int total_afetados = 0;
  int total_dv = 0;
  std::vector<unsigned int> ids_afetados;
  for (const auto* entidade : acao.mais_fracos_primeiro() ? entidades_ordenadas : entidades_cenario) {
    if (!entidade->PodeSerAfetadoPorAcoes()) continue;
    const int dv = entidade->NivelPersonagem();
    if (acao.has_total_dv() && (total_dv + dv) > acao.total_dv()) continue;
    Posicao epos = Acao::AjustaPonto(entidade->PosicaoAcao(), entidade->MultiplicadorTamanho(), pos_origem, acao);
    if (!Acao::PontoAfetadoPorAcao(epos, pos_origem, acao, /*ponto eh origem=*/entidade_origem != nullptr && entidade->Id() == entidade_origem->Id())) {
      continue;
    }
    ids_afetados.push_back(entidade->Id());
    total_dv += dv;
    if (acao.has_total_dv() && total_dv == acao.total_dv()) break;
    ++total_afetados;
    if (acao.has_maximo_criaturas_afetadas() && total_afetados >= acao.maximo_criaturas_afetadas()) {
      break;
    }
  }
  return ids_afetados;
}

bool EntidadeAfetadaPorEfeito(const Tabelas& tabelas, int nivel_conjurador, const AcaoProto::EfeitoAdicional& efeito, const EntidadeProto& alvo) {
  int nivel_alvo = NivelPersonagem(alvo);
  int nivel_base = efeito.referencia_dados_vida_nivel_conjurador() ? nivel_conjurador : 0;
  if (EntidadeImuneEfeito(alvo, efeito.efeito())) {
    return false;
  }
  if (efeito.has_afeta_apenas_dados_vida_igual_a() && nivel_alvo != (nivel_base + efeito.afeta_apenas_dados_vida_igual_a())) {
    return false;
  }
  if (efeito.has_afeta_apenas_tamanhos_menores_ou_igual_a() && alvo.tamanho() > efeito.afeta_apenas_tamanhos_menores_ou_igual_a()) {
    return false;
  }
  if (efeito.has_afeta_apenas_dados_vida_menor_igual_a() && nivel_alvo > (nivel_base + efeito.afeta_apenas_dados_vida_menor_igual_a())) {
    return false;
  }
  if (efeito.has_afeta_apenas_dados_vida_maior_igual_a() && nivel_alvo < (nivel_base + efeito.afeta_apenas_dados_vida_maior_igual_a())) {
    return false;
  }
  if (!efeito.afeta_apenas_tendencias().empty() &&
      c_none_of(efeito.afeta_apenas_tendencias(), [&alvo](int tendencia) { return MesmaTendencia(static_cast<TendenciaSimplificada>(tendencia), alvo); })) {
    return false;
  }
  if (!efeito.afeta_apenas().empty() &&
      c_none_of(efeito.afeta_apenas(), [&alvo](int tipo) { return TemTipoDnD(static_cast<TipoDnD>(tipo), alvo); })) {
    return false;
  }
  if (!efeito.nao_afeta_tipo().empty() &&
      c_any_of(efeito.nao_afeta_tipo(), [&alvo](int tipo) { return TemTipoDnD(static_cast<TipoDnD>(tipo), alvo); })) {
    return false;
  }
  const auto& efeito_tabelado = tabelas.Efeito(efeito.efeito());
  if (!efeito_tabelado.nao_afeta().empty() &&
      c_any_of(efeito_tabelado.nao_afeta(), [&alvo](int tipo) { return TemTipoDnD(static_cast<TipoDnD>(tipo), alvo); })) {
    return false;
  }

  return true;
}

}  // namespace ent
