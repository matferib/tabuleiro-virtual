#include "ifg/qt/atualiza_ui.h"

#include "ent/entidade.pb.h"
#include "ent/constantes.h"
#include "ent/tabelas.h"
#include "ent/util.h"
#include "goog/stringprintf.h"
#include "ifg/qt/constantes.h"
#include "ifg/qt/pericias_util.h"
#include "ifg/qt/util.h"
#include "log/log.h"
#include "net/util.h"

namespace ifg {
namespace qt {

void AtualizaUI(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  AtualizaUIClassesNiveis(tabelas, gerador, proto);
  AtualizaUIAtributos(tabelas, gerador, proto);
  AtualizaUIAtaquesDefesa(tabelas, gerador, proto);
  AtualizaUIIniciativa(tabelas, gerador, proto);
  AtualizaUISalvacoes(gerador, proto);
  AtualizaUITesouro(tabelas, gerador, proto);
  AtualizaUIPontosVida(gerador, proto);
  AtualizaUIPericias(tabelas, gerador, proto);
  AtualizaUIFeiticos(tabelas, gerador, proto);
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
  for (const auto& ic : proto.info_classes()) {
    std::string string_nivel;
    google::protobuf::StringAppendF(&string_nivel, "classe: %s, nível: %d", ic.id().c_str(), ic.nivel());
    if (ic.nivel_conjurador() > 0) {
      google::protobuf::StringAppendF(
          &string_nivel, ", conjurador: %d, mod (%s): %d",
          ic.nivel_conjurador(), TipoAtributo_Name(ic.atributo_conjuracao()).substr(3, 3).c_str(),
          ic.modificador_atributo_conjuracao());
    }
    google::protobuf::StringAppendF(&string_nivel, ", BBA: %d, Salv Fortes: %s", ic.bba(), StringSalvacoesFortes(ic).c_str());
    gerador.lista_niveis->addItem(QString::fromUtf8(string_nivel.c_str()));
  }
  if (indice_antes < proto.info_classes().size()) {
    gerador.lista_niveis->setCurrentRow(indice_antes);
  } else {
    gerador.lista_niveis->setCurrentRow(-1);
  }
}

int IdClasseParaIndice(const std::string& id, const QComboBox* combo) {
  int indice = combo->findData(id.c_str());
  return indice < 0 ? combo->findData("outro") : indice;
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
      gerador.combo_mod_conjuracao, gerador.lista_niveis, gerador.combo_salvacoes_fortes, gerador.combo_classe
  };
  auto BloqueiaSinais = [objs] {
    for (auto* obj : objs) obj->blockSignals(true);
  };
  auto DesbloqueiaSinais = [objs] {
    for (auto* obj : objs) obj->blockSignals(false);
  };

  BloqueiaSinais();
  AtualizaUINiveis(gerador, proto);
  gerador.spin_niveis_negativos->setValue(proto.niveis_negativos());

