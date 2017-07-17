#include "ifg/qt/atualiza_ui.h"

#include "goog/stringprintf.h"
#include "ent/entidade.pb.h"
#include "ent/constantes.h"
#include "ent/util.h"
#include "net/util.h"
#include "ifg/qt/constantes.h"

namespace ifg {
namespace qt {

void AtualizaUI(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  AtualizaUIClassesNiveis(tabelas, gerador, proto);
  AtualizaUIAtributos(tabelas, gerador, proto);
  AtualizaUIAtaque(tabelas, gerador, proto);
  AtualizaUIDefesa(gerador, proto);
  AtualizaUIAtaquesDefesa(tabelas, gerador, proto);
  AtualizaUIIniciativa(tabelas, gerador, proto);
  AtualizaUISalvacoes(gerador, proto);
}

namespace {

QString NumeroSinalizado(int valor) {
  QString ret = QString::number(valor);
  if (valor > 0) ret.prepend("+");
  return ret;
}

void LimpaCamposClasse(ifg::qt::Ui::DialogoEntidade& gerador) {
  gerador.linha_classe->clear();
  gerador.spin_nivel_classe->clear();
  gerador.spin_nivel_conjurador->clear();
  gerador.spin_bba->clear();
  gerador.label_mod_conjuracao->setText("0");
  gerador.combo_mod_conjuracao->setCurrentIndex(0);
  gerador.botao_remover_nivel->setEnabled(false);
}

// Atualiza a UI com a lista de niveis e os totais.
void AtualizaUINiveis(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
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
  for (const auto& info_classe : proto.info_classes()) {
    std::string string_nivel;
    google::protobuf::StringAppendF(&string_nivel, "classe: %s, nível: %d", info_classe.id().c_str(), info_classe.nivel());
    if (info_classe.nivel_conjurador() > 0) {
      google::protobuf::StringAppendF(
          &string_nivel, ", conjurador: %d, mod (%s): %d",
          info_classe.nivel_conjurador(), TipoAtributo_Name(info_classe.atributo_conjuracao()).substr(3, 3).c_str(),
          info_classe.modificador_atributo_conjuracao());
    }
    google::protobuf::StringAppendF(&string_nivel, ", BBA: %d", info_classe.bba());
    gerador.lista_niveis->addItem(QString::fromUtf8(string_nivel.c_str()));
  }
  if (indice_antes < proto.info_classes().size()) {
    gerador.lista_niveis->setCurrentRow(indice_antes);
  } else {
    gerador.lista_niveis->setCurrentRow(-1);
  }
}

}  // namespace

void AtualizaUIClassesNiveis(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  // Objetos da UI a serem bloqueados. Passa por copia.
  std::vector<QObject*> objs = {
      gerador.spin_nivel_classe, gerador.spin_nivel_conjurador, gerador.linha_classe, gerador.spin_bba,
      gerador.combo_mod_conjuracao, gerador.lista_niveis
  };
  auto BloqueiaSinais = [objs] {
    for (auto* obj : objs) obj->blockSignals(true);
  };
  auto DesbloqueiaSinais = [objs] {
    for (auto* obj : objs) obj->blockSignals(false);
  };

  BloqueiaSinais();
  AtualizaUINiveis(gerador, proto);
  const int indice = gerador.lista_niveis->currentRow();
  if (indice < 0 || indice >= proto.info_classes_size()) {
    LimpaCamposClasse(gerador);
    DesbloqueiaSinais();
    return;
  }
  const auto& info_classe = proto.info_classes(indice);
  gerador.botao_remover_nivel->setEnabled(true);
  gerador.linha_classe->setText(QString::fromUtf8(info_classe.id().c_str()));
  gerador.spin_nivel_classe->setValue(info_classe.nivel());
  gerador.spin_nivel_conjurador->setValue(info_classe.nivel_conjurador());
  gerador.spin_bba->setValue(info_classe.bba());
  gerador.combo_mod_conjuracao->setCurrentIndex(info_classe.atributo_conjuracao());
  gerador.label_mod_conjuracao->setText(NumeroSinalizado(info_classe.modificador_atributo_conjuracao()));
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
  gerador.botao_remover_ataque->setEnabled(false);
  gerador.botao_ataque_cima->setEnabled(false);
  gerador.botao_ataque_baixo->setEnabled(false);
  gerador.botao_clonar_ataque->setText(QObject::tr("Adicionar"));

  gerador.combo_empunhadura->setCurrentIndex(0);
  gerador.checkbox_op->setCheckState(Qt::Unchecked);
  gerador.botao_bonus_ataque->setText("0");
  gerador.botao_bonus_dano->setText("0");
  gerador.spin_bonus_magico->setValue(0);
  gerador.linha_dano->clear();
  gerador.spin_alcance_quad->setValue(0);
}

// Monta a string de dano de um ataque incluindo modificadores, como 1d6+3 (x3), CA: 12/12/10.
std::string StringFinalDanoComCA(const ent::EntidadeProto::DadosAtaque& da) {
  // Monta a string.
  std::string critico;
  if (da.multiplicador_critico() > 2 || da.margem_critico() < 20) {
    critico += "(";
    if (da.margem_critico() < 20) {
      critico += net::to_string(da.margem_critico()) + "-20";
      if (da.multiplicador_critico() > 2) {
        critico += "/";
      }
    }
    if (da.multiplicador_critico() > 2) {
      critico += "x" + net::to_string(da.multiplicador_critico());
    }
    critico += "), " + google::protobuf::StringPrintf("CA: %d/%d/%d", da.ca_normal(), da.ca_surpreso(), da.ca_toque());
  }
  return da.dano() + critico;
}

// Retorna o resumo da arma, algo como id: rotulo, alcance: 10 q, 5 incrementos, bonus +3, dano: 1d8(x3), CA: 15, toque: 12, surpresa: 13.
std::string ResumoArma(const ent::EntidadeProto::DadosAtaque& da) {
  // Monta a string.
  char string_rotulo[40] = { '\0' };
  if (!da.rotulo().empty()) {
    snprintf(string_rotulo, 39, "%s, ", da.rotulo().c_str());
  }
  char string_alcance[40] = { '\0' };
  if (da.has_alcance_m()) {
    char string_incrementos[40] = { '\0' };
    if (da.has_incrementos()) {
      snprintf(string_incrementos, 39, ", inc %d", da.incrementos());
    }
    snprintf(string_alcance, 39, "alcance: %0.0f q%s, ", da.alcance_m() * METROS_PARA_QUADRADOS, string_incrementos);
  }
  std::string string_escudo = da.empunhadura() == ent::EA_ARMA_ESCUDO ? "(escudo)" : "";
  return google::protobuf::StringPrintf(
      "id: %s%s, %sbonus: %d, dano: %s, ca%s: %d toque: %d surpresa%s: %d",
      string_rotulo, da.tipo_ataque().c_str(), string_alcance, da.bonus_ataque(), StringFinalDanoComCA(da).c_str(), string_escudo.c_str(), da.ca_normal(),
      da.ca_toque(), string_escudo.c_str(), da.ca_surpreso());
}

// Monta a string de dano de uma arma de um ataque, como 1d6 (x3). Nao inclui modificadores.
std::string StringDano(const ent::EntidadeProto::DadosAtaque& da) {
  std::string critico;
  if (da.multiplicador_critico() > 2 || da.margem_critico() < 20) {
    critico += "(";
    if (da.margem_critico() < 20) {
      critico += net::to_string(da.margem_critico()) + "-20";
      if (da.multiplicador_critico() > 2) {
        critico += "/";
      }
    }
    if (da.multiplicador_critico() > 2) {
      critico += "x" + net::to_string(da.multiplicador_critico());
    }
    critico += ")";
  }
  return da.dano_basico_arma() + critico;
}

int TipoParaIndice(const std::string& tipo_str) {
  // Os tipos sao encontrados no arquivo dados/acoes.asciiproto.
  // Os indices sao na ordem definida pela UI.
  if (tipo_str == "Ácido") {
    return 0;
  } else if (tipo_str == "Ataque Corpo a Corpo") {
    return 1;
  } else if (tipo_str == "Ataque a Distância") {
    return 2;
  } else if (tipo_str == "Bola de Fogo") {
    return 3;
  } else if (tipo_str == "Coluna de Chamas") {
    return 4;
  } else if (tipo_str == "Cone de Gelo") {
    return 5;
  } else if (tipo_str == "Feitiço de Toque") {
    return 6;
  } else if (tipo_str == "Feitiço de Toque a Distância") {
    return 7;
  } else if (tipo_str == "Fogo Alquímico") {
    return 8;
  } else if (tipo_str == "Mãos Flamejantes") {
    return 9;
  } else if (tipo_str == "Míssil Mágico") {
    return 10;
  } else if (tipo_str == "Pedrada (gigante)") {
    return 11;
  } else if (tipo_str == "Raio") {
    return 12;
  } else if (tipo_str == "Relâmpago") {
    return 13;
  } else if (tipo_str == "Tempestade Glacial") {
    return 14;
  } else {
    return 1;
  }
}

}  // namespace

void AtualizaUIAtaque(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  std::vector<QObject*> objs =
      {gerador.spin_bonus_magico, gerador.checkbox_op, gerador.checkbox_possui_acuidade,
       gerador.spin_alcance_quad, gerador.spin_incrementos, gerador.combo_empunhadura,
       gerador.combo_tipo_ataque, gerador.linha_dano, gerador.linha_rotulo_ataque, gerador.lista_ataques };
  for (auto* obj : objs) obj->blockSignals(true);

  gerador.checkbox_possui_acuidade->setCheckState(proto.dados_ataque_globais().acuidade() ? Qt::Checked : Qt::Unchecked);

  // Tem que vir antes do clear.
  const int linha = gerador.lista_ataques->currentRow();
  gerador.lista_ataques->clear();
  for (const auto& da : proto.dados_ataque()) {
    gerador.lista_ataques->addItem(QString::fromUtf8(ResumoArma(da).c_str()));
  }
  // Restaura a linha.
  gerador.lista_ataques->setCurrentRow(linha);
  // BBA.
  int bba = 0;
  for (const auto& info_classe : proto.info_classes()) {
    bba += info_classe.bba();
  }
  gerador.label_bba_base->setText(QString::number(proto.bba().base()));
  gerador.label_bba_agarrar->setText(QString::number(proto.bba().agarrar()));
  gerador.label_bba_cac->setText(QString::number(proto.bba().cac()));
  gerador.label_bba_distancia->setText(QString::number(proto.bba().distancia()));

  if (linha < 0 || linha >= proto.dados_ataque_size()) {
    LimpaCamposAtaque(gerador);
    for (auto* obj : objs) obj->blockSignals(false);
    return;
  }
  gerador.botao_remover_ataque->setEnabled(true);
  const auto& da = proto.dados_ataque(linha);
  gerador.linha_rotulo_ataque->setText(QString::fromUtf8(da.rotulo().c_str()));
  gerador.combo_tipo_ataque->setCurrentIndex(TipoParaIndice(da.tipo_ataque()));
  gerador.linha_dano->setText(QString::fromUtf8(StringDano(da).c_str()));
  gerador.spin_incrementos->setValue(da.incrementos());
  gerador.spin_alcance_quad->setValue(METROS_PARA_QUADRADOS * (da.has_alcance_m() ? da.alcance_m() : -1.5f));
  gerador.checkbox_op->setCheckState(da.obra_prima() ? Qt::Checked : Qt::Unchecked);
  gerador.combo_empunhadura->setCurrentIndex(da.empunhadura());
  gerador.spin_bonus_magico->setValue(ent::BonusIndividualPorOrigem(ent::TB_MELHORIA, "arma_magica", da.outros_bonus_ataque()));
  gerador.botao_bonus_ataque->setText(QString::number(ent::BonusTotal(da.outros_bonus_ataque())));
  gerador.botao_bonus_dano->setText(QString::number(ent::BonusTotal(da.outros_bonus_dano())));
  gerador.botao_clonar_ataque->setText(QObject::tr("Clonar"));
  if (proto.dados_ataque().size() > 1) {
    gerador.botao_ataque_cima->setEnabled(true);
    gerador.botao_ataque_baixo->setEnabled(true);
  }
  for (auto* obj : objs) obj->blockSignals(false);
}

void AtualizaUIDefesa(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  const auto& dd = proto.dados_defesa();
  // combo armadura.
  for (int i = 0; dd.has_id_armadura() && i < gerador.combo_armadura->count(); ++i) {
    QVariant dados_armadura = gerador.combo_armadura->itemData(i);
    if (dados_armadura.toString().toStdString() == dd.id_armadura()) {
      gerador.combo_armadura->setCurrentIndex(i);
      break;
    }
  }
  // combo escudo.
  for (int i = 0; dd.has_id_escudo() && i < gerador.combo_escudo->count(); ++i) {
    QVariant dados_escudo = gerador.combo_escudo->itemData(i);
    if (dados_escudo.toString().toStdString() == dd.id_escudo()) {
      gerador.combo_escudo->setCurrentIndex(i);
      break;
    }
  }
  const int modificador_destreza = proto.atributos().has_destreza() ?
      ent::ModificadorAtributo(ent::BonusTotal(proto.atributos().destreza())) : 0;
  const auto& ca = dd.ca();
  gerador.botao_bonus_ca->setText(QString::number(BonusTotal(ca)));

  std::vector<QWidget*> objs = { gerador.spin_ca_armadura_melhoria, gerador.spin_ca_escudo_melhoria };
  for (auto* obj : objs) obj->blockSignals(true);
  gerador.spin_ca_armadura_melhoria->setValue(ent::BonusIndividualTotal(ent::TB_ARMADURA_MELHORIA, ca));
  gerador.spin_ca_escudo_melhoria->setValue(ent::BonusIndividualTotal(ent::TB_ESCUDO_MELHORIA, ca));
  for (auto* obj : objs) obj->blockSignals(false);
  const int bonus_ca_total = ent::BonusTotal(ca);
  gerador.botao_bonus_ca->setText(QString::number(bonus_ca_total));
  gerador.label_ca_toque->setText(QString::number(
      ent::BonusTotalExcluindo(
        ca,
        { ent::TB_ARMADURA, ent::TB_ESCUDO, ent::TB_ARMADURA_NATURAL, ent::TB_ARMADURA_MELHORIA, ent::TB_ESCUDO_MELHORIA })));
  gerador.label_ca_surpreso->setText(QString::number(bonus_ca_total - std::max(modificador_destreza, 0)));
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
  std::vector<std::tuple<QSpinBox*, QPushButton*, const ent::Bonus*>> tuplas = {
    std::make_tuple(gerador.spin_salvacao_fortitude, gerador.botao_bonus_salvacao_fortitude, &dd.salvacao_fortitude()),
    std::make_tuple(gerador.spin_salvacao_reflexo, gerador.botao_bonus_salvacao_reflexo, &dd.salvacao_reflexo()),
    std::make_tuple(gerador.spin_salvacao_vontade, gerador.botao_bonus_salvacao_vontade, &dd.salvacao_vontade()),
  };
  for (const auto& t : tuplas) {
    QSpinBox* spin; QPushButton* botao; const ent::Bonus* bonus;
    std::tie(spin, botao, bonus) = t;
    spin->setValue(ent::BonusIndividualTotal(ent::TB_BASE, *bonus));
    botao->setText(NumeroSinalizado(ent::BonusTotal(*bonus)));
  }
}

}  // namespace qt
}  // namespace ifg

