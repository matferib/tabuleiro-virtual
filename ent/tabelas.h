#ifndef ENT_TABELAS_H
#define ENT_TABELAS_H

#include <unordered_map>
#include "ent/tabelas.pb.h"

namespace ent {

// Classe que gerencia as tabelas.
class Tabelas {
 public:
  // Carrega as tabelas.
  Tabelas();

  const TodasTabelas& todas() const { return tabelas_; }

  const ArmaduraOuEscudo& Armadura(const std::string& id) const;
  const ArmaduraOuEscudo& Escudo(const std::string& id) const;
  //const Arma& Arma(const std::string& id) const;

 private:
  TodasTabelas tabelas_;
  std::unordered_map<std::string, const ArmaduraOuEscudo*> armaduras_;
  std::unordered_map<std::string, const ArmaduraOuEscudo*> escudos_;
  std::unordered_map<std::string, const Arma*> armas_;
};

}  // namespace ent

#endif
