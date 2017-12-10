#ifndef IFG_QT_ATUALIZA_UI_H
#define IFG_QT_ATUALIZA_UI_H

#include "ifg/qt/ui/entidade.h"

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

// Atualiza sliders de tendencia.
void AtualizaUITendencia(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Atualiza a UI de classes e niveis.
void AtualizaUIClassesNiveis(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Converte as salvacoes para um id de combo de salvacoes.
int SalvacoesFortesParaIndice(const ent::InfoClasse& ic);

// Atualiza a UI de atributos.
void AtualizaUIAtributos(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

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

// Pontos de vida, max e temporarios.
void AtualizaUIPontosVida(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Atualiza a tabela de pericias.
void AtualizaUIPericias(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Atualiza a lista de feiticos da classe.
void AtualizaUIFeiticos(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);
// Atualiza os feiticos de um nivel para uma classe.
void AtualizaFeiticosNivel(
    ifg::qt::Ui::DialogoEntidade& gerador, int nivel, const std::string& id_classe, const ent::EntidadeProto& proto, QTreeWidgetItem* item);
void AdicionaItemFeitico(
    ifg::qt::Ui::DialogoEntidade& gerador, const std::string& nome, const std::string& id_classe, int nivel, int slot, bool memorizado,
    QTreeWidgetItem* pai);

}  // namespace qt
}  // namespace ifg

#endif
