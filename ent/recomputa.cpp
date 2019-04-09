#include "ent/recomputa.h"

#include <unordered_set>
#include "ent/constantes.h"
#include "ent/util.h"
#include "goog/stringprintf.h"
#include "log/log.h"

namespace ent {
namespace {

using google::protobuf::StringPrintf;
using google::protobuf::RepeatedPtrField;

// Remove o item do container se bater com predicado.
template <class T>
void RemoveSe(const std::function<bool(const T& t)>& predicado, RepeatedPtrField<T>* c) {
  for (int i = c->size() - 1; i >= 0; --i) {
    if (predicado(c->Get(i))) c->DeleteSubrange(i, 1);
  }
}

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
    case TM_MEDIO: return dano.medio();
    case TM_PEQUENO: return dano.pequeno();
    case TM_GRANDE: return dano.grande();
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

bool ConsequenciaAfetaDadosAtaque(const ConsequenciaEvento& consequencia, const DadosAtaque& da) {
  if (!consequencia.has_restricao_arma()) return true;
  const auto& ra = consequencia.restricao_arma();
  if (ra.has_prefixo_arma() && da.id_arma().find(ra.prefixo_arma()) == 0) return true;
  return c_any(consequencia.restricao_arma().id_arma(), da.id_arma());
}

// Retorna o dado de ataque que contem a arma, ou nullptr;
const DadosAtaque* DadosAtaqueProto(const std::string& id_arma, const EntidadeProto& proto) {
  for (const auto& da : proto.dados_ataque()) {
    if (da.id_arma() == id_arma) return &da;
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
  AplicaBonusPenalidadeOuRemove(consequencia.bonus_iniciativa(), proto->mutable_bonus_iniciativa());
  for (auto& da : *proto->mutable_dados_ataque()) {
    if (!ConsequenciaAfetaDadosAtaque(consequencia, da)) continue;
    AplicaBonusPenalidadeOuRemove(consequencia.jogada_ataque(), da.mutable_bonus_ataque());
    AplicaBonusPenalidadeOuRemove(consequencia.jogada_dano(), da.mutable_bonus_dano());
  }

  AplicaBonusPenalidadeOuRemove(consequencia.tamanho(), proto->mutable_bonus_tamanho());
  for (const auto& dp : consequencia.dados_pericia()) {
    auto* pericia = PericiaOuNullptr(dp.id(), proto);
    if (pericia == nullptr) continue;
    AplicaBonusPenalidadeOuRemove(dp.bonus(), pericia->mutable_bonus());
  }
}

bool ImuneMorte(const EntidadeProto& proto) {
  return TemTipoDnD(TIPO_CONSTRUCTO, proto) || TemTipoDnD(TIPO_MORTO_VIVO, proto);
}

bool AplicaEfeito(const EntidadeProto::Evento& evento, const ConsequenciaEvento& consequencia, EntidadeProto* proto) {
  AplicaEfeitoComum(consequencia, proto);
  // Aqui eh importante diferenciar entre return e break. Eventos que retornam nao seram considerados processados.
  switch (evento.id_efeito()) {
    case EFEITO_MORTE:
      if (!evento.processado() && !ImuneMorte(*proto)) {
        proto->set_morta(true);
        proto->set_caida(true);
      }
      break;
    case EFEITO_VITALIDADE_ILUSORIA:
      if (!evento.processado()) {
        // Gera os pontos de vida temporarios.
        const int tmp = RolaDado(8);
        auto* po = AtribuiBonus(tmp, TB_SEM_NOME, "vitalidade ilusória", proto->mutable_pontos_vida_temporarios_por_fonte());
        // Nivel conjurador: hard coded.
        // Forca: pelo comum.
      }
      break;
    case EFEITO_FORMA_GASOSA:
      if (!evento.processado()) {
        auto* pd = proto->mutable_dados_defesa()->add_reducao_dano();
        pd->add_descritores(DESC_MAGICO);
        pd->set_valor(15);
        pd->set_id_unico(evento.id_unico());
      }
      break;
    case EFEITO_DRENAR_TEMPORARIO:
      if (!evento.processado()) {
        if (evento.complementos().empty() || evento.complementos(0) <= 0) return false;
        AtribuiBonus(
            evento.complementos(0), TB_SEM_NOME, StringPrintf("drenar temporario %d", evento.id_unico()), proto->mutable_niveis_negativos_dinamicos());
      }
      break;
    case EFEITO_VENENO:
      break;
    case EFEITO_INVISIBILIDADE:
      if (!PossuiEvento(EFEITO_POEIRA_OFUSCANTE, *proto)) {
        proto->set_visivel(false);
      }
      break;
    case EFEITO_POEIRA_OFUSCANTE:
      proto->set_visivel(true);
      proto->set_ignora_luz(true);
      break;
    case EFEITO_COMPETENCIA_PERICIA: {
      if (evento.complementos_str().empty()) return false;
      // Encontra a pericia do efeito.
      auto* pericia_proto = PericiaCriando(evento.complementos_str(0), proto);
      Bonus bonus;
      auto* bi = bonus.add_bonus_individual();
      bi->set_tipo(TB_COMPETENCIA);
      auto* po = bi->add_por_origem();
      po->set_valor(evento.complementos(0));
      po->set_origem(google::protobuf::StringPrintf("competencia (id: %d)", evento.id_unico()));
      AplicaBonusPenalidadeOuRemove(bonus, pericia_proto->mutable_bonus());
    }
    break;
    case EFEITO_AJUDA:
      if (!evento.processado()) {
        // Gera os pontos de vida temporarios.
        int complemento = evento.complementos().empty() ? 3 : evento.complementos(0);
        if (complemento < 3) complemento = 3;
        else if (complemento > 10) complemento = 10;
        const int tmp = RolaDado(8) + complemento;
        auto* po = AtribuiBonus(tmp, TB_SEM_NOME, "ajuda", proto->mutable_pontos_vida_temporarios_por_fonte());
        if (evento.has_id_unico()) po->set_id_unico(evento.id_unico());
      }
    break;
    case EFEITO_PEDRA_ENCANTADA:
      if (!evento.processado()) {
        const auto* funda = DadosAtaqueProto("funda", *proto);
        DadosAtaque da;
        da.set_id_unico_efeito(evento.id_unico());
        da.set_bonus_magico(1);
        da.set_dano_basico_fixo("1d6");
        if (funda != nullptr) {
          da.set_id_arma("funda");
          da.set_empunhadura(funda->empunhadura());
        } else {
          da.set_empunhadura(EA_ARMA_ESCUDO);
        }
        da.set_rotulo(funda != nullptr ? "pedra encantada com funda" : "pedra encantada");
        da.set_tipo_ataque("Ataque a Distância");
        da.set_municao(3);
        InsereInicio(&da, proto->mutable_dados_ataque());
      }
    break;
    case EFEITO_ABENCOAR_ARMA: {
      if (evento.complementos_str().empty()) return false;
      std::vector<DadosAtaque*> das = DadosAtaquePorRotulo(evento.complementos_str(0), proto);
      for (auto* da : das) {
        da->set_alinhamento(DESC_BEM);
      }
    }
    break;
    case EFEITO_PRESA_MAGICA:
    case EFEITO_ARMA_MAGICA: {
      if (evento.complementos_str().empty()) return false;
      int valor = 1;
      if (!evento.complementos().empty()) {
        valor = std::max(0, std::min(evento.complementos(0), 5));
      }
      std::vector<DadosAtaque*> das = DadosAtaquePorRotulo(evento.complementos_str(0), proto);
      for (auto* da : das) {
        AtribuiBonusPenalidadeSeMaior(
            valor, TB_MELHORIA, evento.id_efeito() == EFEITO_ARMA_MAGICA ? "arma_magica_magia" : "presa_magica_magia", da->mutable_bonus_ataque());
      }
    }
    break;
    case EFEITO_TENDENCIA_EM_ARMA: {
      if (evento.complementos_str().size() != 2) return false;
      DescritorAtaque desc = StringParaDescritorAlinhamento(evento.complementos_str(1));
      if (desc == DESC_NENHUM) return false;
      std::vector<DadosAtaque*> das = DadosAtaquePorRotulo(evento.complementos_str(0), proto);
      for (auto* da : das) {
        da->set_alinhamento(desc);
      }
    }
    break;
    case EFEITO_RESISTENCIA_ELEMENTOS: {
      if (evento.complementos_str().size() != 2) return false;
      DescritorAtaque descritor = StringParaDescritorElemento(evento.complementos_str(0));
      if (descritor == DESC_NENHUM) {
        LOG(ERROR) << "descritor invalido: " << evento.complementos_str(0);
        return false;
      }
      int valor = atoi(evento.complementos_str(1).c_str());
      if (valor <= 0 || valor > 1000) return false;
      ResistenciaElementos re;
      re.set_valor(valor);
      re.set_descritor(descritor);
      re.set_id_unico(evento.id_unico());
      auto* re_corrente = AchaResistenciaElemento(evento.id_unico(), proto);
      if (re_corrente == nullptr) {
        re_corrente = proto->mutable_dados_defesa()->add_resistencia_elementos();
      }
      re_corrente->Swap(&re);
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

void AplicaFimPedraEncantada(int id_unico, EntidadeProto* proto) {
  // Encontra o dado de ataque.
  int i = 0;
  for (const auto& da : proto->dados_ataque()) {
    if (da.id_unico_efeito() == id_unico) {
      proto->mutable_dados_ataque()->DeleteSubrange(i, 1);
      return;
    }
  }
}


void AplicaFimAlinhamentoArma(const std::string& rotulo, EntidadeProto* proto) {
  // Encontra o dado de ataque.
  for (auto& da : *proto->mutable_dados_ataque()) {
    if (da.rotulo() == rotulo) {
      da.clear_alinhamento();
    }
  }
}

void AplicaFimEfeito(const EntidadeProto::Evento& evento, const ConsequenciaEvento& consequencia, EntidadeProto* proto) {
  AplicaEfeitoComum(consequencia, proto);
  switch (evento.id_efeito()) {
    case EFEITO_VITALIDADE_ILUSORIA:
      LimpaBonus(TB_SEM_NOME, "vitalidade ilusória", proto->mutable_pontos_vida_temporarios_por_fonte());
      break;
    case EFEITO_FORMA_GASOSA:
      for (int i = 0; i < proto->dados_defesa().reducao_dano().size(); ++i) {
        if (proto->dados_defesa().reducao_dano(i).id_unico() == evento.id_unico()) {
          proto->mutable_dados_defesa()->mutable_reducao_dano()->DeleteSubrange(i, 1);
          break;
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
    case EFEITO_AJUDA: {
      auto* bi = BonusIndividualSePresente(TB_SEM_NOME, proto->mutable_pontos_vida_temporarios_por_fonte());
      auto* po = OrigemSePresente("ajuda", bi);
      if (po == nullptr) {
        break;
      }
      if (evento.has_id_unico()) {
        // Se tiver id unico, respeita o id.
        RemoveSe<BonusIndividual::PorOrigem>([&evento] (const BonusIndividual::PorOrigem& ipo) {
          return ipo.id_unico() == evento.id_unico();
        }, bi->mutable_por_origem());
      } else {
        // Nao tem id, remove a ajuda por completo. Pode dar merda.
        LOG(WARNING) << "Removendo ajuda sem id unico.";
        RemoveBonus(TB_SEM_NOME, "ajuda", proto->mutable_pontos_vida_temporarios_por_fonte());
      }
    }
    break;
    case EFEITO_FURIA_BARBARO:
      AplicaFimFuriaBarbaro(proto);
    break;
    case EFEITO_PEDRA_ENCANTADA:
      AplicaFimPedraEncantada(evento.id_unico(), proto);
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
      LimpaResistenciaElemento(evento.id_unico(), proto);
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
      po.set_origem(StringPrintf("%s, origem: %s", po.origem().c_str(), origem.c_str()));
      po.set_valor(0);
    }
  }
}

// Zera todos os valores de origem para o bonus, para criar o fim de efeito de modelos.
void ZeraValorBonus(Bonus* bonus) {
  for (auto& bi : *bonus->mutable_bonus_individual()) {
    for (auto& po : *bi.mutable_por_origem()) {
      po.set_valor(0);
    }
  }
}

// Caso a consequencia use complemento, preenchera os valores existentes com ela.
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
  if (c.has_jogada_ataque())            PreencheOrigemValor(origem, complementos, c.mutable_jogada_ataque());
  if (c.has_jogada_dano())              PreencheOrigemValor(origem, complementos, c.mutable_jogada_dano());
  if (c.has_tamanho())                  PreencheOrigemValor(origem, complementos, c.mutable_tamanho());
  if (c.has_bonus_iniciativa())         PreencheOrigemValor(origem, complementos, c.mutable_bonus_iniciativa());
  for (auto& dp : *c.mutable_dados_pericia()) {
    PreencheOrigemValor(origem, complementos, dp.mutable_bonus());
  }
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
  if (c.has_jogada_ataque())            PreencheOrigemZeraValor(origem, c.mutable_jogada_ataque());
  if (c.has_jogada_dano())              PreencheOrigemZeraValor(origem, c.mutable_jogada_dano());
  if (c.has_tamanho())                  PreencheOrigemZeraValor(origem, c.mutable_tamanho());
  for (auto& dp : *c.mutable_dados_pericia()) {
    PreencheOrigemZeraValor(origem, dp.mutable_bonus());
  }
  return c;
}

ConsequenciaEvento PreencheConsequenciaFimParaModelos(const ConsequenciaEvento& consequencia_original) {
  ConsequenciaEvento c(consequencia_original);
  if (c.atributos().has_forca())        ZeraValorBonus(c.mutable_atributos()->mutable_forca());
  if (c.atributos().has_destreza())     ZeraValorBonus(c.mutable_atributos()->mutable_destreza());
  if (c.atributos().has_constituicao()) ZeraValorBonus(c.mutable_atributos()->mutable_constituicao());
  if (c.atributos().has_inteligencia()) ZeraValorBonus(c.mutable_atributos()->mutable_inteligencia());
  if (c.atributos().has_sabedoria())    ZeraValorBonus(c.mutable_atributos()->mutable_sabedoria());
  if (c.atributos().has_carisma())      ZeraValorBonus(c.mutable_atributos()->mutable_carisma());
  if (c.dados_defesa().has_ca())        ZeraValorBonus(c.mutable_dados_defesa()->mutable_ca());
  if (c.dados_defesa().has_salvacao_fortitude()) ZeraValorBonus(c.mutable_dados_defesa()->mutable_salvacao_fortitude());
  if (c.dados_defesa().has_salvacao_vontade())   ZeraValorBonus(c.mutable_dados_defesa()->mutable_salvacao_vontade());
  if (c.dados_defesa().has_salvacao_reflexo())   ZeraValorBonus(c.mutable_dados_defesa()->mutable_salvacao_reflexo());
  if (c.dados_defesa().has_cura_acelerada())     ZeraValorBonus(c.mutable_dados_defesa()->mutable_cura_acelerada());
  if (c.has_jogada_ataque())            ZeraValorBonus(c.mutable_jogada_ataque());
  if (c.has_jogada_dano())              ZeraValorBonus(c.mutable_jogada_dano());
  if (c.has_tamanho())                  ZeraValorBonus(c.mutable_tamanho());
  if (c.has_bonus_iniciativa())         ZeraValorBonus(c.mutable_bonus_iniciativa());
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


void RecomputaDependenciasMagiasPorDia(const Tabelas& tabelas, EntidadeProto* proto) {
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

    // Inclui o nivel 0. Portanto, se o nivel maximo eh 2, deve haver 3 elementos.
    // Na tabela, o nivel zero nao esta presente entao tem que ser compensado aqui.
    Redimensiona(nao_possui_nivel_zero ? magias_por_dia.size() + 1 : magias_por_dia.size(), fc->mutable_feiticos_por_nivel());

    if (nao_possui_nivel_zero) {
      fc->mutable_feiticos_por_nivel(0)->Clear();
    }
    for (unsigned int indice = 0; indice < magias_por_dia.size(); ++indice) {
      int nivel_magia = nao_possui_nivel_zero ? indice + 1 : indice;
      int magias_do_nivel =
        (magias_por_dia[indice] - '0') +
        FeiticosBonusPorAtributoPorNivel(
            nivel_magia,
            BonusAtributo(classe_tabelada.atributo_conjuracao(), *proto)) +
        (classe_tabelada.possui_dominio() && nivel_magia > 0 && nivel_magia <= 9 ? 1 : 0);
      Redimensiona(magias_do_nivel, fc->mutable_feiticos_por_nivel(nivel_magia)->mutable_para_lancar());
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
  if (!raca_tabelada.dados_defesa().resistencia_elementos().empty()) {
    *proto->mutable_dados_defesa()->mutable_resistencia_elementos() = raca_tabelada.dados_defesa().resistencia_elementos();
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
  // TODO sinergia e talentos.
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

void RecomputaDependenciasMagiasConhecidas(const Tabelas& tabelas, EntidadeProto* proto) {
  for (auto& ic : *proto->mutable_info_classes()) {
    if (!ic.has_progressao_conjurador() || ic.nivel() <= 0) continue;
    // Encontra a entrada da classe, ou cria se nao houver.
    auto* fc = FeiticosClasse(ic.id(), proto);
    // Le a progressao.
    const int nivel = std::min(ic.nivel(), 20);
    const auto& classe_tabelada = tabelas.Classe(ic.has_id_para_progressao_de_magia() ? ic.id_para_progressao_de_magia() : ic.id());
    // Esse caso deveria dar erro. O cara tem nivel acima do que esta na tabela.
    if (nivel >= classe_tabelada.progressao_feitico().para_nivel_size()) continue;
    const std::string& magias_conhecidas = classe_tabelada.progressao_feitico().para_nivel(nivel).conhecidos();
    // Classe nao tem magias conhecidas.
    if (magias_conhecidas.empty()) continue;

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
      for (auto& fc : *fc->mutable_feiticos_por_nivel(nivel_magia)->mutable_conhecidos()) {
        if (!fc.has_nome()) {
          fc.set_nome(tabelas.Feitico(fc.id()).nome());
        }
      }
    }
  }
}


void RecomputaDependenciasPontosVidaTemporarios(EntidadeProto* proto) {
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

int OutrosModificadoresNivelConjuracao(const EntidadeProto& proto) {
  return PossuiEvento(EFEITO_VITALIDADE_ILUSORIA, proto) ? 1 : 0;
}

void RecomputaNivelConjuracao(const Tabelas& tabelas, const EntidadeProto& proto, InfoClasse* ic) {
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
      ic.set_atributo_conjuracao(classe_tabelada_conjuracao.atributo_conjuracao());
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
  // TODO
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
}

void RecomputaDependenciasSalvacoes(
    int modificador_constituicao, int modificador_destreza, int modificador_sabedoria, const Tabelas& tabelas, EntidadeProto* proto_retornado) {
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
  if (proto->tendencia().has_simples()) {
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
    proto->mutable_tendencia()->clear_simples();
    proto->mutable_tendencia()->set_eixo_bem_mal(bem_mal);
    proto->mutable_tendencia()->set_eixo_ordem_caos(ordem_caos);
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
    for (unsigned int id_efeito : item->ids_efeitos()) {
      ids.insert(id_efeito);
    }
  }
  return ids;
}

// Retorna true se o item que criou o evento nao existe mais.
bool EventoOrfao(const EntidadeProto::Evento& evento, const std::unordered_set<unsigned int>& ids_itens) {
  return evento.continuo() && ids_itens.find(evento.id_unico()) == ids_itens.end();
}

void RecomputaDependenciasEfeitos(const Tabelas& tabelas, EntidadeProto* proto) {
  std::set<int, std::greater<int>> eventos_a_remover;
  const std::unordered_set<unsigned int> ids_itens = IdsItensComEfeitos(*proto);
  int i = 0;
  // Verifica eventos acabados.
  const int total_constituicao_antes = BonusTotal(proto->atributos().constituicao());
  for (const auto& evento : proto->evento()) {
    if (evento.rodadas() < 0 || EventoOrfao(evento, ids_itens)) {
      const auto& efeito = tabelas.Efeito(evento.id_efeito());
      VLOG(1) << "removendo efeito: " << TipoEfeito_Name(efeito.id());
      if (efeito.has_consequencia_fim()) {
        AplicaFimEfeito(evento, PreencheConsequencia(evento.origem(), evento.complementos(), efeito.consequencia_fim()), proto);
      } else {
        AplicaFimEfeito(evento, PreencheConsequenciaFim(evento.origem(), efeito.consequencia()), proto);
      }
      eventos_a_remover.insert(i);
    }
    ++i;
  }
  // Modelos desativados.
  for (auto& modelo : *proto->mutable_modelos()) {
    if (!ModeloDesligavel(tabelas, modelo) || modelo.ativo()) continue;
    const auto& efeito = tabelas.EfeitoModelo(modelo.id_efeito());
    VLOG(1) << "removendo efeito de modelo: " << TipoEfeitoModelo_Name(efeito.id());
    AplicaEfeitoComum(PreencheConsequenciaFimParaModelos(efeito.consequencia()), proto);
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
      VLOG(1) << "ignorando efeito: " << TipoEfeito_Name(efeito.id());
      continue;
    }
    VLOG(1) << "aplicando efeito: " << TipoEfeito_Name(efeito.id());
    if (AplicaEfeito(evento, PreencheConsequencia(evento.origem(), evento.complementos(), efeito.consequencia()), proto)) {
      evento.set_processado(true);
    }
  }
  // Efeito de modelos.
  for (auto& modelo : *proto->mutable_modelos()) {
    if (ModeloDesligavel(tabelas, modelo) && !modelo.ativo()) continue;
    const auto& efeito = tabelas.EfeitoModelo(modelo.id_efeito());
    VLOG(1) << "aplicando efeito de modelo: " << TipoEfeitoModelo_Name(efeito.id());
    AplicaEfeitoComum(efeito.consequencia(), proto);
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
    da->mutable_acao()->MergeFrom(da->acao_fixa());
  }

  const AcaoProto& acao = da->acao();
  if (acao.has_id() && da->tipo_ataque().empty()) {
    da->set_tipo_ataque(acao.id());
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

  if (da->acao().has_dificuldade_salvacao_base() || da->acao().has_dificuldade_salvacao_por_nivel()) {
    // Essa parte eh tricky. Algumas coisas tem que ser a classe mesmo: tipo atributo (feiticeiro usa carisma).
    // Outras tem que ser a classe de feitico, por exemplo, nivel de coluna de chama para mago.
    // A chamada InfoClasseParaFeitico busca a classe do personagem (feiticeiro)
    // enquanto TipoAtaqueParaClasse busca a classe para feitico (mago).
    const auto& ic = InfoClasseParaFeitico(tabelas, da->tipo_ataque(), proto);
    int base = 10;
    if (da->acao().has_dificuldade_salvacao_base()) {
      base = da->acao().dificuldade_salvacao_base();
    } else {
      base += da->has_nivel_conjurador_pergaminho()
        ? NivelFeiticoPergaminho(tabelas, da->tipo_pergaminho(), feitico)
        : NivelFeitico(tabelas, TipoAtaqueParaClasse(tabelas, da->tipo_ataque()), feitico);
    }
    const int mod_atributo = da->has_modificador_atributo_pergaminho()
      ? da->modificador_atributo_pergaminho()
      : da->acao().has_atributo_dificuldade_salvacao()
        ? ModificadorAtributo(da->acao().atributo_dificuldade_salvacao(), proto)
        : ModificadorAtributoConjuracao(ic.id(), proto);
    const int cd_final = base + mod_atributo;
    //da->mutable_acao()->set_dificuldade_salvacao(cd_final);
    da->set_dificuldade_salvacao(cd_final);
  }

  if (da->acao().has_icone()) {
    da->set_icone(da->acao().icone());
  }
  if (da->tipo_ataque().find("Pergaminho") == 0) {
    da->mutable_acao()->set_icone("icon_scroll.png");
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
  if (da->tipo_ataque().empty()) {
    if (PossuiCategoria(CAT_PROJETIL_AREA, arma)) {
      da->set_tipo_ataque("Projétil de Área");
      da->mutable_acao()->set_id("Projétil de Área");
      da->mutable_acao()->set_tipo(ACAO_PROJETIL_AREA);
    } else if (PossuiCategoria(CAT_DISTANCIA, arma)) {
      da->set_tipo_ataque("Ataque a Distância");
      da->mutable_acao()->set_id("Ataque a Distância");
      da->mutable_acao()->set_tipo(ACAO_PROJETIL);
    } else {
      da->set_tipo_ataque("Ataque Corpo a Corpo");
      da->mutable_acao()->set_id("Ataque Corpo a Corpo");
      da->mutable_acao()->set_tipo(ACAO_CORPO_A_CORPO);
    }
  }
  if (arma.has_ataque_toque()) {
    da->set_ataque_toque(arma.ataque_toque());
  }
  if (PossuiCategoria(CAT_CAC, arma)) {
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
    if (da->has_nivel_conjurador_pergaminho()) {
      // Para pergaminhos computarem os efeitos.
      ComputaDano(arma.modelo_dano(), da->nivel_conjurador_pergaminho(), da);
    } else {
      const int nivel = NivelConjurador(TipoAtaqueParaClasse(tabelas, da->tipo_ataque()), proto);
      ComputaDano(arma.modelo_dano(), nivel, da);
    }
  }

  if (arma.has_veneno()) {
    *da->mutable_acao()->mutable_veneno() = arma.veneno();
  }
  if (arma.has_dano_ignora_salvacao()) {
    da->set_dano_ignora_salvacao(arma.dano_ignora_salvacao());
  }
}

void RecomputaDependenciasVenenoParaAtaque(const EntidadeProto& proto, DadosAtaque* da) {
  if (!da->has_veneno() || da->veneno().nao_usar_cd_dinamico()) {
    return;
  }
  const int nivel = Nivel(proto);
  const int mod_con = ModificadorAtributo(TA_CONSTITUICAO, proto);
  da->mutable_veneno()->set_cd(10 + mod_con + nivel / 2);
}

void RecomputaDependenciasArma(const Tabelas& tabelas, const EntidadeProto& proto, DadosAtaque* da) {
  *da->mutable_acao() = AcaoProto::default_instance();
  // Passa alguns campos da acao para o ataque.
  const auto& arma = tabelas.ArmaOuFeitico(da->id_arma());
  ArmaParaDadosAtaque(tabelas, arma, proto, da);
  AcaoParaDadosAtaque(tabelas, arma, proto, da);
  const bool permite_escudo = da->empunhadura() == EA_ARMA_ESCUDO;
  // TODO verificar pericias nas armaduras e escudos.
  //const int penalidade_ataque_armadura = PenalidadeArmadura(tabelas, proto);
  const int penalidade_ataque_escudo = permite_escudo ? PenalidadeEscudo(tabelas, proto) : 0;
  auto* bonus_ataque = da->mutable_bonus_ataque();
  LimpaBonus(TB_PENALIDADE_ARMADURA, "armadura", bonus_ataque);
  LimpaBonus(TB_PENALIDADE_ESCUDO, "escudo", bonus_ataque);
  const int bba_cac = proto.bba().cac();
  const int bba_distancia = proto.bba().distancia();
  if (arma.has_id()) {
    if (da->rotulo().empty()) da->set_rotulo(arma.nome());
    da->set_acuidade(false);
    da->set_nao_letal(arma.nao_letal());
    if (PossuiTalento("acuidade_arma", proto) &&
        bba_distancia > bba_cac &&
        (PossuiCategoria(CAT_LEVE, arma) ||
         arma.id() == "sabre" || arma.id() == "chicote" || arma.id() == "corrente_com_cravos")) {
      da->set_acuidade(true);
      AtribuiBonus(-penalidade_ataque_escudo, TB_PENALIDADE_ESCUDO, "escudo", bonus_ataque);
    }
    da->set_requer_carregamento(arma.carregamento().requer_carregamento());

    // Aplica diferenca de tamanho de arma.
    int tamanho = proto.tamanho();
    if (da->has_tamanho()) {
      tamanho += (da->tamanho() - proto.tamanho());
    }
    if (tamanho < 0) tamanho = 0;
    if (tamanho > TM_COLOSSAL) tamanho = TM_COLOSSAL;
    if (da->empunhadura() == EA_MAO_RUIM && PossuiCategoria(CAT_ARMA_DUPLA, arma) && arma.has_dano_secundario()) {
      da->set_dano_basico(DanoBasicoPorTamanho(static_cast<TamanhoEntidade>(tamanho), arma.dano_secundario()));
      da->set_margem_critico(arma.margem_critico_secundario());
      da->set_multiplicador_critico(arma.multiplicador_critico_secundario());
    } else if (arma.has_dano()) {
      da->set_dano_basico(DanoBasicoPorTamanho(static_cast<TamanhoEntidade>(tamanho), arma.dano()));
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
  } else if (da->ataque_agarrar()) {
    if (!da->has_dano_basico()) {
      da->set_dano_basico(DanoDesarmadoPorTamanho(proto.tamanho()));
    }
    if (PossuiTalento("agarrar_aprimorado", proto) || da->acao().ignora_ataque_toque()) {
      da->set_ignora_ataque_toque(true);
    } else {
      da->clear_ignora_ataque_toque();
    }
  }
  if (da->has_dano_basico_fixo()) {
    da->set_dano_basico(da->dano_basico_fixo());
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
    if (primeiro->has_alinhamento()) da->set_alinhamento(primeiro->alinhamento());
    else da->clear_alinhamento();
  }
  // Descritores de ataque.

  da->clear_descritores();
  if (da->material_arma() != DESC_NENHUM) da->add_descritores(da->material_arma());
  if (da->alinhamento() != DESC_NENHUM) da->add_descritores(da->alinhamento());
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
  } else if (da->ataque_corpo_a_corpo()) {
    bba = da->acuidade() ? bba_distancia : bba_cac;
    usar_forca_dano = true;
  } else if (da->ataque_toque()) {
    bba = PossuiTalento("acuidade_arma", proto) ? std::max(bba_distancia, bba_cac) : bba_cac;
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
  da->set_ca_normal(CATotal(proto, permite_escudo));
  da->set_ca_surpreso(CASurpreso(proto, permite_escudo));
  da->set_ca_toque(CAToque(proto));

  // Veneno.
  RecomputaDependenciasVenenoParaAtaque(proto, da);

  VLOG(1) << "Ataque recomputado: " << da->DebugString();
}

void RecomputaDependenciasDadosAtaque(const Tabelas& tabelas, EntidadeProto* proto) {
  // Remove ataques cujo numero de vezes exista e seja zero.
  std::vector<int> indices_a_remover;
  for (int i = proto->dados_ataque().size() - 1; i >= 0; --i) {
    const auto& da = proto->dados_ataque(i);
    if (da.has_limite_vezes() && da.limite_vezes() <= 0) {
      indices_a_remover.push_back(i);
    }
  }
  for (int indice : indices_a_remover) {
    proto->mutable_dados_ataque()->DeleteSubrange(indice, 1);
  }

  // Se nao tiver agarrar, cria um.
  if (std::none_of(proto->dados_ataque().begin(), proto->dados_ataque().end(),
        [] (const DadosAtaque& da) { return da.ataque_agarrar(); })) {
    auto* da = proto->mutable_dados_ataque()->Add();
    da->set_tipo_ataque("Agarrar");
    da->set_rotulo("agarrar");
  }

  for (auto& da : *proto->mutable_dados_ataque()) {
    RecomputaDependenciasArma(tabelas, *proto, &da);
  }
}

}  // namespace

void RecomputaDependencias(const Tabelas& tabelas, EntidadeProto* proto) {
  VLOG(2) << "Proto antes RecomputaDependencias: " << proto->ShortDebugString();
  ResetComputados(proto);

  RecomputaDependenciasRaciais(tabelas, proto);
  RecomputaDependenciasItensMagicos(tabelas, proto);
  RecomputaDependenciasTendencia(proto);
  RecomputaDependenciasEfeitos(tabelas, proto);
  RecomputaDependenciasNiveisNegativos(tabelas, proto);
  RecomputaDependenciasDestrezaLegado(tabelas, proto);
  RecomputaDependenciasClasses(tabelas, proto);
  RecomputaDependenciasTalentos(tabelas, proto);
  RecomputaDependenciaTamanho(proto);
  RecomputaDependenciasPontosVidaTemporarios(proto);
  RecomputaDependenciasPontosVida(proto);

  int modificador_destreza           = ModificadorAtributo(proto->atributos().destreza());
  const int modificador_constituicao = ModificadorAtributo(proto->atributos().constituicao());
  //const int modificador_inteligencia = ModificadorAtributo(BonusTotal(proto->atributos().inteligencia()));
  const int modificador_sabedoria    = ModificadorAtributo(proto->atributos().sabedoria());
  //const int modificador_carisma      = ModificadorAtributo(BonusTotal(proto->atributos().carisma()));

  // Iniciativa.
  RecomputaDependenciasIniciativa(modificador_destreza, proto);

  // CA.
  RecomputaDependenciasCA(tabelas, proto);
  // Salvacoes.
  RecomputaDependenciasSalvacoes(modificador_constituicao, modificador_destreza, modificador_sabedoria, tabelas, proto);
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
  RecomputaDependenciasMagiasPorDia(tabelas, proto);

  VLOG(2) << "Proto depois RecomputaDependencias: " << proto->ShortDebugString();
}

}  // namespace ent
