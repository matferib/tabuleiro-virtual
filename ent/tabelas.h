#include "ent/tabelas.pb.h"

namespace ent {

// Classe que gerencia as tabelas.
class Tabelas {
 public:
  // Carrega as tabelas.
  Tabelas();

  const ArmaduraOuEscudo& Armadura(const std::string& id) const;
  const ArmaduraOuEscudo& Escudo(const std::string& id) const;
  const Arma& Arma(const std::string& id) const;

 private:
  TodasTabelas tabelas_;
  std::unordered_map<std::string, const ArmaduraOuEscudo&> armaduras_;
  std::unordered_map<std::string, const ArmaduraOuEscudo&> escudos_;
  std::unordered_map<std::string, const Armas&> armas_;
};

}  // namespace ent
