#ifndef ENT_TABELAS_H
#define ENT_TABELAS_H

#include <optional>
#include <unordered_map>

#include "ent/acoes.pb.h"
#include "ent/tabelas.pb.h"
#include "ntf/notificacao.h"

namespace ent {

// Classe que gerencia as tabelas.
class Tabelas : public ntf::Receptor {
 public:
  // Carrega as tabelas.
  Tabelas(ntf::CentralNotificacoes* central);

  // Retorna a instancia unica da tabela.
  static const Tabelas& Unica();

  bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

  const TodasTabelas& todas() const { return tabelas_; }

  // Retorna os feiticos de uma determinada classe, por nivel.
  const std::vector<const ArmaProto*> Feiticos(const std::string& id_classe, int nivel) const;
  // Retorna id de um feitico aleatoriamente.
  struct DadosParaFeiticoAleatorio {
    std::string id_classe;
    int nivel;
    std::optional<std::vector<DescritorAtaque>> descritores_proibidos;
    std::optional<std::vector<std::string>> escolas_proibidas;
    std::vector<std::string> feiticos_excluidos;
  };
  const std::string& FeiticoAleatorio(const DadosParaFeiticoAleatorio& dfa) const;

  // As funcoes retornam a instancia padrao caso nao encontrem a chave.
  const ArmaduraOuEscudoProto& Armadura(const std::string& id) const;
  const ArmaduraOuEscudoProto& Escudo(const std::string& id) const;
  const ArmaProto& Arma(const std::string& id) const;
  const ArmaProto& Feitico(const std::string& id) const;
  enum cura_ou_infligir_e {
    COI_CURA = 0,
    COI_INFLIGIR = 1,
  };
  const std::string FeiticoConversaoEspontanea(const std::string& id_classe, int nivel, cura_ou_infligir_e cura_ou_infligir) const;
  const ArmaProto& ArmaOuFeitico(const std::string& id) const;
  const EfeitoProto& Efeito(TipoEfeito tipo) const;
  const EfeitoModeloProto& EfeitoModelo(TipoEfeitoModelo tipo) const;
  const AcaoProto& Acao(const std::string& id) const;
  const ItemMagicoProto& Pocao(const std::string& id) const;
  const ItemMagicoProto& PergaminhoArcano(const std::string& id) const;
  const ItemMagicoProto& PergaminhoDivino(const std::string& id) const;
  const ItemMagicoProto& Varinha(const std::string& id) const;
  const ItemMagicoProto& Anel(const std::string& id) const;
  const ItemMagicoProto& Manto(const std::string& id) const;
  const ItemMagicoProto& Luvas(const std::string& id) const;
  const ItemMagicoProto& Amuleto(const std::string& id) const;
  const ItemMagicoProto& Chapeu(const std::string& id) const;
  const ItemMagicoProto& Bracadeiras(const std::string& id) const;
  const ItemMagicoProto& Botas(const std::string& id) const;
  const ItemMagicoProto& ItemMundano(const std::string& id) const;
  //const Municao& Municao(const std::string& id) const;
  const TalentoProto& Talento(const std::string& id) const;
  const InfoClasse& Classe(const std::string& id) const;
  const PericiaProto& Pericia(const std::string& id) const;
  const Acoes& TodasAcoes() const { return tabela_acoes_; }
  const Modelos& TodosModelosEntidades() const { return tabela_modelos_entidades_; }
  const RacaProto& Raca(const std::string& id) const;
  const DominioProto& Dominio(const std::string& id) const;
  const Modelo& ModeloEntidade(const std::string& modelo) const;
  const VenenoProto& Veneno(const std::string& id) const;

 private:
  // Dados os protos tabelas_ e tabela_acoes_, preenche os demais mapas.
  void RecarregaMapas();

  TodasTabelas tabelas_;
  // Acoes eh um caso a parte. Ta duplicado. A ideia eh remover do tabuleiro (MapaIdAcoes) depois e deixar so na tabela.
  Acoes tabela_acoes_;
  // Os modelos de entidades.
  Modelos tabela_modelos_entidades_;

  std::unordered_map<std::string, const ArmaduraOuEscudoProto*> armaduras_;
  std::unordered_map<std::string, const ArmaduraOuEscudoProto*> escudos_;
  std::unordered_map<std::string, const ArmaProto*> armas_;
  std::unordered_map<std::string, const ArmaProto*> feiticos_;
  std::unordered_map<std::string, std::unordered_map<int, std::vector<const ArmaProto*>>> feiticos_por_classe_por_nivel_;
  std::unordered_map<int, const EfeitoProto*> efeitos_;
  std::unordered_map<int, const EfeitoModeloProto*> efeitos_modelos_;
  std::unordered_map<std::string, const ItemMagicoProto*> pocoes_;
  // Pergaminhos sao gerados, entao nao tem uma tabela pra apontar (a tabela eh so pra merge).
  std::unordered_map<std::string, ItemMagicoProto> pergaminhos_arcanos_;
  std::unordered_map<std::string, ItemMagicoProto> pergaminhos_divinos_;
  std::unordered_map<std::string, const ItemMagicoProto*> aneis_;
  std::unordered_map<std::string, const ItemMagicoProto*> itens_mundanos_;
  std::unordered_map<std::string, const ItemMagicoProto*> varinhas_;
  //std::unordered_map<std::string, const Municao*> municoes_;
  std::unordered_map<std::string, const ItemMagicoProto*> mantos_;
  std::unordered_map<std::string, const ItemMagicoProto*> luvas_;
  std::unordered_map<std::string, const ItemMagicoProto*> bracadeiras_;
  std::unordered_map<std::string, const ItemMagicoProto*> amuletos_;
  std::unordered_map<std::string, const ItemMagicoProto*> chapeus_;
  std::unordered_map<std::string, const ItemMagicoProto*> botas_;
  std::unordered_map<std::string, const TalentoProto*> talentos_;
  std::unordered_map<std::string, const PericiaProto*> pericias_;
  std::unordered_map<std::string, const InfoClasse*> classes_;
  std::unordered_map<std::string, const RacaProto*> racas_;
  std::unordered_map<std::string, const AcaoProto*> acoes_;
  std::unordered_map<std::string, const DominioProto*> dominios_;
  std::unordered_map<std::string, const Modelo*> modelos_entidades_;
  std::unordered_map<std::string, const VenenoProto*> venenos_;

  ntf::CentralNotificacoes* central_;
};

}  // namespace ent

#endif
