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
#include "goog/stringprintf.h"
#include "net/util.h"

#include "log/log.h"

namespace gl {
bool ImprimeSeErro();
}  // namespace gl

namespace ent {
namespace {
using google::protobuf::StringPrintf;
}  // namespace

// Factory.
Entidade* NovaEntidade(
    const EntidadeProto& proto,
    const Tabelas& tabelas, const Texturas* texturas, const m3d::Modelos3d* m3d, ntf::CentralNotificacoes* central, const ParametrosDesenho* pd) {
  switch (proto.tipo()) {
    case TE_COMPOSTA:
    case TE_ENTIDADE:
    case TE_FORMA: {
      auto* entidade = new Entidade(tabelas, texturas, m3d, central, pd);
      entidade->Inicializa(proto);
      return entidade;
    }
    default:
      std::ostringstream oss;
      oss << "Tipo de entidade inválido: " << proto.tipo();
      throw std::logic_error(oss.str());
  }
}

// Entidade
Entidade::Entidade(
    const Tabelas& tabelas, const Texturas* texturas, const m3d::Modelos3d* m3d, ntf::CentralNotificacoes* central, const ParametrosDesenho* pd)
    : tabelas_(tabelas) {
  vd_.texturas = texturas;
  vd_.m3d = m3d;
  parametros_desenho_ = pd;
  central_ = central;
}

Entidade::~Entidade() {
  EntidadeProto dummy;
  AtualizaTexturas(dummy);
  AtualizaModelo3d(dummy);
}

namespace {

// A variavei translacao_z foi deprecada e nao devera mais ser utilizada. Esta funcao a converte para possiveis
// modelos que venham a aparecer com ela.
void CorrigeTranslacaoDeprecated(EntidadeProto* proto) {
  if (proto->has_translacao_z_deprecated()) {
    proto->mutable_pos()->set_z(proto->pos().z() + proto->translacao_z_deprecated());
    proto->clear_translacao_z_deprecated();
  }
  for (auto& proto_filho : *proto->mutable_sub_forma()) {
    CorrigeTranslacaoDeprecated(&proto_filho);
  }
}

void CorrigeAuraDeprecated(EntidadeProto* proto) {
  if (proto->has_aura()) {
    proto->set_aura_m(proto->aura() * TAMANHO_LADO_QUADRADO);
    proto->clear_aura();
  }
}

void CorrigeCamposDeprecated(EntidadeProto* proto) {
  CorrigeAuraDeprecated(proto);
  CorrigeTranslacaoDeprecated(proto);
}

}  // namespace

bool Entidade::TemTipoDnD(TipoDnD tipo) const {
  return std::any_of(proto_.tipo_dnd().begin(), proto_.tipo_dnd().end(),
      [tipo] (const int t) { return t == tipo; });
}

bool Entidade::TemSubTipoDnD(SubTipoDnD sub_tipo) const {
  return std::any_of(proto_.sub_tipo_dnd().begin(), proto_.sub_tipo_dnd().end(),
      [sub_tipo] (const int st) { return st == sub_tipo; });
}

void Entidade::CorrigeVboRaiz(const ent::EntidadeProto& proto, VariaveisDerivadas* vd) {
  Matrix4 m;
  m.translate(-proto.pos().x(), -proto.pos().y(), -proto.pos().z());
  m.rotateZ(-proto.rotacao_z_graus());
  m.rotateY(-proto.rotacao_y_graus());
  m.rotateX(-proto.rotacao_x_graus());
  if (proto.tipo() != TE_FORMA || proto.sub_tipo() != TF_LIVRE) {
    m.scale(1.0f / proto.escala().x(), 1.0f / proto.escala().y(), 1.0f / proto.escala().z());
  }
  vd->vbos_nao_gravados.Multiplica(m);
#if 0
  for (auto& vbo : vd->vbos) {
    vbo.Translada(-proto.pos().x(), -proto.pos().y(), -proto.pos().z());
    vbo.RodaZ(-proto.rotacao_z_graus());
    vbo.RodaY(-proto.rotacao_y_graus());
    vbo.RodaX(-proto.rotacao_x_graus());
    if (proto.tipo() != TE_FORMA || proto.sub_tipo() != TF_LIVRE) {
      vbo.Escala(1.0f / proto.escala().x(), 1.0f / proto.escala().y(), 1.0f / proto.escala().z());
    }
  }
#endif
}

void Entidade::Inicializa(const EntidadeProto& novo_proto) {
  // Preciso do tipo aqui para atualizar as outras coisas de acordo.
  proto_.set_tipo(novo_proto.tipo());
  // Atualiza texturas e modelos 3d antes de tudo.
  AtualizaTexturas(novo_proto);
  AtualizaModelo3d(novo_proto);
  // mantem o tipo.
  proto_.CopyFrom(novo_proto);
  CorrigeCamposDeprecated(&proto_);
  if (proto_.has_dados_vida() && !proto_.has_max_pontos_vida()) {
    // Geracao automatica de pontos de vida.
    try {
      int pv;
      std::tie(pv, std::ignore) = GeraPontosVida(proto_.dados_vida());
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
  // Evitar oscilacoes juntas.
  vd_.angulo_disco_luz_rad = ((RolaDado(360) - 1.0f) / 180.0f) * M_PI;
  if (proto_.tipo() == TE_FORMA) {
    InicializaForma(proto_, &vd_);
  } else if (proto_.tipo() == TE_COMPOSTA) {
    InicializaComposta(proto_, &vd_);
  }

  AtualizaVbo(parametros_desenho_);
  RecomputaDependencias(tabelas_, &proto_);
  proto_.clear_proxima_salvacao();
}

// static
gl::VbosNaoGravados Entidade::ExtraiVbo(const ent::EntidadeProto& proto, const ParametrosDesenho* pd, bool mundo) {
  return ExtraiVbo(proto, VariaveisDerivadas(), pd, mundo);
}

// static
gl::VbosNaoGravados Entidade::ExtraiVbo(const ent::EntidadeProto& proto, const VariaveisDerivadas& vd, const ParametrosDesenho* pd, bool mundo) {
  if (proto.tipo() == TE_ENTIDADE) {
    return ExtraiVboEntidade(proto, vd, pd, mundo);
  } else if (proto.tipo() == TE_COMPOSTA) {
    return ExtraiVboComposta(proto, vd, pd, mundo);
  } else {
    return ExtraiVboForma(proto, vd, pd, mundo);
  }
}

// static
gl::VbosNaoGravados Entidade::ExtraiVboEntidade(const ent::EntidadeProto& proto, const VariaveisDerivadas& vd, const ParametrosDesenho* pd, bool mundo) {
  if (proto.has_modelo_3d()) {
    gl::VbosNaoGravados vbos;
    const auto* modelo_3d = vd.m3d->Modelo(proto.modelo_3d().id());
    if (modelo_3d != nullptr && modelo_3d->Valido()) {
      vbos.CopiaDe(modelo_3d->vbos_nao_gravados);
      //LOG_EVERY_N(INFO, 10) << "VBO: " << vbos.ParaString(true);
      if (mundo) {
        vbos.Multiplica(MontaMatrizModelagem(true  /*queda*/, true /*trans z*/, proto, vd, pd));
      }
    }
    // Aqui pode retornar o vbo vazio, para o caso de nao ter carregado ainda.
    return vbos;
  }

  // desenha o cone com NUM_FACES faces com raio de RAIO e altura ALTURA
  //const auto& pos = proto.pos();
  if (proto.info_textura().id().empty()) {
    gl::VboNaoGravado vbo = gl::VboConeSolido(TAMANHO_LADO_QUADRADO_2 - 0.2, ALTURA, NUM_FACES, NUM_LINHAS);
    gl::VboNaoGravado vbo_esfera = gl::VboEsferaSolida(TAMANHO_LADO_QUADRADO_2 - 0.4, NUM_FACES, NUM_FACES / 2.0f);
    // Translada todos os Z da esfera em ALTURA.
    for (unsigned int i = 2; i < vbo_esfera.coordenadas().size(); i += vbo_esfera.NumDimensoes()) {
      vbo_esfera.coordenadas()[i] += ALTURA;
    }
    vbo.Concatena(vbo_esfera);
    vbo.AtribuiCor(proto.cor().r(), proto.cor().g(), proto.cor().b(), proto.cor().a());
    if (mundo) {
      vbo.Multiplica(MontaMatrizModelagem(true /*queda*/, true /*trans z*/, proto, vd, pd));
    }
    return gl::VbosNaoGravados(std::move(vbo));
  }

  // tijolo da tela de textura (moldura).
  gl::VboNaoGravado vbo_moldura;
  if (!pd->desenha_texturas_para_cima()) {
    vbo_moldura = gl::VboCuboSolido(TAMANHO_LADO_QUADRADO);
    vbo_moldura.Escala(proto.info_textura().largura(), 0.1f, proto.info_textura().altura());
    vbo_moldura.Translada(0, 0, (TAMANHO_LADO_QUADRADO_2 + TAMANHO_LADO_QUADRADO_10) - (1.0f - proto.info_textura().altura()));
    if (mundo) {
      vbo_moldura.RodaZ(vd.angulo_rotacao_textura_graus);
      vbo_moldura.Multiplica(MontaMatrizModelagem(true  /*queda*/, true /*trans z*/, proto, vd, pd));
    }
  }

  // tijolo da base (altura TAMANHO_LADO_QUADRADO_10).
  if (!proto.morta() && mundo) {
    gl::VboNaoGravado vbo_base = gl::VboCuboSolido(TAMANHO_LADO_QUADRADO);
    if (pd->entidade_selecionada()) {
      vbo_base.RodaZ(vd.angulo_disco_selecao_graus);
    }
    vbo_base.Escala(0.8f, 0.8f, TAMANHO_LADO_QUADRADO_10 / 2);
    vbo_base.Translada(0.0, 0.0, TAMANHO_LADO_QUADRADO_10 / 4);
    vbo_base.Multiplica(MontaMatrizModelagem(
        true,  // queda.
        (vd.altura_voo == 0.0f)  /*z*/,  // so desloca tijolo se nao estiver voando.
        proto, vd, pd));
    vbo_moldura.Concatena(vbo_base);
  }

  return gl::VbosNaoGravados(std::move(vbo_moldura));
}

void Entidade::AtualizaVbo(const ParametrosDesenho* pd) {
  // A atualizacao deve ser feita apenas para os tipos corretos.
#if !VBO_COM_MODELAGEM
  switch (proto_.tipo()) {
    case TE_ENTIDADE: return;  // entidades simples nao possuem vbo.
    case TE_FORMA:
      // apenas LIVRE possui VBO proprio.
      if (proto_.sub_tipo() != TF_LIVRE) {
        return;
      }
      // Compostas sempre possuem VBO proprio.
    case TE_COMPOSTA:
    default: ;
  }
#endif
  vd_.vbos_nao_gravados = ExtraiVbo(pd == nullptr ? &ParametrosDesenho::default_instance() : pd, VBO_COM_MODELAGEM);
  if (!vd_.vbos_nao_gravados.Vazio()) {
    vd_.vbos_gravados.Grava(vd_.vbos_nao_gravados);
  }
  V_ERRO("Erro atualizacao de VBOs");
}

void Entidade::AtualizaModelo3d(const EntidadeProto& novo_proto) {
  VLOG(2) << "Atualizando modelo3d novo proto: " << novo_proto.ShortDebugString() << ", velho: " << proto_.ShortDebugString();
  // Libera textura anterior se houver e for diferente da corrente.
  if (!proto_.modelo_3d().id().empty() &&
      proto_.modelo_3d().id() != novo_proto.modelo_3d().id()) {
    VLOG(1) << "Liberando modelo_3d: " << proto_.modelo_3d().id();
    auto nl = ntf::NovaNotificacao(ntf::TN_DESCARREGAR_MODELO_3D);
    nl->mutable_entidade()->mutable_modelo_3d()->set_id(proto_.modelo_3d().id());
    central_->AdicionaNotificacao(nl.release());
  }
  // Carrega modelo_3d se houver e for diferente da antiga.
  if (!novo_proto.modelo_3d().id().empty() &&
      novo_proto.modelo_3d().id() != proto_.modelo_3d().id()) {
    VLOG(1) << "Carregando modelo_3d: " << novo_proto.modelo_3d().id();
    auto nc = ntf::NovaNotificacao(ntf::TN_CARREGAR_MODELO_3D);
    *nc->mutable_entidade()->mutable_modelo_3d() = novo_proto.modelo_3d();
    central_->AdicionaNotificacao(nc.release());
  }
  if (!novo_proto.modelo_3d().id().empty()) {
    *proto_.mutable_modelo_3d() = novo_proto.modelo_3d();
  } else {
    proto_.clear_modelo_3d();
  }
}

void Entidade::AtualizaTexturas(const EntidadeProto& novo_proto) {
  AtualizaTexturasProto(novo_proto, &proto_, central_);
}

void Entidade::AtualizaTexturasProto(const EntidadeProto& novo_proto, EntidadeProto* proto_atual, ntf::CentralNotificacoes* central) {
  VLOG(2) << "Novo proto: " << novo_proto.ShortDebugString() << ", velho: " << proto_atual->ShortDebugString();
  // Libera textura anterior se houver e for diferente da corrente.
  if (proto_atual->info_textura().id().size() > 0  && proto_atual->info_textura().id() != novo_proto.info_textura().id()) {
    VLOG(1) << "Liberando textura: " << proto_atual->info_textura().id();
    auto nl = ntf::NovaNotificacao(ntf::TN_DESCARREGAR_TEXTURA);
    nl->add_info_textura()->set_id(proto_atual->info_textura().id());
    central->AdicionaNotificacao(nl.release());
  }
  // Carrega textura se houver e for diferente da antiga.
  if (novo_proto.has_info_textura() && !novo_proto.info_textura().id().empty() && novo_proto.info_textura().id() != proto_atual->info_textura().id()) {
    VLOG(1) << "Carregando textura: " << proto_atual->info_textura().id();
    auto nc = ntf::NovaNotificacao(ntf::TN_CARREGAR_TEXTURA);
    nc->add_info_textura()->CopyFrom(novo_proto.info_textura());
    central->AdicionaNotificacao(nc.release());
  }
  if (novo_proto.info_textura().id().size() > 0) {
    proto_atual->mutable_info_textura()->CopyFrom(novo_proto.info_textura());
  } else {
    proto_atual->clear_info_textura();
  }
}

void Entidade::AtualizaProto(const EntidadeProto& novo_proto) {
  VLOG(1) << "Proto antes: " << proto_.ShortDebugString();
  AtualizaTexturas(novo_proto);
  AtualizaModelo3d(novo_proto);

  // mantem o id, posicao (exceto Z) e destino.
  ent::EntidadeProto proto_original(proto_);

  // Eventos.
  // Os valores sao colocados para -1 para o RecomputaDependencias conseguir limpar os que estao sendo removidos.
  {
    // As duracoes -1 serao retiradas ao recomputar dependencias. Os demais serao adicionados no merge.
    std::vector<int> a_remover;
    int i = 0;
    for (auto& evento : *proto_original.mutable_evento()) {
      if (!PossuiEventoEspecifico(novo_proto, evento)) {
        evento.set_rodadas(-1);
      } else {
        // Remove porque estas virao do proto novo.
        a_remover.push_back(i);
      }
      ++i;
    }
    for (auto it = a_remover.rbegin(); it != a_remover.rend(); ++it) {
      proto_original.mutable_evento()->DeleteSubrange(*it, 1);
    }
  }
  // Aqui atribui
  proto_ = novo_proto;
  // Daqui pra baixo, correcoes manuais.


  if (proto_.pontos_vida() > proto_.max_pontos_vida()) {
    proto_.set_pontos_vida(proto_.max_pontos_vida());
  }

  // Deixa os eventos de duracao -1 no comeco.
  proto_original.mutable_evento()->MergeFrom(proto_.evento());
  proto_.mutable_evento()->Swap(proto_original.mutable_evento());

  proto_.set_id(proto_original.id());
  proto_.set_tipo(proto_original.tipo());
  proto_.mutable_pos()->Swap(proto_original.mutable_pos());
  proto_.mutable_pos()->set_z(novo_proto.pos().z());
  if (proto_original.has_destino()) {
    proto_.mutable_destino()->Swap(proto_original.mutable_destino());
  }
  if (proto_original.tipo() == TE_ENTIDADE) {
    *proto_.mutable_escala() = proto_original.escala();
    float fator = novo_proto.escala().x() / proto_original.escala().x();
    int delta = 0;
    if (fator > 1.1f) {
      delta = 1;
    } else if (fator < 0.9f) {
      delta = -1;
    }
    int base_corrente = BonusIndividualTotal(TB_BASE, proto_original.bonus_tamanho());
    if (delta != 0) AtribuiBonus(base_corrente + delta, TB_BASE, "base", proto_.mutable_bonus_tamanho());
  }
  if (proto_.transicao_cenario().id_cenario() == CENARIO_INVALIDO) {
    proto_.clear_transicao_cenario();
  }
#if 0
  if (proto_.tipo() == TE_FORMA) {
    AtualizaProtoForma(proto_original, proto_, &vd_);
  } else if (proto_.tipo() == TE_COMPOSTA) {
    AtualizaProtoComposta(proto_original, proto_, &vd_);
  }
#endif
  AtualizaVbo(parametros_desenho_);
  RecomputaDependencias(tabelas_, &proto_);
  VLOG(1) << "Proto depois: " << proto_.ShortDebugString();
}

void Entidade::AtualizaEfeitos() {
  // Efeitos.
  vd_.nao_desenhar = false;
  std::unordered_set<int> a_remover;
  for (auto& efeito_vd : vd_.complementos_efeitos) {
    a_remover.insert(efeito_vd.first);
  }
  for (const auto& evento : proto_.evento()) {
    if (!evento.has_id_efeito() || evento.id_efeito() == EFEITO_INVALIDO) {
      continue;
    }
    AtualizaEfeito(static_cast<TipoEfeito>(evento.id_efeito()), &vd_.complementos_efeitos[evento.id_efeito()]);
    a_remover.erase(evento.id_efeito());
  }
  for (const auto& id_remocao : a_remover) {
    vd_.complementos_efeitos.erase(id_remocao);
  }
}

void Entidade::AtualizaEfeito(TipoEfeito id_efeito, ComplementoEfeito* complemento) {
  switch (id_efeito) {
    case EFEITO_PISCAR:
      if (++complemento->quantidade >= 40) {
        vd_.nao_desenhar = true;
        if (complemento->quantidade >= 60) {
          complemento->quantidade = 0;
        }
      }
      break;
    default:
      ;
  }
}

void Entidade::AtualizaLuzAcao(int intervalo_ms) {
  auto& luz = vd_.luz_acao;
  if (!luz.inicio.has_raio_m()) return;
  if (luz.tempo_desde_inicio_ms > luz.duracao_ms) {
    LOG(INFO) << "luz acao desligada";
    luz.corrente.Clear();
    luz.inicio.Clear();
    return;
  }
  const float fator_decaimento = (luz.duracao_ms - luz.tempo_desde_inicio_ms) / static_cast<float>(luz.duracao_ms);
  luz.corrente.mutable_cor()->set_r(luz.inicio.cor().r() * fator_decaimento);
  luz.corrente.mutable_cor()->set_g(luz.inicio.cor().g() * fator_decaimento);
  luz.corrente.mutable_cor()->set_b(luz.inicio.cor().b() * fator_decaimento);
  luz.tempo_desde_inicio_ms += intervalo_ms;
}

void Entidade::AtualizaFumaca(int intervalo_ms) {
  auto& f = vd_.fumaca;
  f.duracao_ms -= intervalo_ms;
  if (f.duracao_ms < 0) {
    f.duracao_ms = 0;
  }
  bool fim = f.duracao_ms == 0;
  if (fim && PossuiEvento(EFEITO_QUEIMANDO_FOGO_ALQUIMICO, proto_)) {
    AtivaFumegando(1000);
    AtualizaFumaca(intervalo_ms);
    return;
  }
  if (!fim && intervalo_ms >= f.proxima_emissao_ms) {
    f.proxima_emissao_ms = f.proxima_emissao_ms;
    // Emite nova particula.
    DadosUmaNuvem nuvem;
    nuvem.direcao.z = 1.0f;
    nuvem.pos = PosParaVector3(PosicaoAltura(1.0f));
    nuvem.duracao_ms = f.duracao_nuvem_ms;
    nuvem.velocidade_m_s = 0.25f;
    nuvem.escala = 1.0f;
    f.nuvens.emplace_back(std::move(nuvem));
    f.proxima_emissao_ms = f.intervalo_emissao_ms;
  } else {
    f.proxima_emissao_ms -= intervalo_ms;
  }
  // Atualiza as particulas existentes.
  std::vector<unsigned int> a_remover;
  float intervalo_s = intervalo_ms / 1000.0f;
  for (unsigned int i = 0; i < f.nuvens.size(); ++i) {
    auto& nuvem = f.nuvens[i];
    nuvem.duracao_ms -= intervalo_ms;
    if (nuvem.duracao_ms <= 0) {
      a_remover.push_back(i);
      continue;
    }
    nuvem.pos += nuvem.direcao * nuvem.velocidade_m_s * intervalo_s;
    nuvem.escala += 1.5f * intervalo_s;
    nuvem.alfa = static_cast<float>(nuvem.duracao_ms) / f.intervalo_emissao_ms;
  }
  // Remove as que tem que remover.
  unsigned int removidas = 0;
  for (int i : a_remover) {
    f.nuvens.erase(f.nuvens.begin() + (i - removidas));
  }
  // Recria o VBO.
  std::vector<gl::VboNaoGravado> vbos;
  for (const auto& nuvem : f.nuvens) {
    gl::VboNaoGravado vbo_ng = gl::VboRetangulo(0.2f * MultiplicadorTamanho());
    Vector3 camera = PosParaVector3(parametros_desenho_->pos_olho());
    Vector3 dc = camera - nuvem.pos;
    // Primeiro inclina para a camera. O objeto eh deitado no plano Z, entao tem que rodar 90.0f de cara
    // mais a diferenca de angulo.
    float inclinacao_graus = 0.0f;
    float dc_len = dc.length();
    if (dc_len < 0.001f) {
      inclinacao_graus = 0.0f;
    } else {
      inclinacao_graus = asinf(dc.z / dc_len) * RAD_PARA_GRAUS;
    }
    vbo_ng.RodaY(90.0f - inclinacao_graus);
    // Agora roda no eixo z.
    vbo_ng.RodaZ(VetorParaRotacaoGraus(dc.x, dc.y));
    vbo_ng.Escala(nuvem.escala, nuvem.escala, nuvem.escala);
    vbo_ng.Translada(nuvem.pos.x, nuvem.pos.y, nuvem.pos.z);
    vbo_ng.AtribuiCor(1.0f, 1.0f, 1.0f, nuvem.alfa);
    vbos.emplace_back(std::move(vbo_ng));
  }
  f.vbo = gl::VbosNaoGravados(std::move(vbos));
}

void Entidade::AtualizaMatrizes() {
  MatrizesDesenho md = GeraMatrizesDesenho(proto_, vd_, parametros_desenho_);
  vd_.matriz_modelagem = md.modelagem;
  vd_.matriz_modelagem_tijolo_base = md.tijolo_base;
  vd_.matriz_modelagem_tijolo_tela = md.tijolo_tela;
  vd_.matriz_modelagem_tela_textura = md.tela_textura;
  vd_.matriz_deslocamento_textura = md.deslocamento_textura;
}

// static
Entidade::MatrizesDesenho Entidade::GeraMatrizesDesenho(const EntidadeProto& proto, const VariaveisDerivadas& vd, const ParametrosDesenho* pd) {
  MatrizesDesenho md;
  Matrix4 matriz_modelagem_geral = MontaMatrizModelagem(true, true, proto, vd, pd);
  md.modelagem = matriz_modelagem_geral * Matrix4().rotateZ(vd.angulo_rotacao_textura_graus);
  if (proto.tipo() != TE_ENTIDADE || (proto.has_modelo_3d() && !proto.desenha_base())) {
    return md;
  }
  // tijolo base. Usada para disco de peao tambem.
  if (DesenhaBase(proto) || (!proto.has_info_textura() && !proto.has_modelo_3d())) {
    Matrix4 m;
    if (pd->entidade_selecionada()) {
      m.rotateZ(vd.angulo_disco_selecao_graus);
    }
    md.tijolo_base = MontaMatrizModelagem(true  /*queda*/, TZ_SEM_VOO  /*z*/, proto, vd, pd) * m;
  }
  if (!proto.info_textura().id().empty()) {
    bool achatar = Achatar(proto, pd);
    // tijolo tela.
    if (!achatar) {
      Matrix4 m;
      m.scale(proto.info_textura().largura(), 1.0f, proto.info_textura().altura());
      m.rotateZ(vd.angulo_rotacao_textura_graus);
      md.tijolo_tela = matriz_modelagem_geral  * m;
    }
    // tela.
    {
      Matrix4 m;
      if (achatar) {
        m.translate(0.0, 0.0, -(TAMANHO_LADO_QUADRADO_10 + TAMANHO_LADO_QUADRADO_2));
        m.rotateX(-90.0f);
        m.scale(proto.info_textura().largura(), proto.info_textura().altura(), 1.0f);
        m.translate(0.0f, 0.0f, TAMANHO_LADO_QUADRADO_10);
      } else {
        m.scale(proto.info_textura().largura(), 1.0f, proto.info_textura().altura());
        m.rotateZ(vd.angulo_rotacao_textura_graus);
      }
      md.tela_textura = matriz_modelagem_geral * m;
    }
    // Deslocamento de textura.
    {
      Matrix4 m;
      m.scale(proto.info_textura().largura(), proto.info_textura().altura(), 1.0f);
      m.translate(proto.info_textura().translacao_x(), proto.info_textura().translacao_y(), 0.0f);
      md.deslocamento_textura = m;
    }
  }
  return md;
}


void Entidade::Atualiza(int intervalo_ms, boost::timer::cpu_timer* timer) {
#if DEBUG
  glFinish();
#endif
  timer->stop();

  // Ao retornar, atualiza o vbo se necessario.
  struct AtualizaEscopo {
    AtualizaEscopo(Entidade* e) : e(e) {}
    ~AtualizaEscopo() {
      e->AtualizaMatrizes();
      if (atualizar) e->AtualizaVbo(e->parametros_desenho_);
    }
    Entidade* e;
    bool atualizar = false;
  } vbo_escopo(this);

  if (parametros_desenho_->regera_vbo()) {
    vbo_escopo.atualizar = true;
  }
  if (parametros_desenho_->entidade_selecionada() && Tipo() == TE_ENTIDADE && !proto_.has_modelo_3d()) {
#if VBO_COM_MODELAGEM
    vbo_escopo.atualizar = true;
#endif
    vd_.angulo_disco_selecao_graus = fmod(vd_.angulo_disco_selecao_graus + 1.0, 360.0);
  }

  AtualizaEfeitos();
  AtualizaFumaca(intervalo_ms);
  AtualizaLuzAcao(intervalo_ms);
  if (parametros_desenho_->iniciativa_corrente()) {
    const float DURACAO_OSCILACAO_MS = 4000.0f;
    const float DELTA_ANGULO_INICIATIVA = 2.0f * M_PI * intervalo_ms / DURACAO_OSCILACAO_MS;
    vd_.angulo_disco_iniciativa_rad = fmod(vd_.angulo_disco_iniciativa_rad + DELTA_ANGULO_INICIATIVA, 2 * M_PI);
  }
  // Espiada vai ate 45 graus.
  const float DURACAO_ESPIADA_MS = 250.0f;
  const float DELTA_ESPIADA = intervalo_ms / DURACAO_ESPIADA_MS;
  if (proto_.espiando() != 0) {
    // Espiando.
    if (fabs(vd_.progresso_espiada_) < 1.0f) {
      vd_.progresso_espiada_ += proto_.espiando() * DELTA_ESPIADA;
    }
  } else {
    // Nao esta espiando.
    if (vd_.progresso_espiada_ != 0) {
      float delta = vd_.progresso_espiada_ > 0 ? DELTA_ESPIADA : -DELTA_ESPIADA;
      vd_.progresso_espiada_ = fabs(vd_.progresso_espiada_) > DELTA_ESPIADA ? vd_.progresso_espiada_ - delta : 0.0f;
    }
  }
  //const unsigned int INTERVALO_ZERAR_ATAQUES_MS = 3000;
  //vd_.ultimo_ataque_ms += intervalo_ms;
  //if (vd_.ultimo_ataque_ms > INTERVALO_ZERAR_ATAQUES_MS) {
  //  ReiniciaAtaque();
  //}

  // Voo.
  const float DURACAO_POSICIONAMENTO_INICIAL_MS = 1000.0f;
  const float DURACAO_VOO_MS = 4000.0f;
  const float DELTA_VOO = 2.0f * M_PI * intervalo_ms / DURACAO_VOO_MS;
  const float DURACAO_LUZ_MS = 3000.0f;
  const float DELTA_LUZ = 2.0f * M_PI * intervalo_ms / DURACAO_LUZ_MS;
  if (proto_.has_luz()) {
    vd_.angulo_disco_luz_rad = fmod(vd_.angulo_disco_luz_rad + DELTA_LUZ, 2 * M_PI);
  }
  if (proto_.voadora()) {
#if VBO_COM_MODELAGEM
    vbo_escopo.atualizar = true;
#endif
    if (vd_.altura_voo < ALTURA_VOO) {
      if (vd_.altura_voo == 0.0f) {
        vd_.angulo_disco_voo_rad = 0.0f;
      }
      // Decolando, ate chegar na altura do voo.
      vd_.altura_voo += ALTURA_VOO * static_cast<float>(intervalo_ms) / DURACAO_POSICIONAMENTO_INICIAL_MS;
      if (vd_.altura_voo > ALTURA_VOO) {
        vd_.altura_voo = ALTURA_VOO;
      }
    } else {
      // Chegou na altura do voo, flutua.
      vd_.angulo_disco_voo_rad = fmod(vd_.angulo_disco_voo_rad + DELTA_VOO, 2 * M_PI);
    }
  } else {
    if (vd_.altura_voo > 0) {
#if VBO_COM_MODELAGEM
      vbo_escopo.atualizar = true;
#endif
      const float DECREMENTO = ALTURA_VOO * static_cast<float>(intervalo_ms) / DURACAO_POSICIONAMENTO_INICIAL_MS;
      if (Z() > proto_.z_antes_voo()) {
        proto_.mutable_pos()->set_z(Z() - DECREMENTO);
      } else {
        proto_.mutable_pos()->set_z(proto_.z_antes_voo());
        // Nao eh voadora e esta suspensa. Pousando.
        vd_.altura_voo -= DECREMENTO;
        if (Z() < proto_.z_antes_voo()) {
          proto_.mutable_pos()->set_z(proto_.z_antes_voo());
        }
      }
    } else {
      vd_.altura_voo = 0;
    }
    vd_.angulo_disco_voo_rad = 0.0f;
  }
  if (Tipo() == TE_ENTIDADE && !proto_.has_modelo_3d() &&
      !proto_.info_textura().id().empty()) {
    float angulo = 0.0f;
    if (proto_.caida()) {
      angulo = 0.0f;
    } else if (!Achatar() && parametros_desenho_->texturas_sempre_de_frente()) {
      double dx = proto_.pos().x() - parametros_desenho_->pos_olho().x();
      double dy = proto_.pos().y() - parametros_desenho_->pos_olho().y();
      double r = sqrt(pow(dx, 2) + pow(dy, 2));
      angulo = r > 0.1f ? (acosf(dx / r) * RAD_PARA_GRAUS) : 0.0f;
      if (dy < 0) {
        angulo = -angulo;
      }
      angulo = angulo - 90.0f;
    } else {
      angulo = proto_.rotacao_z_graus();
    }
    if (fabs(angulo - vd_.angulo_rotacao_textura_graus) > 0.1f) {
      vd_.angulo_rotacao_textura_graus = angulo;
#if VBO_COM_MODELAGEM
      vbo_escopo.atualizar = true;
#endif
      //LOG(INFO) << "atualizou angulo: " << angulo;
    }
  }

  // Queda.
  const double DURACAO_QUEDA_MS = 500.0f;
  const float DELTA_QUEDA = (static_cast<float>(intervalo_ms) / DURACAO_QUEDA_MS) * 90.0f;
  if (proto_.caida()) {
    if (vd_.angulo_disco_queda_graus < 90.0f) {
#if VBO_COM_MODELAGEM
      vbo_escopo.atualizar = true;
#endif
      vd_.angulo_disco_queda_graus += DELTA_QUEDA;
      if (vd_.angulo_disco_queda_graus > 90.0f) {
        vd_.angulo_disco_queda_graus = 90.0f;
      }
    }
  } else {
    if (vd_.angulo_disco_queda_graus > 0) {
#if VBO_COM_MODELAGEM
      vbo_escopo.atualizar = true;
#endif
      vd_.angulo_disco_queda_graus -= DELTA_QUEDA;
      if (vd_.angulo_disco_queda_graus < 0) {
        vd_.angulo_disco_queda_graus = 0.0f;
      }
    }
  }

  if (proto_.has_modelo_3d() && vd_.vbos_nao_gravados.Vazio()) {
    vbo_escopo.atualizar = true;
  }

  // Daqui pra baixo, tratamento de destino.
  if (!proto_.has_destino()) {
    return;
  }
#if VBO_COM_MODELAGEM
  vbo_escopo.atualizar = true;
#endif
  auto* po = proto_.mutable_pos();
  const auto& pd = proto_.destino();
  if (proto_.destino().has_id_cenario()) {
    bool mudou_cenario = proto_.destino().id_cenario() != proto_.pos().id_cenario();
    proto_.mutable_pos()->set_id_cenario(proto_.destino().id_cenario());
    if (mudou_cenario) {
      po->set_x(pd.x());
      po->set_y(pd.y());
      po->set_z(pd.z());
      proto_.clear_destino();
      return;
    }
  }
  bool chegou = true;
  // deslocamento em cada eixo (x, y, z) por chamada de atualizacao.
  const float DESLOCAMENTO = TAMANHO_LADO_QUADRADO * (intervalo_ms / 1000.0f);  // anda 4 quadrados em 1s.
  VLOG(3) << "po antes: " << po->ShortDebugString() << ", pd antes: " << pd.ShortDebugString();
  Vector3 d(pd.x(), pd.y(), pd.z());
  Vector3 o(po->x(), po->y(), po->z());
  d -= o;
  float falta = d.length();
  if (falta <= DESLOCAMENTO) {
  } else if (falta > 5.0f * DESLOCAMENTO) {
    d *= 5.0f * DESLOCAMENTO / falta;  // anda 5 * DESLOCAMENTO.
    chegou = false;
  } else {
    d *= DESLOCAMENTO / falta;  // anda DESLOCAMENTO.
    chegou = false;
  }
  po->set_x(o.x + d.x);
  po->set_y(o.y + d.y);
  po->set_z(o.z + d.z);
  VLOG(3) << "pos: " << po->ShortDebugString();

  if (chegou) {
    proto_.clear_destino();
  }
}

void Entidade::MovePara(float x, float y, float z) {
  VLOG(1) << "Entidade antes de mover: " << proto_.pos().ShortDebugString();
  auto* p = proto_.mutable_pos();
  p->set_x(x);
  p->set_y(y);
  p->set_z(z /*std::max(ZChao(x, y), z)*/);
  proto_.clear_destino();
  VLOG(1) << "Movi entidade para: " << proto_.pos().ShortDebugString();
#if VBO_COM_MODELAGEM
  AtualizaVbo(parametros_desenho_);
#endif
}

void Entidade::MovePara(const Posicao& pos) {
  VLOG(1) << "Entidade antes de mover: " << proto_.pos().ShortDebugString();
  *proto_.mutable_pos() = pos;
  proto_.clear_destino();
  VLOG(1) << "Movi entidade para: " << proto_.pos().ShortDebugString();
#if VBO_COM_MODELAGEM
  AtualizaVbo(parametros_desenho_);
#endif
}

void Entidade::MoveDelta(float dx, float dy, float dz) {
  MovePara(X() + dx, Y() + dy, Z() + dz);
}

void Entidade::Destino(const Posicao& pos) {
  proto_.mutable_destino()->CopyFrom(pos);
}

void Entidade::IncrementaZ(float delta) {
  //proto_.set_translacao_z(proto_.translacao_z() + delta);
  proto_.mutable_pos()->set_z(proto_.pos().z() + delta);
#if VBO_COM_MODELAGEM
  AtualizaVbo(parametros_desenho_);
#endif
}

void Entidade::IncrementaRotacaoZGraus(float delta) {
  proto_.set_rotacao_z_graus(proto_.rotacao_z_graus() + delta);
#if VBO_COM_MODELAGEM
  AtualizaVbo(parametros_desenho_);
#endif
}

void Entidade::AlteraRotacaoZGraus(float rotacao_graus) {
  proto_.set_rotacao_z_graus(rotacao_graus);
#if VBO_COM_MODELAGEM
  AtualizaVbo(parametros_desenho_);
#endif
}

EntidadeProto::TipoTransicao Entidade::TipoTransicao() const {
  // Legado.
  if (proto_.has_transicao_cenario()) {
    return EntidadeProto::TRANS_CENARIO;
  }
  return proto_.tipo_transicao();
}

int Entidade::TransicaoCenario() const {
  // Legado.
  if (TipoTransicao() != EntidadeProto::TRANS_CENARIO) {
    return CENARIO_INVALIDO;
  }
  return proto_.transicao_cenario().id_cenario();
}

const Posicao& Entidade::PosTransicao() const {
  return proto_.transicao_cenario();
}

int Entidade::NivelPersonagem() const {
  int total = 0;
  for (const auto& info_classe : proto_.info_classes()) {
    total += info_classe.nivel();
  }
  return total - proto_.niveis_negativos();
}

int Entidade::NivelConjurador() const {
  for (const auto& info_classe : proto_.info_classes()) {
    if (info_classe.nivel_conjurador() > 0) {
      return info_classe.nivel_conjurador();
    }
  }
  return 0;
}

int Entidade::ModificadorAtributoConjuracao() const {
  for (const auto& info_classe : proto_.info_classes()) {
    if (info_classe.nivel_conjurador() > 0) {
      return info_classe.modificador_atributo_conjuracao();
    }
  }
  return 0;
}

int Entidade::BonusBaseAtaque() const {
  int bba = 0;
  for (const auto& info_classe : proto_.info_classes()) {
    bba += info_classe.bba();
  }
  return bba;
}

float Entidade::X() const {
  return proto_.pos().x();
}

float Entidade::Y() const {
  return proto_.pos().y();
}

float Entidade::Z(bool delta_voo) const {
  float delta = delta_voo ? DeltaVoo(vd_) : 0.0f;
  return proto_.pos().z() + delta;
}

float Entidade::ZOlho() const {
  Vector4 ponto(0.0f, 0.0f, proto_.achatado() ? TAMANHO_LADO_QUADRADO_10 : ALTURA, 1.0f);
  return std::max(TAMANHO_LADO_QUADRADO_10, (MontaMatrizModelagem(true  /*queda*/, true  /*z*/, proto_, vd_) * ponto).z);
}

float Entidade::AlturaOlho() const {
  Vector4 ponto(0.0f, 0.0f, proto_.achatado() ? TAMANHO_LADO_QUADRADO_10 : ALTURA, 1.0f);
  return std::max(TAMANHO_LADO_QUADRADO_10, (MontaMatrizModelagem(true  /*queda*/, false  /*z*/, proto_, vd_) * ponto).z);
}

int Entidade::IdCenario() const {
  return proto_.pos().id_cenario();
}

void Entidade::MataEntidade() {
  proto_.set_morta(true);
  proto_.set_caida(true);
  proto_.set_voadora(false);
  proto_.set_aura_m(0.0f);
}

void Entidade::AtualizaPontosVida(int pontos_vida, int dano_nao_letal) {
  if (proto_.max_pontos_vida() == 0) {
    // Entidades sem pontos de vida nao sao afetadas.
    return;
  }
  bool vivo_antes = proto_.pontos_vida() >= proto_.dano_nao_letal();
  bool vivo_depois = pontos_vida > dano_nao_letal;
  if (vivo_antes && !vivo_depois) {
    proto_.set_morta(true);
    proto_.set_caida(true);
    proto_.set_voadora(false);
    proto_.set_aura_m(0.0f);
  } else if (vivo_depois && !vivo_antes) {
    proto_.set_morta(false);
  }
  proto_.set_pontos_vida(std::min(proto_.max_pontos_vida(), pontos_vida));
  proto_.set_dano_nao_letal(dano_nao_letal);
}

void Entidade::AtualizaParcial(const EntidadeProto& proto_parcial) {
  VLOG(1) << "Atualizacao parcial: " << proto_parcial.ShortDebugString();
  bool atualizar_vbo = false;
  int pontos_vida_antes = PontosVida();
  int dano_nao_letal_antes = DanoNaoLetal();
  if (proto_parcial.has_cor()) {
    atualizar_vbo = true;
    proto_.clear_cor();
  }

  // ATENCAO: todos os campos repeated devem ser verificados aqui para nao haver duplicacao apos merge.

  // Evento: se encontrar algum que ja existe, remove para o MergeFrom corrigir.
  if (proto_parcial.evento_size() > 0) {
    std::vector<int> a_remover;
    int i = 0;
    for (const auto& evento : proto_.evento()) {
      if (PossuiEventoEspecifico(proto_parcial, evento)) {
        a_remover.push_back(i);
      }
      ++i;
    }
    // Faz invertido para nao atrapalhar os indices.
    for (auto it = a_remover.rbegin(); it != a_remover.rend(); ++it) {
      proto_.mutable_evento()->DeleteSubrange(*it, 1);
    }
  }

  // Se encontrar alguma resistencia que ja existe, remove pro MergeFromCorrigir.
  if (!proto_parcial.dados_defesa().resistencia_elementos().empty()) {
    std::vector<int> a_remover;
    int i = 0;
    for (const auto& resistencia : proto_.dados_defesa().resistencia_elementos()) {
      if (PossuiResistenciaEspecifica(proto_parcial, resistencia)) a_remover.push_back(i);
      ++i;
    }
    // Faz invertido para nao atrapalhar os indices.
    for (auto it = a_remover.rbegin(); it != a_remover.rend(); ++it) {
      proto_.mutable_dados_defesa()->mutable_resistencia_elementos()->DeleteSubrange(*it, 1);
    }
  }

  if (proto_parcial.tesouro().pocoes_size() > 0) {
    proto_.mutable_tesouro()->clear_pocoes();
  }
  if (proto_parcial.lista_acoes_size() > 0) {
    // repeated.
    proto_.clear_lista_acoes();
  }
  if (proto_parcial.has_info_textura()) {
    AtualizaTexturas(proto_parcial);
    atualizar_vbo = true;
  }
  if (proto_parcial.has_modelo_3d()) {
    AtualizaModelo3d(proto_parcial);
    atualizar_vbo = true;
  }

  auto atributos_antes = proto_.atributos();
  if (proto_parcial.has_atributos()) {
    proto_.clear_atributos();
  }

  auto ca_antes = proto_.dados_defesa().ca();
  if (proto_parcial.dados_defesa().has_ca()) {
    proto_.mutable_dados_defesa()->clear_ca();
  }

  if (!proto_parcial.dados_ataque().empty()) {
    proto_.clear_dados_ataque();
  }

  if (proto_parcial.has_pontos_vida_temporarios_por_fonte()) {
    proto_.clear_pontos_vida_temporarios_por_fonte();
  }

  auto bonus_tamanho_antes = proto_parcial.bonus_tamanho();
  if (proto_parcial.has_bonus_tamanho()) {
    proto_.clear_bonus_tamanho();
  }

  const bool zerar_agarrado_a = proto_parcial.agarrado_a().size() == 1 && proto_parcial.agarrado_a(0) == IdInvalido;
  const bool tem_agarrado_a = !proto_parcial.agarrado_a().empty();
  if (tem_agarrado_a || zerar_agarrado_a) {
    // Merge sera automatico.
    proto_.clear_agarrado_a();
    //EntidadeProto p;
    //*p.mutable_agarrado_a() = proto_.agarrado_a();
    //LOG(INFO) << "antes: " << p.ShortDebugString() << ", novo agarrar: " << proto_parcial.ShortDebugString();
  }

  // ATUALIZACAO.
  proto_.MergeFrom(proto_parcial);

  if (proto_.dados_defesa().entidade_esquiva() == IdInvalido) {
    proto_.mutable_dados_defesa()->clear_entidade_esquiva();
  }

  if (zerar_agarrado_a) {
    proto_.clear_agarrado_a();
  }

  if (proto_.tipo() == TE_ENTIDADE && proto_.has_escala()) {
    float fator = proto_.escala().x();
    proto_.clear_escala();
    int delta = 0;
    if (fator > 1.1f) {
      delta = 1;
    } else if (fator < 0.9f) {
      delta = -1;
    }
    int base_corrente = BonusIndividualTotal(TB_BASE, proto_.bonus_tamanho());
    if (delta != 0) AtribuiBonus(base_corrente + delta, TB_BASE, "base", proto_.mutable_bonus_tamanho());
  }
  if (proto_.tipo() == TE_ENTIDADE && proto_parcial.has_bonus_tamanho()) {
    *proto_.mutable_bonus_tamanho() = bonus_tamanho_antes;
    CombinaBonus(proto_parcial.bonus_tamanho(), proto_.mutable_bonus_tamanho());
  }

  if (proto_.tesouro().pocoes_size() == 1 && !proto_.tesouro().pocoes(0).has_id() && !proto_.tesouro().pocoes(0).has_nome()) {
    proto_.mutable_tesouro()->clear_pocoes();
  }

  if (proto_.info_textura().id().empty()) {
    proto_.clear_info_textura();
  }
  if (proto_.modelo_3d().id().empty()) {
    proto_.clear_modelo_3d();
  }

  if (proto_parcial.dados_defesa().has_ca()) {
    *proto_.mutable_dados_defesa()->mutable_ca() = ca_antes;
    CombinaBonus(proto_parcial.dados_defesa().ca(), proto_.mutable_dados_defesa()->mutable_ca());
  }

  if (proto_parcial.has_atributos()) {
    *proto_.mutable_atributos() = atributos_antes;
    CombinaAtributos(proto_parcial.atributos(), proto_.mutable_atributos());
  }

  if (!proto_parcial.dados_ataque().empty()) {
    *proto_.mutable_dados_ataque() = proto_parcial.dados_ataque();
    if (proto_.dados_ataque_size() == 1 && proto_.dados_ataque(0).tipo_ataque().empty()) proto_.clear_dados_ataque();
  }

  // casos especiais.
  if (proto_parcial.iniciativa() == INICIATIVA_INVALIDA) {
    proto_.clear_iniciativa();
  }
  // Transicao nunca eh atualizacao parcial. Se for, deve considerar se ha transicao.
  // if ((proto_parcial.has_transicao_cenario && proto_parcial.transicao_cenario().id_cenario() == CENARIO_INVALIDO) ||
  //     (proto_parcial.has_tipo_transicao() && proto_parcial.tipo_transicao() == EntidadeProto::TRANS_NENHUMA)) {
  //  proto_.clear_transicao_cenario();
  //}
  if (proto_parcial.has_pos() && !proto_parcial.has_destino()) {
    proto_.clear_destino();
#if VBO_COM_MODELAGEM
    atualizar_vbo = true;
#endif
  }
  if (VBO_COM_MODELAGEM || (proto_parcial.has_escala() && Tipo() == TE_FORMA && proto_.sub_tipo() == TF_LIVRE)) {
    atualizar_vbo = true;
  }

  const auto* luz = proto_.has_luz() ? proto_.mutable_luz() : nullptr;
  if (luz != nullptr && luz->has_raio_m() && luz->raio_m() == 0.0f) {
    proto_.clear_luz();
  }
  const auto* cor_luz = ((luz != nullptr) && luz->has_cor()) ? &luz->cor() : nullptr;
  if (cor_luz != nullptr && (cor_luz->r() == 0 && cor_luz->g() == 0 && cor_luz->b() == 0)) {
    proto_.clear_luz();
  }
  if (proto_.has_cor() && !proto_.cor().has_r() && !proto_.cor().has_g() && !proto_.cor().has_b() && !proto_.cor().has_a()) {
    proto_.clear_cor();
  }
  if (proto_parcial.has_pontos_vida()) {
    // Restaura o que o merge fez para poder aplicar AtualizaPontosVida.
    proto_.set_pontos_vida(pontos_vida_antes);
    proto_.set_dano_nao_letal(dano_nao_letal_antes);
    AtualizaPontosVida(proto_parcial.pontos_vida(), proto_parcial.dano_nao_letal());
  }
  if (atualizar_vbo) {
    AtualizaVbo(parametros_desenho_);
  }
  if (proto_.reiniciar_ataque()) {
    proto_.clear_reiniciar_ataque();
    ReiniciaAtaque();
  }
  RecomputaDependencias(tabelas_, &proto_);
  VLOG(2) << "Entidade apos atualizacao parcial: " << proto_.ShortDebugString();
}

// Acao de display.
void Entidade::AtualizaAcao(const std::string& id_acao) {
  proto_.set_ultima_acao(id_acao);
  const auto* dado_corrente = DadoCorrente();
  if (dado_corrente == nullptr) {
    proto_.clear_ultimo_grupo_acao();
  } else {
    proto_.set_ultimo_grupo_acao(dado_corrente->grupo());
  }
}

bool Entidade::ProximaAcao() {
  if (proto_.dados_ataque().empty()) {
    return false;
  }
  if (proto_.dados_ataque().size() == 1) {
    // Pode acontecer quando a entidade tem ultima_acao default e eh colocada outra
    // manualmente. Neste caso, eh bom setar pra ter certeza.
    proto_.set_ultima_acao(proto_.dados_ataque(0).tipo_ataque());
    return true;
  }
  for (int i = 0; i < static_cast<int>(proto_.dados_ataque().size()) - 1; ++i) {
    proto_.mutable_dados_ataque()->SwapElements(i, i + 1);
  }
  proto_.set_ultima_acao(proto_.dados_ataque(0).tipo_ataque());
  proto_.set_ultimo_grupo_acao(proto_.dados_ataque(0).grupo());
  return true;
}

bool Entidade::AcaoAnterior() {
  if (vd_.ataques_na_rodada > 0) {
    AtaqueAnterior();
    return true;
  }
  if (proto_.dados_ataque().empty()) {
    return false;
  }
  if (proto_.dados_ataque().size() == 1) {
    // ditto.
    proto_.set_ultima_acao(proto_.dados_ataque(0).tipo_ataque());
    return true;
  }
  for (int i = proto_.dados_ataque().size() - 1; i > 0; --i) {
    proto_.mutable_dados_ataque()->SwapElements(i, i - 1);
  }
  proto_.set_ultima_acao(proto_.dados_ataque(0).tipo_ataque());
  proto_.set_ultimo_grupo_acao(proto_.dados_ataque(0).grupo());
  return true;
}

AcaoProto Entidade::Acao(const MapaIdAcao& mapa_acoes) const {
  const auto* da = DadoCorrente();
  auto StringAcao = [this, da]() -> std::string {
    if (da == nullptr) {
      // Entidade nao possui ataques.
      if (!proto_.ultima_acao().empty()) {
        return proto_.ultima_acao();
      }
      return TipoAcaoExecutada(0, { "Ataque Corpo a Corpo", "Ataque a Distância", "Feitiço de Toque" });
    }
    return da->tipo_ataque();
  };
  std::string string_acao = StringAcao();
  auto it = mapa_acoes.find(string_acao);
  if (it == mapa_acoes.end()) {
    return AcaoProto();
  }
  AcaoProto acao = *it->second;
  if (da != nullptr && da->has_acao()) {
    // Merge das informacoes dos DadosAtaque.
    acao.MergeFrom(da->acao());
  }
  return acao;
}

template<class T>
std::string ListaAcoes(const T& t) {
  std::string s;
  for (const auto& ti : t) {
    s += ti + ", ";
  }
  if (s.size() > 0) {
    s.resize(s.size() - 2);
  }
  return s;
}

// Acoes executadas.
void Entidade::AdicionaAcaoExecutada(const std::string& id_acao) {
  int indice_acao = -1;
  // Verifica se acao ja existe.
  for (int i = 0; i < proto_.lista_acoes_size(); ++i) {
    const auto& acao_entidade = proto_.lista_acoes(i);
    if (id_acao == acao_entidade) {
      indice_acao = i;
      break;
    }
  }
  // Se acao nao existe e cabe mais, cria uma nova no ultimo lugar.
  if (indice_acao == -1) {
    // A acao eh inserida no final (que sera descartada) e entao sera movida para a frente no final.
    if (static_cast<unsigned int>(proto_.lista_acoes_size()) < MaxNumAcoes) {
      proto_.add_lista_acoes(id_acao);
    } else {
      proto_.set_lista_acoes(proto_.lista_acoes_size() - 1, id_acao);
    }
    indice_acao = proto_.lista_acoes_size() - 1;
  }
  if (indice_acao == 0) {
    VLOG(1) << "Lista acoes: " << ListaAcoes(proto_.lista_acoes());
    return;
  }
  // Acao jogada para a primeira posicao.
  for (int i = indice_acao - 1; i >= 0; --i) {
    proto_.mutable_lista_acoes()->SwapElements(i, i + 1);
  }
  VLOG(1) << "Lista acoes: " << ListaAcoes(proto_.lista_acoes());
}

std::string Entidade::TipoAcaoExecutada(int indice_acao, const std::vector<std::string>& acoes_padroes) const {
  if (indice_acao < 0 || static_cast<unsigned int>(indice_acao) >= MaxNumAcoes) {
    return "";
  }
  if (indice_acao < proto_.lista_acoes_size()) {
    return proto_.lista_acoes(indice_acao);
  }
  // Junta as acoes da entidade com as padroes.
  std::vector<std::string> acoes(proto_.lista_acoes().begin(), proto_.lista_acoes().end());
  for (const std::string& acao_padrao : acoes_padroes) {
    if (std::find(acoes.begin(), acoes.end(), acao_padrao) == acoes.end()) {
      acoes.push_back(acao_padrao);
    }
    if (acoes.size() == MaxNumAcoes) {
      break;
    }
  }
  return acoes[indice_acao];
}

std::pair<TipoAcao, std::string> Entidade::TipoAcaoComIcone(
    int indice_acao, const std::vector<std::string>& acoes_padroes, const MapaIdAcao& mapa_acoes) const {
  if (indice_acao < 0 || static_cast<unsigned int>(indice_acao) >= MaxNumAcoes) {
    return std::make_pair(ACAO_INVALIDA, "");
  }
  if (indice_acao < proto_.lista_acoes_size()) {
    for (const auto& da : proto_.dados_ataque()) {
      if (da.tipo_ataque() == proto_.lista_acoes(indice_acao)) {
        return std::make_pair(da.tipo_acao(), da.acao().icone());
      }
    }
    auto it = mapa_acoes.find(proto_.lista_acoes(indice_acao));
    return it == mapa_acoes.end()
        ? std::make_pair(ACAO_INVALIDA, "")
        : std::make_pair(it->second->tipo(), it->second->icone());
  }
  // Junta as acoes da entidade com as padroes.
  std::vector<std::string> acoes(proto_.lista_acoes().begin(), proto_.lista_acoes().end());
  for (const std::string& acao_padrao : acoes_padroes) {
    if (std::find(acoes.begin(), acoes.end(), acao_padrao) == acoes.end()) {
      acoes.push_back(acao_padrao);
    }
    if (acoes.size() == MaxNumAcoes) {
      break;
    }
  }
  auto it = mapa_acoes.find(acoes[indice_acao]);
  return it == mapa_acoes.end()
      ? std::make_pair(ACAO_INVALIDA, "")
      : std::make_pair(it->second->tipo(), it->second->icone());
}

const Posicao Entidade::PosicaoAltura(float fator) const {
  Matrix4 matriz;
  matriz = MontaMatrizModelagem(true  /*queda*/, true  /*z*/, proto_, vd_);
  //GLfloat matriz[16];
  //gl::Le(GL_MODELVIEW_MATRIX, matriz);
  //VLOG(2) << "Matriz: " << matriz[0] << " " << matriz[1] << " " << matriz[2] << " " << matriz[3];
  //VLOG(2) << "Matriz: " << matriz[4] << " " << matriz[5] << " " << matriz[6] << " " << matriz[7];
  //VLOG(2) << "Matriz: " << matriz[8] << " " << matriz[9] << " " << matriz[10] << " " << matriz[11];
  //VLOG(2) << "Matriz: " << matriz[12] << " " << matriz[13] << " " << matriz[14] << " " << matriz[15];
  //GLfloat ponto[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  // A posicao da acao eh mais baixa que a altura.
  Vector4 ponto(0.0f, 0.0f, fator * ALTURA, 1.0f);
  //ponto = matriz * ponto;

  //VLOG(2) << "Ponto: " << ponto[0] << " " << ponto[1] << " " << ponto[2] << " " << ponto[3];
  //Posicao pos;
  //pos.set_x(ponto[0]);
  //pos.set_y(ponto[1]);
  //pos.set_z(ponto[2]);
  return Vector4ParaPosicao(matriz * ponto);
}

const Posicao Entidade::PosicaoAcao() const {
  if (proto_.has_posicao_acao()) {
    Matrix4 matriz;
    matriz = MontaMatrizModelagem(true  /*queda*/, true  /*z*/, proto_, vd_);
    Vector4 ponto(PosParaVector4(proto_.posicao_acao()));
    return Vector4ParaPosicao(matriz * ponto);
  }
  return PosicaoAltura(proto_.achatado() ? 0.1f : FATOR_ALTURA);
}

float Entidade::DeltaVoo(const VariaveisDerivadas& vd) {
  return vd.altura_voo + (vd.angulo_disco_voo_rad > 0 ? sinf(vd.angulo_disco_voo_rad) * ALTURA_VOO / 4.0f : 0.0f);
}

// static
void Entidade::MontaMatriz(bool queda,
                           bool transladar_z,
                           const EntidadeProto& proto,
                           const VariaveisDerivadas& vd,
                           const ParametrosDesenho* pd,
                           bool posicao_mundo) {
  Matrix4 matriz(MontaMatrizModelagem(queda, transladar_z, proto, vd, pd));
  gl::MultiplicaMatriz(matriz.get());
#if 0
  const auto& pos = proto.pos();
  bool achatar = (pd != nullptr && pd->desenha_texturas_para_cima()) && !proto.caida() && !proto.has_modelo_3d();
  float translacao_z = ZChao(pos.x(), pos.y());
  if (transladar_z) {
    translacao_z += proto.pos().z() + DeltaVoo(vd);
  }
  gl::Translada(pos.x(), pos.y(), translacao_z, false);
  bool computar_queda = queda && (vd.angulo_disco_queda_graus > 0);
  if (!computar_queda && (proto.has_modelo_3d() || (pd != nullptr && !pd->texturas_sempre_de_frente()))) {
    gl::Roda(proto.rotacao_z_graus(), 0, 0, 1.0f, false);
  }

  if (achatar && !proto.has_info_textura()) {
    // Achata cone.
    gl::Escala(1.0f, 1.0f, 0.1f, false);
  }

  if (computar_queda) {
    // Descomentar essa linha para ajustar a posicao da entidade.
    //gl::Translada(0, -TAMANHO_LADO_QUADRADO_2, 0);
    // Roda pra direcao de queda.
    const auto& dq = proto.direcao_queda();
    if (dq.x() != 0.0f || dq.y() != 0) {
      // Como a queda é sobre o eixo X, subtrai 90 para a direcao ficar certa.
      float direcao_queda_graus = VetorParaRotacaoGraus(dq) - 90.0f;
      gl::Roda(direcao_queda_graus, 0.0f, 0.0f, 1.0f, false);
    }
    if (!achatar) {
      // Roda sobre o eixo X negativo para cair com a face para cima.
      gl::Roda(vd.angulo_disco_queda_graus, -1.0f, 0, 0, false);
    }
  }
  float multiplicador = CalculaMultiplicador(proto.tamanho());
  gl::Escala(multiplicador, multiplicador, multiplicador, false);
  if (pd != nullptr && pd->has_escala_efeito()) {
    const auto& ee = pd->escala_efeito();
    gl::Escala(ee.x(), ee.y(), ee.z(), false);
  }
  if (pd != nullptr && pd->has_rotacao_efeito()) {
    const auto& re = pd->rotacao_efeito();
    if (re.has_x()) {
      gl::Roda(re.x(), 1.0f, 0.0f, 0.0f, false);
    } else if (re.has_y()) {
      gl::Roda(re.y(), 0.0f, 1.0f, 0.0f, false);
    } else if (re.has_z()) {
      gl::Roda(re.z(), 0.0f, 0.0f, 1.0f, false);
    }
  }
  if (pd != nullptr && pd->has_translacao_efeito()) {
    const auto& te = pd->translacao_efeito();
    gl::Translada(te.x(), te.y(), te.z(), false);
  }
#endif
}

// static
Matrix4 Entidade::MontaMatrizModelagem(
    bool queda,
    translacao_z_e tz,
    const EntidadeProto& proto,
    const VariaveisDerivadas& vd,
    const ParametrosDesenho* pd,
    bool posicao_mundo) {
  if (proto.tipo() == TE_FORMA || proto.tipo() == TE_COMPOSTA) {
    // Mesma matriz.
    return MontaMatrizModelagemForma(queda, tz == TZ_NENHUMA ? false : true, proto, vd, pd);
  }

  Matrix4 matrix;
  if (proto.modelo_3d().has_translacao()) {
    const auto& t = proto.modelo_3d().translacao();
    matrix.translate(t.x(), t.y(), t.z());
  }
  if (proto.modelo_3d().has_escala()) {
    const auto& e = proto.modelo_3d().escala();
    matrix.scale(e.x(), e.y(), e.z());
  }

  if (pd != nullptr && pd->has_translacao_efeito()) {
    const auto& te = pd->translacao_efeito();
    matrix.translate(te.x(), te.y(), te.z());
  }
  if (pd != nullptr && pd->has_rotacao_efeito()) {
    const auto& re = pd->rotacao_efeito();
    if (re.has_x()) {
      matrix.rotateX(re.x());
    } else if (re.has_y()) {
      matrix.rotateY(re.y());
    } else if (re.has_z()) {
      matrix.rotateZ(re.z());
    }
  }

  float multiplicador = CalculaMultiplicador(proto.tamanho());
  if (pd != nullptr && pd->has_escala_efeito()) {
    const auto& ee = pd->escala_efeito();
    matrix.scale(ee.x(), ee.y(), ee.z());
  }
  matrix.scale(multiplicador);

  bool achatar = (pd != nullptr && pd->desenha_texturas_para_cima()) && !proto.caida() && !proto.has_modelo_3d();
  bool computar_queda = queda && (vd.angulo_disco_queda_graus > 0);
  if (computar_queda) {
    if (!achatar) {
      // Roda sobre o eixo X negativo para cair com a face para cima.
      matrix.rotateX(- vd.angulo_disco_queda_graus);
    }
    // Roda pra direcao de queda.
    const auto& dq = proto.direcao_queda();
    if (dq.x() != 0.0f || dq.y() != 0) {
      // Como a queda é sobre o eixo X, subtrai 90 para a direcao ficar certa.
      float direcao_queda_graus = VetorParaRotacaoGraus(dq) - 90.0f;
      matrix.rotateZ(direcao_queda_graus);
    }
  }

  if (achatar && !proto.has_info_textura()) {
    // Achata cone.
    matrix.scale(1.0f, 1.0f, 0.1f);
  }

  if (!computar_queda && (proto.has_modelo_3d() || (pd != nullptr && !pd->texturas_sempre_de_frente()))) {
    matrix.rotateZ(proto.rotacao_z_graus());
  }

  if (posicao_mundo) {
    const auto& pos = proto.pos();
    float translacao_z = 0.0f;  //ZChao(pos.x(), pos.y());
    if (tz != TZ_NENHUMA) {
      translacao_z = proto.pos().z();
      if (tz == TZ_COMPLETA) {
        translacao_z += DeltaVoo(vd);
      }
    }
    matrix.translate(pos.x(), pos.y(), translacao_z);
  }
  return matrix;
}

int Entidade::Salvacao(const Entidade& atacante, TipoSalvacao tipo) const {
  Bonus b(BonusContraTendenciaNaSalvacao(atacante.Proto(), proto_));
  const auto& dd = proto_.dados_defesa();
  switch (tipo) {
    case TS_FORTITUDE: CombinaBonus(dd.salvacao_fortitude(), &b); break;
    case TS_REFLEXO: CombinaBonus(dd.salvacao_reflexo(), &b); break;
    case TS_VONTADE: CombinaBonus(dd.salvacao_vontade(), &b); break;
    default:
      LOG(ERROR) << "Tipo de salvacao invalido: " << (int)tipo;
  }
  return BonusTotal(b);
}

int Entidade::SalvacaoSemAtacante(TipoSalvacao tipo) const {
  const auto& dd = proto_.dados_defesa();
  switch (tipo) {
    case TS_FORTITUDE: return BonusTotal(dd.salvacao_fortitude());
    case TS_REFLEXO: return BonusTotal(dd.salvacao_reflexo());
    case TS_VONTADE: return BonusTotal(dd.salvacao_vontade());
    default:
      LOG(ERROR) << "Tipo de salvacao invalido: " << (int)tipo;
  }
  return 0;
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

std::tuple<int, std::string> Entidade::ValorParaAcao(const std::string& id_acao) const {
  std::string s = StringDanoParaAcao();
  if (s.empty()) {
    VLOG(1) << "Acao nao encontrada: " << id_acao;
    return std::make_tuple(0, "ação não encontrada");
  }
  try {
    // Nao deixa valor negativo para evitar danos que curam.
    int valor;
    std::vector<std::pair<int, int>> dados;
    std::tie(valor, dados) = GeraPontosVida(s);
    std::string texto_dados;
    for (const auto& fv : dados) {
      texto_dados += std::string("d") + net::to_string(fv.first) + "=" + net::to_string(fv.second) + ", ";
    }
    if (valor < 0) {
      valor = 0;
    }
    return std::make_tuple(valor, std::string("Valor para acao. ") + s + ", total: " + net::to_string(valor) + ", dados: " + texto_dados);
  } catch (const std::exception& e) {
    return std::make_tuple(0, std::string("string de dano malformada: ") + s);
  }
  return std::make_tuple(0, "nunca deveria chegar aqui");
}

std::string Entidade::DetalhesAcao() const {
  const auto* da = DadoCorrente();
  if (da == nullptr) {
    std::string sca = StringCAParaAcao();
    return sca.empty() ? "" : StringPrintf("CA: %s", sca.c_str());
  }
  return StringAtaque(*da, proto_);
}

std::string Entidade::StringDanoParaAcao() const {
  const auto* da = DadoCorrente();
  if (da == nullptr) {
   return "";
  }
  return ent::StringDanoParaAcao(*da, proto_);
}

std::string Entidade::StringCAParaAcao() const {
  const auto* da = DadoCorrente();
  if (da == nullptr) da = &EntidadeProto::DadosAtaque::default_instance();
  return ent::StringCAParaAcao(*da, proto_);
}

Matrix4 Entidade::MontaMatrizModelagem(const ParametrosDesenho* pd) const {
  return MontaMatrizModelagem(true, true, proto_, vd_, pd);
}

float Entidade::MultiplicadorTamanho() const {
  return CalculaMultiplicador(proto_.tamanho());
}

float Entidade::Espaco() const {
  return MultiplicadorTamanho() * TAMANHO_LADO_QUADRADO_2;
}

const EntidadeProto::DadosAtaque* Entidade::DadoCorrente() const {
  std::vector<const EntidadeProto::DadosAtaque*> ataques_casados;
  std::string ultima_acao = proto_.ultima_acao();
  std::string ultimo_grupo = proto_.ultimo_grupo_acao();
  if (ultima_acao.empty()) {
    ultima_acao = proto_.dados_ataque().empty() ? "Ataque Corpo a Corpo" : proto_.dados_ataque(0).tipo_ataque();
    ultimo_grupo = proto_.dados_ataque().empty() ? "" : proto_.dados_ataque(0).grupo();
  }
  for (const auto& da : proto_.dados_ataque()) {
    if (da.tipo_ataque() == ultima_acao && da.grupo() == ultimo_grupo) {
      VLOG(3) << "Encontrei ataque para " << da.tipo_ataque();
      ataques_casados.push_back(&da);
    }
  }
  if (ataques_casados.empty() || vd_.ataques_na_rodada >= (int)ataques_casados.size()) {
    VLOG(3) << "Dado corrente nao encontrado, tipo ultima acao: " << ultima_acao
            << ", empty? " << ataques_casados.empty()
            << ", at: " << vd_.ataques_na_rodada << ", size: " << ataques_casados.size();
    return nullptr;
  }
  VLOG(3) << "Retornando " << vd_.ataques_na_rodada << "o. ataque para " << ataques_casados[vd_.ataques_na_rodada]->tipo_ataque();
  return ataques_casados[vd_.ataques_na_rodada];
}

const EntidadeProto::DadosAtaque* Entidade::DadoAgarrar() const {
  for (const auto& da : proto_.dados_ataque()) {
    if (da.tipo_ataque() == "Agarrar") {
      return &da;
    }
  }
  return nullptr;
}

std::string Entidade::TipoAtaque() const {
  const auto* da = DadoCorrente();
  if (da != nullptr) {
    return da->tipo_ataque();
  }
  return "Ataque Corpo a Corpo";
}

float Entidade::AlcanceAtaqueMetros() const {
  const auto* da = DadoCorrente();
  if (da == nullptr || !da->has_alcance_m()) {
    return -1.5f;
  }
  return da->alcance_m();
}

float Entidade::AlcanceMinimoAtaqueMetros() const {
  const auto* da = DadoCorrente();
  if (da == nullptr || !da->has_alcance_minimo_m()) {
    return 0;
  }
  return da->alcance_minimo_m();
}

int Entidade::IncrementosAtaque() const {
  const auto* da = DadoCorrente();
  if (da == nullptr) {
    return 0;
  }
  return da->incrementos();
}

int Entidade::BonusAtaque() const {
  const auto* da = DadoCorrente();
  if (da == nullptr) {
    return AtaqueCaInvalido;
  }
  return da->bonus_ataque_final();
}

int Entidade::BonusAtaqueToque() const {
  if (PossuiTalento("acuidade_arma", proto_)) {
    if (!proto_.bba().has_cac() || !proto_.bba().has_distancia()) return AtaqueCaInvalido;
    return std::max(proto_.bba().cac(), proto_.bba().distancia());
  } else {
    if (!proto_.bba().has_cac()) return AtaqueCaInvalido;
    return proto_.bba().cac();
  }
}

int Entidade::BonusAtaqueToqueDistancia() const {
  if (!proto_.bba().has_distancia()) return AtaqueCaInvalido;
  return proto_.bba().distancia();
}

int Entidade::CA(const ent::Entidade& atacante, TipoCA tipo_ca) const {
  Bonus outros_bonus;
  CombinaBonus(BonusContraTendenciaNaCA(atacante.Proto(), proto_), &outros_bonus);
  // Cada tipo de CA sabera compensar a esquiva.
  const int bonus_esquiva =
      PossuiTalento("esquiva", proto_) && atacante.Id() == proto_.dados_defesa().entidade_esquiva() ? 1 : 0;
  AtribuiBonus(bonus_esquiva, TB_ESQUIVA, "esquiva", &outros_bonus);
  const auto* da = DadoCorrente();
  if (proto_.dados_defesa().has_ca()) {
    bool permite_escudo = da == nullptr || da->empunhadura() == EA_ARMA_ESCUDO;
    if (tipo_ca == CA_NORMAL) {
      return proto_.surpreso()
          ? CASurpreso(proto_, permite_escudo, outros_bonus)
          : CATotal(proto_, permite_escudo, outros_bonus);
    } else {
      return proto_.surpreso()
          ? CAToqueSurpreso(proto_, outros_bonus)
          : CAToque(proto_, outros_bonus);
    }
  }
  if (da == nullptr) {
    return AtaqueCaInvalido;
  }
  switch (tipo_ca) {
    case CA_TOQUE: return da->ca_toque() + bonus_esquiva;
    case CA_SURPRESO: return da->ca_surpreso();
    default: return da->ca_normal() + bonus_esquiva;
  }
}

int Entidade::CAReflexos() const {
  const auto& dd = proto_.dados_defesa();
  int modificador_tamanho = BonusIndividualTotal(ent::TB_TAMANHO, dd.ca());
  int modificador_destreza = proto_.surpreso() ? 0 : BonusIndividualTotal(ent::TB_ATRIBUTO, dd.ca());
  return 10 + modificador_tamanho + modificador_destreza;
}

int Entidade::MargemCritico() const {
  const auto* da = DadoCorrente();
  if (da == nullptr) {
    return AtaqueCaInvalido;
  }
  return da->margem_critico();
}

int Entidade::MultiplicadorCritico() const {
  const auto* da = DadoCorrente();
  if (da == nullptr) {
    return AtaqueCaInvalido;
  }
  return da->multiplicador_critico();
}

bool Entidade::ImuneCritico() const {
  return proto_.dados_defesa().imune_critico() || TemTipoDnD(TIPO_MORTO_VIVO) ||
         TemTipoDnD(TIPO_CONSTRUCTO) || TemTipoDnD(TIPO_PLANTA) ||
         TemTipoDnD(TIPO_ELEMENTAL) || TemTipoDnD(TIPO_LIMO) ||
         TemSubTipoDnD(SUBTIPO_ENXAME);
}

bool Entidade::ImuneFurtivo() const {
  return proto_.dados_defesa().imune_furtivo();
}

// static
bool Entidade::DesenhaBase(const EntidadeProto& proto) {
  if (proto.morta()) {
    return false;
  }
  if (proto.has_modelo_3d()) {
    return proto.desenha_base();
  }
  return proto.has_info_textura();
}

// Nome dos buffers de VBO.
std::vector<gl::VboGravado> Entidade::g_vbos;

// static
void Entidade::IniciaGl(ntf::CentralNotificacoes* central) {
  std::vector<gl::VboNaoGravado> vbos_nao_gravados(NUM_VBOS);
  {
    std::unique_ptr<ntf::Notificacao> n(ntf::NovaNotificacao(ntf::TN_CARREGAR_MODELO_3D));
    n->mutable_entidade()->mutable_modelo_3d()->set_id("halo");
    central->AdicionaNotificacao(n.release());
  }
  // Vbo peao.
  {
    auto& vbo = vbos_nao_gravados[VBO_PEAO];
    vbo = gl::VboConeSolido(TAMANHO_LADO_QUADRADO_2 - 0.2f, ALTURA, NUM_FACES, NUM_LINHAS);
    auto vbo_esfera = gl::VboEsferaSolida(TAMANHO_LADO_QUADRADO_2 - 0.4f, NUM_FACES, NUM_FACES / 2.0f);
    // Translada todos os Z da esfera em ALTURA.
    for (unsigned int i = 2; i < vbo_esfera.coordenadas().size(); i += vbo_esfera.NumDimensoes()) {
      vbo_esfera.coordenadas()[i] += ALTURA;
    }
    vbo.Concatena(vbo_esfera);
    vbo.Nomeia("peão");
  }

  // Vbo tijolo.
  {
    auto& vbo = vbos_nao_gravados[VBO_TIJOLO];
    vbo = gl::VboCuboSolido(TAMANHO_LADO_QUADRADO);
    vbo.Nomeia("tijolo");
  }

  // Tela para desenho de texturas de entidades.
  // Virada para o sul, centralizada no meio da tela.
  {
    const unsigned short indices[] = { 0, 1, 2, 3 };
    const float coordenadas[] = {
      -TAMANHO_LADO_QUADRADO_2, -TAMANHO_LADO_QUADRADO_10 / 4.0f - 0.01f, TAMANHO_LADO_QUADRADO_10,
      TAMANHO_LADO_QUADRADO_2,  -TAMANHO_LADO_QUADRADO_10 / 4.0f - 0.01f, TAMANHO_LADO_QUADRADO_10,
      TAMANHO_LADO_QUADRADO_2,  -TAMANHO_LADO_QUADRADO_10 / 4.0f - 0.01f, TAMANHO_LADO_QUADRADO_10 + TAMANHO_LADO_QUADRADO,
      -TAMANHO_LADO_QUADRADO_2, -TAMANHO_LADO_QUADRADO_10 / 4.0f - 0.01f, TAMANHO_LADO_QUADRADO_10 + TAMANHO_LADO_QUADRADO,
    };
    const float coordenadas_textura[] = {
      0.0f, 1.0f,
      1.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f,
    };
    const float coordenadas_normais[] = {
      0.0f, -1.0f, 0.0f,
      0.0f, -1.0f, 0.0f,
      0.0f, -1.0f, 0.0f,
      0.0f, -1.0f, 0.0f,
    };
    auto& vbo = vbos_nao_gravados[VBO_TELA_TEXTURA];
    vbo.AtribuiCoordenadas(3, coordenadas, 12);
    vbo.AtribuiTexturas(coordenadas_textura);
    vbo.AtribuiNormais(coordenadas_normais);
    vbo.AtribuiIndices(indices, 4);
    vbo.Nomeia("tela de textura");
  }
  // Base peça.
  {
    auto& vbo = vbos_nao_gravados[VBO_BASE_PECA];
    const float raio_peca = TAMANHO_LADO_QUADRADO_2 * 0.9f;
    const int num_lados_peca = 6;
    vbo = gl::VboCilindroSolido(raio_peca  /*raio*/, TAMANHO_LADO_QUADRADO_10  /*altura*/, num_lados_peca, 1);
    {
      gl::VboNaoGravado vbo_disco = gl::VboDisco(raio_peca  /*raio*/, num_lados_peca);
      vbo_disco.Escala(-1.0f, 1.0f, -1.0f);
      vbo.Concatena(vbo_disco);
    }
    {
      gl::VboNaoGravado vbo_disco = gl::VboDisco(raio_peca  /*raio*/, num_lados_peca);
      vbo_disco.Translada(0.0f, 0.0f, TAMANHO_LADO_QUADRADO_10);
      vbo.Concatena(vbo_disco);
    }
    vbo.Nomeia("BasePeça");
  }
  // Moldura peça.
  {
    auto& vbo = vbos_nao_gravados[VBO_MOLDURA_PECA];
    vbo = gl::VboCuboSolido(TAMANHO_LADO_QUADRADO);
    vbo.Translada(0.0f, 0.0f, TAMANHO_LADO_QUADRADO_10 + TAMANHO_LADO_QUADRADO_2);
    vbo.Escala(1.0f, 1.0f / 20.0f, 1.0f);
    vbo.Nomeia("MolduraPeça");
  }

  // Cubo.
  {
    auto& vbo = vbos_nao_gravados[VBO_CUBO];
    vbo = gl::VboCuboSolido(1.0f);
    vbo.Nomeia("cubo unitario");
  }

  // Esfera.
  {
    auto& vbo = vbos_nao_gravados[VBO_ESFERA];
    vbo = gl::VboEsferaSolida(0.5f, 24, 12);
    vbo.Nomeia("Esfera unitaria");
  }

  // Piramide.
  {
    auto& vbo = vbos_nao_gravados[VBO_PIRAMIDE];
    vbo = gl::VboPiramideSolida(1.0f, 1.0f);
    vbo.Nomeia("Piramide");
  }

  // Cilindro.
  {
    auto& vbo = vbos_nao_gravados[VBO_CILINDRO];
    vbo = gl::VboCilindroSolido(0.5f  /*raio*/, 1.0f  /*altura*/, 12, 6);
    vbo.Nomeia("Cilindro");
  }
  // Cilindro fechado.
  {
    auto& vbo = vbos_nao_gravados[VBO_CILINDRO_FECHADO];
    vbo = gl::VboCilindroSolido(0.5f  /*raio*/, 1.0f  /*altura*/, 12, 6);
    {
      gl::VboNaoGravado vbo_disco = gl::VboDisco(0.5f  /*raio*/, 12  /*fatias*/);
      vbo_disco.Escala(-1.0f, 1.0f, -1.0f);
      vbo.Concatena(vbo_disco);
    }
    {
      gl::VboNaoGravado vbo_disco = gl::VboDisco(0.5f  /*raio*/, 12  /*fatias*/);
      vbo_disco.Translada(0.0f, 0.0f, 1.0f);
      vbo.Concatena(vbo_disco);
    }
    vbo.Nomeia("CilindroFechado");
  }

  // Disco.
  {
    auto& vbo = vbos_nao_gravados[VBO_DISCO];
    vbo = gl::VboDisco(0.5f  /*raio*/, 12);
    vbo.Nomeia("Disco");
  }

  // Retangulo.
  {
    auto& vbo = vbos_nao_gravados[VBO_RETANGULO];
    vbo = gl::VboRetangulo(1.0f);
    vbo.Nomeia("Retangulo");
  }

  // Triangulo.
  {
    auto& vbo = vbos_nao_gravados[VBO_TRIANGULO];
    vbo = gl::VboTriangulo(1.0f);
    vbo.Nomeia("Triangulo");
  }

  // Cone.
  {
    auto& vbo = vbos_nao_gravados[VBO_CONE];
    vbo = gl::VboConeSolido(0.5f, 1.0f, 12, 6);
    vbo.Nomeia("Cone");
  }

  // Cone fechado.
  {
    auto& vbo = vbos_nao_gravados[VBO_CONE_FECHADO];
    vbo = gl::VboConeSolido(0.5f, 1.0f, 12, 6);
    {
      gl::VboNaoGravado vbo_disco = gl::VboDisco(0.5f  /*raio*/, 12  /*fatias*/);
      vbo_disco.Escala(-1.0f, 1.0f, -1.0f);
      vbo.Concatena(vbo_disco);
    }
    vbo.Nomeia("ConeFechado");
  }

  // Gera os Vbos.
  g_vbos.resize(NUM_VBOS);
  for (int i = 0; i < NUM_VBOS; ++i) {
    g_vbos[i].Desgrava();
    g_vbos[i].Grava(vbos_nao_gravados[i]);
  }
  // Texturas globais.
  {
    // TODO remover essa textura.
    auto n_tex = ntf::NovaNotificacao(ntf::TN_CARREGAR_TEXTURA);
    n_tex->add_info_textura()->set_id("smoke.png");
    central->AdicionaNotificacao(n_tex.release());
  }
}

// static
bool Entidade::Colisao(const EntidadeProto& proto, const Posicao& pos, Vector3* direcao) {
  if (!proto.causa_colisao() || pos.id_cenario() != proto.pos().id_cenario()) {
    return false;
  }
  // Fazer a transformada do ponto e do vetor para a modelagem do objeto, ficara bem mais facil calcular.
  if (proto.tipo() == TE_FORMA) {
    return ColisaoForma(proto, pos, direcao);
  } else if (proto.tipo() == TE_COMPOSTA) {
    return ColisaoComposta(proto, pos, direcao);
  }
  return false;
}

std::string Entidade::ResumoEventos() const {
  if (proto_.evento().empty()) {
    return "";
  }
  std::string resumo_eventos;
  for (const auto& evento : proto_.evento()) {
    if (evento.rodadas() < 0 || evento.id_efeito() == EFEITO_INVALIDO) continue;
    resumo_eventos += evento.descricao() + ", ";
  }
  if (resumo_eventos.size() > 1) {
    resumo_eventos.pop_back();
    resumo_eventos.pop_back();
  }
  return resumo_eventos;
}

int Entidade::ChanceFalhaDefesa() const {
  int chance = 0;
  if (PossuiEvento(EFEITO_BORRAR, proto_)) chance = 20;
  // TODO
  // Esse caso é mais complicado porque depende de outros fatores (poder ver invisibilidade, por exemplo).
  if (PossuiEvento(EFEITO_PISCAR, proto_)) chance = 50;
  if (PossuiEvento(EFEITO_INVISIBILIDADE, proto_)) chance = 50;
  return chance;
}

int Entidade::ChanceFalhaAtaque() const {
  // Chance de ficar etereo ao atacar.
  int chance = 0;
  if (PossuiEvento(EFEITO_PISCAR, proto_)) chance = 20;
  chance = std::max(chance, proto_.dados_ataque_global().chance_falha());
  return chance;
}

bool Entidade::IgnoraChanceFalha() const {
  return proto_.dados_ataque_global().chance_falha() < 0;
}

void Entidade::AlteraTodosFeiticos(const EntidadeProto& proto_parcial) {
  *proto_.mutable_feiticos_classes() = proto_parcial.feiticos_classes();
}

void Entidade::AlteraFeitico(const std::string& id_classe, int nivel, int indice, bool usado) {
  auto* fn = FeiticosNivelOuNullptr(id_classe, nivel, &proto_);
  //LOG(INFO) << "PROTO: " << FeiticosClasse(id_classe, proto_).DebugString();
  if (fn == nullptr || fn->para_lancar().size() <= indice) {
    LOG(ERROR) << "Nao foi possivel alterar o feitico de classe " << id_classe
               << ", nivel: " << nivel << ", indice: " << indice
               << ", para entidade: " << RotuloEntidade(this)
               << ", fn == nullptr: " << (fn == nullptr);
    return;
  }
  fn->mutable_para_lancar(indice)->set_usado(usado);
}

bool Entidade::ImuneVeneno() const {
  if (TemTipoDnD(TIPO_MORTO_VIVO) || TemTipoDnD(TIPO_ELEMENTAL) || TemTipoDnD(TIPO_LIMO) || TemTipoDnD(TIPO_PLANTA) || TemTipoDnD(TIPO_CONSTRUCTO)) {
    return true;
  }
  return std::any_of(proto_.dados_defesa().imunidades().begin(), proto_.dados_defesa().imunidades().end(), [](int desc) { return desc == DESC_VENENO; });
}

bool Entidade::TemLuz() const {
  return proto_.has_luz() || vd_.luz_acao.inicio.has_raio_m();
}

void Entidade::AtivaLuzAcao(const IluminacaoPontual& luz) {
  auto& luz_acao = vd_.luz_acao;
  luz_acao.duracao_ms = luz.duracao_ms();
  luz_acao.tempo_desde_inicio_ms = 0;
  luz_acao.inicio.set_raio_m(luz.raio_m());
  *luz_acao.inicio.mutable_cor() = luz.cor();
  LOG(INFO) << "Luz acao ligada";
}

void Entidade::AtivaFumegando(int duracao_ms) {
  auto& f = vd_.fumaca;
  f.duracao_ms = duracao_ms;
  f.intervalo_emissao_ms = 1000;
  f.duracao_nuvem_ms = 3000;
  f.proxima_emissao_ms = 0;
}

void Entidade::ReiniciaAtaque() {
  vd_.ataques_na_rodada = 0;
}

}  // namespace ent
