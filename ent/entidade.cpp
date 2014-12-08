#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ent/tabuleiro.pb.h"
#include "ent/util.h"
#include "gltab/gl.h"

#include "log/log.h"

namespace ent {
namespace {

const double DURACAO_QUEDA_SEGUNDOS = 0.5f;
// Tamanho da barra de vida.
const float TAMANHO_BARRA_VIDA = TAMANHO_LADO_QUADRADO_2;
const float TAMANHO_BARRA_VIDA_2 = TAMANHO_BARRA_VIDA / 2.0f;
const float ALTURA_TIJOLO_BASE = TAMANHO_LADO_QUADRADO_10;

}  // namespace

// Factory.
Entidade* NovaEntidade(const EntidadeProto& proto, const Texturas* texturas, ntf::CentralNotificacoes* central) {
  switch (proto.tipo()) {
    case TE_COMPOSTA:
    case TE_ENTIDADE:
    case TE_FORMA: {
      auto* e = new Entidade(texturas, central);
      e->Inicializa(proto);
      return e;
    }
    default:
      std::ostringstream oss;
      oss << "Tipo de entidade inválido: " << proto.tipo();
      throw std::logic_error(oss.str());
  }
}

// Entidade
Entidade::Entidade(const Texturas* texturas, ntf::CentralNotificacoes* central) {
  vd_.texturas = texturas;
  central_ = central;
}

Entidade::~Entidade() {
  EntidadeProto dummy;
  AtualizaTexturas(dummy);
}

void Entidade::Inicializa(const EntidadeProto& novo_proto) {
  // Preciso do tipo aqui para atualizar as outras coisas de acordo.
  proto_.set_tipo(novo_proto.tipo());
  // Atualiza texturas antes de tudo.
  AtualizaTexturas(novo_proto);
  // mantem o tipo.
  proto_.CopyFrom(novo_proto);
  if (proto_.has_dados_vida() && !proto_.has_max_pontos_vida()) {
    // Geracao automatica de pontos de vida.
    try {
      int pv = GeraPontosVida(proto_.dados_vida());
      if (pv == 0) {
        pv = 1;
      }
      proto_.set_max_pontos_vida(pv);
      proto_.set_pontos_vida(pv);
    } catch (const std::logic_error& erro) {
      LOG(ERROR) << "Erro inicializando entidade: " << erro.what();
    }
  } else {
    // Usa os pontos de vida que vierem.
    if (!proto_.has_max_pontos_vida()) {
      // Entidades sempre devem ter o maximo de pontos de vida, que eh usado para acoes de dano.
      proto_.set_max_pontos_vida(0);
    }
    if (!proto_.has_pontos_vida() || proto_.pontos_vida() > proto_.max_pontos_vida()) {
      proto_.set_pontos_vida(proto_.max_pontos_vida());
    }
  }
}

void Entidade::AtualizaTexturas(const EntidadeProto& novo_proto) {
  AtualizaTexturasProto(novo_proto, &proto_, central_);
}

void Entidade::AtualizaTexturasProto(const EntidadeProto& novo_proto, EntidadeProto* proto_atual, ntf::CentralNotificacoes* central) {
  if (proto_atual->tipo() == TE_COMPOSTA) {
    VLOG(1) << "Atualizando textura para entidade composta";
    AtualizaTexturasEntidadesCompostasProto(novo_proto, proto_atual, central);
    return;
  }
  VLOG(2) << "Novo proto: " << novo_proto.ShortDebugString() << ", velho: " << proto_atual->ShortDebugString();
  // Libera textura anterior se houver e for diferente da corrente.
  if (proto_atual->has_info_textura() && proto_atual->info_textura().id() != novo_proto.info_textura().id()) {
    VLOG(1) << "Liberando textura: " << proto_atual->info_textura().id();
    auto* nl = ntf::NovaNotificacao(ntf::TN_DESCARREGAR_TEXTURA);
    nl->mutable_info_textura()->set_id(proto_atual->info_textura().id());
    central->AdicionaNotificacao(nl);
  }
  // Carrega textura se houver e for diferente da antiga.
  if (novo_proto.has_info_textura() && novo_proto.info_textura().id() != proto_atual->info_textura().id()) {
    VLOG(1) << "Carregando textura: " << proto_atual->info_textura().id();
    auto* nc = ntf::NovaNotificacao(ntf::TN_CARREGAR_TEXTURA);
    nc->mutable_info_textura()->CopyFrom(novo_proto.info_textura());
    central->AdicionaNotificacao(nc);
  }
  if (novo_proto.has_info_textura()) {
    proto_atual->mutable_info_textura()->CopyFrom(novo_proto.info_textura());
  } else {
    proto_atual->clear_info_textura();
  }
}

void Entidade::AtualizaTexturasEntidadesCompostasProto(
    const EntidadeProto& novo_proto, EntidadeProto* proto_atual, ntf::CentralNotificacoes* central) {
  // Libera todas.
  if (novo_proto.sub_forma_size() != proto_atual->sub_forma_size()) {
    // Libera todos antigos e deixa do mesmo tamanho do novo.
    EntidadeProto dummy;
    for (auto& forma_velha : *proto_atual->mutable_sub_forma()) {
      VLOG(2) << "Liberando textura de sub forma para entidade composta";
      AtualizaTexturasProto(dummy, &forma_velha, central);
    }
    proto_atual->clear_sub_forma();
    for (int i = 0; i < novo_proto.sub_forma_size(); ++i) {
      proto_atual->add_sub_forma();
    }
  }
  for (int i = 0; i < novo_proto.sub_forma_size(); ++i) {
    VLOG(2) << "Atualizando textura de sub forma para entidade composta";
    AtualizaTexturasProto(novo_proto.sub_forma(i), proto_atual->mutable_sub_forma(i), central);
  }
}

void Entidade::AtualizaProto(const EntidadeProto& novo_proto) {
  VLOG(1) << "Proto antes: " << proto_.ShortDebugString();
  AtualizaTexturas(novo_proto);

  // mantem o tipo.
  ent::EntidadeProto copia_proto(proto_);
  proto_.CopyFrom(novo_proto);
  if (proto_.pontos_vida() > proto_.max_pontos_vida()) {
    proto_.set_pontos_vida(proto_.max_pontos_vida());
  }
  proto_.set_id(copia_proto.id());
  proto_.mutable_pos()->Swap(copia_proto.mutable_pos());
  if (copia_proto.has_destino()) {
    proto_.mutable_destino()->Swap(copia_proto.mutable_destino());
  }
  VLOG(1) << "Proto depois: " << proto_.ShortDebugString();
}

void Entidade::Atualiza() {
  auto* po = proto_.mutable_pos();
  vd_.angulo_disco_selecao_graus = fmod(vd_.angulo_disco_selecao_graus + 1.0, 360.0);
  // Voo.
  const float DURACAO_POSICIONAMENTO_INICIAL = 1.0f;
  const float DURACAO_VOO_SEGUNDOS = 4.0f;
  const float DELTA_VOO = 2.0f * M_PI * POR_SEGUNDO_PARA_ATUALIZACAO / DURACAO_VOO_SEGUNDOS;
  float z_chao = ZChao(X(), Y());
  if (proto_.voadora()) {
    if (Z() < z_chao + ALTURA_VOO) {
      po->set_z(po->z() + ALTURA_VOO * POR_SEGUNDO_PARA_ATUALIZACAO / DURACAO_POSICIONAMENTO_INICIAL);
      vd_.angulo_disco_voo_rad = 0.0f;
    } else {
      vd_.angulo_disco_voo_rad = fmod(vd_.angulo_disco_voo_rad + DELTA_VOO, 2 * M_PI);
    }
  } else {
    if (Z() > z_chao) {
      po->set_z(po->z() - ALTURA_VOO * POR_SEGUNDO_PARA_ATUALIZACAO / DURACAO_POSICIONAMENTO_INICIAL);
    }
    vd_.angulo_disco_voo_rad = 0.0f;
  }
  // Queda.
  const float DELTA_QUEDA = (90.0f * POR_SEGUNDO_PARA_ATUALIZACAO / DURACAO_QUEDA_SEGUNDOS);
  if (proto_.caida()) {
    if (vd_.angulo_disco_queda_graus < 90.0f) {
      vd_.angulo_disco_queda_graus += DELTA_QUEDA;
    }
  } else {
    if (vd_.angulo_disco_queda_graus > 0) {
      vd_.angulo_disco_queda_graus -= DELTA_QUEDA;
    }
  }
  // Nunca fica abaixo do solo.
  if (Z() < z_chao) {
    po->set_z(z_chao);
  }

  // Daqui pra baixo, tratamento de destino.
  if (!proto_.has_destino()) {
    return;
  }
  double origens[] = { po->x(), po->y(), po->z() };
  const auto& pd = proto_.destino();
  double destinos[] = { pd.x(), pd.y(), pd.z() };

  bool chegou = true;
  // deslocamento em cada eixo (x, y, z) por chamada de atualizacao.
  const float VELOCIDADE_POR_EIXO = 10.0f * POR_SEGUNDO_PARA_ATUALIZACAO;
  for (int i = 0; i < 3; ++i) {
    double delta = (origens[i] > destinos[i]) ? -VELOCIDADE_POR_EIXO : VELOCIDADE_POR_EIXO;
    float diferenca = fabs(origens[i] - destinos[i]);
    // Acelera para diferencas grandes.
    if (diferenca <= VELOCIDADE_POR_EIXO) {
      origens[i] = destinos[i];
    } else {
      if (diferenca > 5 * VELOCIDADE_POR_EIXO) {
        delta *= 5;
      } else if (diferenca > 3 * VELOCIDADE_POR_EIXO) {
        delta *= 3;
      }
      chegou = false;
      origens[i] += delta;
    }
  }
  po->set_x(origens[0]);
  po->set_y(origens[1]);
  po->set_z(origens[2]);
  if (chegou) {
    proto_.clear_destino();
  }
}

void Entidade::MovePara(float x, float y, float z) {
  VLOG(1) << "Entidade antes de mover: " << proto_.pos().ShortDebugString();
  auto* p = proto_.mutable_pos();
  p->set_x(x);
  p->set_y(y);
  p->set_z(std::max(ZChao(x, y), z));
  proto_.clear_destino();
  VLOG(1) << "Movi entidade para: " << proto_.pos().ShortDebugString();
}

void Entidade::MoveDelta(float dx, float dy, float dz) {
  MovePara(X() + dx, Y() + dy, Z() + dz);
}

void Entidade::Destino(const Posicao& pos) {
  proto_.mutable_destino()->CopyFrom(pos);
}

void Entidade::AlteraTranslacaoZ(float delta) {
  proto_.set_translacao_z(proto_.translacao_z() + delta);
}

void Entidade::AlteraRotacaoZ(float delta) {
  proto_.set_rotacao_z_graus(proto_.rotacao_z_graus() + delta);
}

int Entidade::PontosVida() const {
  return proto_.pontos_vida();
}

float Entidade::X() const {
  return proto_.pos().x();
}

float Entidade::Y() const {
  return proto_.pos().y();
}

float Entidade::Z() const {
  return proto_.pos().z();
}

void Entidade::MataEntidade() {
  proto_.set_morta(true);
  proto_.set_caida(true);
  proto_.set_voadora(false);
  proto_.set_aura(0);
}

void Entidade::AtualizaPontosVida(int pontos_vida) {
  if (proto_.max_pontos_vida() == 0) {
    // Entidades sem pontos de vida nao sao afetadas.
    return;
  }
  if (proto_.pontos_vida() >= 0 && pontos_vida < 0) {
    proto_.set_morta(true);
    proto_.set_caida(true);
    proto_.set_voadora(false);
    proto_.set_aura(0);
  } else if (proto_.pontos_vida() < 0 && pontos_vida >= 0) {
    proto_.set_morta(false);
  }
  proto_.set_pontos_vida(std::min(proto_.max_pontos_vida(), pontos_vida));
}

void Entidade::AtualizaParcial(const EntidadeProto& proto_parcial) {
  int pontos_vida_antes = PontosVida();
  // ATENCAO: todos os campos repeated devem ser verificados aqui para nao haver duplicacao.
  proto_.MergeFrom(proto_parcial);
  // Casos especiais.
  auto* luz = proto_.has_luz() ? proto_.mutable_luz()->mutable_cor() : nullptr;
  if (luz != nullptr && luz->r() == 0 && luz->g() == 0 && luz->b() == 0) {
    proto_.clear_luz();
  }
  if (proto_parcial.has_pontos_vida()) {
    // Restaura o que o merge fez para poder aplicar AtualizaPontosVida.
    proto_.set_pontos_vida(pontos_vida_antes);
    AtualizaPontosVida(proto_parcial.pontos_vida());
  }
}

void Entidade::AtualizaAcao(const std::string& id_acao) {
  proto_.set_ultima_acao(id_acao);
}

const Posicao Entidade::PosicaoAcao() const {
  gl::MatrizEscopo salva_matriz(GL_MODELVIEW);
  gl::CarregaIdentidade();
  MontaMatriz(true  /*em_voo*/, true  /*queda*/, true  /*tz*/,proto_, vd_);
  if (!proto_.achatado()) {
    gl::Translada(0.0f, 0.0f, ALTURA);
  }
  GLfloat matriz[16];
  gl::Le(GL_MODELVIEW_MATRIX, matriz);
  VLOG(2) << "Matriz: " << matriz[0] << " " << matriz[1] << " " << matriz[2] << " " << matriz[3];
  VLOG(2) << "Matriz: " << matriz[4] << " " << matriz[5] << " " << matriz[6] << " " << matriz[7];
  VLOG(2) << "Matriz: " << matriz[8] << " " << matriz[9] << " " << matriz[10] << " " << matriz[11];
  VLOG(2) << "Matriz: " << matriz[12] << " " << matriz[13] << " " << matriz[14] << " " << matriz[15];
  GLfloat ponto[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  MultiplicaMatrizVetor(matriz, ponto);
  VLOG(2) << "Ponto: " << ponto[0] << " " << ponto[1] << " " << ponto[2] << " " << ponto[3];
  Posicao pos;
  pos.set_x(ponto[0]);
  pos.set_y(ponto[1]);
  pos.set_z(ponto[2]);
  return pos;
}

float Entidade::DeltaVoo(const VariaveisDerivadas& vd) {
  return vd.angulo_disco_voo_rad > 0 ? sinf(vd.angulo_disco_voo_rad) * ALTURA_VOO / 4.0f : 0.0f;
}

void Entidade::MontaMatriz(bool em_voo,
                           bool queda,
                           bool transladar_z,
                           const EntidadeProto& proto,
                           const VariaveisDerivadas& vd,
                           const ParametrosDesenho* pd,
                           const float* matriz_shear) {
  const auto& pos = proto.pos();
  bool achatar = (pd != nullptr && pd->desenha_texturas_para_cima());
  float translacao_z = ZChao(pos.x(), pos.y()) + (transladar_z ? pos.z() + proto.translacao_z() : 0);
  if (em_voo) {
    translacao_z += DeltaVoo(vd);
  }
  if (matriz_shear == nullptr) {
    gl::Translada(pos.x(), pos.y(), translacao_z);
  } else {
    gl::Translada(pos.x(), pos.y(), 0);
    gl::MultiplicaMatriz(matriz_shear);
    gl::Translada(0, 0, translacao_z);
  }
  if (achatar && !proto.has_info_textura()) {
    // Achata cone.
    gl::Escala(1.0f, 1.0f, 0.1f);
  }

  // So roda entidades nao achatadas.
  if (queda && vd.angulo_disco_queda_graus > 0 && !achatar) {
    // Descomentar essa linha para ajustar a posicao da entidade.
    //gl::Translada(0, -TAMANHO_LADO_QUADRADO_2, 0);
    // Roda pra direcao de queda.
    const auto& dq = proto.direcao_queda();
    if (dq.x() != 0.0f || dq.y() != 0) {
      // Como a queda é sobre o eixo X, subtrai 90 para a direcao ficar certa.
      float direcao_queda_graus = VetorParaRotacaoGraus(dq) - 90.0f;
      gl::Roda(direcao_queda_graus, 0.0f, 0.0f, 1.0f);
    }
    // Roda sobre o eixo X negativo para cair com a face para cima.
    gl::Roda(vd.angulo_disco_queda_graus, -1.0f, 0, 0);
  }
  float multiplicador = CalculaMultiplicador(proto.tamanho());
  gl::Escala(multiplicador, multiplicador, multiplicador);
}

void Entidade::Desenha(ParametrosDesenho* pd) {
  if (!proto_.visivel() || proto_.cor().a() < 1.0f) {
    // Sera desenhado translucido.
    return;
  }
  DesenhaObjetoComDecoracoes(pd);
}

void Entidade::DesenhaTranslucido(ParametrosDesenho* pd) {
  if (proto_.visivel()) {
    // Visivel so eh desenhado aqui se a cor for transparente.
    if (proto_.cor().a() == 1.0f) {
      return;
    }
  } else {
    // Invisivel, so desenha para o mestre independente da cor (sera translucido).
    if (!pd->modo_mestre()) {
      return;
    }
  }
  DesenhaObjetoComDecoracoes(pd);
}

void Entidade::DesenhaObjeto(ParametrosDesenho* pd, const float* matriz_shear) {
  DesenhaObjetoProto(proto_, vd_, pd, matriz_shear);
}

void Entidade::DesenhaObjetoComDecoracoes(ParametrosDesenho* pd) {
  gl::CarregaNome(Id());
  // Tem que normalizar por causa das operacoes de escala, que afetam as normais.
  gl::Habilita(GL_NORMALIZE);
  DesenhaObjeto(pd);
  DesenhaDecoracoes(pd);
  gl::Desabilita(GL_NORMALIZE);
}

void Entidade::DesenhaDecoracoes(ParametrosDesenho* pd) {
  if (proto_.tipo() != TE_ENTIDADE) {
    // Apenas entidades tem decoracoes.
    return;
  }
  if (!proto_.has_info_textura() && pd->entidade_selecionada()) {
    // Volta pro chao.
    gl::MatrizEscopo salva_matriz;
    MontaMatriz(false  /*em_voo*/, true  /*queda*/, false /*tz*/, proto_, vd_, pd);
    MudaCor(proto_.cor());
    gl::Roda(vd_.angulo_disco_selecao_graus, 0, 0, 1.0f);
    DesenhaDisco(TAMANHO_LADO_QUADRADO_2, 6);
  }
  // Desenha a barra de vida.
  if (pd->desenha_barra_vida()) {
#if 0
    // Codigo para iluminar barra de vida.
    gl::AtributoEscopo salva_attributos(GL_LIGHTING_BIT | GL_ENABLE_BIT);
    // Luz no olho apontando para a barra.
    const Posicao& pos_olho = pd->pos_olho();
    gl::Luz(GL_LIGHT0, GL_DIFFUSE, COR_BRANCA);
    const auto& pos = proto_.pos();
    GLfloat pos_luz[] = { pos_olho.x() - pos.x(), pos_olho.y() - pos.y(), pos_olho.z() - pos.z(), 0.0f };
    gl::Luz(GL_LIGHT0, GL_POSITION, pos_luz);
#endif

    gl::MatrizEscopo salva_matriz;
    MontaMatriz(true  /*em_voo*/, false  /*queda*/, true  /*tz*/, proto_, vd_, pd);
    gl::Translada(0.0f, 0.0f, ALTURA * (proto_.achatado() ? 0.5f : 1.5f));
    {
      gl::MatrizEscopo salva_matriz;
      gl::Escala(0.2, 0.2, 1.0f);
      MudaCor(COR_VERMELHA);
      gl::CuboSolido(TAMANHO_BARRA_VIDA);
    }
    if (proto_.max_pontos_vida() > 0 && proto_.pontos_vida() > 0) {
      float porcentagem = static_cast<float>(proto_.pontos_vida()) / proto_.max_pontos_vida();
      float tamanho_barra = TAMANHO_BARRA_VIDA * porcentagem;
      float delta = -TAMANHO_BARRA_VIDA_2 + (tamanho_barra / 2.0f);
      gl::Translada(0, 0, delta);
      gl::Escala(0.3f, 0.3f, porcentagem);
      gl::HabilitaEscopo habilita_offset(GL_POLYGON_OFFSET_FILL);
      gl::DesvioProfundidade(0, -25.0);
      MudaCor(COR_VERDE);
      gl::CuboSolido(TAMANHO_BARRA_VIDA);
    }
  }

  if (pd->desenha_rotulo() || pd->desenha_rotulo_especial()) {
    gl::DesabilitaEscopo salva_luz(GL_LIGHTING);
    gl::MatrizEscopo salva_matriz;
    MontaMatriz(true  /*em_voo*/, false  /*queda*/, true  /*tz*/, proto_, vd_, pd);
    gl::Translada(0.0f, 0.0f, ALTURA * 1.5f + TAMANHO_BARRA_VIDA);
    MudaCor(COR_BRANCA);
    if (pd->desenha_rotulo()) {
      gl::PosicaoRaster(0.0f, 0.0f, 0.0f);
      gl::DesenhaString(proto_.rotulo());
    }
    if (pd->desenha_rotulo_especial()) {
      gl::PosicaoRaster(0.0f, 0.0f, 0.0f);
      std::string rotulo;
      for (const std::string& rotulo_especial : proto_.rotulo_especial()) {
        rotulo += std::string("\n") + rotulo_especial;
      }
      if (proto_.proxima_salvacao() != RS_FALHOU) {
        rotulo += "\nprox. salv.: ";
        switch (proto_.proxima_salvacao()) {
          case RS_MEIO:
            rotulo += "1/2";
            break;
          case RS_QUARTO:
            rotulo += "1/4";
            break;
          case RS_ANULOU:
            rotulo += "ANULA";
            break;
          default:
            rotulo += "VALOR INVALIDO";
        }
      }
      gl::DesenhaString(rotulo);
    }
  }
}

void Entidade::DesenhaLuz(ParametrosDesenho* pd) {
  if (!pd->iluminacao() || !proto_.has_luz()) {
    return;
  }
  if (!proto_.visivel() && !pd->modo_mestre()) {
    return;
  }

  bool achatado = (pd != nullptr && pd->desenha_texturas_para_cima()) || proto_.achatado();
  gl::MatrizEscopo salva_matriz;
  if (achatado) {
    // So translada para a posicao do objeto.
    gl::Translada(X(), Y(), Z());
  } else {
    MontaMatriz(true  /*em_voo*/, true  /*queda*/, true  /*tz*/, proto_, vd_, pd);
  }
  // Obtem vetor da camera para o objeto e roda para o objeto ficar de frente para camera.
  Posicao vetor_camera_objeto;
  ComputaDiferencaVetor(Pos(), pd->pos_olho(), &vetor_camera_objeto);
  gl::Roda(VetorParaRotacaoGraus(vetor_camera_objeto), 0.0f, 0.0f, 1.0f);

  // Um para direcao da camera para luz iluminar o proprio objeto.
  gl::Translada(-TAMANHO_LADO_QUADRADO_2, 0.0f, ALTURA + TAMANHO_LADO_QUADRADO_2);

  int id_luz = pd->luz_corrente();
  if (id_luz == 0 || id_luz >= pd->max_num_luzes()) {
    LOG(ERROR) << "Limite de luzes alcançado: " << id_luz;
  } else {
    // Objeto de luz. O quarto componente indica que a luz é posicional.
    // Se for 0, a luz é direcional e os componentes indicam sua direção.
    GLfloat pos_luz[] = { 0, 0, 0, 1.0f };
    gl::Luz(GL_LIGHT0 + id_luz, GL_POSITION, pos_luz);
    const ent::Cor& cor = proto_.luz().cor();
    GLfloat cor_luz[] = { cor.r(), cor.g(), cor.b(), cor.a() };
    gl::Luz(GL_LIGHT0 + id_luz, GL_DIFFUSE, cor_luz);
    gl::Luz(GL_LIGHT0 + id_luz, GL_CONSTANT_ATTENUATION, 0.5f);
    //gl::Luz(GL_LIGHT0 + id_luz, GL_LINEAR_ATTENUATION, -0.53f);
    gl::Luz(GL_LIGHT0 + id_luz, GL_QUADRATIC_ATTENUATION, 0.02f);
    gl::Habilita(GL_LIGHT0 + id_luz);
    pd->set_luz_corrente(id_luz + 1);
  }
}

void Entidade::DesenhaAura(ParametrosDesenho* pd) {
  if (!proto_.visivel() && !pd->modo_mestre()) {
    return;
  }
  if (!pd->desenha_aura() || !proto_.has_aura() || proto_.aura() == 0) {
    return;
  }
  gl::MatrizEscopo salva_matriz;
  gl::Translada(X(), Y(), Z() + DeltaVoo(vd_));
  const auto& cor = proto_.cor();
  gl::MudaCor(cor.r(), cor.g(), cor.b(), cor.a() * 0.2f);
  float ent_quadrados = MultiplicadorTamanho();
  if (ent_quadrados < 1.0f) {
    ent_quadrados = 1.0f;
  }
  // A aura estende alem do tamanho da entidade.
  gl::EsferaSolida(
      TAMANHO_LADO_QUADRADO_2 * ent_quadrados + TAMANHO_LADO_QUADRADO * proto_.aura(),
      NUM_FACES, NUM_FACES);
}

void Entidade::DesenhaSombra(ParametrosDesenho* pd, const float* matriz_shear) {
  if (!proto_.visivel() && !pd->modo_mestre()) {
    return;
  }
  gl::Habilita(GL_POLYGON_OFFSET_FILL);
  gl::DesvioProfundidade(-1.0f, -60.0f);
  DesenhaObjeto(pd, matriz_shear);
  gl::Desabilita(GL_POLYGON_OFFSET_FILL);
}

float Entidade::MultiplicadorTamanho() const {
  return CalculaMultiplicador(proto_.tamanho());
}

void Entidade::AtualizaProximaSalvacao(ResultadoSalvacao rs) {
  proto_.set_proxima_salvacao(rs);
}

float Entidade::CalculaMultiplicador(TamanhoEntidade tamanho) {
  switch (tamanho) {
    case ent::TM_MINUSCULO: return 0.4f;
    case ent::TM_DIMINUTO: return 0.5f;
    case ent::TM_MIUDO: return 0.6f;
    case ent::TM_PEQUENO: return 0.7f;
    case ent::TM_MEDIO: return 1.0f;
    case ent::TM_GRANDE: return 2.0f;
    case ent::TM_ENORME: return 3.0f;
    case ent::TM_IMENSO: return 4.0f;
    case ent::TM_COLOSSAL: return 5.0f;
  }
  LOG(ERROR) << "Tamanho inválido: " << tamanho;
  return 1.0f;
}

void Entidade::AtualizaDirecaoDeQueda(float x, float y, float z) {
  Posicao v;
  v.set_x(x);
  v.set_y(y);
  v.set_z(z);
  proto_.mutable_direcao_queda()->Swap(&v);
}

}  // namespace ent
