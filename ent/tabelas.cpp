#include <algorithm>
#include <unordered_map>
#include "arq/arquivo.h"
#include "ent/acoes.pb.h"
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
  try {
    arq::LeArquivoAsciiProto(arq::TIPO_DADOS, "tabelas_nao_srd.asciiproto", &tabelas_);
  } catch (const arq::ParseProtoException & e) {
    LOG(WARNING) << "Erro lendo tabela: tabelas_nao_srd.asciiproto: " << e.what();
    central->AdicionaNotificacao(ntf::NovaNotificacaoErro(
        StringPrintf("Erro lendo tabela: tabelas_nao_srd.asciiproto: %s", e.what())));
  } catch (const std::exception& e) {
    LOG(WARNING) << "Erro lendo tabela: tabelas_nao_srd.asciiproto: " << e.what();
  }
  try {
    TodasTabelas tabelas_padroes;
    arq::LeArquivoAsciiProto(arq::TIPO_DADOS, "tabelas.asciiproto", &tabelas_padroes);
    tabelas_.MergeFrom(tabelas_padroes);
  } catch (const arq::ParseProtoException& e) {
    LOG(WARNING) << "Erro lendo tabela: tabelas.asciiproto: " << e.what();
    central->AdicionaNotificacao(ntf::NovaNotificacaoErro(
        StringPrintf("Erro lendo tabela: tabelas.asciiproto: %s", e.what())));
  } catch (const std::exception& e) {
    LOG(ERROR) << "Erro lendo tabela: tabelas.asciiproto: " << e.what();
    if (central_ != nullptr) {
      central_->AdicionaNotificacao(
          ntf::NovaNotificacaoErro(
            StringPrintf("Erro lendo tabela: tabelas.asciiproto: %s", e.what())));
    }
  }
  try {
    arq::LeArquivoAsciiProto(arq::TIPO_DADOS, "acoes.asciiproto", &tabela_acoes_);
  } catch (const std::exception& e) {
    LOG(ERROR) << "Erro lendo tabela de acoes: acoes.asciiproto";
    if (central_ != nullptr) {
      central_->AdicionaNotificacao(
          ntf::NovaNotificacaoErro(
            google::protobuf::StringPrintf("Erro lendo tabela de acoes: acoes.asciiproto: %s", e.what())));
    }
  }
  RecarregaMapas();
}

void Tabelas::RecarregaMapas() {
  armaduras_.clear();
  escudos_.clear();
  armas_.clear();
  feiticos_.clear();
  efeitos_.clear();
  pocoes_.clear();
  aneis_.clear();
  mantos_.clear();
  luvas_.clear();
  bracadeiras_.clear();
  talentos_.clear();
  pericias_.clear();
  classes_.clear();
  acoes_.clear();

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
    ConverteDano(&arma);
    armas_[arma.id()] = &arma;
  }
  for (auto& feitico : *tabelas_.mutable_tabela_feiticos()->mutable_armas()) {
    if (feitico.nome().empty()) {
      feitico.set_nome(feitico.id());
    }
    feiticos_[feitico.id()] = &feitico;
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
    pocoes_[pocao.id()] = &pocao;
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

  for (auto& bracadeiras : *tabelas_.mutable_tabela_bracadeiras()->mutable_bracadeiras()) {
    bracadeiras.set_tipo(TIPO_BRACADEIRAS);
    bracadeiras_[bracadeiras.id()] = &bracadeiras;
  }

  for (const auto& talento : tabelas_.tabela_talentos().talentos()) {
    talentos_[talento.id()] = &talento;
  }

  for (const auto& efeito : tabelas_.tabela_efeitos().efeitos()) {
    efeitos_[efeito.id()] = &efeito;
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

const AcaoProto& Tabelas::Acao(const std::string& id) const {
  auto it = acoes_.find(id);
  return it == acoes_.end() ? AcaoProto::default_instance() : *it->second;
}

const ItemMagicoProto& Tabelas::Pocao(const std::string& id) const {
  auto it = pocoes_.find(id);
  return it == pocoes_.end() ? ItemMagicoProto::default_instance() : *it->second;
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

const ItemMagicoProto& Tabelas::Amuleto(const std::string& id) const {
  auto it = amuletos_.find(id);
  return it == amuletos_.end() ? ItemMagicoProto::default_instance() : *it->second;
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

}  // namespace
