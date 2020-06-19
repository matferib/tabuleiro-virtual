#ifndef IFG_QT_ATUALIZA_UI_H
#define IFG_QT_ATUALIZA_UI_H

#include "ifg/qt/ui/entidade.h"
#include "ifg/qt/ui/forma.h"

//--------------------------------------------------------------------------------------------------
// As funcoes AtualizaUI* atualizam uma parte especifica da UI. Elas nao chamam dependencias
// pois sabem apenas o que fazer em uma parte. Normalmente, apos se atualizar algum campo, chama-se
// ent::RecomputaDependencias e as AtualizaUI apropriadas.
//--------------------------------------------------------------------------------------------------

namespace ent {
class EntidadeProto;
class InfoClasse;
class Tabelas;
}  // namespace ent

namespace ifg {
namespace qt {

// Chama todas atualizacoes de UI.
void AtualizaUI(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Atualiza combos de evasao.
void AtualizaUIEvasao(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);
void AtualizaUIEsquivaSobrenatural(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Atualiza sliders de tendencia.
void AtualizaUITendencia(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Atualiza a UI de classes e niveis.
void AtualizaUIClassesNiveis(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Converte as salvacoes para um id de combo de salvacoes.
int SalvacoesFortesParaIndice(const ent::InfoClasse& ic);

// Atualiza a UI de atributos.
void AtualizaUIAtributos(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Atualiza a UI de movimentacao.
void AtualizaUIMovimento(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Refresca a lista de ataques toda e atualiza os controles de acordo com a linha selecionada.
void AtualizaUIAtaque(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

void AtualizaUIDefesa(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Atualiza a UI de ataque e defesa baseada no proto.
void AtualizaUIAtaquesDefesa(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Atualiza UI de iniciativa.
void AtualizaUIIniciativa(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

void AtualizaUISalvacoes(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Lista de formas alternativas.
void AtualizaUIFormasAlternativas(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Atualiza a UI de tesouros: pocoes e lista.
void AtualizaUITesouro(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);
void AtualizaUITesouro(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoForma& gerador, const ent::EntidadeProto& proto);

// Pontos de vida, max e temporarios.
void AtualizaUIPontosVida(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Atualiza a tabela de pericias.
void AtualizaUIPericias(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Tipo de elemento da UI de feitico.
enum TipoItemFeitico {
  // Filhos sao feiticos conhecidos.
  RAIZ_CONHECIDO = 0,
  // Filhos sao feiticos para lancar.
  RAIZ_PARA_LANCAR = 1,
  // Um feitico conhecido.
  CONHECIDO = 2,
  // Um feitico para lancar (slot).
  PARA_LANCAR = 3,
};

// Cada coluna dos item widgets tem um significado no Qt::USerRole.
enum TipoDadoColuna {
  // Indica se o item eh feitico para lancar ou conhecido.
  TCOL_CONHECIDO_OU_PARA_LANCAR = 0,
  // Id da classe.
  TCOL_ID_CLASSE = 1,
  // Nivel do feitico.
  TCOL_NIVEL = 2,
  // Indice do feitico.
  TCOL_INDICE = 3,
  // Para lancar, nivel do conhecido.
  TCOL_NIVEL_CONHECIDO = 4,
  // Para lancar, indice do conhecido.
  TCOL_INDICE_CONHECIDO = 5,
  // Para lancar, usado.
  TCOL_USADO = 6,
  // Identificador do feitico, se houver.
  TCOL_ID_FEITICO = 7,
  TCOL_NOME_FEITICO = 8,
};
// Atualiza a lista de feiticos da classe.
void AtualizaUIFeiticos(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);
// Atualiza os feiticos conhecidos de um nivel.
void AtualizaFeiticosConhecidosNivel(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador,
    const std::string& id_classe, int nivel,
    const ent::EntidadeProto& proto, QTreeWidgetItem* pai);
void AdicionaItemFeiticoConhecido(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador,
    const std::string& id, const std::string& nome, const std::string& id_classe, int nivel, int indice,
    const ent::EntidadeProto& proto_retornado,
    QTreeWidgetItem* pai);

// Atualiza os feiticos para lancar de um nivel.
void AtualizaCombosParaLancar(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador,
    const std::string& id_classe, const ent::EntidadeProto& proto);
// Essa versao eh usada para ajustar os slots que apontavam para feiticos conhecidos do mesmo nivel,
// mas apos ou igual o indice.
void AtualizaCombosParaLancarAposRemocao(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador,
    const std::string& id_classe, int nivel, int indice, const ent::EntidadeProto& proto);

}  // namespace qt
}  // namespace ifg

#endif
