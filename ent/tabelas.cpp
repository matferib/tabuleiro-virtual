#include <algorithm>
#include <unordered_map>
#include "arq/arquivo.h"
#include "ent/acoes.pb.h"
#include "ent/constantes.h"
#include "ent/entidade.pb.h"
#include "ent/tabelas.h"
#include "ent/util.h"
#include "goog/stringprintf.h"
#include "log/log.h"

namespace ent {

namespace {
using google::protobuf::StringPrintf;

void ConverteDano(ArmaProto* arma) {
  if (PossuiCategoria(CAT_PROJETIL_AREA, *arma)) {
    arma->mutable_dano()->set_pequeno(arma->dano().medio());
    arma->mutable_dano()->set_grande(arma->dano().medio());
    return;
  }

  // Chaveado por dano medio.
  const static std::unordered_map<std::string, std::unordered_map<int, std::string>> mapa_danos {
    { "1d2", {
      { TM_PEQUENO, "1" }, { TM_GRANDE, "1d3" }, { TM_ENORME, "1d4" }, { TM_IMENSO, "1d6" }, { TM_COLOSSAL, "1d8" }
    } },
    { "1d3", {
      { TM_MIUDO, "1" }, { TM_PEQUENO, "1d2" }, { TM_GRANDE, "1d4" }, { TM_ENORME, "1d6" }, { TM_IMENSO, "1d8" }, { TM_COLOSSAL, "2d6" }
    } },
    { "1d4", {
      { TM_DIMINUTO, "1" }, { TM_MIUDO, "1d2" }, { TM_PEQUENO, "1d3" }, { TM_GRANDE, "1d6" }, { TM_ENORME, "1d8" }, { TM_IMENSO, "2d6" },
      { TM_COLOSSAL, "3d6" }
    } },
    { "1d6", {
      { TM_MINUSCULO, "1" }, { TM_DIMINUTO, "1d2" },  { TM_MIUDO, "1d3" }, { TM_PEQUENO, "1d4" }, { TM_GRANDE, "1d8" }, { TM_ENORME, "2d6" },
      { TM_IMENSO, "3d6" }, { TM_COLOSSAL, "4d6" }
    } },
    { "1d8", {
      { TM_MINUSCULO, "1d2" }, { TM_DIMINUTO, "1d3" },  { TM_MIUDO, "1d4" }, { TM_PEQUENO, "1d6" },
      { TM_GRANDE, "2d6" }, { TM_ENORME, "3d6" }, { TM_IMENSO, "4d6" }, { TM_COLOSSAL, "6d6" }
    } },
    { "1d10", {
      { TM_MINUSCULO, "1d3" }, { TM_DIMINUTO, "1d4" },  { TM_MIUDO, "1d6" }, { TM_PEQUENO, "1d8" },
      { TM_GRANDE, "2d8" }, { TM_ENORME, "3d8" }, { TM_IMENSO, "4d8" }, { TM_COLOSSAL, "6d8" }
    } },
    { "1d12", {
      { TM_MINUSCULO, "1d4" }, { TM_DIMINUTO, "1d6" },  { TM_MIUDO, "1d8" }, { TM_PEQUENO, "1d10" },
      { TM_GRANDE, "3d6" }, { TM_ENORME, "4d6" }, { TM_IMENSO, "6d6" }, { TM_COLOSSAL, "8d6" }
    } },
    { "2d4", {
      { TM_MINUSCULO, "1d2" }, { TM_DIMINUTO, "1d3" },  { TM_MIUDO, "1d4" }, { TM_PEQUENO, "1d6" },
      { TM_GRANDE, "2d6" }, { TM_ENORME, "3d6" }, { TM_IMENSO, "4d6" }, { TM_COLOSSAL, "6d6" }
    } },
    { "2d6", {
      { TM_MINUSCULO, "1d4" }, { TM_DIMINUTO, "1d6" },  { TM_MIUDO, "1d8" }, { TM_PEQUENO, "1d10" },
      { TM_GRANDE, "3d6" }, { TM_ENORME, "4d6" }, { TM_IMENSO, "6d6" }, { TM_COLOSSAL, "8d6" }
    } },
    { "2d8", {
      { TM_MINUSCULO, "1d6" }, { TM_DIMINUTO, "1d8" },  { TM_MIUDO, "1d10" }, { TM_PEQUENO, "2d6" },
      { TM_GRANDE, "3d8" }, { TM_ENORME, "4d8" }, { TM_IMENSO, "6d8" }, { TM_COLOSSAL, "8d8" }
    } },
    { "2d10", {
      { TM_MINUSCULO, "1d8" }, { TM_DIMINUTO, "1d10" },  { TM_MIUDO, "2d6" }, { TM_PEQUENO, "2d8" },
      { TM_GRANDE, "4d8" }, { TM_ENORME, "6d8" }, { TM_IMENSO, "8d8" }, { TM_COLOSSAL, "12d8" }
    } }
  };
  {
    auto it = mapa_danos.find(arma->dano().medio());
    if (it == mapa_danos.end()) {
      if (!arma->dano().medio().empty()) {
        LOG(ERROR) << StringPrintf("Dano nao encontrado: %s", arma->dano().medio().c_str());
      }
      return;
    }
    if (it->second.find(TM_PEQUENO) != it->second.end()) arma->mutable_dano()->set_pequeno(it->second.find(TM_PEQUENO)->second);
    if (it->second.find(TM_GRANDE) != it->second.end()) arma->mutable_dano()->set_grande(it->second.find(TM_GRANDE)->second);
  }
  {
    // dano secundario.
    auto it = mapa_danos.find(arma->dano_secundario().medio());
    if (it == mapa_danos.end()) {
      if (!arma->dano_secundario().medio().empty()) {
        LOG(ERROR) << StringPrintf("Dano nao encontrado: %s", arma->dano_secundario().medio().c_str());
      }
      return;
    }
    if (it->second.find(TM_PEQUENO) != it->second.end()) arma->mutable_dano_secundario()->set_pequeno(it->second.find(TM_PEQUENO)->second);
    if (it->second.find(TM_GRANDE) != it->second.end()) arma->mutable_dano_secundario()->set_grande(it->second.find(TM_GRANDE)->second);
  }
}

}  // namespace

Tabelas::Tabelas(ntf::CentralNotificacoes* central) : central_(central) {
  if (central_ != nullptr) {
    central_->RegistraReceptor(this);
  }

  std::vector<const char*> arquivos_tabelas = {"tabelas_nao_srd.asciiproto", "tabelas_homebrew.asciiproto", "tabelas.asciiproto"};
  tabelas_.Clear();
  // Tabelas.
  for (const char* arquivo : arquivos_tabelas) {
    try {
      TodasTabelas tabelas;
      arq::LeArquivoAsciiProto(arq::TIPO_DADOS, arquivo, &tabelas);
      tabelas_.MergeFrom(tabelas);
    } catch (const arq::ParseProtoException& e) {
      LOG(WARNING) << "Erro lendo tabela: " << arquivo << ": " << e.what();
      central->AdicionaNotificacao(ntf::NovaNotificacaoErro(
          StringPrintf("Erro lendo tabela: %s: %s", arquivo, e.what())));
    } catch (const std::exception& e) {
      LOG(ERROR) << "Erro lendo tabela: " << arquivo << ": " << e.what();
      if (central_ != nullptr) {
        central_->AdicionaNotificacao(
            ntf::NovaNotificacaoErro(
              StringPrintf("Erro lendo tabela: %s: %s", arquivo, e.what())));
      }
    }
  }
  // Acoes.
  try {
    arq::LeArquivoAsciiProto(arq::TIPO_DADOS, "acoes.asciiproto", &tabela_acoes_);
  } catch (const arq::ParseProtoException& ppe) {
    LOG(ERROR) << "Erro lendo tabela de acoes: acoes.asciiproto: " << ppe.what();
    if (central_ != nullptr) {
      central_->AdicionaNotificacao(
          ntf::NovaNotificacaoErro(
            StringPrintf("Erro lendo tabela de acoes: acoes.asciiproto: %s", ppe.what())));
    }
  } catch (const std::exception& e) {
    LOG(ERROR) << "Erro lendo tabela de acoes: acoes.asciiproto";
    if (central_ != nullptr) {
      central_->AdicionaNotificacao(
          ntf::NovaNotificacaoErro(
            StringPrintf("Erro lendo tabela de acoes: acoes.asciiproto: %s", e.what())));
    }
  }
  // Modelos de entidades.
  tabela_modelos_entidades_.Clear();
  std::vector<const char*> arquivos_modelos= {ARQUIVO_MODELOS, ARQUIVO_MODELOS_NAO_SRD};
  for (const char* arquivo : arquivos_modelos) {
    try {
      Modelos modelos_arquivo;
      arq::LeArquivoAsciiProto(arq::TIPO_DADOS, arquivo, &modelos_arquivo);
      tabela_modelos_entidades_.MergeFrom(modelos_arquivo);
    } catch (const arq::ParseProtoException& e) {
      LOG(WARNING) << "Erro lendo modelo: " << arquivo << ": " << e.what();
      central->AdicionaNotificacao(ntf::NovaNotificacaoErro(
          StringPrintf("Erro lendo modelo: %s: %s", arquivo, e.what())));
    } catch (const std::exception& e) {
      LOG(ERROR) << "Erro lendo modelo: " << arquivo << ": " << e.what();
      if (central_ != nullptr) {
        central_->AdicionaNotificacao(
            ntf::NovaNotificacaoErro(
              StringPrintf("Erro lendo modelo: %s: %s", arquivo, e.what())));
      }
    }
  }

  RecarregaMapas();
}

// Quando nao houver origem no item, usa o id dele como a origem do efeito.
// So funcionara mesmo quando o id do item for igual ao do feitico.
void AdicionaOrigemImplicita(ItemMagicoProto* item) {
  if (item->tipo_efeito().size() == 1 && item->origens().empty()) {
    item->add_origens(item->id());
  }
}

// Retorna o custo padrao de um pergaminho de um feitico do nivel passado.
int CustoPadraoPergaminhoNivel(int nivel) {
  switch (nivel) {
    case 0: return 12;
    case 1: return 25;
    case 2: return 150;
    case 3: return 375;
    case 4: return 700;
    case 5: return 1125;
    case 6: return 1650;
    case 7: return 2275;
    case 8: return 3000;
    case 9: return 3825;
    default: ;
  }
  LOG(WARNING) << "retornando custo 0 para nivel: " << nivel;
  return 0;
}

int NivelConjuradorMinimoParaFeiticoNivel(const std::string& id_classe, int nivel) {
  if (nivel < 0) return 1;
  if (id_classe == "bardo") {
    switch (nivel) {
      case 0: return 1;
      case 1: return 2;
      case 2: return 4;
      case 3: return 7;
      case 4: return 10;
      case 5: return 13;
      default: return 16;
    }
  }
  if (id_classe == "ranger" || id_classe == "paladino") {
    // Nivel de conjurador de ranger eh metade do nivel. Como so comeca no 4o, o minimo eh 2.
    switch (nivel) {
      case 0:
      case 1: return 2;
      case 2: return 4;
      case 3: return 5;
      default: return 7;
    }
  }
  return std::max(1, (nivel * 2) - 1);
}

void Tabelas::RecarregaMapas() {
  armaduras_.clear();
  escudos_.clear();
  armas_.clear();
  feiticos_.clear();
  feiticos_por_classe_por_nivel_.clear();
  efeitos_.clear();
  efeitos_modelos_.clear();
  pocoes_.clear();
  pergaminhos_arcanos_.clear();
  pergaminhos_divinos_.clear();
  aneis_.clear();
  mantos_.clear();
  luvas_.clear();
  bracadeiras_.clear();
  amuletos_.clear();
  chapeus_.clear();
  botas_.clear();
  talentos_.clear();
  pericias_.clear();
  classes_.clear();
  acoes_.clear();
  racas_.clear();
  dominios_.clear();
  modelos_entidades_.clear();

  for (const auto& dominio : tabelas_.tabela_dominios().dominios()) {
    dominios_[dominio.id()] = &dominio;
  }

  for (const auto& raca : tabelas_.tabela_racas().racas()) {
    racas_[raca.id()] = &raca;
  }

  for (const auto& armadura : tabelas_.tabela_armaduras().armaduras()) {
    armaduras_[armadura.id()] = &armadura;
  }
  for (const auto& escudo : tabelas_.tabela_escudos().escudos()) {
    escudos_[escudo.id()] = &escudo;
  }
  for (auto& arma : *tabelas_.mutable_tabela_armas()->mutable_armas()) {
    if (arma.nome().empty()) {
      arma.set_nome(arma.id());
    }
    if (std::any_of(arma.categoria().begin(), arma.categoria().end(),
          [] (int c) { return c == CAT_ARCO || c == CAT_ARREMESSO; })) {
      arma.add_categoria(CAT_DISTANCIA);
    }
    if (std::any_of(arma.categoria().begin(), arma.categoria().end(),
          [] (int c) { return c == CAT_ARMA_DUPLA; })) {
      arma.add_categoria(CAT_DUAS_MAOS);
    }
    arma.add_categoria(CAT_ARMA);
    ConverteDano(&arma);
    armas_[arma.id()] = &arma;
  }
  const std::vector<std::string> classes_arcanas = {"mago", "bardo"};
  const std::vector<std::string> classes_divinas = {"druida", "clerigo", "paladino", "ranger"};
  for (auto& feitico : *tabelas_.mutable_tabela_feiticos()->mutable_armas()) {
    if (feitico.nome().empty()) {
      feitico.set_nome(feitico.id());
    }
    if (feitico.has_acao()) {
      for (auto& ea : *feitico.mutable_acao()->mutable_efeitos_adicionais()) {
        ea.set_origem(feitico.id());
      }
      for (auto& ea : *feitico.mutable_acao()->mutable_efeitos_adicionais_se_salvou()) {
        ea.set_origem(feitico.id());
      }
    }
    feiticos_[feitico.id()] = &feitico;
    for (const auto& ic : feitico.info_classes()) {
      auto& mapa_classe = feiticos_por_classe_por_nivel_[ic.id()];
      mapa_classe[ic.nivel()].push_back(&feitico);
    }
    for (const auto& ic : feitico.info_classes()) {
      ItemMagicoProto* pergaminho = nullptr;
      if (c_any(classes_arcanas, ic.id())) {
        pergaminho = &pergaminhos_arcanos_[feitico.id()];
      } else if (c_any(classes_divinas, ic.id())) {
        pergaminho = &pergaminhos_divinos_[feitico.id()];
      } else {
        continue;
      }
      pergaminho->set_id(feitico.id());
      pergaminho->set_nome(feitico.nome());
      const int custo_po = CustoPadraoPergaminhoNivel(ic.nivel());
      if (!pergaminho->has_custo_po() || pergaminho->custo_po() > custo_po) {
        pergaminho->set_custo_po(custo_po);
      }
      const int nivel_conjurador = NivelConjuradorMinimoParaFeiticoNivel(ic.id(), ic.nivel());
      if (!pergaminho->has_nivel_conjurador() || pergaminho->nivel_conjurador() > nivel_conjurador) {
        pergaminho->set_nivel_conjurador(nivel_conjurador);
        pergaminho->set_modificador_atributo(ic.nivel() / 2);
      }
    }
  }

  auto CriaArcoComposto = [this] (int i, int preco, const ArmaProto& arco_base) {
    auto* novo_arco = tabelas_.mutable_tabela_armas()->add_armas();
    *novo_arco = arco_base;
    novo_arco->set_id(google::protobuf::StringPrintf("%s_%d", arco_base.id().c_str(), i));
    novo_arco->set_nome(google::protobuf::StringPrintf("%s (%d)", arco_base.nome().c_str(), i));
    novo_arco->set_preco(google::protobuf::StringPrintf("%d PO", (i * preco) + preco));
    novo_arco->set_max_forca(i);
    armas_[novo_arco->id()] = novo_arco;
  };
  for (int i = 1; i < 10; ++i) CriaArcoComposto(i, 75, Arma("arco_curto_composto"));
  for (int i = 1; i < 10; ++i) CriaArcoComposto(i, 100, Arma("arco_longo_composto"));

  for (auto& pocao : *tabelas_.mutable_tabela_pocoes()->mutable_pocoes()) {
    pocao.set_tipo(TIPO_POCAO);
    AdicionaOrigemImplicita(&pocao);
    pocoes_[pocao.id()] = &pocao;
  }

  // Preenche a tabela de pergaminhos arcanos, mergeando o tabelado e depois regerando.
  for (auto& pergaminho : *tabelas_.mutable_tabela_pergaminhos()->mutable_pergaminhos_arcanos()) {
    pergaminho.set_tipo(TIPO_PERGAMINHO_ARCANO);
    pergaminhos_arcanos_[pergaminho.id()].MergeFrom(pergaminho);
  }
  tabelas_.mutable_tabela_pergaminhos()->clear_pergaminhos_arcanos();
  for (const auto& it : pergaminhos_arcanos_) {
    *tabelas_.mutable_tabela_pergaminhos()->add_pergaminhos_arcanos() = it.second;
  }
  // Preenche a tabela de pergaminhos divinos, mergeando o tabelado e depois regerando.
  for (auto& pergaminho : *tabelas_.mutable_tabela_pergaminhos()->mutable_pergaminhos_divinos()) {
    pergaminho.set_tipo(TIPO_PERGAMINHO_DIVINO);
    pergaminhos_divinos_[pergaminho.id()].MergeFrom(pergaminho);
  }
  tabelas_.mutable_tabela_pergaminhos()->clear_pergaminhos_divinos();
  for (const auto& it : pergaminhos_divinos_) {
    *tabelas_.mutable_tabela_pergaminhos()->add_pergaminhos_divinos() = it.second;
  }

  for (auto& anel : *tabelas_.mutable_tabela_aneis()->mutable_aneis()) {
    anel.set_tipo(TIPO_ANEL);
    aneis_[anel.id()] = &anel;
  }

  for (auto& manto : *tabelas_.mutable_tabela_mantos()->mutable_mantos()) {
    manto.set_tipo(TIPO_MANTO);
    mantos_[manto.id()] = &manto;
  }

  for (auto& luvas : *tabelas_.mutable_tabela_luvas()->mutable_luvas()) {
    luvas.set_tipo(TIPO_LUVAS);
    luvas_[luvas.id()] = &luvas;
  }

  for (auto& amuleto : *tabelas_.mutable_tabela_amuletos()->mutable_amuletos()) {
    amuleto.set_tipo(TIPO_AMULETO);
    amuletos_[amuleto.id()] = &amuleto;
  }

  for (auto& chapeu : *tabelas_.mutable_tabela_chapeus()->mutable_chapeus()) {
    chapeu.set_tipo(TIPO_CHAPEU);
    chapeus_[chapeu.id()] = &chapeu;
  }

  for (auto& bracadeiras : *tabelas_.mutable_tabela_bracadeiras()->mutable_bracadeiras()) {
    bracadeiras.set_tipo(TIPO_BRACADEIRAS);
    bracadeiras_[bracadeiras.id()] = &bracadeiras;
  }

  for (auto& botas : *tabelas_.mutable_tabela_botas()->mutable_botas()) {
    botas.set_tipo(TIPO_BOTAS);
    botas_[botas.id()] = &botas;
  }

  for (const auto& talento : tabelas_.tabela_talentos().talentos()) {
    talentos_[talento.id()] = &talento;
  }

  for (const auto& efeito : tabelas_.tabela_efeitos().efeitos()) {
    efeitos_[efeito.id()] = &efeito;
  }

  for (auto& efeito : *tabelas_.mutable_tabela_efeitos_modelos()->mutable_efeitos()) {
    if (!efeito.consequencia().dados_defesa().resistencia_elementos().empty()) {
      for (auto& re : *efeito.mutable_consequencia()->mutable_dados_defesa()->mutable_resistencia_elementos()) {
        re.set_id_efeito_modelo(efeito.id());
      }
    }
    efeitos_modelos_[efeito.id()] = &efeito;
  }

  for (const auto& classe : tabelas_.tabela_classes().info_classes()) {
    classes_[classe.id()] = &classe;
  }

  for (const auto& pericia : tabelas_.tabela_pericias().pericias()) {
    pericias_[pericia.id()] = &pericia;
  }

  for (const auto& acao : tabela_acoes_.acao()) {
    acoes_[acao.id()] = &acao;
  }

  // Modelos: tem que reconstruir os modelos compostos.
  for (const auto& modelo : tabela_modelos_entidades_.modelo()) {
    modelos_entidades_[modelo.id()] = &modelo;
  }


  // Aqui vai ser bem tosco. Enquanto houver entidade com id base, continua a saga.
  bool fim = true;
  do {
    fim =  true;
    for (auto& m : *tabela_modelos_entidades_.mutable_modelo()) {
      if (m.id_entidade_base().empty()) continue;
      // Todas as dependencias devem estar sanadas.
      bool processar = true;
      for (const auto& id_entidade_base : m.id_entidade_base()) {
        auto it = modelos_entidades_.find(id_entidade_base);
        if (it != modelos_entidades_.end() && !it->second->id_entidade_base().empty()) {
          fim = false;
          processar = false;
          LOG(INFO) << "Postergando " << m.id() << " por causa de " << id_entidade_base;
          break;
        }
      }
      if (!processar) continue;

      EntidadeProto entidade;
      for (const auto& id_entidade_base : m.id_entidade_base()) {
        auto it = modelos_entidades_.find(id_entidade_base);
        if (it == modelos_entidades_.end()) {
          LOG(ERROR) << "falha lendo id base de " << m.id() << ", base: " << id_entidade_base;
          continue;
        }
        entidade.MergeFrom(it->second->entidade());
      }
      entidade.MergeFrom(m.entidade());
      m.mutable_entidade()->Swap(&entidade);
      m.clear_id_entidade_base();
    }
  } while (!fim);
}

const ArmaduraOuEscudoProto& Tabelas::Armadura(const std::string& id) const {
  auto it = armaduras_.find(id);
  return it == armaduras_.end() ? ArmaduraOuEscudoProto::default_instance() : *it->second;
}

const ArmaduraOuEscudoProto& Tabelas::Escudo(const std::string& id) const {
  auto it = escudos_.find(id);
  return it == escudos_.end() ? ArmaduraOuEscudoProto::default_instance() : *it->second;
}

const ArmaProto& Tabelas::Arma(const std::string& id) const {
  auto it = armas_.find(id);
  return it == armas_.end() ? ArmaProto::default_instance() : *it->second;
}

const std::string Tabelas::FeiticoConversaoEspontanea(
    const std::string& id_classe, int nivel,
    cura_ou_infligir_e cura_ou_infligir) const {
  if (id_classe == "clerigo") {
    switch (nivel) {
      case 0: return cura_ou_infligir == COI_CURA ? "curar_ferimentos_minimos" : "infligir_ferimentos_minimos";
      case 1: return cura_ou_infligir == COI_CURA ? "curar_ferimentos_leves" : "infligir_ferimentos_leves";
      case 2: return cura_ou_infligir == COI_CURA ? "curar_ferimentos_moderados" : "infligir_gerimentos_moderados";
      case 3: return cura_ou_infligir == COI_CURA ? "curar_ferimentos_graves" : "infligir_ferimentos_graves";
      case 4: return cura_ou_infligir == COI_CURA ? "curar_ferimentos_criticos" : "infligir_ferimentos_criticos";
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      default: ;
    }
  }
  return "";
}

const ArmaProto& Tabelas::Feitico(const std::string& id) const {
  auto it = feiticos_.find(id);
  return it == feiticos_.end() ? ArmaProto::default_instance() : *it->second;
}

const ArmaProto& Tabelas::ArmaOuFeitico(const std::string& id) const {
  const auto& arma = Arma(id);
  return arma.has_id() ? arma : Feitico(id);
}

const EfeitoProto& Tabelas::Efeito(TipoEfeito tipo) const {
  auto it = efeitos_.find(tipo);
  return it == efeitos_.end() ? EfeitoProto::default_instance() : *it->second;
}

const EfeitoModeloProto& Tabelas::EfeitoModelo(TipoEfeitoModelo tipo) const {
  auto it = efeitos_modelos_.find(tipo);
  return it == efeitos_modelos_.end() ? EfeitoModeloProto::default_instance() : *it->second;
}

const AcaoProto& Tabelas::Acao(const std::string& id) const {
  auto it = acoes_.find(id);
  return it == acoes_.end() ? AcaoProto::default_instance() : *it->second;
}

const Modelo& Tabelas::ModeloEntidade(const std::string& id) const {
  auto it = modelos_entidades_.find(id);
  return it == modelos_entidades_.end() ? Modelo::default_instance() : *it->second;
}

const ItemMagicoProto& Tabelas::Pocao(const std::string& id) const {
  auto it = pocoes_.find(id);
  return it == pocoes_.end() ? ItemMagicoProto::default_instance() : *it->second;
}

const ItemMagicoProto& Tabelas::PergaminhoArcano(const std::string& id) const {
  auto it = pergaminhos_arcanos_.find(id);
  return it == pergaminhos_arcanos_.end() ? ItemMagicoProto::default_instance() : it->second;
}

const ItemMagicoProto& Tabelas::PergaminhoDivino(const std::string& id) const {
  auto it = pergaminhos_divinos_.find(id);
  return it == pergaminhos_divinos_.end() ? ItemMagicoProto::default_instance() : it->second;
}

const ItemMagicoProto& Tabelas::Anel(const std::string& id) const {
  auto it = aneis_.find(id);
  return it == aneis_.end() ? ItemMagicoProto::default_instance() : *it->second;
}

const ItemMagicoProto& Tabelas::Manto(const std::string& id) const {
  auto it = mantos_.find(id);
  return it == mantos_.end() ? ItemMagicoProto::default_instance() : *it->second;
}

const ItemMagicoProto& Tabelas::Luvas(const std::string& id) const {
  auto it = luvas_.find(id);
  return it == luvas_.end() ? ItemMagicoProto::default_instance() : *it->second;
}

const ItemMagicoProto& Tabelas::Botas(const std::string& id) const {
  auto it = botas_.find(id);
  return it == botas_.end() ? ItemMagicoProto::default_instance() : *it->second;
}

const ItemMagicoProto& Tabelas::Amuleto(const std::string& id) const {
  auto it = amuletos_.find(id);
  return it == amuletos_.end() ? ItemMagicoProto::default_instance() : *it->second;
}

const ItemMagicoProto& Tabelas::Chapeu(const std::string& id) const {
  auto it = chapeus_.find(id);
  return it == chapeus_.end() ? ItemMagicoProto::default_instance() : *it->second;
}

const ItemMagicoProto& Tabelas::Bracadeiras(const std::string& id) const {
  auto it = bracadeiras_.find(id);
  return it == bracadeiras_.end() ? ItemMagicoProto::default_instance() : *it->second;
}

const TalentoProto& Tabelas::Talento(const std::string& id) const {
  auto it = talentos_.find(id);
  return it == talentos_.end() ? TalentoProto::default_instance() : *it->second;
}

const InfoClasse& Tabelas::Classe(const std::string& id) const {
  auto it = classes_.find(id);
  return it == classes_.end() ? InfoClasse::default_instance() : *it->second;
}

const RacaProto& Tabelas::Raca(const std::string& id) const {
  auto it = racas_.find(id);
  return it == racas_.end() ? RacaProto::default_instance() : *it->second;
}

const DominioProto& Tabelas::Dominio(const std::string& id) const {
  auto it = dominios_.find(id);
  return it == dominios_.end() ? DominioProto::default_instance() : *it->second;
}

const PericiaProto& Tabelas::Pericia(const std::string& id) const {
  auto it = pericias_.find(id);
  return it == pericias_.end() ? PericiaProto::default_instance() : *it->second;
}

bool Tabelas::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_ENVIAR_IDS_TABELAS_TEXTURAS_E_MODELOS_3D: {
      // Cliente enviando requisicao de tabelas.
      // É possivel?
      if (!notificacao.local()) return false;
      VLOG(1) << "Enviando requisicao TN_REQUISITAR_TABELAS para servidor";
      auto n = ntf::NovaNotificacao(ntf::TN_REQUISITAR_TABELAS);
      n->set_id_rede(notificacao.id_rede());
      central_->AdicionaNotificacaoRemota(n.release());
      return true;
    }
    case ntf::TN_REQUISITAR_TABELAS: {
      // Servidor recebendo requisicao de tabelas.
      // É possivel?
      if (notificacao.local()) return false;
      VLOG(1) << "Enviando tabelas para cliente '" << notificacao.id_rede() << "'";
      auto n = ntf::NovaNotificacao(ntf::TN_ENVIAR_TABELAS);
      *n->mutable_tabelas() = tabelas_;
      n->set_id_rede(notificacao.id_rede());
      central_->AdicionaNotificacaoRemota(n.release());
      return true;
    }
    case ntf::TN_ENVIAR_TABELAS: {
      // Cliente recebendo tabelas.
      if (notificacao.local()) return false;
      tabelas_ = notificacao.tabelas();
      RecarregaMapas();
      return true;
    }
    default:
      return false;
  }
}

