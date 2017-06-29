#include "arq/arquivo.h"
#include "ent/tabelas.h"
#include "log/log.h"

namespace ent {

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
}

const ArmaduraOuEscudo& Tabelas::Armadura(const std::string& id) const {
  auto it = armaduras_.find(id);
  return it == armaduras_.end() ? ArmaduraOuEscudo::default_instance() : *it->second;
}

const ArmaduraOuEscudo& Tabelas::Escudo(const std::string& id) const {
  auto it = escudos_.find(id);
  return it == escudos_.end() ? ArmaduraOuEscudo::default_instance() : *it->second;
}

}  // namespace
