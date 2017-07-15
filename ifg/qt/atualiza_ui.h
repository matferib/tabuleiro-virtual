#ifndef IFG_QT_ATUALIZA_UI_H
#define IFG_QT_ATUALIZA_UI_H

#include "ent/tabelas.h"
#include "ifg/qt/bonus_util.h"
#include "ifg/qt/constantes.h"
#include "ifg/qt/evento_util.h"
#include "ifg/qt/ui/entidade.h"
#include "ifg/qt/util.h"
#include "ifg/qt/visualizador3d.h"

//--------------------------------------------------------------------------------------------------
// As funcoes AtualizaUI* atualizam uma parte especifica da UI. Elas nao chamam dependencias
// pois sabem apenas o que fazer em uma parte. Normalmente, apos se atualizar algum campo, chama-se
// ent::RecomputaDependencias e as AtualizaUI apropriadas.
//--------------------------------------------------------------------------------------------------

namespace ifg {
namespace qt {

// Atualiza a UI de atributos.
void AtualizaUIAtributos(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

void LimpaCamposAtaque(ifg::qt::Ui::DialogoEntidade& gerador);

// Refresca a lista de ataques toda e atualiza os controles de acordo com a linha selecionada.
void AtualizaUIAtaque(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

void AtualizaUIDefesa(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Atualiza a UI de ataque e defesa baseada no proto.
void AtualizaUIAtaquesDefesa(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

// Atualiza UI de iniciativa.
void AtualizaUIIniciativa(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

void AtualizaUISalvacoes(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto);

}  // namespace qt
}  // namespace ifg

#endif
