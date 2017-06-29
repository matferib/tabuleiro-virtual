#include "ent/tabelas.h"

namespace ent {

Tabelas::Tabelas() {
  try {
    arq::LeArquivoAsciiProto(arq::TIPO_DADOS, "tabelas.asciiproto", &tabelas_);
  } catch (...) {
    LOG(WARNING) << "Erro lendo tabela: tabelas.asciiproto";
  }
  for (const auto& armadura : tabelas_.tabela_armaduras().armaduras()) {
    armaduras_.insert(std::make_pair(armadura.id(), armadura));
  }
  for (const auto& escudo : tabelas_.tabela_escudos().escudos()) {
    escudos_.insert(std::make_pair(escudo.id(), escudo));
  }
}

const ArmaduraOuEscudo& Tabelas::Armadura(const std::string& id) const {
  auto it = armaduras_.find(id);
  return it == armaduras.end() ? ArmaduraOuEscudo::default_instance() : it->second;
}

const EscudoOuEscudo& Tabelas::Escudo(const std::string& id) const {
  auto it = escudos_.find(id);
  return it == escudos.end() ? ArmaduraOuEscudo::default_instance() : it->second;
}

}  // namespace
