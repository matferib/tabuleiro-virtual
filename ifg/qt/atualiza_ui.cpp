#include "ifg/qt/atualiza_ui.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QCheckbox>
#include <unordered_set>
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "ent/entidade.pb.h"
#include "ent/constantes.h"
#include "ent/tabelas.h"
#include "ent/util.h"
#include "ifg/qt/constantes.h"
#include "ifg/qt/evento_util.h"
#include "ifg/qt/feiticos_util.h"
#include "ifg/qt/itens_magicos_util.h"
#include "ifg/qt/pericias_util.h"
#include "ifg/qt/util.h"
#include "log/log.h"
#include "net/util.h"

namespace ifg {
namespace qt {

using google::protobuf::RepeatedPtrField;

void AtualizaUIEventos(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  auto* model = qobject_cast<ModeloEvento*>(gerador.tabela_lista_eventos->model());
  if (model == nullptr) return;
  gerador.tabela_lista_eventos->blockSignals(true);
  model->AtualizaModelo(proto);
  gerador.tabela_lista_eventos->blockSignals(false);
}

void AtualizaUIMovimento(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  const auto& mov = proto.movimento();
  std::vector<std::tuple<QSpinBox*, int, QPushButton*, const ent::Bonus*>> tuplas = {
    std::make_tuple(gerador.spin_mov_terrestre, mov.terrestre_basico_q(),  gerador.botao_mov_terrestre, &proto.movimento().terrestre_q()),
    std::make_tuple(gerador.spin_mov_aereo,     mov.aereo_basico_q(),      gerador.botao_mov_aereo,     &proto.movimento().aereo_q()),
    std::make_tuple(gerador.spin_mov_nadando,   mov.aquatico_basico_q(),   gerador.botao_mov_nadando,   &proto.movimento().aquatico_q()),
    std::make_tuple(gerador.spin_mov_escavando, mov.escavando_basico_q(),  gerador.botao_mov_escavando, &proto.movimento().escavando_q()),
    std::make_tuple(gerador.spin_mov_escalando, mov.escalando_basico_q(),  gerador.botao_mov_escalando, &proto.movimento().escalando_q()),
  };

  for (const auto& t : tuplas) {
    QSpinBox* spin;
    int basico;
    QPushButton* botao;
    const ent::Bonus* bonus;
    std::tie(spin, basico, botao, bonus) = t;
    spin->blockSignals(true);
    spin->setValue(basico);
    spin->blockSignals(false);
    botao->blockSignals(true);
    botao->setText(QString::number(BonusTotal(*bonus)));
    botao->blockSignals(false);
  }
}

int TipoEvasaoParaIndiceCombo(ent::TipoEvasao te) {
  // TODO tratar com switch?
  return (int)te;
}

void AtualizaUIEvasao(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  gerador.combo_evasao_estatica->blockSignals(true);
  gerador.combo_evasao_dinamica->blockSignals(true);
  gerador.combo_evasao_estatica->setCurrentIndex(TipoEvasaoParaIndiceCombo(proto.dados_defesa().evasao_estatica()));
  gerador.combo_evasao_dinamica->setCurrentIndex(TipoEvasaoParaIndiceCombo(proto.dados_defesa().evasao()));
  gerador.combo_evasao_dinamica->blockSignals(false);
  gerador.combo_evasao_estatica->blockSignals(false);
}

int TipoEsquivaSobrenaturalParaIndiceCombo(const ent::EntidadeProto& proto) {
  if (PossuiHabilidadeEspecial("esquiva_sobrenatural_aprimorada", proto)) {
    return 2;
  } else if (PossuiHabilidadeEspecial("esquiva_sobrenatural", proto)) {
    return 1;
  }
  return 0;
}

void AtualizaUIEsquivaSobrenatural(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  gerador.combo_esquiva_sobrenatural->setEnabled(false);
  gerador.combo_esquiva_sobrenatural->setCurrentIndex(TipoEsquivaSobrenaturalParaIndiceCombo(proto));
}

void AtualizaUI(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  AtualizaUIClassesNiveis(tabelas, gerador, proto);
  AtualizaUIAtributos(tabelas, gerador, proto);
  AtualizaUIMovimento(tabelas, gerador, proto);
  AtualizaUIAtaquesDefesa(tabelas, gerador, proto);
  AtualizaUIIniciativa(tabelas, gerador, proto);
  AtualizaUISalvacoes(gerador, proto);
  AtualizaUITesouro(tabelas, gerador, proto);
  AtualizaUIPontosVida(gerador, proto);
  AtualizaUIPericias(tabelas, gerador, proto);
  AtualizaUIFeiticos(tabelas, gerador, proto);
  AtualizaUIEventos(tabelas, gerador, proto);
  AtualizaUIEvasao(tabelas, gerador, proto);
  AtualizaUIEsquivaSobrenatural(tabelas, gerador, proto);
}

int SalvacoesFortesParaIndice(const ent::InfoClasse& ic) {
  bool fortitude_forte = ClassePossuiSalvacaoForte(ent::TS_FORTITUDE, ic);
  bool reflexo_forte = ClassePossuiSalvacaoForte(ent::TS_REFLEXO, ic);
  bool vontade_forte = ClassePossuiSalvacaoForte(ent::TS_VONTADE, ic);
  if (fortitude_forte && reflexo_forte && vontade_forte) {
    return 6;
  } else if (reflexo_forte && vontade_forte) {
    return 5;
  } else if (fortitude_forte && vontade_forte) {
    return 4;
  } else if (fortitude_forte && reflexo_forte) {
    return 3;
  } else if (vontade_forte) {
    return 2;
  } else if (reflexo_forte) {
    return 1;
  } else {
    return 0;
  }
}

namespace {

QString NumeroSinalizado(int valor) {
  QString ret = QString::number(valor);
  if (valor > 0) ret.prepend("+");
  return ret;
}

std::string StringSalvacoesFortes(const ent::InfoClasse& ic) {
  std::string salvacoes_fortes;
  if (ClassePossuiSalvacaoForte(ent::TS_FORTITUDE, ic)) {
    salvacoes_fortes += "F";
  }
  if (ClassePossuiSalvacaoForte(ent::TS_REFLEXO, ic)) {
    salvacoes_fortes += "R";
  }
  if (ClassePossuiSalvacaoForte(ent::TS_VONTADE, ic)) {
    salvacoes_fortes += "V";
  }
  return salvacoes_fortes;
}

// Atualiza a UI com a lista de niveis e os totais.
void AtualizaUINiveis(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  // nivel total.
  int total = 0;
  int total_bba = 0;
  for (const auto& info_classe : proto.info_classes()) {
    total += info_classe.nivel();
    total_bba += info_classe.bba();
  }
  gerador.linha_nivel->setText(QString::number(total));
  gerador.linha_bba->setText(QString::number(total_bba));

  // Lista de niveis.
  const int indice_antes = gerador.lista_niveis->currentRow();
  gerador.lista_niveis->clear();
  for (const auto& ic : proto.info_classes()) {
    std::string string_nivel;
    absl::StrAppendFormat(&string_nivel, "classe: %s, nível: %d", ic.id().c_str(), ic.nivel());
    if (ic.nivel_conjurador() > 0) {
      absl::StrAppendFormat(
          &string_nivel, ", conjurador: %d, mod (%s): %d",
          ic.nivel_conjurador(), TipoAtributo_Name(ic.atributo_conjuracao()).substr(3, 3).c_str(),
          ic.modificador_atributo_conjuracao());
    }
    absl::StrAppendFormat(&string_nivel, ", BBA: %d, Salv Fortes: %s", ic.bba(), StringSalvacoesFortes(ic).c_str());
    const auto& classe_tabelada = tabelas.Classe(ic.id());
    if (classe_tabelada.possui_dominio()) {
      const auto& fc = ent::FeiticosClasse(ic.id(), proto);
      if (fc.dominios_size() == 2) {
        absl::StrAppendFormat(&string_nivel, ", dominios: %s, %s", tabelas.Dominio(fc.dominios(0)).nome().c_str(), tabelas.Dominio(fc.dominios(1)).nome().c_str());
      } else {
        LOG(ERROR) << "dominios de tamanho errado";
      }
    }
    if (classe_tabelada.id() == "mago") {
      const auto& fc = ent::FeiticosClasse(ic.id(), proto);
      if (!fc.especializacao().empty()) {
        absl::StrAppendFormat(
            &string_nivel, ", especialização: %s, %s",
            fc.especializacao().c_str(),
            fc.escolas_proibidas().size() != 2
              ? "faltando escolas proibidas"
              : absl::StrFormat("escolas proibidas: %s, %s", fc.escolas_proibidas(0).c_str(), fc.escolas_proibidas(1).c_str()).c_str());
      } else {
        absl::StrAppendFormat(&string_nivel, ", sem especialização");
      }
    }
    gerador.lista_niveis->addItem(QString::fromUtf8(string_nivel.c_str()));
  }
  if (indice_antes < proto.info_classes().size()) {
    gerador.lista_niveis->setCurrentRow(indice_antes);
  } else {
    gerador.lista_niveis->setCurrentRow(-1);
  }
}

void SelecionaIndicePorId(const std::string& id, QComboBox* combo) {
  combo->setCurrentIndex(combo->findData(id.c_str()));
}

}  // namespace

void AtualizaUITendencia(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  gerador.slider_bem_mal->setValue(proto.tendencia().eixo_bem_mal() * 8);
  gerador.slider_ordem_caos->setValue(proto.tendencia().eixo_ordem_caos() * 8);
}

void AtualizaUIClassesNiveis(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  // Objetos da UI a serem bloqueados. Passa por copia.
  std::vector<QObject*> objs = {
      gerador.spin_niveis_negativos, gerador.spin_nivel_classe, gerador.spin_nivel_conjurador, gerador.linha_classe, gerador.spin_bba,
      gerador.combo_mod_conjuracao, gerador.lista_niveis, gerador.combo_salvacoes_fortes, gerador.combo_classe, gerador.combo_raca,
      gerador.combo_dominio_1, gerador.combo_dominio_2, gerador.combo_especializacao_escola, gerador.combo_escola_proibida_1, gerador.combo_escola_proibida_2
  };
  auto BloqueiaSinais = [objs] {
    for (auto* obj : objs) obj->blockSignals(true);
  };
  auto DesbloqueiaSinais = [objs] {
    for (auto* obj : objs) obj->blockSignals(false);
  };

  BloqueiaSinais();
  AtualizaUINiveis(tabelas, gerador, proto);
  gerador.spin_niveis_negativos->setValue(ent::BonusIndividualTotal(ent::TB_BASE, proto.niveis_negativos_dinamicos()));

  const int indice = gerador.lista_niveis->currentRow();
  // Se tiver selecao, preenche.
  if (indice >= 0 && indice < proto.info_classes_size()) {
    const auto& info_classe = proto.info_classes(indice);
    gerador.botao_remover_nivel->setEnabled(true);
    SelecionaIndicePorId(info_classe.id(), gerador.combo_classe);
    gerador.linha_classe->setText(QString::fromUtf8(info_classe.id().c_str()));
    gerador.spin_nivel_classe->setValue(info_classe.nivel());
    gerador.spin_nivel_conjurador->setValue(info_classe.nivel_conjurador());
    gerador.spin_bba->setValue(info_classe.bba());
    gerador.combo_mod_conjuracao->setCurrentIndex(info_classe.atributo_conjuracao());
    gerador.label_mod_conjuracao->setText(NumeroSinalizado(info_classe.modificador_atributo_conjuracao()));
    gerador.combo_salvacoes_fortes->setCurrentIndex(SalvacoesFortesParaIndice(info_classe));
    const auto& classe_tabelada = tabelas.Classe(info_classe.id());
    if (classe_tabelada.possui_dominio()) {
      gerador.combo_dominio_1->setEnabled(true);
      gerador.combo_dominio_2->setEnabled(true);
      const auto& fc = ent::FeiticosClasse(classe_tabelada.id(), proto);
      std::string dominio_1 = fc.dominios().size() == 2 ? fc.dominios(0) : "nenhum";
      SelecionaIndicePorId(dominio_1, gerador.combo_dominio_1);
      gerador.combo_dominio_1->setToolTip(gerador.combo_dominio_1->tr(tabelas.Dominio(dominio_1).descricao().c_str()));
      std::string dominio_2 = fc.dominios().size() == 2 ? fc.dominios(1) : "nenhum";
      SelecionaIndicePorId(dominio_2, gerador.combo_dominio_2);
      gerador.combo_dominio_2->setToolTip(gerador.combo_dominio_2->tr(tabelas.Dominio(dominio_2).descricao().c_str()));
    } else {
      gerador.combo_dominio_1->setEnabled(false);
      gerador.combo_dominio_2->setEnabled(false);
      SelecionaIndicePorId("nenhum", gerador.combo_dominio_1);
      gerador.combo_dominio_1->setToolTip("");
      SelecionaIndicePorId("nenhum", gerador.combo_dominio_2);
      gerador.combo_dominio_2->setToolTip("");
    }
    if (classe_tabelada.id() == "mago") {
      const auto& fc = ent::FeiticosClasse(classe_tabelada.id(), proto);
      gerador.combo_especializacao_escola->setEnabled(true);
      SelecionaIndicePorId(fc.especializacao().empty() ? "nenhuma" : fc.especializacao(), gerador.combo_especializacao_escola);
      const bool habilitar = gerador.combo_especializacao_escola->itemData(gerador.combo_especializacao_escola->currentIndex()).toString().toStdString() != "nenhuma";
      gerador.combo_escola_proibida_1->setEnabled(habilitar);
      gerador.combo_escola_proibida_2->setEnabled(habilitar);
      SelecionaIndicePorId(fc.escolas_proibidas().size() < 1 ? "nenhuma" : fc.escolas_proibidas(0), gerador.combo_escola_proibida_1);
      SelecionaIndicePorId(fc.escolas_proibidas().size() < 2 ? "nenhuma" : fc.escolas_proibidas(1), gerador.combo_escola_proibida_2);
    } else {
      gerador.combo_especializacao_escola->setEnabled(false);
      gerador.combo_escola_proibida_1->setEnabled(false);
      gerador.combo_escola_proibida_2->setEnabled(false);
    }
  }

  // Override de coisas tabeladas, independente de selecao.
  const auto& classe_tabelada =
      tabelas.Classe(gerador.combo_classe->itemData(gerador.combo_classe->currentIndex()).toString().toStdString());
  bool habilitar = !classe_tabelada.has_nome();
  gerador.linha_classe->setEnabled(habilitar);
  gerador.spin_bba->setEnabled(habilitar);
  gerador.combo_mod_conjuracao->setEnabled(habilitar);
  gerador.spin_nivel_conjurador->setEnabled(habilitar);
  gerador.combo_salvacoes_fortes->setEnabled(habilitar);
  if (classe_tabelada.has_nome()) {
    gerador.linha_classe->setText(QString::fromUtf8(classe_tabelada.id().c_str()));
    gerador.combo_mod_conjuracao->setCurrentIndex(classe_tabelada.atributo_conjuracao());
    gerador.combo_salvacoes_fortes->setCurrentIndex(SalvacoesFortesParaIndice(classe_tabelada));
  }

  gerador.combo_raca->setCurrentIndex(gerador.combo_raca->findData(QVariant(proto.raca().c_str())));
  DesbloqueiaSinais();
}

void AtualizaUIAtributos(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  const auto& a = proto.atributos();
  std::vector<std::tuple<const ent::Bonus*, QSpinBox*, QPushButton*, QLabel*>> tuplas = {
    std::make_tuple(&a.forca(),        gerador.spin_forca,        gerador.botao_bonus_forca,        gerador.label_mod_forca),
    std::make_tuple(&a.destreza(),     gerador.spin_destreza,     gerador.botao_bonus_destreza,     gerador.label_mod_destreza),
    std::make_tuple(&a.constituicao(), gerador.spin_constituicao, gerador.botao_bonus_constituicao, gerador.label_mod_constituicao),
    std::make_tuple(&a.inteligencia(), gerador.spin_inteligencia, gerador.botao_bonus_inteligencia, gerador.label_mod_inteligencia),
    std::make_tuple(&a.sabedoria(),    gerador.spin_sabedoria,    gerador.botao_bonus_sabedoria,    gerador.label_mod_sabedoria),
    std::make_tuple(&a.carisma(),      gerador.spin_carisma,      gerador.botao_bonus_carisma,      gerador.label_mod_carisma),
  };
  for (const auto& t : tuplas) {
    const ent::Bonus* bonus;
    QSpinBox* spin;
    QPushButton* botao;
    QLabel* label;
    std::tie(bonus, spin, botao, label) = t;
    spin->blockSignals(true);
    int bonus_total = ent::BonusTotal(*bonus);
    if (!PossuiBonus(ent::TB_BASE, *bonus)) {
      bonus_total += 10;
      spin->setValue(10);
    } else {
      spin->setValue(ent::BonusIndividualTotal(ent::TB_BASE, *bonus));
    }
    spin->blockSignals(false);
    botao->setText(QString::number(bonus_total));
    label->setText(NumeroSinalizado(ent::ModificadorAtributo(*bonus)));
    if (label == gerador.label_mod_destreza) {
      if (ent::BonusIndividualTotal(ent::TB_ARMADURA, proto.atributos().destreza()) < 0) {
        label->setStyleSheet("color: red;");
      } else {
        label->setStyleSheet("");
      }
    }
  }
}

namespace {

void LimpaCamposAtaque(ifg::qt::Ui::DialogoEntidade& gerador) {
  gerador.botao_ataque_cima->setEnabled(false);
  gerador.botao_ataque_baixo->setEnabled(false);
  gerador.botao_clonar_ataque->setEnabled(false);

  gerador.combo_empunhadura->setCurrentIndex(0);
  gerador.checkbox_op->setCheckState(Qt::Unchecked);
  gerador.checkbox_ignora_rm->setCheckState(Qt::Unchecked);
  gerador.checkbox_permite_salvacao->setCheckState(Qt::Unchecked);
  gerador.checkbox_ataque_agarrar->setCheckState(Qt::Unchecked);
  gerador.checkbox_ataque_toque->setCheckState(Qt::Unchecked);
  gerador.botao_bonus_ataque->setText("0");
  gerador.botao_bonus_dano->setText("0");
  gerador.spin_bonus_magico->setValue(0);
  gerador.spin_municao->setValue(0);
  gerador.spin_limite_vezes->setValue(0);
  gerador.linha_dano->clear();
  gerador.spin_alcance_quad->setValue(0);
}

void PreencheComboArma(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const std::string& tipo_ataque, const ent::EntidadeProto& proto) {
  const bool cac = tipo_ataque == "Ataque Corpo a Corpo" || tipo_ataque.empty();
  const bool projetil_area = tipo_ataque == "Projétil de Área";
  const bool distancia = tipo_ataque == "Ataque a Distância";
  const bool feitico_de = tipo_ataque.find("Feitiço de ") == 0;
  const bool pergaminho = tipo_ataque.find("Pergaminho") == 0;
  const bool varinha = tipo_ataque.find("Varinha") == 0;
  std::map<std::string, std::string> nome_id_map;
  if (cac || projetil_area || distancia) {
    for (const auto& arma_tesouro : proto.tesouro().armas()) {
      // Arma do personagem.
      const auto& arma_tabelada = tabelas.Arma(arma_tesouro.id_tabela());
      const bool arma_projetil_area = ent::PossuiCategoria(ent::CAT_PROJETIL_AREA, arma_tabelada);
      if ((cac && ent::PossuiCategoria(ent::CAT_CAC, arma_tabelada)) ||
          (projetil_area && arma_projetil_area) ||
          (distancia && !arma_projetil_area && ent::PossuiCategoria(ent::CAT_DISTANCIA, arma_tabelada))) {
        nome_id_map[absl::StrFormat(" do equipamento: %s", arma_tesouro.nome().c_str())] = absl::StrFormat("equipamento:%s", arma_tesouro.id().c_str());
      }
    }
    for (const auto& arma : tabelas.todas().tabela_armas().armas()) {
      const bool arma_projetil_area = ent::PossuiCategoria(ent::CAT_PROJETIL_AREA, arma);
      if ((cac && ent::PossuiCategoria(ent::CAT_CAC, arma)) ||
          (projetil_area && arma_projetil_area) ||
          (distancia && !arma_projetil_area && ent::PossuiCategoria(ent::CAT_DISTANCIA, arma))) {
        nome_id_map[arma.nome()] = arma.id();
      }
    }
  } else if (feitico_de) {
    std::string id_classe = TipoAtaqueParaClasse(tabelas, tipo_ataque);
    const int nivel_para_conjuracao = NivelParaCalculoMagiasPorDia(tabelas, id_classe, proto);
    const int nivel_maximo_feitico = NivelMaximoFeitico(tabelas, id_classe, nivel_para_conjuracao);
    for (const auto& feitico : tabelas.todas().tabela_feiticos().armas()) {
      if (PodeConjurarFeitico(feitico, nivel_maximo_feitico, IdParaMagia(tabelas, id_classe))) {
        nome_id_map[feitico.nome()] = feitico.id();
      }
    }
  } else if (pergaminho || varinha) {
    for (const auto& feitico : tabelas.todas().tabela_feiticos().armas()) {
      nome_id_map[feitico.nome()] = feitico.id();
    }
  }
  gerador.combo_arma->clear();
  gerador.combo_arma->addItem("Nenhuma", QVariant("nenhuma"));
  for (const auto& [name, id] : nome_id_map) {
    gerador.combo_arma->addItem(QString::fromUtf8(name.c_str()), QVariant(id.c_str()));
  }
}

int MaterialArmaParaIndice(ent::DescritorAtaque descritor) {
  switch (descritor) {
    case ent::DESC_ADAMANTE: return 1;
    case ent::DESC_FERRO_FRIO: return 2;
    case ent::DESC_MADEIRA_NEGRA: return 3;
    case ent::DESC_MITRAL: return 4;
    case ent::DESC_PRATA_ALQUIMICA: return 5;
    default: return 0;
  }
}

int MaterialArmaduraParaIndice(ent::DescritorAtaque descritor) {
  switch (descritor) {
    case ent::DESC_ADAMANTE: return 1;
    case ent::DESC_COURO_DRAGAO: return 2;
    case ent::DESC_MITRAL: return 3;
    default: return 0;
  }
}

int MaterialEscudoParaIndice(ent::DescritorAtaque descritor) {
  switch (descritor) {
    case ent::DESC_ADAMANTE: return 1;
    case ent::DESC_COURO_DRAGAO: return 2;
    case ent::DESC_MADEIRA_NEGRA: return 3;
    case ent::DESC_MITRAL: return 4;
    default: return 0;
  }
}

// retorna o identificador que vai pro QVariant do item do combo.
std::string IdArmaParaCombo(const ent::DadosAtaque& da) {
  if (da.id_arma_tesouro().empty()) {
    return da.id_arma();
  } else {
    return absl::StrCat(kPrefixoEquipamento, da.id_arma_tesouro().c_str());
  }
}

}  // namespace

void AtualizaUIAtaque(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  std::vector<QObject*> objs =
      {gerador.spin_bonus_magico, gerador.checkbox_op, gerador.checkbox_ignora_rm, gerador.checkbox_permite_salvacao,
       gerador.checkbox_ataque_agarrar, gerador.checkbox_ataque_toque, gerador.spin_municao,
       gerador.spin_alcance_quad, gerador.spin_incrementos, gerador.spin_limite_vezes, gerador.combo_empunhadura,
       gerador.combo_tipo_ataque, gerador.linha_dano, gerador.linha_grupo_ataque, gerador.linha_rotulo_ataque, gerador.lista_ataques,
       gerador.combo_arma, gerador.spin_ordem_ataque, gerador.combo_material_arma,
       gerador.spin_nivel_conjurador_pergaminho, gerador.spin_modificador_atributo_pergaminho };
  for (auto* obj : objs) obj->blockSignals(true);

  // Tem que vir antes do clear.
  const int linha = gerador.lista_ataques->currentRow();
  auto lista_itens = gerador.lista_ataques->selectedItems();
  std::unordered_set<int> selecionados;
  for (const auto* item : lista_itens) {
    selecionados.insert(gerador.lista_ataques->row(item));
  }

  gerador.lista_ataques->clear();
  for (const auto& da : proto.dados_ataque()) {
    gerador.lista_ataques->addItem(QString::fromUtf8(ent::StringResumoArma(tabelas, da).c_str()));
  }
  // Restaura a linha.
  gerador.lista_ataques->setCurrentRow(linha);
  for (int indice : selecionados) {
    if (indice == -1 || indice >= proto.dados_ataque().size()) continue;
    gerador.lista_ataques->item(indice)->setSelected(true);
  }

  // BBA.
  gerador.label_bba_base->setText(QString::number(proto.bba().base()));
  gerador.label_bba_base->setToolTip(QString::fromUtf8(proto.bba().base_detalhes().c_str()));
  gerador.label_bba_agarrar->setText(QString::number(proto.bba().agarrar()));
  gerador.label_bba_agarrar->setToolTip(QString::fromUtf8(proto.bba().agarrar_detalhes().c_str()));
  gerador.label_bba_cac->setText(QString::number(proto.bba().cac()));
  gerador.label_bba_cac->setToolTip(QString::fromUtf8(proto.bba().cac_detalhes().c_str()));
  gerador.label_bba_distancia->setText(QString::number(proto.bba().distancia()));
  gerador.label_bba_distancia->setToolTip(QString::fromUtf8(proto.bba().distancia_detalhes().c_str()));


  const bool linha_valida = linha >= 0 && linha < proto.dados_ataque_size();
  const auto& tipo_ataque = linha_valida ? proto.dados_ataque(linha).tipo_ataque() : CurrentData(gerador.combo_tipo_ataque).toString().toStdString();
  const bool pergaminho = tipo_ataque.find("Pergaminho") == 0;
  gerador.combo_arma->setEnabled(
      tipo_ataque == "Ataque Corpo a Corpo" || tipo_ataque == "Ataque a Distância" || tipo_ataque == "Projétil de Área" ||
      tipo_ataque.find("Feitiço de ") == 0 || pergaminho);
  PreencheComboArma(tabelas, gerador, tipo_ataque, proto);
  gerador.spin_nivel_conjurador_pergaminho->setEnabled(pergaminho);
  gerador.spin_modificador_atributo_pergaminho->setEnabled(pergaminho);

  gerador.combo_material_arma->setEnabled(
      tipo_ataque == "Ataque Corpo a Corpo" || tipo_ataque == "Ataque a Distância");
  if (!linha_valida) {
    LimpaCamposAtaque(gerador);
    gerador.botao_remover_ataque->setEnabled(!proto.dados_ataque().empty());
    for (auto* obj : objs) obj->blockSignals(false);
    return;
  }
  const auto& da = proto.dados_ataque(linha);
  const bool do_equipamento = !da.id_arma_tesouro().empty();
  std::string id_arma_combo = IdArmaParaCombo(da);
  std::string id_arma_tabela = do_equipamento ? ent::ArmaPersonagem(da.id_arma_tesouro(), proto).id_tabela() : da.id_arma();
  gerador.combo_arma->setCurrentIndex(id_arma_combo.empty() ? 0 : gerador.combo_arma->findData(QVariant(id_arma_combo.c_str())));
  gerador.combo_material_arma->setCurrentIndex(MaterialArmaParaIndice(da.material_arma()));
  gerador.botao_remover_ataque->setEnabled(true);
  gerador.linha_grupo_ataque->setText(QString::fromUtf8(da.grupo().c_str()));
  gerador.linha_rotulo_ataque->setText(QString::fromUtf8(da.rotulo().c_str()));
  const auto& tipo_str = da.tipo_ataque();
  gerador.combo_tipo_ataque->setCurrentIndex(gerador.combo_tipo_ataque->findData(tipo_str.c_str()));
  gerador.linha_dano->setText(QString::fromUtf8(ent::StringDanoBasicoComCritico(da).c_str()));
  gerador.linha_dano->setEnabled(!tabelas.ArmaOuFeitico(id_arma_tabela).has_dano());
  gerador.spin_incrementos->setValue(da.incrementos());
  gerador.spin_alcance_quad->setValue(ent::METROS_PARA_QUADRADOS * (da.has_alcance_m() ? da.alcance_m() : -1.5f));
  gerador.checkbox_op->setCheckState(da.obra_prima() ? Qt::Checked : Qt::Unchecked);
  gerador.checkbox_op->setEnabled(!do_equipamento);
  gerador.checkbox_ignora_rm->setCheckState(da.acao_fixa().ignora_resistencia_magia() ? Qt::Checked : Qt::Unchecked);
  gerador.checkbox_permite_salvacao->setCheckState(da.acao_fixa().permite_salvacao() ? Qt::Checked : Qt::Unchecked);
  gerador.checkbox_ataque_agarrar->setCheckState(da.acao_fixa().ataque_agarrar() ? Qt::Checked : Qt::Unchecked);
  gerador.checkbox_ataque_toque->setCheckState(da.acao_fixa().ataque_toque() ? Qt::Checked : Qt::Unchecked);
  gerador.combo_empunhadura->setCurrentIndex(da.empunhadura());
  gerador.spin_bonus_magico->setValue(ent::BonusIndividualPorOrigem(ent::TB_MELHORIA, "arma_magica", da.bonus_ataque()));
  gerador.spin_bonus_magico->setEnabled(!do_equipamento);
  gerador.spin_municao->setValue(da.municao());
  gerador.spin_limite_vezes->setValue(da.limite_vezes());
  if (pergaminho) {
    gerador.spin_nivel_conjurador_pergaminho->setValue(da.nivel_conjurador_pergaminho());
    gerador.spin_modificador_atributo_pergaminho->setValue(da.modificador_atributo_pergaminho());
  }
  // A ordem eh indexada em 0, mas usuarios entendem primeiro como 1.
  gerador.spin_ordem_ataque->setValue(da.ordem_ataque() + 1);

  gerador.botao_bonus_ataque->setText(QString::number(ent::BonusTotal(da.bonus_ataque())));
  gerador.botao_bonus_dano->setText(QString::number(ent::BonusTotal(da.bonus_dano())));
  gerador.botao_clonar_ataque->setEnabled(true);
  if (proto.dados_ataque().size() > 1) {
    gerador.botao_ataque_cima->setEnabled(true);
    gerador.botao_ataque_baixo->setEnabled(true);
  }
  for (auto* obj : objs) obj->blockSignals(false);
}

void AtualizaUIDefesa(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  const auto& dd = proto.dados_defesa();
  // combo armadura.
  for (int i = 0; i < gerador.combo_armadura->count(); ++i) {
    QVariant dados_armadura = gerador.combo_armadura->itemData(i);
    if ((dd.id_armadura_tesouro().empty() && dados_armadura.toString().toStdString() == dd.id_armadura()) ||
        dados_armadura.toString().toStdString() == absl::StrFormat("equipamento:%s", dd.id_armadura_tesouro().c_str())) {
      gerador.combo_armadura->blockSignals(true);
      gerador.combo_armadura->setCurrentIndex(i);
      gerador.combo_armadura->blockSignals(false);
      break;
    }
  }
  gerador.combo_material_armadura->setCurrentIndex(MaterialArmaduraParaIndice(dd.material_armadura()));
  // combo escudo.
  for (int i = 0; dd.has_id_escudo() && i < gerador.combo_escudo->count(); ++i) {
    QVariant dados_escudo = gerador.combo_escudo->itemData(i);
    if ((dd.id_escudo_tesouro().empty() && dados_escudo.toString().toStdString() == dd.id_escudo()) ||
        dados_escudo.toString().toStdString() == absl::StrFormat("equipamento:%s", dd.id_escudo_tesouro().c_str())) {
      gerador.combo_escudo->blockSignals(true);
      gerador.combo_escudo->setCurrentIndex(i);
      gerador.combo_escudo->blockSignals(false);
      break;
    }
  }
  gerador.combo_material_escudo->setCurrentIndex(MaterialEscudoParaIndice(dd.material_escudo()));
  const auto& ca = dd.ca();
  gerador.botao_bonus_ca->setText(QString::number(BonusTotal(ca)));

  std::vector<QWidget*> objs = { gerador.spin_ca_armadura_melhoria, gerador.spin_ca_escudo_melhoria, gerador.checkbox_armadura_obra_prima, gerador.checkbox_escudo_obra_prima, gerador.botao_resistencia_magia };
  for (auto* obj : objs) obj->blockSignals(true);
  gerador.spin_ca_armadura_melhoria->setValue(dd.bonus_magico_armadura());
  gerador.spin_ca_escudo_melhoria->setValue(dd.bonus_magico_escudo());
  gerador.checkbox_armadura_obra_prima->setCheckState(dd.armadura_obra_prima() ? Qt::Checked : Qt::Unchecked);
  gerador.checkbox_escudo_obra_prima->setCheckState(dd.escudo_obra_prima() ? Qt::Checked : Qt::Unchecked);
  gerador.botao_resistencia_magia->setText(
      NumeroSinalizado(ent::BonusTotal(proto.dados_defesa().resistencia_magia_variavel())));
  for (auto* obj : objs) obj->blockSignals(false);
  gerador.botao_bonus_ca->setText(QString::number(ent::CATotal(proto, /*permite_escudo=*/true)));
  gerador.label_ca_toque->setText(QString::number(ent::CAToque(proto)));
  gerador.label_ca_surpreso->setText(QString::number(ent::CASurpreso(proto, /*permite_escudo=*/true)));
}

void AtualizaUIAtaquesDefesa(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  AtualizaUIAtaque(tabelas, gerador, proto);
  AtualizaUIDefesa(gerador, proto);
}

void AtualizaUIIniciativa(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  gerador.checkbox_iniciativa->setCheckState(proto.has_iniciativa() ? Qt::Checked : Qt::Unchecked);
  gerador.spin_iniciativa->setValue(proto.iniciativa());
  gerador.botao_bonus_iniciativa->setText(NumeroSinalizado(ent::BonusTotal(proto.bonus_iniciativa())));
}

void AtualizaUISalvacoes(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  const auto& dd = proto.dados_defesa();
  std::vector<std::tuple<QPushButton*, const ent::Bonus*>> tuplas = {
    std::make_tuple(gerador.botao_bonus_salvacao_fortitude, &dd.salvacao_fortitude()),
    std::make_tuple(gerador.botao_bonus_salvacao_reflexo, &dd.salvacao_reflexo()),
    std::make_tuple(gerador.botao_bonus_salvacao_vontade, &dd.salvacao_vontade()),
  };
  for (const auto& t : tuplas) {
    QPushButton* botao; const ent::Bonus* bonus;
    std::tie(botao, bonus) = t;
    botao->setText(NumeroSinalizado(ent::BonusTotal(*bonus)));
  }
}

void AtualizaUIFormasAlternativas(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  gerador.lista_formas_alternativas->blockSignals(true);
  int indice_antes = gerador.lista_formas_alternativas->currentRow();
  gerador.lista_formas_alternativas->clear();
  int i = 0;
  for (const auto& fa : proto.formas_alternativas()) {
    gerador.lista_formas_alternativas->addItem(QString::fromUtf8(
          i == proto.forma_alternativa_corrente()
          ? absl::StrFormat("%s (corrente)", fa.rotulo().c_str()).c_str()
          : absl::StrFormat("%s (secundária)", fa.rotulo().c_str()).c_str()));
    ++i;
  }
  gerador.lista_formas_alternativas->setCurrentRow(indice_antes);
  gerador.lista_formas_alternativas->blockSignals(false);
}

void AtualizaListaItemMagico(const ent::Tabelas& tabelas, ent::TipoItem tipo, QListWidget* lista, const ent::EntidadeProto& proto) {
  const int indice = lista->currentRow();
  lista->clear();
  for (const auto& item : ent::ItensProto(tipo, proto)) {
    auto* wi = new QListWidgetItem(QString::fromUtf8(
          NomeParaLista(tabelas, tipo, item).c_str()), lista);
    wi->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  }
  lista->setCurrentRow(indice);
}

enum arma_armadura_ou_escudo_e {
  ITEM_ARMA = 0,
  ITEM_ARMADURA = 1,
  ITEM_ESCUDO = 2
};

const RepeatedPtrField<ent::EntidadeProto::ArmaArmaduraOuEscudoPersonagem>& BuscaArmasArmadurasEscudos(arma_armadura_ou_escudo_e tipo, const ent::EntidadeProto& proto) {
  switch (tipo) {
    case ITEM_ARMA:
      return proto.tesouro().armas();
      break;
    case ITEM_ARMADURA:
      return proto.tesouro().armaduras();
      break;
    default:
      return proto.tesouro().escudos();
  }
}

void AtualizaListaArmaArmaduraOuEscudo(const ent::Tabelas& tabelas, arma_armadura_ou_escudo_e tipo, QListWidget* lista, QCheckBox* checkbox_op, QSpinBox* spin_bonus, QSpinBox* spin_bonus_secundario,
                                       const ent::EntidadeProto& proto) {
  const int indice = lista->currentRow();
  lista->clear();
  const auto& aaes = BuscaArmasArmadurasEscudos(tipo, proto);
  for (const auto& aae : aaes) {
    const std::string& nome = aae.nome();
    auto* wi = new QListWidgetItem(QString::fromUtf8(nome.c_str()), lista);
    wi->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  }
  lista->setCurrentRow(indice);
  if (indice < 0 || indice >= aaes.size()) {
    if (checkbox_op != nullptr) {
      checkbox_op->blockSignals(true);
      checkbox_op->setCheckState(Qt::Unchecked);
      checkbox_op->blockSignals(false);
    }
    if (spin_bonus != nullptr) {
      spin_bonus->blockSignals(true);
      spin_bonus->setValue(0);
      spin_bonus->blockSignals(false);
    }
    if (spin_bonus_secundario != nullptr) {
      spin_bonus_secundario->blockSignals(true);
      spin_bonus_secundario->setValue(0);
      spin_bonus_secundario->setEnabled(false);
      spin_bonus_secundario->blockSignals(false);
    }
    return;
  }
  if (checkbox_op != nullptr) {
    checkbox_op->blockSignals(true);
    checkbox_op->setCheckState(aaes[indice].obra_prima() ? Qt::Checked : Qt::Unchecked);
    checkbox_op->blockSignals(false);
  }
  if (spin_bonus != nullptr) {
    spin_bonus->blockSignals(true);
    spin_bonus->setValue(aaes[indice].bonus_magico());
    spin_bonus->blockSignals(false);
  }
  if (spin_bonus_secundario != nullptr) {
    spin_bonus_secundario->blockSignals(true);
    spin_bonus_secundario->setEnabled(ent::PossuiCategoria(ent::CAT_ARMA_DUPLA, tabelas.Arma(aaes[indice].id_tabela())));
    spin_bonus_secundario->setValue(aaes[indice].bonus_magico_secundario());
    spin_bonus_secundario->blockSignals(false);
  }
}

template <class Dialogo>
void AtualizaUITesouroGenerica(const ent::Tabelas& tabelas, Dialogo& gerador, const ent::EntidadeProto& proto) {
  std::vector<QWidget*> objs = {
      gerador.lista_tesouro,  gerador.lista_pocoes, gerador.lista_aneis,
      gerador.lista_mantos,   gerador.lista_luvas,  gerador.lista_bracadeiras,
      gerador.lista_amuletos, gerador.lista_botas, gerador.lista_chapeus,
      gerador.lista_pergaminhos_arcanos, gerador.lista_pergaminhos_divinos,
      gerador.spin_po, gerador.spin_pp, gerador.spin_pc, gerador.spin_pl, gerador.spin_pe,
      gerador.lista_itens_mundanos, gerador.lista_armas, gerador.lista_armaduras, gerador.lista_escudos,
  };
  for (auto* obj : objs) obj->blockSignals(true);

  // Moedas.
  gerador.spin_po->setValue(proto.tesouro().moedas().po());
  gerador.spin_pp->setValue(proto.tesouro().moedas().pp());
  gerador.spin_pc->setValue(proto.tesouro().moedas().pc());
  gerador.spin_pl->setValue(proto.tesouro().moedas().pl());
  gerador.spin_pe->setValue(proto.tesouro().moedas().pe());

  AtualizaListaItemMagico(tabelas, ent::TipoItem::TIPO_POCAO, gerador.lista_pocoes, proto);
  AtualizaListaItemMagico(tabelas, ent::TipoItem::TIPO_ANEL, gerador.lista_aneis, proto);
  AtualizaListaItemMagico(tabelas, ent::TipoItem::TIPO_LUVAS, gerador.lista_luvas, proto);
  AtualizaListaItemMagico(tabelas, ent::TipoItem::TIPO_MANTO, gerador.lista_mantos, proto);
  AtualizaListaItemMagico(tabelas, ent::TipoItem::TIPO_BRACADEIRAS, gerador.lista_bracadeiras, proto);
  AtualizaListaItemMagico(tabelas, ent::TipoItem::TIPO_AMULETO, gerador.lista_amuletos, proto);
  AtualizaListaItemMagico(tabelas, ent::TipoItem::TIPO_BOTAS, gerador.lista_botas, proto);
  AtualizaListaItemMagico(tabelas, ent::TipoItem::TIPO_CHAPEU, gerador.lista_chapeus, proto);
  AtualizaListaItemMagico(tabelas, ent::TipoItem::TIPO_PERGAMINHO_ARCANO, gerador.lista_pergaminhos_arcanos, proto);
  AtualizaListaItemMagico(tabelas, ent::TipoItem::TIPO_PERGAMINHO_DIVINO, gerador.lista_pergaminhos_divinos, proto);
  AtualizaListaItemMagico(tabelas, ent::TipoItem::TIPO_VARINHA, gerador.lista_varinhas, proto);
  AtualizaListaItemMagico(tabelas, ent::TipoItem::TIPO_ITEM_MUNDANO, gerador.lista_itens_mundanos, proto);
  AtualizaListaArmaArmaduraOuEscudo(tabelas, ITEM_ARMA, gerador.lista_armas, gerador.checkbox_arma_op, gerador.spin_bonus_arma, gerador.spin_bonus_arma_secundario, proto);
  AtualizaListaArmaArmaduraOuEscudo(tabelas, ITEM_ARMADURA, gerador.lista_armaduras, nullptr, nullptr, nullptr, proto);
  AtualizaListaArmaArmaduraOuEscudo(tabelas, ITEM_ESCUDO, gerador.lista_escudos, nullptr, nullptr, nullptr, proto);

  for (auto* obj : objs) obj->blockSignals(false);
}

void AtualizaUITesouro(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoForma& gerador, const ent::EntidadeProto& proto) {
  AtualizaUITesouroGenerica(tabelas, gerador, proto);
}
void AtualizaUITesouro(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  AtualizaUITesouroGenerica(tabelas, gerador, proto);
}

void AtualizaUIPontosVida(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  gerador.spin_pontos_vida->setValue(proto.pontos_vida());
  gerador.spin_dano_nao_letal->setValue(proto.dano_nao_letal());
  gerador.botao_bonus_pv_temporario->setText(QString::number(BonusTotal(proto.pontos_vida_temporarios_por_fonte())));
  gerador.spin_max_pontos_vida->setValue(proto.max_pontos_vida());
}

void AtualizaPontosGastosPericia(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  int total_gasto = 0;
  for (const auto& ip : proto.info_pericias()) {
    total_gasto += ip.pontos();
  }
  int total_permitido = TotalPontosPericiaPermitidos(tabelas, proto);
  gerador.label_pericias->setText(QString("Perícias: pontos gastos %1, permitidos: %2").arg(total_gasto).arg(total_permitido));
}

void AtualizaUIPericias(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  // Se no windows nao tiver Moc vai dar erro de linker. Ai, transformar isso em reinterpret_cast.
  auto* modelo = qobject_cast<ModeloPericias*>(gerador.tabela_pericias->model());
  if (modelo != nullptr) {
    modelo->Recomputa();
  } else {
    LOG(ERROR) << "modelo eh nullptr";
  }
  gerador.tabela_pericias->update();
  AtualizaPontosGastosPericia(tabelas, gerador, proto);
}

// Feiticos.
void AdicionaItemFeiticoConhecido(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador,
    const std::string& id, const std::string& nome, const std::string& id_classe, int nivel, int slot,
    const ent::EntidadeProto& proto,
    QTreeWidgetItem* pai) {
  gerador.arvore_feiticos->blockSignals(true);
  auto* item_feitico = new ItemFeiticoConhecido(tabelas, id_classe, nivel, proto, pai);
  item_feitico->setIdNome(QString::fromUtf8(id.c_str()), QString::fromUtf8(nome.c_str()));
  item_feitico->setData(TCOL_CONHECIDO_OU_PARA_LANCAR, Qt::UserRole, QVariant(CONHECIDO));
  item_feitico->setData(TCOL_ID_CLASSE, Qt::UserRole, QVariant(id_classe.c_str()));
  item_feitico->setData(TCOL_NIVEL, Qt::UserRole, QVariant(nivel));
  item_feitico->setData(TCOL_INDICE, Qt::UserRole, QVariant(slot));
  item_feitico->setData(TCOL_ID_FEITICO, Qt::UserRole, QVariant(""));
  item_feitico->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
  gerador.arvore_feiticos->blockSignals(false);
}

void AtualizaFeiticosConhecidosNivel(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const std::string& id_classe, int nivel,
    const ent::EntidadeProto& proto, QTreeWidgetItem* pai) {
  gerador.arvore_feiticos->blockSignals(true);
  auto filhos = pai->takeChildren();
  for (auto* f : filhos) {
    delete f;
  }
  const auto& feiticos_nivel = ent::FeiticosNivel(id_classe, nivel, proto);
  int slot = 0;
  for (const auto& conhecido : feiticos_nivel.conhecidos()) {
    AdicionaItemFeiticoConhecido(
        tabelas, gerador, conhecido.id(),
        conhecido.has_nome() ? conhecido.nome() : tabelas.Feitico(conhecido.id()).nome(),
        id_classe, nivel, slot++, proto, pai);
    gerador.arvore_feiticos->blockSignals(true);
  }
  gerador.arvore_feiticos->blockSignals(false);
}

QCheckBox* CriaCheckboxUsado(
    const std::string& id_classe, int nivel_para_lancar, int indice_para_lancar,
    const ent::EntidadeProto& proto, QTreeWidgetItem* item_feitico) {
  auto* checkbox = new QCheckBox();
  const auto& fn_para_lancar = FeiticoParaLancar(id_classe, nivel_para_lancar, indice_para_lancar, proto);
  checkbox->setCheckState(fn_para_lancar.usado() ? Qt::Checked : Qt::Unchecked);
  item_feitico->setData(TCOL_USADO, Qt::UserRole, QVariant(fn_para_lancar.usado()));
  QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  sizePolicy.setHorizontalStretch(0);
  checkbox->setSizePolicy(sizePolicy);
  lambda_connect(checkbox, SIGNAL(stateChanged(int)), [item_feitico] (int estado) {
    item_feitico->setData(TCOL_USADO, Qt::UserRole, QVariant(estado == Qt::Checked));
  });;
  return checkbox;
}

QLabel* CriaLabelParaLancar(
    const std::string& id_classe, int nivel_para_lancar, int indice_para_lancar,
    const ent::EntidadeProto& proto) {
  auto* label = new QLabel();
  const auto& fn_para_lancar = FeiticoParaLancar(id_classe, nivel_para_lancar, indice_para_lancar, proto);
  const auto& fc = FeiticosClasse(id_classe, proto);
  if (fn_para_lancar.restrito()) {
    if (id_classe == "clerigo") {
      label->setText(QString::fromUtf8("Domínio"));
    } else {
      label->setText(QString::fromUtf8(fc.especializacao().c_str()));
    }
  }
  return label;
}

void PreencheComboParaLancar(
    const ent::Tabelas& tabelas,
    const std::string& id_classe, int nivel_para_lancar, int indice_para_lancar,
    const ent::EntidadeProto& proto, QTreeWidgetItem* item_feitico, QComboBox* combo) {
  // Mapeia os indices do combo para (nivel_conhecido, indice_conhecido).
  std::vector<std::pair<int, int>> mapa;
  const auto& fc = FeiticosClasse(id_classe, proto);
  const auto& fn_para_lancar = FeiticoParaLancar(id_classe, nivel_para_lancar, indice_para_lancar, proto);
  int indice_corrente = -1;
  combo->clear();
  for (int nivel_conhecido = nivel_para_lancar; nivel_conhecido >= 0; --nivel_conhecido) {
    QStringList lista;
    combo->addItem(QString(combo->tr("Nível %1")).arg(nivel_conhecido));
    QFont fonte = combo->itemData(combo->count() - 1, Qt::FontRole).value<QFont>();
    fonte.setWeight(QFont::Bold);
    combo->setItemData(combo->count() - 1, QVariant::fromValue(fonte), Qt::FontRole);
    mapa.push_back(std::make_pair(nivel_conhecido, -1));
    const auto& fn = ent::FeiticosNivel(id_classe, nivel_conhecido, proto);
    int indice_conhecido = 0;
    for (const auto& c : fn.conhecidos()) {
      const auto& feitico_tabelado = tabelas.Feitico(c.id());
      if (fn_para_lancar.restrito()) {
        if (id_classe == "clerigo" &&
            !FeiticoDominio(std::vector<std::string>(fc.dominios().begin(), fc.dominios().end()), feitico_tabelado)) {
          ++indice_conhecido;
          continue;
        }
        if (FeiticoEscolaProibida(std::vector<std::string>(fc.escolas_proibidas().begin(), fc.escolas_proibidas().end()), feitico_tabelado)) {
          ++indice_conhecido;
          continue;
        }
      }
      lista.push_back(QString::fromUtf8(c.has_nome() ? c.nome().c_str() : feitico_tabelado.nome().c_str()));
      mapa.push_back(std::make_pair(nivel_conhecido, indice_conhecido));
      if (fn_para_lancar.has_nivel_conhecido() && fn_para_lancar.nivel_conhecido() == nivel_conhecido &&
          fn_para_lancar.has_indice_conhecido() && fn_para_lancar.indice_conhecido() == indice_conhecido) {
        // Dados do feitico conhecido selecionado.
        indice_corrente = mapa.size() - 1;
        item_feitico->setData(TCOL_NIVEL_CONHECIDO, Qt::UserRole, QVariant(nivel_conhecido));
        item_feitico->setData(TCOL_INDICE_CONHECIDO, Qt::UserRole, QVariant(indice_conhecido));
      }
      ++indice_conhecido;
    }
    combo->addItems(lista);
  }
  combo->setCurrentIndex(indice_corrente);
  combo->disconnect();
  ExpandeComboBox(combo);
  lambda_connect(combo, SIGNAL(currentIndexChanged(int)), [item_feitico, mapa] (int indice) {
      if (mapa[indice].second == -1) {
        // Selecionou o label.
        return;
      }
      // Trigar apenas 1 evento.
      item_feitico->treeWidget()->blockSignals(true);
      item_feitico->setData(TCOL_NIVEL_CONHECIDO, Qt::UserRole, QVariant(mapa[indice].first));
      item_feitico->treeWidget()->blockSignals(false);
      item_feitico->setData(TCOL_INDICE_CONHECIDO, Qt::UserRole, QVariant(mapa[indice].second));
  });;
}

// Preenche o item da arvore com um combo que possui todos os feiticos conhecidos ate o nivel passado.
QComboBox* CriaComboParaLancar(
    const ent::Tabelas& tabelas,
    const std::string& id_classe, int nivel_para_lancar, int indice_para_lancar,
    const ent::EntidadeProto& proto, QTreeWidgetItem* item_feitico) {
  auto* combo = new QComboBox();
  combo->setObjectName("combo_para_lancar");
  PreencheComboParaLancar(tabelas, id_classe, nivel_para_lancar, indice_para_lancar, proto, item_feitico, combo);
  return combo;
}

// Adiciona o item para lancar. Caso a classe precise de memorizar, ira mostrar o nome do feitico memorizado
// no indice.
void AdicionaItemFeiticoParaLancar(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador,
    const std::string& id_classe, int nivel, int indice, const ent::EntidadeProto::InfoLancar& para_lancar,
    const ent::EntidadeProto& proto, QTreeWidgetItem* pai) {
  gerador.arvore_feiticos->blockSignals(true);
  auto* item_feitico = new QTreeWidgetItem(pai);
  if (tabelas.Classe(id_classe).possui_dominio() && indice == 0) {
    item_feitico->setBackground(0, QBrush(Qt::lightGray));
  }
  auto* hwidget = new QWidget;
  auto* hbox = new QHBoxLayout;
  hbox->addWidget(CriaCheckboxUsado(id_classe, nivel, indice, proto, item_feitico), 0, Qt::AlignLeft);
  hbox->addWidget(CriaLabelParaLancar(id_classe, nivel, indice, proto), 0, Qt::AlignLeft);
  if (ent::ClassePrecisaMemorizar(tabelas, id_classe)) {
    hbox->addWidget(CriaComboParaLancar(tabelas, id_classe, nivel, indice, proto, item_feitico));
  }
  hwidget->setLayout(hbox);
  item_feitico->treeWidget()->setItemWidget(item_feitico, 0, hwidget);
  item_feitico->setData(0, Qt::UserRole, QVariant(PARA_LANCAR));
  item_feitico->setData(1, Qt::UserRole, QVariant(id_classe.c_str()));
  item_feitico->setData(2, Qt::UserRole, QVariant(nivel));
  item_feitico->setData(3, Qt::UserRole, QVariant(indice));
  item_feitico->setFlags(
      item_feitico->flags() | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
  gerador.arvore_feiticos->blockSignals(false);
}

void AtualizaFeiticosParaLancarNivel(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador,
    const std::string& id_classe, int nivel, const ent::EntidadeProto& proto, QTreeWidgetItem* pai) {
  gerador.arvore_feiticos->blockSignals(true);
  auto filhos = pai->takeChildren();
  for (auto* f : filhos) {
    delete f;
  }
  const auto& feiticos_nivel = ent::FeiticosNivel(id_classe, nivel, proto);
  int slot = 0;
  VLOG(1)
      << "Para lancar nivel: " << nivel << ", qde: " << feiticos_nivel.para_lancar().size()
      << ", proto: " << feiticos_nivel.DebugString();
  for (const auto& para_lancar : feiticos_nivel.para_lancar()) {
    AdicionaItemFeiticoParaLancar(tabelas, gerador, id_classe, nivel, slot++, para_lancar, proto, pai);
    gerador.arvore_feiticos->blockSignals(true);
  }
  gerador.arvore_feiticos->blockSignals(false);
}

void AtualizaFeiticosClasse(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador,
    const std::string& id_classe, const ent::EntidadeProto& proto, QTreeWidgetItem* pai) {
  gerador.arvore_feiticos->blockSignals(true);
  for (int i = 0; i <= 9; ++i) {
    auto it = ent::FeiticosClasse(id_classe, proto).mapa_feiticos_por_nivel().find(i);
    if (it == ent::FeiticosClasse(id_classe, proto).mapa_feiticos_por_nivel().end()) continue;
    const auto& [nivel, fn] = *it;
    VLOG(1) << "atualizando " << id_classe << " nivel: " << nivel << ", tem " << fn.conhecidos().size() << " conhecidos e " << fn.para_lancar().size() << " para lancar";
    auto* item_nivel = new QTreeWidgetItem(pai);
    item_nivel->setText(0, QString::number(nivel));
    item_nivel->setData(TCOL_NIVEL, Qt::UserRole, QVariant(nivel));
    {
      auto* item_conhecidos = new ItemConhecidos(id_classe, nivel, item_nivel);
      item_conhecidos->setData(TCOL_CONHECIDO_OU_PARA_LANCAR, Qt::UserRole, QVariant(RAIZ_CONHECIDO));
      item_conhecidos->setData(TCOL_ID_CLASSE, Qt::UserRole, QVariant(id_classe.c_str()));
      item_conhecidos->setData(TCOL_NIVEL, Qt::UserRole, QVariant(nivel));
      AtualizaFeiticosConhecidosNivel(tabelas, gerador, id_classe, nivel, proto, item_conhecidos);
      gerador.arvore_feiticos->blockSignals(true);
    }
    {
      auto* item_para_lancar = new QTreeWidgetItem(item_nivel);
      item_para_lancar->setData(TCOL_CONHECIDO_OU_PARA_LANCAR, Qt::UserRole, QVariant(RAIZ_PARA_LANCAR));
      item_para_lancar->setData(TCOL_ID_CLASSE, Qt::UserRole, QVariant(id_classe.c_str()));
      item_para_lancar->setData(TCOL_NIVEL, Qt::UserRole, QVariant(nivel));
      item_para_lancar->setText(0, QString::fromUtf8("Para Lançar"));
      AtualizaFeiticosParaLancarNivel(tabelas, gerador, id_classe, nivel, proto, item_para_lancar);
      gerador.arvore_feiticos->blockSignals(true);
    }
  }
  gerador.arvore_feiticos->blockSignals(false);
}

void AtualizaCombosParaLancarDoNivel(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador,
    const std::string& id_classe, int nivel, const ent::EntidadeProto& proto, QTreeWidgetItem* item_nivel) {
  if (nivel < 0 || nivel > 9) return;
  QTreeWidgetItem* item_raiz_para_lancar = nullptr;
  for (int i = 0; i < item_nivel->childCount(); ++i) {
    if (item_nivel->child(i)->data(TCOL_CONHECIDO_OU_PARA_LANCAR, Qt::UserRole).toInt() == RAIZ_PARA_LANCAR) {
      item_raiz_para_lancar = item_nivel->child(i);
      break;
    }
  }
  if (item_raiz_para_lancar == nullptr) {
    LOG(ERROR) << "Nao encontrei o item raiz para lancar";
    return;
  }
  for (int i = 0; i < item_raiz_para_lancar->childCount(); ++i) {
    auto* item_para_lancar = item_raiz_para_lancar->child(i);
    // Aqui deve ser uma widget contendo o combo.
    auto* combo = gerador.arvore_feiticos->itemWidget(item_para_lancar, 0)
        ->findChild<QComboBox*>("combo_para_lancar");
    if (combo == nullptr) {
      LOG(ERROR) << "Nao encontrei o combo para lancar";
      continue;
    }
    combo->blockSignals(true);
    PreencheComboParaLancar(tabelas, id_classe, nivel, i, proto, item_para_lancar, combo);
    combo->blockSignals(false);
  }
}

QTreeWidgetItem* ItemRaizParaLancar(QTreeWidgetItem* item_nivel) {
  for (int i = 0; i < item_nivel->childCount(); ++i) {
    if (item_nivel->child(i)->data(TCOL_CONHECIDO_OU_PARA_LANCAR, Qt::UserRole).toInt() == RAIZ_PARA_LANCAR) {
      return item_nivel->child(i);
    }
  }
  LOG(ERROR) << "Nao encontrei o item raiz para lancar";
  return nullptr;
}

// Retorna o combo para lancar dentro de item_para_lancar.
QComboBox* ComboParaLancar(QTreeWidgetItem* item_para_lancar, QTreeWidget* arvore) {
  auto* combo = arvore->itemWidget(item_para_lancar, 0)
      ->findChild<QComboBox*>("combo_para_lancar");
  if (combo == nullptr) {
    LOG(ERROR) << "Nao encontrei o combo para lancar";
  }
  return combo;
}

// Encontra o item da classe da arvore de feiticos.
QTreeWidgetItem* ItemClasse(const std::string& id_classe, QTreeWidget* arvore) {
  for (int i = 0; i < arvore->topLevelItemCount(); ++i) {
    auto* item_classe = arvore->topLevelItem(i);
    if (item_classe->data(TCOL_ID_CLASSE, Qt::UserRole).toString().toStdString() != id_classe) {
      continue;
    }
    return item_classe;
  }
  return nullptr;
}

void AtualizaCombosParaLancarDoNivel(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador,
    const std::string& id_classe, int nivel, int nivel_removido, int indice_removido,
    const ent::EntidadeProto& proto, QTreeWidgetItem* item_nivel) {
  if (nivel < 0 || nivel > 9) return;
  QTreeWidgetItem* item_raiz_para_lancar = ItemRaizParaLancar(item_nivel);
  if (item_raiz_para_lancar == nullptr) return;

  for (int i = 0; i < item_raiz_para_lancar->childCount(); ++i) {
    auto* item_para_lancar = item_raiz_para_lancar->child(i);
    // Aqui deve ser uma widget contendo o combo.
    auto* combo = ComboParaLancar(item_para_lancar, gerador.arvore_feiticos);
    if (combo == nullptr) continue;
    combo->blockSignals(true);
    PreencheComboParaLancar(tabelas, id_classe, nivel, i, proto, item_para_lancar, combo);
    combo->blockSignals(false);
  }
}

void AtualizaCombosParaLancar(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador,
    const std::string& id_classe, const ent::EntidadeProto& proto) {
  auto* item_classe = ItemClasse(id_classe, gerador.arvore_feiticos);
  if (item_classe == nullptr) {
    LOG(ERROR) << "Nao encontrei item da classe";
    return;
  }
  // Itera nos niveis da classe.
  for (int i = 0; i < item_classe->childCount(); ++i) {
    auto* item = item_classe->child(i);
    int nivel = item->data(TCOL_NIVEL, Qt::UserRole).toInt();
    AtualizaCombosParaLancarDoNivel(tabelas, gerador, id_classe, nivel, proto, item);
  }
}

void AtualizaUIFeiticos(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  gerador.arvore_feiticos->blockSignals(true);
  gerador.arvore_feiticos->clear();
  for (const auto& ic : proto.info_classes()) {
    if (!ic.has_nivel_conjurador()) continue;

    // Acha a entrada de feitico do proto.
    auto* item_classe = new QTreeWidgetItem(gerador.arvore_feiticos);
    item_classe->setText(0, QString::fromUtf8(ic.nome().c_str()));
    item_classe->setData(TCOL_ID_CLASSE, Qt::UserRole, QVariant(ic.id().c_str()));
    // Aqui se passa o id ao inves de id_para_progressao porque as informacoes de magia estao vinculadas ao id.
    // Por exemplo, uma aberracao que lanca magias de feiticeiro armazena seus dados em aberracao e nao feiticeiro.
    AtualizaFeiticosClasse(tabelas, gerador, ic.id(), proto, item_classe);
    gerador.arvore_feiticos->blockSignals(true);
  }
  gerador.arvore_feiticos->blockSignals(false);
}

}  // namespace qt
}  // namespace ifg
