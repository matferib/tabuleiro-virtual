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

  // As funcoes retornam a instancia padrao caso nao encontrem a chave.
  const ArmaduraOuEscudoProto& Armadura(const std::string& id) const;
  const ArmaduraOuEscudoProto& Escudo(const std::string& id) const;
  const ArmaProto& Arma(const std::string& id) const;
  const EfeitoProto& Efeito(TipoEvento tipo) const;
  const AcaoProto& Acao(const std::string& id) const;
  const Acoes& TodasAcoes() const { return tabela_acoes_; }

 private:
  TodasTabelas tabelas_;
  std::unordered_map<std::string, const ArmaduraOuEscudoProto*> armaduras_;
  std::unordered_map<std::string, const ArmaduraOuEscudoProto*> escudos_;
  std::unordered_map<std::string, const ArmaProto*> armas_;
  std::unordered_map<int, const EfeitoProto*> efeitos_;

  // Acoes eh um caso a parte. Ta duplicado. A ideia eh remover do tabuleiro (MapaIdAcoes) depois e deixar so na tabela.
  Acoes tabela_acoes_;
  std::unordered_map<std::string, const AcaoProto*> acoes_;
};

}  // namespace ent

#endif