  const int indice = gerador.lista_niveis->currentRow();
  // Se tiver selecao, preenche.
  if (indice >= 0 && indice < proto.info_classes_size()) {
    const auto& info_classe = proto.info_classes(indice);
    gerador.botao_remover_nivel->setEnabled(true);
    gerador.combo_classe->setCurrentIndex(IdClasseParaIndice(info_classe.id(), gerador.combo_classe));
    gerador.linha_classe->setText(QString::fromUtf8(info_classe.id().c_str()));
    gerador.spin_nivel_classe->setValue(info_classe.nivel());
    gerador.spin_nivel_conjurador->setValue(info_classe.nivel_conjurador());
    gerador.spin_bba->setValue(info_classe.bba());
    gerador.combo_mod_conjuracao->setCurrentIndex(info_classe.atributo_conjuracao());
    gerador.label_mod_conjuracao->setText(NumeroSinalizado(info_classe.modificador_atributo_conjuracao()));
    gerador.combo_salvacoes_fortes->setCurrentIndex(SalvacoesFortesParaIndice(info_classe));
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
  gerador.botao_clonar_ataque->setText(QObject::tr("Adicionar"));

  gerador.combo_empunhadura->setCurrentIndex(0);
  gerador.checkbox_op->setCheckState(Qt::Unchecked);
  gerador.botao_bonus_ataque->setText("0");
  gerador.botao_bonus_dano->setText("0");
  gerador.spin_bonus_magico->setValue(0);
  gerador.spin_municao->setValue(0);
  gerador.linha_dano->clear();
  gerador.spin_alcance_quad->setValue(0);
}

void PreencheComboArma(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const std::string& tipo_ataque) {
  const bool cac = tipo_ataque == "Ataque Corpo a Corpo";
  const bool projetil_area = tipo_ataque == "Projétil de Área";
  const bool distancia = tipo_ataque == "Ataque a Distância";
  const bool feitico_mago = tipo_ataque == "Feitiço de Mago";
  const bool feitico_clerigo = tipo_ataque == "Feitiço de Clérigo";
  const bool feitico_druida = tipo_ataque == "Feitiço de Druida";
  std::map<std::string, std::string> nome_id_map;
  if (cac || projetil_area || distancia) {
    for (const auto& arma : tabelas.todas().tabela_armas().armas()) {
      const bool arma_projetil_area = ent::PossuiCategoria(ent::CAT_PROJETIL_AREA, arma);
      if ((cac && ent::PossuiCategoria(ent::CAT_CAC, arma)) ||
          (projetil_area && arma_projetil_area) ||
          (distancia && !arma_projetil_area && ent::PossuiCategoria(ent::CAT_DISTANCIA, arma))) {
        nome_id_map[arma.nome()] = arma.id();
      }
    }
  } else if (feitico_mago || feitico_clerigo || feitico_druida) {
    for (const auto& arma : tabelas.todas().tabela_feiticos().armas()) {
      nome_id_map[arma.nome()] = arma.id();
    }
  }
  gerador.combo_arma->clear();
  gerador.combo_arma->addItem("Nenhuma", QVariant("nenhuma"));
  for (const auto& name_id : nome_id_map) {
    gerador.combo_arma->addItem(QString::fromUtf8(name_id.first.c_str()), QVariant(name_id.second.c_str()));
  }
}

}  // namespace

void AtualizaUIAtaque(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  std::vector<QObject*> objs =
      {gerador.spin_bonus_magico, gerador.checkbox_op, gerador.spin_municao,
       gerador.spin_alcance_quad, gerador.spin_incrementos, gerador.combo_empunhadura,
       gerador.combo_tipo_ataque, gerador.linha_dano, gerador.linha_rotulo_ataque, gerador.lista_ataques,
       gerador.combo_arma, gerador.spin_ordem_ataque };
  for (auto* obj : objs) obj->blockSignals(true);

  // Tem que vir antes do clear.
  const int linha = gerador.lista_ataques->currentRow();
  gerador.lista_ataques->clear();
  for (const auto& da : proto.dados_ataque()) {
    gerador.lista_ataques->addItem(QString::fromUtf8(ent::StringResumoArma(tabelas, da).c_str()));
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

  const bool linha_valida = linha >= 0 && linha < proto.dados_ataque_size();
  const auto& tipo_ataque = linha_valida ? proto.dados_ataque(linha).tipo_ataque() : CurrentData(gerador.combo_tipo_ataque).toString().toStdString();
  gerador.combo_arma->setEnabled(
      tipo_ataque == "Ataque Corpo a Corpo" || tipo_ataque == "Ataque a Distância" || tipo_ataque == "Projétil de Área" ||
      tipo_ataque == "Feitiço de Mago" || tipo_ataque == "Feitiço de Clérigo" || tipo_ataque == "Feitiço de Druida");
  PreencheComboArma(tabelas, gerador, tipo_ataque);
  if (!linha_valida) {
    LimpaCamposAtaque(gerador);
    gerador.botao_remover_ataque->setEnabled(!proto.dados_ataque().empty());
    for (auto* obj : objs) obj->blockSignals(false);
    return;
  }
  const auto& da = proto.dados_ataque(linha);
  gerador.combo_arma->setCurrentIndex(da.id_arma().empty() ? 0 : gerador.combo_arma->findData(da.id_arma().c_str()));
  gerador.botao_remover_ataque->setEnabled(true);
  gerador.linha_rotulo_ataque->setText(QString::fromUtf8(da.rotulo().c_str()));
  const auto& tipo_str = da.tipo_ataque();
  gerador.combo_tipo_ataque->setCurrentIndex(gerador.combo_tipo_ataque->findData(tipo_str.c_str()));
  gerador.linha_dano->setText(QString::fromUtf8(ent::StringDanoBasicoComCritico(da).c_str()));
  gerador.spin_incrementos->setValue(da.incrementos());
  gerador.spin_alcance_quad->setValue(ent::METROS_PARA_QUADRADOS * (da.has_alcance_m() ? da.alcance_m() : -1.5f));
  gerador.checkbox_op->setCheckState(da.obra_prima() ? Qt::Checked : Qt::Unchecked);
  gerador.combo_empunhadura->setCurrentIndex(da.empunhadura());
  gerador.spin_bonus_magico->setValue(ent::BonusIndividualPorOrigem(ent::TB_MELHORIA, "arma_magica", da.bonus_ataque()));
  gerador.spin_municao->setValue(da.municao());
  // A ordem eh indexada em 0, mas usuarios entendem primeiro como 1.
  gerador.spin_ordem_ataque->setValue(da.ordem_ataque() + 1);

  gerador.botao_bonus_ataque->setText(QString::number(ent::BonusTotal(da.bonus_ataque())));
  gerador.botao_bonus_dano->setText(QString::number(ent::BonusTotal(da.bonus_dano())));
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
          ? google::protobuf::StringPrintf("%s (corrente)", fa.rotulo().c_str()).c_str()
          : google::protobuf::StringPrintf("%s (secundária)", fa.rotulo().c_str()).c_str()));
    ++i;
  }
  gerador.lista_formas_alternativas->setCurrentRow(indice_antes);
  gerador.lista_formas_alternativas->blockSignals(false);
}

