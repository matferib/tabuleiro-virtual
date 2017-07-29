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
class Tabelas;
}  // namespace ent

namespace ifg {
namespace qt {

// Chama todas atualizacoes de UI.
void AtualizaUI(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Atualiza a UI de classes e niveis.
void AtualizaUIClassesNiveis(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

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

}  // namespace qt
}  // namespace ifg

#endif