const std::vector<const ArmaProto*> Tabelas::Feiticos(const std::string& id_classe, int nivel) const {
  // TODO usar o mapa de feiticos_por_classe_por_nivel.
  std::vector<const ArmaProto*> feiticos;
  for (const auto& feitico : tabelas_.tabela_feiticos().armas()) {
    for (const auto ic : feitico.info_classes()) {
      if (ic.nivel() == nivel && ic.id() == id_classe) {
        feiticos.push_back(&feitico);
        break;
      }
    }
  }
  return feiticos;
}

const std::string& Tabelas::FeiticoAleatorio(const std::string& id_classe, int nivel) const {
  auto it_classe = feiticos_por_classe_por_nivel_.find(id_classe);
  if (it_classe == feiticos_por_classe_por_nivel_.end()) return ArmaProto::default_instance().id();
  auto it_nivel = it_classe->second.find(nivel);
  if (it_nivel == it_classe->second.end()) return ArmaProto::default_instance().id();
  const auto& feiticos = it_nivel->second;
  if (feiticos.empty()) return ArmaProto::default_instance().id();
  int indice = RolaDado(feiticos.size()) - 1;
  VLOG(1) << "retornando aleatoriamente " << feiticos[indice]->id() << " para classe " << id_classe << ", nivel " << nivel;
  return feiticos[indice]->id();
}

}  // namespace
