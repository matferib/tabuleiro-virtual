#include <algorithm>
#include <unordered_map>
#include "arq/arquivo.h"
#include "ent/entidade.pb.h"
#include "ent/tabelas.h"
#include "goog/stringprintf.h"
#include "log/log.h"

namespace ent {

namespace {

void ConverteDano(ArmaProto* arma) {
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
        LOG(ERROR) << google::protobuf::StringPrintf("Dano nao encontrado: %s", arma->dano().medio().c_str());
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
        LOG(ERROR) << google::protobuf::StringPrintf("Dano nao encontrado: %s", arma->dano_secundario().medio().c_str());
      }
      return;
    }
    if (it->second.find(TM_PEQUENO) != it->second.end()) arma->mutable_dano_secundario()->set_pequeno(it->second.find(TM_PEQUENO)->second);
    if (it->second.find(TM_GRANDE) != it->second.end()) arma->mutable_dano_secundario()->set_grande(it->second.find(TM_GRANDE)->second);
  }
}

}  // namespace

Tabelas::Tabelas() {
  try {
    arq::LeArquivoAsciiProto(arq::TIPO_DADOS, "tabelas.asciiproto", &tabelas_);
  } catch (...) {
    LOG(WARNING) << "Erro lendo tabela: tabelas.asciiproto";
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
    ConverteDano(&arma);
    armas_[arma.id()] = &arma;
  }
  {
    const ArmaProto& arco_curto_composto = Arma("arco_curto_composto");
    for (int i = 1; i < 10; ++i) {
      auto* novo_arco = tabelas_.mutable_tabela_armas()->add_armas();
      *novo_arco = arco_curto_composto;
      novo_arco->set_id(google::protobuf::StringPrintf("%s_%d", arco_curto_composto.id().c_str(), i));
      novo_arco->set_preco(google::protobuf::StringPrintf("%d PO", (i * 75) + 75));
      armas_[novo_arco->id()] = novo_arco;
    }
  }
  {
    const ArmaProto& arco_longo_composto = Arma("arco_longo_composto");
    for (int i = 1; i < 10; ++i) {
      auto* novo_arco = tabelas_.mutable_tabela_armas()->add_armas();
      *novo_arco = arco_longo_composto;
      novo_arco->set_id(google::protobuf::StringPrintf("%s_%d", arco_longo_composto.id().c_str(), i));
      novo_arco->set_preco(google::protobuf::StringPrintf("%d PO", (i * 100) + 100));
      armas_[novo_arco->id()] = novo_arco;
    }
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

}  // namespace