void AtualizaUITesouro(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  std::vector<QWidget*> objs = { gerador.lista_tesouro, gerador.lista_pocoes };
  for (auto* obj : objs) obj->blockSignals(true);

  // Nao atualiza o campo de texto, faz isso so no final.
  //auto cursor = gerador.lista_tesouro->cursor();
  //gerador.lista_tesouro->setPlainText(QString::fromUtf8(proto.tesouro().tesouro().c_str()));
  //gerador.lista_tesouro->setCursor(cursor);

  const int indice = gerador.lista_pocoes->currentRow();
  gerador.lista_pocoes->clear();
  for (const auto& pocao : proto.tesouro().pocoes()) {
    const auto& pp = tabelas.Pocao(pocao.id());
    auto* item = new QListWidgetItem(QString::fromUtf8(pp.nome().c_str()), gerador.lista_pocoes);
    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  }
  gerador.lista_pocoes->setCurrentRow(indice);
  for (auto* obj : objs) obj->blockSignals(false);
}

void AtualizaUIPontosVida(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  gerador.spin_pontos_vida->setValue(proto.pontos_vida());
  gerador.spin_dano_nao_letal->setValue(proto.dano_nao_letal());
  gerador.botao_bonus_pv_temporario->setText(QString::number(BonusTotal(proto.pontos_vida_temporarios_por_fonte())));
  gerador.spin_max_pontos_vida->setValue(proto.max_pontos_vida());
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
}

// Feiticos.
void AdicionaItemFeitico(
    ifg::qt::Ui::DialogoEntidade& gerador, const std::string& nome, const std::string& id_classe, int nivel, int slot, bool memorizado,
    QTreeWidgetItem* pai) {
  gerador.arvore_feiticos->blockSignals(true);
  auto* item_feitico = new QTreeWidgetItem(pai);
  item_feitico->setText(0, QString::fromUtf8(nome.c_str()));
  item_feitico->setData(0, Qt::UserRole, QVariant(1));
  item_feitico->setData(1, Qt::UserRole, QVariant(id_classe.c_str()));
  item_feitico->setData(2, Qt::UserRole, QVariant(nivel));
  item_feitico->setData(3, Qt::UserRole, QVariant(slot));
  item_feitico->setCheckState(0, memorizado ? Qt::Checked : Qt::Unchecked);
  item_feitico->setFlags(item_feitico->flags() | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);
  gerador.arvore_feiticos->blockSignals(false);
}

void AtualizaFeiticosNivel(
    ifg::qt::Ui::DialogoEntidade& gerador, int nivel, const std::string& id_classe, const ent::EntidadeProto& proto, QTreeWidgetItem* item) {
  gerador.arvore_feiticos->blockSignals(true);
  auto filhos = item->takeChildren();
  for (auto* f : filhos) {
    delete f;
  }
  const auto& feiticos_nivel = ent::FeiticosNivel(nivel, id_classe, proto);
  int slot = 0;
  for (const auto& feitico : feiticos_nivel.feiticos()) {
    AdicionaItemFeitico(gerador, feitico.nome(), id_classe, nivel, slot++, feitico.memorizado(), item);
  }
  gerador.arvore_feiticos->blockSignals(false);
}

void AtualizaFeiticosClasse(ifg::qt::Ui::DialogoEntidade& gerador, const ent::InfoClasse& ic, const ent::EntidadeProto& proto, QTreeWidgetItem* item) {
  for (int i = 0; i < 10; ++i) {
    auto* item_nivel = new QTreeWidgetItem(item);
    item_nivel->setText(0, QString::number(i));
    item_nivel->setData(0, Qt::UserRole, QVariant(0));
    item_nivel->setData(1, Qt::UserRole, QVariant(ic.id().c_str()));
    item_nivel->setData(2, Qt::UserRole, QVariant(i));
    AtualizaFeiticosNivel(gerador, i, ic.id(), proto, item_nivel);
  }
}

void AtualizaUIFeiticos(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  gerador.arvore_feiticos->blockSignals(true);
  gerador.arvore_feiticos->clear();
  for (const auto& ic : proto.info_classes()) {
    if (ic.has_nivel_conjurador()) {
      // Acha a entrada de feitico do proto.
      auto* item_classe = new QTreeWidgetItem(gerador.arvore_feiticos);
      item_classe->setText(0, QString::fromUtf8(ic.nome().c_str()));
      AtualizaFeiticosClasse(gerador, ic, proto, item_classe);
    }
  }
  gerador.arvore_feiticos->blockSignals(false);
}

}  // namespace qt
}  // namespace ifg

