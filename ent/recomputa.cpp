#include "ent/recomputa.h"

#include <unordered_set>
#include "ent/acoes.h"
#include "ent/constantes.h"
#include "ent/util.h"
#include "goog/stringprintf.h"
#include "log/log.h"

namespace ent {
namespace {

using google::protobuf::StringPrintf;
using google::protobuf::RepeatedPtrField;

// Redimensiona o container.
template <class T>
void Redimensiona(int tam, RepeatedPtrField<T>* c) {
  if (tam == c->size()) return;
  if (tam < c->size()) {
    c->DeleteSubrange(tam, c->size() - tam);
    return;
  }
  while (c->size() < tam) c->Add();
}

// Retorna o nivel do feitico para determinada classe.
int NivelFeitico(const Tabelas& tabelas, const std::string& id_classe, const ArmaProto& arma) {
  const auto& id = IdParaMagia(tabelas, id_classe);
  for (const auto& ic : arma.info_classes()) {
    if (ic.id() == id) return ic.nivel();
  }
  return -1;
}

void DobraMargemCritico(DadosAtaque* da) {
  int margem = 21 - da->margem_critico();
  margem *= 2;
  da->set_margem_critico(21 - margem);
}

// Retorna o bonus do talento para a pericia ou zero caso nao haja.
int BonusTalento(const std::string& id_pericia, const TalentoProto& talento) {
  for (const auto& bp : talento.bonus_pericias()) {
    if (bp.id() == id_pericia) return bp.valor();
  }
  return 0;
}

int CalculaBonusBaseAtaque(const EntidadeProto& proto) {
  if (PossuiEvento(EFEITO_PODER_DIVINO, proto)) {
    return Nivel(proto);
  }
  int bba = 0;
  for (const auto& info_classe : proto.info_classes()) {
    bba += info_classe.bba();
  }
  return bba;
}

// Retorna o bonus de ataque para uma determinada arma.
int CalculaBonusBaseParaAtaque(const DadosAtaque& da, const EntidadeProto& proto) {
  return BonusTotal(da.bonus_ataque());
}

// Retorna a string de dano para uma arma.
std::string CalculaDanoParaAtaque(const DadosAtaque& da, const EntidadeProto& proto) {
  const int mod_final = BonusTotal(da.bonus_dano());
  return da.dano_basico().c_str() + (mod_final != 0 ? google::protobuf::StringPrintf("%+d", mod_final) : "");
}

std::string DanoBasicoPorTamanho(TamanhoEntidade tamanho, const StringPorTamanho& dano) {
  if (dano.has_invariavel()) {
    return dano.invariavel();
  }
  switch (tamanho) {
    case TM_MINUSCULO: return dano.minusculo();
    case TM_DIMINUTO: return dano.diminuto();
    case TM_MIUDO: return dano.miudo();
    case TM_PEQUENO: return dano.pequeno();
    case TM_MEDIO: return dano.medio();
    case TM_GRANDE: return dano.grande();
    case TM_ENORME: return dano.enorme();
    case TM_IMENSO: return dano.imenso();
    case TM_COLOSSAL: return dano.colossal();
    default: return "";
  }
}

// Retorna a arma da outra mao.
const ArmaProto& ArmaOutraMao(
    const Tabelas& tabelas, const DadosAtaque& da_mao, const EntidadeProto& proto) {
  const DadosAtaque* da_outra_mao = &da_mao;
  for (const auto& da : proto.dados_ataque()) {
    if (da.rotulo() == da_mao.rotulo() && da.empunhadura() != da_mao.empunhadura()) {
      da_outra_mao = &da;
      break;
    }
  }
  if (da_outra_mao == &da_mao) LOG(WARNING) << "Nao encontrei a arma na outra mao, fallback pro mesmo tipo";
  for (const auto& da : proto.dados_ataque()) {
    if (da.tipo_ataque() == da_mao.tipo_ataque() && da.empunhadura() != da_mao.empunhadura()) {
      da_outra_mao = &da;
      break;
    }
  }
  if (da_outra_mao == &da_mao) {
    LOG(ERROR) << "Nao encontrei a arma na outra mao, retornando a mesma: " << da_mao.id_arma();
  }
  return tabelas.Arma(da_outra_mao->id_arma());
}

// Aplica o bonus ou remove, se for 0. Bonus vazios sao ignorados.
void AplicaBonusPenalidadeOuRemove(const Bonus& bonus, Bonus* alvo) {
  for (const auto& bi : bonus.bonus_individual()) {
    for (const auto& po : bi.por_origem()) {
      if (po.valor() != 0) {
        AtribuiBonusPenalidadeSeMaior(po.valor(), bi.tipo(), po.origem(), alvo);
      } else {
        RemoveBonus(bi.tipo(), po.origem(), alvo);
      }
    }
  }
}

// As duas funcoes assumem que a consequencia tem modificador de ataque e dano.
// Normalmente nao ha restricao, mas se houver, respeita.
bool ConsequenciaAfetaDadosAtaque(const ConsequenciaEvento& consequencia, const DadosAtaque& da) {
  if (!consequencia.has_restricao_arma()) return true;
  const auto& ra = consequencia.restricao_arma();
  if (ra.has_prefixo_arma() && da.id_arma().find(ra.prefixo_arma()) == 0)
    return true;
  if (ra.apenas_armas() && da.eh_arma()) return true;
  if (ra.apenas_armas_para_dano())
    return true;  // a restricao se aplica apenas ao dano.
  return c_any(consequencia.restricao_arma().id_arma(), da.id_arma());
}

bool ConsequenciaAfetaDano(const ConsequenciaEvento& consequencia, const DadosAtaque& da) {
  const auto& ra = consequencia.restricao_arma();
  return (!ra.apenas_armas() || da.eh_arma());
}

// Retorna o dado de ataque que contem a arma, ou nullptr;
const DadosAtaque* DadosAtaquePorIdArma(const std::string& id_arma, const EntidadeProto& proto, bool mao_ruim = false) {
  for (const auto& da : proto.dados_ataque()) {
    if (da.id_arma() == id_arma) {
      if (mao_ruim && da.empunhadura() != EA_MAO_RUIM) continue;
      return &da;
    }
  }
  return nullptr;
}

DadosAtaque* DadosAtaquePorIdUnico(int id_unico, EntidadeProto* proto, bool mao_ruim = false) {
  for (auto& da : *proto->mutable_dados_ataque()) {
    if (da.id_unico_efeito() == id_unico) {
      if (mao_ruim && da.empunhadura() != EA_MAO_RUIM) continue;
      if (!mao_ruim && da.empunhadura() == EA_MAO_RUIM) continue;
      return &da;
    }
  }
  return nullptr;
}

DadosAtaque* DadosAtaquePorTalento(const std::string& id_talento, EntidadeProto* proto) {
  for (auto& da : *proto->mutable_dados_ataque()) {
    if (da.id_talento() == id_talento) {
      return &da;
    }
  }
  return nullptr;
}

// Retorna os dado de ataque com o mesmo rotulo.
std::vector<DadosAtaque*> DadosAtaquePorRotulo(const std::string& rotulo, EntidadeProto* proto) {
  std::vector<DadosAtaque*> das;
  for (auto& da : *proto->mutable_dados_ataque()) {
    if (da.rotulo() == rotulo) das.push_back(&da);
  }
  return das;
}

#if 0
bool PossuiArma(const std::string& id_arma, const EntidadeProto& proto) {
  return std::any_of(proto.dados_ataque().begin(), proto.dados_ataque().end(), [&id_arma] (
        const DadosAtaque& da) {
      return da.id_arma() == id_arma;
  });
}
#endif

// Retorna a penalidade do escudo de acordo com seu material. A penalidade é positiva (ou seja, penalidade 1 da -1).
int PenalidadeEscudo(const Tabelas& tabelas, const EntidadeProto& proto) {
  const auto& dd = proto.dados_defesa();
  int penalidade = tabelas.Escudo(dd.id_escudo()).penalidade_armadura();
  if (dd.escudo_obra_prima()) --penalidade;
  if (dd.material_escudo() == DESC_ADAMANTE) --penalidade;
  if (dd.material_escudo() == DESC_MADEIRA_NEGRA) penalidade -= 2;
  if (dd.material_escudo() == DESC_MITRAL) penalidade -= 3;
  return std::max(0, penalidade);
}

google::protobuf::RepeatedField<int> TiposDanoParaAtaqueFisico(const google::protobuf::RepeatedField<int>& tipos_dano) {
  google::protobuf::RepeatedField<int> tipos_ataque_fisico;
  for (int td : tipos_dano) {
    switch (td) {
      case TD_CORTANTE: tipos_ataque_fisico.Add(DESC_CORTANTE); break;
      case TD_PERFURANTE: tipos_ataque_fisico.Add(DESC_PERFURANTE); break;
      case TD_CONCUSSAO: tipos_ataque_fisico.Add(DESC_ESTOURANTE); break;
      default: ;
    }
  }
  return tipos_ataque_fisico;
}


// Poe e na primeira posicao de rf, movendo todos uma posicao para tras. Parametro 'e' fica invalido.
template <class T>
void InsereInicio(T* e, RepeatedPtrField<T>* rf) {
  rf->Add()->Swap(e);
  for (int i = static_cast<int>(rf->size()) - 1; i > 0; --i) {
    rf->SwapElements(i, i-1);
  }
}

DescritorAtaque StringParaDescritorAlinhamento(const std::string& alinhamento_str) {
  std::string normalizado = StringSemUtf8(alinhamento_str);
  std::transform(normalizado.begin(), normalizado.end(), normalizado.begin(), ::tolower);
  if (normalizado == "bom" || normalizado == "bem") return DESC_BEM;
  if (normalizado == "mal" || normalizado == "mau") return DESC_MAL;
  if (normalizado == "caos") return DESC_CAOS;
  if (normalizado == "lei" || normalizado == "leal") return DESC_LEAL;
  return DESC_NENHUM;
}


DescritorAtaque StringParaDescritorElemento(const std::string& elemento_str) {
  std::string normalizado = StringSemUtf8(elemento_str);
  std::transform(normalizado.begin(), normalizado.end(), normalizado.begin(), ::tolower);
  if (normalizado == "fogo") return DESC_FOGO;
  if (normalizado == "frio") return DESC_FRIO;
  if (normalizado == "acido") return DESC_ACIDO;
  if (normalizado == "sonico") return DESC_SONICO;
  if (normalizado == "eletricidade") return DESC_ELETRICIDADE;
  if (normalizado == "veneno") return DESC_VENENO;
  return DESC_NENHUM;
}

// Aplica o efeito. Alguns especificos serao feitos aqui. Alguns efeitos sao aplicados apenas uma vez e usam processado como controle.
void AplicaEfeitoComum(const ConsequenciaEvento& consequencia, EntidadeProto* proto) {
  VLOG(2) << "consequencia: " << consequencia.DebugString();
  AplicaBonusPenalidadeOuRemove(consequencia.atributos().forca(), proto->mutable_atributos()->mutable_forca());
  AplicaBonusPenalidadeOuRemove(consequencia.atributos().destreza(), proto->mutable_atributos()->mutable_destreza());
  AplicaBonusPenalidadeOuRemove(consequencia.atributos().constituicao(), proto->mutable_atributos()->mutable_constituicao());
  AplicaBonusPenalidadeOuRemove(consequencia.atributos().inteligencia(), proto->mutable_atributos()->mutable_inteligencia());
  AplicaBonusPenalidadeOuRemove(consequencia.atributos().sabedoria(), proto->mutable_atributos()->mutable_sabedoria());
  AplicaBonusPenalidadeOuRemove(consequencia.atributos().carisma(), proto->mutable_atributos()->mutable_carisma());
  AplicaBonusPenalidadeOuRemove(consequencia.dados_defesa().ca(), proto->mutable_dados_defesa()->mutable_ca());
  AplicaBonusPenalidadeOuRemove(consequencia.dados_defesa().salvacao_fortitude(), proto->mutable_dados_defesa()->mutable_salvacao_fortitude());
  AplicaBonusPenalidadeOuRemove(consequencia.dados_defesa().salvacao_reflexo(), proto->mutable_dados_defesa()->mutable_salvacao_reflexo());
  AplicaBonusPenalidadeOuRemove(consequencia.dados_defesa().salvacao_vontade(), proto->mutable_dados_defesa()->mutable_salvacao_vontade());
  AplicaBonusPenalidadeOuRemove(consequencia.dados_defesa().cura_acelerada(), proto->mutable_dados_defesa()->mutable_cura_acelerada());
  AplicaBonusPenalidadeOuRemove(consequencia.dados_defesa().resistencia_magia_variavel(), proto->mutable_dados_defesa()->mutable_resistencia_magia_variavel());
  for (const auto& re : consequencia.dados_defesa().resistencia_elementos()) {
    if (!re.has_id_efeito_modelo()) continue;
    if (re.valor() <= 0) {
      LimpaResistenciaElementoEfeitoModelo(re.descritor(), re.id_efeito_modelo(), proto);
    } else {
      *AchaOuCriaResistenciaElementoEfeitoModelo(re.descritor(), re.id_efeito_modelo(), proto) = re;
    }
  }
  for (const auto& rd : consequencia.dados_defesa().reducao_dano()) {
    if (!rd.has_id_efeito_modelo()) continue;
    if (rd.valor() <= 0) {
      LimpaReducaoDanoEfeitoModelo(rd.id_efeito_modelo(), proto);
    } else {
      *AchaOuCriaReducaoDanoEfeitoModelo(rd.id_efeito_modelo(), proto) = rd;
    }
  }

  AplicaBonusPenalidadeOuRemove(consequencia.bonus_iniciativa(), proto->mutable_bonus_iniciativa());
  for (auto& da : *proto->mutable_dados_ataque()) {
    if (!ConsequenciaAfetaDadosAtaque(consequencia, da)) continue;
    AplicaBonusPenalidadeOuRemove(consequencia.jogada_ataque(),
                                  da.mutable_bonus_ataque());
    if (!ConsequenciaAfetaDano(consequencia, da)) continue;
    AplicaBonusPenalidadeOuRemove(consequencia.jogada_dano(), da.mutable_bonus_dano());
  }

  AplicaBonusPenalidadeOuRemove(consequencia.tamanho(), proto->mutable_bonus_tamanho());
  for (const auto& dp : consequencia.dados_pericia()) {
    auto* pericia = PericiaOuNullptr(dp.id(), proto);
    if (pericia == nullptr) continue;
    AplicaBonusPenalidadeOuRemove(dp.bonus(), pericia->mutable_bonus());
  }

  AplicaBonusPenalidadeOuRemove(consequencia.movimento().terrestre_q(), proto->mutable_movimento()->mutable_terrestre_q());
  AplicaBonusPenalidadeOuRemove(consequencia.movimento().aereo_q(),     proto->mutable_movimento()->mutable_aereo_q());
  AplicaBonusPenalidadeOuRemove(consequencia.movimento().aquatico_q(),  proto->mutable_movimento()->mutable_aquatico_q());
  AplicaBonusPenalidadeOuRemove(consequencia.movimento().escavando_q(), proto->mutable_movimento()->mutable_escavando_q());
  AplicaBonusPenalidadeOuRemove(consequencia.movimento().escalando_q(), proto->mutable_movimento()->mutable_escalando_q());
}

bool ImuneMorte(const EntidadeProto& proto) {
  return TemTipoDnD(TIPO_CONSTRUCTO, proto) ||
         TemTipoDnD(TIPO_MORTO_VIVO, proto) ||
         PossuiEvento(EFEITO_PROTECAO_CONTRA_MORTE, proto);
}

void AplicaAlinhamento(DescritorAtaque desc, DadosAtaque* da) {
  if (desc == DESC_BEM || desc == DESC_MAL) {
    da->set_alinhamento_bem_mal(desc);
  } else if (desc == DESC_LEAL || desc == DESC_CAOS) {
    da->set_alinhamento_ordem_caos(desc);
  } else {
    LOG(ERROR) << "Aplicando descritor errado para alinhamento: " << desc;
  }
}

void AplicaInicioAtaqueIdUnico(int id_unico, const RepeatedPtrField<DadosAtaque>& dados_ataque, EntidadeProto* proto) {
  for (auto it = dados_ataque.rbegin(); it != dados_ataque.rend(); ++it) {
    DadosAtaque da = *it;
    da.set_id_unico_efeito(id_unico);
    if (!da.has_empunhadura()) {
      if (proto->dados_defesa().id_escudo().empty()) {
        da.set_empunhadura(EA_ARMA_APENAS);
      } else {
        da.set_empunhadura(EA_ARMA_ESCUDO);
      }
    }
    std::string grupo = da.grupo();
    InsereInicio(&da, proto->mutable_dados_ataque());
    proto->set_ultimo_grupo_acao(grupo);
    proto->set_ultima_acao(da.tipo_ataque());
  }
}

void AplicaFimAtaquePorIdUnico(int id_unico, EntidadeProto* proto) {
  RemoveSe<DadosAtaque>([id_unico](const DadosAtaque& da) {
    if (da.id_unico_efeito() == id_unico) {
      return true;
    }
    return false;
  }, proto->mutable_dados_ataque());
}

void AplicaLuz(const IluminacaoPontual& luz, EntidadeProto::Evento* evento, EntidadeProto* proto) {
  if (proto->has_luz()) {
    EntidadeProto proto_salvo;
    *proto_salvo.mutable_luz() = proto->luz();
    evento->set_estado_anterior(proto_salvo.SerializeAsString());
  }
  *proto->mutable_luz() = luz;
}

void AplicaEfeitoComumNaoProcessado(EntidadeProto::Evento* evento, const ConsequenciaEvento& consequencia, EntidadeProto* proto) {
  if (!consequencia.dados_ataque().empty()) {
    AplicaInicioAtaqueIdUnico(evento->id_unico(), consequencia.dados_ataque(), proto);
  }
  if (consequencia.has_luz()) {
    AplicaLuz(consequencia.luz(), evento, proto);
  }
  if (consequencia.has_pontos_vida_temporarios()) {
    AplicaBonusPenalidadeOuRemove(consequencia.pontos_vida_temporarios(), proto->mutable_pontos_vida_temporarios_por_fonte());
  }
}

// Entidade pode ser nullptr em testes.
bool AplicaEfeito(EntidadeProto::Evento* evento, const ConsequenciaEvento& consequencia, EntidadeProto* proto, Entidade* entidade) {
  AplicaEfeitoComum(consequencia, proto);
  if (!evento->processado()) {
    AplicaEfeitoComumNaoProcessado(evento, consequencia, proto);
  }
  // Aqui eh importante diferenciar entre return e break. Eventos que retornam nao seram considerados processados.
  switch (evento->id_efeito()) {
    case EFEITO_IMUNIDADE_FEITICO: {
      if (!evento->has_id_unico() || evento->complementos_str().empty() || evento->complementos_str(0).empty() ||
          c_any_of(proto->dados_defesa().imunidade_feiticos(),
            [evento] (const DadosDefesa::ImunidadeFeitico& imf) { return imf.id_unico() == evento->id_unico(); })) break;
      auto* imf = proto->mutable_dados_defesa()->add_imunidade_feiticos();
      imf->set_id_unico(evento->id_unico());
      imf->set_id_feitico(evento->complementos_str(0));
      break;
    }
    case EFEITO_METAMORFOSE_TORRIDA:
      if (!evento->processado()) {
        EntidadeProto proto_salvo;
        if (proto->has_info_textura()) *proto_salvo.mutable_info_textura() = proto->info_textura();
        if (proto->has_modelo_3d()) *proto_salvo.mutable_modelo_3d() = proto->modelo_3d();
        AtribuiBonus(BonusIndividualPorOrigem(TB_BASE, "base", proto->bonus_tamanho()), TB_BASE, "base", proto_salvo.mutable_bonus_tamanho());
        evento->set_estado_anterior(proto_salvo.SerializeAsString());

        if (entidade != nullptr) {
          EntidadeProto proto_sapo;
          proto_sapo.mutable_info_textura()->set_id("toad.png");
          entidade->AtualizaTexturas(proto_sapo);
          entidade->AtualizaModelo3d(proto_sapo);
        }
        // Para UI funcionar.
        proto->mutable_info_textura()->set_id("toad.png");
        proto->clear_modelo_3d();
        AtribuiBonus(TM_DIMINUTO, TB_BASE, "base", proto->mutable_bonus_tamanho());
      }
      break;
    case EFEITO_INCONSCIENTE:
      if (!evento->processado()) {
        if (!ImuneAcaoMental(*proto)) {
          EntidadeProto proto_salvo;
          proto_salvo.set_inconsciente(proto->inconsciente());
          proto_salvo.set_caida(proto->caida());
          evento->set_estado_anterior(proto_salvo.SerializeAsString());
          proto->set_inconsciente(true);
          proto->set_caida(true);
        }
      }
      break;
    case EFEITO_MORTE:
      if (!evento->processado()) {
        if (!ImuneMorte(*proto)) {
          EntidadeProto proto_salvo;
          proto_salvo.set_morta(proto->morta());
          proto_salvo.set_caida(proto->caida());
          proto_salvo.set_pontos_vida(proto->pontos_vida());
          evento->set_estado_anterior(proto_salvo.SerializeAsString());
          proto->set_morta(true);
          proto->set_caida(true);
          proto->set_pontos_vida(-100);
        }
      }
      break;
    case EFEITO_DRENAR_FORCA_VITAL:
      if (!evento->processado()) {
        // Gera os pontos de vida temporarios.
        const int tmp = RolaDado(8);
        AtribuiBonus(tmp, TB_SEM_NOME, "drenar forca vital", proto->mutable_pontos_vida_temporarios_por_fonte());
        // Nivel conjurador: hard coded.
        // Forca: pelo comum.
      }
      break;
    case EFEITO_PELE_ROCHOSA:
      if (!evento->processado()) {
        auto* pd = proto->mutable_dados_defesa()->add_reducao_dano();
        pd->add_descritores(DESC_ADAMANTE);
        pd->set_id_unico(evento->id_unico());
      }
      for (int i = 0; i < proto->dados_defesa().reducao_dano().size(); ++i) {
        if (proto->dados_defesa().reducao_dano(i).id_unico() != evento->id_unico()) continue;
        auto* rd = proto->mutable_dados_defesa()->mutable_reducao_dano(i);
        if (evento->complementos().empty() || evento->complementos(0) < 0) {
          rd->set_valor(0);
        } else {
          rd->set_valor(std::min(evento->complementos(0), 10));
        }
        break;
      }
      break;
    case EFEITO_FORMA_GASOSA:
      if (!evento->processado()) {
        auto* pd = proto->mutable_dados_defesa()->add_reducao_dano();
        pd->add_descritores(DESC_MAGICO);
        pd->set_valor(15);
        pd->set_id_unico(evento->id_unico());
      }
      break;
    case EFEITO_DRENAR_TEMPORARIO:
      if (!evento->processado()) {
        if (evento->complementos().empty() || evento->complementos(0) <= 0) return false;
        AtribuiBonus(
            evento->complementos(0), TB_SEM_NOME, StringPrintf("drenar temporario %d", evento->id_unico()), proto->mutable_niveis_negativos_dinamicos());
      }
      break;
    case EFEITO_VENENO:
      break;
    case EFEITO_INVISIBILIDADE:
      if (!PossuiEvento(EFEITO_POEIRA_OFUSCANTE, *proto) && !PossuiEvento(EFEITO_FOGO_DAS_FADAS, *proto)) {
        proto->set_visivel(false);
      }
      break;
    case EFEITO_POEIRA_OFUSCANTE:
      proto->set_visivel(true);
      proto->set_ignora_luz(true);
      break;
    case EFEITO_COMPETENCIA_PERICIA: {
      if (evento->complementos_str().empty()) return false;
      // Encontra a pericia do efeito.
      auto* pericia_proto = PericiaCriando(evento->complementos_str(0), proto);
      Bonus bonus;
      auto* bi = bonus.add_bonus_individual();
      bi->set_tipo(TB_COMPETENCIA);
      auto* po = bi->add_por_origem();
      po->set_valor(evento->complementos(0));
      po->set_origem(google::protobuf::StringPrintf("competencia (id: %d)", evento->id_unico()));
      AplicaBonusPenalidadeOuRemove(bonus, pericia_proto->mutable_bonus());
    }
    break;
    case EFEITO_RESISTENCIA_MAGIA:
      if (evento->complementos().empty()) return false;
      if (!evento->processado()) {
        // Gera os pontos de vida temporarios.
        int complemento = evento->complementos(0);
        if (complemento < 0) return false;
        auto* po = AtribuiBonusPenalidadeSeMaior(
            complemento, TB_BASE, "resistencia_magia", proto->mutable_dados_defesa()->mutable_resistencia_magia_variavel());
        if (evento->has_id_unico()) po->set_id_unico(evento->id_unico());
      }
    break;
    case EFEITO_ARMA_ABENCOADA: {
      // A ideia é sempre processar o evento pro tamanho alterar tb.
      auto* arma_abencoada = DadosAtaquePorIdUnico(evento->id_unico(), proto);
      auto* arma_abencoada_secundaria = DadosAtaquePorIdUnico(evento->id_unico(), proto, /*mao_ruim=*/true);
      if (arma_abencoada == nullptr) {
        LOG(ERROR) << "arma abençoada não foi criada";
        evento->set_rodadas(-1);
        break;
      }
      const DadosAtaque* da_primario = DadosAtaquePorIdArma("bordao", *proto);
      const DadosAtaque* da_secundario = DadosAtaquePorIdArma("bordao", *proto, /*mao_ruim=*/true);
      if (da_primario == nullptr) {
        da_primario = DadosAtaquePorIdArma("porrete", *proto);
      }
      if (da_primario == nullptr) {
        LOG(INFO) << "arma abençoada não encontrada";
        evento->set_rodadas(-1);
        break;
      }
      TamanhoEntidade tamanho = static_cast<TamanhoEntidade>(std::min(proto->tamanho() + 2, (int)TM_COLOSSAL));
      arma_abencoada->set_id_arma(da_primario->id_arma());
      arma_abencoada->set_empunhadura(da_primario->empunhadura());
      arma_abencoada->set_tamanho(tamanho);
      if (da_secundario != nullptr && arma_abencoada_secundaria != nullptr) {
        arma_abencoada_secundaria->set_id_arma(da_secundario->id_arma());
        arma_abencoada_secundaria->set_empunhadura(EA_MAO_RUIM);
        arma_abencoada_secundaria->set_tamanho(tamanho);
      } else if (arma_abencoada_secundaria != nullptr) {
        // Remove o segundo ataque criado.
        RemoveSe<DadosAtaque>([arma_abencoada_secundaria](const DadosAtaque& da) {
          return &da == arma_abencoada_secundaria;
        }, proto->mutable_dados_ataque());
      }
    }
    break;
    case EFEITO_PEDRA_ENCANTADA:
      if (!evento->processado()) {
        const auto* funda = DadosAtaquePorIdArma("funda", *proto);
        auto* pedra_encantada = DadosAtaquePorIdUnico(evento->id_unico(), proto);
        if (pedra_encantada == nullptr) {
          LOG(ERROR) << "Não encontrei o ataque da pedra encantada.";
          break;
        }
        if (funda != nullptr) {
          VLOG(1) << "encontrei a funda para pedra encantada.";
          pedra_encantada->set_id_arma("funda");
          pedra_encantada->set_empunhadura(funda->empunhadura());
          pedra_encantada->set_rotulo("pedra encantada com funda");
        } else {
          VLOG(1) << "nao encontrei a funda para pedra encantada.";
          pedra_encantada->set_tipo_ataque("Ataque a Distância");
          pedra_encantada->set_ataque_arremesso(true);
        }
      }
    break;
    case EFEITO_CONVOCAR_RELAMPAGOS: {
      auto* relampagos = DadosAtaquePorIdUnico(evento->id_unico(), proto);
      if (relampagos == nullptr) {
        evento->set_rodadas(-1);
        break;
      } else {
        if (!evento->processado()) {
          // Como a magia tem duracao de 10 rodadas por nivel, da pra inferir.
          int nivel_conjurador = evento->rodadas() / 10;
          relampagos->set_limite_vezes(std::min(nivel_conjurador, 10));
        }
      }
    }
    break;
    case EFEITO_BOM_FRUTO:
      if (!evento->processado()) {
        for (auto& da : *proto->mutable_dados_ataque()) {
          if (!da.has_id_unico_efeito() || da.id_unico_efeito() != evento->id_unico()) continue;
          da.set_limite_vezes(RolaValor("2d4"));
          break;
        }
      }
    break;
    case EFEITO_CRIAR_CHAMA:
      if (!evento->processado()) {
        for (auto& da : *proto->mutable_dados_ataque()) {
          if (!da.has_id_unico_efeito() || da.id_unico_efeito() != evento->id_unico()) continue;
          // Como a magia tem duracao de 10 rodadas por nivel, da pra inferir.
          int nivel_conjurador = evento->rodadas() / 10;
          int mod = std::min(5, nivel_conjurador);
          da.set_dano_basico_fixo(StringPrintf("%s+%d", da.dano_basico_fixo().c_str(), mod));
          da.set_limite_vezes(std::max(1, nivel_conjurador));
          break;
        }
      } else {
        // Se ja processou, elimina o evento se gastou tudo.
        bool encontrou = false;
        for (auto& da : *proto->mutable_dados_ataque()) {
          if (!da.has_id_unico_efeito() || da.id_unico_efeito() != evento->id_unico()) continue;
          // Como a magia tem duracao de 10 rodadas por nivel, da pra inferir.
          encontrou = true;
          break;
        }
        if (!encontrou) {
          // Fim do evento.
          evento->set_rodadas(-1);
        }
      }
    break;
    case EFEITO_LAMINA_FLAMEJANTE:
      if (!evento->processado()) {
        for (auto& da : *proto->mutable_dados_ataque()) {
          if (!da.has_id_unico_efeito() || da.id_unico_efeito() != evento->id_unico()) continue;
          // Como a magia tem duracao de 10 rodadas por nivel, da pra inferir.
          int nivel_conjurador = evento->rodadas() / 10;
          int mod = std::min(10, nivel_conjurador / 2);
          da.set_dano_basico_fixo(StringPrintf("%s+%d", da.dano_basico_fixo().c_str(), mod));
          break;
        }
      }
    break;
    case EFEITO_ABENCOAR_ARMA: {
      if (evento->complementos_str().empty()) return false;
      std::vector<DadosAtaque*> das = DadosAtaquePorRotulo(evento->complementos_str(0), proto);
      for (auto* da : das) {
        AplicaAlinhamento(DESC_BEM, da);
      }
    }
    break;
    case EFEITO_PRESA_MAGICA: {
      // TODO
      if (evento->complementos_str().empty()) {
        // TODO encontrar o ataque primario.
        return false;
      }
      int valor = 1;
      std::vector<DadosAtaque*> das = DadosAtaquePorRotulo(evento->complementos_str(0), proto);
      // TODO
      // Pegar apenas um ataque de cada grupo que case com o id_arma passado.
      for (auto* da : das) {
        AtribuiBonusPenalidadeSeMaior(
            valor, TB_MELHORIA, evento->id_efeito() == EFEITO_ARMA_MAGICA ? "arma_magica_magia" : "presa_magica_magia", da->mutable_bonus_ataque());
      }
    }
    break;
    case EFEITO_ARMA_MAGICA: {
      if (evento->complementos_str().empty()) return false;
      int valor = 1;
      if (!evento->complementos().empty()) {
        valor = std::max(0, std::min(evento->complementos(0), 5));
      }
      std::vector<DadosAtaque*> das = DadosAtaquePorRotulo(evento->complementos_str(0), proto);
      for (auto* da : das) {
        AtribuiBonusPenalidadeSeMaior(
            valor, TB_MELHORIA, evento->id_efeito() == EFEITO_ARMA_MAGICA ? "arma_magica_magia" : "presa_magica_magia", da->mutable_bonus_ataque());
      }
    }
    break;
    case EFEITO_TENDENCIA_EM_ARMA: {
      if (evento->complementos_str().size() != 2) return false;
      DescritorAtaque desc = StringParaDescritorAlinhamento(evento->complementos_str(1));
      if (desc == DESC_NENHUM) return false;
      std::vector<DadosAtaque*> das = DadosAtaquePorRotulo(evento->complementos_str(0), proto);
      for (auto* da : das) {
        AplicaAlinhamento(desc, da);
      }
    }
    break;
    case EFEITO_RESISTENCIA_ELEMENTOS: {
      if (evento->complementos_str().size() != 2) return false;
      DescritorAtaque descritor = StringParaDescritorElemento(evento->complementos_str(0));
      if (descritor == DESC_NENHUM) {
        LOG(ERROR) << "descritor invalido: " << evento->complementos_str(0);
        return false;
      }
      int valor = atoi(evento->complementos_str(1).c_str());
      if (valor <= 0 || valor > 1000) return false;
      ResistenciaElementos re;
      re.set_valor(valor);
      re.set_descritor(descritor);
      re.set_id_unico(evento->id_unico());
      AchaOuCriaResistenciaElementoIdUnico(descritor, evento->id_unico(), proto)->Swap(&re);
    }
    break;
    case EFEITO_SONO: {
      proto->set_caida(true);
    }
    break;
    default: ;
  }
  return true;
}

void AplicaFimFuriaBarbaro(EntidadeProto* proto) {
  if (Nivel("barbaro", *proto) >= 17) return;
  auto* evento = proto->add_evento();
  evento->set_id_efeito(EFEITO_FADIGA);
  evento->set_descricao("fadiga_furia_barbaro");
  // Dura pelo resto do encontro.
  evento->set_rodadas(100);
}

void AplicaFimAlinhamentoArma(const std::string& rotulo, EntidadeProto* proto) {
  // Encontra o dado de ataque.
  for (auto& da : *proto->mutable_dados_ataque()) {
    if (da.rotulo() == rotulo) {
      da.clear_alinhamento_bem_mal();
      da.clear_alinhamento_ordem_caos();
    }
  }
}

void AplicaFimLuz(const EntidadeProto::Evento& evento, EntidadeProto* proto) {
  if (evento.has_estado_anterior()) {
    EntidadeProto proto_salvo;
    proto_salvo.ParseFromString(evento.estado_anterior());
    if (proto_salvo.has_luz()) {
      *proto->mutable_luz() = proto_salvo.luz();
      return;
    }
  }
  proto->clear_luz();
}

void AplicaFimEfeitosProcessados(const EntidadeProto::Evento& evento, const ConsequenciaEvento& consequencia, EntidadeProto* proto) {
  if (!consequencia.dados_ataque().empty()) {
    AplicaFimAtaquePorIdUnico(evento.id_unico(), proto);
  }
  if (consequencia.has_luz()) {
    AplicaFimLuz(evento, proto);
  }
  if (consequencia.has_pontos_vida_temporarios()) {
    AplicaBonusPenalidadeOuRemove(consequencia.pontos_vida_temporarios(), proto->mutable_pontos_vida_temporarios_por_fonte());
  }
}

void AplicaFimEfeito(const EntidadeProto::Evento& evento, const ConsequenciaEvento& consequencia, EntidadeProto* proto, Entidade* entidade) {
  AplicaEfeitoComum(consequencia, proto);
  AplicaFimEfeitosProcessados(evento, consequencia, proto);
  switch (evento.id_efeito()) {
    case EFEITO_IMUNIDADE_FEITICO: {
      if (!evento.has_id_unico()) break;
      int indice = -1;
      for (int i = 0; i < proto->dados_defesa().imunidade_feiticos().size(); ++i) {
        const auto& imf = proto->dados_defesa().imunidade_feiticos(i);
        if (imf.id_unico() != evento.id_unico()) continue;
        indice = i;
        break;
      }
      if (indice == -1) break;
      proto->mutable_dados_defesa()->mutable_imunidade_feiticos()->DeleteSubrange(indice, 1);
      break;
    }
    case EFEITO_INCONSCIENTE: {
      if (!evento.has_estado_anterior()) break;
      EntidadeProto proto_salvo;
      proto_salvo.ParseFromString(evento.estado_anterior());
      proto->set_caida(proto_salvo.caida());
      proto->set_inconsciente(proto_salvo.inconsciente());
    }
    break;
    case EFEITO_MORTE: {
      if (!evento.has_estado_anterior()) break;
      EntidadeProto proto_salvo;
      proto_salvo.ParseFromString(evento.estado_anterior());
      proto->set_caida(proto_salvo.caida());
      proto->set_morta(proto_salvo.morta());
      proto->set_pontos_vida(proto_salvo.pontos_vida());
    }
    break;
    case EFEITO_METAMORFOSE_TORRIDA: {
      if (!evento.has_estado_anterior()) break;
      EntidadeProto proto_salvo;
      proto_salvo.ParseFromString(evento.estado_anterior());
      if (entidade != nullptr) {
        entidade->AtualizaTexturas(proto_salvo);
        if (proto_salvo.has_modelo_3d()) {
          entidade->AtualizaModelo3d(proto_salvo);
        }
      } else {
        if (proto_salvo.has_info_textura()) *proto->mutable_info_textura() = proto_salvo.info_textura();
        if (proto_salvo.has_modelo_3d()) *proto->mutable_modelo_3d() = proto_salvo.modelo_3d();
      }
      AtribuiBonus(BonusIndividualPorOrigem(TB_BASE, "base", proto_salvo.bonus_tamanho()), TB_BASE, "base", proto->mutable_bonus_tamanho());
    }
    break;
    case EFEITO_DRENAR_FORCA_VITAL:
      LimpaBonus(TB_SEM_NOME, "drenar forca vital", proto->mutable_pontos_vida_temporarios_por_fonte());
      break;
    case EFEITO_FORMA_GASOSA:
      for (int i = 0; i < proto->dados_defesa().reducao_dano().size(); ++i) {
        if (proto->dados_defesa().reducao_dano(i).id_unico() == evento.id_unico()) {
          proto->mutable_dados_defesa()->mutable_reducao_dano()->DeleteSubrange(i, 1);
          break;
        }
      }
      break;
    case EFEITO_PELE_ROCHOSA:
      for (int i = 0; i < proto->dados_defesa().reducao_dano().size(); ++i) {
        if (proto->dados_defesa().reducao_dano(i).id_unico() == evento.id_unico()) {
          proto->mutable_dados_defesa()->mutable_reducao_dano()->DeleteSubrange(i, 1);
        }
      }
      break;
    case EFEITO_DRENAR_TEMPORARIO:
      RemoveBonus(TB_SEM_NOME, StringPrintf("drenar temporario %d", evento.id_unico()), proto->mutable_niveis_negativos_dinamicos());
      break;
    case EFEITO_VENENO:
    break;
    case EFEITO_INVISIBILIDADE:
      proto->set_visivel(true);
    break;
    case EFEITO_POEIRA_OFUSCANTE:
      proto->set_ignora_luz(false);
    break;
    case EFEITO_COMPETENCIA_PERICIA: {
      if (evento.complementos_str().empty()) return;
      // Encontra a pericia do efeito.
      auto* pericia_proto = PericiaCriando(evento.complementos_str(0), proto);
      Bonus bonus;
      auto* bi = bonus.add_bonus_individual();
      bi->set_tipo(TB_COMPETENCIA);
      auto* po = bi->add_por_origem();
      po->set_valor(0);
      po->set_origem(google::protobuf::StringPrintf("competencia (id: %d)", evento.id_unico()));
      AplicaBonusPenalidadeOuRemove(bonus, pericia_proto->mutable_bonus());
    }
    break;
    case EFEITO_RESISTENCIA_MAGIA: {
      if (evento.has_id_unico()) {
        // Se tiver id unico, respeita o id.
        auto* bi = BonusIndividualSePresente(TB_BASE, proto->mutable_dados_defesa()->mutable_resistencia_magia_variavel());
        RemoveSe<BonusIndividual::PorOrigem>([&evento] (const BonusIndividual::PorOrigem& ipo) {
          return ipo.id_unico() == evento.id_unico();
        }, bi->mutable_por_origem());
      } else {
        // Nao tem id, remove a ajuda por completo. Pode dar merda.
        LOG(WARNING) << "Removendo ajuda sem id unico.";
        RemoveBonus(TB_BASE, "resistencia_magia", proto->mutable_dados_defesa()->mutable_resistencia_magia_variavel());
      }
    }
    break;
    case EFEITO_FURIA_BARBARO:
      AplicaFimFuriaBarbaro(proto);
    break;
    case EFEITO_ABENCOAR_ARMA:
    case EFEITO_TENDENCIA_EM_ARMA: {
      if (evento.complementos_str().size() >= 1) {
        AplicaFimAlinhamentoArma(evento.complementos_str(0), proto);
      }
    }
    break;
    case EFEITO_PRESA_MAGICA:
    case EFEITO_ARMA_MAGICA: {
      if (evento.complementos_str().empty()) return;
      std::vector<DadosAtaque*> das = DadosAtaquePorRotulo(evento.complementos_str(0), proto);
      for (auto* da : das) {
        LimpaBonus(TB_MELHORIA, evento.id_efeito() == EFEITO_ARMA_MAGICA ? "arma_magica_magia" : "presa_magica_magia", da->mutable_bonus_ataque());
      }
    }
    break;
    case EFEITO_RESISTENCIA_ELEMENTOS: {
      if (evento.complementos_str().empty()) {
        LOG(ERROR) << "evento sem complentos" << evento.DebugString();
        return;
      }
      DescritorAtaque descritor = StringParaDescritorElemento(evento.complementos_str(0));
      if (descritor == DESC_NENHUM) {
        LOG(ERROR) << "descritor invalido: " << evento.complementos_str(0);
        return;
      }
      LimpaResistenciaElementoIdUnico(descritor, evento.id_unico(), proto);
    }
    break;
    default: ;
  }
}

// Adiciona o id unico a cada origem de bonus.
// Preenche os bonus de acordo com o complemento se houver.
void PreencheOrigemValor(
    const std::string& origem, const google::protobuf::RepeatedField<int>& complementos, Bonus* bonus) {
  for (auto& bi : *bonus->mutable_bonus_individual()) {
    for (auto& po : *bi.mutable_por_origem()) {
      if (origem.empty()) {
        po.set_origem(po.origem());
      } else {
        po.set_origem(StringPrintf("%s, %s", po.origem().c_str(), origem.c_str()));
      }
      if (po.has_indice_complemento() && po.indice_complemento() >= 0 && po.indice_complemento() < complementos.size()) {
        po.set_valor(complementos.Get(po.indice_complemento()));
      }
    }
  }
}

// Dado um efeito, preenche o valor com zero e seta origem para o id do evento.
void PreencheOrigemZeraValor(const std::string& origem, Bonus* bonus) {
  for (auto& bi : *bonus->mutable_bonus_individual()) {
    for (auto& po : *bi.mutable_por_origem()) {
      if (origem.empty()) {
        po.set_origem(po.origem());
      } else {
        po.set_origem(StringPrintf("%s, %s", po.origem().c_str(), origem.c_str()));
      }
      po.set_valor(0);
    }
  }
}

// Caso a consequencia use complemento, preenchera os valores existentes com
// ela.
ConsequenciaEvento PreencheConsequencia(
    const std::string& origem,
    const google::protobuf::RepeatedField<int>& complementos,
    const ConsequenciaEvento& consequencia_original) {
  ConsequenciaEvento c(consequencia_original);
  if (c.atributos().has_forca())        PreencheOrigemValor(origem, complementos, c.mutable_atributos()->mutable_forca());
  if (c.atributos().has_destreza())     PreencheOrigemValor(origem, complementos, c.mutable_atributos()->mutable_destreza());
  if (c.atributos().has_constituicao()) PreencheOrigemValor(origem, complementos, c.mutable_atributos()->mutable_constituicao());
  if (c.atributos().has_inteligencia()) PreencheOrigemValor(origem, complementos, c.mutable_atributos()->mutable_inteligencia());
  if (c.atributos().has_sabedoria())    PreencheOrigemValor(origem, complementos, c.mutable_atributos()->mutable_sabedoria());
  if (c.atributos().has_carisma())      PreencheOrigemValor(origem, complementos, c.mutable_atributos()->mutable_carisma());
  if (c.dados_defesa().has_ca())        PreencheOrigemValor(origem, complementos, c.mutable_dados_defesa()->mutable_ca());
  if (c.dados_defesa().has_salvacao_fortitude()) PreencheOrigemValor(origem, complementos, c.mutable_dados_defesa()->mutable_salvacao_fortitude());
  if (c.dados_defesa().has_salvacao_vontade())   PreencheOrigemValor(origem, complementos, c.mutable_dados_defesa()->mutable_salvacao_vontade());
  if (c.dados_defesa().has_salvacao_reflexo())   PreencheOrigemValor(origem, complementos, c.mutable_dados_defesa()->mutable_salvacao_reflexo());
  if (c.dados_defesa().has_cura_acelerada())   PreencheOrigemValor(origem, complementos, c.mutable_dados_defesa()->mutable_cura_acelerada());
  if (c.dados_defesa().has_resistencia_magia_variavel())   PreencheOrigemValor(origem, complementos, c.mutable_dados_defesa()->mutable_resistencia_magia_variavel());
  for (auto& re : *c.mutable_dados_defesa()->mutable_resistencia_elementos()) {
    if (!re.has_indice_complemento() || (re.indice_complemento() < 0) || (re.indice_complemento() >= complementos.size())) continue;
    re.set_valor(complementos.Get(re.indice_complemento()));
  }
  for (auto& rd : *c.mutable_dados_defesa()->mutable_reducao_dano()) {
    if (!rd.has_indice_complemento() || (rd.indice_complemento() < 0) || (rd.indice_complemento() >= complementos.size())) continue;
    rd.set_valor(complementos.Get(rd.indice_complemento()));
  }
  if (c.has_jogada_ataque())            PreencheOrigemValor(origem, complementos, c.mutable_jogada_ataque());
  if (c.has_jogada_dano())              PreencheOrigemValor(origem, complementos, c.mutable_jogada_dano());
  if (c.has_tamanho())                  PreencheOrigemValor(origem, complementos, c.mutable_tamanho());
  if (c.has_bonus_iniciativa())         PreencheOrigemValor(origem, complementos, c.mutable_bonus_iniciativa());
  for (auto& dp : *c.mutable_dados_pericia()) {
    PreencheOrigemValor(origem, complementos, dp.mutable_bonus());
  }
  if (c.has_pontos_vida_temporarios())  PreencheOrigemValor(origem, complementos, c.mutable_pontos_vida_temporarios());
  return c;
}

// Caso a consequencia use complemento, preenchera os valores existentes com ela.
ConsequenciaEvento PreencheConsequenciaFim(const std::string& origem, const ConsequenciaEvento& consequencia_original) {
  ConsequenciaEvento c(consequencia_original);
  if (c.atributos().has_forca())        PreencheOrigemZeraValor(origem, c.mutable_atributos()->mutable_forca());
  if (c.atributos().has_destreza())     PreencheOrigemZeraValor(origem, c.mutable_atributos()->mutable_destreza());
  if (c.atributos().has_constituicao()) PreencheOrigemZeraValor(origem, c.mutable_atributos()->mutable_constituicao());
  if (c.atributos().has_inteligencia()) PreencheOrigemZeraValor(origem, c.mutable_atributos()->mutable_inteligencia());
  if (c.atributos().has_sabedoria())    PreencheOrigemZeraValor(origem, c.mutable_atributos()->mutable_sabedoria());
  if (c.atributos().has_carisma())      PreencheOrigemZeraValor(origem, c.mutable_atributos()->mutable_carisma());
  if (c.dados_defesa().has_ca())        PreencheOrigemZeraValor(origem, c.mutable_dados_defesa()->mutable_ca());
  if (c.dados_defesa().has_salvacao_fortitude()) PreencheOrigemZeraValor(origem, c.mutable_dados_defesa()->mutable_salvacao_fortitude());
  if (c.dados_defesa().has_salvacao_vontade())   PreencheOrigemZeraValor(origem, c.mutable_dados_defesa()->mutable_salvacao_vontade());
  if (c.dados_defesa().has_salvacao_reflexo())   PreencheOrigemZeraValor(origem, c.mutable_dados_defesa()->mutable_salvacao_reflexo());
  if (c.dados_defesa().has_cura_acelerada())     PreencheOrigemZeraValor(origem, c.mutable_dados_defesa()->mutable_cura_acelerada());
  if (c.dados_defesa().has_resistencia_magia_variavel())     PreencheOrigemZeraValor(origem, c.mutable_dados_defesa()->mutable_resistencia_magia_variavel());
  if (c.has_jogada_ataque())            PreencheOrigemZeraValor(origem, c.mutable_jogada_ataque());
  if (c.has_jogada_dano())              PreencheOrigemZeraValor(origem, c.mutable_jogada_dano());
  if (c.has_tamanho())                  PreencheOrigemZeraValor(origem, c.mutable_tamanho());
  for (auto& re : *c.mutable_dados_defesa()->mutable_resistencia_elementos()) {
    re.set_valor(0);
  }
  for (auto& dp : *c.mutable_dados_pericia()) {
    PreencheOrigemZeraValor(origem, dp.mutable_bonus());
  }
  PreencheOrigemZeraValor(origem, c.mutable_pontos_vida_temporarios());
  return c;
}

// Adiciona eventos nao presentes.
// Retorna o bonus base de uma salvacao, dado o nivel. Forte indica que a salvacao eh forte.
int CalculaBaseSalvacao(bool forte, int nivel) {
  if (forte) {
    return 2 + (nivel / 2);
  } else {
    return nivel / 3;
  }
}


Bonus* BonusSalvacao(TipoSalvacao ts, EntidadeProto* proto) {
  switch (ts) {
    case TS_FORTITUDE: return proto->mutable_dados_defesa()->mutable_salvacao_fortitude();
    case TS_REFLEXO: return proto->mutable_dados_defesa()->mutable_salvacao_reflexo();
    case TS_VONTADE: return proto->mutable_dados_defesa()->mutable_salvacao_vontade();
    default:
      LOG(ERROR) << "Tipo de salvacao invalido: " << (int)ts;
      return proto->mutable_dados_defesa()->mutable_salvacao_fortitude();
  }
}

int ReducaoDanoBarbaro(int nivel) {
  if (nivel < 6) return 0;
  else if (nivel < 10) return 1;
  else if (nivel < 13) return 2;
  else if (nivel < 16) return 3;
  else if (nivel < 19) return 4;
  return 5;
}

int FeiticosBonusPorAtributoPorNivel(int nivel, const Bonus& atributo) {
  int modificador_atributo = ModificadorAtributo(atributo);
  // Nunca ha bonus de nivel 0.
  if (nivel <= 0 || nivel > 9) return 0;
  if (modificador_atributo < nivel) return 0;
  return static_cast<int>(floor(((modificador_atributo - nivel) / 4) + 1));
}

void RecomputaAlteracaoConstituicao(int total_antes, int total_depois, EntidadeProto* proto) {
  if (total_antes == total_depois) return;
  VLOG(1) << "Recomputando alteracao de constituicao";
  VLOG(1) << "max original: " << proto->max_pontos_vida();
  VLOG(1) << "pv original: " << proto->pontos_vida();
  if (total_depois > total_antes) {
    // Incremento de constituicao.
    const int delta_con = total_depois - total_antes;
    const int delta_pv = Nivel(*proto) * (((total_antes % 2) == 1 && (delta_con % 2) == 1) ? (delta_con / 2) + 1 : delta_con / 2);
    VLOG(1) << "aumentando CON de: " << total_antes << " para: " << total_depois << ", delta_pv: " << delta_pv;
    proto->set_max_pontos_vida(proto->max_pontos_vida() + delta_pv);
    proto->set_pontos_vida(proto->pontos_vida() + delta_pv);
    if (proto->pontos_vida() >= 0) {
      proto->set_morta(false);
    }
  } else if (total_antes > total_depois) {
    // Decremento de constituicao.
    const int delta_con = total_antes - total_depois;
    const int delta_pv = Nivel(*proto) * (((total_antes % 2) == 0 && (delta_con % 2) == 1) ? (delta_con / 2) + 1 : delta_con / 2);
    VLOG(1) << "diminuindo CON de: " << total_antes << " para: " << total_depois << ", delta_pv: " << delta_pv;
    proto->set_max_pontos_vida(proto->max_pontos_vida() - delta_pv);
    proto->set_pontos_vida(proto->pontos_vida() - delta_pv);
    if (proto->pontos_vida() < 0) {
      proto->set_caida(true);
      proto->set_morta(true);
    }
  }
  VLOG(1) << "max modificado: " << proto->max_pontos_vida();
  VLOG(1) << "pv modificado: " << proto->pontos_vida();
}

void CombinaFeiticosClasse(RepeatedPtrField<EntidadeProto::InfoFeiticosClasse>* feiticos_classes) {
  std::unordered_map<std::string, EntidadeProto::InfoFeiticosClasse*> mapa;
  for (auto& fc : *feiticos_classes) {
    if (fc.operacao() == OC_NOP) {
      mapa[fc.id_classe()] = &fc;
    }
  }
  std::vector<int> a_remover;
  int indice = 0;
  for (const auto& fc : *feiticos_classes) {
    switch (fc.operacao()) {
      case OC_SOBRESCREVE:
      case OC_COMBINA:
        if (mapa.find(fc.id_classe()) == mapa.end()) {
          LOG(ERROR) << "Operação invalida: nao ha feitico para classe " << fc.id_classe();
          continue;
        }
        if (fc.operacao() == OC_SOBRESCREVE) { *mapa[fc.id_classe()] = fc; }
        else { mapa[fc.id_classe()]->MergeFrom(fc); }
        a_remover.push_back(indice);
        break;
      default: ;
    }
    mapa[fc.id_classe()]->clear_operacao();
    ++indice;
  }

  for (auto it = a_remover.rbegin(); it != a_remover.rend(); ++it) {
    feiticos_classes->DeleteSubrange(*it, 1);
  }
}

void CombinaFeiticosPorNivel(RepeatedPtrField<EntidadeProto::FeiticosPorNivel>* feiticos_por_niveis) {
  std::unordered_map<int, EntidadeProto::FeiticosPorNivel*> mapa;
  for (auto& fn : *feiticos_por_niveis) {
    if (!fn.has_operacao()) {
      mapa[fn.nivel()] = &fn;
    }
  }
  std::vector<int> a_remover;
  int indice = 0;
  for (const auto& fn : *feiticos_por_niveis) {
    switch (fn.operacao()) {
      case OC_SOBRESCREVE:
      case OC_COMBINA:
        if (mapa.find(fn.nivel()) == mapa.end()) {
          LOG(ERROR) << "Operação invalida: nao ha feitico do nivel " << fn.nivel();
          continue;
        }
        if (fn.operacao() == OC_SOBRESCREVE) { *mapa[fn.nivel()] = fn; }
        else { mapa[fn.nivel()]->MergeFrom(fn); }
        break;
      default: ;
    }
    mapa[fn.nivel()]->clear_operacao();
    ++indice;
  }
  for (auto it = a_remover.rbegin(); it != a_remover.rend(); ++it) {
    feiticos_por_niveis->DeleteSubrange(*it, 1);
  }
}

// Indica a restricao do slot e os dados da restricao.
struct RestricaoFeitico {
  bool ha_restricao = false;
  bool dominio = false;
  std::vector<std::string> dominios;
  std::string especializacao;
  std::vector<std::string> escolas_proibidas;
};

void RecomputaDependenciasDominios(const Tabelas& tabelas, EntidadeProto* proto) {
  auto* ifc_ptr = FeiticosClasseOuNullptr("clerigo", proto);
  const auto& ifc = ifc_ptr == nullptr ? EntidadeProto::InfoFeiticosClasse::default_instance() : *ifc_ptr;
  for (const auto& d : ifc.dominios()) {
    const auto& dominio_tabelado = tabelas.Dominio(d);
    for (const auto& da_tabelado : dominio_tabelado.dados_ataque()) {
      if (c_none_of(proto->dados_ataque(), [&dominio_tabelado, &da_tabelado](const DadosAtaque& da) {
            return da.has_dominio() && da.dominio() == dominio_tabelado.id() && da.rotulo() == da_tabelado.rotulo(); })) {
        *proto->add_dados_ataque() = da_tabelado;
      }
    }
  }
  RemoveSe<DadosAtaque>([&ifc](const DadosAtaque& da) {
      return da.has_dominio() && c_none(ifc.dominios(), da.dominio());
  }, proto->mutable_dados_ataque());
}

void ConfiguraSlotsRestritos(bool ha_restricao, RepeatedPtrField<EntidadeProto::InfoLancar>* para_lancar) {
  if (!ha_restricao) {
    VLOG(1) << "Nao ha slots restritos, limpando restricoes";
    for (auto& il : *para_lancar) {
      il.clear_restrito();
    }
    return;
  }
  if (para_lancar->empty()) return;
  int num_restritos = std::count_if(para_lancar->begin(), para_lancar->end(), [](const EntidadeProto::InfoLancar& il) {
    return il.restrito();
  });
  if (num_restritos == 1) {
    VLOG(1) << "slots restritos corretos";
    return;
  }
  if (num_restritos > 1) {
    LOG(INFO) << "Mais de um slot restritos encontrado, corrigindo para apenas 1.";
    bool found = false;
    for (auto& il : *para_lancar) {
      if (!found && il.restrito()) {
        found = true;
      } else {
        il.clear_restrito();
      }
    }
    return;
  }
  if (num_restritos == 0) {
    LOG(INFO) << "Nenhum slot restrito encontrado, setando o primeiro.";
    // Marca o primeiro como restrito.
    para_lancar->Mutable(0)->set_restrito(true);
  }
}

EntidadeProto::InfoLancar* SlotRestrito(RepeatedPtrField<EntidadeProto::InfoLancar>* para_lancar) {
  for (auto& il : *para_lancar) {
    if (il.restrito()) return &il;
  }
  return nullptr;
}

// Limpa os feiticos que nao corresponderem as restricoes.
void CorrigeSlotsRestritos(
    const Tabelas& tabelas, const RestricaoFeitico& restricao, const EntidadeProto::InfoFeiticosClasse& fc,
    RepeatedPtrField<EntidadeProto::InfoLancar>* para_lancar) {
  auto* il = SlotRestrito(para_lancar);
  if (il == nullptr) {
    LOG(ERROR) << "Nao achei slot restrito! Nivel" << il->nivel_conhecido() << ", indice: " << il->indice_conhecido();
    return;
  }
  if (il->nivel_conhecido() < 0 || il->nivel_conhecido() >= fc.feiticos_por_nivel_size()) {
    VLOG(1) << "nivel conhecido invalido: " << il->nivel_conhecido() << ", size: " << fc.feiticos_por_nivel_size();
    il->clear_nivel_conhecido();
    il->clear_indice_conhecido();
    return;
  }
  if (il->indice_conhecido() < 0 || il->indice_conhecido() >= fc.feiticos_por_nivel(il->nivel_conhecido()).conhecidos_size()) {
    VLOG(1) << "indice conhecido invalido: " << il->indice_conhecido()
      << ", size: " << fc.feiticos_por_nivel(il->nivel_conhecido()).conhecidos_size();
    il->clear_indice_conhecido();
    return;
  }
  // Slot restrito: aqui so pode usar feiticos de dominio ou especifico de escola.
  const auto& feitico_conhecido = fc.feiticos_por_nivel(il->nivel_conhecido()).conhecidos(il->indice_conhecido());
  const auto& feitico_tabelado = tabelas.Feitico(feitico_conhecido.id());
  // Nao limpa feiticos nao tabelados.
  if (!feitico_tabelado.has_id()) {
    VLOG(1) << "nao vou corrigir slot nao tabelado. Nivel " << il->nivel_conhecido() << ", indice: " << il->indice_conhecido();
    return;
  }
  bool ok = false;
  if (restricao.dominio) {
    ok = c_any_of(feitico_tabelado.info_classes(), [&restricao](const ArmaProto::InfoClasseParaFeitico& classe) {
        return classe.dominio() && c_any(restricao.dominios, classe.id()); });
  } else {
    ok = c_none(restricao.escolas_proibidas, feitico_tabelado.escola());
  }
  if (!ok) {
    VLOG(1) << "corrigindo slot restrito nivel " << il->nivel_conhecido() << ", indice: " << il->indice_conhecido();
    il->clear_nivel_conhecido();
    il->clear_indice_conhecido();
  }
}

// restricoes: vetor com as escolas ou com "dominio"
void ConfiguraSlotsRestritosCorrigindo(
    const Tabelas& tabelas, const RestricaoFeitico& restricao, const EntidadeProto::InfoFeiticosClasse& fc,
    RepeatedPtrField<EntidadeProto::InfoLancar>* para_lancar) {
  ConfiguraSlotsRestritos(restricao.ha_restricao, para_lancar);
  if (restricao.ha_restricao) {
    CorrigeSlotsRestritos(tabelas, restricao, fc, para_lancar);
  }
}

// Retorna true se o feitico for da escola proibida segundo a restricao.
bool FeiticoEscolaProibida(const RestricaoFeitico& rf, const ArmaProto& feitico_tabelado) {
  return ent::FeiticoEscolaProibida(rf.escolas_proibidas, feitico_tabelado); 
}

// Retorna true se o feitico for de clerigo.
bool FeiticoClerigo(const ArmaProto& feitico_tabelado) {
  return c_any_of(feitico_tabelado.info_classes(), [](const ArmaProto::InfoClasseParaFeitico& classe) {
    return classe.id() == "clerigo";
  });
}

// Retorna true se o feitico for de dominio de acordo com a restricao.
bool FeiticoDominio(const RestricaoFeitico& rf, const ArmaProto& feitico_tabelado) {
  return ent::FeiticoDominio(rf.dominios, feitico_tabelado);
}

// Retorna true se o feitico for da escola especializada, segundo restricao.
bool FeiticoEscolaEspecializada(const RestricaoFeitico& rf, const ArmaProto& feitico_tabelado) {
  return feitico_tabelado.escola() == rf.especializacao;
}

void ComputaFeiticoAleatorioComum(
    int nivel_magia, const EntidadeProto::InfoFeiticosClasse& fc,
    EntidadeProto::InfoLancar* il) {
  if (nivel_magia < 0 || nivel_magia >= fc.feiticos_por_nivel_size()) {
    LOG(ERROR) << "nivel magia invalido: " << nivel_magia;
    return;
  }

  if (!il->has_nivel_conhecido()) {
    il->set_nivel_conhecido(nivel_magia);
  }
}

void SorteiaIndice(const std::vector<int>& indices_validos, EntidadeProto::InfoLancar* il) {
  if (indices_validos.empty()) {
    VLOG(1) << "nao ha feiticos conhecidos para sorteio";
    return;
  }

  const int num_opcoes = indices_validos.size();
  const int sorteio = RolaDado(num_opcoes) - 1;
  VLOG(2) << "rolando d" << num_opcoes << ": resultado -1 = " << sorteio << ", mapeado para " << indices_validos[sorteio];
  il->set_indice_conhecido(indices_validos[sorteio]);
}

// Computa feiticos nao especializados (da classe certa ou de escola nao proibida).
void ComputaFeiticoAleatorioGeral(
    const Tabelas& tabelas, const RestricaoFeitico& rf, int nivel_magia, const EntidadeProto::InfoFeiticosClasse& fc,
    EntidadeProto::InfoLancar* il) {
  ComputaFeiticoAleatorioComum(nivel_magia, fc, il);
  if (il->has_indice_conhecido()) {
    return;
  }
  // Essa linha permitira aleatoriedade para qq nivel.
  nivel_magia = il->nivel_conhecido();
  std::vector<int> indices_validos;
  for (int i = 0; i < fc.feiticos_por_nivel(nivel_magia).conhecidos_size(); ++i) {
    const auto& ic = fc.feiticos_por_nivel(nivel_magia).conhecidos(i);
    const auto& feitico_tabelado = tabelas.Feitico(ic.id());
    if ((rf.dominio && FeiticoClerigo(feitico_tabelado)) ||
        (!FeiticoEscolaProibida(rf, feitico_tabelado))) {
      indices_validos.push_back(i);
    }
  }
  SorteiaIndice(indices_validos, il);
}

// Computa o feitico especializado (de dominio ou da escola especializada).
void ComputaFeiticoAleatorioEspecializado(
    const Tabelas& tabelas, const RestricaoFeitico& rf, int nivel_magia, const EntidadeProto::InfoFeiticosClasse& fc,
    EntidadeProto::InfoLancar* il) {
  ComputaFeiticoAleatorioComum(nivel_magia, fc, il);
  if (il->has_indice_conhecido()) {
    return;
  }
  // Essa linha permitira aleatoriedade para qq nivel.
  nivel_magia = il->nivel_conhecido();
  std::vector<int> indices_validos;
  for (int i = 0; i < fc.feiticos_por_nivel(nivel_magia).conhecidos_size(); ++i) {
    const auto& ic = fc.feiticos_por_nivel(nivel_magia).conhecidos(i);
    const auto& feitico_tabelado = tabelas.Feitico(ic.id());
    if ((rf.dominio && FeiticoDominio(rf, feitico_tabelado)) ||
        (FeiticoEscolaEspecializada(rf, feitico_tabelado))) {
      indices_validos.push_back(i);
    }
  }
  SorteiaIndice(indices_validos, il);
}

// Computa os feiticos aleatorios por slot, respeitando as restricoes, assumindo que elas estao certas.
void ComputaFeiticosAleatoriosParaLancarDoNivel(
    const Tabelas& tabelas, const RestricaoFeitico& rf, int nivel_magia, EntidadeProto::InfoFeiticosClasse* fc) {
  for (auto& pl : *fc->mutable_feiticos_por_nivel(nivel_magia)->mutable_para_lancar()) {
    if (pl.restrito()) {
      ComputaFeiticoAleatorioEspecializado(tabelas, rf, nivel_magia, *fc, &pl);
    } else {
      ComputaFeiticoAleatorioGeral(tabelas, rf, nivel_magia, *fc, &pl);
    }
  }
}

void RecomputaDependenciasMagiasParaLancarPorDia(const Tabelas& tabelas, EntidadeProto* proto) {
  CombinaFeiticosClasse(proto->mutable_feiticos_classes());
  for (auto& ic : *proto->mutable_info_classes()) {
    if (!ic.has_progressao_conjurador() || ic.nivel() <= 0) continue;
    // Encontra a entrada da classe, ou cria se nao houver.
    auto* fc = FeiticosClasse(ic.id(), proto);
    // Le a progressao.
    const int nivel_para_conjuracao = std::min(NivelParaCalculoMagiasPorDia(tabelas, ic.id(), *proto), 20);
    const auto& classe_tabelada = tabelas.Classe(ic.has_id_para_progressao_de_magia() ? ic.id_para_progressao_de_magia() : ic.id());

    // Esse caso deveria dar erro. O cara tem nivel acima do que esta na tabela.
    if (nivel_para_conjuracao >= classe_tabelada.progressao_feitico().para_nivel_size()) continue;

    const std::string& magias_por_dia = classe_tabelada.progressao_feitico().para_nivel(nivel_para_conjuracao).magias_por_dia();

    const bool nao_possui_nivel_zero = classe_tabelada.progressao_feitico().nao_possui_nivel_zero();

    CombinaFeiticosPorNivel(fc->mutable_feiticos_por_nivel());

    // Inclui o nivel 0. Portanto, se o nivel maximo eh 2, deve haver 3 elementos.
    // Na tabela, o nivel zero nao esta presente entao tem que ser compensado aqui.
    Redimensiona(nao_possui_nivel_zero ? magias_por_dia.size() + 1 : magias_por_dia.size(), fc->mutable_feiticos_por_nivel());

    if (nao_possui_nivel_zero) {
      fc->mutable_feiticos_por_nivel(0)->Clear();
    }
    RestricaoFeitico rf;
    const bool classe_possui_dominio = classe_tabelada.possui_dominio();
    const bool classe_possui_especializacao = !fc->especializacao().empty();
    if (classe_possui_dominio) {
      rf.dominio = true;
      for (int i = 0; i < 2; ++i) {
        rf.dominios.push_back(i < fc->dominios_size() ? fc->dominios(i) : "");
      }
    } else if (classe_possui_especializacao) {
      rf.dominio = false;
      const int num_escolas_proibidas = fc->especializacao() == "adivinhacao" ? 1 : 2;
      rf.especializacao = fc->especializacao();
      for (int i = 0; i < num_escolas_proibidas; ++i) {
        rf.escolas_proibidas.push_back(i < fc->escolas_proibidas().size() ? fc->escolas_proibidas(i) : "");
      }
    }
    for (unsigned int indice = 0; indice < magias_por_dia.size(); ++indice) {
      int nivel_magia = nao_possui_nivel_zero ? indice + 1 : indice;
      const bool feitico_extra =
          (classe_possui_dominio && nivel_magia > 0 && nivel_magia <= 9) ||
          (classe_possui_especializacao && nivel_magia >= 0 && nivel_magia <= 9);
      int magias_do_nivel =
          (magias_por_dia[indice] - '0') +
          FeiticosBonusPorAtributoPorNivel(
              nivel_magia,
              BonusAtributo(classe_tabelada.atributo_conjuracao(), *proto)) +
          (feitico_extra ? 1 : 0);
      Redimensiona(magias_do_nivel, fc->mutable_feiticos_por_nivel(nivel_magia)->mutable_para_lancar());
      rf.ha_restricao = feitico_extra;
      ConfiguraSlotsRestritosCorrigindo(
          tabelas, rf, *fc,
          fc->mutable_feiticos_por_nivel(nivel_magia)->mutable_para_lancar());
      ComputaFeiticosAleatoriosParaLancarDoNivel(tabelas, rf, nivel_magia, fc);
    }
  }
}

// Reseta todos os campos computados que tem que ser feito no inicio.
void ResetComputados(EntidadeProto* proto) {
  proto->mutable_dados_defesa()->clear_cura_acelerada();
}

void RecomputaDependenciasRaciais(const Tabelas& tabelas, EntidadeProto* proto) {
  const auto& raca_tabelada = tabelas.Raca(proto->raca());
  if (raca_tabelada.has_tamanho()) {
    AtribuiBonus(raca_tabelada.tamanho(), TB_BASE, "base", proto->mutable_bonus_tamanho());
  }
  if (raca_tabelada.has_tipo()) {
    proto->clear_tipo_dnd();
    proto->add_tipo_dnd(raca_tabelada.tipo());
  }
  if (raca_tabelada.has_sub_tipo()) {
    proto->clear_sub_tipo_dnd();
    proto->add_sub_tipo_dnd(raca_tabelada.sub_tipo());
  }
  auto* atributos = proto->mutable_atributos();
  const auto& atributos_raca = raca_tabelada.bonus_atributos();
  AplicaBonusPenalidadeOuRemove(atributos_raca.forca(), atributos->mutable_forca());
  AplicaBonusPenalidadeOuRemove(atributos_raca.destreza(), atributos->mutable_destreza());
  AplicaBonusPenalidadeOuRemove(atributos_raca.constituicao(), atributos->mutable_constituicao());
  AplicaBonusPenalidadeOuRemove(atributos_raca.inteligencia(), atributos->mutable_inteligencia());
  AplicaBonusPenalidadeOuRemove(atributos_raca.sabedoria(), atributos->mutable_sabedoria());
  AplicaBonusPenalidadeOuRemove(atributos_raca.carisma(), atributos->mutable_carisma());
  AplicaBonusPenalidadeOuRemove(raca_tabelada.dados_defesa().ca(), proto->mutable_dados_defesa()->mutable_ca());
  AplicaBonusPenalidadeOuRemove(raca_tabelada.dados_defesa().bonus_salvacao_veneno(), proto->mutable_dados_defesa()->mutable_bonus_salvacao_veneno());
  AplicaBonusPenalidadeOuRemove(raca_tabelada.dados_defesa().bonus_salvacao_feitico(), proto->mutable_dados_defesa()->mutable_bonus_salvacao_feitico());
  if (!raca_tabelada.dados_defesa().resistencia_elementos().empty()) {
    *proto->mutable_dados_defesa()->mutable_resistencia_elementos() = raca_tabelada.dados_defesa().resistencia_elementos();
  }
  for (const auto& dados_ataque_raca : raca_tabelada.dados_ataque()) {
    if (c_none_of(proto->dados_ataque(), [&raca_tabelada, &dados_ataque_raca](const DadosAtaque& da) {
          return da.id_raca() == raca_tabelada.id() && da.rotulo() == dados_ataque_raca.rotulo();
        })) {
      *proto->add_dados_ataque() = dados_ataque_raca;
    }
  }
  RemoveSe<DadosAtaque>(
      [&raca_tabelada](const DadosAtaque& da) {
        return da.has_id_raca() && da.id_raca() != raca_tabelada.id();
      }, proto->mutable_dados_ataque());

  if (proto->has_raca()) {
    // So aplica os modificadores de pericia se tiver raca, para nao apagar o que estiver tabelado nos monstros.
    for (auto& info_pericia : *proto->mutable_info_pericias()) {
      LimpaBonus(TB_RACIAL, "racial", info_pericia.mutable_bonus());
    }
    for (const auto& info_pericia_raca : raca_tabelada.bonus_pericias()) {
      InfoPericia* pericia = nullptr;
      for (auto& info_pericia : *proto->mutable_info_pericias()) {
        if (info_pericia_raca.id() == info_pericia.id()) {
          pericia = &info_pericia;
          break;
        }
      }
      if (pericia == nullptr) {
        pericia = proto->add_info_pericias();
        pericia->set_id(info_pericia_raca.id());
      }
      AplicaBonusPenalidadeOuRemove(info_pericia_raca.bonus(), pericia->mutable_bonus());
    }
  }
}

void RecomputaDependenciasPericias(const Tabelas& tabelas, EntidadeProto* proto) {
  // Pericias afetadas por talentos.
  std::unordered_map<std::string, std::vector<const TalentoProto*>> talentos_por_pericia;
  for (const auto& talento : tabelas.todas().tabela_talentos().talentos()) {
    for (const auto& bp : talento.bonus_pericias()) {
      talentos_por_pericia[bp.id()].push_back(&talento);
    }
  }

  // Mapa do proto do personagem, porque iremos iterar nas pericias existentes na tabela.
  std::unordered_map<std::string, InfoPericia*> mapa_pericias_proto;
  for (auto& ip : *proto->mutable_info_pericias()) {
    mapa_pericias_proto[ip.id()] = &ip;
  }

  // Cria todas as pericias do personagem.
  for (const auto& pt : tabelas.todas().tabela_pericias().pericias()) {
    // Acha a pericia no personagem se houver para pegar os pontos e calcular a graduacao.
    auto it = mapa_pericias_proto.find(pt.id());
    if (it == mapa_pericias_proto.end()) {
      auto* pericia_proto = proto->add_info_pericias();
      pericia_proto->set_id(pt.id());
      mapa_pericias_proto[pt.id()] = pericia_proto;
    } else {
      it->second->mutable_restricoes_sinergia()->Clear();
    }
  }

  // Iteracao.
  const bool heroismo = PossuiEvento(EFEITO_HEROISMO, *proto);

  for (const auto& pt : tabelas.todas().tabela_pericias().pericias()) {
    // Graduacoes.
    auto* pericia_proto = mapa_pericias_proto[pt.id()];
    int graduacoes = PericiaDeClasse(tabelas, pt.id(), *proto) ? pericia_proto->pontos() : pericia_proto->pontos() / 2;
    AtribuiOuRemoveBonus(graduacoes, TB_BASE, "graduacao", pericia_proto->mutable_bonus());

    // Sinergia.
    for (const auto& s : pt.sinergias()) {
      auto* pericia_alvo = mapa_pericias_proto[s.id()];
      AtribuiOuRemoveBonus(graduacoes >= 5 ? 2 : 0, TB_SINERGIA, StringPrintf("sinergia_%s", pt.id().c_str()), pericia_alvo->mutable_bonus());
      if (!s.restricao().empty()) {
        pericia_alvo->add_restricoes_sinergia(s.restricao());
      }
    }

    // Atributo.
    AtribuiOuRemoveBonus(ModificadorAtributo(pt.atributo(), *proto), TB_ATRIBUTO, "atributo", pericia_proto->mutable_bonus());

    if (pt.id() == "esconderse") {
      // Bonus de tamanho.
      AtribuiOuRemoveBonus(ModificadorTamanhoEsconderse(proto->tamanho()), TB_TAMANHO, "tamanho", pericia_proto->mutable_bonus());
    }

    // Talento.
    auto par_pericia_talentos = talentos_por_pericia.find(pt.id());
    if (par_pericia_talentos != talentos_por_pericia.end()) {
      for (const auto* talento : par_pericia_talentos->second) {
        const int bonus_talento = PossuiTalento(talento->id(), *proto) ? BonusTalento(pt.id(), *talento) : 0;
        AtribuiOuRemoveBonus(bonus_talento, TB_TALENTO, "talento", pericia_proto->mutable_bonus());
      }
    }

    // Heroismo
    AtribuiOuRemoveBonus(heroismo ? 2 : 0, TB_MORAL, "heroismo", pericia_proto->mutable_bonus());
   //LOG(INFO) << "pericia_proto: " << pericia_proto->ShortDebugString();
  }
}

void RecomputaClasseFeiticoAtiva(const Tabelas& tabelas, EntidadeProto* proto) {
  if (Nivel(proto->classe_feitico_ativa(), *proto) > 0) return;
  int nivel = 0;
  std::string id_classe;
  for (const auto& ic : proto->info_classes()) {
    if (ic.nivel_conjurador() > 0 && ic.nivel() > nivel) {
      nivel = ic.nivel();
      id_classe = ic.id();
    }
  }
  if (!id_classe.empty()) {
    proto->set_classe_feitico_ativa(id_classe);
  } else {
    proto->clear_classe_feitico_ativa();
  }
}

std::vector<std::string> FeiticosJaConhecidos(const RepeatedPtrField<EntidadeProto::InfoConhecido>& conhecidos) {
  std::vector<std::string> ret;
  ret.reserve(conhecidos.size());
  for (const auto& c : conhecidos) {
    ret.push_back(c.id());
  }
  return ret;
}

void PreencheFeiticoConhecidoAleatorio(
    const Tabelas& tabelas, const std::string& id_para_magia, int nivel_magia, const RepeatedPtrField<EntidadeProto::InfoConhecido>& conhecidos,
    EntidadeProto::InfoConhecido* ic) {
  if (ic->id() == "auto") {
    ic->set_id(tabelas.FeiticoAleatorio(id_para_magia, nivel_magia, FeiticosJaConhecidos(conhecidos)));
    ic->clear_nome();
  }
  if (!ic->has_nome()) {
    ic->set_nome(tabelas.Feitico(ic->id()).nome());
  }
}

void AdicionaFeiticosDominioSeAusentes(
    const Tabelas& tabelas, int nivel, const std::vector<std::string>& dominios,
    EntidadeProto::InfoFeiticosClasse* fc) {
  std::vector<std::string> ids_feiticos_dominio;
  for (const std::string& dominio : dominios) {
    std::vector<const ArmaProto*> feiticos_dominio = tabelas.Feiticos(dominio, nivel);
    if (feiticos_dominio.empty()) {
      LOG(INFO) << "Faltando feitico de dominio " << dominio << " nivel " << nivel;
      continue;
    }
    if (feiticos_dominio.size() > 1) {
      LOG(ERROR) << "Mais de um feitico de dominio " << dominio << " nivel " << nivel << ", usando primeiro.";
    }
    ids_feiticos_dominio.push_back(feiticos_dominio[0]->id());
  }
  std::vector<bool> added(ids_feiticos_dominio.size());
  for (const auto& c : fc->feiticos_por_nivel(nivel).conhecidos()) {
    for (unsigned int i = 0; i < ids_feiticos_dominio.size(); ++i) {
      if (ids_feiticos_dominio[i] == c.id()) {
        added[i] = true;
      }
    }
  }
  for (unsigned int i = 0; i < added.size(); ++i) {
    if (added[i]) continue;
    auto* c = fc->mutable_feiticos_por_nivel(nivel)->add_conhecidos();
    c->set_id(ids_feiticos_dominio[i]);
  }
}

std::vector<std::string> DominiosClasse(const EntidadeProto::InfoFeiticosClasse& fc) {
  std::vector<std::string> ret;
  for (int i = 0; i < 2; ++i) {
    ret.push_back(i < fc.dominios_size() ? fc.dominios(i) : "");
  }
  return ret;
}

void RecomputaDependenciasMagiasConhecidasParaClasse(
    const Tabelas& tabelas, const InfoClasse& ic, EntidadeProto* proto) {
  if (!ic.has_progressao_conjurador() || ic.nivel() <= 0) return;
  auto* fc = FeiticosClasse(ic.id(), proto);
  // Le a progressao.
  const int nivel = std::min(NivelConjurador(ic.id(), *proto), 20);
  const auto& classe_tabelada = tabelas.Classe(ic.has_id_para_progressao_de_magia() ? ic.id_para_progressao_de_magia() : ic.id());
  // Esse caso deveria dar erro. O cara tem nivel acima do que esta na tabela.
  if (nivel >= classe_tabelada.progressao_feitico().para_nivel_size()) return;
  const std::string& magias_conhecidas = classe_tabelada.progressao_feitico().para_nivel(nivel).conhecidos();
  if (ic.possui_dominio()) {
    std::vector<std::string> dominios = DominiosClasse(*fc);
    // Adiciona os feiticos de dominio se nao estiverem presentes.
    // Seta os feiticos que sao auto.
    for (int nivel_magia = 0; nivel_magia < fc->feiticos_por_nivel().size(); ++nivel_magia) {
      AdicionaFeiticosDominioSeAusentes(tabelas, nivel_magia, dominios, fc);
    }
  }
  // Classe nao tem magias conhecidas.
  if (magias_conhecidas.empty()) {
    // Seta os feiticos que sao auto.
    for (int nivel_magia = 0; nivel_magia < fc->feiticos_por_nivel().size(); ++nivel_magia) {
      auto* fn = fc->mutable_feiticos_por_nivel(nivel_magia);
      for (auto& conhecido : *fn->mutable_conhecidos()) {
        PreencheFeiticoConhecidoAleatorio(
            tabelas, classe_tabelada.has_id_para_magia() ? classe_tabelada.id_para_magia() : classe_tabelada.id(), nivel_magia, fn->conhecidos(), &conhecido);
      }
    }
    return;
  }
  const bool nao_possui_nivel_zero = classe_tabelada.progressao_feitico().nao_possui_nivel_zero();

  // Inclui o nivel 0. Portanto, se o nivel maximo eh 2, deve haver 3 elementos.
  // Como na tabela nao ha nivel zero, tem que compensar aqui.
  Redimensiona(nao_possui_nivel_zero ? magias_conhecidas.size() + 1 : magias_conhecidas.size(), fc->mutable_feiticos_por_nivel());

  if (nao_possui_nivel_zero) {
    Redimensiona(0, fc->mutable_feiticos_por_nivel(0)->mutable_conhecidos());
  }
  for (unsigned int indice = 0; indice < magias_conhecidas.size(); ++indice) {
    const int magias_conhecidas_do_nivel = magias_conhecidas[indice] - '0';
    const int nivel_magia = indice + (classe_tabelada.progressao_feitico().nao_possui_nivel_zero() ? 1 : 0);
    Redimensiona(magias_conhecidas_do_nivel, fc->mutable_feiticos_por_nivel(nivel_magia)->mutable_conhecidos());
    auto* fn = fc->mutable_feiticos_por_nivel(nivel_magia);
    for (auto& conhecido : *fn->mutable_conhecidos()) {
      PreencheFeiticoConhecidoAleatorio(
          tabelas, classe_tabelada.has_id_para_magia() ? classe_tabelada.id_para_magia() : classe_tabelada.id(), nivel_magia, fn->conhecidos(), &conhecido);
    }
  }
  // TODO invalida feiticos que nao puderem ser usados.
}

void RecomputaDependenciasMagiasConhecidas(const Tabelas& tabelas, EntidadeProto* proto) {
  for (auto& ic : *proto->mutable_info_classes()) {
    RecomputaDependenciasMagiasConhecidasParaClasse(tabelas, ic, proto);
  }
}

void RecomputaDependenciasPontosVidaTemporarios(EntidadeProto* proto) {
  // TODO fazer isso igual ao de resistencia a magia, usando TB_BASE.
  auto* bpv = proto->mutable_pontos_vida_temporarios_por_fonte();
  // Remove todos que nao sao sem nome.
  RemoveSe<BonusIndividual>([] (const BonusIndividual& bi) {
    return bi.tipo() != TB_SEM_NOME;
  }, bpv->mutable_bonus_individual());
  if (!bpv->bonus_individual().empty()) {
    // Mantem apenas o maximo de cada um.
    auto* bi = BonusIndividualSePresente(TB_SEM_NOME, bpv);
    if (bi != nullptr) {
      struct Info {
        int max = -1;
        const BonusIndividual::PorOrigem* ptr = nullptr;
      };
      std::unordered_map<std::string, Info> infos;
      for (int i = 0; i < bi->por_origem().size(); ++i) {
        const auto& po = bi->por_origem(i);
        auto& info = infos[po.origem()];
        if (info.ptr == nullptr || po.valor() > info.max) {
          info.max = po.valor();
          info.ptr = &po;
        }
      }
      // Remove os que nao sao maximo.
      RemoveSe<BonusIndividual::PorOrigem>([&infos](const BonusIndividual::PorOrigem& po) {
        return po.valor() == 0 || &po != infos[po.origem()].ptr;
      }, bi->mutable_por_origem());
    }
  }
  // So pode haver um de cada tipo. Todos tem TB_SEM_NOME, difere eh a origem.
  // Apenas o maior de cada origem eh mantido.
  proto->set_pontos_vida_temporarios(BonusTotal(*bpv));
}

void RecomputaDependenciasPontosVida(EntidadeProto* proto) {
  const int max_pontos_vida = proto->max_pontos_vida() - proto->niveis_negativos() * 5;
  if (proto->pontos_vida() > max_pontos_vida) {
    proto->set_pontos_vida(max_pontos_vida);
  }
}

void RecomputaDependenciasResistenciaMagia(EntidadeProto* proto) {
  auto* brm = proto->mutable_dados_defesa()->mutable_resistencia_magia_variavel();
  AtribuiBonus(proto->dados_defesa().resistencia_magia_racial(), TB_BASE, "racial", brm);
  // Remove todos que nao sao BASE.
  RemoveSe<BonusIndividual>([] (const BonusIndividual& bi) {
    return bi.tipo() != TB_BASE;
  }, brm->mutable_bonus_individual());
  // Aqui so tem BASE, entao pode pegar o bonus total.
  int total = BonusTotal(*brm);
  if (total > 0) {
    proto->mutable_dados_defesa()->set_resistencia_magia(BonusTotal(*brm));
  } else {
    proto->mutable_dados_defesa()->clear_resistencia_magia();
  }
}

int OutrosModificadoresNivelConjuracao(const EntidadeProto& proto) {
  return PossuiEvento(EFEITO_DRENAR_FORCA_VITAL, proto) ? 1 : 0;
}

void RecomputaNivelConjuracao(const Tabelas& tabelas, const EntidadeProto& proto, InfoClasse* ic) {
  if (ic->has_nivel_conjurador_nativo()) {
    ic->set_nivel_conjurador(ic->nivel_conjurador_nativo());
    return;
  }
  int niveis_da_classe = NivelParaCalculoMagiasPorDia(tabelas, ic->id(), proto);
  int valor = -proto.niveis_negativos() + OutrosModificadoresNivelConjuracao(proto);
  switch (tabelas.Classe(ic->id()).progressao_conjurador()) {
    case PCONJ_MEIO_MIN_4:
      valor += niveis_da_classe < 4 ? 0 : niveis_da_classe / 2;
      break;
    case PCONJ_UM:
      valor += niveis_da_classe;
      break;
    case PCONJ_ZERO:
    default: ;
  }
  if (valor <= 0) {
    ic->clear_nivel_conjurador();
  } else {
    ic->set_nivel_conjurador(valor);
  }
}

// Recomputa os modificadores de conjuracao.
void RecomputaDependenciasClasses(const Tabelas& tabelas, EntidadeProto* proto) {
  int salvacao_fortitude = 0;
  int salvacao_reflexo = 0;
  int salvacao_vontade = 0;
  // Para evitar recomputar quando nao tiver base.
  bool recomputa_base = false;
  proto->mutable_dados_defesa()->clear_reducao_dano_barbaro();
  std::set<int, std::greater<int>> a_remover;
  for (int i = 0; i < proto->info_classes_size(); ++i) {
    const auto& ic = proto->info_classes(i);
    if (ic.has_combinar_com()) {
      a_remover.insert(i);
      if (ic.combinar_com() >= 0 && ic.combinar_com() < proto->info_classes_size()) {
        proto->mutable_info_classes(ic.combinar_com())->MergeFrom(ic);
        proto->mutable_info_classes(ic.combinar_com())->clear_combinar_com();
      } else {
        LOG(ERROR) << "Combinar com invalido: " << ic.combinar_com() << ", tamanho: " << proto->info_classes_size();
      }
    }
  }
  for (int i : a_remover) {
    proto->mutable_info_classes()->DeleteSubrange(i, 1);
  }
  for (auto& ic : *proto->mutable_info_classes()) {
    {
      const auto& classe_tabelada = tabelas.Classe(ic.id());
      if (classe_tabelada.has_nome()) {
        ic.clear_salvacoes_fortes();
        ic.clear_habilidades_por_nivel();
        ic.clear_pericias();
        ic.clear_progressao_feitico();
        ic.MergeFrom(classe_tabelada);
      }
    }
    if (ic.has_atributo_conjuracao() || ic.has_id_para_progressao_de_magia()) {
      const auto& classe_tabelada_conjuracao =
          tabelas.Classe(ic.has_id_para_progressao_de_magia() ? ic.id_para_progressao_de_magia() : ic.id());
      if (!ic.has_atributo_conjuracao()) {
        ic.set_atributo_conjuracao(classe_tabelada_conjuracao.atributo_conjuracao());
      }
      ic.set_modificador_atributo_conjuracao(ModificadorAtributo(ic.atributo_conjuracao(), *proto));
      RecomputaNivelConjuracao(tabelas, *proto, &ic);
    }
    if (ic.has_progressao_bba()) {
      switch (ic.progressao_bba()) {
        case PBBA_ZERO: ic.set_bba(0); break;
        case PBBA_MEIO: ic.set_bba(ic.nivel() / 2); break;
        case PBBA_TRES_QUARTOS: ic.set_bba((ic.nivel() * 3) / 4); break;
        case PBBA_UM: ic.set_bba(ic.nivel()); break;
      }
    }

    if (ic.salvacoes_fortes_size() > 0) {
      recomputa_base = true;
      salvacao_fortitude += CalculaBaseSalvacao(ClassePossuiSalvacaoForte(TS_FORTITUDE, ic), ic.nivel());
      salvacao_reflexo += CalculaBaseSalvacao(ClassePossuiSalvacaoForte(TS_REFLEXO, ic), ic.nivel());
      salvacao_vontade += CalculaBaseSalvacao(ClassePossuiSalvacaoForte(TS_VONTADE, ic), ic.nivel());
    }
    if (ic.id() == "barbaro") {
      proto->mutable_dados_defesa()->set_reducao_dano_barbaro(ReducaoDanoBarbaro(ic.nivel()));
    }
  }
  if (recomputa_base) {
    AtribuiBonus(salvacao_fortitude, TB_BASE, "base", BonusSalvacao(TS_FORTITUDE, proto));
    AtribuiBonus(salvacao_reflexo, TB_BASE, "base", BonusSalvacao(TS_REFLEXO, proto));
    AtribuiBonus(salvacao_vontade, TB_BASE, "base", BonusSalvacao(TS_VONTADE, proto));
  } else {
    RemoveBonus(TB_BASE, "base", BonusSalvacao(TS_FORTITUDE, proto));
    RemoveBonus(TB_BASE, "base", BonusSalvacao(TS_REFLEXO, proto));
    RemoveBonus(TB_BASE, "base", BonusSalvacao(TS_VONTADE, proto));
  }
}

void RecomputaDependenciasTalentos(const Tabelas& tabelas, EntidadeProto* proto) {
  // Limitar talentos gerais por nivel de personagem.
  const int nivel = Nivel(*proto);
  const int numero = (nivel / 3) + 1;
  if (proto->info_talentos().gerais().size() > numero) {
    LOG(WARNING) << "Um dia irei capar talentos da entidade "
      << RotuloEntidade(*proto) << ", gerais: " << proto->info_talentos().gerais().size() << ", permitido: " << numero;
  }
  //while (proto->info_talentos().gerais().size() > numero) {
  //  proto->mutable_info_talentos()->mutable_gerais()->RemoveLast();
  //}
  proto->mutable_info_talentos()->clear_automaticos();
  for (const auto& ic : proto->info_classes()) {
    const auto& classe_tabelada = tabelas.Classe(ic.id());
    for (const std::string& id_talento : classe_tabelada.talentos_automaticos()) {
      proto->mutable_info_talentos()->add_automaticos()->set_id(id_talento);
    }
  }
}

void RecomputaDependenciasCA(const Tabelas& tabelas, EntidadeProto* proto_retornado) {
  auto* dd = proto_retornado->mutable_dados_defesa();
  int bonus_maximo = std::numeric_limits<int>::max();
  if (dd->has_id_armadura()) {
    bonus_maximo = std::min(tabelas.Armadura(dd->id_armadura()).max_bonus_destreza(), bonus_maximo);
  }
  if (dd->has_id_escudo()) {
    bonus_maximo = std::min(tabelas.Escudo(dd->id_escudo()).max_bonus_destreza(), bonus_maximo);
  }
  const int modificador_destreza = std::min(ModificadorAtributo(proto_retornado->atributos().destreza()), bonus_maximo);
  AtribuiBonus(modificador_destreza, TB_ATRIBUTO, "destreza", dd->mutable_ca());
  const int modificador_tamanho = ModificadorTamanho(proto_retornado->tamanho());
  ent::AtribuiBonus(10, TB_BASE, "base",  dd->mutable_ca());
  AtribuiOuRemoveBonus(modificador_tamanho, TB_TAMANHO, "tamanho", dd->mutable_ca());
  AtribuiOuRemoveBonus(dd->has_id_armadura() ? tabelas.Armadura(dd->id_armadura()).bonus() : 0, TB_ARMADURA, "armadura", dd->mutable_ca());
  AtribuiOuRemoveBonus(dd->has_bonus_magico_armadura()
      ? dd->bonus_magico_armadura() : 0, TB_ARMADURA_MELHORIA, "armadura_melhoria", dd->mutable_ca());
  if (dd->bonus_magico_armadura() > 0) {
    dd->set_armadura_obra_prima(true);
  }
  AtribuiOuRemoveBonus(dd->has_id_escudo() ? tabelas.Escudo(dd->id_escudo()).bonus() : 0, TB_ESCUDO, "escudo", dd->mutable_ca());
  AtribuiOuRemoveBonus(dd->has_bonus_magico_escudo()
      ? dd->bonus_magico_escudo() : 0, TB_ESCUDO_MELHORIA, "escudo_melhoria", dd->mutable_ca());
  if (dd->bonus_magico_escudo() > 0) {
    dd->set_escudo_obra_prima(true);
  }

  for (const auto& talento : tabelas.todas().tabela_talentos().talentos()) {
    if (talento.has_bonus_ca()) {
      if (PossuiTalento(talento.id(), *proto_retornado)) {
        CombinaBonus(talento.bonus_ca(), proto_retornado->mutable_dados_defesa()->mutable_ca());
      } else {
        LimpaBonus(talento.bonus_ca(), proto_retornado->mutable_dados_defesa()->mutable_ca());
      }
    }
  }
  const int nivel_monge = Nivel("monge", *proto_retornado);
  if (nivel_monge > 0 && dd->id_armadura().empty() && dd->id_escudo().empty()) {
    int bonus_monge = nivel_monge / 5;
    AtribuiOuRemoveBonus(bonus_monge, TB_SEM_NOME, "monge", dd->mutable_ca());
    const int modificador_sabedoria = std::max(0, ModificadorAtributo(proto_retornado->atributos().sabedoria()));
    AtribuiOuRemoveBonus(modificador_sabedoria, TB_SEM_NOME, "monge_sabedoria", dd->mutable_ca());
  }
}

void RecomputaDependenciasSalvacoes(
    int modificador_constituicao, int modificador_destreza, int modificador_sabedoria, int modificador_carisma, const Tabelas& tabelas, EntidadeProto* proto_retornado) {
  auto* dd = proto_retornado->mutable_dados_defesa();

  // Testes de resistencia.
  AtribuiBonus(modificador_constituicao, TB_ATRIBUTO, "constituicao", dd->mutable_salvacao_fortitude());
  AtribuiBonus(modificador_destreza, TB_ATRIBUTO, "destreza", dd->mutable_salvacao_reflexo());
  AtribuiBonus(modificador_sabedoria, TB_ATRIBUTO, "sabedoria", dd->mutable_salvacao_vontade());

  // Percorre todos os talentos que dao bonus em salvacao.
  for (const auto& talento : tabelas.todas().tabela_talentos().talentos()) {
    for (const auto& bonus_salvacao : talento.bonus_salvacao()) {
      if (PossuiTalento(talento.id(), *proto_retornado)) {
        CombinaBonus(bonus_salvacao.bonus(), BonusSalvacao(bonus_salvacao.tipo(), proto_retornado));
      } else {
        LimpaBonus(bonus_salvacao.bonus(), BonusSalvacao(bonus_salvacao.tipo(), proto_retornado));
      }
    }
  }

  bool graca_divina = PossuiHabilidadeEspecial("graca_divina", *proto_retornado);
  if (graca_divina && modificador_carisma > 0) {
    AtribuiBonus(modificador_carisma, TB_SEM_NOME, "graca_divina", dd->mutable_salvacao_fortitude());
    AtribuiBonus(modificador_carisma, TB_SEM_NOME, "graca_divina", dd->mutable_salvacao_reflexo());
    AtribuiBonus(modificador_carisma, TB_SEM_NOME, "graca_divina", dd->mutable_salvacao_vontade());
  } else {
    LimpaBonus(TB_SEM_NOME, "graca_divina", dd->mutable_salvacao_fortitude());
    LimpaBonus(TB_SEM_NOME, "graca_divina", dd->mutable_salvacao_reflexo());
    LimpaBonus(TB_SEM_NOME, "graca_divina", dd->mutable_salvacao_vontade());
  }

  const int mod_nivel_negativo = -proto_retornado->niveis_negativos();
  AtribuiBonus(mod_nivel_negativo, TB_SEM_NOME, "niveis_negativos", dd->mutable_salvacao_fortitude());
  AtribuiBonus(mod_nivel_negativo, TB_SEM_NOME, "niveis_negativos", dd->mutable_salvacao_reflexo());
  AtribuiBonus(mod_nivel_negativo, TB_SEM_NOME, "niveis_negativos", dd->mutable_salvacao_vontade());
}

void RecomputaDependenciasEvasao(const Tabelas& tabelas, EntidadeProto* proto_retornado) {
  auto* dd = proto_retornado->mutable_dados_defesa();
  dd->clear_evasao();
  if (PossuiHabilidadeEspecial("evasao_aprimorada", *proto_retornado) || dd->evasao_estatica() == TE_EVASAO_APRIMORADA) {
    dd->set_evasao(TE_EVASAO_APRIMORADA);
  } else if (PossuiHabilidadeEspecial("evasao", *proto_retornado) || dd->evasao_estatica() == TE_EVASAO) {
    dd->set_evasao(TE_EVASAO);
  } else {
    dd->clear_evasao();
  }
}

void RecomputaDependenciaTamanho(EntidadeProto* proto) {
  // Aplica efeito cria isso, entao melhor ver se tem algum bonus individual.
  if (!PossuiBonus(TB_BASE, proto->bonus_tamanho())) {
    AtribuiBonus(proto->tamanho(), TB_BASE, "base", proto->mutable_bonus_tamanho());
  }
  int total = BonusTotal(proto->bonus_tamanho());
  if (total > TM_COLOSSAL) { total = TM_COLOSSAL; }
  else if (total < TM_MINUSCULO) { total = TM_MINUSCULO; }
  proto->set_tamanho(TamanhoEntidade(total));
}

void RecomputaDependenciasIniciativa(int modificador_destreza, EntidadeProto* proto) {
  AtribuiBonus(modificador_destreza, TB_ATRIBUTO, "destreza", proto->mutable_bonus_iniciativa());
  AtribuiOuRemoveBonus(
      PossuiTalento("iniciativa_aprimorada", *proto) ? 4 : 0, TB_TALENTO, "talento", proto->mutable_bonus_iniciativa());
  proto->set_modificador_iniciativa(BonusTotal(proto->bonus_iniciativa()));
}

void RecomputaDependenciasTendencia(EntidadeProto* proto) {
  if (!proto->tendencia().has_eixo_bem_mal() || !proto->tendencia().has_eixo_ordem_caos()) {
    // se nao tem dinamica, computa.
    float bem_mal = 0.5f;
    float ordem_caos = 0.5f;
    switch (proto->tendencia().simples()) {
      case TD_LEAL_BOM:    bem_mal = ordem_caos = 1.0f;       break;
      case TD_LEAL_NEUTRO: bem_mal = 0.5f; ordem_caos = 1.0f; break;
      case TD_LEAL_MAU:    bem_mal = 0.0f; ordem_caos = 1.0f; break;
      case TD_NEUTRO_BOM:  bem_mal = 1.0f; ordem_caos = 0.5f; break;
      case TD_NEUTRO:      bem_mal = ordem_caos = 0.5f;       break;
      case TD_NEUTRO_MAU:  bem_mal = 0.0f; ordem_caos = 0.5f; break;
      case TD_CAOTICO_BOM:    bem_mal = 1.0f; ordem_caos = 0.0f; break;
      case TD_CAOTICO_NEUTRO: bem_mal = 0.5f; ordem_caos = 0.0f; break;
      case TD_CAOTICO_MAU:    bem_mal = 0.0f; ordem_caos = 0.0f; break;
    }
    // So pra escrever o valor se for o padrao do proto.
    proto->mutable_tendencia()->set_simples(proto->tendencia().simples());
    proto->mutable_tendencia()->set_eixo_bem_mal(bem_mal);
    proto->mutable_tendencia()->set_eixo_ordem_caos(ordem_caos);
  } else {
    // Se tem tendencia dinamica, recalcula simplificada.
    const float bem_mal = proto->tendencia().eixo_bem_mal();
    const float ordem_caos = proto->tendencia().eixo_ordem_caos();
    const float kLimiteCima = 0.66;
    const float kLimiteBaixo = 0.33;
    if (bem_mal >= kLimiteCima) {
      if (ordem_caos >= kLimiteCima) {
        proto->mutable_tendencia()->set_simples(TD_LEAL_BOM);
      } else if (ordem_caos <= kLimiteBaixo) {
        proto->mutable_tendencia()->set_simples(TD_CAOTICO_BOM);
      } else {
        proto->mutable_tendencia()->set_simples(TD_NEUTRO_BOM);
      }
    } else if (bem_mal <= kLimiteBaixo) {
      // Mau.
      if (ordem_caos >= kLimiteCima) {
        proto->mutable_tendencia()->set_simples(TD_LEAL_MAU);
      } else if (ordem_caos <= kLimiteBaixo) {
        proto->mutable_tendencia()->set_simples(TD_CAOTICO_MAU);
      } else {
        proto->mutable_tendencia()->set_simples(TD_NEUTRO_MAU);
      }
    } else {
      // Nem bom nem mau.
      if (ordem_caos >= kLimiteCima) {
        proto->mutable_tendencia()->set_simples(TD_LEAL_NEUTRO);
      } else if (ordem_caos <= kLimiteBaixo) {
        proto->mutable_tendencia()->set_simples(TD_CAOTICO_NEUTRO);
      } else {
        proto->mutable_tendencia()->set_simples(TD_NEUTRO);
      }
    }
  }
}


void RecomputaDependenciasItensMagicos(const Tabelas& tabelas, EntidadeProto* proto) {
  // TODO So pra garantir pros itens criados antes da tabela ter tipo. Depois rola de remover.
  for (auto& item : *proto->mutable_tesouro()->mutable_aneis()) item.set_tipo(TIPO_ANEL);
  for (auto& item : *proto->mutable_tesouro()->mutable_mantos()) item.set_tipo(TIPO_MANTO);
  for (auto& item : *proto->mutable_tesouro()->mutable_luvas()) item.set_tipo(TIPO_LUVAS);
  for (auto& item : *proto->mutable_tesouro()->mutable_bracadeiras()) item.set_tipo(TIPO_BRACADEIRAS);
  for (auto& item : *proto->mutable_tesouro()->mutable_pocoes()) item.set_tipo(TIPO_POCAO);
  for (auto& item : *proto->mutable_tesouro()->mutable_amuletos()) item.set_tipo(TIPO_AMULETO);
  for (auto& item : *proto->mutable_tesouro()->mutable_botas()) item.set_tipo(TIPO_BOTAS);
  for (auto& item : *proto->mutable_tesouro()->mutable_chapeus()) item.set_tipo(TIPO_CHAPEU);

  // Adiciona efeitos nao existentes e expira os que ja foram.
  std::vector<ItemMagicoProto*> itens;
  std::vector<ItemMagicoProto*> itens_a_expirar;
  for (auto& item : *proto->mutable_tesouro()->mutable_aneis()) {
    if (item.em_uso() && item.ids_efeitos().empty()) itens.push_back(&item);
    else if (!item.em_uso() && !item.ids_efeitos().empty()) itens_a_expirar.push_back(&item);
  }
  for (auto& item : *proto->mutable_tesouro()->mutable_mantos()) {
    if (item.em_uso() && item.ids_efeitos().empty()) itens.push_back(&item);
    else if (!item.em_uso() && !item.ids_efeitos().empty()) itens_a_expirar.push_back(&item);
  }
  for (auto& item : *proto->mutable_tesouro()->mutable_luvas()) {
    if (item.em_uso() && item.ids_efeitos().empty()) itens.push_back(&item);
    else if (!item.em_uso() && !item.ids_efeitos().empty()) itens_a_expirar.push_back(&item);
  }
  for (auto& item : *proto->mutable_tesouro()->mutable_bracadeiras()) {
    if (item.em_uso() && item.ids_efeitos().empty()) itens.push_back(&item);
    else if (!item.em_uso() && !item.ids_efeitos().empty()) itens_a_expirar.push_back(&item);
  }
  for (auto& item : *proto->mutable_tesouro()->mutable_amuletos()) {
    if (item.em_uso() && item.ids_efeitos().empty()) itens.push_back(&item);
    else if (!item.em_uso() && !item.ids_efeitos().empty()) itens_a_expirar.push_back(&item);
  }
  for (auto& item : *proto->mutable_tesouro()->mutable_botas()) {
    if (item.em_uso() && item.ids_efeitos().empty()) itens.push_back(&item);
    else if (!item.em_uso() && !item.ids_efeitos().empty()) itens_a_expirar.push_back(&item);
  }
  for (auto& item : *proto->mutable_tesouro()->mutable_chapeus()) {
    if (item.em_uso() && item.ids_efeitos().empty()) itens.push_back(&item);
    else if (!item.em_uso() && !item.ids_efeitos().empty()) itens_a_expirar.push_back(&item);
  }

  for (auto* item : itens_a_expirar) {
    ExpiraEventosItemMagico(item, proto);
  }
  std::vector<int> ids_unicos(IdsUnicosProto(*proto));
  for (auto* item : itens) {
    AdicionaEventosItemMagicoContinuo(tabelas, item, &ids_unicos, proto);
  }
}

std::unordered_set<unsigned int> IdsItensComEfeitos(const EntidadeProto& proto) {
  std::unordered_set<unsigned int> ids;
  std::vector<const ItemMagicoProto*> itens = TodosItensExcetoPocoes(proto);
  for (const auto* item : itens) {
    //LOG(INFO) << "-> item: " << item->ShortDebugString();
    if (!item->em_uso()) {
      //LOG(INFO) << "   nao adicionando nenhum id";
      continue;
    }
    if (item->ids_efeitos().empty()) {
      //LOG(INFO) << "   nao adicionando nenhum id, vazio";
    }
    for (unsigned int id_efeito : item->ids_efeitos()) {
      ids.insert(id_efeito);
      //LOG(INFO) << "   adicionando id efeito " << id_efeito;
    }
  }
  return ids;
}

bool PossuiModelo(const Tabelas& tabelas, TipoEfeitoModelo id_efeito_modelo, const EntidadeProto& proto) {
  for (const auto& modelo : proto.modelos()) {
    if (modelo.id_efeito() != id_efeito_modelo) continue;
    return !ModeloDesligavel(tabelas, modelo) || modelo.ativo();
  }
  return false;
}

// Retorna true se o item que criou o evento nao existe mais.
bool EventoOrfao(const Tabelas& tabelas, const EntidadeProto::Evento& evento, const std::unordered_set<unsigned int>& ids_itens, const EntidadeProto& proto) {
  if (evento.has_requer_modelo_ativo() && !PossuiModelo(tabelas, evento.requer_modelo_ativo(), proto)) {
    return true;
  }
  bool ret = evento.requer_pai() && c_none(ids_itens, evento.id_unico());
  //LOG(INFO) << "EventoOrfao: " << ret << ", evento: " << evento.ShortDebugString();
  return ret;
}

void RecomputaDependenciasEfeitos(const Tabelas& tabelas, EntidadeProto* proto, Entidade* entidade) {
  //LOG(INFO) << "-----------------------------";
  std::set<int, std::greater<int>> eventos_a_remover;
  const std::unordered_set<unsigned int> ids_itens = IdsItensComEfeitos(*proto);
  int i = 0;
  // Verifica eventos acabados.
  const int total_constituicao_antes = BonusTotal(proto->atributos().constituicao());
  for (const auto& evento : proto->evento()) {
    //LOG(INFO) << "evento: " << evento.ShortDebugString();
    const bool encerrado = evento.rodadas() < 0;
    const bool orfao = EventoOrfao(tabelas, evento, ids_itens, *proto);
    if (encerrado || orfao) {
      const auto& efeito = tabelas.Efeito(evento.id_efeito());
      // Usa id_efeito do evento porque efeito pode nao ser tabelado.
      VLOG(1) << "removendo efeito: " << TipoEfeito_Name(evento.id_efeito()) << " (" << evento.id_efeito() << "), " << (encerrado ? "encerrado" : "orfão");
      if (efeito.has_consequencia_fim()) {
        AplicaFimEfeito(evento, PreencheConsequencia(evento.origem(), evento.complementos(), efeito.consequencia_fim()), proto, entidade);
      } else {
        AplicaFimEfeito(evento, PreencheConsequenciaFim(evento.origem(), efeito.consequencia()), proto, entidade);
      }
      eventos_a_remover.insert(i);
    }
    ++i;
  }
  // Modelos desativados.
  for (auto& modelo : *proto->mutable_modelos()) {
    if (!ModeloDesligavel(tabelas, modelo) || modelo.ativo()) continue;
    const auto& efeito_modelo = tabelas.EfeitoModelo(modelo.id_efeito());
    VLOG(1) << "removendo efeito de modelo: " << TipoEfeitoModelo_Name(efeito_modelo.id());
    AplicaEfeitoComum(PreencheConsequenciaFim(efeito_modelo.nome(), efeito_modelo.consequencia()), proto);
  }

  for (int i : eventos_a_remover) {
    proto->mutable_evento()->DeleteSubrange(i, 1);
  }
  // Computa os eventos ainda ativos. Os que nao se acumulam sao ignorados.
  std::unordered_set<int> efeitos_computados;
  VLOG(1) << "Loop";
  for (auto& evento : *proto->mutable_evento()) {
    const bool computado = efeitos_computados.find(evento.id_efeito()) != efeitos_computados.end();
    efeitos_computados.insert(evento.id_efeito());
    const auto& efeito = tabelas.Efeito(evento.id_efeito());
    if (computado && efeito.nao_cumulativo()) {
      // Usar o id do efeito porque ele pode nao ser tabelado.
      VLOG(1) << "ignorando efeito: " << TipoEfeito_Name(evento.id_efeito()) << " (" << evento.id_efeito() << ") ";
      continue;
    }
    VLOG(1) << "aplicando efeito: " << TipoEfeito_Name(evento.id_efeito()) << " (" << evento.id_efeito() << ") ";
    if (AplicaEfeito(&evento, PreencheConsequencia(evento.origem(), evento.complementos(), efeito.consequencia()), proto, entidade)) {
      evento.set_processado(true);
    }
  }
  // Efeito de modelos.
  for (auto& modelo : *proto->mutable_modelos()) {
    if (ModeloDesligavel(tabelas, modelo) && !modelo.ativo()) continue;
    const auto& efeito_modelo = tabelas.EfeitoModelo(modelo.id_efeito());
    VLOG(1) << "aplicando efeito de modelo: " << TipoEfeitoModelo_Name(efeito_modelo.id());
    AplicaEfeitoComum(PreencheConsequencia(efeito_modelo.nome(), modelo.complementos(), efeito_modelo.consequencia()), proto);
  }

  const int total_constituicao_depois = BonusTotal(proto->atributos().constituicao());
  RecomputaAlteracaoConstituicao(total_constituicao_antes, total_constituicao_depois, proto);
}

void RecomputaDependenciasNiveisNegativos(const Tabelas& tabelas, EntidadeProto* proto) {
  proto->set_niveis_negativos(BonusTotal(proto->niveis_negativos_dinamicos()));
}

void RecomputaDependenciasDestrezaLegado(const Tabelas& tabelas, EntidadeProto* proto) {
  // Legado, apenas para limpar o que foi feito errado. O bonus maximo de destreza afeta apenas a CA.
  AtribuiBonus(0, TB_ARMADURA, "armadura_escudo", proto->mutable_atributos()->mutable_destreza());
}

int NivelFeiticoPergaminho(const Tabelas& tabelas, TipoMagia tipo_pergaminho, const ArmaProto& feitico) {
  std::vector<std::string> classes;
  if (tipo_pergaminho == TM_DIVINA) {
    // divino: tenta clerigo, druida, paladino, ranger
    classes.push_back("clerigo");
    classes.push_back("druida");
    classes.push_back("paladino");
    classes.push_back("ranger");
  } else if (tipo_pergaminho == TM_ARCANA) {
    // arcano: tenta mago, bardo.
    classes.push_back("mago");
    classes.push_back("bardo");
  }
  for (const auto& classe : classes) {
    int nivel = NivelFeitico(tabelas, classe, feitico);
    if (nivel >= 0) return nivel;
  }
  LOG(ERROR) << "Não achei nivel certo para pergaminho tipo: " << tipo_pergaminho;
  return 0;
}

// Aplica os dados de acoes que forem colocados soltos no ataque.
void AcaoParaDadosAtaque(const Tabelas& tabelas, const ArmaProto& feitico, const EntidadeProto& proto, DadosAtaque* da) {
  {
    // O que for tabelado comum do tipo do ataque.
    const auto& acao_tabelada = tabelas.Acao(da->tipo_ataque());
    da->mutable_acao()->MergeFrom(acao_tabelada);
  }

  // O que for especifico deste ataque.
  if (da->has_acao_fixa()) {
    if (!da->acao_fixa().id().empty()) {
      const auto& acao_tabelada = tabelas.Acao(da->acao_fixa().id());
      da->mutable_acao()->MergeFrom(acao_tabelada);
    }
    da->mutable_acao()->MergeFrom(da->acao_fixa());
  }

  CombinaEfeitos(da->mutable_acao());

  // Aqui temos a acao finalizada. Agora passa tudo pro da.
  const AcaoProto& acao = da->acao();
  if (acao.has_ignora_reflexos()) {
    da->set_ignora_reflexos(acao.ignora_reflexos());
  }
  if (acao.ignora_municao()) {
    da->clear_municao();
  }
  if (acao.has_ataque_corpo_a_corpo()) {
    da->set_ataque_corpo_a_corpo(acao.ataque_corpo_a_corpo());
  }
  if (acao.has_ataque_agarrar()) {
    da->set_ataque_agarrar(acao.ataque_agarrar());
  }
  if (acao.has_ataque_toque()) {
    da->set_ataque_toque(acao.ataque_toque());
  }
  if (acao.has_ataque_distancia()) {
    da->set_ataque_distancia(acao.ataque_distancia());
  }
  if (acao.has_resultado_ao_salvar()) {
    da->set_resultado_ao_salvar(acao.resultado_ao_salvar());
  }
  if (acao.has_tipo_pergaminho()) {
    da->set_tipo_pergaminho(acao.tipo_pergaminho());
  }
  if (acao.has_ataque_arremesso()) {
    da->set_ataque_arremesso(true);
  }
  if (acao.has_tipo_salvacao()) {
    da->set_tipo_salvacao(acao.tipo_salvacao());
  }

  if (da->acao().has_dificuldade_salvacao_base() || da->acao().dificuldade_salvacao_por_nivel()) {
    // Essa parte eh tricky. Algumas coisas tem que ser a classe mesmo: tipo atributo (feiticeiro usa carisma).
    // Outras tem que ser a classe de feitico, por exemplo, nivel de coluna de chama para mago.
    // A chamada InfoClasseParaFeitico busca a classe do personagem (feiticeiro)
    // enquanto TipoAtaqueParaClasse busca a classe para feitico (mago).
    const auto& ic = InfoClasseParaFeitico(tabelas, da->tipo_ataque(), proto);
    const auto& fc = FeiticosClasse(ic.id(), proto);
    int mod_especializacao = !fc.especializacao().empty() && feitico.escola() == fc.especializacao() ? 1 : 0;
    int mod_trama_sombras = PossuiTalento("magia_trama_sombras", proto) && EscolaBoaTramaDasSombras(feitico) ? 1 : 0;
    int base = 10;
    if (da->acao().has_dificuldade_salvacao_base()) {
      base = da->acao().dificuldade_salvacao_base() + mod_especializacao + mod_trama_sombras;
    } else {
      base += da->has_nivel_conjurador_pergaminho()
        ? NivelFeiticoPergaminho(tabelas, da->tipo_pergaminho(), feitico)
        : NivelFeitico(tabelas, TipoAtaqueParaClasse(tabelas, da->tipo_ataque()), feitico) + mod_especializacao;
    }
    const int mod_atributo = da->has_modificador_atributo_pergaminho()
      ? da->modificador_atributo_pergaminho()
      : da->acao().has_atributo_dificuldade_salvacao()
        ? ModificadorAtributo(da->acao().atributo_dificuldade_salvacao(), proto)
        : ModificadorAtributoConjuracao(ic.id(), proto);


    const int cd_final = base + mod_atributo;
    da->set_dificuldade_salvacao(cd_final);
  }

  if (da->acao().has_icone()) {
    da->set_icone(da->acao().icone());
  }
}

// Passa alguns dados de acao proto para dados ataque. Preenche o tipo com o tipo da arma se nao houver.
void ArmaParaDadosAtaque(const Tabelas& tabelas, const ArmaProto& arma, const EntidadeProto& proto, DadosAtaque* da) {
  // Aplica acao da arma.
  if (arma.has_acao()) {
    if (arma.acao().has_id()) {
      const auto& acao_tabelada = tabelas.Acao(arma.acao().id());
      *da->mutable_acao() = acao_tabelada;
      da->mutable_acao()->MergeFrom(arma.acao());
    } else {
      *da->mutable_acao() = arma.acao();
    }
  }

  // Toda acao valida deve ter um tipo. Se comecou sem, ja preenche com alguns automaticos.
  if (!da->acao().has_tipo()) {
    if (PossuiCategoria(CAT_PROJETIL_AREA, arma)) {
      da->mutable_acao()->set_tipo(ACAO_PROJETIL_AREA);
    } else if (PossuiCategoria(CAT_DISTANCIA, arma)) {
      da->mutable_acao()->set_tipo(ACAO_PROJETIL);
    } else {
      da->mutable_acao()->set_tipo(ACAO_CORPO_A_CORPO);
    }
  }
  if (PossuiCategoria(CAT_ARMA, arma)) {
    da->set_eh_arma(true);
  }
  if (arma.has_ataque_toque()) {
    da->set_ataque_toque(arma.ataque_toque());
  }
  // Como corpo a corpo é o padrao se nao tiver nada, verifica aqui ao inves da categoria.
  if (PossuiCategoria(CAT_CAC, arma) || da->acao().tipo() == ACAO_CORPO_A_CORPO) {
    da->set_ataque_corpo_a_corpo(true);
  }
  if (PossuiCategoria(CAT_DISTANCIA, arma)) {
    da->set_ataque_distancia(true);
  }
  if (PossuiCategoria(CAT_PROJETIL_AREA, arma)) {
    da->set_ataque_toque(true);
    da->set_ataque_distancia(true);
  }
  if (PossuiCategoria(CAT_ARREMESSO, arma)) {
    da->set_ataque_distancia(true);
    da->set_ataque_arremesso(true);
  }

  if (arma.has_modelo_dano()) {
    // Computa o dano basico fixo (independente de tamanho).
    int nivel = da->has_nivel_conjurador_pergaminho()
        ?  da->nivel_conjurador_pergaminho()
        : NivelConjurador(TipoAtaqueParaClasse(tabelas, da->tipo_ataque()), proto);
    ComputaDano(arma.modelo_dano(), nivel, da);
  }

  if (arma.has_veneno()) {
    //*da->mutable_acao()->mutable_veneno() = arma.veneno();
    *da->mutable_veneno() = arma.veneno();
  }
  if (arma.has_dano_ignora_salvacao()) {
    da->set_dano_ignora_salvacao(arma.dano_ignora_salvacao());
  }
  if (!arma.info_classes().empty()) {
    da->set_eh_feitico(true);
  }
}

void RecomputaDependenciasVenenoParaAtaque(const EntidadeProto& proto, DadosAtaque* da) {
  if (!da->has_veneno() || da->veneno().nao_usar_cd_dinamico()) {
    return;
  }
  const int nivel = Nivel(proto);
  const int mod = ModificadorAtributo(da->veneno().atributo_para_cd(), proto);
  da->mutable_veneno()->set_cd(10 + mod + nivel / 2);
}

void RecomputaDependenciasBesta(const DadosAtaque& da, Bonus* bonus_ataque) {
  const char kBestaUmaMao[] = "besta_uma_mao";
  if (da.empunhadura() == EA_ARMA_APENAS) {
    LimpaBonus(TB_SEM_NOME, kBestaUmaMao, bonus_ataque);
    return;
  }
  // A penalidade de duas armas sera recomputada em outro lugar.
  int penalidade = 0;
  if (da.empunhadura() == EA_ARMA_ESCUDO || da.empunhadura() == EA_MAO_BOA || da.empunhadura() == EA_MAO_RUIM) {
    const auto& id_arma = da.id_arma();
    if (id_arma.find("besta_de_mao") == 0) {
      ;
    } else if (id_arma.find("besta_leve") == 0) {
      penalidade = 2;
    } else if (id_arma.find("besta_pesada") == 0) {
      penalidade = 4;
    }
  }
  AtribuiOuRemoveBonus(-penalidade, TB_SEM_NOME, kBestaUmaMao, bonus_ataque);
}

void ResetDadosAtaque(DadosAtaque* da) {
  // Ao alterar aqui, altere o teste CamposResetadosNaoSetados em utils_test.cpp.
  da->clear_ataque_agarrar();
  da->clear_ataque_toque();
  da->clear_ataque_distancia();
  da->clear_ataque_arremesso();
  da->clear_ataque_corpo_a_corpo();
  da->clear_nao_letal();
  da->clear_requer_carregamento();
}

int LimiteOriginalAtaqueAtordoante(const EntidadeProto& proto) {
  int nivel_monge = Nivel("monge", proto);
  int nivel_personagem = NivelPersonagem(proto);
  int nivel_outros = nivel_personagem - nivel_monge;
  int limite_vezes = nivel_monge + (nivel_outros / 4);
  if (limite_vezes <= 0) {
    LOG(ERROR) << "erro gerando ataque atordoante, nivel de monge: " << nivel_monge << ", nivel outros: " << nivel_outros;
    return 0;
  }
  return limite_vezes;
}

std::string DanoBasicoMonge(int nivel) {
  if (nivel <= 0) return "1d3";
  // Aqui a gente seta o dano basico, porque queremos modificar por tamanho ainda.
  int mult = 1;
  int dado = 6;
  if (nivel > 3 && nivel <= 7) {
    mult = 1;
    dado = 8;
  } else if (nivel <= 11) {
    mult = 1;
    dado = 10;
  } else if (nivel <= 15) {
    mult = 2;
    dado = 6;
  } else if (nivel <= 19) {
    mult = 2;
    dado = 8;
  } else {
    mult = 2;
    dado = 10;
  }
  return StringPrintf("%dd%d", mult, dado);
}

void RecomputaDependenciasUmDadoAtaque(const Tabelas& tabelas, const EntidadeProto& proto, DadosAtaque* da) {
  ResetDadosAtaque(da);
  *da->mutable_acao() = AcaoProto::default_instance();

  // Passa alguns campos da acao para o ataque.
  const auto& arma = tabelas.ArmaOuFeitico(da->id_arma());
  ArmaParaDadosAtaque(tabelas, arma, proto, da);
  AcaoParaDadosAtaque(tabelas, arma, proto, da);
  const bool usando_escudo = da->empunhadura() == EA_ARMA_ESCUDO;
  // TODO verificar pericias nas armaduras e escudos.
  //const int penalidade_ataque_armadura = PenalidadeArmadura(tabelas, proto);
  const int penalidade_ataque_escudo = usando_escudo ? PenalidadeEscudo(tabelas, proto) : 0;
  auto* bonus_ataque = da->mutable_bonus_ataque();
  LimpaBonus(TB_PENALIDADE_ARMADURA, "armadura", bonus_ataque);
  LimpaBonus(TB_PENALIDADE_ESCUDO, "escudo", bonus_ataque);
  const int bba_cac = proto.bba().cac();
  const int bba_distancia = proto.bba().distancia();
  if (usando_escudo && !TalentoComEscudo(proto.dados_defesa().id_escudo(), proto)) {
    AtribuiBonus(-penalidade_ataque_escudo, TB_PENALIDADE_ESCUDO, "escudo_sem_talento", bonus_ataque);
  }
  // Aplica diferenca de tamanho de arma.
  TamanhoEntidade tamanho = proto.tamanho();
  if (da->has_tamanho()) {
    tamanho = static_cast<TamanhoEntidade>(tamanho + (da->tamanho() - proto.tamanho()));
  }
  if (tamanho < 0) {
    tamanho = TM_MINUSCULO;
  }
  if (tamanho > TM_COLOSSAL) {
    tamanho = TM_COLOSSAL;
  }

  if (arma.has_id()) {
    if (da->rotulo().empty()) {
      da->set_rotulo(arma.nome());
    }
    da->set_acuidade(false);
    da->set_nao_letal(arma.nao_letal());
    if (PossuiTalento("acuidade_arma", proto) &&
        bba_distancia > bba_cac &&
        (PossuiCategoria(CAT_LEVE, arma) ||
         arma.id() == "sabre" || arma.id() == "chicote" || arma.id() == "corrente_com_cravos")) {
      da->set_acuidade(true);
      AtribuiBonus(-penalidade_ataque_escudo, TB_PENALIDADE_ESCUDO, "escudo", bonus_ataque);
    }
    if (da->id_arma().find("besta") == 0) {
      RecomputaDependenciasBesta(*da, bonus_ataque);
    }
    da->set_requer_carregamento(arma.carregamento().requer_carregamento());
    if (!da->requer_carregamento()) {
      da->clear_descarregada();
    }

    int nivel_monge = Nivel("monge", proto);
    if (arma.id() == "desarmado" && nivel_monge > 0) {
      da->set_dano_basico(ConverteDanoBasicoMedioParaTamanho(DanoBasicoMonge(nivel_monge), tamanho));
      da->set_margem_critico(arma.margem_critico());
      da->set_multiplicador_critico(arma.multiplicador_critico());
    } else if (da->empunhadura() == EA_MAO_RUIM && PossuiCategoria(CAT_ARMA_DUPLA, arma) && arma.has_dano_secundario()) {
      da->set_dano_basico(DanoBasicoPorTamanho(tamanho, arma.dano_secundario()));
      da->set_margem_critico(arma.margem_critico_secundario());
      da->set_multiplicador_critico(arma.multiplicador_critico_secundario());
    } else if (arma.has_dano()) {
      da->set_dano_basico(DanoBasicoPorTamanho(tamanho, arma.dano()));
      da->set_margem_critico(arma.margem_critico());
      da->set_multiplicador_critico(arma.multiplicador_critico());
    }
    // TODO o efeito de lamina afiada se aplica a uma arma. Aqui to robando, usando id arma. Se for uma arma diferente com o mesmo id
    // vai dar errado (por exemplo, duas espadas iguais).
    if (PossuiTalento("sucesso_decisivo_aprimorado", da->id_arma(), proto) ||
        (PossuiEvento(EFEITO_LAMINA_AFIADA, da->rotulo(), proto) && (c_any(da->descritores(), DESC_CORTANTE) || c_any(da->descritores(), DESC_PERFURANTE)))) {
      DobraMargemCritico(da);
    }
    if (PossuiCategoria(CAT_ARREMESSO, arma) || PossuiCategoria(CAT_PROJETIL_AREA, arma)) {
      da->set_incrementos(5);
    } else if (PossuiCategoria(CAT_DISTANCIA, arma)) {
      da->set_incrementos(10);
    }
    *da->mutable_tipo_ataque_fisico() = TiposDanoParaAtaqueFisico(arma.tipo_dano());
  } else if (da->ataque_agarrar() && !da->has_dano_basico_medio_natural()) {
    int nivel_monge = Nivel("monge", proto);
    if (nivel_monge > 0) {
      da->set_dano_basico(ConverteDanoBasicoMedioParaTamanho(DanoBasicoMonge(nivel_monge), tamanho));
    } else {
      da->set_dano_basico(DanoDesarmadoPorTamanho(tamanho));
    }
    if (PossuiTalento("agarrar_aprimorado", proto) || da->acao().ignora_ataque_toque()) {
      da->set_ignora_ataque_toque(true);
    } else {
      da->clear_ignora_ataque_toque();
    }
  }
  if (da->has_dano_basico_fixo()) {
    da->set_dano_basico(da->dano_basico_fixo());
  } else if (da->has_dano_basico_medio_natural()) {
    da->set_dano_basico(ConverteDanoBasicoMedioParaTamanho(da->dano_basico_medio_natural(), tamanho));
  }

  if (da->id_talento() == "ataque_atordoante") {
    da->set_limite_vezes_original(LimiteOriginalAtaqueAtordoante(proto));
    da->set_dificuldade_salvacao(10 + NivelPersonagem(proto) / 2 + ModificadorAtributo(TA_SABEDORIA, proto));
  }

  // Tenta achar o primeiro da lista com mesmo rotulo para obter coisas derivadas do primeiro (municao, descritores).
  const DadosAtaque* primeiro = nullptr;
  for (const auto& dda : proto.dados_ataque()) {
    if (dda.grupo() == da->grupo() && dda.tipo_ataque() == da->tipo_ataque() && dda.rotulo() == da->rotulo()) {
      primeiro = &dda;
      break;
    }
  }

  if (da != primeiro && primeiro != nullptr) {
    // municao.
    if (primeiro->has_municao()) da->set_municao(primeiro->municao());
    da->set_descarregada(primeiro->descarregada());
    // Elemento.
    if (primeiro->has_acao()) *da->mutable_acao() = primeiro->acao();
    // material.
    if (primeiro->material_arma() == DESC_NENHUM) da->clear_material_arma();
    else da->set_material_arma(primeiro->material_arma());
    // tipo ataque fisico.
    if (primeiro->tipo_ataque_fisico().empty()) da->tipo_ataque_fisico();
    else *da->mutable_tipo_ataque_fisico() = primeiro->tipo_ataque_fisico();
    // Alinhamento.
    if (primeiro->has_alinhamento_bem_mal()) da->set_alinhamento_bem_mal(primeiro->alinhamento_bem_mal());
    else da->clear_alinhamento_bem_mal();
    if (primeiro->has_alinhamento_ordem_caos()) da->set_alinhamento_ordem_caos(primeiro->alinhamento_ordem_caos());
    else da->clear_alinhamento_ordem_caos();
  }
  // Descritores de ataque.

  da->clear_descritores();
  if (da->material_arma() != DESC_NENHUM) da->add_descritores(da->material_arma());
  if (da->alinhamento_bem_mal() != DESC_NENHUM) da->add_descritores(da->alinhamento_bem_mal());
  if (da->alinhamento_ordem_caos() != DESC_NENHUM) da->add_descritores(da->alinhamento_ordem_caos());
  if (BonusIndividualTotal(TB_MELHORIA, da->bonus_dano()) > 0 || da->bonus_magico() > 0) {
    da->add_descritores(DESC_MAGICO);
  }
  if (!da->tipo_ataque_fisico().empty()) {
    std::copy(da->tipo_ataque_fisico().begin(),
              da->tipo_ataque_fisico().end(),
              google::protobuf::RepeatedFieldBackInserter(da->mutable_descritores()));
  }
  // Alcance do ataque. Se a arma tiver alcance, respeita o que esta nela (armas a distancia). Caso contrario, usa o tamanho.
  if (arma.has_alcance_quadrados()) {
    int mod_distancia_quadrados = 0;
    const int nivel = NivelParaFeitico(tabelas, *da, proto);
    switch (arma.modificador_alcance()) {
      case ArmaProto::MOD_2_QUAD_NIVEL:
        mod_distancia_quadrados = 2 * nivel;
        break;
      case ArmaProto::MOD_8_QUAD_NIVEL:
        mod_distancia_quadrados = 8 * nivel;
        break;
      case ArmaProto::MOD_1_QUAD_CADA_2_NIVEIS:
        mod_distancia_quadrados = nivel / 2;
        break;
      default:
        ;
    }
    da->set_alcance_m((arma.alcance_quadrados() + mod_distancia_quadrados) * QUADRADOS_PARA_METROS);
    da->set_alcance_minimo_m(0);
  } else if (da->has_alcance_q()) {
    da->set_alcance_m(da->alcance_q() * QUADRADOS_PARA_METROS);
  } else if (da->ataque_corpo_a_corpo()) {
    // Regra para alcance. Criaturas com alcance zero nao se beneficiam de armas de haste.
    // https://rpg.stackexchange.com/questions/47227/do-creatures-with-inappropriately-sized-reach-weapons-threaten-different-areas/47338#47338
    int alcance = AlcanceTamanhoQuadrados(proto.tamanho());
    int alcance_minimo = 0;
    if (arma.haste()) {
      alcance_minimo = alcance;
      alcance *= 2;
    }
    da->set_alcance_m(alcance * QUADRADOS_PARA_METROS);
    da->set_alcance_minimo_m(alcance_minimo * QUADRADOS_PARA_METROS);
  }

  int bba = 0;
  bool usar_forca_dano = false;
  const int modificador_forca = ModificadorAtributo(proto.atributos().forca());
  if (da->ataque_distancia()) {
    bba = bba_distancia;
    if (PossuiCategoria(CAT_ARCO, arma)) {
      if (arma.has_max_forca()) {
        // Ajuste de arcos compostos.
        if (modificador_forca < arma.max_forca()) bba -= 2;
        usar_forca_dano = true;
      }
    } else if (da->ataque_arremesso()) {
      usar_forca_dano = true;
    }
  } else if (da->ataque_agarrar()) {
    bba = proto.bba().agarrar();
    usar_forca_dano = true;
  } else if (da->ataque_toque()) {
    // Toque nao aplica bonus de forca.
    bba = PossuiTalento("acuidade_arma", proto) ? std::max(bba_distancia, bba_cac) : bba_cac;
  } else if (da->ataque_corpo_a_corpo()) {
    bba = da->acuidade() ? bba_distancia : bba_cac;
    usar_forca_dano = true;
  } else if (da->acao().permite_ataque_vs_defesa()) {
    LOG(WARNING) << "Tipo de ataque não se enquadra em nada: " << da->DebugString();
  }

  {
    auto* bonus_dano = da->mutable_bonus_dano();
    // Obra prima e bonus magico.
    AtribuiBonus(bba, TB_BASE, "base", bonus_ataque);
    if (da->bonus_magico() > 0) {
      da->set_obra_prima(true);  // Toda arma magica eh obra prima.
      AtribuiBonus(da->bonus_magico(), TB_MELHORIA, "arma_magica", bonus_ataque);
      AtribuiBonus(da->bonus_magico(), TB_MELHORIA, "arma_magica", bonus_dano);
    } else {
      RemoveBonus(TB_MELHORIA, "arma_magica", bonus_ataque);
      RemoveBonus(TB_MELHORIA, "arma_magica", bonus_dano);
    }
    AtribuiOuRemoveBonus(da->obra_prima() ? 1 : 0, TB_MELHORIA, "obra_prima", bonus_ataque);
    // Talentos.
    AtribuiOuRemoveBonus(PossuiTalento("foco_em_arma", da->id_arma(), proto) ? 1 : 0, TB_SEM_NOME, "foco_em_arma", bonus_ataque);
    AtribuiOuRemoveBonus(PossuiTalento("foco_em_arma_maior", da->id_arma(), proto) ? 1 : 0, TB_SEM_NOME, "foco_em_arma_maior", bonus_ataque);
    // Duas maos ou armas naturais.
    switch (da->empunhadura()) {
      case EA_MAO_BOA: {
        // TODO detectar a arma da outra mao.
        const auto& arma_outra_mao = ArmaOutraMao(tabelas, *da, proto);
        int penalidade = PossuiCategoria(CAT_LEVE, arma_outra_mao) || PossuiCategoria(CAT_ARMA_DUPLA, arma) ? -4 : -6;
        if (PossuiTalento("combater_duas_armas", proto)) penalidade += 2;
        AtribuiBonus(penalidade, TB_SEM_NOME, "empunhadura", bonus_ataque);
        break;
      }
      case EA_MAO_RUIM: {
        int penalidade = PossuiCategoria(CAT_LEVE, arma) || PossuiCategoria(CAT_ARMA_DUPLA, arma) ? -8 : -10;
        if (PossuiTalento("combater_duas_armas", proto)) penalidade += 6;
        AtribuiBonus(penalidade, TB_SEM_NOME, "empunhadura", bonus_ataque);
        break;
      }
      case EA_MONSTRO_ATAQUE_SECUNDARIO: {
        int penalidade = PossuiTalento("ataques_multiplos", proto) ? -2 : -5;
        AtribuiBonus(penalidade, TB_SEM_NOME, "empunhadura", bonus_ataque);
        break;
      }
      default:
        RemoveBonus(TB_SEM_NOME, "empunhadura", bonus_ataque);
    }
    // Outros ataques.
    AtribuiOuRemoveBonus(-da->ordem_ataque() * 5, TB_SEM_NOME, "multiplos_ataque", bonus_ataque);
  }
  // Forca no dano.
  if (usar_forca_dano) {
    int modificador_forca_dano = arma.has_max_forca() ? std::min(modificador_forca, arma.max_forca()) : modificador_forca;
    int dano_forca = 0;
    EmpunhaduraArma ea = da->empunhadura();
    if (modificador_forca_dano < 0) {
      dano_forca = modificador_forca;
    } else if (ea == EA_2_MAOS) {
      dano_forca = floorf(modificador_forca_dano * 1.5f);
    } else if (ea == EA_MAO_RUIM || ea == EA_MONSTRO_ATAQUE_SECUNDARIO) {
      dano_forca = modificador_forca_dano / 2;
    } else {
      dano_forca = modificador_forca_dano;
    }
    AtribuiBonus(dano_forca, TB_ATRIBUTO, "forca", da->mutable_bonus_dano());
  } else {
    RemoveBonus(TB_ATRIBUTO, "forca", da->mutable_bonus_dano());
  }
  AtribuiOuRemoveBonus(
      PossuiTalento("especializacao_arma", da->id_arma(), proto) ? 2 : 0, TB_SEM_NOME, "especializacao_arma", da->mutable_bonus_dano());
  AtribuiOuRemoveBonus(
      PossuiTalento("especializacao_arma_maior", da->id_arma(), proto) ? 2 : 0, TB_SEM_NOME, "especializacao_arma_maior", da->mutable_bonus_dano());
  // Estes dois campos (bonus_ataque_final e dano) sao os mais importantes, porque sao os que valem.
  // So atualiza o BBA se houver algo para atualizar. Caso contrario deixa como esta.
  if (proto.has_bba() || !da->has_bonus_ataque_final()) da->set_bonus_ataque_final(CalculaBonusBaseParaAtaque(*da, proto));
  if (da->has_dano_basico() || !da->has_dano()) da->set_dano(CalculaDanoParaAtaque(*da, proto));
  if (da->grupo().empty()) da->set_grupo(google::protobuf::StringPrintf("%s|%s", da->tipo_ataque().c_str(), da->rotulo().c_str()));

  // CA do ataque.
  da->set_ca_normal(CATotal(proto, usando_escudo));
  da->set_ca_surpreso(CASurpreso(proto, usando_escudo));
  da->set_ca_toque(CAToque(proto));

  // Veneno.
  RecomputaDependenciasVenenoParaAtaque(proto, da);

  VLOG(1) << "Ataque recomputado: " << da->DebugString();
}

void RecomputaDependenciasDadosAtaque(const Tabelas& tabelas, EntidadeProto* proto) {
  // Remove ataques cujo numero de vezes exista e seja zero.
  RemoveSe<DadosAtaque>([](const DadosAtaque& da) {
    return da.has_limite_vezes() && da.limite_vezes() <= 0 &&
        !da.mantem_com_limite_zerado() && !da.has_taxa_refrescamento();
  }, proto->mutable_dados_ataque());

  // Se nao tiver agarrar, cria um.
  if (proto->gerar_agarrar() && c_none_of(proto->dados_ataque(),
        [] (const DadosAtaque& da) { return da.ataque_agarrar(); })) {
    auto* da = proto->mutable_dados_ataque()->Add();
    da->set_tipo_ataque("Agarrar");
    da->set_rotulo("agarrar");
  }
  // Se nao tiver ataque atordoante, gera um.
  if (PossuiTalento("ataque_atordoante", *proto)) {
    auto* dat = DadosAtaquePorTalento("ataque_atordoante", proto);
    if (dat == nullptr) {
      int limite_vezes = LimiteOriginalAtaqueAtordoante(*proto);
      if (limite_vezes > 0) {
        DadosAtaque da;
        da.set_id_talento("ataque_atordoante");
        da.set_rotulo("ataque atordoante");
        da.set_id_arma("desarmado");
        da.set_limite_vezes(limite_vezes);
        da.set_taxa_refrescamento(StringPrintf("%d", DIA_EM_RODADAS));
        da.set_mantem_com_limite_zerado(true);
        da.set_dano_ignora_salvacao(true);
        auto* acao = da.mutable_acao_fixa();
        acao->set_permite_salvacao(true);
        acao->set_tipo_salvacao(TS_FORTITUDE);
        auto* ed = acao->add_efeitos_adicionais();
        ed->set_efeito(EFEITO_ATORDOADO);
        ed->set_rodadas(1);
        InsereInicio(&da, proto->mutable_dados_ataque());
        dat = proto->mutable_dados_ataque(0);
      }
    }
  }

  for (auto& da : *proto->mutable_dados_ataque()) {
    RecomputaDependenciasUmDadoAtaque(tabelas, *proto, &da);
  }
  //EntidadeProto p;
  //*p.mutable_dados_ataque() = proto->dados_ataque();
  //LOG(INFO) << "dados_ataque: "  << p.DebugString();
}

void RecomputaDependenciasMovimento(const Tabelas& tabelas, EntidadeProto* proto) {
  auto* movimento = proto->mutable_movimento();
  AtribuiOuRemoveBonus(movimento->terrestre_basico_q(), TB_BASE, "base", movimento->mutable_terrestre_q());
  AtribuiOuRemoveBonus(movimento->aereo_basico_q(),     TB_BASE, "base", movimento->mutable_aereo_q());
  AtribuiOuRemoveBonus(movimento->aquatico_basico_q(),  TB_BASE, "base", movimento->mutable_aquatico_q());
  AtribuiOuRemoveBonus(movimento->escavando_basico_q(), TB_BASE, "base", movimento->mutable_escavando_q());
  AtribuiOuRemoveBonus(movimento->escalando_basico_q(), TB_BASE, "base", movimento->mutable_escalando_q());
}

}  // namespace

void RecomputaDependencias(const Tabelas& tabelas, EntidadeProto* proto, Entidade* entidade) {
  VLOG(2) << "Proto antes RecomputaDependencias: " << proto->ShortDebugString();
  ResetComputados(proto);

  RecomputaDependenciasRaciais(tabelas, proto);
  RecomputaDependenciasItensMagicos(tabelas, proto);
  RecomputaDependenciasTendencia(proto);
  RecomputaDependenciasEfeitos(tabelas, proto, entidade);
  auto* bonus_forca = BonusAtributo(TA_FORCA, proto);
  auto* bonus_destreza = BonusAtributo(TA_DESTREZA, proto);
  if (PossuiEventoNaoPossuiOutro(EFEITO_PARALISIA, EFEITO_MOVIMENTACAO_LIVRE, *proto)) {
    // Zera destreza e força.
    const int forca = BonusTotal(*bonus_forca);
    if (forca > 0) {
      AtribuiBonus(-forca, TB_SEM_NOME, "paralisia", bonus_forca);
    }
    const int destreza = BonusTotal(*bonus_destreza);
    if (destreza > 0) {
      AtribuiBonus(-destreza, TB_SEM_NOME, "paralisia", bonus_destreza);
    }
  } else {
    LimpaBonus(TB_SEM_NOME, "paralisia", bonus_forca);
    LimpaBonus(TB_SEM_NOME, "paralisia", bonus_destreza);
  }

  RecomputaDependenciasNiveisNegativos(tabelas, proto);
  RecomputaDependenciasDestrezaLegado(tabelas, proto);
  RecomputaDependenciasClasses(tabelas, proto);
  RecomputaDependenciasDominios(tabelas, proto);
  RecomputaDependenciasTalentos(tabelas, proto);
  RecomputaDependenciaTamanho(proto);
  RecomputaDependenciasPontosVidaTemporarios(proto);
  RecomputaDependenciasPontosVida(proto);
  RecomputaDependenciasResistenciaMagia(proto);

  // TODO: porque ta pegando o atributo e nao o bonus total? Mesmo assim parece que funciona.
  int modificador_destreza           = ModificadorAtributo(proto->atributos().destreza());
  const int modificador_constituicao = ModificadorAtributo(proto->atributos().constituicao());
  //const int modificador_inteligencia = ModificadorAtributo(BonusTotal(proto->atributos().inteligencia()));
  const int modificador_sabedoria    = ModificadorAtributo(proto->atributos().sabedoria());
  const int modificador_carisma      = ModificadorAtributo(proto->atributos().carisma());

  // Iniciativa.
  RecomputaDependenciasIniciativa(modificador_destreza, proto);

  // Classe de Armadura.
  RecomputaDependenciasCA(tabelas, proto);
  // Salvacoes.
  RecomputaDependenciasSalvacoes(
      modificador_constituicao, modificador_destreza, modificador_sabedoria, modificador_carisma,
      tabelas, proto);
  // Evasao.
  RecomputaDependenciasEvasao(tabelas, proto);

  // BBA: tenta atualizar por classe, se nao houver, pelo bba base, senao nao faz nada.
  if (proto->info_classes_size() > 0 ||  proto->bba().has_base()) {
    const int modificador_forca = ModificadorAtributo(proto->atributos().forca());
    const int modificador_tamanho = ModificadorTamanho(proto->tamanho());
    const int bba = proto->info_classes_size() > 0 ? CalculaBonusBaseAtaque(*proto) : proto->bba().base();
    const int niveis_negativos = proto->niveis_negativos();
    proto->mutable_bba()->set_base(bba);
    proto->mutable_bba()->set_cac(modificador_forca + modificador_tamanho + bba - niveis_negativos);
    proto->mutable_bba()->set_distancia(modificador_destreza + modificador_tamanho + bba - niveis_negativos);
    int total_agarrar = modificador_forca + ModificadorTamanhoAgarrar(proto->tamanho()) + bba - niveis_negativos;
    if (PossuiTalento("agarrar_aprimorado", *proto)) {
      total_agarrar += 4;
    }
    proto->mutable_bba()->set_agarrar(total_agarrar);
  }

  // Atualiza os bonus de ataques.
  RecomputaDependenciasDadosAtaque(tabelas, proto);

  RecomputaDependenciasPericias(tabelas, proto);

  RecomputaClasseFeiticoAtiva(tabelas, proto);
  RecomputaDependenciasMagiasConhecidas(tabelas, proto);
  RecomputaDependenciasMagiasParaLancarPorDia(tabelas, proto);
  RecomputaDependenciasMovimento(tabelas, proto);

  VLOG(2) << "Proto depois RecomputaDependencias: " << proto->ShortDebugString();
}

}  // namespace ent
