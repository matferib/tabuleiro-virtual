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

namespace gl {
bool ImprimeSeErro();
}  // namespace gl

namespace ent {

// Factory.
Entidade* NovaEntidade(const EntidadeProto& proto, const Texturas* texturas, const m3d::Modelos3d* m3d, ntf::CentralNotificacoes* central, const ParametrosDesenho* pd) {
  switch (proto.tipo()) {
    case TE_COMPOSTA:
    case TE_ENTIDADE:
    case TE_FORMA: {
      auto* entidade = new Entidade(texturas, m3d, central, pd);
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
Entidade::Entidade(const Texturas* texturas, const m3d::Modelos3d* m3d, ntf::CentralNotificacoes* central, const ParametrosDesenho* pd) {
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
  // Evitar oscilacoes juntas.
  vd_.angulo_disco_luz_rad = ((RolaDado(360) - 1.0f) / 180.0f) * M_PI;
  if (proto_.tipo() == TE_FORMA) {
    InicializaForma(proto_, &vd_);
  } else if (proto_.tipo() == TE_COMPOSTA) {
    InicializaComposta(proto_, &vd_);
  }
  AtualizaVbo(parametros_desenho_);
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
    auto* nl = ntf::NovaNotificacao(ntf::TN_DESCARREGAR_MODELO_3D);
    nl->mutable_entidade()->mutable_modelo_3d()->set_id(proto_.modelo_3d().id());
    central_->AdicionaNotificacao(nl);
  }
  // Carrega modelo_3d se houver e for diferente da antiga.
  if (!novo_proto.modelo_3d().id().empty() &&
      novo_proto.modelo_3d().id() != proto_.modelo_3d().id()) {
    VLOG(1) << "Carregando modelo_3d: " << novo_proto.modelo_3d().id();
    auto* nc = ntf::NovaNotificacao(ntf::TN_CARREGAR_MODELO_3D);
    *nc->mutable_entidade()->mutable_modelo_3d() = novo_proto.modelo_3d();
    central_->AdicionaNotificacao(nc);
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
    auto* nl = ntf::NovaNotificacao(ntf::TN_DESCARREGAR_TEXTURA);
    nl->add_info_textura()->set_id(proto_atual->info_textura().id());
    central->AdicionaNotificacao(nl);
  }
  // Carrega textura se houver e for diferente da antiga.
  if (novo_proto.has_info_textura() && !novo_proto.info_textura().id().empty() && novo_proto.info_textura().id() != proto_atual->info_textura().id()) {
    VLOG(1) << "Carregando textura: " << proto_atual->info_textura().id();
    auto* nc = ntf::NovaNotificacao(ntf::TN_CARREGAR_TEXTURA);
    nc->add_info_textura()->CopyFrom(novo_proto.info_textura());
    central->AdicionaNotificacao(nc);
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
  proto_.CopyFrom(novo_proto);
  if (proto_.pontos_vida() > proto_.max_pontos_vida()) {
    proto_.set_pontos_vida(proto_.max_pontos_vida());
  }
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
    if (fator > 1.1f) {
      proto_.set_tamanho(static_cast<TamanhoEntidade>(std::min<int>(TM_COLOSSAL, proto_.tamanho() + 1)));
    } else if (fator < 0.9f) {
      proto_.set_tamanho(static_cast<TamanhoEntidade>(std::max<int>(TM_MINUSCULO, proto_.tamanho() - 1)));
    }
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
    if (!evento.has_id_efeito()) {
      continue;
    }
    AtualizaEfeito(static_cast<efeitos_e>(evento.id_efeito()), &vd_.complementos_efeitos[evento.id_efeito()]);
    a_remover.erase(evento.id_efeito());
  }
  for (const auto& id_remocao : a_remover) {
    vd_.complementos_efeitos.erase(id_remocao);
  }
}

void Entidade::AtualizaEfeito(efeitos_e id_efeito, ComplementoEfeito* complemento) {
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
  if (proto.tipo() != TE_ENTIDADE || proto.has_modelo_3d()) {
    return md;
  }
  // tijolo base. Usada para disco de peao tambem.
  if (!proto.morta()) {
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


void Entidade::Atualiza(int intervalo_ms) {
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
  const unsigned int INTERVALO_ZERAR_ATAQUES_MS = 3000;
  vd_.ultimo_ataque_ms += intervalo_ms;
  if (vd_.ultimo_ataque_ms > INTERVALO_ZERAR_ATAQUES_MS) {
    vd_.ataques_na_rodada = 0;
  }

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
    return -2;
  }
  return proto_.transicao_cenario().id_cenario();
}

const Posicao& Entidade::PosTransicao() const {
  return proto_.transicao_cenario();
}

int Entidade::PontosVida() const {
  return proto_.pontos_vida();
}

int Entidade::MaximoPontosVida() const {
  return proto_.max_pontos_vida();
}

int Entidade::PontosVidaTemporarios() const {
  return proto_.pontos_vida_temporarios();
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

void Entidade::AtualizaPontosVida(int pontos_vida) {
  if (proto_.max_pontos_vida() == 0) {
    // Entidades sem pontos de vida nao sao afetadas.
    return;
  }
  if (proto_.pontos_vida() >= 0 && pontos_vida < 0) {
    proto_.set_morta(true);
    proto_.set_caida(true);
    proto_.set_voadora(false);
    proto_.set_aura_m(0.0f);
  } else if (proto_.pontos_vida() < 0 && pontos_vida >= 0) {
    proto_.set_morta(false);
  }
  proto_.set_pontos_vida(std::min(proto_.max_pontos_vida(), pontos_vida));
}

void Entidade::AtualizaParcial(const EntidadeProto& proto_parcial) {
  bool atualizar_vbo = false;
  int pontos_vida_antes = PontosVida();
  if (proto_parcial.has_cor()) {
    atualizar_vbo = true;
    proto_.clear_cor();
  }
  // ATENCAO: todos os campos repeated devem ser verificados aqui para nao haver duplicacao apos merge.
  if (proto_parcial.evento_size() > 0) {
    // Evento eh repeated, merge nao serve.
    proto_.clear_evento();
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

  // ATUALIZACAO.
  proto_.MergeFrom(proto_parcial);

  // casos especiais.
  if (proto_parcial.iniciativa() == INICIATIVA_INVALIDA) {
    proto_.clear_iniciativa();
  }
  if (proto_parcial.evento_size() == 1 && !proto_parcial.evento(0).has_rodadas()) {
    // Evento dummy so para limpar eventos.
    proto_.clear_evento();
  }
  if (proto_parcial.transicao_cenario().id_cenario() == CENARIO_INVALIDO) {
    proto_.clear_transicao_cenario();
  }
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
    AtualizaPontosVida(proto_parcial.pontos_vida());
  }
  if (atualizar_vbo) {
    AtualizaVbo(parametros_desenho_);
  }
  VLOG(1) << "Entidade apos atualizacao parcial: " << proto_.ShortDebugString();
}

// Acao de display.
void Entidade::AtualizaAcao(const std::string& id_acao) {
  proto_.set_ultima_acao(id_acao);
}

bool Entidade::ProximaAcao() {
  if (proto_.dados_ataque().empty()) {
    return false;
  }
  if (proto_.dados_ataque().size() == 1) {
    return true;
  }
  for (int i = 0; i < static_cast<int>(proto_.dados_ataque().size()) - 1; ++i) {
    proto_.mutable_dados_ataque()->SwapElements(i, i + 1);
  }
  proto_.set_ultima_acao(proto_.dados_ataque(0).tipo_ataque());
  return true;
}

bool Entidade::AcaoAnterior() {
  if (proto_.dados_ataque().empty()) {
    return false;
  }
  if (proto_.dados_ataque().size() == 1) {
    return true;
  }
  for (int i = proto_.dados_ataque().size() - 1; i > 0; --i) {
    proto_.mutable_dados_ataque()->SwapElements(i, i - 1);
  }
  proto_.set_ultima_acao(proto_.dados_ataque(0).tipo_ataque());
  return true;
}

std::string Entidade::Acao(const std::vector<std::string>& acoes_padroes) const {
  const auto* da = DadoCorrente();
  if (da == nullptr) {
    // Comportamento legado.
    if (!proto_.ultima_acao().empty()) {
      return proto_.ultima_acao();
    }
    if (acoes_padroes.empty()) {
      return "";
    }
    return AcaoExecutada(0, acoes_padroes);
  }
  return da->tipo_ataque();
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

std::string Entidade::AcaoExecutada(int indice_acao, const std::vector<std::string>& acoes_padroes) const {
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

const Posicao Entidade::PosicaoAcao() const {
  Matrix4 matriz;
  matriz = MontaMatrizModelagem(true  /*queda*/, true  /*z*/, proto_, vd_) * matriz;
  //GLfloat matriz[16];
  //gl::Le(GL_MODELVIEW_MATRIX, matriz);
  //VLOG(2) << "Matriz: " << matriz[0] << " " << matriz[1] << " " << matriz[2] << " " << matriz[3];
  //VLOG(2) << "Matriz: " << matriz[4] << " " << matriz[5] << " " << matriz[6] << " " << matriz[7];
  //VLOG(2) << "Matriz: " << matriz[8] << " " << matriz[9] << " " << matriz[10] << " " << matriz[11];
  //VLOG(2) << "Matriz: " << matriz[12] << " " << matriz[13] << " " << matriz[14] << " " << matriz[15];
  //GLfloat ponto[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  // A posicao da acao eh mais baixa que a altura.
  Vector4 ponto(0.0f, 0.0f, proto_.achatado() ? TAMANHO_LADO_QUADRADO_10 : ALTURA_ACAO, 1.0f);
  ponto = matriz * ponto;

  //VLOG(2) << "Ponto: " << ponto[0] << " " << ponto[1] << " " << ponto[2] << " " << ponto[3];
  Posicao pos;
  pos.set_x(ponto[0]);
  pos.set_y(ponto[1]);
  pos.set_z(ponto[2]);
  return pos;
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

int Entidade::ValorParaAcao(const std::string& id_acao) const {
  std::string s = StringDanoParaAcao();
  if (s.empty()) {
    VLOG(1) << "Acao nao encontrada: " << id_acao;
    return 0;
  }
  try {
    // Nao deixa valor negativo para evitar danos que curam.
    int valor = GeraPontosVida(s);
    if (valor < 0) {
      valor = 0;
    }
    return valor;
  } catch (const std::exception& e) {
    return 0;
  }
}

std::string Entidade::DetalhesAcao() const {
  const auto* dado_ataque = DadoCorrente();
  if (dado_ataque == nullptr) {
    return "";
  }
  return dado_ataque->rotulo() + ": " + (dado_ataque->bonus_ataque() > 0 ? "+" : "") + net::to_string(dado_ataque->bonus_ataque()) + ", " + dado_ataque->dano();
}

std::string Entidade::StringDanoParaAcao() const {
  const auto* dado_ataque = DadoCorrente();
  return dado_ataque == nullptr ? "" : dado_ataque->dano();
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
  if (ultima_acao.empty()) {
    ultima_acao = proto_.dados_ataque().empty() ? "Ataque Corpo a Corpo" : proto_.dados_ataque(0).tipo_ataque();
  }
  for (const auto& da : proto_.dados_ataque()) {
    if (da.tipo_ataque() == ultima_acao) {
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
  return da->bonus_ataque();
}

int Entidade::CA(TipoCA tipo_ca) const {
  const auto* da = DadoCorrente();
  if (da == nullptr) {
    return AtaqueCaInvalido;
  }
  switch (tipo_ca) {
    case CA_TOQUE: return da->ca_toque();
    case CA_SURPRESO: return da->ca_surpreso();
    default: return da->ca_normal();
  }
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
  return proto_.dados_defesa().imune_critico();

}

// Nome dos buffers de VBO.
std::vector<gl::VboGravado> Entidade::g_vbos;

// static
void Entidade::IniciaGl() {
  std::vector<gl::VboNaoGravado> vbos_nao_gravados(NUM_VBOS);
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
    g_vbos[i].Grava(vbos_nao_gravados[i]);
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

}  // namespace ent
