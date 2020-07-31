#include <gtest/gtest.h>

#include <google/protobuf/text_format.h>
#include <queue>
#include "arq/arquivo.h"
#include "ent/acoes.pb.h"
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/recomputa.h"
#include "ent/tabelas.h"
#include "ent/tabuleiro.h"
#include "ent/util.h"
#include "log/log.h"
#include "ntf/notificacao.h"

namespace ent {

extern std::queue<int> g_dados_teste;
namespace {

class CentralColetora : public ntf::CentralNotificacoes {
 public:
  CentralColetora() : ntf::CentralNotificacoes() {
    // Para ter notificacao remota coletada.
    RegistraEmissorRemoto(&emissor_remoto_);
  }
  std::vector<std::unique_ptr<ntf::Notificacao>>& Notificacoes() { return notificacoes_; }
  std::vector<std::unique_ptr<ntf::Notificacao>>& NotificacoesRemotas() { return notificacoes_remotas_; }

 private:
  class EmissorFake : public ntf::EmissorRemoto {
    bool TrataNotificacaoRemota(const ntf::Notificacao& notificacao) override { return true; }
  };
  EmissorFake emissor_remoto_;
};
CentralColetora g_central;
Tabelas g_tabelas(&g_central);

class TabuleiroTeste : public Tabuleiro {
 public:
  TabuleiroTeste(const std::vector<Entidade*>& entidades) : Tabuleiro(OpcoesProto::default_instance(), g_tabelas, nullptr, nullptr, &g_central) {
    for (auto* entidade : entidades) {
      entidades_.insert(std::make_pair(entidade->Id(), std::unique_ptr<Entidade>(entidade)));
    }
  }
};

const DadosAtaque& DadosAtaquePorGrupo(const std::string& grupo, const EntidadeProto& proto, int n = 0) {
  for (const auto& da : proto.dados_ataque()) {
    if (da.grupo() == grupo) {
      if (n <= 0) return da;
      --n;
      continue;
    }
  }
  return DadosAtaque::default_instance();
}

DadosAtaque* DadosAtaquePorGrupoOuCria(const std::string& grupo, EntidadeProto* proto) {
  for (auto& da : *proto->mutable_dados_ataque()) {
    if (da.grupo() == grupo) return &da;
  }
  auto* da = proto->add_dados_ataque();
  da->set_grupo(grupo);
  return da;
}

const ntf::Notificacao& NotificacaoFilhaOuPadrao(const ntf::Notificacao& n, int indice = 0) {
  return indice >= 0 && indice < n.notificacao().size() ? n.notificacao(indice) : ntf::Notificacao::default_instance();
}

}  // namespace

TEST(TesteBonus, TesteBonusCumulativo) {
  Bonus bonus;
  const char* bonus_texto = R"__(
    bonus_individual {
      tipo: TB_CIRCUNSTANCIA
      por_origem { valor: 3 origem: 'origem0' }
      # Nao acumula com anterior. Fica so o 5.
      por_origem { valor: 5 origem: 'origem0' }
      # Acumula, origem diferente.
      por_origem { valor: 10 origem: 'origem1' }

      # penalidades.
      por_origem { valor: -2 origem: 'origempenalidade0' }
      # Nao acumula.
      por_origem { valor: -1 origem: 'origempenalidade0' }
      # Acumula.
      por_origem { valor: -1 origem: 'origempenalidade1' }
    }
  )__";
  ASSERT_TRUE(google::protobuf::TextFormat::ParseFromString(bonus_texto, &bonus));
  EXPECT_EQ(12, BonusTotal(bonus));
}

TEST(TesteBonus, TesteBonusNaoCumulativo) {
  Bonus bonus;
  const char* bonus_texto = R"__(
    bonus_individual {
      tipo: TB_MELHORIA
      por_origem { valor: 3 origem: 'origem0' }
      # Nao acumula com anterior. Fica so o 5.
      por_origem { valor: 5 origem: 'origem0' }
      # Acumula, origem diferente.
      por_origem { valor: 10 origem: 'origem1' }

      # penalidades.
      por_origem { valor: -2 origem: 'origempenalidade0' }
      # Nao acumula.
      #por_origem { valor: -1 origem: 'origempenalidade0' }
      # Acumula.
      #por_origem { valor: -1 origem: 'origempenalidade1' }
    }
  )__";
  ASSERT_TRUE(google::protobuf::TextFormat::ParseFromString(bonus_texto, &bonus));
  EXPECT_EQ(8, BonusTotal(bonus));
}

TEST(TesteItemMagico, TesteItemMagicoContinuo) {
  EntidadeProto proto;
  auto* anel = proto.mutable_tesouro()->add_aneis();
  anel->set_id("protecao_1");
  anel->set_em_uso(true);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_FALSE(proto.evento().empty());
  const auto& evento = proto.evento(0);
  EXPECT_EQ(evento.id_efeito(), EFEITO_DEFLEXAO_CA);
  EXPECT_EQ(evento.id_unico(), 0);
  EXPECT_TRUE(evento.continuo());
  EXPECT_TRUE(evento.requer_pai());

  proto.mutable_tesouro()->clear_aneis();
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_TRUE(proto.evento().empty());
}

TEST(TesteItemMagico, TesteItemMagicoParalisia) {
  EntidadeProto proto;
  AtribuiBaseAtributo(12, TA_FORCA, &proto);
  AtribuiBaseAtributo(12, TA_DESTREZA, &proto);
  auto* luvas = proto.mutable_tesouro()->add_luvas();
  luvas->set_id("luvas_destreza_2");
  luvas->set_em_uso(true);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(12, BonusTotal(BonusAtributo(TA_FORCA, proto)));
  EXPECT_EQ(14, BonusTotal(BonusAtributo(TA_DESTREZA, proto)));

  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_PARALISIA);
  evento->set_rodadas(1);
  RecomputaDependencias(g_tabelas, &proto);

  EXPECT_EQ(0, BonusTotal(BonusAtributo(TA_FORCA, proto)));
  EXPECT_EQ(0, BonusTotal(BonusAtributo(TA_DESTREZA, proto)));

  evento->set_rodadas(-1);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(12, BonusTotal(BonusAtributo(TA_FORCA, proto)));
  EXPECT_EQ(14, BonusTotal(BonusAtributo(TA_DESTREZA, proto)));
}

TEST(TesteItemMagico, TesteItemMagicoPoeTira) {
  EntidadeProto proto;
  AtribuiBaseAtributo(12, TA_DESTREZA, &proto);
  {
    auto* luvas = proto.mutable_tesouro()->add_luvas();
    luvas->set_id("luvas_destreza_2");
    luvas->set_em_uso(true);
  }
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(14, BonusTotal(BonusAtributo(TA_DESTREZA, proto)));
  {
    auto* luvas = proto.mutable_tesouro()->mutable_luvas(0);
    luvas->set_id("luvas_destreza_2");
    luvas->set_em_uso(false);
  }
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(12, BonusTotal(BonusAtributo(TA_DESTREZA, proto)));
}

TEST(TesteArmas, TestePistola) {
  EntidadeProto proto;
  {
    auto* ic = proto.add_info_classes();
    ic->set_nivel(1);
    ic->set_id("guerreiro");
  }
  {
    std::unique_ptr<Entidade> entidade (NovaEntidadeParaTestes(proto, g_tabelas));
    EXPECT_FALSE(TalentoComArma(g_tabelas.Arma("pistola"), entidade->Proto()));
    EXPECT_FALSE(TalentoComArma(g_tabelas.Arma("mosquete"), entidade->Proto()));
  }
  {
    proto.mutable_info_talentos()->add_outros()->set_id("usar_arma_exotica_fogo");
    std::unique_ptr<Entidade> entidade (NovaEntidadeParaTestes(proto, g_tabelas));
    EXPECT_TRUE(TalentoComArma(g_tabelas.Arma("pistola"), entidade->Proto()));
    EXPECT_TRUE(TalentoComArma(g_tabelas.Arma("mosquete"), entidade->Proto()));
  }
}


TEST(TesteArmas, TesteChicote) {
  auto modelo = g_tabelas.ModeloEntidade("Humano Plebeu 1");
  modelo.mutable_entidade()->mutable_info_talentos()->add_outros()->set_id("tiro_longo");
  // Corpo a corpo.
  auto* da = DadosAtaquePorGrupoOuCria("chicote", modelo.mutable_entidade());
  da->set_tipo_ataque("Ataque Corpo a Corpo");
  da->set_id_arma("chicote");
  // Distancia sem pericia.
  auto plebeu = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
  {
    // Corpo a corpo.
    const auto& da = DadosAtaquePorGrupo("chicote", plebeu->Proto());
    EXPECT_FLOAT_EQ(da.alcance_m(), 4.5f);
    EXPECT_EQ(da.incrementos(), 0);
  }
}

TEST(TesteArmas, TestePedrada) {
  EntidadeProto proto;
  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Pedrada (gigante)");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_arremesso()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_PROJETIL) << "acao: " << acao.DebugString();
}

TEST(TesteArmas, TesteFunda) {
  EntidadeProto proto;
  auto* da = proto.add_dados_ataque();
  da->set_id_arma("funda");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_arremesso()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_PROJETIL) << "acao: " << acao.DebugString();
}

TEST(TesteArmas, TesteEspadaLaminaAfiada) {
  EntidadeProto proto;
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("espada_longa");
    da->set_rotulo("espada longa 1");
  }
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("espada_longa");
    da->set_rotulo("espada longa 2");
  }
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_GE(proto.dados_ataque().size(), 2);
  EXPECT_EQ(proto.dados_ataque(0).margem_critico(), 19);

  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_LAMINA_AFIADA);
  evento->set_rodadas(1);
  evento->add_complementos_str("espada longa 1");
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_GE(proto.dados_ataque().size(), 2);
  EXPECT_EQ(proto.dados_ataque(0).margem_critico(), 17);
  EXPECT_EQ(proto.dados_ataque(1).margem_critico(), 19);
}

TEST(TesteArmas, TesteLaminaAfiadaAutomatico) {
  auto proto = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_LAMINA_AFIADA);
  evento->set_rodadas(1);
  auto orc = NovaEntidadeParaTestes(proto, g_tabelas);
  const auto& da = DadosAtaquePorGrupo("ataque_total_machado", orc->Proto(), 0);
  EXPECT_EQ(da.margem_critico(), 19);
}

TEST(TesteArmas, TesteArmaAbencoadaComPorrete) {
  EntidadeProto proto;
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("porrete");
    da->set_empunhadura(EA_ARMA_ESCUDO);
    proto.mutable_dados_defesa()->set_id_escudo("escudo_grande");
  }
  ASSERT_EQ(proto.dados_ataque().size(), 1);
  RecomputaDependencias(g_tabelas, &proto);

  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_ARMA_ABENCOADA);
  evento->set_rodadas(10);
  evento->set_id_unico(1);
  RecomputaDependencias(g_tabelas, &proto);
  EntidadeProto protod;
  *protod.mutable_dados_ataque() = proto.dados_ataque();
  ASSERT_EQ(proto.dados_ataque().size(), 3);// << protod.DebugString();
  EXPECT_EQ(proto.dados_ataque(0).grupo(), "shillelagh");
  EXPECT_EQ(proto.dados_ataque(0).rotulo(), "mão boa");
  EXPECT_EQ(proto.dados_ataque(0).empunhadura(), EA_ARMA_ESCUDO);
  EXPECT_EQ(proto.dados_ataque(0).dano(), "2d6+1");
}

TEST(TesteArmas, TesteArmaAbencoadaComBordao) {
  EntidadeProto proto;
  proto.set_tamanho(TM_PEQUENO);
  AtribuiBaseAtributo(12, TA_FORCA, &proto);
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("bordao");
    da->set_empunhadura(EA_MAO_BOA);
  }
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("bordao");
    da->set_empunhadura(EA_MAO_RUIM);
  }
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(proto.dados_ataque().size(), 3);

  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_ARMA_ABENCOADA);
  evento->set_rodadas(10);
  evento->set_id_unico(1);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(proto.dados_ataque().size(), 5);
  EXPECT_EQ(proto.dados_ataque(0).grupo(), "shillelagh");
  EXPECT_EQ(proto.dados_ataque(0).rotulo(), "mão boa");
  EXPECT_EQ(proto.dados_ataque(0).empunhadura(), EA_MAO_BOA);
  EXPECT_EQ(proto.dados_ataque(0).dano(), "1d8+2");
  EXPECT_EQ(proto.dados_ataque(1).grupo(), "shillelagh");
  EXPECT_EQ(proto.dados_ataque(1).rotulo(), "mão ruim");
  EXPECT_EQ(proto.dados_ataque(1).empunhadura(), EA_MAO_RUIM);
  EXPECT_EQ(proto.dados_ataque(1).dano(), "1d8+1");

  proto.set_tamanho(TM_MEDIO);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(proto.dados_ataque().size(), 5);
  EXPECT_EQ(proto.dados_ataque(0).rotulo(), "mão boa");
  EXPECT_EQ(proto.dados_ataque(0).empunhadura(), EA_MAO_BOA);
  EXPECT_EQ(proto.dados_ataque(0).dano(), "2d6+2");
  EXPECT_EQ(proto.dados_ataque(1).rotulo(), "mão ruim");
  EXPECT_EQ(proto.dados_ataque(1).empunhadura(), EA_MAO_RUIM);
  EXPECT_EQ(proto.dados_ataque(1).dano(), "2d6+1");

  evento->set_rodadas(-1);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(proto.dados_ataque().size(), 3);
  EXPECT_NE(proto.dados_ataque(0).grupo(), "shillelagh");
}

TEST(TesteArmas, TestePedraEncantadaComFunda) {
  EntidadeProto proto;
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(proto.dados_ataque().size(), 1);
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("funda");
    da->set_empunhadura(EA_ARMA_ESCUDO);
  }

  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_PEDRA_ENCANTADA);
  evento->set_rodadas(1);
  evento->set_id_unico(1);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(proto.dados_ataque().size(), 3);
  EXPECT_EQ(proto.dados_ataque(0).rotulo(), "pedra encantada com funda");
  EXPECT_EQ(proto.dados_ataque(0).empunhadura(), EA_ARMA_ESCUDO);
}

TEST(TesteArmas, TestePedraEncantada) {
  EntidadeProto proto;
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(proto.dados_ataque().size(), 1);

  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_PEDRA_ENCANTADA);
  evento->set_rodadas(1);
  evento->set_id_unico(1);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(proto.dados_ataque().size(), 2);
  EXPECT_EQ(proto.dados_ataque(0).rotulo(), "pedra encantada");
  EXPECT_EQ(proto.dados_ataque(0).dano(), "1d6+1");
  EXPECT_EQ(proto.dados_ataque(0).municao(), static_cast<unsigned int>(3));
}

TEST(TesteArmas, TesteMetamorfoseTorrida) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(20);
  AtribuiBaseAtributo(20, TA_INTELIGENCIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Mago");
  da->set_id_arma("metamorfose_torrida");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->alcance_m(), 15 * QUADRADOS_PARA_METROS) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  EXPECT_FALSE(da->acao().permite_ataque_vs_defesa()) << "DA completo: " << da->DebugString();
  const AcaoProto& acao = da->acao();
  const int nivel_conjurador = NivelConjurador(TipoAtaqueParaClasse(g_tabelas, da->tipo_ataque()), proto);
  ASSERT_FALSE(acao.efeitos_adicionais().empty());
  EXPECT_TRUE(da->acao().permite_salvacao()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->acao().ignora_resistencia_magia()) << "DA completo: " << da->DebugString();

  EntidadeProto proto_alvo;
  std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto_alvo, g_tabelas));
  std::vector<int> ids_unicos;
  ntf::Notificacao n;
  PreencheNotificacaoEventoEfeitoAdicional(proto.id(), std::nullopt, nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
  alvo->AtualizaParcial(n.entidade());
  ASSERT_FALSE(alvo->Proto().evento().empty());
  const auto& evento = alvo->Proto().evento(0);
  EXPECT_EQ(evento.id_efeito(), EFEITO_METAMORFOSE_TORRIDA);
  EXPECT_EQ(evento.id_unico(), 0);
  EXPECT_TRUE(evento.continuo());
  EXPECT_EQ(alvo->Proto().info_textura().id(), "toad.png");
}

TEST(TesteArmas, TestePalavraDoPoderCegar) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(20);
  AtribuiBaseAtributo(20, TA_INTELIGENCIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Mago");
  da->set_id_arma("palavra_poder_cegar");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->alcance_m(), 15 * QUADRADOS_PARA_METROS) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  EXPECT_FALSE(da->acao().permite_ataque_vs_defesa()) << "DA completo: " << da->DebugString();
  const AcaoProto& acao = da->acao();
  const int nivel_conjurador = NivelConjurador(TipoAtaqueParaClasse(g_tabelas, da->tipo_ataque()), proto);
  ASSERT_FALSE(acao.efeitos_adicionais().empty());
  EXPECT_FALSE(da->acao().permite_salvacao()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->acao().ignora_resistencia_magia()) << "DA completo: " << da->DebugString();

  EntidadeProto proto_alvo;
  proto_alvo.set_max_pontos_vida(201);
  {
    proto_alvo.set_pontos_vida(50);
    std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto_alvo, g_tabelas));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), std::nullopt, nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_CEGO);
    EXPECT_EQ(evento.id_unico(), 0);
    EXPECT_TRUE(evento.continuo());
  }
  {
    proto_alvo.set_pontos_vida(51);
    std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto_alvo, g_tabelas));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    g_dados_teste.push(1);
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), DadosIniciativa{.iniciativa=10, .modificador=3}, nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_CEGO);
    EXPECT_EQ(evento.id_unico(), 0);
    EXPECT_EQ(evento.rodadas(), 20);
    EXPECT_EQ(evento.iniciativa(), 10);
    EXPECT_EQ(evento.modificador_iniciativa(), 3);
  }
  {
    proto_alvo.set_pontos_vida(101);
    std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto_alvo, g_tabelas));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    g_dados_teste.push(1);
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), DadosIniciativa{.iniciativa=10, .modificador=3}, nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_CEGO);
    EXPECT_EQ(evento.id_unico(), 0);
    EXPECT_EQ(evento.rodadas(), 2);
    EXPECT_EQ(evento.iniciativa(), 10);
    EXPECT_EQ(evento.modificador_iniciativa(), 3);
  }
  {
    proto_alvo.set_pontos_vida(201);
    std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto_alvo, g_tabelas));
    EXPECT_FALSE(AcaoAfetaAlvo(acao, *alvo)) << "acao: " << acao.DebugString();
  }
}

TEST(TesteArmas, TestePalavraDoPoderAtordoar) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(20);
  AtribuiBaseAtributo(20, TA_INTELIGENCIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Mago");
  da->set_id_arma("palavra_poder_atordoar");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->alcance_m(), 15 * QUADRADOS_PARA_METROS) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  EXPECT_FALSE(da->acao().permite_ataque_vs_defesa()) << "DA completo: " << da->DebugString();
  const AcaoProto& acao = da->acao();
  const int nivel_conjurador = NivelConjurador(TipoAtaqueParaClasse(g_tabelas, da->tipo_ataque()), proto);
  ASSERT_FALSE(acao.efeitos_adicionais().empty());
  EXPECT_FALSE(da->acao().permite_salvacao()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->acao().ignora_resistencia_magia()) << "DA completo: " << da->DebugString();

  EntidadeProto proto_alvo;
  proto_alvo.set_max_pontos_vida(200);
  {
    proto_alvo.set_pontos_vida(50);
    std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto_alvo, g_tabelas));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    g_dados_teste.push(1);
    g_dados_teste.push(1);
    g_dados_teste.push(1);
    g_dados_teste.push(1);
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), DadosIniciativa{.iniciativa=5, .modificador=4}, nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_ATORDOADO);
    EXPECT_EQ(evento.id_unico(), 0);
    EXPECT_EQ(evento.rodadas(), 4);
    EXPECT_EQ(evento.iniciativa(), 5);
    EXPECT_EQ(evento.modificador_iniciativa(), 4);
  }
  {
    proto_alvo.set_pontos_vida(51);
    std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto_alvo, g_tabelas));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    g_dados_teste.push(1);
    g_dados_teste.push(1);
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), DadosIniciativa{.iniciativa=5, .modificador=4}, nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_ATORDOADO);
    EXPECT_EQ(evento.id_unico(), 0);
    EXPECT_EQ(evento.rodadas(), 2);
    EXPECT_EQ(evento.iniciativa(), 5);
    EXPECT_EQ(evento.modificador_iniciativa(), 4);
  }
  {
    proto_alvo.set_pontos_vida(101);
    std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto_alvo, g_tabelas));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    g_dados_teste.push(1);
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), DadosIniciativa{.iniciativa=5, .modificador=4}, nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_ATORDOADO);
    EXPECT_EQ(evento.id_unico(), 0);
    EXPECT_EQ(evento.rodadas(), 1);
    EXPECT_EQ(evento.iniciativa(), 5);
    EXPECT_EQ(evento.modificador_iniciativa(), 4);
  }
  {
    proto_alvo.set_pontos_vida(151);
    std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto_alvo, g_tabelas));
    EXPECT_FALSE(AcaoAfetaAlvo(acao, *alvo));
  }
}

TEST(TesteArmas, TestePoderDivino) {
  EntidadeProto proto;
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("clerigo");
    ic->set_nivel(10);
  }
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(5);
  }

  AtribuiBaseAtributo(10, TA_FORCA, &proto);
  AtribuiBaseAtributo(20, TA_SABEDORIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Clérigo");
  da->set_id_arma("poder_divino");
  RecomputaDependencias(g_tabelas, &proto);
  const AcaoProto& acao = da->acao();
  const int nivel_conjurador = NivelConjurador(TipoAtaqueParaClasse(g_tabelas, da->tipo_ataque()), proto);
  ASSERT_FALSE(acao.efeitos_adicionais().empty());

  std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto, g_tabelas));
  {
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), DadosIniciativa{.iniciativa=3, .modificador=6}, nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_PODER_DIVINO);
    alvo->AtualizaParcial(n.entidade());
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_FORCA, alvo->Proto())), 16);
    EXPECT_EQ(alvo->Proto().bba().base(), 15);
    EXPECT_EQ(BonusTotal(alvo->Proto().pontos_vida_temporarios_por_fonte()), 10);
    EXPECT_EQ(evento.iniciativa(), 3);
    EXPECT_EQ(evento.modificador_iniciativa(), 6);
  }
}

TEST(TesteArmas, TestePalavraDoPoderMatar) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(20);
  AtribuiBaseAtributo(20, TA_INTELIGENCIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Mago");
  da->set_id_arma("palavra_poder_matar");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->alcance_m(), 15 * QUADRADOS_PARA_METROS) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  EXPECT_FALSE(da->acao().permite_ataque_vs_defesa()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->acao().permite_salvacao()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->acao().ignora_resistencia_magia()) << "DA completo: " << da->DebugString();
  const AcaoProto& acao = da->acao();
  const int nivel_conjurador = NivelConjurador(TipoAtaqueParaClasse(g_tabelas, da->tipo_ataque()), proto);
  ASSERT_FALSE(acao.efeitos_adicionais().empty());

  EntidadeProto proto_alvo;
  proto_alvo.set_max_pontos_vida(200);
  {
    proto_alvo.set_pontos_vida(100);
    std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto_alvo, g_tabelas));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), std::nullopt, nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_MORTE);
    EXPECT_FALSE(evento.has_iniciativa());
    EXPECT_FALSE(evento.has_modificador_iniciativa());
  }
  {
    proto_alvo.set_pontos_vida(101);
    std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto_alvo, g_tabelas));
    EXPECT_FALSE(AcaoAfetaAlvo(acao, *alvo));
  }
}

TEST(TesteArmas, TesteRaioEnfraquecimento) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(3);
  AtribuiBaseAtributo(15, TA_INTELIGENCIA, &proto);
  proto.mutable_info_talentos()->add_outros()->set_id("tiro_longo");

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Mago");
  da->set_id_arma("raio_enfraquecimento");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->ataque_arremesso()) << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->alcance_m(), 6 * QUADRADOS_PARA_METROS) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_RAIO) << "acao: " << acao.DebugString();
  const int nivel_conjurador = NivelConjurador(TipoAtaqueParaClasse(g_tabelas, da->tipo_ataque()), proto);
  ASSERT_FALSE(acao.efeitos_adicionais().empty());

  EntidadeProto proto_alvo;
  AtribuiBaseAtributo(20, TA_FORCA, &proto_alvo);
  std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto_alvo, g_tabelas));
  {
    // Primeiro ataque.
    g_dados_teste.push(3);
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), std::nullopt, nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_PENALIDADE_FORCA);
    ASSERT_FALSE(evento.complementos().empty());
    EXPECT_EQ(evento.complementos(0), -4);
    EXPECT_EQ(evento.id_unico(), 0);
    // Ataque.
    alvo->AtualizaParcial(n.entidade());
    EXPECT_EQ(ModificadorAtributo(TA_FORCA, alvo->Proto()), ModificadorAtributo(16))
        << "bonus: " << BonusAtributo(TA_FORCA, alvo->Proto()).DebugString();
  }
  {
    // Segundo ataque: nao cumulativo, apenas o menor prevalece.
    g_dados_teste.push(1);
    std::vector<int> ids_unicos = IdsUnicosEntidade(*alvo);
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), std::nullopt, nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_PENALIDADE_FORCA);
    ASSERT_FALSE(evento.complementos().empty());
    EXPECT_EQ(evento.complementos(0), -2);
    EXPECT_EQ(evento.id_unico(), 1);
    // Ataque.
    alvo->AtualizaParcial(n.entidade());
    EXPECT_EQ(ModificadorAtributo(TA_FORCA, alvo->Proto()), ModificadorAtributo(16))
        << "bonus: " << BonusAtributo(TA_FORCA, alvo->Proto()).DebugString();
  }
  {
    // Terceiro ataque: nao cumulativo, apenas o maior prevalece.
    g_dados_teste.push(5);
    std::vector<int> ids_unicos = IdsUnicosEntidade(*alvo);
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), std::nullopt, nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_PENALIDADE_FORCA);
    ASSERT_FALSE(evento.complementos().empty());
    EXPECT_EQ(evento.complementos(0), -6);
    EXPECT_EQ(evento.id_unico(), 2);
    // Ataque.
    alvo->AtualizaParcial(n.entidade());
    EXPECT_EQ(ModificadorAtributo(TA_FORCA, alvo->Proto()), ModificadorAtributo(14))
        << "bonus: " << BonusAtributo(TA_FORCA, alvo->Proto()).DebugString();
  }
}

TEST(TesteArmas, TesteArcoLongo) {
  EntidadeProto proto;
  auto* da = proto.add_dados_ataque();
  da->set_id_arma("arco_longo");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_PROJETIL) << "acao: " << acao.DebugString();
}

TEST(TesteArmas, TesteEscudoComCravos) {
  auto modelo = g_tabelas.ModeloEntidade("Humano Plebeu 1");  // +1 forca
  modelo.mutable_entidade()->set_gerar_agarrar(false);
  modelo.mutable_entidade()->mutable_info_talentos()->add_outros()->set_id("usar_armas_comuns");
  modelo.mutable_entidade()->mutable_info_talentos()->add_outros()->set_id("usar_escudo");
  modelo.mutable_entidade()->mutable_info_talentos()->add_outros()->set_id("usar_armadura_leve");
  modelo.mutable_entidade()->mutable_dados_defesa()->set_id_armadura("couro_batido");
  modelo.mutable_entidade()->mutable_dados_defesa()->set_id_escudo("leve_aco");

  // Corpo a corpo.
  auto* da = DadosAtaquePorGrupoOuCria("espada_longa_com_escudo", modelo.mutable_entidade());
  da->set_id_arma("espada_longa");
  da->set_empunhadura(EA_MAO_BOA);
  da = DadosAtaquePorGrupoOuCria("espada_longa_com_escudo2", modelo.mutable_entidade());
  da->set_grupo("espada_longa_com_escudo");
  da->set_id_arma("escudo_pequeno_com_cravos");
  da->set_empunhadura(EA_MAO_RUIM);
  // Distancia sem pericia.
  {
    auto plebeu = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
    const auto& da_espada = DadosAtaquePorGrupo("espada_longa_com_escudo", plebeu->Proto(), 0);
    EXPECT_EQ(da_espada.bonus_ataque_final(), -3) << da_espada.DebugString();
    const auto& da_escudo = DadosAtaquePorGrupo("espada_longa_com_escudo", plebeu->Proto(), 1);
    EXPECT_EQ(da_escudo.bonus_ataque_final(), -7) << da_escudo.DebugString();
    plebeu->AtualizaAcaoPorGrupo("espada_longa_com_escudo");
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 13) << plebeu->Proto().dados_defesa().ca().DebugString();
  }
  {
    modelo.mutable_entidade()->mutable_info_talentos()->add_outros()->set_id("ataque_escudo_aprimorado");
    auto plebeu = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
    const auto& da_espada = DadosAtaquePorGrupo("espada_longa_com_escudo", plebeu->Proto(), 0);
    EXPECT_EQ(da_espada.bonus_ataque_final(), -3) << da_espada.DebugString();
    const auto& da_escudo = DadosAtaquePorGrupo("espada_longa_com_escudo", plebeu->Proto(), 1);
    EXPECT_EQ(da_escudo.bonus_ataque_final(), -7) << da_escudo.DebugString();
    plebeu->AtualizaAcaoPorGrupo("espada_longa_com_escudo");
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 14) << plebeu->Proto().dados_defesa().ca().DebugString();
  }
}

TEST(TesteArmas, TesteIdBase) {
  auto modelo = g_tabelas.ModeloEntidade("Humano Plebeu 1");
  // Corpo a corpo.
  auto* da = DadosAtaquePorGrupoOuCria("arco_longo_composto", modelo.mutable_entidade());
  da->set_tipo_ataque("Ataque a Distância");
  da->set_id_arma("arco_longo_composto");
  // Distancia sem pericia.
  {
    auto plebeu = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
    const auto& da = DadosAtaquePorGrupo("arco_longo_composto", plebeu->Proto());
    EXPECT_FLOAT_EQ(da.alcance_m(), 22 * QUADRADOS_PARA_METROS);
    EXPECT_EQ(da.incrementos(), 10);
    EXPECT_EQ(da.bonus_ataque_final(), -4);
  }
  {
    auto* t = modelo.mutable_entidade()->mutable_info_talentos()->add_outros();
    t->set_id("usar_armas_comuns");
    t->set_complemento("arco_longo");
    t = modelo.mutable_entidade()->mutable_info_talentos()->add_outros();
    t->set_id("foco_em_arma");
    t->set_complemento("arco_longo");
    auto plebeu = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
    const auto& da = DadosAtaquePorGrupo("arco_longo_composto", plebeu->Proto());
    EXPECT_FLOAT_EQ(da.alcance_m(), 22 * QUADRADOS_PARA_METROS);
    EXPECT_EQ(da.incrementos(), 10);
    EXPECT_EQ(da.bonus_ataque_final(), 1);
  }
}

TEST(TesteArmas, TesteBoleadeira) {
  EntidadeProto proto_ataque;
  auto* da = proto_ataque.add_dados_ataque();
  da->set_id_arma("boleadeira");
  RecomputaDependencias(g_tabelas, &proto_ataque);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_PROJETIL) << "acao: " << acao.DebugString();
}

TEST(TesteArmas, TesteEspada) {
  EntidadeProto proto_ataque;
  auto* da = proto_ataque.add_dados_ataque();
  da->set_id_arma("espada_curta");
  RecomputaDependencias(g_tabelas, &proto_ataque);
  EXPECT_FALSE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_CORPO_A_CORPO) << "acao: " << acao.DebugString();
}

TEST(TesteArmas, TesteDanoIgnoraSalvacao) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("clerigo");
  ic->set_nivel(3);
  AtribuiBaseAtributo(15, TA_SABEDORIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Clérigo");
  da->set_id_arma("explosao_sonora");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(da->dano_ignora_salvacao()) << "DA completo: " << da->DebugString();
}

TEST(TesteArmas, TesteMaosFlamejantes) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(3);
  AtribuiBaseAtributo(15, TA_INTELIGENCIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Mago");
  da->set_id_arma("maos_flamejantes");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(da->dano(), "3d4") << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->dificuldade_salvacao(), 13) << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->resultado_ao_salvar(), RS_MEIO);
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_DISPERSAO) << "acao: " << acao.DebugString();
}

TEST(TesteArmas, TesteMaosFlamejantesEspecialista) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(3);
  auto* fc = proto.add_feiticos_classes();
  fc->set_id_classe("mago");
  fc->set_especializacao("evocacao");
  AtribuiBaseAtributo(15, TA_INTELIGENCIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Mago");
  da->set_id_arma("maos_flamejantes");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(da->dano(), "3d4") << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->dificuldade_salvacao(), 14) << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->resultado_ao_salvar(), RS_MEIO);
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_DISPERSAO) << "acao: " << acao.DebugString();
}

TEST(TesteArmas, TesteRaio) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(3);
  AtribuiBaseAtributo(15, TA_INTELIGENCIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Mago");
  da->set_id_arma("raio_ardente");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(da->dano(), "4d6") << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->acao().permite_ataque_vs_defesa()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_toque());
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_RAIO) << "acao: " << acao.DebugString();
}

TEST(TesteArmas, TesteVeneno) {
  Modelos modelos;
  EntidadeProto proto = g_tabelas.ModeloEntidade("Centopéia Enorme").entidade();
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_FALSE(proto.dados_ataque().empty());
  EXPECT_TRUE(proto.dados_ataque(0).has_veneno());
}

TEST(TesteArmas, TesteArmaTemAcao) {
  EntidadeProto proto_ataque;
  auto* da = proto_ataque.add_dados_ataque();
  da->set_id_arma("adaga");
  RecomputaDependencias(g_tabelas, &proto_ataque);
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_PROJETIL);
}

TEST(TesteArmas, TesteProjetilAreaCompatibilidade) {
  EntidadeProto proto_ataque;
  {
    proto_ataque.set_gerar_agarrar(false);
    proto_ataque.set_tamanho(TM_GRANDE);
    auto* da = proto_ataque.add_dados_ataque();
    da->set_id_arma("fogo_alquimico");
    da->set_municao(3);
  }
  auto e = NovaEntidadeParaTestes(proto_ataque, g_tabelas);
  ASSERT_FALSE(e->Proto().dados_ataque().empty());
  const auto& da = e->Proto().dados_ataque(0);
  EXPECT_TRUE(da.ataque_toque());
  EXPECT_TRUE(da.ataque_distancia());
  EXPECT_FLOAT_EQ(da.alcance_m(), 3.0);
  EXPECT_EQ(da.incrementos(), 5);
  EXPECT_TRUE(da.has_acao());
  EXPECT_EQ(da.dano(), "1d6");
  const AcaoProto& acao = da.acao();
  EXPECT_EQ(acao.tipo(), ACAO_PROJETIL_AREA);
  EXPECT_EQ(e->Proto().tesouro().itens_mundanos().size(), 3);
  EXPECT_EQ(e->Proto().tesouro().itens_mundanos(0).id(), "fogo_alquimico");
}

TEST(TesteArmas, TesteProjetilArea) {
  EntidadeProto proto_ataque;
  AtribuiBaseAtributo(18, TA_FORCA, &proto_ataque);
  {
    proto_ataque.set_tamanho(TM_GRANDE);
    proto_ataque.set_gerar_agarrar(false);
    proto_ataque.mutable_tesouro()->add_itens_mundanos()->set_id("fogo_alquimico");
    proto_ataque.mutable_tesouro()->add_itens_mundanos()->set_id("fogo_alquimico");
    proto_ataque.mutable_tesouro()->add_itens_mundanos()->set_id("bomba");
  }
  auto e = NovaEntidadeParaTestes(proto_ataque, g_tabelas);
  // Dois fogos.
  {
    ASSERT_FALSE(e->Proto().dados_ataque().empty());
    const auto& da = e->Proto().dados_ataque(0);
    EXPECT_TRUE(da.ataque_toque());
    EXPECT_TRUE(da.ataque_distancia());
    EXPECT_TRUE(da.has_acao());
    EXPECT_EQ(da.dano(), "1d6");
    EXPECT_EQ(da.municao(), 2U);
    const AcaoProto& acao = da.acao();
    EXPECT_EQ(acao.tipo(), ACAO_PROJETIL_AREA);
  }
  // Bomba.
  {
    const auto& da = DadosAtaquePorGrupo("bomba", e->Proto());
    EXPECT_TRUE(da.ataque_toque());
    EXPECT_TRUE(da.ataque_distancia());
    EXPECT_TRUE(da.has_acao());
    EXPECT_EQ(da.dano(), "2d6");
    EXPECT_EQ(da.municao(), 1U);
    EXPECT_EQ(da.dificuldade_salvacao(), 15);
    const AcaoProto& acao = da.acao();
    EXPECT_TRUE(acao.respingo_causa_mesmo_dano());
    EXPECT_EQ(acao.tipo(), ACAO_PROJETIL_AREA);
  }

  // Consome 1 fogo.
  {
    ntf::Notificacao n;
    PreencheNotificacaoConsumoAtaque(*e, e->DadoCorrenteNaoNull(), &n, nullptr);
    e->AtualizaParcial(n.entidade());
    ASSERT_FALSE(e->Proto().dados_ataque().empty());
    const auto& da = e->Proto().dados_ataque(0);
    EXPECT_EQ(da.municao(), 1U);
  }
  // Consome o ultimo.
  {
    ntf::Notificacao n;
    PreencheNotificacaoConsumoAtaque(*e, e->DadoCorrenteNaoNull(), &n, nullptr);
    e->AtualizaParcial(n.entidade());
    ASSERT_EQ(e->Proto().dados_ataque().size(), 1);  // sobra a bomba.
  }
}

TEST(TesteCA, TesteLutaDefensiva) {
  std::unique_ptr<Entidade> e;
  {
    EntidadeProto proto;
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("fogo_alquimico");
    e = NovaEntidadeParaTestes(proto, g_tabelas);
  }

  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
  auto n = PreencheNotificacaoLutarDefensivamente(true, *e);
  e->AtualizaParcial(n.entidade());
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 12);
  EXPECT_FALSE(e->Proto().dados_ataque().empty());
  for (const auto& da : e->Proto().dados_ataque()) {
    EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "luta_defensiva", da.bonus_ataque()), -4) << ", da: " << da.DebugString();
  }
  e->AtualizaParcial(n.entidade_antes());
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
  for (const auto& da : e->Proto().dados_ataque()) {
    EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "luta_defensiva", da.bonus_ataque()), 0) << ", da: " << da.DebugString();
  }
}

TEST(TesteCA, TesteLutaDefensivaEspecializacaoEmCombate) {
  std::unique_ptr<Entidade> e;
  {
    EntidadeProto proto;
    AtribuiBaseAtributo(13, TA_INTELIGENCIA, &proto);
    auto* ic = proto.add_info_classes();
    ic->set_id("guerreiro");
    ic->set_nivel(2);
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("fogo_alquimico");
    auto* t = proto.mutable_info_talentos()->add_outros();
    t->set_id("especializacao_em_combate");
    t->set_complemento("4");
    e = NovaEntidadeParaTestes(proto, g_tabelas);
  }

  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
  auto n = PreencheNotificacaoLutarDefensivamente(true, *e);
  e->AtualizaParcial(n.entidade());
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 12);
  EXPECT_FALSE(e->Proto().dados_ataque().empty());
  for (const auto& da : e->Proto().dados_ataque()) {
    EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "luta_defensiva", da.bonus_ataque()), -2) << ", da: " << da.DebugString();
  }
  e->AtualizaParcial(n.entidade_antes());
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
  for (const auto& da : e->Proto().dados_ataque()) {
    EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "luta_defensiva", da.bonus_ataque()), 0) << ", da: " << da.DebugString();
  }
}

TEST(TesteCA, TesteLutaDefensivaEspecializacaoEmCombateSemComplementos) {
  std::unique_ptr<Entidade> e;
  {
    EntidadeProto proto;
    AtribuiBaseAtributo(13, TA_INTELIGENCIA, &proto);
    auto* ic = proto.add_info_classes();
    ic->set_id("guerreiro");
    ic->set_nivel(3);
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("fogo_alquimico");
    auto* t = proto.mutable_info_talentos()->add_outros();
    t->set_id("especializacao_em_combate");
    e = NovaEntidadeParaTestes(proto, g_tabelas);
  }

  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
  auto n = PreencheNotificacaoLutarDefensivamente(true, *e);
  e->AtualizaParcial(n.entidade());
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 12);
  EXPECT_FALSE(e->Proto().dados_ataque().empty());
  for (const auto& da : e->Proto().dados_ataque()) {
    EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "luta_defensiva", da.bonus_ataque()), -2) << ", da: " << da.DebugString();
  }
  e->AtualizaParcial(n.entidade_antes());
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
  for (const auto& da : e->Proto().dados_ataque()) {
    EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "luta_defensiva", da.bonus_ataque()), 0) << ", da: " << da.DebugString();
  }
}

TEST(TesteCA, TesteLutaDefensivaEspecializacaoEmCombateComplementoMaiorQueBba) {
  std::unique_ptr<Entidade> e;
  {
    EntidadeProto proto;
    AtribuiBaseAtributo(13, TA_INTELIGENCIA, &proto);
    auto* ic = proto.add_info_classes();
    ic->set_id("guerreiro");
    ic->set_nivel(3);
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("fogo_alquimico");
    auto* t = proto.mutable_info_talentos()->add_outros();
    t->set_id("especializacao_em_combate");
    t->set_complemento("4");
    e = NovaEntidadeParaTestes(proto, g_tabelas);
  }

  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
  auto n = PreencheNotificacaoLutarDefensivamente(true, *e);
  e->AtualizaParcial(n.entidade());
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 13);
  EXPECT_FALSE(e->Proto().dados_ataque().empty());
  for (const auto& da : e->Proto().dados_ataque()) {
    EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "luta_defensiva", da.bonus_ataque()), -3) << ", da: " << da.DebugString();
  }
  e->AtualizaParcial(n.entidade_antes());
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
  for (const auto& da : e->Proto().dados_ataque()) {
    EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "luta_defensiva", da.bonus_ataque()), 0) << ", da: " << da.DebugString();
  }
}

TEST(TesteCA, TesteLutaDefensivaEspecializacaoEmCombateComplementoMaiorQue5) {
  std::unique_ptr<Entidade> e;
  {
    EntidadeProto proto;
    AtribuiBaseAtributo(13, TA_INTELIGENCIA, &proto);
    auto* ic = proto.add_info_classes();
    ic->set_id("guerreiro");
    ic->set_nivel(10);
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("fogo_alquimico");
    auto* t = proto.mutable_info_talentos()->add_outros();
    t->set_id("especializacao_em_combate");
    t->set_complemento("6");
    e = NovaEntidadeParaTestes(proto, g_tabelas);
  }

  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
  auto n = PreencheNotificacaoLutarDefensivamente(true, *e);
  e->AtualizaParcial(n.entidade());
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 15);
  EXPECT_FALSE(e->Proto().dados_ataque().empty());
  for (const auto& da : e->Proto().dados_ataque()) {
    EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "luta_defensiva", da.bonus_ataque()), -5) << ", da: " << da.DebugString();
  }
  e->AtualizaParcial(n.entidade_antes());
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
  for (const auto& da : e->Proto().dados_ataque()) {
    EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "luta_defensiva", da.bonus_ataque()), 0) << ", da: " << da.DebugString();
  }
}

TEST(TesteCA, TesteLutaDefensivaComAcrobacias) {
  std::unique_ptr<Entidade> e;
  {
    EntidadeProto proto;
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("fogo_alquimico");
    auto* ic = proto.add_info_classes();
    ic->set_id("ladino");
    ic->set_nivel(2);
    auto* ip = proto.add_info_pericias();
    ip->set_id("acrobacias");
    ip->set_pontos(5);
    e = NovaEntidadeParaTestes(proto, g_tabelas);
  }

  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
  auto n = PreencheNotificacaoLutarDefensivamente(true, *e);
  e->AtualizaParcial(n.entidade());
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 13);
  EXPECT_FALSE(e->Proto().dados_ataque().empty());
  for (const auto& da : e->Proto().dados_ataque()) {
    EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "luta_defensiva", da.bonus_ataque()), -4) << ", da: " << da.DebugString();
  }
  e->AtualizaParcial(n.entidade_antes());
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
  for (const auto& da : e->Proto().dados_ataque()) {
    EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "luta_defensiva", da.bonus_ataque()), 0) << ", da: " << da.DebugString();
  }
}

TEST(TesteCA, TesteDefesaTotal) {
  std::unique_ptr<Entidade> e;
  {
    EntidadeProto proto;
    e = NovaEntidadeParaTestes(proto, g_tabelas);
  }
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
  auto n = PreencheNotificacaoDefesaTotal(true, e->Proto());
  e->AtualizaParcial(n.entidade());
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 14);
  e->AtualizaParcial(n.entidade_antes());
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
}

TEST(TesteCA, TesteDefesaTotalComAcrobacia) {
  std::unique_ptr<Entidade> e;
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("ladino");
    ic->set_nivel(2);
    auto* ip = proto.add_info_pericias();
    ip->set_id("acrobacias");
    ip->set_pontos(5);
    e = NovaEntidadeParaTestes(proto, g_tabelas);
  }
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
  auto n = PreencheNotificacaoDefesaTotal(true, e->Proto());
  e->AtualizaParcial(n.entidade());
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 16);
  e->AtualizaParcial(n.entidade_antes());
  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
}

TEST(TesteCA, TesteDestrezaCA) {
  EntidadeProto proto;
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(DestrezaNaCA(proto));

  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_CEGO);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_FALSE(DestrezaNaCA(proto));

  proto.mutable_info_talentos()->add_gerais()->set_id("lutar_as_cegas");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(DestrezaNaCA(proto));

  DadosAtaque da;
  da.set_ataque_distancia(true);
  EXPECT_FALSE(DestrezaNaCAContraAtaque(&da, proto));
}

TEST(TesteCA, TesteDestrezaCALadinoEsquivaSobrenatural) {
  const auto& modelo_ladino = g_tabelas.ModeloEntidade("Humano Ladino 5");
  EntidadeProto proto = modelo_ladino.entidade();
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(DestrezaNaCA(proto));
  EXPECT_EQ(CATotal(proto, /*escudo=*/true, Bonus()), 18);
  EXPECT_EQ(CASurpreso(proto, /*escudo=*/true, Bonus()), 18);

  proto.set_surpreso(true);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(DestrezaNaCA(proto));

  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_CEGO);
    RecomputaDependencias(g_tabelas, &proto);
    EXPECT_TRUE(DestrezaNaCA(proto));
  }

  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_PARALISIA);
    RecomputaDependencias(g_tabelas, &proto);
    EXPECT_FALSE(DestrezaNaCA(proto));
  }
}

TEST(TesteResistenciaMagia, TesteResistenciaMagia) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(3);
  {
    // Sera ignorada.
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_RESISTENCIA_MAGIA);
    evento->set_rodadas(100);
    evento->add_complementos(3);
  }
  proto.mutable_dados_defesa()->set_resistencia_magia_racial(10);

  std::unique_ptr<Entidade> ea(NovaEntidadeParaTestes(proto, g_tabelas));
  std::unique_ptr<Entidade> ed(NovaEntidadeParaTestes(proto, g_tabelas));

  g_dados_teste.push(7);
  bool sucesso;
  std::string texto;
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(g_tabelas, DadosAtaque::default_instance(), *ea, *ed);
  EXPECT_TRUE(sucesso) << texto;

  g_dados_teste.push(6);
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(g_tabelas, DadosAtaque::default_instance(), *ea, *ed);
  EXPECT_FALSE(sucesso) << texto;

  proto.mutable_info_talentos()->add_gerais()->set_id("magia_penetrante");
  std::unique_ptr<Entidade> eamp(NovaEntidadeParaTestes(proto, g_tabelas));
  g_dados_teste.push(5);
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(g_tabelas, DadosAtaque::default_instance(), *eamp, *ed);
  EXPECT_TRUE(sucesso) << texto;

  proto.mutable_info_talentos()->add_gerais()->set_id("magia_penetrante_maior");
  std::unique_ptr<Entidade> eampm(NovaEntidadeParaTestes(proto, g_tabelas));
  g_dados_teste.push(3);
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(g_tabelas, DadosAtaque::default_instance(), *eampm, *ed);
  EXPECT_TRUE(sucesso) << texto;
}

TEST(TesteResistenciaMagia, TesteResistenciaMagia2) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(3);
  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_RESISTENCIA_MAGIA);
    evento->set_rodadas(100);
    evento->add_complementos(10);
  }
  // Sera ignorada.
  proto.mutable_dados_defesa()->set_resistencia_magia_racial(3);

  std::unique_ptr<Entidade> ea(NovaEntidadeParaTestes(proto, g_tabelas));
  std::unique_ptr<Entidade> ed(NovaEntidadeParaTestes(proto, g_tabelas));

  g_dados_teste.push(7);
  bool sucesso;
  std::string texto;
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(g_tabelas, DadosAtaque::default_instance(), *ea, *ed);
  EXPECT_TRUE(sucesso) << texto;

  g_dados_teste.push(6);
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(g_tabelas, DadosAtaque::default_instance(), *ea, *ed);
  EXPECT_FALSE(sucesso) << texto;

  proto.mutable_info_talentos()->add_gerais()->set_id("magia_penetrante");
  std::unique_ptr<Entidade> eamp(NovaEntidadeParaTestes(proto, g_tabelas));
  g_dados_teste.push(5);
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(g_tabelas, DadosAtaque::default_instance(), *eamp, *ed);
  EXPECT_TRUE(sucesso) << texto;

  proto.mutable_info_talentos()->add_gerais()->set_id("magia_penetrante_maior");
  std::unique_ptr<Entidade> eampm(NovaEntidadeParaTestes(proto, g_tabelas));
  g_dados_teste.push(3);
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(g_tabelas, DadosAtaque::default_instance(), *eampm, *ed);
  EXPECT_TRUE(sucesso) << texto;
}

TEST(TesteResistenciaMagia, TesteResistenciaMagiaTramaSombras) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(3);
  proto.mutable_dados_defesa()->set_resistencia_magia_racial(10);

  std::unique_ptr<Entidade> ea(NovaEntidadeParaTestes(proto, g_tabelas));
  std::unique_ptr<Entidade> ed(NovaEntidadeParaTestes(proto, g_tabelas));

  g_dados_teste.push(7);
  bool sucesso;
  std::string texto;
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(g_tabelas, DadosAtaque::default_instance(), *ea, *ed);
  EXPECT_TRUE(sucesso) << texto;

  g_dados_teste.push(6);
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(g_tabelas, DadosAtaque::default_instance(), *ea, *ed);
  EXPECT_FALSE(sucesso) << texto;

  proto.mutable_info_talentos()->add_gerais()->set_id("magia_trama_sombras");
  std::unique_ptr<Entidade> eats(NovaEntidadeParaTestes(proto, g_tabelas));
  g_dados_teste.push(6);
  DadosAtaque da;
  da.set_id_arma("imobilizar_pessoa");  // encantamento
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(g_tabelas, da, *eats, *ed);
  EXPECT_TRUE(sucesso) << texto;

  g_dados_teste.push(7);
  da.set_id_arma("missil_magico");  // evocacao
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(g_tabelas, da, *eats, *ed);
  EXPECT_FALSE(sucesso) << texto;
}

TEST(TestePergaminho, MaosFlamejantes) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(3);
  AtribuiBaseAtributo(15, TA_INTELIGENCIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Pergaminho Arcano");
  da->set_id_arma("maos_flamejantes");
  da->set_nivel_conjurador_pergaminho(1);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(da->tipo_pergaminho(), TM_ARCANA);
  EXPECT_EQ(da->dano(), "1d4") << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->dificuldade_salvacao(), 11) << "DA completo: " << da->DebugString();
}

TEST(TestePergaminho, FlechaAcida) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(3);
  AtribuiBaseAtributo(12, TA_INTELIGENCIA, &proto);
  AtribuiBaseAtributo(15, TA_DESTREZA, &proto);  // +2

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Pergaminho Arcano");
  da->set_id_arma("flecha_acida");
  da->set_nivel_conjurador_pergaminho(3);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(da->tipo_pergaminho(), TM_ARCANA);
  EXPECT_EQ(da->dano(), "2d4") << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->bonus_ataque_final(), 3);  // 1 bba + 2 des.
}

TEST(TestePergaminho, ValoresTabelados) {
  {
    const auto& pergaminho = g_tabelas.PergaminhoArcano("identificacao");
    EXPECT_EQ(pergaminho.custo_po(), 125);
    EXPECT_EQ(pergaminho.nivel_conjurador(), 1);
    EXPECT_EQ(pergaminho.modificador_atributo(), 0);
  }
  {
    const auto& pergaminho = g_tabelas.PergaminhoArcano("missil_magico");
    EXPECT_EQ(pergaminho.custo_po(), 25);
    EXPECT_EQ(pergaminho.nivel_conjurador(), 1);
    EXPECT_EQ(pergaminho.modificador_atributo(), 0);
  }
  {
    const auto& pergaminho = g_tabelas.PergaminhoArcano("nublar");
    EXPECT_EQ(pergaminho.custo_po(), 150);
    EXPECT_EQ(pergaminho.nivel_conjurador(), 3);
    EXPECT_EQ(pergaminho.modificador_atributo(), 1);
  }
}

TEST(TesteElixir, PodeUsarCuspirFogo) {
  EntidadeProto proto;
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_ELIXIR_CUSPIR_FOGO);
  evento->set_rodadas(199);
  auto* ic = proto.add_info_classes();
  ic->set_id("clerigo");
  ic->set_nivel(3);
  auto clerigo = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("cuspir fogo", clerigo->Proto());
    auto [pode, texto] = PodeLancarItemMagico(g_tabelas, clerigo->Proto(), da);
    EXPECT_TRUE(pode) << texto;
  }
}

TEST(TesteVarinha, PodeUsar) {
  EntidadeProto proto;
  AtribuiBaseAtributo(11, TA_SABEDORIA, &proto);
  proto.mutable_tesouro()->add_varinhas()->set_id("curar_ferimentos_leves");
  proto.mutable_tesouro()->add_varinhas()->set_id("missil_magico_1");
  auto* ic = proto.add_info_classes();
  ic->set_id("clerigo");
  ic->set_nivel(3);
  auto druida = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("Varinha curar ferimentos leves", druida->Proto());
    auto [pode, texto] = PodeLancarItemMagico(g_tabelas, druida->Proto(), da);
    EXPECT_TRUE(pode) << texto;
  }
  {
    const auto& da = DadosAtaquePorGrupo("Varinha míssil mágico (1)", druida->Proto());
    auto [pode, texto] = PodeLancarItemMagico(g_tabelas, druida->Proto(), da);
    EXPECT_FALSE(pode) << texto;
  }
}

TEST(TesteVarinha, PodeUsarIndependenteNivelOuAtributo) {
  EntidadeProto proto;
  proto.mutable_tesouro()->add_varinhas()->set_id("missil_magico_2");
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(1);
  auto mago = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("Varinha mísseis mágicos (2)", mago->Proto());
    auto [pode, texto] = PodeLancarItemMagico(g_tabelas, mago->Proto(), da);
    EXPECT_TRUE(pode) << texto;
  }
}

TEST(TestePergaminho, PodeLancar) {
  EntidadeProto proto;
  AtribuiBaseAtributo(11, TA_SABEDORIA, &proto);
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("druida");
    ic->set_nivel(7);
    {
      // Arcano: barrado.
      auto* da = proto.add_dados_ataque();
      da->set_tipo_ataque("Pergaminho Arcano");
      da->set_id_arma("curar_ferimentos_leves");
    }
    {
      // Fora da lista.
      auto* da = proto.add_dados_ataque();
      da->set_tipo_ataque("Pergaminho Divino");
      da->set_tipo_pergaminho(TM_DIVINA);
      da->set_id_arma("luz_cegante");
    }
    {
      // Atributo invalido.
      auto* da = proto.add_dados_ataque();
      da->set_tipo_ataque("Pergaminho Divino");
      da->set_id_arma("curar_ferimentos_moderados");
      da->set_modificador_atributo_pergaminho(1);
    }
    {
      // Esse vai.
      auto* da = proto.add_dados_ataque();
      da->set_tipo_ataque("Pergaminho Divino");
      da->set_id_arma("curar_ferimentos_leves");
      da->set_modificador_atributo_pergaminho(0);
    }
  }
  RecomputaDependencias(g_tabelas, &proto);
  // Tipo errado.
  EXPECT_FALSE(PodeLancarItemMagico(g_tabelas, proto, proto.dados_ataque(0)).first);
  // Fora da lista.
  EXPECT_FALSE(PodeLancarItemMagico(g_tabelas, proto, proto.dados_ataque(1)).first);
  // Atributo invalido.
  EXPECT_FALSE(PodeLancarItemMagico(g_tabelas, proto, proto.dados_ataque(2)).first);

  EXPECT_TRUE(PodeLancarItemMagico(g_tabelas, proto, proto.dados_ataque(3)).first);
}

TEST(TestePergaminho, PodeLancarRangerCurarFerimentosLeves) {
  EntidadeProto proto;
  AtribuiBaseAtributo(11, TA_SABEDORIA, &proto);
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("ranger");
    ic->set_nivel(9);
    {
      // Arcano: barrado.
      auto* da = proto.add_dados_ataque();
      da->set_tipo_ataque("Pergaminho Divino");
      da->set_id_arma("curar_ferimentos_leves");
    }
  }
  RecomputaDependencias(g_tabelas, &proto);
  // Tipo errado.
  auto [pode_lancar, texto] = PodeLancarItemMagico(g_tabelas, proto, proto.dados_ataque(0));
  EXPECT_FALSE(pode_lancar) << texto;
}

TEST(TestePergaminho, PodeLancarRangerClerigoCurarFerimentosLeves) {
  EntidadeProto proto;
  proto.set_classe_feitico_ativa("clerigo");
  AtribuiBaseAtributo(11, TA_SABEDORIA, &proto);
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("ranger");
    ic->set_nivel(9);
    ic = proto.add_info_classes();
    ic->set_id("clerigo");
    ic->set_nivel(1);
    {
      // Arcano: barrado.
      auto* da = proto.add_dados_ataque();
      da->set_tipo_ataque("Pergaminho Divino");
      da->set_id_arma("curar_ferimentos_leves");
    }
  }
  RecomputaDependencias(g_tabelas, &proto);
  // Tipo errado.
  auto [pode_lancar, texto] = PodeLancarItemMagico(g_tabelas, proto, proto.dados_ataque(0));
  EXPECT_TRUE(pode_lancar) << texto;
}

TEST(TestePergaminho, PodeLancarMagoClerigoCurarFerimentosLeves) {
  EntidadeProto proto;
  proto.set_classe_feitico_ativa("mago");
  AtribuiBaseAtributo(11, TA_SABEDORIA, &proto);
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(9);
    ic = proto.add_info_classes();
    ic->set_id("clerigo");
    ic->set_nivel(1);
    {
      // Arcano: barrado.
      auto* da = proto.add_dados_ataque();
      da->set_tipo_ataque("Pergaminho Divino");
      da->set_id_arma("curar_ferimentos_leves");
    }
  }
  RecomputaDependencias(g_tabelas, &proto);
  // Tipo errado.
  auto [pode_lancar, texto] = PodeLancarItemMagico(g_tabelas, proto, proto.dados_ataque(0));
  EXPECT_TRUE(pode_lancar) << texto;
}

TEST(TestePergaminho, PodeLancarDominio) {
  EntidadeProto proto;
  AtribuiBaseAtributo(11, TA_SABEDORIA, &proto);
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("clerigo");
    ic->set_nivel(1);
    {
      auto* fc = ent::FeiticosClasse("clerigo", &proto);
      fc->add_dominios("conhecimento");
    }
    {
      auto* da = proto.add_dados_ataque();
      da->set_tipo_ataque("Pergaminho Divino");
      da->set_id_arma("detectar_portas_secretas");
    }
    {
      auto* da = proto.add_dados_ataque();
      da->set_tipo_ataque("Pergaminho Divino");
      da->set_id_arma("aumentar_pessoa");
    }
  }
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(PodeLancarItemMagico(g_tabelas, proto, proto.dados_ataque(0)).first);
  EXPECT_FALSE(PodeLancarItemMagico(g_tabelas, proto, proto.dados_ataque(1)).first);
}

TEST(TestePergaminho, PodeLancarEscolaProibida) {
  EntidadeProto proto;
  AtribuiBaseAtributo(18, TA_INTELIGENCIA, &proto);
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(10);
    {
      auto* fc = ent::FeiticosClasse("mago", &proto);
      fc->set_especializacao("evocacao");
      fc->add_escolas_proibidas("encantamento");
      fc->add_escolas_proibidas("transmutacao");
    }
    {
      auto* da = proto.add_dados_ataque();
      da->set_tipo_ataque("Pergaminho Arcano");
      da->set_id_arma("missil_magico");
    }
    {
      auto* da = proto.add_dados_ataque();
      da->set_tipo_ataque("Pergaminho Arcano");
      da->set_id_arma("aumentar_pessoa");
    }
    {
      auto* da = proto.add_dados_ataque();
      da->set_tipo_ataque("Pergaminho Arcano");
      da->set_id_arma("enfeiticar_pessoa");
    }
  }
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(PodeLancarItemMagico(g_tabelas, proto, proto.dados_ataque(0)).first);
  EXPECT_FALSE(PodeLancarItemMagico(g_tabelas, proto, proto.dados_ataque(1)).first);
  EXPECT_FALSE(PodeLancarItemMagico(g_tabelas, proto, proto.dados_ataque(2)).first);
}

TEST(TestePergaminho, TesteLancarPergaminhoSemRisco) {
  EntidadeProto proto;
  AtribuiBaseAtributo(11, TA_SABEDORIA, &proto);
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("druida");
    ic->set_nivel(7);
    {
      auto* da = proto.add_dados_ataque();
      da->set_tipo_ataque("Pergaminho Divino");
      da->set_id_arma("curar_ferimentos_leves");
      da->set_modificador_atributo_pergaminho(0);
    }
  }
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(TesteLancarPergaminho(g_tabelas, proto, proto.dados_ataque(0)).ok);
}

TEST(TestePergaminho, TesteLancarPergaminhoSucesso) {
  EntidadeProto proto;
  AtribuiBaseAtributo(11, TA_SABEDORIA, &proto);
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("druida");
    ic->set_nivel(1);
    {
      auto* da = proto.add_dados_ataque();
      da->set_tipo_ataque("Pergaminho Divino");
      da->set_id_arma("curar_ferimentos_leves");
      da->set_nivel_conjurador_pergaminho(2);
    }
  }
  RecomputaDependencias(g_tabelas, &proto);
  g_dados_teste.push(3);
  auto res = TesteLancarPergaminho(g_tabelas, proto, proto.dados_ataque(0));
  EXPECT_TRUE(res.ok) << res.texto;
}

TEST(TestePergaminho, TesteLancarPergaminhoFalhaSemFiasco) {
  EntidadeProto proto;
  AtribuiBaseAtributo(11, TA_SABEDORIA, &proto);
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("druida");
    ic->set_nivel(1);
    {
      auto* da = proto.add_dados_ataque();
      da->set_tipo_ataque("Pergaminho Divino");
      da->set_id_arma("curar_ferimentos_leves");
      da->set_nivel_conjurador_pergaminho(2);
    }
  }
  RecomputaDependencias(g_tabelas, &proto);
  g_dados_teste.push(1);  // 1 + 1 de nivel = 2 < 3 (nivel conjurador pergaminho  1);
  g_dados_teste.push(5);  // 4 + 0 de sabedoria >= 5.
  auto res = TesteLancarPergaminho(g_tabelas, proto, proto.dados_ataque(0));
  EXPECT_FALSE(res.ok) << res.texto;
  EXPECT_FALSE(res.fiasco) << res.texto;
}

TEST(TestePergaminho, TesteLancarPergaminhoFalhaComFiasco) {
  EntidadeProto proto;
  AtribuiBaseAtributo(11, TA_SABEDORIA, &proto);
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("druida");
    ic->set_nivel(1);
    {
      auto* da = proto.add_dados_ataque();
      da->set_tipo_ataque("Pergaminho Divino");
      da->set_id_arma("curar_ferimentos_leves");
      da->set_nivel_conjurador_pergaminho(2);
    }
  }
  RecomputaDependencias(g_tabelas, &proto);
  g_dados_teste.push(1);
  g_dados_teste.push(4);  // 4 + 0 sabedoria < 5.
  auto res = TesteLancarPergaminho(g_tabelas, proto, proto.dados_ataque(0));
  EXPECT_FALSE(res.ok) << res.texto;
  EXPECT_TRUE(res.fiasco) << res.texto;
}

TEST(TesteTalentoPericias, TesteVitalidade) {
  auto proto_orc = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  proto_orc.mutable_info_talentos()->add_outros()->set_id("vitalidade");
  proto_orc.mutable_info_talentos()->add_outros()->set_id("vitalidade");
  auto orc = NovaEntidadeParaTestes(proto_orc, g_tabelas);
  // 6 vitalidade, 7 constituicao.
  EXPECT_EQ(BonusTotal(orc->Proto().bonus_dados_vida()), 13) << orc->Proto().bonus_dados_vida().DebugString() << " " << orc->NivelPersonagem();
}

TEST(TesteTalentoPericias, TesteDadosVidaAutomatico) {
  EntidadeProto proto;
  AtribuiBaseAtributo(12, TA_CONSTITUICAO, &proto);
  {
    auto* bonus = BonusAtributo(TA_INTELIGENCIA, &proto);
    AtribuiBonus(14, TB_BASE, "base", bonus);
    AtribuiBonus(6, TB_RACIAL, "racial", bonus);
    AtribuiBonus(2, TB_NIVEL, "nivel", bonus);  // ignorado
  }
  AtribuiBaseAtributo(16, TA_CARISMA, &proto);
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(4);
  proto.mutable_info_talentos()->add_gerais()->set_id("vitalidade");
  proto.mutable_info_talentos()->add_gerais()->set_id("mente_sobre_materia");
  proto.mutable_info_talentos()->add_outros()->set_id("elevar_magia");
  proto.mutable_info_talentos()->add_outros()->set_id("magia_silenciosa");
  proto.set_dados_vida_automatico(true);
  auto e = NovaEntidadeParaTestes(proto, g_tabelas);
  EXPECT_EQ(e->Proto().dados_vida(), "4+3d4+7+3+3");
}


namespace {
const AcaoProto::PorEntidade& PrimeiraEntidadeOuPadrao(const AcaoProto& acao) {
  if (acao.por_entidade().empty()) return AcaoProto::PorEntidade::default_instance();
  return acao.por_entidade(0);
}
}  // namespace

TEST(TesteTalentoPericias, TesteIntimidacao) {
  auto proto_vrock = g_tabelas.ModeloEntidade("Demônio Vrock").entidade();
  proto_vrock.set_id(1);
  proto_vrock.set_selecionavel_para_jogador(true);
  auto* vrock = NovaEntidadeParaTestes(proto_vrock, g_tabelas).release();
  auto proto_orc = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  proto_orc.set_id(2);
  proto_orc.set_selecionavel_para_jogador(true);
  auto* evento = proto_orc.add_evento();
  evento->set_id_efeito(EFEITO_SABEDORIA_CORUJA);
  evento->set_rodadas(1);
  AtribuiBonus(5, TB_SEM_NOME, "teste", proto_orc.mutable_dados_defesa()->mutable_bonus_salvacao_medo());
  auto* orc = NovaEntidadeParaTestes(proto_orc, g_tabelas).release();

  ntf::Notificacao grupo_desfazer;
  TabuleiroTeste tabuleiro({vrock, orc});
  ntf::Notificacao notificacao_pericia;
  notificacao_pericia.set_local(true);
  *notificacao_pericia.mutable_entidade() = vrock->Proto();
  tabuleiro.EntraModoPericia("intimidacao", notificacao_pericia);
  g_dados_teste.push(10);
  g_dados_teste.push(10);
  tabuleiro.TrataBotaoPericiaPressionadoPosPicking(2, OBJ_ENTIDADE);
  const auto& notificacoes = g_central.NotificacoesRemotas();
  ASSERT_GE(notificacoes.size(), 4ULL);
  EXPECT_NE(PrimeiraEntidadeOuPadrao(notificacoes[notificacoes.size() - 4]->acao()).texto().find("10 +16 = 26"), std::string::npos) << notificacoes[notificacoes.size() - 4]->DebugString();
  EXPECT_NE(PrimeiraEntidadeOuPadrao(notificacoes[notificacoes.size() - 3]->acao()).texto().find("10 +7 +2 +5 = 24"), std::string::npos) << notificacoes[notificacoes.size() - 3]->DebugString();
  orc->AtualizaParcial(notificacoes[notificacoes.size() - 2]->entidade());
  EXPECT_TRUE(orc->PossuiEfeito(EFEITO_ABALADO));
}

TEST(TesteTalentoPericias, TesteCombateMontado) {
  auto proto_orc = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  proto_orc.set_id(1);
  proto_orc.mutable_info_talentos()->add_outros()->set_id("combate_montado");
  proto_orc.set_montado_em(2);
  auto* montador = NovaEntidadeParaTestes(proto_orc, g_tabelas).release();
  proto_orc.set_id(2);
  proto_orc.clear_montado_em();
  proto_orc.add_entidades_montadas(1);
  auto* montaria = NovaEntidadeParaTestes(proto_orc, g_tabelas).release();
  ntf::Notificacao grupo_desfazer;
  TabuleiroTeste tabuleiro({montador, montaria});
  AcaoProto acao;
  g_dados_teste.push(10);
  acao.add_por_entidade();
  EXPECT_EQ(
      -10, DesviaMontariaSeAplicavel(
        g_tabelas, -10, /*total_ataque=*/12, *montaria,
        DadosAtaquePorGrupo("ataque_total_machado", montador->Proto()), &tabuleiro, acao.mutable_por_entidade(0), &grupo_desfazer))
    << acao.por_entidade(0).DebugString();
  g_dados_teste.push(11);
  acao.add_por_entidade();
  EXPECT_EQ(
      0, DesviaMontariaSeAplicavel(
        g_tabelas, -10, /*total_ataque=*/12, *montaria,
        DadosAtaquePorGrupo("ataque_total_machado", montador->Proto()), &tabuleiro, acao.mutable_por_entidade(1), &grupo_desfazer))
    << acao.por_entidade(1).DebugString();
}

TEST(TesteTalentoPericias, TesteTalentoMenteSobreMateriaCA) {
  auto proto_orc = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  auto* talento = proto_orc.mutable_info_talentos()->add_outros();
  talento->set_id("mente_sobre_materia");
  auto orc_sem = NovaEntidadeParaTestes(proto_orc, g_tabelas);
  EXPECT_EQ(orc_sem->CA(*orc_sem, Entidade::CA_NORMAL), 19) << orc_sem->Proto().dados_defesa().ca().DebugString();

  auto* ic = proto_orc.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(1);
  auto orc_com = NovaEntidadeParaTestes(proto_orc, g_tabelas);
  EXPECT_EQ(orc_com->CA(*orc_com, Entidade::CA_NORMAL), 20) << orc_com->Proto().dados_defesa().ca().DebugString();
}

TEST(TesteTalentoPericias, TesteAtaquePoderoso) {
  const auto& modelo_orc = g_tabelas.ModeloEntidade("Orc Capitão");
  auto orc = NovaEntidadeParaTestes(modelo_orc.entidade(), g_tabelas);
  EXPECT_FALSE(AtacandoPoderosamente(orc->Proto()));
  auto n = PreencheNotificacaoAtacandoPoderosamente(true, *orc);
  orc->AtualizaParcial(n.entidade());
  EXPECT_TRUE(AtacandoPoderosamente(orc->Proto()));
  const auto& da = DadosAtaquePorGrupo("ataque_total_machado", orc->Proto());
  EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "ataque_poderoso", da.bonus_ataque()), -3);
  EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "ataque_poderoso", da.bonus_dano()), 6);
  orc->AtualizaParcial(n.entidade_antes());
  EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "ataque_poderoso", da.bonus_ataque()), 0);
  EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "ataque_poderoso", da.bonus_dano()), 0);
}

TEST(TesteTalentoPericias, TesteAtaquePoderosoComComplemento) {
  auto proto = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  auto* t = TalentoOuCria("ataque_poderoso", &proto);
  t->set_complemento("5");
  auto orc = NovaEntidadeParaTestes(proto, g_tabelas);
  EXPECT_FALSE(AtacandoPoderosamente(orc->Proto()));
  auto n = PreencheNotificacaoAtacandoPoderosamente(true, *orc);
  orc->AtualizaParcial(n.entidade());
  EXPECT_TRUE(AtacandoPoderosamente(orc->Proto()));
  const auto& da = DadosAtaquePorGrupo("ataque_total_machado", orc->Proto());
  EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "ataque_poderoso", da.bonus_ataque()), -5);
  EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "ataque_poderoso", da.bonus_dano()), 10);
  const auto& darco = DadosAtaquePorGrupo("ataque_total_arco", orc->Proto());
  EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "ataque_poderoso", darco.bonus_ataque()), 0);
  EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "ataque_poderoso", darco.bonus_dano()), 0);

  orc->AtualizaParcial(n.entidade_antes());
  EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "ataque_poderoso", da.bonus_ataque()), 0);
  EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "ataque_poderoso", da.bonus_dano()), 0);
}

TEST(TesteTalentoPericias, TesteAtaquePoderosoComComplementoAcimaBba) {
  auto proto = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  auto* t = TalentoOuCria("ataque_poderoso", &proto);
  t->set_complemento("8");
  auto orc = NovaEntidadeParaTestes(proto, g_tabelas);
  EXPECT_FALSE(AtacandoPoderosamente(orc->Proto()));
  auto n = PreencheNotificacaoAtacandoPoderosamente(true, *orc);
  orc->AtualizaParcial(n.entidade());
  EXPECT_TRUE(AtacandoPoderosamente(orc->Proto()));
  const auto& da = DadosAtaquePorGrupo("ataque_total_machado", orc->Proto());
  EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "ataque_poderoso", da.bonus_ataque()), -7);
  EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "ataque_poderoso", da.bonus_dano()), 14);
}

TEST(TesteTalentoPericias, TesteAtaquePoderosoComComplementoInvalido) {
  auto proto = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  auto* t = TalentoOuCria("ataque_poderoso", &proto);
  t->set_complemento("fooooo");
  auto orc = NovaEntidadeParaTestes(proto, g_tabelas);
  EXPECT_FALSE(AtacandoPoderosamente(orc->Proto()));
  auto n = PreencheNotificacaoAtacandoPoderosamente(true, *orc);
  orc->AtualizaParcial(n.entidade());
  EXPECT_TRUE(AtacandoPoderosamente(orc->Proto()));
  const auto& da = DadosAtaquePorGrupo("ataque_total_machado", orc->Proto());
  EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "ataque_poderoso", da.bonus_ataque()), -1);
  EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "ataque_poderoso", da.bonus_dano()), 2);
}

TEST(TesteTalentoPericias, TesteAtaquePoderosoUmaMao) {
  auto proto = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  {
    auto* da = DadosAtaquePorGrupoOuCria("ataque_total_machado", &proto);
    da->set_empunhadura(EA_ARMA_APENAS);
    da->set_id_arma("espada_longa");
  }
  auto orc = NovaEntidadeParaTestes(proto, g_tabelas);
  EXPECT_FALSE(AtacandoPoderosamente(orc->Proto()));
  auto n = PreencheNotificacaoAtacandoPoderosamente(true, *orc);
  orc->AtualizaParcial(n.entidade());
  EXPECT_TRUE(AtacandoPoderosamente(orc->Proto()));
  const auto& da = DadosAtaquePorGrupo("ataque_total_machado", orc->Proto());
  EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "ataque_poderoso", da.bonus_ataque()), -3);
  EXPECT_EQ(BonusIndividualPorOrigem(TB_SEM_NOME, "ataque_poderoso", da.bonus_dano()), 3) << EmpunhaduraArma_Name(da.empunhadura());
}

TEST(TesteTalentoPericias, TesteMobilidade) {
  DadosAtaque da_ataque;
  da_ataque.set_bonus_ataque_final(12);
  const auto& modelo_orc = g_tabelas.ModeloEntidade("Orc Capitão");
  auto orc = NovaEntidadeParaTestes(modelo_orc.entidade(), g_tabelas);
  auto modelo_druida = g_tabelas.ModeloEntidade("Halfling Druida 10");
  modelo_druida.mutable_entidade()->mutable_info_talentos()->add_outros()->set_id("mobilidade");
  auto druida = NovaEntidadeParaTestes(modelo_druida.entidade(), g_tabelas);
  // 10 acerta na pinta.
  g_dados_teste.push(10);
  ResultadoAtaqueVsDefesa resultado = AtaqueVsDefesa(0.0f, DadosAtaquePorGrupo("ataque_total_machado", orc->Proto()).acao(), *orc, *druida, Posicao::default_instance(), /*ataque_oportunidade=*/false);
  EXPECT_TRUE(resultado.Sucesso()) << resultado.texto;
  // 9 ja erra.
  g_dados_teste.push(9);
  resultado = AtaqueVsDefesa(0.0f, DadosAtaquePorGrupo("ataque_total_machado", orc->Proto()).acao(), *orc, *druida, Posicao::default_instance(), /*ataque_oportunidade=*/false);
  EXPECT_FALSE(resultado.Sucesso()) << resultado.texto;
  // Com mobilidade, 10 erra (+4 na CA).
  g_dados_teste.push(10);
  resultado = AtaqueVsDefesa(0.0f, DadosAtaquePorGrupo("ataque_total_machado", orc->Proto()).acao(), *orc, *druida, Posicao::default_instance(), /*ataque_oportunidade=*/true);
  EXPECT_FALSE(resultado.Sucesso()) << resultado.texto;
  // Caido, ganha +4 e perde mobilidade.
  {
    EntidadeProto atu;
    atu.set_caida(true);
    druida->AtualizaParcial(atu);
  }
  g_dados_teste.push(6);
  resultado = AtaqueVsDefesa(0.0f, DadosAtaquePorGrupo("ataque_total_machado", orc->Proto()).acao(), *orc, *druida, Posicao::default_instance(), /*ataque_oportunidade=*/true);
  EXPECT_TRUE(resultado.Sucesso()) << resultado.texto;
}

TEST(TesteTalentoPericias, TestePotencializarInvocacao) {
  EntidadeProto proto;
  proto.set_gerar_agarrar(false);
  auto* ic = proto.add_info_classes();
  ic->set_id("druida");
  ic->set_nivel(6);
  AtribuiBaseAtributo(14, TA_SABEDORIA, &proto);
  RecomputaDependencias(g_tabelas, &proto);
  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Clérigo");
  da->set_id_arma("invocar_aliado_natureza_i");
  RecomputaDependencias(g_tabelas, &proto);
  AcaoProto acao = da->acao();
  ASSERT_EQ(acao.tipo(), ACAO_CRIACAO_ENTIDADE);
  ASSERT_EQ(acao.parametros_lancamento().parametros().size(), 5);
  ASSERT_EQ(acao.parametros_lancamento().parametros(0).id_modelo_entidade(), "Rato Atroz IANI");
  const auto& modelo_rato = g_tabelas.ModeloEntidade(acao.parametros_lancamento().parametros(0).id_modelo_entidade());
  EntidadeProto proto_rato = modelo_rato.entidade();
  ASSERT_TRUE(modelo_rato.has_parametros());

  {
    std::unique_ptr<Entidade> referencia(NovaEntidadeParaTestes(proto, g_tabelas));
    PreencheModeloComParametros(g_tabelas.Feitico(da->id_arma()), modelo_rato.parametros(), *referencia, &proto_rato);
    EXPECT_EQ(proto_rato.dados_vida(), "1d8+1");
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_FORCA, proto_rato)), 10);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_CONSTITUICAO, proto_rato)), 12);
  }

  {
    proto.mutable_info_talentos()->add_gerais()->set_id("potencializar_invocacao");
    std::unique_ptr<Entidade> referencia(NovaEntidadeParaTestes(proto, g_tabelas));
    PreencheModeloComParametros(g_tabelas.Feitico(da->id_arma()), modelo_rato.parametros(), *referencia, &proto_rato);
    EXPECT_EQ(proto_rato.dados_vida(), "1d8+1+2");
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_FORCA, proto_rato)), 14);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_CONSTITUICAO, proto_rato)), 16);
  }
}

TEST(TesteTalentoPericias, TesteTabeladoTalentosPericias) {
  for (const auto& modelo : g_tabelas.TodosModelosEntidades().modelo()) {
    EntidadeProto proto = modelo.entidade();
    const int antes = proto.info_talentos().gerais().size();
    RecomputaDependencias(g_tabelas, &proto);
    const int depois = proto.info_talentos().gerais().size();
    EXPECT_LE(antes, depois) << "falhou para: " << modelo.id();
    for (const auto& talentos : { proto.info_talentos().gerais(), proto.info_talentos().automaticos(), proto.info_talentos().outros()}) {
      for (const auto& talento : talentos) {
        EXPECT_FALSE(!talento.id().empty() && g_tabelas.Talento(talento.id()).id().empty())
            << "talento invalido: " << talento.id() << " para modelo " << modelo.id();
      }
    }
    int total_gasto = 0;
    for (const auto& pericia : proto.info_pericias()) {
      EXPECT_FALSE(g_tabelas.Pericia(pericia.id()).id().empty())
          << "perícia inválida: " << pericia.id() << " para modelo " << modelo.id();
      total_gasto += pericia.pontos();
    }
    int total_permitido = TotalPontosPericiaPermitidos(g_tabelas, proto);
    EXPECT_LE(total_gasto, total_permitido)
        << " modelo " << modelo.id() << " estourou limite de pericias. gasto " << total_gasto << ", permitido: " << total_permitido;
    if (total_gasto < total_permitido) {
      //LOG(INFO) << "modelo " << modelo.id() << " esta usando menos pericia que pode. Gasto: " << total_gasto << ", permitido: " << total_permitido;
    }
  }
}

TEST(TesteTalentoPericias, TramaDasSombras) {
  EntidadeProto proto;
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(7);
    proto.mutable_info_talentos()->add_outros()->set_id("magia_trama_sombras");
  }
  auto mago = NovaEntidadeParaTestes(proto, g_tabelas);
  EXPECT_EQ(NivelParaCalculoMagiasPorDia(g_tabelas, "mago", mago->Proto()), 7);
  EXPECT_EQ(mago->NivelConjurador("mago"), 7);
  EXPECT_EQ(mago->Proto().classe_feitico_ativa(), "mago");
  EXPECT_EQ(mago->NivelConjuradorParaMagia("mago", g_tabelas.Feitico("bola_fogo")), 6);
  EXPECT_EQ(mago->NivelConjuradorParaMagia("mago", g_tabelas.Feitico("enfeiticar_pessoa")), 8);
  EXPECT_EQ(mago->NivelConjuradorParaMagia("mago", g_tabelas.Feitico("ler_magia")), 7);
}

TEST(TesteTalentoPericias, AumentaNivelDeDruida) {
  EntidadeProto proto;
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("druida");
    ic->set_nivel(7);
  }
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("hathran");
    ic->set_nivel(2);
  }
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(NivelParaCalculoMagiasPorDia(g_tabelas, "druida", proto), 9);
  EXPECT_EQ(NivelConjurador("druida", proto), 9);
  EXPECT_EQ(NivelMaximoFeitico(g_tabelas, "druida", 9), 5);
  EXPECT_EQ(proto.classe_feitico_ativa(), "druida");
}

TEST(TesteTalentoPericias, AumentaNivelDeRanger) {
  EntidadeProto proto;
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("ranger");
    ic->set_nivel(7);
  }
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("agente_harpista");
    ic->set_nivel(2);
  }
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(NivelParaCalculoMagiasPorDia(g_tabelas, "ranger", proto), 9);
  EXPECT_EQ(NivelConjurador("ranger", proto), 4);
  EXPECT_EQ(NivelMaximoFeitico(g_tabelas, "ranger", 9), 2);
}

TEST(TesteTalentoPericias, AumentaNivelConjuradorDrenarForcaVital) {
  EntidadeProto proto;
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("clerigo");
    ic->set_nivel(7);
  }
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("adepto_sombras");
    ic->set_nivel(1);
  }
  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_DRENAR_FORCA_VITAL);
    evento->set_rodadas(100);
  }
  AtribuiBaseAtributo(12, TA_FORCA, &proto);

  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(NivelParaCalculoMagiasPorDia(g_tabelas, "clerigo", proto), 8);
  EXPECT_EQ(NivelConjurador("clerigo", proto), 9);
  EXPECT_EQ(ModificadorAtributo(proto.atributos().forca()), ModificadorAtributo(14));
}

TEST(TesteTalentoPericias, TesteHabilidadesEspeciais) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("monge");

  ic->set_nivel(1);
  proto.mutable_dados_defesa()->set_evasao_estatica(TE_EVASAO_APRIMORADA);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_FALSE(PossuiHabilidadeEspecial("evasao", proto));
  EXPECT_FALSE(PossuiHabilidadeEspecial("evasao_aprimorada", proto));
  EXPECT_TRUE(TipoEvasaoPersonagem(proto) == TE_EVASAO_APRIMORADA);

  ic->set_nivel(2);
  proto.mutable_dados_defesa()->clear_evasao_estatica();
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(PossuiHabilidadeEspecial("evasao", proto));
  EXPECT_FALSE(PossuiHabilidadeEspecial("evasao_aprimorada", proto));
  EXPECT_TRUE(TipoEvasaoPersonagem(proto) == TE_EVASAO);

  ic->set_nivel(9);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(PossuiHabilidadeEspecial("evasao", proto));
  EXPECT_TRUE(PossuiHabilidadeEspecial("evasao_aprimorada", proto));
  EXPECT_TRUE(TipoEvasaoPersonagem(proto) == TE_EVASAO_APRIMORADA);
}

TEST(TesteVazamento, TesteVazamento) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("monge");
  ic->set_nivel(2);
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_CURA_ACELERADA);
  evento->add_complementos(5);
  RecomputaDependencias(g_tabelas, &proto);
  const int tamanho = proto.ByteSize();
  for (int i = 0; i < 2; ++i) {
    RecomputaDependencias(g_tabelas, &proto);
    EXPECT_EQ(tamanho, proto.ByteSize()) << ", iteração: " << i << ", proto: " << proto.ShortDebugString();
    //break;
  }
}

TEST(TesteVazamento, TesteVazamento2) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("ladino");
  ic->set_nivel(6);
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_FLECHA_ACIDA);
  evento->set_rodadas(5);
  RecomputaDependencias(g_tabelas, &proto);
  const int tamanho = proto.ByteSize();
  for (int i = 0; i < 5; ++i) {
    RecomputaDependencias(g_tabelas, &proto);
    EXPECT_EQ(tamanho, proto.ByteSize()) << ", iteração: " << i;
    //break;
  }
}

TEST(TesteVazamento, TesteVazamento3) {
  EntidadeProto proto;
  proto.set_raca("meio_elfo");
  auto* ic = proto.add_info_classes();
  ic->set_id("barbaro");
  ic->set_nivel(6);
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_QUEIMANDO_FOGO_ALQUIMICO);
  evento->set_rodadas(1);
  RecomputaDependencias(g_tabelas, &proto);
  const int tamanho = proto.ByteSize();
  for (int i = 0; i < 5; ++i) {
    RecomputaDependencias(g_tabelas, &proto);
    EXPECT_EQ(tamanho, proto.ByteSize()) << ", iteração: " << i;
    //break;
  }
}

TEST(TesteVezes, TesteRemoveLimiteVezesExpirados) {
  EntidadeProto proto;
  proto.set_gerar_agarrar(false);
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("espada_longa");
    da->set_limite_vezes(-1);
  }
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("espada_curta");
  }
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("cimitarra");
    da->set_limite_vezes(0);
  }

  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(proto.dados_ataque().size(), 1);
  EXPECT_EQ(proto.dados_ataque(0).id_arma(), "espada_curta");
}

TEST(TesteVezes, TesteRefrescamento) {
  EntidadeProto proto;
  {
    proto.set_gerar_agarrar(false);
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("espada_longa");
    da->set_grupo("espada_longa");
    da->set_limite_vezes(0);
    da->set_mantem_com_limite_zerado(true);
    da->set_taxa_refrescamento("1");
    da->set_limite_vezes_original(3);
    da->set_usado_rodada(true);
  }
  std::unique_ptr<Entidade> e(NovaEntidadeParaTestes(proto, g_tabelas));
  ASSERT_EQ(e->Proto().dados_ataque_size(), 1);
  auto grupo = NovoGrupoNotificacoes();
  auto grupo_desfazer = NovoGrupoNotificacoes();
  PreencheNotificacaoAtaqueAoPassarRodada(*e, grupo.get(), grupo_desfazer.get());
  {
    e->AtualizaParcial(NotificacaoFilhaOuPadrao(*grupo).entidade());
    EXPECT_EQ(DadosAtaquePorGrupo("espada_longa", e->Proto()).limite_vezes(), 3) << e->Proto().dados_ataque(0).DebugString();
  }
  {
    e->AtualizaParcial(NotificacaoFilhaOuPadrao(*grupo_desfazer).entidade_antes());
    EXPECT_EQ(DadosAtaquePorGrupo("espada_longa", e->Proto()).limite_vezes(), 0) << e->Proto().dados_ataque(0).DebugString();
  }
}

TEST(TesteVezes, TesteRefrescamento2) {
  EntidadeProto proto;
  {
    proto.set_gerar_agarrar(false);
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("espada_longa");
    da->set_grupo("espada_longa");
    da->set_limite_vezes(0);
    da->set_mantem_com_limite_zerado(true);
    da->set_taxa_refrescamento("2");
    da->set_limite_vezes_original(3);
    da->set_usado_rodada(true);
  }
  std::unique_ptr<Entidade> e(NovaEntidadeParaTestes(proto, g_tabelas));
  ASSERT_EQ(e->Proto().dados_ataque_size(), 1);

  auto grupo = NovoGrupoNotificacoes();
  auto grupo_desfazer = NovoGrupoNotificacoes();
  PreencheNotificacaoAtaqueAoPassarRodada(*e, grupo.get(), grupo_desfazer.get());

  {
    e->AtualizaParcial(NotificacaoFilhaOuPadrao(*grupo).entidade());
    EXPECT_EQ(DadosAtaquePorGrupo("espada_longa", e->Proto()).limite_vezes(), 0) << e->Proto().dados_ataque(0).DebugString();
    EXPECT_EQ(DadosAtaquePorGrupo("espada_longa", e->Proto()).disponivel_em(), 1) << e->Proto().dados_ataque(0).DebugString();
  }
  {
    e->AtualizaParcial(NotificacaoFilhaOuPadrao(*grupo_desfazer).entidade_antes());
    EXPECT_EQ(DadosAtaquePorGrupo("espada_longa", e->Proto()).limite_vezes(), 0) << e->Proto().dados_ataque(0).DebugString();
    EXPECT_FALSE(DadosAtaquePorGrupo("espada_longa", e->Proto()).has_disponivel_em()) << e->Proto().dados_ataque(0).DebugString();
  }
}

TEST(TesteTalentoPericias, TesteTalentoPericias) {
  EntidadeProto proto;
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("ladino");
    ic->set_nivel(1);
    AtribuiBaseAtributo(12, TA_FORCA, &proto);
    AtribuiBaseAtributo(14, TA_DESTREZA, &proto);
    proto.mutable_info_talentos()->add_gerais()->set_id("maos_leves");
    PericiaCriando("usar_cordas", &proto)->set_pontos(5);
    RecomputaDependencias(g_tabelas, &proto);
  }

  // 2 des, 2 talento, 5 pontos de classe.
  EXPECT_EQ(9, BonusTotal(Pericia("usar_cordas", proto).bonus())) << "bonus: " << Pericia("usar_cordas", proto).bonus().DebugString();
  // 1 forca, 2 sinergia.
  EXPECT_EQ(3, BonusTotal(Pericia("escalar", proto).bonus())) << "bonus: " << Pericia("escalar", proto).bonus().DebugString();

  // Pericia deixa de ser de classe.
  ASSERT_FALSE(proto.info_classes().empty());
  proto.mutable_info_classes(0)->set_id("guerreiro");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(6, BonusTotal(Pericia("usar_cordas", proto).bonus()));
  // Sem sinergia.
  EXPECT_EQ(1, BonusTotal(Pericia("escalar", proto).bonus()));
}

TEST(TesteFormaAlternativa, TesteFormaAlternativa) {
  EntidadeProto proto;
  proto.set_gerar_agarrar(false);
  AtribuiBaseAtributo(14, TA_FORCA, &proto);
  AtribuiBonus(2, TB_NIVEL, "nivel", BonusAtributo(TA_FORCA, &proto));
  AtribuiBaseAtributo(10, TA_DESTREZA, &proto);
  AtribuiBaseAtributo(18, TA_CONSTITUICAO, &proto);
  AtribuiBaseAtributo(20, TA_INTELIGENCIA, &proto);
  AtribuiBaseAtributo(22, TA_SABEDORIA, &proto);
  AtribuiBaseAtributo(24, TA_CARISMA, &proto);
  AtribuiBonus(TM_PEQUENO, TB_BASE, "base", proto.mutable_bonus_tamanho());
  proto.mutable_dados_defesa()->set_id_armadura("couro");
  proto.mutable_dados_defesa()->set_bonus_magico_armadura(2);
  proto.mutable_dados_defesa()->set_id_escudo("leve_madeira");
  proto.mutable_dados_defesa()->set_bonus_magico_escudo(1);
  DadosAtaquePorGrupoOuCria("adaga_de_soco", &proto)->set_empunhadura(EA_ARMA_ESCUDO);

  EntidadeProto forma;
  AtribuiBaseAtributo(4, TA_FORCA, &forma);
  AtribuiBaseAtributo(6, TA_DESTREZA, &forma);
  AtribuiBaseAtributo(8, TA_CONSTITUICAO, &forma);
  // Ignorados
  AtribuiBaseAtributo(30, TA_INTELIGENCIA, &forma);
  AtribuiBaseAtributo(30, TA_SABEDORIA, &forma);
  AtribuiBaseAtributo(30, TA_CARISMA, &forma);
  AtribuiBonus(TM_GRANDE, TB_BASE, "base", forma.mutable_bonus_tamanho());
  forma.set_tipo_visao(VISAO_BAIXA_LUMINOSIDADE);
  forma.mutable_info_talentos()->add_gerais()->set_id("vigor");

  EntidadeProto forma_filtrada = ProtoFormaAlternativa(forma);

  std::unique_ptr<Entidade> e(NovaEntidadeParaTestes(proto, g_tabelas));
  EXPECT_EQ(TM_PEQUENO, e->Proto().tamanho());
  {
    e->AtualizaParcial(forma_filtrada);
    const auto& proto_pos_forma = e->Proto();
    EXPECT_EQ(4, BonusTotal(BonusAtributo(TA_FORCA, proto_pos_forma)));
    EXPECT_EQ(6, BonusTotal(BonusAtributo(TA_DESTREZA, proto_pos_forma)));
    EXPECT_EQ(8, BonusTotal(BonusAtributo(TA_CONSTITUICAO, proto_pos_forma)));
    EXPECT_EQ(20, BonusTotal(BonusAtributo(TA_INTELIGENCIA, proto_pos_forma)));
    EXPECT_EQ(22, BonusTotal(BonusAtributo(TA_SABEDORIA, proto_pos_forma)));
    EXPECT_EQ(24, BonusTotal(BonusAtributo(TA_CARISMA, proto_pos_forma)));
    EXPECT_EQ(TM_GRANDE, proto_pos_forma.tamanho());
    // -1 tam, -2 des.
    EXPECT_EQ(7, BonusTotal(proto_pos_forma.dados_defesa().ca()));
    // Nao ganha qualidades.
    forma.set_tipo_visao(VISAO_NORMAL);
    EXPECT_TRUE(proto_pos_forma.info_talentos().gerais().empty());
  }
  {
    e->AtualizaParcial(proto);
    const auto& proto_pos_forma = e->Proto();
    EXPECT_EQ(16, BonusTotal(BonusAtributo(TA_FORCA, proto_pos_forma)));
    EXPECT_EQ(10, BonusTotal(BonusAtributo(TA_DESTREZA, proto_pos_forma)));
    EXPECT_EQ(18, BonusTotal(BonusAtributo(TA_CONSTITUICAO, proto_pos_forma)));
    EXPECT_EQ(20, BonusTotal(BonusAtributo(TA_INTELIGENCIA, proto_pos_forma)));
    EXPECT_EQ(22, BonusTotal(BonusAtributo(TA_SABEDORIA, proto_pos_forma)));
    EXPECT_EQ(24, BonusTotal(BonusAtributo(TA_CARISMA, proto_pos_forma)));
    EXPECT_EQ(TM_PEQUENO, proto_pos_forma.tamanho());
    // +1 tam, 0 des, +2 escudo, +4 armadura.
    EXPECT_EQ(17, BonusTotal(proto_pos_forma.dados_defesa().ca())) << proto_pos_forma.dados_defesa().ca().DebugString();
  }
}

TEST(TesteFormaAlternativa, TesteFormaAlternativa2) {
  const auto& modelo = g_tabelas.ModeloEntidade("Humano Druida 5");
  EntidadeProto proto = modelo.entidade();
  auto* anel = proto.mutable_tesouro()->add_aneis();
  anel->set_em_uso(true);
  anel->set_id("protecao_1");
  std::unique_ptr<Entidade> entidade(NovaEntidadeParaTestes(proto, g_tabelas));
  const int max_pv = entidade->MaximoPontosVida();
  {
    // 10 de dano.
    ntf::Notificacao n;
    PreencheNotificacaoAtualizacaoPontosVida(*entidade, -10, TD_LETAL, &n, nullptr);
    entidade->AtualizaParcial(n.entidade());
    EXPECT_EQ(entidade->PontosVida(), max_pv - 10);
  }
  {
    const EntidadeProto& proto = entidade->Proto();
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_FORCA, proto)), 10);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_DESTREZA, proto)), 14);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_CONSTITUICAO, proto)), 13);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_INTELIGENCIA, proto)), 12);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_SABEDORIA, proto)), 16);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_CARISMA, proto)), 8);
    ASSERT_EQ(proto.dados_ataque().size(), 5);
    EXPECT_EQ(proto.dados_ataque(0).id_arma(), "cimitarra");
    EXPECT_EQ(entidade->PontosVida(), max_pv - 10);
    // 2 destreza, 1 deflexao, 3 armadura, 2 escudo.
    EXPECT_EQ(entidade->CA(*entidade, Entidade::CA_NORMAL), 18) << entidade->Proto().dados_defesa().ca().DebugString();
  }

  {
   // Vira urso.
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
    PreencheNotificacaoFormaAlternativa(g_tabelas, entidade->Proto(), &n, nullptr);
    ASSERT_EQ(n.notificacao_size(), 2);
    EntidadeProto teste;
    *teste.mutable_tesouro()->mutable_itens_mundanos() = n.notificacao(0).entidade().tesouro().itens_mundanos();
    *teste.mutable_tesouro()->mutable_itens_mundanos() = entidade->Proto().tesouro().itens_mundanos();
    entidade->AtualizaParcial(n.notificacao(0).entidade());
    *teste.mutable_tesouro()->mutable_itens_mundanos() = entidade->Proto().tesouro().itens_mundanos();
    entidade->AtualizaParcial(n.notificacao(1).entidade());
    const EntidadeProto& proto = entidade->Proto();
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_FORCA, proto)), 20);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_DESTREZA, proto)), 13);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_CONSTITUICAO, proto)), 15);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_INTELIGENCIA, proto)), 12);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_SABEDORIA, proto)), 16);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_CARISMA, proto)), 8);
    // +2 natural, +1 destreza.
    EXPECT_EQ(entidade->CA(*entidade, Entidade::CA_NORMAL), 13) << entidade->Proto().dados_defesa().ca().DebugString();
    ASSERT_EQ(proto.dados_ataque().size(), 4);
    EXPECT_EQ(proto.dados_ataque(0).rotulo(), "garra");
    EXPECT_EQ(entidade->PontosVida(), max_pv - 5);
  }
  {
    // Volta humano.
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
    PreencheNotificacaoFormaAlternativa(g_tabelas, entidade->Proto(), &n, nullptr);
    ASSERT_EQ(n.notificacao_size(), 1);
    entidade->AtualizaParcial(n.notificacao(0).entidade());
    const EntidadeProto& proto = entidade->Proto();
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_FORCA, proto)), 10);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_DESTREZA, proto)), 14);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_CONSTITUICAO, proto)), 13);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_INTELIGENCIA, proto)), 12);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_SABEDORIA, proto)), 16);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_CARISMA, proto)), 8);
    ASSERT_EQ(proto.dados_ataque().size(), 5);
    EXPECT_EQ(proto.dados_ataque(0).id_arma(), "cimitarra");
    // Nao cura.
    EXPECT_EQ(entidade->PontosVida(), max_pv - 5);
    // 2 destreza, 1 deflexao, 3 armadura, 2 escudo.
    EXPECT_EQ(entidade->CA(*entidade, Entidade::CA_NORMAL), 18) << entidade->Proto().dados_defesa().ca().DebugString();
  }
}

TEST(TesteFormaAlternativa, TesteFormaAlternativa3) {
  const auto& modelo = g_tabelas.ModeloEntidade("Elfo Druida 5");
  EntidadeProto proto = modelo.entidade();
  std::unique_ptr<Entidade> entidade(NovaEntidadeParaTestes(proto, g_tabelas));
  {
    const EntidadeProto& proto = entidade->Proto();
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_DESTREZA, proto)), 16);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_CONSTITUICAO, proto)), 11);
  }
  {
   // Vira urso.
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
    PreencheNotificacaoFormaAlternativa(g_tabelas, entidade->Proto(), &n, nullptr);
    ASSERT_EQ(n.notificacao_size(), 2);
    entidade->AtualizaParcial(n.notificacao(0).entidade());
    const EntidadeProto& proto = entidade->Proto();
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_DESTREZA, proto)), 13);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_CONSTITUICAO, proto)), 15);
  }
  {
    // Volta humano.
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
    PreencheNotificacaoFormaAlternativa(g_tabelas, entidade->Proto(), &n, nullptr);
    ASSERT_EQ(n.notificacao_size(), 1);
    entidade->AtualizaParcial(n.notificacao(0).entidade());
    const EntidadeProto& proto = entidade->Proto();
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_DESTREZA, proto)), 16);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_CONSTITUICAO, proto)), 11);
  }
}

TEST(TesteDependencias, TestePericias) {
  EntidadeProto proto;
  AtribuiBaseAtributo(12, TA_INTELIGENCIA, &proto);
  auto* ic = proto.add_info_classes();
  ic->set_id("ladino");
  ic->set_nivel(1);
  EXPECT_TRUE(PericiaDeClasse(g_tabelas, "observar", proto));
  EXPECT_FALSE(PericiaDeClasse(g_tabelas, "adestrar_animais", proto));
  EXPECT_EQ(TotalPontosPericiaPermitidos(g_tabelas, proto), 36);
}

TEST(TesteDependencias, TesteNiveisNegativos) {
  EntidadeProto proto;
  proto.set_max_pontos_vida(10);
  proto.set_pontos_vida(10);
  auto* ic = proto.add_info_classes();
  ic->set_id("clerigo");
  ic->set_nivel(3);
  auto* talento = proto.mutable_info_talentos()->add_gerais();
  talento->set_id("usar_armas_comuns");
  talento->set_complemento("espada_longa");
  // Ataques.
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("espada_longa");
    da->set_empunhadura(EA_2_MAOS);
  }
  AtribuiBonus(1, TB_BASE, "base", proto.mutable_niveis_negativos_dinamicos());

  RecomputaDependencias(g_tabelas, &proto);
  // Clerigo tem 2 de bonus.
  EXPECT_EQ(2, ic->bba());
  // Mas o ataque recebe -1 do nivel negativo.
  EXPECT_EQ(1, proto.dados_ataque(0).bonus_ataque_final());
  // 3 1 3 vira 2 0 2;
  EXPECT_EQ(2, BonusTotal(proto.dados_defesa().salvacao_fortitude()));
  EXPECT_EQ(0, BonusTotal(proto.dados_defesa().salvacao_reflexo()));
  EXPECT_EQ(2, BonusTotal(proto.dados_defesa().salvacao_vontade()));
  // Perde 5 pontos vida.
  EXPECT_EQ(5, proto.pontos_vida());
  // TODO: penalidade de pericias e feiticos.
}

TEST(TesteDependencias, TesteNiveisNegativosDrenarTemporario) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("clerigo");
  ic->set_nivel(3);
  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_DRENAR_TEMPORARIO);
    evento->set_rodadas(1);
    evento->add_complementos(1);
    evento->set_id_unico(1);
  }
  AtribuiBonus(1, TB_BASE, "base", proto.mutable_niveis_negativos_dinamicos());

  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(proto.niveis_negativos(), 2);

  {
    // Multiplos se acumulam.
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_DRENAR_TEMPORARIO);
    evento->set_rodadas(1);
    evento->add_complementos(2);
    evento->set_id_unico(2);
  }

  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(proto.niveis_negativos(), 4);
}

TEST(TesteDependencias, TesteReducaoDanoModeloAbissal) {
  const auto& modelo = g_tabelas.ModeloEntidade("Centopéia Enorme Abissal");
  EntidadeProto proto = modelo.entidade();
  std::unique_ptr<Entidade> entidade(NovaEntidadeParaTestes(proto, g_tabelas));
  proto = entidade->Proto();

  EntidadeProto proto_ataque;
  auto* da = proto_ataque.add_dados_ataque();
  da->set_id_arma("adaga");
  da->set_obra_prima(true);
  RecomputaDependencias(g_tabelas, &proto_ataque);

  ResultadoReducaoDano resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, da->descritores());
  EXPECT_EQ(resultado.delta_pv, -5) << resultado.texto;

  da->set_bonus_magico(1);
  RecomputaDependencias(g_tabelas, &proto_ataque);
  resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, da->descritores());
  EXPECT_EQ(resultado.delta_pv, -10) << resultado.texto;
}

TEST(TesteDependencias, TesteReducaoDanoBarbaro) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("barbaro");
  ic->set_nivel(3);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(0, proto.dados_defesa().reducao_dano_barbaro());
  ic->set_nivel(7);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(1, proto.dados_defesa().reducao_dano_barbaro());
  ic->set_nivel(10);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(2, proto.dados_defesa().reducao_dano_barbaro());
  ic->set_nivel(13);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(3, proto.dados_defesa().reducao_dano_barbaro());
  ic->set_nivel(16);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(4, proto.dados_defesa().reducao_dano_barbaro());
  ic->set_nivel(19);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(5, proto.dados_defesa().reducao_dano_barbaro());
}

TEST(TesteDependencias, TesteInvestida) {
  EntidadeProto proto;
  proto.mutable_info_talentos()->add_gerais()->set_id("usar_armas_comuns");
  // Ataques.
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("espada_longa");
  }
  // Eventos.
  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_INVESTIDA);
    evento->add_complementos(2);
  }

  RecomputaDependencias(g_tabelas, &proto);
  // 0 + 2.
  EXPECT_EQ(2, proto.dados_ataque(0).bonus_ataque_final());
  // 10 - 2.
  EXPECT_EQ(8, BonusTotal(proto.dados_defesa().ca()));
}

TEST(TesteDependencias, TesteInspirarCoragem) {
  EntidadeProto proto;
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("barbaro");
    ic->set_nivel(3);
  }
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("feiticeiro");
    ic->set_nivel(1);
  }

  // Ataques.
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("espada_longa");
  }
  {
    auto* da = proto.add_dados_ataque();
    da->set_tipo_ataque("Feitiço de Feiticeiro");
    da->set_id_arma("maos_flamejantes");
  }

  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(proto.dados_ataque(0).bonus_ataque_final(), 3);
  EXPECT_EQ(proto.dados_ataque(0).dano(), "1d8");
  EXPECT_EQ(proto.dados_ataque(1).bonus_ataque_final(), 3);
  EXPECT_EQ(proto.dados_ataque(1).dano(), "1d4");

  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_INSPIRAR_CORAGEM);
    evento->add_complementos(1);
  }
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(proto.dados_ataque(0).bonus_ataque_final(), 4)
      << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(0).dano(), "1d8+1");
  EXPECT_EQ(proto.dados_ataque(1).bonus_ataque_final(), 3);
  EXPECT_EQ(proto.dados_ataque(1).dano(), "1d4");
}

TEST(TesteDependencias, TesteDependencias) {
  EntidadeProto proto;
  proto.set_tamanho(TM_GRANDE);
  auto* ic = proto.add_info_classes();
  ic->set_id("barbaro");
  ic->set_nivel(3);
  // Ataques.
  {
    auto* da = proto.add_dados_ataque();
    da->set_grupo("espada_longa_2_maos");
    da->set_id_arma("espada_longa");
    da->set_obra_prima(true);
    da->set_empunhadura(EA_2_MAOS);
  }
  {
    auto* da = proto.add_dados_ataque();
    da->set_grupo("espada_longa_escudo");
    da->set_id_arma("espada_longa");
    da->set_obra_prima(true);
    da->set_empunhadura(EA_ARMA_ESCUDO);
  }

  // Eventos.
  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_FURIA);
    evento->add_complementos(6);
    evento->add_complementos(3);
  }
  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_AGILIDADE_GATO);
  }
  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_PROTECAO_CONTRA_BEM);
  }

  auto* dd = proto.mutable_dados_defesa();
  dd->set_id_armadura("cota_malha");
  dd->set_bonus_magico_armadura(2);
  dd->set_id_escudo("pesado_madeira");
  dd->set_bonus_magico_escudo(3);
  proto.mutable_tendencia()->set_simples(TD_LEAL_BOM);
  proto.set_ultimo_grupo_acao("espada_longa_escudo");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(3, ic->bba());
  // 16 da +3 de bonus.
  EXPECT_EQ(3, ModificadorAtributo(proto.atributos().forca()));
  EXPECT_EQ(3, ModificadorAtributo(proto.atributos().constituicao()));
  // 3 + 3 con;
  EXPECT_EQ(6, BonusTotal(proto.dados_defesa().salvacao_fortitude()));
  // 1 + 2 destreza.
  EXPECT_EQ(3, BonusTotal(proto.dados_defesa().salvacao_reflexo()));
  // 1 de vontade, +3 bonus.
  EXPECT_EQ(4, BonusTotal(proto.dados_defesa().salvacao_vontade()));
  // CA: 10 + 2 des + (5+2) cota, (2+3) escudo, -2 furia, -1 tamanho.
  EXPECT_EQ(21, BonusTotal(dd->ca()));

  // Primeiro ataque.
  {
    const auto& da = DadosAtaquePorGrupo("espada_longa_2_maos", proto);
    EXPECT_EQ("espada longa", da.rotulo());
    // Espada longa grande com duas maos.
    EXPECT_EQ("2d6+4", da.dano());
    EXPECT_EQ(19, da.margem_critico());
    // 3 base, 3 forca, -1 tamanho, 1 obra prima.
    EXPECT_EQ(6, da.bonus_ataque_final());
    // CA: 10 + 2 des + (5+2) cota, -2 furia, -1 tamanho.
    ASSERT_EQ(16, da.ca_normal());
    EXPECT_EQ(16, da.ca_surpreso());  // esquiva sobrenatural
    EXPECT_EQ(9, da.ca_toque());
  }
  // Segundo ataque.
  // Espada longa grande com uma mao.
  {
    const auto& da = DadosAtaquePorGrupo("espada_longa_escudo", proto);
    EXPECT_EQ("2d6+3", da.dano());
    EXPECT_EQ(21, da.ca_normal());
    EXPECT_EQ(21, da.ca_surpreso());  // esquiva sobrenatural.
    EXPECT_EQ(9, da.ca_toque());
  }
  EXPECT_GE(proto.tendencia().eixo_bem_mal(), 0.6f);

  proto.set_ultimo_grupo_acao("espada_longa_2_maos");
  auto ea = NovaEntidadeParaTestes(proto, g_tabelas);
  auto ed = NovaEntidadeParaTestes(proto, g_tabelas);
  // 16 normal +2 contra o bem.
  EXPECT_EQ(18, ed->CA(*ea, Entidade::CA_NORMAL));
  // 6 normal + 2 contra o bem.
  EXPECT_EQ(6, ed->Salvacao(*ea, TS_VONTADE));
}

TEST(TesteDependencias, TesteDependenciasTalentosSalvacoes) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("barbaro");
  ic->set_nivel(3);

  {
    auto* bi = proto.mutable_atributos()->mutable_constituicao()->add_bonus_individual();
    bi->set_tipo(TB_BASE);
    auto* po = bi->add_por_origem();
    po->set_origem("base");
    po->set_valor(18);  // +4.
  }
  {
    auto* bi = proto.mutable_atributos()->mutable_destreza()->add_bonus_individual();
    bi->set_tipo(TB_BASE);
    auto* po = bi->add_por_origem();
    po->set_origem("base");
    po->set_valor(16);  // +3.
  }
  {
    auto* bi = proto.mutable_atributos()->mutable_sabedoria()->add_bonus_individual();
    bi->set_tipo(TB_BASE);
    auto* po = bi->add_por_origem();
    po->set_origem("base");
    po->set_valor(14);  // +2.
  }

  // Fortitude maior.
  TalentoOuCria("fortitude_maior", &proto);
  RecomputaDependencias(g_tabelas, &proto);
  // 3 + 4 con + 2 fortitude maior;
  EXPECT_EQ(9, BonusTotal(proto.dados_defesa().salvacao_fortitude()));
  // 1 + 3 destreza.
  EXPECT_EQ(4, BonusTotal(proto.dados_defesa().salvacao_reflexo()));
  // 1 de vontade, +2 bonus.
  EXPECT_EQ(3, BonusTotal(proto.dados_defesa().salvacao_vontade()));

  // Adiciona reflexos rapidos.
  TalentoOuCria("reflexos_rapidos", &proto);
  RecomputaDependencias(g_tabelas, &proto);
  // 3 + 3 con + 2 fortitude maior;
  EXPECT_EQ(9, BonusTotal(proto.dados_defesa().salvacao_fortitude()));
  // 1 + 3 destreza + 2 reflexos rapidos.
  EXPECT_EQ(6, BonusTotal(proto.dados_defesa().salvacao_reflexo()));
  // 1 de vontade, +2 bonus.
  EXPECT_EQ(3, BonusTotal(proto.dados_defesa().salvacao_vontade()));

  // Adiciona vontade de ferro.
  TalentoOuCria("vontade_ferro", &proto);
  RecomputaDependencias(g_tabelas, &proto);
  // 3 + 3 con + 2 fortitude maior;
  EXPECT_EQ(9, BonusTotal(proto.dados_defesa().salvacao_fortitude()));
  // 1 + 3 destreza + 2 reflexos rapidos.
  EXPECT_EQ(6, BonusTotal(proto.dados_defesa().salvacao_reflexo()));
  // 1 de vontade, +2 bonus + 2 vontade ferro.
  EXPECT_EQ(5, BonusTotal(proto.dados_defesa().salvacao_vontade()));
  // Adiciona de novo (sem efeito).
  proto.mutable_info_talentos()->add_gerais()->set_id("vontade_ferro");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(9, BonusTotal(proto.dados_defesa().salvacao_fortitude()));

  // Remove reflexos rapidos.
  TalentoOuCria("reflexos_rapidos", &proto)->set_id("");
  RecomputaDependencias(g_tabelas, &proto);
  // 1 + 3 destreza.
  EXPECT_EQ(4, BonusTotal(proto.dados_defesa().salvacao_reflexo()));
}

TEST(TesteDependencias, TesteAgarrar) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("guerreiro");
  ic->set_nivel(3);

  {
    auto* bi = proto.mutable_atributos()->mutable_forca()->add_bonus_individual();
    bi->set_tipo(TB_BASE);
    auto* po = bi->add_por_origem();
    po->set_origem("base");
    po->set_valor(14);  // +2.
  }

  proto.set_tamanho(TM_GRANDE);


  RecomputaDependencias(g_tabelas, &proto);
  // 3 + 2 forca + 4 tamanho.
  EXPECT_EQ(9, proto.bba().agarrar());

  proto.mutable_info_talentos()->add_gerais()->set_id("agarrar_aprimorado");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(13, proto.bba().agarrar());
}

TEST(TesteDependencias, TesteAgarrar2) {
  DadosAtaque da_ataque;
  da_ataque.set_bonus_ataque_final(12);
  const auto& modelo_trex = g_tabelas.ModeloEntidade("Tiranossauro");
  auto trex = NovaEntidadeParaTestes(modelo_trex.entidade(), g_tabelas);
  const auto& modelo_druida = g_tabelas.ModeloEntidade("Halfling Druida 10");
  auto druida = NovaEntidadeParaTestes(modelo_druida.entidade(), g_tabelas);
  g_dados_teste.push(10);
  g_dados_teste.push(10);
  ResultadoAtaqueVsDefesa resultado = AtaqueVsDefesaAgarrar(*trex, *druida);
  EXPECT_TRUE(resultado.Sucesso());
  // Trex: 13 + 8 tamanho + 9 força.
  // Halfing: 7 - 4 tamanho - 1 força.
  EXPECT_EQ(resultado.texto.find("agarrar sucesso: 10+30 >= 10+2"), (unsigned int)0) << resultado.texto;
}

TEST(TesteDependencias, TesteVirtude) {
  EntidadeProto proto;
  std::vector<int> ids_unicos;
  auto* ev = AdicionaEvento(DadosIniciativa{.iniciativa=3, .modificador=4}, /*origem*/"virtude", EFEITO_VIRTUDE, /*rodadas=*/10, /*continuo=*/false, &ids_unicos, &proto);
  RecomputaDependencias(g_tabelas, &proto);
  // Neste ponto, espera-se uma entrada em pontos de vida temporario SEM_NOME, "ajuda".
  auto* po = OrigemSePresente(TB_SEM_NOME, "virtude", proto.mutable_pontos_vida_temporarios_por_fonte());
  ASSERT_NE(po, nullptr) << proto.pontos_vida_temporarios_por_fonte().DebugString();
  EXPECT_EQ(proto.pontos_vida_temporarios(), 1);

  // Nova chamada, mantem o mesmo valor. Verifica duplicatas.
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(1, proto.pontos_vida_temporarios_por_fonte().bonus_individual().size());
  ASSERT_EQ(1, proto.pontos_vida_temporarios_por_fonte().bonus_individual(0).por_origem().size());
  po = OrigemSePresente(TB_SEM_NOME, "virtude", proto.mutable_pontos_vida_temporarios_por_fonte());
  ASSERT_NE(po, nullptr);
  EXPECT_EQ(proto.pontos_vida_temporarios(), 1);

  // Termina o efeito.
  ev->set_rodadas(-1);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(proto.pontos_vida_temporarios(), 0) << proto.pontos_vida_temporarios_por_fonte().DebugString();
}

TEST(TesteDependencias, TesteVitalidadeIlusoria) {
  EntidadeProto proto;
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(3);
    auto* da = proto.add_dados_ataque();
    da->set_tipo_ataque("Feitiço de Mago");
    da->set_id_arma("vitalidade_ilusoria");
  }
  CentralColetora central;
  std::unique_ptr<Entidade> e(NovaEntidadeParaTestes(proto, g_tabelas));
  std::vector<int> ids_unicos = IdsUnicosProto(e->Proto());
  auto* da = e->DadoCorrente();
  if (da == nullptr) {
    da = &DadosAtaque::default_instance();
  }
  AcaoProto acao = e->Acao();
  ASSERT_NE(da, nullptr);
  ntf::Notificacao grupo_desfazer;
  g_dados_teste.push(5);  // 5 no d10.
  AplicaEfeitosAdicionais(g_tabelas, 0.0f, /*salvou=*/false, *e, *e, TAL_DESCONHECIDO, *da, acao.add_por_entidade(), &acao, &ids_unicos, &ids_unicos, &grupo_desfazer, &central);
  auto& ns = central.Notificacoes();
  ASSERT_EQ(ns.size(), 1U) << ", acao: " << acao.DebugString();
  e->AtualizaParcial(ns[0]->entidade());

  proto = e->Proto();
  // Neste ponto, espera-se uma entrada em pontos de vida temporario SEM_NOME, "vitalidade ilusoria".
  auto* po = OrigemSePresente(TB_SEM_NOME, "vitalidade_ilusoria", proto.mutable_pontos_vida_temporarios_por_fonte());
  ASSERT_NE(po, nullptr) << "pvt: " << proto.pontos_vida_temporarios_por_fonte().DebugString();
  EXPECT_EQ(proto.pontos_vida_temporarios(), 8);
  const int valor = po->valor();

  // Nova chamada, mantem o mesmo valor. Verifica duplicatas.
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(1, proto.pontos_vida_temporarios_por_fonte().bonus_individual().size());
  ASSERT_EQ(1, proto.pontos_vida_temporarios_por_fonte().bonus_individual(0).por_origem().size());
  po = OrigemSePresente(TB_SEM_NOME, "vitalidade_ilusoria", proto.mutable_pontos_vida_temporarios_por_fonte());
  ASSERT_NE(po, nullptr) << "pvt: " << proto.pontos_vida_temporarios_por_fonte().DebugString();
  EXPECT_EQ(proto.pontos_vida_temporarios(), valor);

  // Termina o efeito.
  {
    ntf::Notificacao n;
    EntidadeProto *proto_antes, *proto_depois;
    std::tie(proto_antes, proto_depois) = PreencheNotificacaoEntidade(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, *e, &n);
    *proto_depois->mutable_evento() = e->Proto().evento();;
    ASSERT_EQ(proto_depois->evento().size(), 1);
    proto_depois->mutable_evento(0)->set_rodadas(-1);
    e->AtualizaParcial(*proto_depois);
    proto = e->Proto();
  }
  EXPECT_EQ(e->PontosVidaTemporarios(), 0) << proto.pontos_vida_temporarios_por_fonte().DebugString();
}

TEST(TesteDependencias, TesteAjuda) {
  EntidadeProto proto;
  std::vector<int> ids_unicos;
  auto* ev = AdicionaEvento(std::nullopt, /*origem=*/"ajuda", EFEITO_AJUDA, 10, false, &ids_unicos, &proto);
  ev->add_complementos(5);
  RecomputaDependencias(g_tabelas, &proto);
  // Neste ponto, espera-se uma entrada em pontos de vida temporario SEM_NOME, "ajuda".
  auto* po = OrigemSePresente(TB_SEM_NOME, "ajuda", proto.mutable_pontos_vida_temporarios_por_fonte());
  ASSERT_NE(po, nullptr);
  EXPECT_EQ(proto.pontos_vida_temporarios(), 5);
  const int valor = po->valor();

  // Nova chamada, mantem o mesmo valor. Verifica duplicatas.
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(1, proto.pontos_vida_temporarios_por_fonte().bonus_individual().size());
  ASSERT_EQ(1, proto.pontos_vida_temporarios_por_fonte().bonus_individual(0).por_origem().size());
  po = OrigemSePresente(TB_SEM_NOME, "ajuda", proto.mutable_pontos_vida_temporarios_por_fonte());
  ASSERT_NE(po, nullptr);
  EXPECT_EQ(proto.pontos_vida_temporarios(), valor);

  // Termina o efeito.
  ev->set_rodadas(-1);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(proto.pontos_vida_temporarios(), 0);
}

TEST(TesteDependencias, TesteAjuda2) {
  EntidadeProto proto;
  std::vector<int> ids_unicos;
  auto* ev = AdicionaEvento(std::nullopt, /*origem=*/"ajuda", EFEITO_AJUDA, 10, false, &ids_unicos, &proto);
  ev->add_complementos(5);
  ev = AdicionaEvento(std::nullopt, /*origem*/"ajuda", EFEITO_AJUDA, 10, false, &ids_unicos, &proto);
  ev->add_complementos(6);
  int id_segundo_evento = ev->id_unico();
  RecomputaDependencias(g_tabelas, &proto);
  // Neste ponto, espera-se uma entrada em pontos de vida temporario SEM_NOME, "ajuda".
  ASSERT_EQ(proto.pontos_vida_temporarios_por_fonte().bonus_individual().size(), 1) << proto.pontos_vida_temporarios_por_fonte().DebugString();
  ASSERT_EQ(proto.pontos_vida_temporarios_por_fonte().bonus_individual(0).por_origem().size(), 1);
  EXPECT_EQ(proto.pontos_vida_temporarios_por_fonte().bonus_individual(0).por_origem(0).origem(), "ajuda");

  // Forcar o temporario a vir do segundo evento (porque sao aleatorios os valores).
  auto* po = OrigemSePresente(TB_SEM_NOME, "ajuda", proto.mutable_pontos_vida_temporarios_por_fonte());
  ASSERT_NE(po, nullptr);
  po->set_id_unico(id_segundo_evento);
  ev->set_rodadas(-1);

  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(proto.pontos_vida_temporarios(), 0);
}

TEST(TesteDependencias, TesteAjuda3) {
  g_dados_teste.push(4);
  EntidadeProto proto;
  std::vector<int> ids_unicos;
  auto* ev = AdicionaEvento(std::nullopt, /*origem*/"ajuda", EFEITO_AJUDA, 10, false, &ids_unicos, &proto);
  ev->add_complementos(7);
  RecomputaDependencias(g_tabelas, &proto);
  // Neste ponto, espera-se uma entrada em pontos de vida temporario SEM_NOME, "ajuda".
  auto* po = OrigemSePresente(TB_SEM_NOME, "ajuda", proto.mutable_pontos_vida_temporarios_por_fonte());
  ASSERT_NE(po, nullptr);
  EXPECT_EQ(proto.pontos_vida_temporarios(), 7);
  {
    std::unique_ptr<Entidade> entidade(NovaEntidadeParaTestes(proto, g_tabelas));
    EXPECT_EQ(entidade->PontosVida(), 7);
    // Aplica 1 de dano:
    ntf::Notificacao n;
    PreencheNotificacaoAtualizacaoPontosVida(*entidade, /*delta_pontos_vida=*/-1, TD_LETAL, &n, nullptr);
    entidade->AtualizaParcial(n.entidade());
    EXPECT_EQ(entidade->PontosVida(), 6) << entidade->Proto().pontos_vida_temporarios_por_fonte().DebugString();
  }
}

// Teste basico gera dados.
TEST(TesteGeraDados, TesteGeraDados) {
  int pv = GeraMaxPontosVida("\t4 d   6 - 5  ");
  EXPECT_EQ(19, pv);
  pv = GeraMaxPontosVida("  4 d   4 +\t1  ");
  EXPECT_EQ(17, pv);
  pv = GeraMaxPontosVida("4d3");
  EXPECT_EQ(12, pv);
  pv = GeraMaxPontosVida("4d3 + 5 - 2d8 -  3");
  EXPECT_EQ(-2, pv);
  pv = GeraMaxPontosVida("2d3 + 2 + 1d4 + 1");
  EXPECT_EQ(13, pv);
  pv = GeraMaxPontosVida("2d3 + 1d4 + 1");
  EXPECT_EQ(11, pv);
  pv = GeraMaxPontosVida("2d3 + 1d4 + 1 + 2");
  EXPECT_EQ(13, pv);
}

TEST(TesteStringSemUtf8, TesteStringSemUtf8) {
  std::string s("áÁéÉíÍóÓúÚç");
  EXPECT_EQ("aAeEiIoOuUc", StringSemUtf8(s));
}

TEST(TesteDados, Dados) {
  std::map<int, int> valores;
  for (int i = 0; i < 1000000; ++i) {
    valores[RolaDado(20)]++;
  }
  int min = std::numeric_limits<int>::max();
  int max = 0;
  for (auto it : valores) {
    LOG(INFO) << "valor: " << it.first << ": " << it.second;
    min = std::min(min, it.second);
    max = std::max(max, it.second);
  }
  float max_min = static_cast<float>(max) / min;
  LOG(INFO) << "max / min: " << max_min;
  EXPECT_LT(max_min, 1.04);
}

TEST(TesteDanoArma, TesteDanoArma) {
  DanoArma dano_arma;
  dano_arma = LeDanoArma("1d8");
  EXPECT_EQ("1d8", dano_arma.dano);
  EXPECT_EQ(20, dano_arma.margem_critico);
  EXPECT_EQ(2, dano_arma.multiplicador);

  dano_arma = LeDanoArma("1d8 (19-20)");
  EXPECT_EQ("1d8", dano_arma.dano);
  EXPECT_EQ(19, dano_arma.margem_critico);
  EXPECT_EQ(2, dano_arma.multiplicador);

  dano_arma = LeDanoArma("1d6 (x4)");
  EXPECT_EQ("1d6", dano_arma.dano);
  EXPECT_EQ(20, dano_arma.margem_critico);
  EXPECT_EQ(4, dano_arma.multiplicador);

  dano_arma = LeDanoArma("1d8+5(19-20/x3)");
  EXPECT_EQ("1d8+5", dano_arma.dano);
  EXPECT_EQ(19, dano_arma.margem_critico);
  EXPECT_EQ(3, dano_arma.multiplicador);

  dano_arma = LeDanoArma("1d8+5(19-20 /    x 4)");
  EXPECT_EQ("1d8+5", dano_arma.dano);
  EXPECT_EQ(19, dano_arma.margem_critico);
  EXPECT_EQ(4, dano_arma.multiplicador);

  dano_arma = LeDanoArma("1d8+5 (18 × 3)");
  EXPECT_EQ("1d8+5", dano_arma.dano);
  EXPECT_EQ(18, dano_arma.margem_critico);
  EXPECT_EQ(3, dano_arma.multiplicador);
}

TEST(TesteMatriz, TesteMatriz) {
  {
    Vector3 v(1.0f, 0.0f, 0.0f);
    Vector3 vt(1.0f, 0.0f, 0.0f);
    Matrix4 m = MatrizRotacao(v);
    EXPECT_EQ(v, m * vt);
  }

  {
    Vector3 v(0.0f, 1.0f, 0.0f);
    Vector3 vt(1.0f, 0.0f, 0.0f);
    Matrix4 m = MatrizRotacao(v);
    Vector3 vr = m * vt;
    EXPECT_NEAR(v.x, vr.x, 0.001);
    EXPECT_NEAR(v.y, vr.y, 0.001);
    EXPECT_NEAR(v.z, vr.z, 0.001);
  }

  {
    Vector3 v(0.0f, 0.0f, 1.0f);
    Vector3 vt(1.0f, 0.0f, 0.0f);
    Matrix4 m = MatrizRotacao(v);
    Vector3 vr = m * vt;
    EXPECT_NEAR(v.x, vr.x, 0.001);
    EXPECT_NEAR(v.y, vr.y, 0.001);
    EXPECT_NEAR(v.z, vr.z, 0.001);
  }

  {
    Vector4 v1(4.0f, 1.0f, 5.0f, 1.0f);
    Vector4 v2(1.0f, 1.0f, 1.0f, 1.0f);
    Vector4 v3 = v1 - v2;  // da 3.0, 0.0, 4.0 (tamanho 5).
    Vector4 vt(0.0f, 0.0f, 0.0f, 1.0f);
    Matrix4 m;
    m.rotateY(90.0f);
    m.rotateZ(180.0f);
    m.translate(1.0f, 0.0f, 0.0f);
    m.scale(5.0f, 0.0f, 0.0f);
    m = MatrizRotacao(Vector3(v3.x, v3.y, v3.z)) * m;
    m.translate(v2.x, v2.y, v2.z);
    Vector4 vr = m * vt;
    EXPECT_NEAR(v1.x, vr.x, 0.001);
    EXPECT_NEAR(v1.y, vr.y, 0.001);
    EXPECT_NEAR(v1.z, vr.z, 0.001);
  }
}

TEST(TesteFurtivo, TesteFurtivo) {
  {
    const auto& modelo = g_tabelas.ModeloEntidade("Carniçal");
    auto ed = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
    EXPECT_TRUE(ed->ImuneFurtivo(*ed));
  }
  {
    EntidadeProto proto;
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_NUBLAR);  // da camuflagem.
    evento->set_rodadas(1);
    evento->set_id_unico(1);
    auto ed = NovaEntidadeParaTestes(proto, g_tabelas);
    auto* icl = proto.add_info_classes();
    icl->set_id("ladino");
    icl->set_nivel(4);
    auto ea = NovaEntidadeParaTestes(proto, g_tabelas);
    EXPECT_TRUE(ed->ImuneFurtivo(*ea));
  }
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("barbaro");
    ic->set_nivel(5);  // ganha esquiva sobrenatural aprimorada.
    auto ed = NovaEntidadeParaTestes(proto, g_tabelas);
    EXPECT_TRUE(ed->ImuneFurtivo(*ed));
    ic->set_id("ladino");
    ic->set_nivel(9);
    auto ea = NovaEntidadeParaTestes(proto, g_tabelas);
    EXPECT_FALSE(ed->ImuneFurtivo(*ea));
  }
  {
    EntidadeProto proto;
    auto* icb = proto.add_info_classes();
    icb->set_id("barbaro");
    icb->set_nivel(2);  // ganha esquiva sobrenatural.
    auto* icl = proto.add_info_classes();
    icl->set_id("ladino");
    icl->set_nivel(4);  // ganha esquiva sobrenatural, converte em aprimorada.
    auto ed = NovaEntidadeParaTestes(proto, g_tabelas);  // nivel total 6.
    EXPECT_TRUE(ed->ImuneFurtivo(*ed));
    {
      icl->set_nivel(9);  // nivel ladino 9.
      auto ea = NovaEntidadeParaTestes(proto, g_tabelas);
      EXPECT_TRUE(ed->ImuneFurtivo(*ea));
    }
    {
      icl->set_nivel(10);  // nivel ladino 10.
      auto ea = NovaEntidadeParaTestes(proto, g_tabelas);
      EXPECT_FALSE(ed->ImuneFurtivo(*ea));
    }
  }
}

// Wrapper por causa da mudanca de assinatura da funcao.
int ModificadorAtaqueWrapper(TipoAtaque tipo_ataque, const EntidadeProto& ea, const EntidadeProto& ed) {
  DadosAtaque da;
  switch (tipo_ataque) {
    case TipoAtaque::CORPO_A_CORPO:
      da.set_ataque_corpo_a_corpo(true);
      break;
    case TipoAtaque::DISTANCIA:
      da.set_ataque_distancia(true);
      break;
    case TipoAtaque::AGARRAR:
      da.set_ataque_agarrar(true);
      break;
    case TipoAtaque::DESARMAR:
      da.set_ataque_desarmar(true);
      break;
  }
  return ModificadorAtaque(da, ea, ed);
}

TEST(TesteModificadorAtaque, TesteModificadorAtaqueFlanqueando) {
  EntidadeProto ea;
  ea.mutable_dados_ataque_global()->set_flanqueando(true);
  {
    EntidadeProto ed;
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, ea, ed), 2);
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::DISTANCIA, ea, ed), 0);
  }
  {
    EntidadeProto ed;
    auto* ic = ed.add_info_classes();
    ic->set_id("barbaro");
    ic->set_nivel(5);  // ganha esquiva sobrenatural.
    RecomputaDependencias(g_tabelas, &ed);
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, ea, ed), 0);
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::DISTANCIA, ea, ed), 0);
  }
  {
    EntidadeProto ed;
    auto* ic = ed.add_info_classes();
    ic->set_id("ladino");
    ic->set_nivel(8);  // ganha esquiva sobrenatural.
    RecomputaDependencias(g_tabelas, &ed);
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, ea, ed), 0);
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::DISTANCIA, ea, ed), 0);
  }
  {
    EntidadeProto ed;
    {
      auto* ic = ed.add_info_classes();
      ic->set_id("barbaro");
      ic->set_nivel(2);  // ganha esquiva.
    }
    {
      auto* ic = ed.add_info_classes();
      ic->set_id("ladino");
      ic->set_nivel(4);  // ganha esquiva.
    }
    RecomputaDependencias(g_tabelas, &ed);
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, ea, ed), 0);
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::DISTANCIA, ea, ed), 0);
  }
  {
    EntidadeProto ed;
    {
      auto* ic = ed.add_info_classes();
      ic->set_id("barbaro");
      ic->set_nivel(1);  // sem esquiva.
    }
    {
      auto* ic = ed.add_info_classes();
      ic->set_id("ladino");
      ic->set_nivel(4);  // sem esquiva.
    }
    RecomputaDependencias(g_tabelas, &ed);
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, ea, ed), 2);
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::DISTANCIA, ea, ed), 0);
  }
  {
    EntidadeProto ed;
    {
      auto* ic = ed.add_info_classes();
      ic->set_id("barbaro");
      ic->set_nivel(2);  // sem esquiva.
    }
    {
      auto* ic = ed.add_info_classes();
      ic->set_id("ladino");
      ic->set_nivel(3);  // sem esquiva.
    }
    RecomputaDependencias(g_tabelas, &ed);
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, ea, ed), 2);
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::DISTANCIA, ea, ed), 0);
  }
}

TEST(TesteModificadorAtaque, TesteModificadorAtaqueElementalTerra) {
  const auto& modelo = g_tabelas.ModeloEntidade("Elemental da Terra (Pequeno)");
  auto el = modelo.entidade();
  EntidadeProto ed;
  EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, el, ed), 1);
  EXPECT_EQ(ModificadorDano(DadosAtaquePorGrupo("pancada", el), el, ed), 1);
  ed.set_voadora(true);
  EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, el, ed), -4);
  EXPECT_EQ(ModificadorDano(DadosAtaquePorGrupo("pancada", el), el, ed), -4);
  ed.set_voadora(false);
  ed.set_nadando(true);
  EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, el, ed), -4);
  EXPECT_EQ(ModificadorDano(DadosAtaquePorGrupo("pancada", el), el, ed), -4);
  ed.set_nadando(false);
  el.set_voadora(true);
  EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, el, ed), 0);
  EXPECT_EQ(ModificadorDano(DadosAtaquePorGrupo("pancada", el), el, ed), 0);
  el.set_voadora(false);
  el.set_nadando(true);
  EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, el, ed), 0);
  EXPECT_EQ(ModificadorDano(DadosAtaquePorGrupo("pancada", el), el, ed), 0);
  el.set_nadando(false);
  EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, el, ed), 1);
  EXPECT_EQ(ModificadorDano(DadosAtaquePorGrupo("pancada", el), el, ed), 1);
}

TEST(TesteModificadorAtaque, TesteModificadorAtaqueElementalAgua) {
  const auto& modelo = g_tabelas.ModeloEntidade("Elemental da Água (Pequeno)");
  ASSERT_EQ(modelo.id(), "Elemental da Água (Pequeno)");
  auto el = modelo.entidade();
  EntidadeProto ed;
  EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, el, ed), -4);
  EXPECT_EQ(ModificadorDano(DadosAtaquePorGrupo("pancada", el), el, ed), -4);
  ed.set_voadora(true);
  EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, el, ed), -4);
  EXPECT_EQ(ModificadorDano(DadosAtaquePorGrupo("pancada", el), el, ed), -4);
  ed.set_voadora(false);
  ed.set_nadando(true);
  EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, el, ed), -4);
  EXPECT_EQ(ModificadorDano(DadosAtaquePorGrupo("pancada", el), el, ed), -4);
  ed.set_nadando(false);
  el.set_voadora(true);
  EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, el, ed), -4);
  EXPECT_EQ(ModificadorDano(DadosAtaquePorGrupo("pancada", el), el, ed), -4);
  el.set_voadora(false);
  el.set_nadando(true);
  EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, el, ed), -4);
  EXPECT_EQ(ModificadorDano(DadosAtaquePorGrupo("pancada", el), el, ed), -4);
  ed.set_nadando(true);
  EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, el, ed), 1);
  EXPECT_EQ(ModificadorDano(DadosAtaquePorGrupo("pancada", el), el, ed), 1);
  el.set_nadando(false);
  el.set_voadora(true);
  EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, el, ed), 0);
  EXPECT_EQ(ModificadorDano(DadosAtaquePorGrupo("pancada", el), el, ed), 0);
}

TEST(TesteModificadorAtaque, TesteModificadorAtaque) {
  {
    EntidadeProto ea;
    EntidadeProto ed;
    ed.set_caida(true);

    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, ea, ed), 4);
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::DISTANCIA, ea, ed), -4);
  }
  {
    EntidadeProto ea;
    EntidadeProto ed;
    ed.set_em_corpo_a_corpo(true);

    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, ea, ed), 0);
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::DISTANCIA, ea, ed), -4);
  }
  {
    EntidadeProto ea;
    auto* tp = ea.mutable_info_talentos()->add_outros();
    tp->set_id("tiro_preciso");
    EntidadeProto ed;
    ed.set_em_corpo_a_corpo(true);

    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, ea, ed), 0);
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::DISTANCIA, ea, ed), 0);
  }
  {
    EntidadeProto ea;
    EntidadeProto ed;

    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::AGARRAR, ea, ed), 0);
    ea.mutable_info_talentos()->add_gerais()->set_id("agarrar_aprimorado");
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::AGARRAR, ea, ed), 4);
    ed.mutable_info_talentos()->add_gerais()->set_id("agarrar_aprimorado");
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::AGARRAR, ea, ed), 0);
  }
}

TEST(TesteModificadorAlcance, TesteModificadorAlcance) {
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(3);
    auto* da = proto.add_dados_ataque();
    da->set_tipo_ataque("Feitiço de Mago");
    da->set_id_arma("missil_magico");
    RecomputaDependencias(g_tabelas, &proto);

    EXPECT_EQ(da->alcance_m(), 26 * TAMANHO_LADO_QUADRADO);
  }
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("feiticeiro");
    ic->set_nivel(2);
    auto* da = proto.add_dados_ataque();
    da->set_tipo_ataque("Feitiço de Mago");
    da->set_id_arma("missil_magico");
    RecomputaDependencias(g_tabelas, &proto);

    EXPECT_EQ(da->alcance_m(), 24 * TAMANHO_LADO_QUADRADO);
  }
}

TEST(TesteRodadasDinamico, TesteRodadasDinamico) {
  ntf::Notificacao n;
  EntidadeProto proto;
  std::unique_ptr<Entidade> e(NovaEntidadeParaTestes(proto, g_tabelas));
  std::vector<int> ids_unicos = IdsUnicosEntidade(*e);
  PreencheNotificacaoEventoEfeitoAdicional(
      proto.id(), std::nullopt, /*nivel*/3, *e, g_tabelas.Feitico("sono").acao().efeitos_adicionais(0), &ids_unicos, &n, nullptr);
  ASSERT_FALSE(n.entidade().evento().empty());
  EXPECT_EQ(n.entidade().evento(0).rodadas(), 30);
  ASSERT_EQ(ids_unicos.size(), 1ULL);
  EXPECT_EQ(n.entidade().evento(0).id_unico(), ids_unicos[0]);
}

TEST(TesteRodadasDinamico, TesteRodadasDinamicoDependenciaAnterior) {
  ntf::Notificacao n;
  EntidadeProto proto;
  {
    auto* ic = proto.add_info_classes();
    ic->set_nivel(4);
    ic->set_id("mago");
  }
  std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto, g_tabelas));
  {
    auto* da = proto.add_dados_ataque();
    da->set_tipo_ataque("Feitiço de Mago");
    da->set_id_arma("leque_cromatico");
  }
  RecomputaDependencias(g_tabelas, &proto);

  std::vector<int> ids_unicos = IdsUnicosEntidade(*alvo);
  AcaoProto acao = proto.dados_ataque(0).acao();
  ResolveEfeitosAdicionaisVariaveis(4, proto, *alvo, &acao);
  EXPECT_GT(acao.efeitos_adicionais(1).rodadas(), 0);
  EXPECT_EQ(acao.efeitos_adicionais(2).rodadas(), acao.efeitos_adicionais(1).rodadas() + 1);
}

TEST(TesteRodadasDinamico, TesteRodadasDinamicoDependenciaAnterior2) {
  ntf::Notificacao n;
  EntidadeProto proto;
  proto.set_gerar_agarrar(false);
  {
    auto* ic = proto.add_info_classes();
    ic->set_nivel(4);
    ic->set_id("mago");
  }
  std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto, g_tabelas));

  proto.mutable_tesouro()->add_itens_mundanos()->set_id("bolsa_cola");
  RecomputaDependencias(g_tabelas, &proto);

  std::vector<int> ids_unicos = IdsUnicosEntidade(*alvo);
  AcaoProto acao = DadosAtaquePorGrupo("bolsa_cola", proto).acao();
  g_dados_teste.push(1);
  g_dados_teste.push(1);
  ResolveEfeitosAdicionaisVariaveis(/*nivel_conjurador=*/0, proto, *alvo, &acao);
  ASSERT_EQ(acao.efeitos_adicionais().size(), 2) << acao.DebugString();
  EXPECT_EQ(acao.efeitos_adicionais(0).rodadas(), 2);
  EXPECT_EQ(acao.efeitos_adicionais(1).rodadas(), 2);
}

// Este teste simula mais ou menos a forma como os efeitos adicionais de feiticos sao aplicados.
TEST(TesteEfeitosAdicionaisMultiplos, TesteEfeitosAdicionaisMultiplos) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(3);
  std::unique_ptr<Entidade> e(NovaEntidadeParaTestes(proto, g_tabelas));
  EXPECT_EQ(ValorFinalPericia("ouvir", e->Proto()), 0);
  ntf::Notificacao n;
  std::vector<int> ids_unicos = IdsUnicosEntidade(*e);
  const auto& feitico = g_tabelas.Feitico("aterrorizar");
  ASSERT_EQ(feitico.acao().efeitos_adicionais().size(), 2);
  PreencheNotificacaoEventoEfeitoAdicional(
      proto.id(), std::nullopt, /*nivel=*/3, *e,
      feitico.acao().efeitos_adicionais(0), &ids_unicos, n.add_notificacao(), nullptr);
  PreencheNotificacaoEventoEfeitoAdicional(
      proto.id(), std::nullopt, /*nivel=*/3, *e,
      feitico.acao().efeitos_adicionais(1), &ids_unicos,
      n.add_notificacao(), nullptr);
  e->AtualizaParcial(n.notificacao(0).entidade());
  e->AtualizaParcial(n.notificacao(1).entidade());
  ASSERT_EQ(e->Proto().evento().size(), 2);
  EXPECT_EQ(e->Proto().evento(0).id_efeito(), EFEITO_AMEDRONTADO);
  EXPECT_EQ(e->Proto().evento(1).id_efeito(), EFEITO_ABALADO);
  EXPECT_EQ(ValorFinalPericia("ouvir", e->Proto()), -2);
  ASSERT_EQ(ids_unicos.size(), 2ULL);
  EXPECT_EQ(ids_unicos[0], 0);
  EXPECT_EQ(ids_unicos[1], 1);
}

TEST(TesteEfeitosAdicionaisMultiplos, TesteToqueIdiotice) {
  {
    EntidadeProto proto;
    AtribuiBaseAtributo(12, TA_INTELIGENCIA, &proto);
    AtribuiBaseAtributo(10, TA_SABEDORIA, &proto);
    AtribuiBaseAtributo(8, TA_CARISMA, &proto);
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(3);
    std::unique_ptr<Entidade> e(NovaEntidadeParaTestes(proto, g_tabelas));
    ntf::Notificacao n;
    std::vector<int> ids_unicos = IdsUnicosEntidade(*e);
    const auto& feitico = g_tabelas.Feitico("toque_idiotice");
    ASSERT_EQ(feitico.acao().efeitos_adicionais().size(), 3);
    g_dados_teste.push(1);
    g_dados_teste.push(2);
    g_dados_teste.push(3);
    LOG(INFO) << "int";
    PreencheNotificacaoEventoEfeitoAdicional(
        proto.id(), std::nullopt, /*nivel=*/3, *e,
        feitico.acao().efeitos_adicionais(0), &ids_unicos, n.add_notificacao(), nullptr);
    LOG(INFO) << "sab";
    PreencheNotificacaoEventoEfeitoAdicional(
        proto.id(), std::nullopt, /*nivel=*/3, *e,
        feitico.acao().efeitos_adicionais(1), &ids_unicos,
        n.add_notificacao(), nullptr);
    LOG(INFO) << "car";
    PreencheNotificacaoEventoEfeitoAdicional(
        proto.id(), std::nullopt, /*nivel=*/3, *e,
        feitico.acao().efeitos_adicionais(2), &ids_unicos,
        n.add_notificacao(), nullptr);
    e->AtualizaParcial(n.notificacao(0).entidade());
    e->AtualizaParcial(n.notificacao(1).entidade());
    e->AtualizaParcial(n.notificacao(2).entidade());
    // Int.
    ASSERT_EQ(e->Proto().evento().size(), 3);
    EXPECT_EQ(e->Proto().evento(0).id_efeito(), EFEITO_PENALIDADE_INTELIGENCIA);
    ASSERT_FALSE(e->Proto().evento(0).complementos().empty());
    EXPECT_EQ(e->Proto().evento(0).complementos(0), -1);
    EXPECT_EQ(11, BonusTotal(BonusAtributo(TA_INTELIGENCIA, e->Proto())));
    // Sab.
    EXPECT_EQ(e->Proto().evento(1).id_efeito(), EFEITO_PENALIDADE_SABEDORIA);
    ASSERT_FALSE(e->Proto().evento(1).complementos().empty());
    EXPECT_EQ(e->Proto().evento(1).complementos(0), -2);
    EXPECT_EQ(8, BonusTotal(BonusAtributo(TA_SABEDORIA, e->Proto())));
    // Car.
    EXPECT_EQ(e->Proto().evento(2).id_efeito(), EFEITO_PENALIDADE_CARISMA);
    ASSERT_FALSE(e->Proto().evento(2).complementos().empty());
    EXPECT_EQ(e->Proto().evento(2).complementos(0), -3);
    EXPECT_EQ(5, BonusTotal(BonusAtributo(TA_CARISMA, e->Proto())));

    // Ids unicos.
    ASSERT_EQ(ids_unicos.size(), 3ULL);
    EXPECT_EQ(ids_unicos[0], 0);
    EXPECT_EQ(ids_unicos[1], 1);
    EXPECT_EQ(ids_unicos[2], 2);

    // Aplica de novo pra ver se nao acumula.
    e->AtualizaParcial(n.notificacao(0).entidade());
    EXPECT_EQ(11, BonusTotal(BonusAtributo(TA_INTELIGENCIA, e->Proto())));
  }
}

TEST(TesteEfeitos, TesteArmaEnvenenada) {
  auto proto = g_tabelas.ModeloEntidade("Humana Ranger 9 Duas Armas").entidade();
  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_ARMA_ENVENENADA);
    evento->add_complementos_str("escorpiao_grande");
    evento->add_complementos_str("espada_de_duas_laminas_mao_boa");
    evento->set_rodadas(1);
    evento->set_id_unico(666);
  }
  auto ranger = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("2 Armas", ranger->Proto());
    EXPECT_EQ(da.veneno().id_unico_efeito(), 666);
    EXPECT_EQ(da.veneno().cd(), 18) << da.DebugString();
  }
  {
    const auto& da = DadosAtaquePorGrupo("2 Armas", ranger->Proto(), 1);
    EXPECT_FALSE(da.has_veneno());
  }
}

TEST(TesteEfeitos, TesteAbencoarArma) {
  auto proto = g_tabelas.ModeloEntidade("Humana Ranger 9 Duas Armas").entidade();
  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_ABENCOAR_ARMA);
    evento->set_rodadas(1);
    evento->set_id_unico(666);
  }
  auto ranger = NovaEntidadeParaTestes(proto, g_tabelas);
  auto* evento = AchaEvento(666, ranger->Proto());
  ASSERT_NE(evento, nullptr);
  ASSERT_FALSE(evento->complementos_str().empty());
  EXPECT_EQ(evento->complementos_str(0), "espada_de_duas_laminas_mao_boa") << evento->DebugString();
}

TEST(TesteEfeitos, TesteFuriaFadiga) {
  EntidadeProto proto;
  AtribuiBaseAtributo(10, TA_FORCA, &proto);
  AtribuiBaseAtributo(10, TA_DESTREZA, &proto);
  AtribuiBaseAtributo(10, TA_CONSTITUICAO, &proto);
  AtribuiBaseAtributo(10, TA_SABEDORIA, &proto);
  auto* ic = proto.add_info_classes();
  ic->set_id("barbaro");
  ic->set_nivel(3);
  auto e = NovaEntidadeParaTestes(proto, g_tabelas);

  {
    auto grupo = NovoGrupoNotificacoes();
    EXPECT_TRUE(PreencheNotificacaoAlternarFuria(g_tabelas, *e, grupo.get(), nullptr));
    ASSERT_EQ(grupo->notificacao().size(), 1);
    e->AtualizaParcial(grupo->notificacao(0).entidade());
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_FORCA, e->Proto())), 14);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_CONSTITUICAO, e->Proto())), 14);
    EXPECT_EQ(e->Salvacao(*e, TS_VONTADE), 3);
  }
  {
    auto grupo = NovoGrupoNotificacoes();
    EXPECT_FALSE(PreencheNotificacaoAlternarFuria(g_tabelas, *e, grupo.get(), nullptr));
    ASSERT_EQ(grupo->notificacao().size(), 2);
    e->AtualizaParcial(grupo->notificacao(0).entidade());
    e->AtualizaParcial(grupo->notificacao(1).entidade());
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_FORCA, e->Proto())), 8);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_DESTREZA, e->Proto())), 8);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_CONSTITUICAO, e->Proto())), 10);
    EXPECT_EQ(e->Salvacao(*e, TS_VONTADE), 1);
  }
}

TEST(TesteEfeitos, TesteFuriaSemFadiga17) {
  EntidadeProto proto;
  AtribuiBaseAtributo(10, TA_FORCA, &proto);
  AtribuiBaseAtributo(10, TA_DESTREZA, &proto);
  AtribuiBaseAtributo(10, TA_CONSTITUICAO, &proto);
  AtribuiBaseAtributo(10, TA_SABEDORIA, &proto);
  auto* ic = proto.add_info_classes();
  ic->set_id("barbaro");
  ic->set_nivel(17);
  auto e = NovaEntidadeParaTestes(proto, g_tabelas);

  {
    auto grupo = NovoGrupoNotificacoes();
    EXPECT_TRUE(PreencheNotificacaoAlternarFuria(g_tabelas, *e, grupo.get(), nullptr));
    ASSERT_EQ(grupo->notificacao().size(), 1);
    e->AtualizaParcial(grupo->notificacao(0).entidade());
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_FORCA, e->Proto())), 16);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_CONSTITUICAO, e->Proto())), 16);
    EXPECT_EQ(e->Salvacao(*e, TS_VONTADE), 8);
  }
  {
    auto grupo = NovoGrupoNotificacoes();
    EXPECT_FALSE(PreencheNotificacaoAlternarFuria(g_tabelas, *e, grupo.get(), nullptr));
    ASSERT_EQ(grupo->notificacao().size(), 1);
    e->AtualizaParcial(grupo->notificacao(0).entidade());
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_FORCA, e->Proto())), 10);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_DESTREZA, e->Proto())), 10);
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_CONSTITUICAO, e->Proto())), 10);
    EXPECT_EQ(e->Salvacao(*e, TS_VONTADE), 5);
  }
}

TEST(TesteEfeitos, TesteFuriaFazerDesfazer) {
  EntidadeProto proto;
  AtribuiBaseAtributo(10, TA_FORCA, &proto);
  AtribuiBaseAtributo(10, TA_CONSTITUICAO, &proto);
  AtribuiBaseAtributo(10, TA_SABEDORIA, &proto);
  auto* ic = proto.add_info_classes();
  ic->set_id("barbaro");
  ic->set_nivel(3);
  auto e = NovaEntidadeParaTestes(proto, g_tabelas);

  auto grupo = NovoGrupoNotificacoes();
  EXPECT_TRUE(PreencheNotificacaoAlternarFuria(g_tabelas, *e, grupo.get(), nullptr));
  ASSERT_EQ(grupo->notificacao().size(), 1);
  e->AtualizaParcial(grupo->notificacao(0).entidade());
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_FORCA, e->Proto())), 14);
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_CONSTITUICAO, e->Proto())), 14);
  EXPECT_EQ(e->Salvacao(*e, TS_VONTADE), 3);
  e->AtualizaParcial(grupo->notificacao(0).entidade_antes());
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_FORCA, e->Proto())), 10) << BonusAtributo(TA_FORCA, e->Proto()).DebugString();
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_CONSTITUICAO, e->Proto())), 10);
  EXPECT_EQ(e->Salvacao(*e, TS_VONTADE), 1);
}

TEST(TesteEfeitos, TesteTigreAtrozPresaMagicaMaiorGarra) {
  const auto& modelo = g_tabelas.ModeloEntidade("Tigre Atroz");
  EntidadeProto proto = modelo.entidade();
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_PRESA_MAGICA_MAIOR);
  evento->set_rodadas(HORAS_EM_RODADAS * 4);
  evento->set_id_unico(2);
  evento->add_complementos_str("garra");
  proto.set_gerar_agarrar(false);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(proto.dados_ataque().size(), 8);
  // Normal;
  EXPECT_EQ(proto.dados_ataque(0).dano(), "2d4+10") << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(0).bonus_ataque_final(), 22) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(1).dano(), "2d4+8") << proto.dados_ataque(1).DebugString();
  EXPECT_EQ(proto.dados_ataque(1).bonus_ataque_final(), 20) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(2).dano(), "2d6+4") << proto.dados_ataque(2).DebugString();
  EXPECT_EQ(proto.dados_ataque(2).bonus_ataque_final(), 14) << proto.dados_ataque(0).DebugString();
  // Bote.
  EXPECT_EQ(proto.dados_ataque(3).dano(), "2d4+10") << proto.dados_ataque(3).DebugString();
  EXPECT_EQ(proto.dados_ataque(3).bonus_ataque_final(), 22) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(4).dano(), "2d4+8") << proto.dados_ataque(4).DebugString();
  EXPECT_EQ(proto.dados_ataque(4).bonus_ataque_final(), 20) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(5).dano(), "2d4+4") << proto.dados_ataque(5).DebugString();
  EXPECT_EQ(proto.dados_ataque(5).bonus_ataque_final(), 14) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(6).dano(), "2d4+4") << proto.dados_ataque(6).DebugString();
  EXPECT_EQ(proto.dados_ataque(6).bonus_ataque_final(), 14) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(7).dano(), "2d6+4") << proto.dados_ataque(7).DebugString();
  EXPECT_EQ(proto.dados_ataque(7).bonus_ataque_final(), 14) << proto.dados_ataque(0).DebugString();
}

TEST(TesteEfeitos, TesteTigreAtrozPresaMagicaMaiorGeral) {
  const auto& modelo = g_tabelas.ModeloEntidade("Tigre Atroz");
  EntidadeProto proto = modelo.entidade();
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_PRESA_MAGICA_MAIOR);
  evento->set_rodadas(HORAS_EM_RODADAS * 4);
  evento->set_id_unico(2);
  proto.set_gerar_agarrar(false);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(proto.dados_ataque().size(), 8);
  // Normal;
  EXPECT_EQ(proto.dados_ataque(0).dano(), "2d4+9") << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(0).bonus_ataque_final(), 21) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(1).dano(), "2d4+9") << proto.dados_ataque(1).DebugString();
  EXPECT_EQ(proto.dados_ataque(1).bonus_ataque_final(), 21) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(2).dano(), "2d6+5") << proto.dados_ataque(2).DebugString();
  EXPECT_EQ(proto.dados_ataque(2).bonus_ataque_final(), 15) << proto.dados_ataque(0).DebugString();
  // Bote.
  EXPECT_EQ(proto.dados_ataque(3).dano(), "2d4+9") << proto.dados_ataque(3).DebugString();
  EXPECT_EQ(proto.dados_ataque(3).bonus_ataque_final(), 21) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(4).dano(), "2d4+9") << proto.dados_ataque(4).DebugString();
  EXPECT_EQ(proto.dados_ataque(4).bonus_ataque_final(), 21) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(5).dano(), "2d4+5") << proto.dados_ataque(5).DebugString();
  EXPECT_EQ(proto.dados_ataque(5).bonus_ataque_final(), 15) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(6).dano(), "2d4+5") << proto.dados_ataque(6).DebugString();
  EXPECT_EQ(proto.dados_ataque(6).bonus_ataque_final(), 15) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(7).dano(), "2d6+5") << proto.dados_ataque(7).DebugString();
  EXPECT_EQ(proto.dados_ataque(7).bonus_ataque_final(), 15) << proto.dados_ataque(0).DebugString();
}

TEST(TesteEfeitos, TesteTigreAtrozPresaMagica) {
  const auto& modelo = g_tabelas.ModeloEntidade("Tigre Atroz");
  EntidadeProto proto = modelo.entidade();
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_PRESA_MAGICA);
  evento->set_rodadas(1);
  evento->set_id_unico(2);
  proto.set_gerar_agarrar(false);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(proto.dados_ataque().size(), 8);
  // Normal;
  EXPECT_EQ(proto.dados_ataque(0).dano(), "2d4+9") << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(0).bonus_ataque_final(), 21) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(1).dano(), "2d4+8") << proto.dados_ataque(1).DebugString();
  EXPECT_EQ(proto.dados_ataque(1).bonus_ataque_final(), 20) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(2).dano(), "2d6+4") << proto.dados_ataque(2).DebugString();
  EXPECT_EQ(proto.dados_ataque(2).bonus_ataque_final(), 14) << proto.dados_ataque(0).DebugString();
  // Bote.
  EXPECT_EQ(proto.dados_ataque(3).dano(), "2d4+9") << proto.dados_ataque(3).DebugString();
  EXPECT_EQ(proto.dados_ataque(3).bonus_ataque_final(), 21) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(4).dano(), "2d4+8") << proto.dados_ataque(4).DebugString();
  EXPECT_EQ(proto.dados_ataque(4).bonus_ataque_final(), 20) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(5).dano(), "2d4+4") << proto.dados_ataque(5).DebugString();
  EXPECT_EQ(proto.dados_ataque(5).bonus_ataque_final(), 14) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(6).dano(), "2d4+4") << proto.dados_ataque(6).DebugString();
  EXPECT_EQ(proto.dados_ataque(6).bonus_ataque_final(), 14) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(7).dano(), "2d6+4") << proto.dados_ataque(7).DebugString();
  EXPECT_EQ(proto.dados_ataque(7).bonus_ataque_final(), 14) << proto.dados_ataque(0).DebugString();
}

TEST(TesteEfeitos, TesteTigreAtrozPresaMagicaMordida) {
  const auto& modelo = g_tabelas.ModeloEntidade("Tigre Atroz");
  EntidadeProto proto = modelo.entidade();
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_PRESA_MAGICA);
  evento->set_rodadas(1);
  evento->set_id_unico(2);
  evento->add_complementos_str("mordida");
  proto.set_gerar_agarrar(false);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(proto.dados_ataque().size(), 8);
  // Normal;
  EXPECT_EQ(proto.dados_ataque(0).dano(), "2d4+8") << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(0).bonus_ataque_final(), 20) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(1).dano(), "2d4+8") << proto.dados_ataque(1).DebugString();
  EXPECT_EQ(proto.dados_ataque(1).bonus_ataque_final(), 20) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(2).dano(), "2d6+5") << proto.dados_ataque(2).DebugString();
  EXPECT_EQ(proto.dados_ataque(2).bonus_ataque_final(), 15) << proto.dados_ataque(0).DebugString();
  // Bote.
  EXPECT_EQ(proto.dados_ataque(3).dano(), "2d4+8") << proto.dados_ataque(3).DebugString();
  EXPECT_EQ(proto.dados_ataque(3).bonus_ataque_final(), 20) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(4).dano(), "2d4+8") << proto.dados_ataque(4).DebugString();
  EXPECT_EQ(proto.dados_ataque(4).bonus_ataque_final(), 20) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(5).dano(), "2d4+4") << proto.dados_ataque(5).DebugString();
  EXPECT_EQ(proto.dados_ataque(5).bonus_ataque_final(), 14) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(6).dano(), "2d4+4") << proto.dados_ataque(6).DebugString();
  EXPECT_EQ(proto.dados_ataque(6).bonus_ataque_final(), 14) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(7).dano(), "2d6+5") << proto.dados_ataque(7).DebugString();
  EXPECT_EQ(proto.dados_ataque(7).bonus_ataque_final(), 15) << proto.dados_ataque(0).DebugString();
}

TEST(TesteEfeitos, TesteTigreAtrozPresaMagicaGarra) {
  const auto& modelo = g_tabelas.ModeloEntidade("Tigre Atroz");
  EntidadeProto proto = modelo.entidade();
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_PRESA_MAGICA);
  evento->set_rodadas(1);
  evento->set_id_unico(2);
  evento->add_complementos_str("garra");
  proto.set_gerar_agarrar(false);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(proto.dados_ataque().size(), 8);
  // Normal;
  EXPECT_EQ(proto.dados_ataque(0).dano(), "2d4+9") << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(0).bonus_ataque_final(), 21) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(1).dano(), "2d4+8") << proto.dados_ataque(1).DebugString();
  EXPECT_EQ(proto.dados_ataque(1).bonus_ataque_final(), 20) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(2).dano(), "2d6+4") << proto.dados_ataque(2).DebugString();
  EXPECT_EQ(proto.dados_ataque(2).bonus_ataque_final(), 14) << proto.dados_ataque(0).DebugString();
  // Bote.
  EXPECT_EQ(proto.dados_ataque(3).dano(), "2d4+9") << proto.dados_ataque(3).DebugString();
  EXPECT_EQ(proto.dados_ataque(3).bonus_ataque_final(), 21) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(4).dano(), "2d4+8") << proto.dados_ataque(4).DebugString();
  EXPECT_EQ(proto.dados_ataque(4).bonus_ataque_final(), 20) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(5).dano(), "2d4+4") << proto.dados_ataque(5).DebugString();
  EXPECT_EQ(proto.dados_ataque(5).bonus_ataque_final(), 14) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(6).dano(), "2d4+4") << proto.dados_ataque(6).DebugString();
  EXPECT_EQ(proto.dados_ataque(6).bonus_ataque_final(), 14) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(7).dano(), "2d6+4") << proto.dados_ataque(7).DebugString();
  EXPECT_EQ(proto.dados_ataque(7).bonus_ataque_final(), 14) << proto.dados_ataque(0).DebugString();
}

TEST(TesteEfeitos, TesteTigreAtrozPresaMagicaGarraTraseira) {
  const auto& modelo = g_tabelas.ModeloEntidade("Tigre Atroz");
  EntidadeProto proto = modelo.entidade();
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_PRESA_MAGICA);
  evento->set_rodadas(1);
  evento->set_id_unico(2);
  evento->add_complementos_str("garra traseira");
  proto.set_gerar_agarrar(false);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(proto.dados_ataque().size(), 8);
  // Normal;
  EXPECT_EQ(proto.dados_ataque(0).dano(), "2d4+8") << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(0).bonus_ataque_final(), 20) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(1).dano(), "2d4+8") << proto.dados_ataque(1).DebugString();
  EXPECT_EQ(proto.dados_ataque(1).bonus_ataque_final(), 20) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(2).dano(), "2d6+4") << proto.dados_ataque(2).DebugString();
  EXPECT_EQ(proto.dados_ataque(2).bonus_ataque_final(), 14) << proto.dados_ataque(0).DebugString();
  // Bote.
  EXPECT_EQ(proto.dados_ataque(3).dano(), "2d4+8") << proto.dados_ataque(3).DebugString();
  EXPECT_EQ(proto.dados_ataque(3).bonus_ataque_final(), 20) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(4).dano(), "2d4+8") << proto.dados_ataque(4).DebugString();
  EXPECT_EQ(proto.dados_ataque(4).bonus_ataque_final(), 20) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(5).dano(), "2d4+5") << proto.dados_ataque(5).DebugString();
  EXPECT_EQ(proto.dados_ataque(5).bonus_ataque_final(), 15) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(6).dano(), "2d4+4") << proto.dados_ataque(6).DebugString();
  EXPECT_EQ(proto.dados_ataque(6).bonus_ataque_final(), 14) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(7).dano(), "2d6+4") << proto.dados_ataque(7).DebugString();
  EXPECT_EQ(proto.dados_ataque(7).bonus_ataque_final(), 14) << proto.dados_ataque(0).DebugString();
}

TEST(TesteSalvacaoDinamica, TesteSalvacaoDinamicaSlotAcima) {
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(3);
    AtribuiBaseAtributo(12, TA_INTELIGENCIA, &proto);
    auto* da = proto.add_dados_ataque();
    da->set_tipo_ataque("Feitiço de Mago");
    da->set_id_arma("bola_fogo");
    da->set_nivel_slot(5);
    RecomputaDependencias(g_tabelas, &proto);

    EXPECT_EQ(da->dificuldade_salvacao(), 14);
  }
  {
    EntidadeProto proto;
    auto* talento = proto.mutable_info_talentos()->add_gerais();
    talento->set_id("elevar_magia");
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(3);
    AtribuiBaseAtributo(12, TA_INTELIGENCIA, &proto);
    auto* da = proto.add_dados_ataque();
    da->set_tipo_ataque("Feitiço de Mago");
    da->set_id_arma("bola_fogo");
    da->set_nivel_slot(5);
    RecomputaDependencias(g_tabelas, &proto);

    EXPECT_EQ(da->dificuldade_salvacao(), 16);
  }
}

TEST(TesteSalvacaoDinamica, TesteSalvacaoDinamica) {
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(3);
    AtribuiBaseAtributo(12, TA_INTELIGENCIA, &proto);
    auto* da = proto.add_dados_ataque();
    da->set_tipo_ataque("Feitiço de Mago");
    da->set_id_arma("bola_fogo");
    RecomputaDependencias(g_tabelas, &proto);

    EXPECT_EQ(da->dificuldade_salvacao(), 14);
  }
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("feiticeiro");
    ic->set_nivel(3);
    AtribuiBaseAtributo(14, TA_CARISMA, &proto);
    auto* da = proto.add_dados_ataque();
    da->set_tipo_ataque("Feitiço de Feiticeiro");
    da->set_id_arma("bola_fogo");
    RecomputaDependencias(g_tabelas, &proto);

    EXPECT_EQ(da->dificuldade_salvacao(), 15);
  }
}

TEST(TesteSalvacao, TesteModificadoresPorTipoDiferente) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("feiticeiro");
  ic->set_nivel(3);
  AtribuiBaseAtributo(13, TA_CARISMA, &proto);
  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Feiticeiro");
  da->set_id_arma("riso_histerico");
  da->set_eh_feitico(true);
  std::unique_ptr<Entidade> referencia(NovaEntidadeParaTestes(proto, g_tabelas));
  {
    g_dados_teste.push(9);
    auto [delta, passou, texto] = AtaqueVsSalvacao(0, referencia->Proto().dados_ataque(0), *referencia, *referencia);
    EXPECT_FALSE(passou) << "tirou " << delta << ", " << texto;
  }
  {
    g_dados_teste.push(10);
    auto [delta, passou, texto] = AtaqueVsSalvacao(0, referencia->Proto().dados_ataque(0), *referencia, *referencia);
    EXPECT_TRUE(passou) << "tirou " << delta << ", " << texto;
  }

  // Ganha +4 de bonus.
  proto.add_tipo_dnd(TIPO_BESTA_MAGICA);
  std::unique_ptr<Entidade> besta_magica(NovaEntidadeParaTestes(proto, g_tabelas));
  {
    g_dados_teste.push(5);
    auto [delta, passou, texto] = AtaqueVsSalvacao(0, referencia->Proto().dados_ataque(0), *referencia, *besta_magica);
    EXPECT_FALSE(passou) << "tirou " << delta << ", " << texto;
  }
  {
    g_dados_teste.push(6);
    auto [delta, passou, texto] = AtaqueVsSalvacao(0, referencia->Proto().dados_ataque(0), *referencia, *besta_magica);
    EXPECT_TRUE(passou) << "tirou " << delta << ", " << texto;
  }
}

TEST(TesteFeiticos, TesteCurarNaoAplicaForca) {
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("clerigo");
    ic->set_nivel(1);
    AtribuiBaseAtributo(12, TA_FORCA, &proto);
    AtribuiBaseAtributo(14, TA_SABEDORIA, &proto);
    auto* da = proto.add_dados_ataque();
    da->set_tipo_ataque("Feitiço de Clérigo");
    da->set_id_arma("curar_ferimentos_minimos");
    RecomputaDependencias(g_tabelas, &proto);
    EXPECT_EQ(da->dano(), "1");
    EXPECT_TRUE(da->cura());
  }
}

TEST(TesteFeiticos, TesteBencao) {
  std::unique_ptr<Entidade> referencia;
  {
    EntidadeProto proto;
    proto.set_gerar_agarrar(false);
    auto* ic = proto.add_info_classes();
    ic->set_id("clerigo");
    ic->set_nivel(3);
    AtribuiBaseAtributo(12, TA_FORCA, &proto);
    AtribuiBaseAtributo(14, TA_SABEDORIA, &proto);
    auto* da = proto.add_dados_ataque();
    da->set_tipo_ataque("Feitiço de Clérigo");
    da->set_id_arma("bencao");
    referencia = NovaEntidadeParaTestes(proto, g_tabelas);
  }
  CentralColetora central;
  ntf::Notificacao grupo_desfazer;
  std::vector<int> ids_unicos = IdsUnicosEntidade(*referencia);
  const auto* da = referencia->DadoCorrente();
  ASSERT_NE(da, nullptr);
  AcaoProto acao = da->acao();
  AplicaEfeitosAdicionais(
      g_tabelas, /*atraso_s=*/0, /*salvou=*/false, *referencia, *referencia, TAL_DESCONHECIDO, *da, acao.add_por_entidade(),
      &acao, &ids_unicos, &ids_unicos,
      &grupo_desfazer, &central);
  ASSERT_FALSE(central.Notificacoes().empty());
  ASSERT_FALSE(central.Notificacoes()[0]->entidade().evento().empty());
  const auto& evento = central.Notificacoes()[0]->entidade().evento(0);
  EXPECT_EQ(evento.id_efeito(), EFEITO_BENCAO);
  EXPECT_EQ(evento.rodadas(), 30);
}

TEST(TesteFeiticos, TesteMaldicaoMenor) {
  std::unique_ptr<Entidade> referencia;
  {
    EntidadeProto proto;
    proto.set_gerar_agarrar(false);
    auto* ic = proto.add_info_classes();
    ic->set_id("clerigo");
    ic->set_nivel(3);
    AtribuiBaseAtributo(12, TA_FORCA, &proto);
    AtribuiBaseAtributo(14, TA_SABEDORIA, &proto);
    auto* da = proto.add_dados_ataque();
    da->set_tipo_ataque("Feitiço de Clérigo");
    da->set_id_arma("maldicao_menor");
    referencia = NovaEntidadeParaTestes(proto, g_tabelas);
  }
  CentralColetora central;
  ntf::Notificacao grupo_desfazer;
  std::vector<int> ids_unicos = IdsUnicosEntidade(*referencia);
  const auto* da = referencia->DadoCorrente();
  ASSERT_NE(da, nullptr);
  AcaoProto acao = da->acao();
  AplicaEfeitosAdicionais(
      g_tabelas, /*atraso_s=*/0, /*salvou=*/false, *referencia, *referencia, TAL_INIMIGO, *da, acao.add_por_entidade(),
      &acao, &ids_unicos, &ids_unicos,
      &grupo_desfazer, &central);
  ASSERT_FALSE(central.Notificacoes().empty());
  ASSERT_FALSE(central.Notificacoes()[0]->entidade().evento().empty());
  const auto& evento = central.Notificacoes()[0]->entidade().evento(0);
  EXPECT_EQ(evento.id_efeito(), EFEITO_MALDICAO_MENOR);
  EXPECT_EQ(evento.rodadas(), 30);
}

TEST(TesteFeiticos, TesteOracao) {
  std::unique_ptr<Entidade> referencia;
  {
    EntidadeProto proto;
    proto.set_gerar_agarrar(false);
    auto* ic = proto.add_info_classes();
    ic->set_id("clerigo");
    ic->set_nivel(3);
    auto* talento = proto.mutable_info_talentos()->add_gerais();
    talento->set_id("usar_armas_comuns");
    talento->set_complemento("espada_curta");

    AtribuiBaseAtributo(12, TA_FORCA, &proto);
    AtribuiBaseAtributo(16, TA_DESTREZA, &proto);
    AtribuiBaseAtributo(14, TA_SABEDORIA, &proto);
    auto* da = proto.add_dados_ataque();
    da->set_tipo_ataque("Feitiço de Clérigo");
    da->set_id_arma("oracao");
    da = proto.add_dados_ataque();
    da->set_id_arma("espada_curta");

    proto.mutable_tesouro()->add_itens_mundanos()->set_id("pedra_trovao");
    referencia = NovaEntidadeParaTestes(proto, g_tabelas);
  }
  CentralColetora central;
  ntf::Notificacao grupo_desfazer;
  std::vector<int> ids_unicos = IdsUnicosEntidade(*referencia);
  const auto* da = referencia->DadoCorrente();
  ASSERT_NE(da, nullptr);
  AcaoProto acao = da->acao();
  AplicaEfeitosAdicionais(
      g_tabelas, /*atraso_s=*/0, /*salvou=*/false, *referencia, *referencia, TAL_ALIADO, *da, acao.add_por_entidade(),
      &acao, &ids_unicos, &ids_unicos,
      &grupo_desfazer, &central);
  {
    ASSERT_EQ(central.Notificacoes().size(), 1U);
    ASSERT_FALSE(central.Notificacoes()[0]->entidade().evento().empty());
    const auto& evento = central.Notificacoes()[0]->entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_ORACAO_ALIADOS);
    EXPECT_EQ(evento.rodadas(), 3);
    // Pedra do trovao nao pode ser afetada por oracao.
    referencia->AtualizaParcial(central.Notificacoes()[0]->entidade());
    ASSERT_EQ(referencia->Proto().dados_ataque().size(), 3);
    EXPECT_EQ(referencia->Proto().dados_ataque(1).bonus_ataque_final(), 2 + 1 + 1) << referencia->Proto().dados_ataque(1).bonus_ataque().DebugString();
    EXPECT_EQ(referencia->Proto().dados_ataque(1).dano(), "1d6+2");
    EXPECT_EQ(referencia->Proto().dados_ataque(2).bonus_ataque_final(), 2 + 3 + 1) << referencia->Proto().dados_ataque(2).bonus_ataque().DebugString();
    EXPECT_TRUE(referencia->Proto().dados_ataque(2).dano().empty());
  }

  AplicaEfeitosAdicionais(
      g_tabelas, /*atraso_s=*/0, /*salvou=*/false, *referencia, *referencia, TAL_INIMIGO, *da, acao.add_por_entidade(),
      &acao, &ids_unicos, &ids_unicos,
      &grupo_desfazer, &central);
  ASSERT_EQ(central.Notificacoes().size(), 2U);
  ASSERT_FALSE(central.Notificacoes()[1]->entidade().evento().empty());
  {
    const auto& evento = central.Notificacoes()[1]->entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_ORACAO_INIMIGOS);
    EXPECT_EQ(evento.rodadas(), 3);
  }
}


TEST(TesteFeiticos, TesteFeiticos) {
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(3);
    AtribuiBaseAtributo(14, TA_INTELIGENCIA, &proto);
    RecomputaDependencias(g_tabelas, &proto);

    ASSERT_EQ(proto.feiticos_classes().size(), 1);
    EXPECT_EQ(proto.feiticos_classes(0).id_classe(), "mago");
    // Progressao tabelada: 4 2 1.
    ASSERT_EQ(proto.feiticos_classes(0).mapa_feiticos_por_nivel().size(), 3UL);
    // Nivel 0: Fixo em 4.
    ASSERT_EQ(proto.feiticos_classes(0).mapa_feiticos_por_nivel().at(0).para_lancar().size(), 4);
    // Nivel 1: 2 + 1 de bonus de atributo.
    ASSERT_EQ(proto.feiticos_classes(0).mapa_feiticos_por_nivel().at(1).para_lancar().size(), 3);
    // Nivel 2: 1 + 1 de bonus de atributo.
    ASSERT_EQ(proto.feiticos_classes(0).mapa_feiticos_por_nivel().at(2).para_lancar().size(), 2);
  }
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("clerigo");
    ic->set_nivel(1);
    AtribuiBaseAtributo(12, TA_SABEDORIA, &proto);
    RecomputaDependencias(g_tabelas, &proto);

    ASSERT_EQ(proto.feiticos_classes().size(), 1);
    EXPECT_EQ(proto.feiticos_classes(0).id_classe(), "clerigo");
    // Progressao tabelada: 3 1+1.
    ASSERT_EQ(proto.feiticos_classes(0).mapa_feiticos_por_nivel().size(), 2UL);
    // Nivel 0: Fixo em 3.
    ASSERT_EQ(proto.feiticos_classes(0).mapa_feiticos_por_nivel().at(0).para_lancar().size(), 3);
    // Nivel 1: 1 + 1 dominio + 1 de bonus de atributo.
    ASSERT_EQ(proto.feiticos_classes(0).mapa_feiticos_por_nivel().at(1).para_lancar().size(), 3);
  }
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("ranger");
    ic->set_nivel(10);
    AtribuiBaseAtributo(12, TA_SABEDORIA, &proto);
    RecomputaDependencias(g_tabelas, &proto);

    ASSERT_EQ(proto.feiticos_classes().size(), 1);
    EXPECT_EQ(proto.feiticos_classes(0).id_classe(), "ranger");
    // Progressao tabelada: 1 1 sem nivel 0.
    ASSERT_EQ(proto.feiticos_classes(0).mapa_feiticos_por_nivel().size(), 2UL);
    // Nivel 1: 1 + 1 de sabedoria.
    ASSERT_EQ(proto.feiticos_classes(0).mapa_feiticos_por_nivel().at(1).para_lancar().size(), 2);
    // Nivel 2: 1.
    ASSERT_EQ(proto.feiticos_classes(0).mapa_feiticos_por_nivel().at(2).para_lancar().size(), 1);
  }
}

TEST(TesteFeiticos, TesteProtegerOutro) {
  EntidadeProto proto;
  proto.set_id(123);
  auto* ic = proto.add_info_classes();
  ic->set_id("clerigo");
  ic->set_nivel(3);
  AtribuiBaseAtributo(14, TA_SABEDORIA, &proto);
  RecomputaDependencias(g_tabelas, &proto);
  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Clérigo");
  da->set_id_arma("proteger_outro");
  std::unique_ptr<Entidade> referencia(NovaEntidadeParaTestes(proto, g_tabelas));
  const int nivel_conjurador = NivelConjurador(TipoAtaqueParaClasse(g_tabelas, da->tipo_ataque()), proto);

  RecomputaDependencias(g_tabelas, &proto);

  AcaoProto acao = da->acao();

  EntidadeProto proto_alvo;
  {
    std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto_alvo, g_tabelas));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), DadosIniciativa{.iniciativa=2, .modificador=2}, nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_PROTEGER_OUTRO);
    ASSERT_FALSE(evento.complementos().empty());
    EXPECT_EQ(evento.complementos(0), 123);
    EXPECT_EQ(evento.iniciativa(), 2);
    EXPECT_EQ(evento.modificador_iniciativa(), 2);
  }

}

TEST(TesteFeiticos, TesteMaosFlamejantesComoDominio) {
  EntidadeProto proto;
  proto.set_gerar_agarrar(false);
  AtribuiBaseAtributo(14, TA_INTELIGENCIA, &proto);
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("clerigo");
    ic->set_nivel(3);
    auto* ifc = FeiticosClasse("clerigo", &proto);
    ifc->add_dominios("fogo");
  }
  std::unique_ptr<Entidade> entidade(NovaEntidadeParaTestes(proto, g_tabelas));
  auto grupo = NovoGrupoNotificacoes();
  ExecutaFeitico(g_tabelas, g_tabelas.Feitico("maos_flamejantes"), /*nivel_conjurador=*/3, "clerigo", 1, std::nullopt, *entidade, grupo.get(), nullptr);
  for (const auto& n : grupo->notificacao()) {
    entidade->AtualizaParcial(n.entidade());
  }
  EXPECT_EQ(DadosAtaquePorGrupo("mãos flamejantes", entidade->Proto()).dano(), "3d4");
}

TEST(TesteFeiticos, TesteTramaDasSombras) {
  const auto& modelo_vulto = g_tabelas.ModeloEntidade("Vulto Feiticeiro 5");
  auto proto_vulto = modelo_vulto.entidade();
  proto_vulto.mutable_info_talentos()->add_outros()->set_id("magia_trama_sombras");
  auto vulto = NovaEntidadeParaTestes(proto_vulto, g_tabelas);
  auto grupo = NovoGrupoNotificacoes();
  ExecutaFeitico(
      g_tabelas, g_tabelas.Feitico("missil_magico"),
      /*nivel_conjurador=*/vulto->NivelConjuradorParaMagia("mago", g_tabelas.Feitico("missil_magico")), "mago", 1,
      std::nullopt, *vulto, grupo.get(), nullptr);
  for (const auto& n : grupo->notificacao()) {
    vulto->AtualizaParcial(n.entidade());
  }
  EXPECT_EQ(DadosAtaquePorGrupo("mísseis mágicos", vulto->Proto()).limite_vezes(), 2);
}

TEST(TesteFeiticos, TesteEsferaFlamejante) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(3);
  AtribuiBaseAtributo(14, TA_INTELIGENCIA, &proto);
  RecomputaDependencias(g_tabelas, &proto);
  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Mago");
  da->set_id_arma("esfera_flamejante");
  std::unique_ptr<Entidade> referencia(NovaEntidadeParaTestes(proto, g_tabelas));

  RecomputaDependencias(g_tabelas, &proto);

  AcaoProto acao = da->acao();
  ASSERT_EQ(acao.tipo(), ACAO_CRIACAO_ENTIDADE);
  ASSERT_EQ(acao.id_modelo_entidade(), "Esfera Flamejante");

  const auto& modelo_esfera = g_tabelas.ModeloEntidade(acao.id_modelo_entidade());
  EntidadeProto proto_esfera = modelo_esfera.entidade();
  ASSERT_TRUE(modelo_esfera.has_parametros());
  PreencheModeloComParametros(g_tabelas.Feitico(da->id_arma()), modelo_esfera.parametros(), *referencia, &proto_esfera);

  ASSERT_FALSE(proto_esfera.dados_ataque().empty());
  EXPECT_EQ(proto_esfera.dados_ataque(0).dificuldade_salvacao(), 14);

  RecomputaDependencias(g_tabelas, &proto_esfera);
  ASSERT_FALSE(proto_esfera.dados_ataque().empty());
  EXPECT_EQ(proto_esfera.dados_ataque(0).dificuldade_salvacao(), 14);

  // Sem feitico.
  {
    EntidadeProto proto_esfera = modelo_esfera.entidade();
    ASSERT_TRUE(modelo_esfera.has_parametros());
    PreencheModeloComParametros(ArmaProto::default_instance(), modelo_esfera.parametros(), *referencia, &proto_esfera);
    RecomputaDependencias(g_tabelas, &proto_esfera);
    ASSERT_FALSE(proto_esfera.dados_ataque().empty());
    EXPECT_EQ(proto_esfera.dados_ataque(0).dificuldade_salvacao(), 14);
  }
}

TEST(TesteFeiticos, TesteArmaEspiritual) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("clerigo");
  ic->set_nivel(6);
  AtribuiBaseAtributo(14, TA_SABEDORIA, &proto);
  RecomputaDependencias(g_tabelas, &proto);
  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Clérigo");
  da->set_id_arma("arma_espiritual");
  std::unique_ptr<Entidade> referencia(NovaEntidadeParaTestes(proto, g_tabelas));

  RecomputaDependencias(g_tabelas, &proto);

  AcaoProto acao = da->acao();
  ASSERT_EQ(acao.tipo(), ACAO_CRIACAO_ENTIDADE);
  ASSERT_EQ(acao.parametros_lancamento().parametros().size(), 5);
  ASSERT_EQ(acao.parametros_lancamento().parametros(3).id_modelo_entidade(), "Arma Espiritual Espada");

  const auto& modelo_arma = g_tabelas.ModeloEntidade(acao.parametros_lancamento().parametros(2).id_modelo_entidade());
  EntidadeProto proto_arma = modelo_arma.entidade();
  ASSERT_TRUE(modelo_arma.has_parametros());
  PreencheModeloComParametros(g_tabelas.Feitico(da->id_arma()), modelo_arma.parametros(), *referencia, &proto_arma);

  ASSERT_FALSE(proto_arma.dados_ataque().empty());
  EXPECT_EQ(proto_arma.dados_ataque(0).dano_basico(), "1d8+2");
}

TEST(TesteFeiticos, TesteMuralhaLaminas) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("clerigo");
  ic->set_nivel(16);
  AtribuiBaseAtributo(16, TA_SABEDORIA, &proto);
  RecomputaDependencias(g_tabelas, &proto);
  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Clérigo");
  da->set_id_arma("barreira_laminas");
  std::unique_ptr<Entidade> referencia(NovaEntidadeParaTestes(proto, g_tabelas));

  RecomputaDependencias(g_tabelas, &proto);

  AcaoProto acao = da->acao();
  ASSERT_EQ(acao.tipo(), ACAO_CRIACAO_ENTIDADE);
  ASSERT_EQ(acao.parametros_lancamento().parametros_size(), 2);
  for (int i = 0; i < acao.parametros_lancamento().parametros_size(); ++i) {
    const auto& modelo_arma = g_tabelas.ModeloEntidade(acao.parametros_lancamento().parametros(i).id_modelo_entidade());
    EntidadeProto proto_arma = modelo_arma.entidade();
    ASSERT_TRUE(modelo_arma.has_parametros());
    PreencheModeloComParametros(g_tabelas.Feitico(da->id_arma()), modelo_arma.parametros(), *referencia, &proto_arma);

    ASSERT_FALSE(proto_arma.dados_ataque().empty());
    EXPECT_EQ(proto_arma.dados_ataque(0).dano_basico(), "15d6");
  }
}

TEST(TesteFeiticos, TesteConstricao) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("druida");
  ic->set_nivel(3);
  AtribuiBaseAtributo(14, TA_SABEDORIA, &proto);
  RecomputaDependencias(g_tabelas, &proto);
  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Druida");
  da->set_id_arma("constricao");
  std::unique_ptr<Entidade> referencia(NovaEntidadeParaTestes(proto, g_tabelas));

  RecomputaDependencias(g_tabelas, &proto);

  AcaoProto acao = da->acao();
  ASSERT_EQ(acao.tipo(), ACAO_CRIACAO_ENTIDADE);
  ASSERT_EQ(acao.id_modelo_entidade(), "Constrição");

  const auto& modelo_constricao = g_tabelas.ModeloEntidade(acao.id_modelo_entidade());
  EntidadeProto proto_constricao = modelo_constricao.entidade();
  ASSERT_TRUE(modelo_constricao.has_parametros());
  PreencheModeloComParametros(g_tabelas.Feitico(da->id_arma()), modelo_constricao.parametros(), *referencia, &proto_constricao);

  ASSERT_FALSE(proto_constricao.dados_ataque().empty());
  EXPECT_EQ(proto_constricao.dados_ataque(0).dificuldade_salvacao(), 13);

  RecomputaDependencias(g_tabelas, &proto_constricao);
  ASSERT_FALSE(proto_constricao.dados_ataque().empty());
  EXPECT_EQ(proto_constricao.dados_ataque(0).dificuldade_salvacao(), 13);
}

TEST(TesteFeiticos, TesteConjuracoesPossuiModeloValido) {
  for (const auto& feitico : g_tabelas.todas().tabela_feiticos().armas()) {
    const auto& pl = feitico.acao().parametros_lancamento();
    if (pl.consequencia() != AcaoProto::CP_ATRIBUI_MODELO_ENTIDADE) continue;
    for (const auto& p : pl.parametros()) {
      EXPECT_FALSE(g_tabelas.ModeloEntidade(p.id_modelo_entidade()).id().empty()) << "nao achei: " << p.id_modelo_entidade();
    }
  }
}

TEST(TesteImunidades, TesteImunidadeElemento) {
  {
    EntidadeProto proto;
    RecomputaDependencias(g_tabelas, &proto);
    EXPECT_FALSE(EntidadeImuneElemento(proto, DESC_ACIDO));
  }
  {
    EntidadeProto proto;
    proto.mutable_dados_defesa()->add_imunidades(DESC_ACIDO);
    RecomputaDependencias(g_tabelas, &proto);
    EXPECT_TRUE(EntidadeImuneElemento(proto, DESC_ACIDO));
  }
}

TEST(TesteImunidades, TesteImunidadePadrao) {
  {
    EntidadeProto proto;
    proto.set_naturalmente_cego(true);
    RecomputaDependencias(g_tabelas, &proto);
    EXPECT_FALSE(EntidadeImuneElemento(proto, DESC_MENTAL));
    EXPECT_TRUE(EntidadeImuneElemento(proto, DESC_MENTAL_PADRAO_VISIVEL));
  }
}

TEST(TesteImunidades, TesteImunidadeFeiticoNativa) {
  {
    EntidadeProto proto;
    RecomputaDependencias(g_tabelas, &proto);
    EXPECT_FALSE(EntidadeImuneFeitico(proto, "bola_fogo"));
  }
  {
    EntidadeProto proto;
    proto.mutable_dados_defesa()->add_imunidade_feiticos()->set_id_feitico("bola_fogo");
    RecomputaDependencias(g_tabelas, &proto);
    EXPECT_TRUE(EntidadeImuneFeitico(proto, "bola_fogo"));
  }
}

TEST(TesteImunidades, TesteImunidadeFeiticoEfeito) {
  EntidadeProto proto;
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_IMUNIDADE_FEITICO);
  evento->set_rodadas(1);
  evento->add_complementos_str("bola_fogo");
  evento->set_id_unico(2);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(EntidadeImuneFeitico(proto, "bola_fogo"));

  evento->set_rodadas(-1);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_FALSE(EntidadeImuneFeitico(proto, "bola_fogo"));

  evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_IMUNIDADE_FEITICO);
  evento->set_rodadas(1);
  evento->add_complementos_str("maos_flamejantes");
  evento->set_id_unico(2);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(EntidadeImuneFeitico(proto, "maos_flamejantes"));
}

TEST(TesteImunidades, TesteReducaoDanoFormaGasosa) {
  EntidadeProto proto;
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_FORMA_GASOSA);
  evento->set_rodadas(1);
  auto* dd = proto.mutable_dados_defesa();
  dd->set_id_armadura("cota_malha");
  dd->set_bonus_magico_armadura(2);
  std::unique_ptr<Entidade> e(NovaEntidadeParaTestes(proto, g_tabelas));
  proto = e->Proto();

  EntidadeProto proto_ataque;
  auto* da = proto_ataque.add_dados_ataque();
  da->set_id_arma("adaga");
  da->set_obra_prima(true);
  RecomputaDependencias(g_tabelas, &proto_ataque);
  std::unique_ptr<Entidade> ea(NovaEntidadeParaTestes(proto, g_tabelas));
  EXPECT_EQ(e->CA(*ea, Entidade::CA_NORMAL), 10);

  ResultadoReducaoDano resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, da->descritores());
  EXPECT_EQ(resultado.delta_pv, 0) << resultado.texto;

  da->set_bonus_magico(1);
  RecomputaDependencias(g_tabelas, &proto_ataque);
  resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, da->descritores());
  EXPECT_EQ(resultado.delta_pv, -10) << resultado.texto;
}

TEST(TesteImunidades, TesteReducaoDanoSimples) {
  {
    google::protobuf::RepeatedField<int> descritores;
    descritores.Add(DESC_FERRO_FRIO);
    EntidadeProto proto;
    RecomputaDependencias(g_tabelas, &proto);
    ResultadoReducaoDano resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(resultado.delta_pv, -10);
  }
  {
    google::protobuf::RepeatedField<int> descritores;
    EntidadeProto proto;
    auto* rd = proto.mutable_dados_defesa()->add_reducao_dano();
    rd->set_valor(6);
    rd->add_descritores(DESC_FERRO_FRIO);
    RecomputaDependencias(g_tabelas, &proto);
    ResultadoReducaoDano resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(resultado.delta_pv, -4) << resultado.texto;
  }
  {
    google::protobuf::RepeatedField<int> descritores;
    descritores.Add(DESC_FERRO_FRIO);
    EntidadeProto proto;
    auto* rd = proto.mutable_dados_defesa()->add_reducao_dano();
    rd->set_valor(6);
    rd->add_descritores(DESC_FERRO_FRIO);
    RecomputaDependencias(g_tabelas, &proto);
    ResultadoReducaoDano resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(resultado.delta_pv, -10) << resultado.texto;
  }
}

TEST(TesteImunidades, TesteReducaoDanoCombinacaoE) {
  {
    google::protobuf::RepeatedField<int> descritores;
    descritores.Add(DESC_FERRO_FRIO);
    EntidadeProto proto;
    auto* rd = proto.mutable_dados_defesa()->add_reducao_dano();
    rd->set_valor(6);
    rd->set_tipo_combinacao(COMB_E);
    rd->add_descritores(DESC_FERRO_FRIO);
    rd->add_descritores(DESC_BEM);
    RecomputaDependencias(g_tabelas, &proto);
    ResultadoReducaoDano resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(resultado.delta_pv, -4) << resultado.texto;
  }
  {
    google::protobuf::RepeatedField<int> descritores;
    descritores.Add(DESC_BEM);
    EntidadeProto proto;
    auto* rd = proto.mutable_dados_defesa()->add_reducao_dano();
    rd->set_valor(6);
    rd->set_tipo_combinacao(COMB_E);
    rd->add_descritores(DESC_FERRO_FRIO);
    rd->add_descritores(DESC_BEM);
    RecomputaDependencias(g_tabelas, &proto);
    ResultadoReducaoDano resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(resultado.delta_pv, -4) << resultado.texto;
  }
  {
    google::protobuf::RepeatedField<int> descritores;
    descritores.Add(DESC_BEM);
    descritores.Add(DESC_FERRO_FRIO);
    EntidadeProto proto;
    auto* rd = proto.mutable_dados_defesa()->add_reducao_dano();
    rd->set_valor(6);
    rd->set_tipo_combinacao(COMB_E);
    rd->add_descritores(DESC_FERRO_FRIO);
    rd->add_descritores(DESC_BEM);
    RecomputaDependencias(g_tabelas, &proto);
    ResultadoReducaoDano resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(resultado.delta_pv, -10) << resultado.texto;
  }
}

TEST(TesteImunidades, TesteReducaoDanoCombinacaoOu) {
  {
    google::protobuf::RepeatedField<int> descritores;
    EntidadeProto proto;
    auto* rd = proto.mutable_dados_defesa()->add_reducao_dano();
    rd->set_valor(6);
    rd->set_tipo_combinacao(COMB_OU);
    rd->add_descritores(DESC_FERRO_FRIO);
    rd->add_descritores(DESC_BEM);
    RecomputaDependencias(g_tabelas, &proto);
    ResultadoReducaoDano resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(resultado.delta_pv, -4) << resultado.texto;
  }
  {
    google::protobuf::RepeatedField<int> descritores;
    descritores.Add(DESC_BEM);
    EntidadeProto proto;
    auto* rd = proto.mutable_dados_defesa()->add_reducao_dano();
    rd->set_valor(6);
    rd->set_tipo_combinacao(COMB_OU);
    rd->add_descritores(DESC_FERRO_FRIO);
    rd->add_descritores(DESC_BEM);
    RecomputaDependencias(g_tabelas, &proto);
    ResultadoReducaoDano resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(resultado.delta_pv, -10) << resultado.texto;
  }
  {
    google::protobuf::RepeatedField<int> descritores;
    descritores.Add(DESC_FERRO_FRIO);
    EntidadeProto proto;
    auto* rd = proto.mutable_dados_defesa()->add_reducao_dano();
    rd->set_valor(6);
    rd->set_tipo_combinacao(COMB_OU);
    rd->add_descritores(DESC_FERRO_FRIO);
    rd->add_descritores(DESC_BEM);
    RecomputaDependencias(g_tabelas, &proto);
    ResultadoReducaoDano resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(resultado.delta_pv, -10) << resultado.texto;
  }
}

TEST(TesteImunidades, TesteReducaoDanoCombinacaoOuProtoAtaqueSucesso) {
  EntidadeProto proto_defesa;
  auto* rd = proto_defesa.mutable_dados_defesa()->add_reducao_dano();
  rd->set_valor(6);
  rd->set_tipo_combinacao(COMB_OU);
  rd->add_descritores(DESC_FERRO_FRIO);
  rd->add_descritores(DESC_BEM);
  RecomputaDependencias(g_tabelas, &proto_defesa);

  EntidadeProto proto_ataque;
  auto* da = proto_ataque.add_dados_ataque();
  da->set_material_arma(DESC_FERRO_FRIO);
  RecomputaDependencias(g_tabelas, &proto_ataque);

  ResultadoReducaoDano resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto_defesa, proto_ataque.dados_ataque(0).descritores());
  EXPECT_EQ(resultado.delta_pv, -10) << resultado.texto;
}

TEST(TesteImunidades, TesteReducaoDanoCombinacaoEProtoAtaqueFalhou) {
  EntidadeProto proto_defesa;
  auto* rd = proto_defesa.mutable_dados_defesa()->add_reducao_dano();
  rd->set_valor(6);
  rd->set_tipo_combinacao(COMB_E);
  rd->add_descritores(DESC_FERRO_FRIO);
  rd->add_descritores(DESC_BEM);
  RecomputaDependencias(g_tabelas, &proto_defesa);

  EntidadeProto proto_ataque;
  auto* da = proto_ataque.add_dados_ataque();
  da->set_material_arma(DESC_FERRO_FRIO);
  RecomputaDependencias(g_tabelas, &proto_ataque);

  ResultadoReducaoDano resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto_defesa, proto_ataque.dados_ataque(0).descritores());
  EXPECT_EQ(resultado.delta_pv, -4) << resultado.texto;
}

TEST(TesteImunidades, TesteReducaoDanoCombinacaoEProtoAtaqueSucesso) {
  EntidadeProto proto_defesa;
  auto* rd = proto_defesa.mutable_dados_defesa()->add_reducao_dano();
  rd->set_valor(6);
  rd->set_tipo_combinacao(COMB_E);
  rd->add_descritores(DESC_FERRO_FRIO);
  rd->add_descritores(DESC_PERFURANTE);
  rd->add_descritores(DESC_CORTANTE);
  RecomputaDependencias(g_tabelas, &proto_defesa);

  EntidadeProto proto_ataque;
  auto* da = proto_ataque.add_dados_ataque();
  da->set_material_arma(DESC_FERRO_FRIO);
  da->set_id_arma("adaga");  // implica DESC_PERFURANTE e DESC_CORTANTE
  RecomputaDependencias(g_tabelas, &proto_ataque);

  ResultadoReducaoDano resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto_defesa, proto_ataque.dados_ataque(0).descritores());
  EXPECT_EQ(resultado.delta_pv, -10) << resultado.texto;
}

TEST(TesteImunidades, TesteReducaoDanoCombinacaoEProtoAtaqueAlinhadoSucesso) {
  EntidadeProto proto_defesa;
  auto* rd = proto_defesa.mutable_dados_defesa()->add_reducao_dano();
  rd->set_valor(6);
  rd->set_tipo_combinacao(COMB_E);
  rd->add_descritores(DESC_FERRO_FRIO);
  rd->add_descritores(DESC_BEM);
  RecomputaDependencias(g_tabelas, &proto_defesa);

  EntidadeProto proto_ataque;
  auto* evento = proto_ataque.add_evento();
  evento->set_id_efeito(EFEITO_ABENCOAR_ARMA);
  evento->add_complementos_str("rotulo_teste");
  evento->set_rodadas(1);
  auto* da = proto_ataque.add_dados_ataque();
  da->set_material_arma(DESC_FERRO_FRIO);
  da->set_rotulo("rotulo_teste");
  RecomputaDependencias(g_tabelas, &proto_ataque);

  ResultadoReducaoDano resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto_defesa, proto_ataque.dados_ataque(0).descritores());
  EXPECT_EQ(resultado.delta_pv, -10) << resultado.texto;
}

TEST(TesteImunidades, TesteReducaoDanoCombinacaoEProtoAtaqueAlinhado2Sucesso) {
  EntidadeProto proto_defesa;
  auto* rd = proto_defesa.mutable_dados_defesa()->add_reducao_dano();
  rd->set_valor(6);
  rd->set_tipo_combinacao(COMB_E);
  rd->add_descritores(DESC_FERRO_FRIO);
  rd->add_descritores(DESC_BEM);
  RecomputaDependencias(g_tabelas, &proto_defesa);

  EntidadeProto proto_ataque;
  auto* evento = proto_ataque.add_evento();
  evento->set_id_efeito(EFEITO_TENDENCIA_EM_ARMA);
  evento->add_complementos_str("rotulo_teste");
  evento->add_complementos_str("bem");
  evento->set_rodadas(1);
  auto* da = proto_ataque.add_dados_ataque();
  da->set_material_arma(DESC_FERRO_FRIO);
  da->set_rotulo("rotulo_teste");
  RecomputaDependencias(g_tabelas, &proto_ataque);

  ResultadoReducaoDano resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto_defesa, proto_ataque.dados_ataque(0).descritores());
  EXPECT_EQ(resultado.delta_pv, -10) << resultado.texto;
}

TEST(TesteImunidades, TesteReducaoDanoCombinacaoEProtoAtaqueAlinhadoFalha) {
  EntidadeProto proto_defesa;
  auto* rd = proto_defesa.mutable_dados_defesa()->add_reducao_dano();
  rd->set_valor(6);
  rd->set_tipo_combinacao(COMB_E);
  rd->add_descritores(DESC_FERRO_FRIO);
  rd->add_descritores(DESC_BEM);
  RecomputaDependencias(g_tabelas, &proto_defesa);

  EntidadeProto proto_ataque;
  auto* evento = proto_ataque.add_evento();
  evento->set_id_efeito(EFEITO_TENDENCIA_EM_ARMA);
  evento->add_complementos_str("rotulo_teste");
  evento->add_complementos_str("mau");
  evento->set_rodadas(1);
  auto* da = proto_ataque.add_dados_ataque();
  da->set_tipo_ataque("Ataque Corpo a Corpo");
  da->set_material_arma(DESC_FERRO_FRIO);
  da->set_rotulo("rotulo_teste");
  RecomputaDependencias(g_tabelas, &proto_ataque);

  ResultadoReducaoDano resultado = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto_defesa, proto_ataque.dados_ataque(0).descritores());
  EXPECT_EQ(resultado.delta_pv, -4) << resultado.texto;
}

TEST(TesteImunidades, TesteImunidadesNada) {
  EntidadeProto proto;
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(ImunidadeOuResistenciaParaElemento(-10, DadosAtaque::default_instance(), proto, DESC_ACIDO).causa, ALT_NENHUMA);
}

TEST(TesteImunidades, TesteEscudoVsMisseisMagicos) {
  EntidadeProto proto;
  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_ESCUDO_ARCANO);
    evento->set_id_unico(0);
    evento->set_rodadas(1);
  }
  RecomputaDependencias(g_tabelas, &proto);
  DadosAtaque da;
  da.set_id_arma("missil_magico");
  auto resistencia = ImunidadeOuResistenciaParaElemento(-10, da, proto, DESC_NENHUM);
  EXPECT_EQ(resistencia.causa, ALT_IMUNIDADE);
  EXPECT_EQ(resistencia.resistido, 10);
}

TEST(TesteImunidades, TesteImunidade) {
  EntidadeProto proto;
  proto.mutable_dados_defesa()->add_imunidades(DESC_ACIDO);
  RecomputaDependencias(g_tabelas, &proto);
  auto resultado = ImunidadeOuResistenciaParaElemento(-10, DadosAtaque::default_instance(), proto, DESC_ACIDO);
  EXPECT_EQ(resultado.causa, ALT_IMUNIDADE);
  EXPECT_EQ(resultado.resistido, 10);
}

TEST(TesteImunidades, TesteResistencia) {
  EntidadeProto proto;
  auto* resistencia = proto.mutable_dados_defesa()->add_resistencia_elementos();
  resistencia->set_valor(10);
  resistencia->set_descritor(DESC_ACIDO);
  RecomputaDependencias(g_tabelas, &proto);
  auto resultado = ImunidadeOuResistenciaParaElemento(-10, DadosAtaque::default_instance(), proto, DESC_ACIDO);
  EXPECT_EQ(resultado.causa, ALT_RESISTENCIA);
  EXPECT_EQ(resultado.resistido, 10);
}

TEST(TesteImunidades, TesteMultiplasResistencia) {
  EntidadeProto proto;
  auto* resistencia = proto.mutable_dados_defesa()->add_resistencia_elementos();
  resistencia->set_valor(10);
  resistencia->set_descritor(DESC_ACIDO);
  auto* resistencia2 = proto.mutable_dados_defesa()->add_resistencia_elementos();
  resistencia2->set_valor(12);
  resistencia2->set_descritor(DESC_ACIDO);

  RecomputaDependencias(g_tabelas, &proto);
  auto resultado = ImunidadeOuResistenciaParaElemento(-15, DadosAtaque::default_instance(), proto, DESC_ACIDO);
  EXPECT_EQ(resultado.causa, ALT_RESISTENCIA);
  EXPECT_EQ(resultado.resistido, 12);
}

TEST(TesteImunidades, TesteResistenciaNaoBate) {
  EntidadeProto proto;
  auto* resistencia = proto.mutable_dados_defesa()->add_resistencia_elementos();
  resistencia->set_valor(10);
  resistencia->set_descritor(DESC_ACIDO);

  RecomputaDependencias(g_tabelas, &proto);
  auto resultado = ImunidadeOuResistenciaParaElemento(-10, DadosAtaque::default_instance(), proto, DESC_FOGO);
  EXPECT_EQ(resultado.causa, ALT_NENHUMA);
}

TEST(TesteImunidades, TesteCelestial) {
  EntidadeProto proto;
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("druida");
    ic->set_nivel(1);
    auto* resistencia = proto.mutable_dados_defesa()->add_resistencia_elementos();
    resistencia->set_valor(10);
    resistencia->set_descritor(DESC_ACIDO);
  }
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(proto.dados_defesa().resistencia_magia(), 0);
  EXPECT_EQ(proto.dados_defesa().resistencia_elementos().size(), 1);

  auto* celestial = proto.add_modelos();
  // RM: 6, resistencia frio, acido ou eletricidade: 5.
  celestial->set_id_efeito(EFEITO_MODELO_CELESTIAL);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(proto.dados_defesa().resistencia_magia(), 6);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos().size(), 4);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(0).valor(), 10);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(0).descritor(), DESC_ACIDO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).valor(), 5);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).descritor(), DESC_FRIO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).id_efeito_modelo(), EFEITO_MODELO_CELESTIAL);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).valor(), 5);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).descritor(), DESC_ACIDO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).id_efeito_modelo(), EFEITO_MODELO_CELESTIAL);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).valor(), 5);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).descritor(), DESC_ELETRICIDADE);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).id_efeito_modelo(), EFEITO_MODELO_CELESTIAL);
  // De novo pra ver se num ta quebrando nada.
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(proto.dados_defesa().resistencia_magia(), 6);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos().size(), 4);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(0).valor(), 10);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(0).descritor(), DESC_ACIDO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).valor(), 5);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).descritor(), DESC_FRIO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).valor(), 5);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).descritor(), DESC_ACIDO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).valor(), 5);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).descritor(), DESC_ELETRICIDADE);
}

TEST(TesteImunidades, TesteAbissal) {
  EntidadeProto proto;
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("druida");
    ic->set_nivel(4);
    auto* resistencia = proto.mutable_dados_defesa()->add_resistencia_elementos();
    resistencia->set_valor(10);
    resistencia->set_descritor(DESC_ACIDO);
    resistencia = proto.mutable_dados_defesa()->add_resistencia_elementos();
    resistencia->set_valor(11);
    resistencia->set_descritor(DESC_FOGO);
  }
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(proto.dados_defesa().resistencia_magia(), 0);
  EXPECT_EQ(proto.dados_defesa().resistencia_elementos().size(), 2);

  auto* celestial = proto.add_modelos();
  // RM: 9, frio, fogo: 5, reducao: 5.
  celestial->set_id_efeito(EFEITO_MODELO_ABISSAL);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(proto.dados_defesa().resistencia_magia(), 9);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos().size(), 4);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(0).valor(), 10);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(0).descritor(), DESC_ACIDO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).valor(), 11);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).descritor(), DESC_FOGO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).valor(), 5);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).descritor(), DESC_FRIO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).id_efeito_modelo(), EFEITO_MODELO_ABISSAL);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).valor(), 5);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).descritor(), DESC_FOGO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).id_efeito_modelo(), EFEITO_MODELO_ABISSAL);
  ASSERT_FALSE(proto.dados_defesa().reducao_dano().empty());
  ASSERT_EQ(proto.dados_defesa().reducao_dano(0).id_efeito_modelo(), EFEITO_MODELO_ABISSAL);
  ASSERT_TRUE(c_any(proto.dados_defesa().reducao_dano(0).descritores(), DESC_MAGICO));
  // De novo pra ver se num ta quebrando nada.
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(proto.dados_defesa().resistencia_magia(), 9);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos().size(), 4);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(0).valor(), 10);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(0).descritor(), DESC_ACIDO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).valor(), 11);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).descritor(), DESC_FOGO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).valor(), 5);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).descritor(), DESC_FRIO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).id_efeito_modelo(), EFEITO_MODELO_ABISSAL);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).valor(), 5);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).descritor(), DESC_FOGO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).id_efeito_modelo(), EFEITO_MODELO_ABISSAL);
  ASSERT_FALSE(proto.dados_defesa().reducao_dano().empty());
  ASSERT_EQ(proto.dados_defesa().reducao_dano(0).id_efeito_modelo(), EFEITO_MODELO_ABISSAL);
  ASSERT_TRUE(c_any(proto.dados_defesa().reducao_dano(0).descritores(), DESC_MAGICO));
}

TEST(TesteAfetaApenas, TesteAfetaApenasNegativo) {
  EntidadeProto proto;
  std::unique_ptr<Entidade> e(NovaEntidadeParaTestes(proto, g_tabelas));

  AcaoProto acao;
  acao.add_afeta_apenas(TIPO_MORTO_VIVO);

  EXPECT_FALSE(AcaoAfetaAlvo(acao, *e));
}

TEST(TesteAfetaApenas, TesteAfetaApenasPositivo) {
  EntidadeProto proto;
  proto.add_tipo_dnd(TIPO_MORTO_VIVO);
  std::unique_ptr<Entidade> e(NovaEntidadeParaTestes(proto, g_tabelas));

  AcaoProto acao;
  acao.add_afeta_apenas(TIPO_MORTO_VIVO);

  EXPECT_TRUE(AcaoAfetaAlvo(acao, *e));
}

TEST(TesteAfetaApenas, TesteAfetaApenasGenerico) {
  EntidadeProto proto;
  std::unique_ptr<Entidade> e(NovaEntidadeParaTestes(proto, g_tabelas));

  AcaoProto acao;

  EXPECT_TRUE(AcaoAfetaAlvo(acao, *e));
}

TEST(TesteCuraAcelerada, TesteCuraAcelerada) {
  EntidadeProto proto;
  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_CURA_ACELERADA);
    evento->add_complementos(5);
    evento->set_id_unico(0);
  }
  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_CURA_ACELERADA);
    evento->add_complementos(3);
    evento->set_id_unico(1);
  }
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(5, CuraAcelerada(proto));
}

TEST(TesteModelo, TesteEnxameCentepeias) {
  auto proto = g_tabelas.ModeloEntidade("Enxame (centopéias)").entidade();
  auto enxame = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("enxame", enxame->Proto());
    EXPECT_EQ(da.dano(), "2d6");
    EXPECT_EQ(da.veneno().cd(), 13);
    EXPECT_EQ(da.ca_normal(), 18) << enxame->Proto().dados_defesa().ca().DebugString();
  }
  EXPECT_EQ(ValorFinalPericia("escalar", enxame->Proto()), 12) << BonusPericia("escalar", enxame->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("observar", enxame->Proto()), 4) << BonusPericia("observar", enxame->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("esconderse", enxame->Proto()), 16) << BonusPericia("esconderse", enxame->Proto()).DebugString();
}

TEST(TesteModelo, TesteEnxameRatos) {
  auto proto = g_tabelas.ModeloEntidade("Enxame (ratos)").entidade();
  auto enxame = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("enxame", enxame->Proto());
    EXPECT_EQ(da.dano(), "1d6");
    EXPECT_EQ(da.doenca().cd(), 12);
    EXPECT_EQ(da.ca_normal(), 14) << enxame->Proto().dados_defesa().ca().DebugString();
  }
  EXPECT_EQ(ValorFinalPericia("equilibrio", enxame->Proto()), 10) << BonusPericia("equilibrio", enxame->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("escalar", enxame->Proto()), 10) << BonusPericia("escalar", enxame->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("esconderse", enxame->Proto()), 16) << BonusPericia("esconderse", enxame->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("ouvir", enxame->Proto()), 6) << BonusPericia("ouvir", enxame->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("furtividade", enxame->Proto()), 8) << BonusPericia("furtividade", enxame->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("observar", enxame->Proto()), 7) << BonusPericia("observar", enxame->Proto()).DebugString();
}

TEST(TesteModelo, TesteFungoVioleta) {
  auto proto = g_tabelas.ModeloEntidade("Fungo Violeta").entidade();
  auto fungo = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("corpo a corpo", fungo->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 3);
    EXPECT_EQ(da.dano(), "1d6+2");
    EXPECT_EQ(da.veneno().cd(), 14);
    EXPECT_EQ(da.ca_normal(), 13) << fungo->Proto().dados_defesa().ca().DebugString();
  }
}

TEST(TesteModelo, TesteFungoVioleta6DV) {
  auto proto = g_tabelas.ModeloEntidade("Fungo Violeta 6 DV").entidade();
  auto fungo = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("corpo a corpo", fungo->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 6);
    EXPECT_EQ(da.dano(), "1d6+2");
    EXPECT_EQ(da.veneno().cd(), 16);
    EXPECT_EQ(da.ca_normal(), 13) << fungo->Proto().dados_defesa().ca().DebugString();
  }
}

TEST(TesteCuraAcelerada, TesteCuraAcelerada2) {
  EntidadeProto proto;
  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_CURA_ACELERADA);
    evento->add_complementos(5);
  }
  // Vai dar max de 15 PV. 2 de dano temporario, 10 de dano normal.
  AtribuiBonus(1, TB_BASE, "base", proto.mutable_niveis_negativos_dinamicos());
  proto.set_max_pontos_vida(20);
  proto.set_pontos_vida(13);
  proto.set_dano_nao_letal(2);

  ntf::Notificacao n;
  std::unique_ptr<Entidade> e(NovaEntidadeParaTestes(proto, g_tabelas));
  PreencheNotificacaoCuraAcelerada(*e, &n);
  e->AtualizaParcial(n.entidade());

  EXPECT_EQ(e->DanoNaoLetal(), 0);
  EXPECT_EQ(e->PontosVida(), 15);
  EXPECT_EQ(e->MaximoPontosVida(), 15);
}

TEST(TesteModelo, TesteEspecialista7) {
  auto proto = g_tabelas.ModeloEntidade("Humana Especialista (Tecelã) 7").entidade();
  auto esp = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("corpo a corpo", esp->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 7);
    EXPECT_EQ(da.dano(), "1d4");
    EXPECT_EQ(da.ca_normal(), 16) << esp->Proto().dados_defesa().ca().DebugString();
  }
  {
    const auto& da = DadosAtaquePorGrupo("distancia", esp->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 7);
    EXPECT_EQ(da.dano(), "1d4-1");
    EXPECT_EQ(da.ca_normal(), 16) << esp->Proto().dados_defesa().ca().DebugString();
  }
  EXPECT_EQ(ValorFinalPericia("oficios", esp->Proto()), 17) << BonusPericia("oficios", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("oficios_alquimia", esp->Proto()), 14) << BonusPericia("oficios_alquimia", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("conhecimento_nobreza_e_realeza", esp->Proto()), 12) << BonusPericia("conhecimento_nobreza_e_realeza", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("sentir_motivacao", esp->Proto()), 9) << BonusPericia("sentir_motivacao", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("avaliacao", esp->Proto()), 12) << BonusPericia("avaliacao", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("avaliacao_oficio", esp->Proto()), 14) << BonusPericia("avaliacao_oficio", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("avaliacao_alquimia", esp->Proto()), 14) << BonusPericia("avaliacao_alquimia", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("conhecimento_religiao", esp->Proto()), 12)  << BonusPericia("conhecimento_religiao", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("obter_informacao", esp->Proto()), 10) << BonusPericia("obter_informacao", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("blefar", esp->Proto()), 10) << BonusPericia("blefar", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("ouvir", esp->Proto()), 4) << BonusPericia("ouvir", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("intimidacao", esp->Proto()), 7) << BonusPericia("intimidacao", esp->Proto()).DebugString();
}

TEST(TesteModelo, TesteAnaoGuerreiro1Especialista11) {
  auto proto = g_tabelas.ModeloEntidade("Anão Guerreiro 1/Especialista (Pedreiro) 11").entidade();
  auto esp = NovaEntidadeParaTestes(proto, g_tabelas);
  EXPECT_EQ(esp->Proto().dados_vida(), "10+11d6+24");
  {
    const auto& da = DadosAtaquePorGrupo("corpo a corpo", esp->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 14);
    EXPECT_EQ(da.dano(), "1d8+6");
    EXPECT_EQ(da.ca_normal(), 24) << esp->Proto().dados_defesa().ca().DebugString();
  }
  {
    const auto& da = DadosAtaquePorGrupo("distancia", esp->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 10);
    EXPECT_EQ(da.dano(), "1d4+2");
    EXPECT_EQ(da.ca_normal(), 24) << esp->Proto().dados_defesa().ca().DebugString();
  }
  EXPECT_EQ(ValorFinalPericia("oficios_pedreiro", esp->Proto()), 24) << BonusPericia("oficios", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("oficios_alquimia", esp->Proto()), 19) << BonusPericia("oficios_alquimia", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("conhecimento_masmorras", esp->Proto()), 18) << BonusPericia("conhecimento_masmorras", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("sentir_motivacao", esp->Proto()), 14) << BonusPericia("sentir_motivacao", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("avaliacao", esp->Proto()), 19) << BonusPericia("avaliacao", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("avaliacao_pedreiro", esp->Proto()), 21) << BonusPericia("avaliacao_oficio", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("avaliacao_alquimia", esp->Proto()), 21) << BonusPericia("avaliacao_alquimia", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("blefar", esp->Proto()), 16) << BonusPericia("blefar", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("ouvir", esp->Proto()), 12) << BonusPericia("ouvir", esp->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("intimidacao", esp->Proto()), 17) << BonusPericia("intimidacao", esp->Proto()).DebugString();
}

TEST(TesteModelo, TesteWorg) {
  auto proto = g_tabelas.ModeloEntidade("Worg").entidade();
  auto worg = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("mordida", worg->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 7);
    EXPECT_EQ(da.dano(), "1d6+4");
    EXPECT_EQ(da.ca_normal(), 14) << worg->Proto().dados_defesa().ca().DebugString();
  }

  EXPECT_EQ(ValorFinalPericia("ouvir", worg->Proto()), 6);
  EXPECT_EQ(ValorFinalPericia("furtividade", worg->Proto()), 6);
  EXPECT_EQ(ValorFinalPericia("observar", worg->Proto()), 6);
  EXPECT_EQ(ValorFinalPericia("esconderse", worg->Proto()), 6);
  EXPECT_EQ(ValorFinalPericia("rastrear", worg->Proto()), 6);
}

TEST(TesteModelo, TesteGoblin) {
  auto proto = g_tabelas.ModeloEntidade("Goblin").entidade();
  auto goblin = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("maca_estrela", goblin->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 2);
    EXPECT_EQ(da.dano(), "1d6");
    EXPECT_EQ(da.ca_normal(), 15) << goblin->Proto().dados_defesa().ca().DebugString();
  }
  {
    const auto& da = DadosAtaquePorGrupo("azagaia", goblin->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 3);
    EXPECT_EQ(da.dano(), "1d4");
    EXPECT_EQ(da.ca_normal(), 15) << goblin->Proto().dados_defesa().ca().DebugString();
  }

  EXPECT_EQ(ValorFinalPericia("esconderse", goblin->Proto()), 5);
  EXPECT_EQ(ValorFinalPericia("ouvir", goblin->Proto()), 2);
  EXPECT_EQ(ValorFinalPericia("furtividade", goblin->Proto()), 5);
  // Valor do livro é 4, mas num faz sentido pq tem bonus racial + destreza.
  EXPECT_EQ(ValorFinalPericia("cavalgar", goblin->Proto()), 5);
  EXPECT_EQ(ValorFinalPericia("observar", goblin->Proto()), 2);
}

TEST(TesteModelo, TesteKobold) {
  auto proto = g_tabelas.ModeloEntidade("Kobold").entidade();
  auto kobold = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("lanca", kobold->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 1);
    EXPECT_EQ(da.dano(), "1d6-1");
    EXPECT_EQ(da.ca_normal(), 15) << kobold->Proto().dados_defesa().ca().DebugString();
  }
  {
    const auto& da = DadosAtaquePorGrupo("funda", kobold->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 3);
    EXPECT_EQ(da.dano(), "1d3-1");
    EXPECT_EQ(da.ca_normal(), 15) << kobold->Proto().dados_defesa().ca().DebugString();
  }

  // Valor é 0 por causa da falta de ferramentas.
  EXPECT_EQ(ValorFinalPericia("oficios_armadilharia", kobold->Proto()), 0) << BonusPericia("oficios_armadilharia", kobold->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("esconderse", kobold->Proto()), 6);
  EXPECT_EQ(ValorFinalPericia("ouvir", kobold->Proto()), 2);
  EXPECT_EQ(ValorFinalPericia("furtividade", kobold->Proto()), 2);
  EXPECT_EQ(ValorFinalPericia("profissao", kobold->Proto()), 2) << BonusPericia("profissao", kobold->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("procurar", kobold->Proto()), 2);
  EXPECT_EQ(ValorFinalPericia("observar", kobold->Proto()), 2);
}

TEST(TesteModelo, TesteLadino5) {
  auto proto = g_tabelas.ModeloEntidade("Humano Ladino 5").entidade();
  auto ladino = NovaEntidadeParaTestes(proto, g_tabelas);
  EXPECT_EQ(ladino->CA(*ladino, Entidade::CA_NORMAL), 13+3+1+1) << ladino->Proto().dados_defesa().ca().DebugString();
  {
    const auto& da = DadosAtaquePorGrupo("sabre", ladino->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 3+1+1);
    EXPECT_EQ(da.dano(), "1d6+1");
    EXPECT_EQ(da.ca_normal(), 13+3+1+1) << ladino->Proto().dados_defesa().ca().DebugString();
  }
  {
    const auto& da = DadosAtaquePorGrupo("adaga", ladino->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 3+1);
    EXPECT_EQ(da.dano(), "1d4+1");
    EXPECT_EQ(da.ca_normal(), 13+3+1+1) << ladino->Proto().dados_defesa().ca().DebugString();
  }
  {
    const auto& da = DadosAtaquePorGrupo("arco", ladino->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 3+3+1);
    EXPECT_EQ(da.dano(), "1d6");
    EXPECT_EQ(da.ca_normal(), 13+3+1) << ladino->Proto().dados_defesa().ca().DebugString();
  }
  EXPECT_EQ(ladino->Proto().dados_ataque_global().dano_furtivo(), "3d6");

  EXPECT_EQ(ValorFinalPericia("arte_da_fuga", ladino->Proto()), 11);
  EXPECT_EQ(ValorFinalPericia("equilibrio", ladino->Proto()), 10);
  EXPECT_EQ(ValorFinalPericia("esconderse", ladino->Proto()), 11);
  EXPECT_EQ(ValorFinalPericia("blefar", ladino->Proto()), 7);
  EXPECT_EQ(ValorFinalPericia("saltar", ladino->Proto()), 8);
  EXPECT_EQ(ValorFinalPericia("furtividade", ladino->Proto()), 11);
  EXPECT_EQ(ValorFinalPericia("abrir_fechaduras", ladino->Proto()), 13);
  EXPECT_EQ(ValorFinalPericia("procurar", ladino->Proto()), 9);
  EXPECT_EQ(ValorFinalPericia("ouvir", ladino->Proto()), 8);
  EXPECT_EQ(ValorFinalPericia("observar", ladino->Proto()), 7);
  EXPECT_EQ(ValorFinalPericia("operar_mecanismo", ladino->Proto()), 12);
}

TEST(TesteModelo, TesteLadino7) {
  auto proto = g_tabelas.ModeloEntidade("Humano Ladino 7").entidade();
  auto ladino = NovaEntidadeParaTestes(proto, g_tabelas);
  EXPECT_EQ(ladino->Proto().dados_vida(), "6+6d6+7");
  {
    const auto& da = DadosAtaquePorGrupo("corpo a corpo", ladino->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 9);
    EXPECT_EQ(da.dano(), "1d6+1");
    EXPECT_EQ(da.ca_normal(), 21) << ladino->Proto().dados_defesa().ca().DebugString();
  }
  {
    const auto& da = DadosAtaquePorGrupo("distancia", ladino->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 9);
    EXPECT_EQ(da.dano(), "1d6");
    EXPECT_EQ(da.ca_normal(), 19) << ladino->Proto().dados_defesa().ca().DebugString();
  }
  EXPECT_EQ(ladino->Proto().dados_ataque_global().dano_furtivo(), "4d6");

  EXPECT_EQ(ValorFinalPericia("arte_da_fuga", ladino->Proto()), 10) << Pericia("arte_da_fuga", ladino->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("equilibrio", ladino->Proto()), 9) << Pericia("equilibrio", ladino->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("esconderse", ladino->Proto()), 10) << Pericia("esconderse", ladino->Proto()).DebugString();;
  EXPECT_EQ(ValorFinalPericia("blefar", ladino->Proto()), 6) << Pericia("blefar", ladino->Proto()).DebugString();;
  EXPECT_EQ(ValorFinalPericia("saltar", ladino->Proto()), 7) << Pericia("saltar", ladino->Proto()).DebugString();;
  EXPECT_EQ(ValorFinalPericia("furtividade", ladino->Proto()), 11) << Pericia("furtividade", ladino->Proto()).DebugString();;
  EXPECT_EQ(ValorFinalPericia("abrir_fechaduras", ladino->Proto()), 13) << Pericia("abrir_fechaduras", ladino->Proto()).DebugString();;
  EXPECT_EQ(ValorFinalPericia("procurar", ladino->Proto()), 10) << Pericia("procurar", ladino->Proto()).DebugString();;
  EXPECT_EQ(ValorFinalPericia("ouvir", ladino->Proto()), 7) << Pericia("ouvir", ladino->Proto()).DebugString();;
  EXPECT_EQ(ValorFinalPericia("observar", ladino->Proto()), 8) << Pericia("observar", ladino->Proto()).DebugString();;
  EXPECT_EQ(ValorFinalPericia("operar_mecanismo", ladino->Proto()), 9) << Pericia("operar_mecanismo", ladino->Proto()).DebugString();;
}

TEST(TesteModelo, TesteBisao) {
  auto proto = g_tabelas.ModeloEntidade("Bisão").entidade();
  auto bisao = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("chifrada", bisao->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 8);
    EXPECT_EQ(da.dano(), "1d8+9");
    EXPECT_EQ(bisao->CA(*bisao, Entidade::CA_NORMAL), 13) << bisao->Proto().dados_defesa().ca().DebugString();
  }
  EXPECT_EQ(ValorFinalPericia("ouvir", bisao->Proto()), 7);
  EXPECT_EQ(ValorFinalPericia("observar", bisao->Proto()), 5);
}

TEST(TesteModelo, TesteCachorro) {
  auto proto = g_tabelas.ModeloEntidade("Cachorro").entidade();
  auto cachorro = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("mordida", cachorro->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 2);
    EXPECT_EQ(da.dano(), "1d4+1");
    EXPECT_EQ(cachorro->CA(*cachorro, Entidade::CA_NORMAL), 15) << cachorro->Proto().dados_defesa().ca().DebugString();
  }
  // Este valor tem que estar errado no livro, nao tem como ser 7 (4 racial, 4 movimento, 1 forca).
  EXPECT_EQ(ValorFinalPericia("saltar", cachorro->Proto()), 9)
      << Pericia("saltar", cachorro->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("ouvir", cachorro->Proto()), 5);
  EXPECT_EQ(ValorFinalPericia("observar", cachorro->Proto()), 5);
  EXPECT_EQ(ValorFinalPericia("rastrear", cachorro->Proto()), 5);
  EXPECT_EQ(ValorFinalPericia("sobrevivencia", cachorro->Proto()), 1);
}

TEST(TesteModelo, TesteCachorroMontaria) {
  auto proto = g_tabelas.ModeloEntidade("Cachorro de Montaria").entidade();
  auto cachorro = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("mordida", cachorro->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 3);
    EXPECT_EQ(da.dano(), "1d6+3");
    EXPECT_EQ(cachorro->CA(*cachorro, Entidade::CA_NORMAL), 16) << cachorro->Proto().dados_defesa().ca().DebugString();
  }
  // Este valor tem que estar errado no livro, nao tem como ser 8 (4 racial, 4 movimento, 2 forca).
  EXPECT_EQ(ValorFinalPericia("saltar", cachorro->Proto()), 10) << Pericia("saltar", cachorro->Proto()).DebugString();
  EXPECT_EQ(ValorFinalPericia("ouvir", cachorro->Proto()), 5);
  EXPECT_EQ(ValorFinalPericia("observar", cachorro->Proto()), 5);
  EXPECT_EQ(ValorFinalPericia("natacao", cachorro->Proto()), 3);
  EXPECT_EQ(ValorFinalPericia("rastrear", cachorro->Proto()), 5);
  EXPECT_EQ(ValorFinalPericia("sobrevivencia", cachorro->Proto()), 1);
}

TEST(TesteModelo, TesteEscorpiao) {
  auto proto = g_tabelas.ModeloEntidade("Escorpião Monstruoso Médio").entidade();
  auto escorpiao = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("Ataque Total", escorpiao->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 2);
    EXPECT_EQ(da.dano(), "1d4+1");
    EXPECT_EQ(escorpiao->CA(*escorpiao, Entidade::CA_NORMAL), 14) << escorpiao->Proto().dados_defesa().ca().DebugString();
    EXPECT_EQ(da.dano_constricao(), "1d4+1");
  }
  {
    const auto& da = DadosAtaquePorGrupo("Ataque Total", escorpiao->Proto(), 1);
    EXPECT_EQ(da.bonus_ataque_final(), 2);
    EXPECT_EQ(da.dano(), "1d4+1");
    EXPECT_EQ(escorpiao->CA(*escorpiao, Entidade::CA_NORMAL), 14) << escorpiao->Proto().dados_defesa().ca().DebugString();
    EXPECT_EQ(da.dano_constricao(), "1d4+1");
  }
  {
    const auto& da = DadosAtaquePorGrupo("Ataque Total", escorpiao->Proto(), 2);
    EXPECT_EQ(da.bonus_ataque_final(), -3);
    EXPECT_EQ(da.dano(), "1d4");
    EXPECT_EQ(escorpiao->CA(*escorpiao, Entidade::CA_NORMAL), 14) << escorpiao->Proto().dados_defesa().ca().DebugString();
    EXPECT_TRUE(da.dano_constricao().empty());
    EXPECT_EQ(da.veneno().cd(), 13);
  }
  {
    const auto& da = DadosAtaquePorGrupo("Agarrar", escorpiao->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 2);
    EXPECT_EQ(da.dano(), "1d4+1");
  }
  EXPECT_EQ(ValorFinalPericia("escalar", escorpiao->Proto()), 5);
  EXPECT_EQ(ValorFinalPericia("esconderse", escorpiao->Proto()), 4);
  EXPECT_EQ(ValorFinalPericia("observar", escorpiao->Proto()), 4);
}

TEST(TesteModelo, TesteEscorpiaoEnorme) {
  auto proto = g_tabelas.ModeloEntidade("Escorpião Monstruoso Enorme").entidade();
  auto escorpiao = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("Ataque Total", escorpiao->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 11);
    EXPECT_EQ(da.dano(), "1d8+6");
    EXPECT_EQ(escorpiao->CA(*escorpiao, Entidade::CA_NORMAL), 20) << escorpiao->Proto().dados_defesa().ca().DebugString();
    EXPECT_EQ(da.dano_constricao(), "1d8+6");
  }
  {
    const auto& da = DadosAtaquePorGrupo("Ataque Total", escorpiao->Proto(), 1);
    EXPECT_EQ(da.bonus_ataque_final(), 11);
    EXPECT_EQ(da.dano(), "1d8+6");
    EXPECT_EQ(escorpiao->CA(*escorpiao, Entidade::CA_NORMAL), 20) << escorpiao->Proto().dados_defesa().ca().DebugString();
    EXPECT_EQ(da.dano_constricao(), "1d8+6");
  }
  {
    const auto& da = DadosAtaquePorGrupo("Ataque Total", escorpiao->Proto(), 2);
    EXPECT_EQ(da.bonus_ataque_final(), 6);
    EXPECT_EQ(da.dano(), "2d4+3");
    EXPECT_EQ(escorpiao->CA(*escorpiao, Entidade::CA_NORMAL), 20) << escorpiao->Proto().dados_defesa().ca().DebugString();
    EXPECT_TRUE(da.dano_constricao().empty());
    EXPECT_EQ(da.veneno().cd(), 18);
  }
  {
    const auto& da = DadosAtaquePorGrupo("Agarrar", escorpiao->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 21);
    EXPECT_EQ(da.dano(), "1d8+9");
  }
  EXPECT_EQ(ValorFinalPericia("escalar", escorpiao->Proto()), 10);
  EXPECT_EQ(ValorFinalPericia("esconderse", escorpiao->Proto()), -4);
  EXPECT_EQ(ValorFinalPericia("observar", escorpiao->Proto()), 4);
}

TEST(TesteModelo, TesteArbustoErrante) {
  auto proto = g_tabelas.ModeloEntidade("Arbusto Errante").entidade();
  auto arbusto = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("total_com_constricao", arbusto->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 11);
    EXPECT_EQ(da.dano(), "2d6+5");
    EXPECT_EQ(arbusto->CA(*arbusto, Entidade::CA_NORMAL), 20) << arbusto->Proto().dados_defesa().ca().DebugString();
    EXPECT_TRUE(da.dano_constricao().empty());
  }
  {
    const auto& da = DadosAtaquePorGrupo("total_com_constricao", arbusto->Proto(), 1);
    EXPECT_EQ(da.bonus_ataque_final(), 11);
    EXPECT_EQ(da.dano(), "2d6+5");
    EXPECT_EQ(arbusto->CA(*arbusto, Entidade::CA_NORMAL), 20) << arbusto->Proto().dados_defesa().ca().DebugString();
    EXPECT_EQ(da.dano_constricao(), "2d6+7");
  }
  {
    EntidadeProto pt;
    *pt.mutable_dados_ataque() = arbusto->Proto().dados_ataque();
    const auto& da = DadosAtaquePorGrupo("Agarrar", arbusto->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 15) << da.DebugString();
    EXPECT_EQ(da.dano(), "2d6+7") << da.DebugString();
  }

  EXPECT_EQ(ValorFinalPericia("esconderse", arbusto->Proto()), 3);
  EXPECT_EQ(ValorFinalPericia("esconderse_pantano", arbusto->Proto()), 11);
  EXPECT_EQ(ValorFinalPericia("ouvir", arbusto->Proto()), 8);
  EXPECT_EQ(ValorFinalPericia("furtividade", arbusto->Proto()), 8);
  EXPECT_TRUE(EntidadeImuneElemento(arbusto->Proto(), DESC_ELETRICIDADE));
}

TEST(TesteModelo, TesteStirge) {
  auto proto = g_tabelas.ModeloEntidade("Stirge").entidade();
  auto stirge = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("picada", stirge->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 7);
    EXPECT_EQ(da.dano(), "");
    EXPECT_EQ(stirge->CA(*stirge, Entidade::CA_NORMAL), 16) << stirge->Proto().dados_defesa().ca().DebugString();
  }
  {
    const auto& da = DadosAtaquePorGrupo("agarrar", stirge->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 1);
    EXPECT_EQ(da.dano(), "");
  }
  EXPECT_EQ(ValorFinalPericia("esconderse", stirge->Proto()), 14);
  EXPECT_EQ(ValorFinalPericia("ouvir", stirge->Proto()), 4);
  EXPECT_EQ(ValorFinalPericia("observar", stirge->Proto()), 4);
}

TEST(TesteModelo, TesteCarcaju) {
  auto proto = g_tabelas.ModeloEntidade("Carcajú").entidade();
  auto carcaju = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("ataque", carcaju->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 4);
    EXPECT_EQ(da.dano(), "1d4+2");
    EXPECT_EQ(carcaju->CA(*carcaju, Entidade::CA_NORMAL), 14) << carcaju->Proto().dados_defesa().ca().DebugString();
  }
  {
    const auto& da = DadosAtaquePorGrupo("ataque", carcaju->Proto(), 1);
    EXPECT_EQ(da.bonus_ataque_final(), 4);
    EXPECT_EQ(da.dano(), "1d4+2");
    EXPECT_EQ(carcaju->CA(*carcaju, Entidade::CA_NORMAL), 14) << carcaju->Proto().dados_defesa().ca().DebugString();
  }
  {
    const auto& da = DadosAtaquePorGrupo("ataque", carcaju->Proto(), 2);
    EXPECT_EQ(da.bonus_ataque_final(), -1);
    EXPECT_EQ(da.dano(), "1d6+1");
    EXPECT_EQ(carcaju->CA(*carcaju, Entidade::CA_NORMAL), 14) << carcaju->Proto().dados_defesa().ca().DebugString();
  }
  EXPECT_EQ(ValorFinalPericia("escalar", carcaju->Proto()), 10);
  EXPECT_EQ(ValorFinalPericia("ouvir", carcaju->Proto()), 6);
  EXPECT_EQ(ValorFinalPericia("observar", carcaju->Proto()), 6);
}

TEST(TesteModelo, TesteCarcajuAtroz) {
  auto proto = g_tabelas.ModeloEntidade("Carcajú Atroz").entidade();
  auto carcaju = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("ataque", carcaju->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 8);
    EXPECT_EQ(da.dano(), "1d6+6");
    EXPECT_EQ(carcaju->CA(*carcaju, Entidade::CA_NORMAL), 16) << carcaju->Proto().dados_defesa().ca().DebugString();
  }
  {
    const auto& da = DadosAtaquePorGrupo("ataque", carcaju->Proto(), 1);
    EXPECT_EQ(da.bonus_ataque_final(), 8);
    EXPECT_EQ(da.dano(), "1d6+6");
    EXPECT_EQ(carcaju->CA(*carcaju, Entidade::CA_NORMAL), 16) << carcaju->Proto().dados_defesa().ca().DebugString();
  }
  {
    const auto& da = DadosAtaquePorGrupo("ataque", carcaju->Proto(), 2);
    EXPECT_EQ(da.bonus_ataque_final(), 3);
    EXPECT_EQ(da.dano(), "1d8+3");
    EXPECT_EQ(carcaju->CA(*carcaju, Entidade::CA_NORMAL), 16) << carcaju->Proto().dados_defesa().ca().DebugString();
  }
  EXPECT_EQ(ValorFinalPericia("escalar", carcaju->Proto()), 14);
  EXPECT_EQ(ValorFinalPericia("ouvir", carcaju->Proto()), 7);
  EXPECT_EQ(ValorFinalPericia("observar", carcaju->Proto()), 7);
}
TEST(TesteModelo, TesteArminho) {
  auto proto = g_tabelas.ModeloEntidade("Arminho").entidade();
  auto arminho = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("ataque", arminho->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 4);
    EXPECT_EQ(da.dano(), "1d3-4");
    EXPECT_EQ(arminho->CA(*arminho, Entidade::CA_NORMAL), 14) << arminho->Proto().dados_defesa().ca().DebugString();
  }
  EXPECT_EQ(ValorFinalPericia("equilibrio", arminho->Proto()), 12);
  EXPECT_EQ(ValorFinalPericia("escalar", arminho->Proto()), 10);
  EXPECT_EQ(ValorFinalPericia("arte_da_fuga", arminho->Proto()), 4);
  EXPECT_EQ(ValorFinalPericia("esconderse", arminho->Proto()), 11);
  // Deveria ser 8 pelo livro, mas a conta num fecha.
  EXPECT_EQ(ValorFinalPericia("furtividade", arminho->Proto()), 7);
  EXPECT_EQ(ValorFinalPericia("observar", arminho->Proto()), 3);
}

TEST(TesteModelo, TesteArminhoAtroz) {
  auto proto = g_tabelas.ModeloEntidade("Arminho Atroz").entidade();
  auto arminho = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("ataque", arminho->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 6);
    EXPECT_EQ(da.dano(), "1d6+3");
    EXPECT_EQ(arminho->CA(*arminho, Entidade::CA_NORMAL), 16) << arminho->Proto().dados_defesa().ca().DebugString();
  }
  {
    const auto& da = DadosAtaquePorGrupo("Agarrar", arminho->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 4);
  }
  EXPECT_EQ(ValorFinalPericia("esconderse", arminho->Proto()), 8);
  EXPECT_EQ(ValorFinalPericia("ouvir", arminho->Proto()), 3);
  EXPECT_EQ(ValorFinalPericia("furtividade", arminho->Proto()), 8);
  EXPECT_EQ(ValorFinalPericia("observar", arminho->Proto()), 5);
}

TEST(TesteModelo, TesteCaoInfernal) {
  auto modelo = g_tabelas.ModeloEntidade("Cão Infernal");
  auto cao = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
  {
    const auto& da = DadosAtaquePorGrupo("mordida", cao->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 5);
    EXPECT_EQ(da.dano(), "1d8+1");
    EXPECT_EQ(da.dano_adicional(), "1d6");
    EXPECT_FLOAT_EQ(da.alcance_m(), 1.5f);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 0.0f);
    EXPECT_EQ(cao->CA(*cao, Entidade::CA_NORMAL), 16) << cao->Proto().dados_defesa().ca().DebugString();
  }
  {
    const auto& da = DadosAtaquePorGrupo("sopro_fogo", cao->Proto());
    EXPECT_EQ(da.dano(), "2d6");
    EXPECT_FLOAT_EQ(da.alcance_m(), 3.f);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 0.0f);
    EXPECT_EQ(da.dificuldade_salvacao(), 13);
    EXPECT_EQ(cao->CA(*cao, Entidade::CA_NORMAL), 16) << cao->Proto().dados_defesa().ca().DebugString();
  }
  EXPECT_EQ(ValorFinalPericia("esconderse", cao->Proto()), 13);
  EXPECT_EQ(ValorFinalPericia("ouvir", cao->Proto()), 7);
  EXPECT_EQ(ValorFinalPericia("furtividade", cao->Proto()), 13);
  EXPECT_EQ(ValorFinalPericia("observar", cao->Proto()), 7);
  // Nao tem como ser 12 como no livro, devido ao movimento.
  EXPECT_EQ(ValorFinalPericia("saltar", cao->Proto()), 16);
  EXPECT_EQ(ValorFinalPericia("sobrevivencia", cao->Proto()), 15);
}


TEST(TesteModelo, TesteRanger9) {
  // Forca 16, +3.
  // Destreza 12, +1.
  auto modelo = g_tabelas.ModeloEntidade("Humana Ranger 9 Duas Armas");
  auto ranger = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
  ASSERT_EQ(ranger->NivelClasse("ranger"), 9);
  EXPECT_EQ(FeiticosNivel("ranger", 1, ranger->Proto()).conhecidos().size(), 1);
  for (const auto& c : FeiticosNivel("ranger", 1, ranger->Proto()).conhecidos()) {
    EXPECT_FALSE(c.id().empty());
    EXPECT_NE(c.id(), "auto");
  }
  {
    const auto& da = DadosAtaquePorGrupo("2 Armas", ranger->Proto(), 0);
    // 9 bab, 3 forca, 1 arma magica, -2 duas armas segunda leve.
    EXPECT_EQ(da.bonus_ataque_final(), 9+3+1-2);
    EXPECT_EQ(da.dano(), "1d8+4");
    EXPECT_FLOAT_EQ(da.alcance_m(), 1.5f);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 0.0f);
    // 4+1 camisao magico, +2 des, +1 anel, +1 amuleto.
    EXPECT_EQ(ranger->CA(*ranger, Entidade::CA_NORMAL), 19) << ranger->Proto().dados_defesa().ca().DebugString();
    EXPECT_EQ(da.margem_critico(), 17);
  }
  {
    const auto& da = DadosAtaquePorGrupo("2 Armas", ranger->Proto(), 1);
    // 9 bab, 3 forca, +1 OP -2 duas armas segunda leve, mão ruim.
    EXPECT_EQ(da.bonus_ataque_final(), 9+3+1-2) << da.DebugString();
    EXPECT_EQ(da.dano(), "1d8+1");
    EXPECT_FLOAT_EQ(da.alcance_m(), 1.5f);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 0.0f);
    EXPECT_EQ(ranger->CA(*ranger, Entidade::CA_NORMAL), 19) << ranger->Proto().dados_defesa().ca().DebugString();
    EXPECT_EQ(da.margem_critico(), 17);
  }
  {
    const auto& da = DadosAtaquePorGrupo("2 Armas", ranger->Proto(), 2);
    // 9 bab, 3 forca, 1 arma magica, -2 duas armas segunda leve, segundo ataque.
    EXPECT_EQ(da.bonus_ataque_final(), 9+3+1-2-5);
    EXPECT_EQ(da.dano(), "1d8+4");
    EXPECT_FLOAT_EQ(da.alcance_m(), 1.5f);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 0.0f);
    EXPECT_EQ(ranger->CA(*ranger, Entidade::CA_NORMAL), 19) << ranger->Proto().dados_defesa().ca().DebugString();
    EXPECT_EQ(da.margem_critico(), 17);
  }
  {
    const auto& da = DadosAtaquePorGrupo("2 Armas", ranger->Proto(), 3);
    // 9 bab, 3 forca, 1 OP, -2 duas armas segunda leve, segundo ataque.
    EXPECT_EQ(da.bonus_ataque_final(), 9+3+1-2-5);
    EXPECT_EQ(da.dano(), "1d8+1");
    EXPECT_FLOAT_EQ(da.alcance_m(), 1.5f);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 0.0f);
    EXPECT_EQ(ranger->CA(*ranger, Entidade::CA_NORMAL), 19) << ranger->Proto().dados_defesa().ca().DebugString();
    EXPECT_EQ(da.margem_critico(), 17);
  }

  {
    const auto& da = DadosAtaquePorGrupo("Arco", ranger->Proto());
    // 9 bab, 2 des, 1 OP.
    EXPECT_EQ(da.bonus_ataque_final(), 9+2+1);
    EXPECT_EQ(da.dano(), "1d8+3");
    EXPECT_FLOAT_EQ(da.alcance_m(), 33);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 0.0f);
    ranger->AtualizaAcaoPorGrupo("Arco");
    EXPECT_EQ(ranger->CA(*ranger, Entidade::CA_NORMAL), 19) << ranger->Proto().dados_defesa().ca().DebugString();
  }
}

TEST(TesteModelo, TesteGuepardo) {
  auto modelo = g_tabelas.ModeloEntidade("Guepardo");
  auto guepardo = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
  EXPECT_TRUE(guepardo->TemTipoDnD(TIPO_ANIMAL));
  ASSERT_EQ(guepardo->NivelClasse("animal"), 3);
  ASSERT_EQ(guepardo->Proto().tipo_visao(), VISAO_BAIXA_LUMINOSIDADE);
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_FORCA, guepardo->Proto())), 16);
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_DESTREZA, guepardo->Proto())), 19);
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_CONSTITUICAO, guepardo->Proto())), 15);
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_INTELIGENCIA, guepardo->Proto())), 2);
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_SABEDORIA, guepardo->Proto())), 12);
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_CARISMA, guepardo->Proto())), 6);
  EXPECT_EQ(guepardo->CA(*guepardo, Entidade::CA_NORMAL), 15);
  EXPECT_EQ(ValorFinalPericia("esconderse", guepardo->Proto()), 6);
  EXPECT_EQ(ValorFinalPericia("ouvir", guepardo->Proto()), 4);
  EXPECT_EQ(ValorFinalPericia("furtividade", guepardo->Proto()), 6);
  EXPECT_EQ(ValorFinalPericia("observar", guepardo->Proto()), 4);
}

TEST(TesteModelo, TesteAbelhaGiganteCelestial) {
  auto modelo = g_tabelas.ModeloEntidade("Abelha Gigante Celestial");
  auto abelha = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
  EXPECT_TRUE(abelha->TemTipoDnD(TIPO_BESTA_MAGICA));
  EXPECT_TRUE(abelha->TemSubTipoDnD(SUBTIPO_PLANAR));
  ASSERT_EQ(abelha->NivelClasse("besta_magica"), 3);
  ASSERT_EQ(abelha->Proto().tipo_visao(), VISAO_ESCURO);
  ASSERT_FLOAT_EQ(abelha->Proto().alcance_visao_m(), 18.0f);
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_INTELIGENCIA, abelha->Proto())), 3);
}

TEST(TesteModelo, TesteLoboAbissal) {
  auto modelo = g_tabelas.ModeloEntidade("Lobo Abissal");
  auto lobo = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
  EXPECT_TRUE(lobo->TemTipoDnD(TIPO_BESTA_MAGICA));
  EXPECT_TRUE(lobo->TemSubTipoDnD(SUBTIPO_PLANAR));
  ASSERT_EQ(lobo->NivelClasse("besta_magica"), 2);
  ASSERT_EQ(lobo->Proto().tipo_visao(), VISAO_ESCURO_E_BAIXA_LUMINOSIDADE);
  ASSERT_FLOAT_EQ(lobo->Proto().alcance_visao_m(), 18.0f);
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_INTELIGENCIA, lobo->Proto())), 3);
}

TEST(TesteModelo, TesteLeaoCelestial) {
  auto modelo = g_tabelas.ModeloEntidade("Leão Celestial");
  auto lc = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
  ASSERT_EQ(lc->Proto().tipo_visao(), VISAO_ESCURO_E_BAIXA_LUMINOSIDADE);
  ASSERT_FLOAT_EQ(lc->Proto().alcance_visao_m(), 18.0f);
}

TEST(TesteModelo, TestePlebeu1) {
  auto modelo = g_tabelas.ModeloEntidade("Humano Plebeu 1");
  auto plebeu = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
  EXPECT_FALSE(plebeu->TemTipoDnD(TIPO_BESTA_MAGICA));
  EXPECT_FALSE(plebeu->TemSubTipoDnD(SUBTIPO_PLANAR));
  {
    // Corpo a corpo.
    const auto& da = DadosAtaquePorGrupo("clava", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 1);
    EXPECT_EQ(da.dano(), "1d6+1");
    EXPECT_FLOAT_EQ(da.alcance_m(), 1.5f);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 0.0f);
    EXPECT_EQ(da.incrementos(), 0);
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 10);
  }
  {
    // Distancia sem pericia.
    const auto& da = DadosAtaquePorGrupo("adaga", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), -4);
    EXPECT_EQ(da.dano(), "1d4+1");
    EXPECT_FLOAT_EQ(da.alcance_m(), 3.0f);
    EXPECT_EQ(da.incrementos(), 5);
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 10);
  }
  {
    // Distancia sem pericia com tiro longo.
    modelo.mutable_entidade()->mutable_info_talentos()->add_outros()->set_id("tiro_longo");
    auto plebeu = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
    const auto& da = DadosAtaquePorGrupo("adaga", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), -4);
    EXPECT_EQ(da.dano(), "1d4+1");
    EXPECT_FLOAT_EQ(da.alcance_m(), 6.0f);
    EXPECT_EQ(da.incrementos(), 5);
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 10);
  }
}

TEST(TesteModelo, TestePlebeu1Grande) {
  auto proto = g_tabelas.ModeloEntidade("Humano Plebeu 1").entidade();
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_AUMENTAR_PESSOA);
  evento->set_rodadas(1);
  auto plebeu = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    // Corpo a corpo.
    const auto& da = DadosAtaquePorGrupo("clava", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 1);
    EXPECT_EQ(da.dano(), "1d8+3");
    EXPECT_FLOAT_EQ(da.alcance_m(), 3.0f);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 0.0f);
    EXPECT_EQ(da.incrementos(), 0);
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 8);
  }
  {
    // Distancia sem pericia.
    const auto& da = DadosAtaquePorGrupo("adaga", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), -6);
    EXPECT_EQ(da.dano(), "1d6+2");
    EXPECT_FLOAT_EQ(da.alcance_m(), 3.0f);
    EXPECT_EQ(da.incrementos(), 5);
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 8);
  }
}

TEST(TesteModelo, TestePlebeu3) {
  auto proto = g_tabelas.ModeloEntidade("Humano Plebeu 3").entidade();
  auto plebeu = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    // Corpo a corpo.
    const auto& da = DadosAtaquePorGrupo("clava", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 4) << da.bonus_ataque().DebugString();
    EXPECT_EQ(da.dano(), "1d6+1");
    EXPECT_FLOAT_EQ(da.alcance_m(), 1.5f);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 0.0f);
    EXPECT_EQ(da.incrementos(), 0);
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 10);
  }
  {
    // Distancia.
    const auto& da = DadosAtaquePorGrupo("adaga", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), -3);
    EXPECT_EQ(da.dano(), "1d4+1");
    EXPECT_FLOAT_EQ(da.alcance_m(), 3.0f);
    EXPECT_EQ(da.incrementos(), 5);
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 10);
  }
}

TEST(TesteModelo, TestePlebeu4) {
  auto proto = g_tabelas.ModeloEntidade("Humano Plebeu 4").entidade();
  auto plebeu = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    // Corpo a corpo.
    const auto& da = DadosAtaquePorGrupo("lanca_longa", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 5);
    EXPECT_EQ(da.dano(), "1d8+3");
    EXPECT_FLOAT_EQ(da.alcance_m(), 3.0f);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 1.5f);
    EXPECT_EQ(da.incrementos(), 0);
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 10);
  }
  {
    // Distancia.
    const auto& da = DadosAtaquePorGrupo("adaga", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 2);
    EXPECT_EQ(da.dano(), "1d4+2");
    EXPECT_FLOAT_EQ(da.alcance_m(), 3.0f);
    EXPECT_EQ(da.incrementos(), 5);
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 10);
  }
}

TEST(TesteModelo, TestePlebeu4Grande) {
  auto proto = g_tabelas.ModeloEntidade("Humano Plebeu 4").entidade();
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_AUMENTAR_PESSOA);
  evento->set_rodadas(1);
  auto plebeu = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    // Corpo a corpo.
    const auto& da = DadosAtaquePorGrupo("lanca_longa", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 5);
    EXPECT_EQ(da.dano(), "2d6+4");
    EXPECT_FLOAT_EQ(da.alcance_m(), 6.0f);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 3.0f);
    EXPECT_EQ(da.incrementos(), 0);
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 8);
  }
  {
    // Distancia.
    const auto& da = DadosAtaquePorGrupo("adaga", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 0) << da.bonus_ataque().DebugString();
    EXPECT_EQ(da.dano(), "1d6+3");
    EXPECT_FLOAT_EQ(da.alcance_m(), 3.0f);
    EXPECT_EQ(da.incrementos(), 5);
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 8);
  }
}

TEST(TesteModelo, TestePlebeu1Invertido) {
  auto modelo = g_tabelas.ModeloEntidade("Humano Plebeu 1");
  // Corpo a corpo.
  DadosAtaquePorGrupoOuCria("clava", modelo.mutable_entidade())->set_tipo_ataque("Ataque a Distância");
  // Distancia sem pericia.
  DadosAtaquePorGrupoOuCria("adaga", modelo.mutable_entidade())->set_tipo_ataque("Ataque Corpo a Corpo");
  auto plebeu = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
  {
    // Corpo a corpo.
    const auto& da = DadosAtaquePorGrupo("clava", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 0);
    EXPECT_EQ(da.dano(), "1d6+1");
    EXPECT_FLOAT_EQ(da.alcance_m(), 3.0f);
    EXPECT_EQ(da.incrementos(), 5);
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 10);
  }
  {
    // Distancia sem pericia.
    const auto& da = DadosAtaquePorGrupo("adaga", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), -3);
    EXPECT_EQ(da.dano(), "1d4+1");
    EXPECT_FLOAT_EQ(da.alcance_m(), 1.5f);
    EXPECT_EQ(da.incrementos(), 0);
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 10);
  }
}

TEST(TesteModelo, TestePlebeia1) {
  auto modelo = g_tabelas.ModeloEntidade("Humana Plebeia 1");
  auto plebeu = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
  {
    // Corpo a corpo.
    const auto& da = DadosAtaquePorGrupo("adaga cac", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 0);
    EXPECT_EQ(da.dano(), "1d4");
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 11);
  }
  {
    // Distancia sem pericia.
    const auto& da = DadosAtaquePorGrupo("adaga distancia", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 1);
    EXPECT_EQ(da.dano(), "1d4");
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 11);
  }
}

TEST(TesteModelo, TestePlebeia3) {
  auto modelo = g_tabelas.ModeloEntidade("Humana Plebeia 3");
  auto plebeu = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
  {
    // Corpo a corpo.
    const auto& da = DadosAtaquePorGrupo("adaga cac", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 2);
    EXPECT_EQ(da.dano(), "1d4");
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 12);
  }
  {
    // Distancia sem pericia.
    const auto& da = DadosAtaquePorGrupo("adaga distancia", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 2);
    EXPECT_EQ(da.dano(), "1d4");
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 12);
  }
}

TEST(TesteModelo, TestePlebeia6) {
  auto modelo = g_tabelas.ModeloEntidade("Humana Plebeia 6");
  auto plebeu = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
  {
    // Corpo a corpo.
    const auto& da = DadosAtaquePorGrupo("adaga cac", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 4);
    EXPECT_EQ(da.dano(), "1d4");
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 17);
  }
  {
    // Distancia sem pericia.
    const auto& da = DadosAtaquePorGrupo("adaga distancia", plebeu->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 5);
    EXPECT_EQ(da.dano(), "1d4");
    EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 17);
  }
}

TEST(TesteModelo, TesteEsqueletoLobo) {
  auto modelo = g_tabelas.ModeloEntidade("Esqueleto (Lobo 2)");
  auto lobo = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
  const auto* da = lobo->DadoCorrente();
  ASSERT_NE(da, nullptr);
  EXPECT_EQ(da->bonus_ataque_final(), 2);
  EXPECT_EQ(da->dano(), "1d6+1");
}

TEST(TesteModelo, TesteHalflingDruida10) {
  auto modelo_druida = g_tabelas.ModeloEntidade("Halfling Druida 10");
  auto* ev = modelo_druida.mutable_entidade()->add_evento();
  ev->set_id_efeito(EFEITO_CONVOCAR_RELAMPAGOS);
  ev->set_rodadas(10);  // para ter limite de vezes = 1
  ev->set_id_unico(666);  // para o teste considerar a entidade no alcance do tiro certeiro.
  modelo_druida.mutable_entidade()->set_id(666);
  auto druida = NovaEntidadeParaTestes(modelo_druida.entidade(), g_tabelas);

  // Base 7 3 7
  // Atributos: for 10-2 (-1), des 14+2+2 (4), con 13+1 (2), int 12 (1), sab 15+1 (3), car 8 (-1)
  // Manto de resistencia +1;
  // Halfling +1.
  EXPECT_EQ(BonusTotal(druida->Proto().dados_defesa().salvacao_fortitude()), 7 + 2 + 1 + 1) << druida->Proto().dados_defesa().salvacao_fortitude().DebugString();
  EXPECT_EQ(BonusTotal(druida->Proto().dados_defesa().salvacao_reflexo()), 3 + 4 + 1 + 1) << druida->Proto().dados_defesa().salvacao_reflexo().DebugString();
  EXPECT_EQ(BonusTotal(druida->Proto().dados_defesa().salvacao_vontade()), 7 + 3 + 1 + 1) << druida->Proto().dados_defesa().salvacao_vontade().DebugString();
  EXPECT_EQ(SalvacaoFeitico(g_tabelas.Feitico("aterrorizar"), druida->Proto(), EntidadeProto::default_instance(), TS_VONTADE), 7 + 3 + 1 + 1 + 2) << druida->Proto().dados_defesa().salvacao_vontade().DebugString();

  {
    const auto* da = druida->DadoAtaque("relampago", 0);
    ASSERT_NE(da, nullptr);
    // Nao pode aplicar modificador de tiro certeiro.
    EXPECT_EQ(std::get<0>(StringDanoParaAcao(*da, druida->Proto(), druida->Proto())), "3d6");
  }
  {
    // Destreza +2.
    // +7 BAB, destreza +4, tamanho +1, +1 OP, +1 halfling.
    const auto& da = DadosAtaquePorGrupo("funda", druida->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 14) << da.bonus_ataque().DebugString();
    // Forca foi pra 8 por ser halfling.
    EXPECT_EQ(da.dano(), "1d3-1");
  }
}

TEST(TesteModelo, CamposResetadosNaoSetados) {
  for (const auto& modelo : g_tabelas.TodosModelosEntidades().modelo()) {
    const auto& e = modelo.entidade();
    for (const auto& da : e.dados_ataque()) {
      ASSERT_FALSE(da.has_ataque_agarrar()) << " modelo: " << modelo.id();
      ASSERT_FALSE(da.has_ataque_toque()) << " modelo: " << modelo.id();
      ASSERT_FALSE(da.has_ataque_distancia()) << " modelo: " << modelo.id();
      ASSERT_FALSE(da.has_ataque_arremesso()) << " modelo: " << modelo.id();
      ASSERT_FALSE(da.has_ataque_corpo_a_corpo()) << " modelo: " << modelo.id();
      ASSERT_FALSE(da.has_nao_letal()) << " modelo: " << modelo.id();
    }
  }
}

TEST(TesteModelo, TodasAcoesTemTipo) {
  for (const auto& modelo : g_tabelas.TodosModelosEntidades().modelo()) {
    LOG(INFO) << "modelo: " << modelo.id();
    const auto& proto = modelo.entidade();
    auto e = NovaEntidadeParaTestes(proto, g_tabelas);
    for (const auto& da : e->Proto().dados_ataque()) {
      EXPECT_TRUE(da.acao().has_tipo())
        << " modelo: " << modelo.id() << ", da grupo: " << da.grupo() << ", rotulo: " << da.rotulo();
    }
  }
}

TEST(TesteModelo, TesteElementalFogo) {
  const auto& modelo = g_tabelas.ModeloEntidade("Elemental do Fogo (Pequeno)");
  EntidadeProto proto = modelo.entidade();
  proto.set_gerar_agarrar(false);
  RecomputaDependencias(g_tabelas, &proto);
  const auto& da = DadosAtaquePorGrupo("pancada", proto);
  EXPECT_EQ(std::get<0>(StringDanoParaAcao(da, proto, proto)), "1d4");
  EXPECT_EQ(std::get<1>(StringDanoParaAcao(da, proto, proto)), "1d4");
  auto vopt = VulnerabilidadeParaElemento(-5, proto, DESC_FRIO);
  ASSERT_TRUE(vopt.has_value());
  auto [delta, texto] = *vopt;
  EXPECT_EQ(delta, -2) << texto;
  vopt = VulnerabilidadeParaElemento(-5, proto, DESC_FOGO);
  ASSERT_FALSE(vopt.has_value());
}

TEST(TesteModelo, TesteElementais) {
  for (const auto& modelo : g_tabelas.TodosModelosEntidades().modelo()) {
    if (modelo.id().find("Elemental ") != 0) continue;
    EntidadeProto proto = modelo.entidade();
    proto.set_gerar_agarrar(false);
    std::unique_ptr<Entidade> el(NovaEntidadeParaTestes(proto, g_tabelas));
    EXPECT_TRUE(el->ImuneEfeito(EFEITO_SONO)) << modelo.id();
    EXPECT_TRUE(el->ImuneEfeito(EFEITO_ATORDOADO)) << modelo.id();
    EXPECT_EQ(el->Proto().tipo_visao(), VISAO_ESCURO);
    EXPECT_TRUE(el->ImuneCritico());
  }
}

TEST(TesteModelo, TesteLeaoAtroz) {
  const auto& modelo = g_tabelas.ModeloEntidade("Leão Atroz");
  EntidadeProto proto = modelo.entidade();
  proto.set_gerar_agarrar(false);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(proto.dados_ataque().size(), 8);
  // Normal;
  EXPECT_EQ(proto.dados_ataque(0).dano(), "1d6+7") << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(1).dano(), "1d6+7") << proto.dados_ataque(1).DebugString();
  EXPECT_EQ(proto.dados_ataque(2).dano(), "1d8+3") << proto.dados_ataque(2).DebugString();
  // Bote.
  EXPECT_EQ(proto.dados_ataque(3).dano(), "1d6+7") << proto.dados_ataque(3).DebugString();
  EXPECT_EQ(proto.dados_ataque(4).dano(), "1d6+7") << proto.dados_ataque(4).DebugString();
  EXPECT_EQ(proto.dados_ataque(5).dano(), "1d6+3") << proto.dados_ataque(5).DebugString();
  EXPECT_EQ(proto.dados_ataque(6).dano(), "1d6+3") << proto.dados_ataque(6).DebugString();
  EXPECT_EQ(proto.dados_ataque(7).dano(), "1d8+3") << proto.dados_ataque(7).DebugString();

  EXPECT_EQ(ValorFinalPericia("esconderse", proto), 2);
  EXPECT_EQ(ValorFinalPericia("ouvir", proto), 7);
  // Livro diz que é 5, mas é impossivel pela descricao.
  EXPECT_EQ(ValorFinalPericia("furtividade", proto), 9);
  EXPECT_EQ(ValorFinalPericia("observar", proto), 7);
}

TEST(TesteModelo, TesteUrsoAtroz) {
  const auto& modelo = g_tabelas.ModeloEntidade("Urso Atroz");
  EntidadeProto proto = modelo.entidade();
  proto.set_gerar_agarrar(false);
  std::unique_ptr<Entidade> urso(NovaEntidadeParaTestes(proto, g_tabelas));
  // Normal;
  EXPECT_EQ(DadosAtaquePorGrupo("ataque", urso->Proto(), 0).dano(), "2d4+10");
  EXPECT_EQ(DadosAtaquePorGrupo("ataque", urso->Proto(), 1).dano(), "2d4+10");
  EXPECT_EQ(DadosAtaquePorGrupo("ataque", urso->Proto(), 2).dano(), "2d8+5");
  EXPECT_EQ(urso->CA(*urso, Entidade::CA_NORMAL), 17);
  EXPECT_EQ(ValorFinalPericia("ouvir", urso->Proto()), 10);
  EXPECT_EQ(ValorFinalPericia("observar", urso->Proto()), 10);
  // No livro ta 13, mas nao eh possivel.
  EXPECT_EQ(ValorFinalPericia("natacao", urso->Proto()), 11);
}

TEST(TesteModelo, TesteTigreAtroz) {
  const auto& modelo = g_tabelas.ModeloEntidade("Tigre Atroz");
  EntidadeProto proto = modelo.entidade();
  proto.set_gerar_agarrar(false);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_EQ(proto.dados_ataque().size(), 8);
  // Normal;
  EXPECT_EQ(proto.dados_ataque(0).dano(), "2d4+8") << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(1).dano(), "2d4+8") << proto.dados_ataque(1).DebugString();
  EXPECT_EQ(proto.dados_ataque(2).dano(), "2d6+4") << proto.dados_ataque(2).DebugString();
  // Bote.
  EXPECT_EQ(proto.dados_ataque(3).dano(), "2d4+8") << proto.dados_ataque(3).DebugString();
  EXPECT_EQ(proto.dados_ataque(4).dano(), "2d4+8") << proto.dados_ataque(4).DebugString();
  EXPECT_EQ(proto.dados_ataque(5).dano(), "2d4+4") << proto.dados_ataque(5).DebugString();
  EXPECT_EQ(proto.dados_ataque(6).dano(), "2d4+4") << proto.dados_ataque(6).DebugString();
  EXPECT_EQ(proto.dados_ataque(7).dano(), "2d6+4") << proto.dados_ataque(7).DebugString();
}

TEST(TesteModelo, TesteVrock) {
  const auto& modelo = g_tabelas.ModeloEntidade("Demônio Vrock");
  EntidadeProto proto = modelo.entidade();
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_GT(proto.dados_ataque().size(), 5);
  EXPECT_EQ(NivelConjurador(TipoAtaqueParaClasse(g_tabelas, proto.dados_ataque(5).tipo_ataque()), proto), 12);
}

TEST(TesteModelo, TesteCentopeiaEnormeAbissal) {
  const auto& modelo = g_tabelas.ModeloEntidade("Centopéia Enorme Abissal");
  EntidadeProto proto = modelo.entidade();
  std::unique_ptr<Entidade> entidade(NovaEntidadeParaTestes(proto, g_tabelas));
  const auto& da = DadosAtaquePorGrupo("destruir_bem", entidade->Proto());
  EXPECT_EQ(da.grupo(), "destruir_bem");
  EXPECT_EQ(da.rotulo(), "mordida com veneno");
  EXPECT_EQ(da.taxa_refrescamento(), "14400");
  EXPECT_EQ(da.dano(), "2d6+4");

  const auto& modelo_alvo = g_tabelas.ModeloEntidade("Cachorro Celestial");
  std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(modelo_alvo.entidade(), g_tabelas));
  EXPECT_EQ(std::get<0>(StringDanoParaAcao(da, entidade->Proto(), alvo->Proto())), "2d6+4+6");
}

TEST(TesteModelo, TesteModeloVulto) {
  EntidadeProto proto;
  AtribuiBaseAtributo(11, TA_SABEDORIA, &proto);
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("druida");
    ic->set_nivel(1);
    {
      auto* da = proto.add_dados_ataque();
      da->set_tipo_ataque("Ataque Corpo a Corpo");
      da->set_id_arma("clava");
    }
  }
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_GE(proto.dados_ataque().size(), 1);
  EXPECT_EQ(proto.dados_ataque(0).bonus_ataque_final(), 0);
  EXPECT_EQ(proto.dados_defesa().resistencia_magia(), 0);

  auto* vulto = proto.add_modelos();
  vulto->set_id_efeito(EFEITO_MODELO_VULTO);
  vulto->set_ativo(true);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_GE(proto.dados_ataque().size(), 1);
  EXPECT_EQ(proto.dados_ataque(0).bonus_ataque_final(), 2);
  EXPECT_EQ(proto.dados_defesa().resistencia_magia(), 12);

  vulto->set_ativo(false);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_GE(proto.dados_ataque().size(), 1);
  EXPECT_EQ(proto.dados_ataque(0).bonus_ataque_final(), 0);
  EXPECT_EQ(proto.dados_defesa().resistencia_magia(), 0);
}

TEST(TesteComposicaoEntidade, TesteComposicaoEntidade) {
  const auto& modelo = g_tabelas.ModeloEntidade("Gigante das Pedras, Ancião");
  EntidadeProto proto = modelo.entidade();
  RecomputaDependencias(g_tabelas, &proto);

  ASSERT_FALSE(proto.info_classes().empty());
  EXPECT_EQ(proto.info_classes(0).nivel_conjurador(), 10);
  EXPECT_EQ(proto.info_classes(0).atributo_conjuracao(), TA_CARISMA);
}

TEST(TesteComposicaoEntidade, TesteHumanaAristocrata6) {
  const auto& modelo_ha6 = g_tabelas.ModeloEntidade("Humana Aristocrata 6");
  EntidadeProto proto = modelo_ha6.entidade();
  RecomputaDependencias(g_tabelas, &proto);

  EXPECT_EQ(BonusTotal(BonusAtributo(TA_INTELIGENCIA, proto)), 11);
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_FORCA, proto)), 8);
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_DESTREZA, proto)), 12);
  EXPECT_TRUE(PossuiTalento("negociador", proto));
  EXPECT_TRUE(PossuiTalento("persuasivo", proto));
  auto aris = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    // Corpo a corpo.
    const auto& da = DadosAtaquePorGrupo("fogo_alquimico", aris->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 5);
    EXPECT_EQ(da.dano(), "1d6");
    EXPECT_FLOAT_EQ(da.alcance_m(), 3.0f);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 0.0f);
    EXPECT_EQ(da.incrementos(), 5);
    EXPECT_TRUE(da.ataque_toque());
    EXPECT_EQ(da.municao(), 1U);
    EXPECT_EQ(aris->CA(*aris, Entidade::CA_NORMAL), 16);
  }

  {
    // Corpo a corpo.
    const auto& da = DadosAtaquePorGrupo("sabre", aris->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 6);
    EXPECT_EQ(da.dano(), "1d6");
    EXPECT_FLOAT_EQ(da.alcance_m(), 1.5f);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 0.0f);
    EXPECT_EQ(da.incrementos(), 0);
    EXPECT_EQ(aris->CA(*aris, Entidade::CA_NORMAL), 16);
  }
  {
    // Besta.
    const auto& da = DadosAtaquePorGrupo("besta", aris->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 4) << da.bonus_ataque().DebugString();
    EXPECT_EQ(da.dano(), "1d8");
    EXPECT_FLOAT_EQ(da.alcance_m(), 24.0f);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 0.0f);
    EXPECT_EQ(da.incrementos(), 10);
    EXPECT_EQ(aris->CA(*aris, Entidade::CA_NORMAL), 16);
  }
  LOG(INFO) << "--------";
  {
    // Besta com tiro longo.
    proto.mutable_info_talentos()->add_outros()->set_id("tiro_longo");
    auto aris = NovaEntidadeParaTestes(proto, g_tabelas);
    const auto& da = DadosAtaquePorGrupo("besta", aris->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 4) << da.bonus_ataque().DebugString();
    EXPECT_EQ(da.dano(), "1d8");
    EXPECT_FLOAT_EQ(da.alcance_m(), 36.0f) << da.id_arma();
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 0.0f);
    EXPECT_EQ(da.incrementos(), 10);
    EXPECT_EQ(aris->CA(*aris, Entidade::CA_NORMAL), 16);
  }
}

TEST(TesteComposicaoEntidade, TesteHumanoAristocrata6) {
  const auto& modelo_ha6 = g_tabelas.ModeloEntidade("Humano Aristocrata 6");
  EntidadeProto proto = modelo_ha6.entidade();
  RecomputaDependencias(g_tabelas, &proto);

  EXPECT_EQ(BonusTotal(BonusAtributo(TA_INTELIGENCIA, proto)), 11);
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_FORCA, proto)), 12);
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_DESTREZA, proto)), 8);
  EXPECT_TRUE(PossuiTalento("negociador", proto));
  EXPECT_TRUE(PossuiTalento("persuasivo", proto));
  auto aris = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    // Corpo a corpo.
    const auto& da = DadosAtaquePorGrupo("gas_alquimico_sono", aris->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 3);
    EXPECT_EQ(da.dano(), "");
    EXPECT_FLOAT_EQ(da.alcance_m(), 3.0f);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 0.0f);
    EXPECT_EQ(da.incrementos(), 5);
    EXPECT_EQ(da.municao(), 1U);
    EXPECT_EQ(aris->CA(*aris, Entidade::CA_NORMAL), 16);
  }
  {
    // Corpo a corpo.
    const auto& da = DadosAtaquePorGrupo("espada_longa", aris->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 7) << da.bonus_ataque().DebugString();
    EXPECT_EQ(da.dano(), "1d8+2");
    EXPECT_FLOAT_EQ(da.alcance_m(), 1.5f);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 0.0f);
    EXPECT_EQ(da.incrementos(), 0);
    EXPECT_EQ(aris->CA(*aris, Entidade::CA_NORMAL), 16);
  }
  {
    // Besta.
    const auto& da = DadosAtaquePorGrupo("besta", aris->Proto());
    EXPECT_EQ(da.bonus_ataque_final(), 2) << da.bonus_ataque().DebugString();
    EXPECT_EQ(da.dano(), "1d8");
    EXPECT_FLOAT_EQ(da.alcance_m(), 24.0f);
    EXPECT_FLOAT_EQ(da.alcance_minimo_m(), 0.0f);
    EXPECT_EQ(da.incrementos(), 10);
    EXPECT_EQ(aris->CA(*aris, Entidade::CA_NORMAL), 16);
  }
}

TEST(TesteComposicaoEntidade, TesteBardoVulto5) {
  const auto& modelo_vb5 = g_tabelas.ModeloEntidade("Vulto Bardo 5");
  EntidadeProto proto = modelo_vb5.entidade();
  RecomputaDependencias(g_tabelas, &proto);
  std::unique_ptr<Entidade> entidade(NovaEntidadeParaTestes(proto, g_tabelas));

  ASSERT_EQ(proto.dados_ataque().size(), 3 + 3 + 1);
  // Humano Bardo 5.
  EXPECT_EQ(proto.dados_ataque(2).rotulo(), "inspirar_coragem aliados (5/dia)");
  EXPECT_EQ(proto.dados_ataque(2).limite_vezes(), 5);

  // Vulto Base.
  EXPECT_EQ(proto.dados_ataque(3).rotulo(), "reflexos 3/dia");
  EXPECT_TRUE(proto.dados_ataque(3).acao().classe_conjuracao().empty());
  EXPECT_EQ(proto.dados_ataque(3).limite_vezes(), 3);
  EXPECT_EQ(proto.raca(), "humano");

  // Vulto Bardo 5.
  ASSERT_FALSE(proto.modelos().empty());
  EXPECT_EQ(proto.modelos(0).id_efeito(), EFEITO_MODELO_VULTO);

  // Aplica reflexos pra ver se vai fazer certinho.
  const auto& acao = proto.dados_ataque(3).acao();
  ASSERT_FALSE(acao.efeitos_adicionais().empty());
  std::vector<int> ids_unicos = IdsUnicosEntidade(*entidade);
  ntf::Notificacao n;
  // d4 de reflexos.
  g_dados_teste.push(1);
  PreencheNotificacaoEventoEfeitoAdicional(
      proto.id(), std::nullopt,
      NivelConjuradorParaAcao(acao, g_tabelas.Feitico(proto.dados_ataque(3).id_arma()), *entidade),
      *entidade, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
  const int proximo = proto.evento().size();
  entidade->AtualizaParcial(n.entidade());
  proto = entidade->Proto();
  ASSERT_GE(proto.evento().size(), proximo);
  const auto& evento = proto.evento(proximo);
  EXPECT_EQ(evento.id_efeito(), EFEITO_REFLEXOS);
  ASSERT_FALSE(evento.complementos().empty());
  // 1 do dado, +1 de nivel 5.
  EXPECT_EQ(evento.complementos(0), 2);
  EXPECT_EQ(evento.rodadas(), 50);

  EntidadeProto alvo_proto;
  alvo_proto.mutable_luz()->set_raio_m(12.0f);
  std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(alvo_proto, g_tabelas));
  ASSERT_TRUE(alvo->TemLuz());
  {
    auto acao = proto.dados_ataque(4).acao();
    ntf::Notificacao n;
    PreencheNotificacaoReducaoLuzComConsequencia(
        NivelConjuradorParaAcao(acao, g_tabelas.Feitico(proto.dados_ataque(4).id_arma()), *entidade), *alvo, &acao, &n, nullptr);
    alvo->AtualizaParcial(n.entidade());
    EXPECT_EQ(alvo->Proto().luz().raio_m(), 6.0f);
    EXPECT_EQ(acao.consequencia(), TC_REDUZ_LUZ_ALVO);
    EXPECT_EQ(acao.reducao_luz(), 0.5f);
  }
}

TEST(TesteComposicaoEntidade, TesteClerigo5Adepto1) {
  const auto& modelo_vb5 = g_tabelas.ModeloEntidade("Vulto Clérigo de Shar/Adepto das Sombras 5/1");
  EntidadeProto proto = modelo_vb5.entidade();
  EXPECT_EQ(proto.raca(), "humano");
  RecomputaDependencias(g_tabelas, &proto);
  std::unique_ptr<Entidade> entidade(NovaEntidadeParaTestes(proto, g_tabelas));

  EntidadeProto alvo_proto;
  alvo_proto.mutable_luz()->set_raio_m(12.0f);
  std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(alvo_proto, g_tabelas));
  ASSERT_TRUE(alvo->TemLuz());
  {
    ASSERT_GT(proto.dados_ataque().size(), 2);
    auto acao = proto.dados_ataque(2).acao();
    ntf::Notificacao n;
    PreencheNotificacaoReducaoLuzComConsequencia(
        NivelConjuradorParaAcao(acao, g_tabelas.Feitico(proto.dados_ataque(2).id_arma()), *entidade), *alvo, &acao, &n, nullptr);
    alvo->AtualizaParcial(n.entidade());
    EXPECT_FLOAT_EQ(alvo->Proto().luz().raio_m(), 12 * 0.4);
    EXPECT_EQ(acao.consequencia(), TC_REDUZ_LUZ_ALVO);
    EXPECT_FLOAT_EQ(acao.reducao_luz(), 0.4f);
  }
  {
    // Ataque de morte, penultimo antes do agarrar.
    const auto& da = DadosAtaquePorGrupo("dominio morte", proto);
    EXPECT_TRUE(da.ataque_toque()) << da.DebugString();
    EXPECT_FALSE(da.acao().efeitos_adicionais().empty());
    EXPECT_EQ(da.acao().efeitos_adicionais(0).efeito(), EFEITO_MORTE);
  }
}

TEST(TesteComposicaoEntidade, TesteMonge5) {
  const auto& modelo_m5 = g_tabelas.ModeloEntidade("Humano Monge 5");
  EntidadeProto proto = modelo_m5.entidade();
  RecomputaDependencias(g_tabelas, &proto);

  // 10 + 3 sab + 1 des + 1 de monge.
  EXPECT_EQ(BonusTotal(proto.dados_defesa().ca()), 15) << proto.dados_defesa().DebugString();
  ASSERT_GE(proto.dados_ataque().size(), 8);
  // Desarmado atordoante.
  EXPECT_EQ(proto.dados_ataque(0).bonus_ataque_final(), 5) << "id_arma: " <<  proto.dados_ataque(0).id_arma() << ", bonus ataque: " << proto.dados_ataque(0).bonus_ataque().DebugString();
  EXPECT_EQ(proto.dados_ataque(0).dano(), "1d8+2");
  EXPECT_EQ(proto.dados_ataque(0).dificuldade_salvacao(), 15);
  // Kama +1: derrubar, +3 ataque, +2 força, +1 arma, +1 foco em arma, -1 rajada. Dano: +2 de força, +1 arma.
  EXPECT_EQ(proto.dados_ataque(1).bonus_ataque_final(), 6) << "id_arma: " <<  proto.dados_ataque(1).id_arma() << ", bonus ataque: " << proto.dados_ataque(1).bonus_ataque().DebugString();
  EXPECT_TRUE(proto.dados_ataque(1).dano().empty());
  EXPECT_TRUE(proto.dados_ataque(1).ataque_derrubar());
  // Desarmado: +3 ataque, +2 força, -1 rajada. Dano: +2 de força.
  EXPECT_EQ(proto.dados_ataque(3).bonus_ataque_final(), 4) << proto.dados_ataque(2).bonus_ataque().DebugString();
  EXPECT_EQ(proto.dados_ataque(3).dano(), "1d8+2") << proto.dados_ataque(2).bonus_dano().DebugString();
  // Kama +1 sem rajada: derrubar, +3 ataque, +2 força, +1 foco em arma. Dano: +2 de força, +1 arma.
  EXPECT_EQ(proto.dados_ataque(5).bonus_ataque_final(), 7);
  EXPECT_TRUE(proto.dados_ataque(5).dano().empty());
  EXPECT_TRUE(proto.dados_ataque(5).ataque_derrubar());
  // Desarmado sem rajada: +3 ataque, +2 força. Dano +2 de força.
  EXPECT_EQ(proto.dados_ataque(6).bonus_ataque_final(), 5);
  EXPECT_EQ(proto.dados_ataque(6).dano(), "1d8+2");
  // Funda OP: +3 ataque, +1 destreza, +1 OP. Dano: +2 de força.
  EXPECT_EQ(proto.dados_ataque(7).bonus_ataque_final(), 5);
  EXPECT_EQ(proto.dados_ataque(7).dano(), "1d4+2");
}

TEST(TesteComposicaoEntidade, TesteMonge5Grande) {
  const auto& modelo_m5 = g_tabelas.ModeloEntidade("Humano Monge 5");
  EntidadeProto proto = modelo_m5.entidade();
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_AUMENTAR_PESSOA);
  evento->set_rodadas(1);
  RecomputaDependencias(g_tabelas, &proto);

  // 10 + 3 sab + 0 des + 1 de monge - 1 de tamanho.
  EXPECT_EQ(BonusTotal(proto.dados_defesa().ca()), 13) << proto.dados_defesa().DebugString();
  ASSERT_GE(proto.dados_ataque().size(), 8);
  // Desarmado atordoante.
  EXPECT_EQ(proto.dados_ataque(0).bonus_ataque_final(), 5) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(0).dano(), "2d6+3");
  EXPECT_EQ(proto.dados_ataque(0).dificuldade_salvacao(), 15);
  // Kama +1: +3 ataque, +3 força, +1 arma, +1 foco em arma, -1 rajada, -1 tamanho. Dano: +3 de força, +1 arma.
  EXPECT_EQ(proto.dados_ataque(1).bonus_ataque_final(), 6) << proto.dados_ataque(0).DebugString();
  EXPECT_TRUE(proto.dados_ataque(1).dano().empty());
  EXPECT_TRUE(proto.dados_ataque(1).ataque_derrubar());
  // Desarmado: +3 ataque, +3 força, -1 rajada, -1 tamanho. Dano: +2 de força.
  EXPECT_EQ(proto.dados_ataque(3).bonus_ataque_final(), 4) << proto.dados_ataque(2).DebugString();
  EXPECT_EQ(proto.dados_ataque(3).dano(), "2d6+3") << proto.dados_ataque(2).DebugString();
  // Kama +1 sem rajada: derrubar, +3 ataque, +2 força, +1 foco em arma. Dano: +2 de força, +1 arma.
  EXPECT_EQ(proto.dados_ataque(5).bonus_ataque_final(), 7);
  EXPECT_TRUE(proto.dados_ataque(5).dano().empty());
  EXPECT_TRUE(proto.dados_ataque(5).ataque_derrubar());
  // Desarmado sem rajada: +3 ataque, +3 força, -1 tamanho. Dano +3 de força.
  EXPECT_EQ(proto.dados_ataque(6).bonus_ataque_final(), 5);
  EXPECT_EQ(proto.dados_ataque(6).dano(), "2d6+3");
  // Funda OP: +3 ataque, +0 destreza, +1 OP, -1 tamanho. Dano: +3 de força.
  EXPECT_EQ(proto.dados_ataque(7).bonus_ataque_final(), 3);
  EXPECT_EQ(proto.dados_ataque(7).dano(), "1d6+3");
}

TEST(TesteConsequenciaPontosVida, Normal) {
  EntidadeProto proto;
  proto.set_max_pontos_vida(10);
  RecomputaDependencias(g_tabelas, &proto);

  ntf::Notificacao n;
  PreencheNotificacaoConsequenciaAlteracaoPontosVida(
      /*pontos_vida*/ 4, /*dano_nao_letal*/ 3, proto, &n);
  std::unique_ptr<Entidade> entidade(NovaEntidadeParaTestes(proto, g_tabelas));
  entidade->AtualizaParcial(n.entidade());
  proto = entidade->Proto();
  EXPECT_FALSE(proto.morta());
  EXPECT_FALSE(proto.inconsciente());
  EXPECT_FALSE(proto.incapacitada());
  EXPECT_FALSE(proto.caida());

  entidade->AtualizaParcial(n.entidade_antes());
  proto = entidade->Proto();
  EXPECT_FALSE(proto.morta());
  EXPECT_FALSE(proto.inconsciente());
  EXPECT_FALSE(proto.incapacitada());
  EXPECT_FALSE(proto.caida());
}

TEST(TesteConsequenciaPontosVida, Nocauteado) {
  EntidadeProto proto;
  proto.set_max_pontos_vida(10);
  RecomputaDependencias(g_tabelas, &proto);

  ntf::Notificacao n;
  PreencheNotificacaoConsequenciaAlteracaoPontosVida(
      /*pontos_vida*/ 4, /*dano_nao_letal*/ 4, proto, &n);
  std::unique_ptr<Entidade> entidade(NovaEntidadeParaTestes(proto, g_tabelas));
  entidade->AtualizaParcial(n.entidade());
  proto = entidade->Proto();
  EXPECT_FALSE(proto.morta());
  EXPECT_FALSE(proto.inconsciente());
  EXPECT_TRUE(proto.nocauteada());
  EXPECT_FALSE(proto.incapacitada());
  EXPECT_FALSE(proto.caida()) << proto.DebugString();

  entidade->AtualizaParcial(n.entidade_antes());
  proto = entidade->Proto();
  EXPECT_FALSE(proto.morta());
  EXPECT_FALSE(proto.inconsciente());
  EXPECT_FALSE(proto.nocauteada());
  EXPECT_FALSE(proto.incapacitada());
  EXPECT_FALSE(proto.caida());
}

TEST(TesteConsequenciaPontosVida, Incapacitado) {
  EntidadeProto proto;
  proto.set_max_pontos_vida(10);
  RecomputaDependencias(g_tabelas, &proto);

  ntf::Notificacao n;
  PreencheNotificacaoConsequenciaAlteracaoPontosVida(
      /*pontos_vida*/ 0, /*dano_nao_letal*/ 0, proto, &n);
  std::unique_ptr<Entidade> entidade(NovaEntidadeParaTestes(proto, g_tabelas));
  entidade->AtualizaParcial(n.entidade());
  proto = entidade->Proto();
  EXPECT_FALSE(proto.morta());
  EXPECT_FALSE(proto.inconsciente());
  EXPECT_FALSE(proto.nocauteada());
  EXPECT_TRUE(proto.incapacitada());
  EXPECT_FALSE(proto.caida()) << proto.DebugString();

  entidade->AtualizaParcial(n.entidade_antes());
  proto = entidade->Proto();
  EXPECT_FALSE(proto.morta());
  EXPECT_FALSE(proto.inconsciente());
  EXPECT_FALSE(proto.nocauteada());
  EXPECT_FALSE(proto.incapacitada());
  EXPECT_FALSE(proto.caida());
}

TEST(TesteConsequenciaPontosVida, Inconsciente) {
  EntidadeProto proto;
  proto.set_max_pontos_vida(10);
  AtribuiBaseAtributo(12, TA_CONSTITUICAO, &proto);
  RecomputaDependencias(g_tabelas, &proto);

  ntf::Notificacao n;
  PreencheNotificacaoConsequenciaAlteracaoPontosVida(
      /*pontos_vida*/ 5, /*dano_nao_letal*/ 6, proto, &n);
  std::unique_ptr<Entidade> entidade(NovaEntidadeParaTestes(proto, g_tabelas));
  entidade->AtualizaParcial(n.entidade());
  proto = entidade->Proto();
  EXPECT_FALSE(proto.morta());
  EXPECT_TRUE(proto.inconsciente());
  EXPECT_TRUE(proto.incapacitada());
  EXPECT_TRUE(proto.caida());

  entidade->AtualizaParcial(n.entidade_antes());
  proto = entidade->Proto();
  EXPECT_FALSE(proto.morta());
  EXPECT_FALSE(proto.inconsciente());
  EXPECT_FALSE(proto.incapacitada());
  EXPECT_FALSE(proto.caida());
}

TEST(TesteConsequenciaPontosVida, InconscienteParaIncapacitado) {
  EntidadeProto proto;
  proto.set_max_pontos_vida(10);
  proto.set_pontos_vida(-1);
  proto.set_inconsciente(true);
  proto.set_caida(true);
  proto.set_incapacitada(true);
  RecomputaDependencias(g_tabelas, &proto);

  ntf::Notificacao n;
  PreencheNotificacaoConsequenciaAlteracaoPontosVida(
      /*pontos_vida=*/ 0, /*dano_nao_letal=*/ 0, proto, &n);
  std::unique_ptr<Entidade> entidade(NovaEntidadeParaTestes(proto, g_tabelas));
  entidade->AtualizaParcial(n.entidade());
  proto = entidade->Proto();
  EXPECT_FALSE(proto.morta());
  EXPECT_FALSE(proto.inconsciente());
  EXPECT_FALSE(proto.nocauteada());
  EXPECT_TRUE(proto.incapacitada());
  EXPECT_TRUE(proto.caida());
}

TEST(TesteConsequenciaPontosVida, InconscienteParaNormal) {
  EntidadeProto proto;
  proto.set_max_pontos_vida(10);
  proto.set_pontos_vida(-1);
  proto.set_inconsciente(true);
  proto.set_caida(true);
  proto.set_incapacitada(true);
  RecomputaDependencias(g_tabelas, &proto);

  ntf::Notificacao n;
  PreencheNotificacaoConsequenciaAlteracaoPontosVida(
      /*pontos_vida=*/2, /*dano_nao_letal=*/1, proto, &n);
  std::unique_ptr<Entidade> entidade(NovaEntidadeParaTestes(proto, g_tabelas));
  entidade->AtualizaParcial(n.entidade());
  proto = entidade->Proto();
  EXPECT_FALSE(proto.morta());
  EXPECT_FALSE(proto.inconsciente());
  EXPECT_FALSE(proto.incapacitada());
  EXPECT_FALSE(proto.nocauteada());
  EXPECT_TRUE(proto.caida());
}

TEST(TesteConsequenciaPontosVida, InconscienteDuroDeMatar) {
  EntidadeProto proto;
  proto.set_inconsciente(true);
  proto.set_caida(true);
  proto.set_max_pontos_vida(10);
  {
    auto* talento = proto.mutable_info_talentos()->add_gerais();
    talento->set_id("duro_de_matar");
  }
  AtribuiBaseAtributo(12, TA_CONSTITUICAO, &proto);
  RecomputaDependencias(g_tabelas, &proto);

  ntf::Notificacao n;
  PreencheNotificacaoConsequenciaAlteracaoPontosVida(
      /*pontos_vida*/5, /*dano_nao_letal*/6, proto, &n);
  std::unique_ptr<Entidade> entidade(NovaEntidadeParaTestes(proto, g_tabelas));
  entidade->AtualizaParcial(n.entidade());
  proto = entidade->Proto();
  EXPECT_FALSE(proto.morta());
  EXPECT_FALSE(proto.inconsciente());
  EXPECT_TRUE(proto.incapacitada());
  EXPECT_TRUE(proto.caida());

  entidade->AtualizaParcial(n.entidade_antes());
  proto = entidade->Proto();
  EXPECT_FALSE(proto.morta());
  EXPECT_TRUE(proto.inconsciente());
  EXPECT_FALSE(proto.incapacitada());
  EXPECT_FALSE(proto.nocauteada());
  EXPECT_TRUE(proto.caida());
}

TEST(TesteConsequenciaPontosVida, Morta) {
  EntidadeProto proto;
  proto.set_max_pontos_vida(10);
  proto.set_voadora(true);
  AtribuiBaseAtributo(12, TA_CONSTITUICAO, &proto);
  RecomputaDependencias(g_tabelas, &proto);

  ntf::Notificacao n;
  PreencheNotificacaoConsequenciaAlteracaoPontosVida(
      /*pontos_vida*/ -12, /*dano_nao_letal*/ 0, proto, &n);
  std::unique_ptr<Entidade> entidade(NovaEntidadeParaTestes(proto, g_tabelas));
  entidade->AtualizaParcial(n.entidade());
  proto = entidade->Proto();
  EXPECT_TRUE(proto.morta());
  EXPECT_TRUE(proto.inconsciente());
  EXPECT_TRUE(proto.incapacitada());
  EXPECT_TRUE(proto.caida());
  EXPECT_FALSE(proto.voadora());
}

TEST(TesteConsequenciaPontosVida, PontosVidaTemporarios) {
  EntidadeProto proto;
  proto.set_max_pontos_vida(10);
  AtribuiBonus(1, TB_SEM_NOME, "temp", proto.mutable_pontos_vida_temporarios_por_fonte());
  RecomputaDependencias(g_tabelas, &proto);
  std::unique_ptr<Entidade> entidade(NovaEntidadeParaTestes(proto, g_tabelas));

  ntf::Notificacao n;
  PreencheNotificacaoAtualizacaoPontosVida(*entidade, /*delta_pontos_vida=*/-10, TD_LETAL, &n, nullptr);
  entidade->AtualizaParcial(n.entidade());
  proto = entidade->Proto();
  EXPECT_FALSE(proto.morta());
  EXPECT_FALSE(proto.inconsciente());
  EXPECT_FALSE(proto.incapacitada());
  EXPECT_FALSE(proto.caida());
}

TEST(TesteEscudo, TesteEscudoSemPenalidade) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("guerreiro");
  ic->set_nivel(1);
  proto.mutable_dados_defesa()->set_id_escudo("leve_madeira");
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("espada_longa");
    da->set_empunhadura(EA_ARMA_ESCUDO);
  }
  RecomputaDependencias(g_tabelas, &proto);

  EXPECT_EQ(1, proto.dados_ataque(0).bonus_ataque_final());
}

TEST(TesteEscudo, TesteEscudoComPenalidade) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(1);
  proto.mutable_dados_defesa()->set_id_escudo("leve_madeira");
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("adaga");
    da->set_empunhadura(EA_ARMA_ESCUDO);
  }
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("azagaia");
    da->set_empunhadura(EA_ARMA_ESCUDO);
  }
  RecomputaDependencias(g_tabelas, &proto);

  EXPECT_EQ(-1, proto.dados_ataque(0).bonus_ataque_final());
  EXPECT_EQ(-5, proto.dados_ataque(1).bonus_ataque_final());
}

TEST(TesteEscudo, TesteEscudoTalentoGuerreiro) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("guerreiro");
  ic->set_nivel(1);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(TalentoComEscudo("leve_madeira", proto));
  EXPECT_TRUE(TalentoComEscudo("pesado_aco", proto));
  EXPECT_TRUE(TalentoComEscudo("corpo", proto));
}

TEST(TesteEscudo, TesteEscudoBesta) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("guerreiro");
  ic->set_nivel(1);
  auto* talento = proto.mutable_info_talentos()->add_outros();
  talento->set_id("usar_arma_exotica");
  talento->set_complemento("besta_de_mao");
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("besta_de_mao");
    da->set_empunhadura(EA_ARMA_ESCUDO);
  }
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("besta_leve");
    da->set_empunhadura(EA_ARMA_ESCUDO);
  }
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("besta_pesada");
    da->set_empunhadura(EA_ARMA_ESCUDO);
  }

  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(1, proto.dados_ataque(0).bonus_ataque_final());
  EXPECT_EQ(-1, proto.dados_ataque(1).bonus_ataque_final());
  EXPECT_EQ(-3, proto.dados_ataque(2).bonus_ataque_final());
}

TEST(TesteEscudo, TesteEscudoTalentoRanger) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("ranger");
  ic->set_nivel(1);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_TRUE(TalentoComEscudo("leve_madeira", proto));
  EXPECT_TRUE(TalentoComEscudo("pesado_aco", proto));
  EXPECT_FALSE(TalentoComEscudo("corpo", proto));
}

TEST(TesteEscudo, TesteEscudoMago) {
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(1);
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_FALSE(TalentoComEscudo("leve_madeira", proto));
  EXPECT_FALSE(TalentoComEscudo("pesado_aco", proto));
  EXPECT_FALSE(TalentoComEscudo("corpo", proto));
}

TEST(TesteDependencias, TesteGracaDivina) {
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("paladino");
    ic->set_nivel(1);
    AtribuiBaseAtributo(12, TA_CARISMA, &proto);

    RecomputaDependencias(g_tabelas, &proto);
    // Carisma nao afeta ainda.
    EXPECT_EQ(2, BonusTotal(proto.dados_defesa().salvacao_fortitude()));
    EXPECT_EQ(0, BonusTotal(proto.dados_defesa().salvacao_reflexo()));
    EXPECT_EQ(0, BonusTotal(proto.dados_defesa().salvacao_vontade()));
  }
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("paladino");
    ic->set_nivel(2);
    AtribuiBaseAtributo(12, TA_CARISMA, &proto);

    // Carisma adiciona 1.
    RecomputaDependencias(g_tabelas, &proto);
    EXPECT_EQ(4, BonusTotal(proto.dados_defesa().salvacao_fortitude()));
    EXPECT_EQ(1, BonusTotal(proto.dados_defesa().salvacao_reflexo()));
    EXPECT_EQ(1, BonusTotal(proto.dados_defesa().salvacao_vontade()));
  }
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("paladino");
    ic->set_nivel(2);
    AtribuiBaseAtributo(8, TA_CARISMA, &proto);

    // Carisma negativo nao subtrai.
    RecomputaDependencias(g_tabelas, &proto);
    EXPECT_EQ(3, BonusTotal(proto.dados_defesa().salvacao_fortitude()));
    EXPECT_EQ(0, BonusTotal(proto.dados_defesa().salvacao_reflexo()));
    EXPECT_EQ(0, BonusTotal(proto.dados_defesa().salvacao_vontade()));
  }
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("paladino");
    ic->set_nivel(2);
    AtribuiBaseAtributo(10, TA_CARISMA, &proto);
    auto* manto = proto.mutable_tesouro()->add_mantos();
    manto->set_id("manto_carisma_2");
    manto->set_em_uso(true);

    // Carisma com item.
    RecomputaDependencias(g_tabelas, &proto);
    EXPECT_EQ(4, BonusTotal(proto.dados_defesa().salvacao_fortitude()));
    EXPECT_EQ(1, BonusTotal(proto.dados_defesa().salvacao_reflexo()));
    EXPECT_EQ(1, BonusTotal(proto.dados_defesa().salvacao_vontade()));
  }
}

TEST(TesteTesouro, TesteTesouroEsperado) {
  for (const auto& modelo : g_tabelas.TodosModelosEntidades().modelo()) {
    std::unique_ptr<Entidade> e(NovaEntidadeParaTestes(modelo.entidade(), g_tabelas));
    const auto& tesouro = e->Proto().tesouro();
    if (!tesouro.has_valor_esperado_po()) continue;
    int soma = 0;
    const auto& todos = TodosItens(e->Proto());
    for (const auto& item : todos) {
      soma += PrecoItemPo(ItemTabela(g_tabelas, *item));
    }
    for (const auto& arma : tesouro.armas()) {
      soma += PrecoArmaPo(arma);
    }
    for (const auto& aoes : {tesouro.armaduras(), tesouro.escudos()}) {
      for (const auto& aoe : aoes) {
        soma += PrecoArmaduraOuEscudoPo(aoe);
      }
    }
    for (const auto& item : tesouro.itens_mundanos()) {
      soma += g_tabelas.ItemMundano(item.id()).has_custo_po() ? g_tabelas.ItemMundano(item.id()).custo_po() : g_tabelas.ItemMundano(item.id()).custo().po();
    }

    soma += tesouro.moedas().po();
    EXPECT_LE(soma, tesouro.valor_esperado_po()) << "modelo extrapolou tesouro: " << modelo.id();
    if (soma <= tesouro.valor_esperado_po()) {
      LOG(INFO) << "valor de tesouro de " << modelo.id() << ": " << soma << " PO está dentro do esperado: " << tesouro.valor_esperado_po();
    }
  }
}

TEST(TesteTesouro, TesteTransicao) {
  EntidadeProto doador_proto;
  {
    auto* anel = doador_proto.mutable_tesouro()->add_aneis();
    anel->set_id("protecao_1");
    anel->set_em_uso(true);
    auto* moedas = doador_proto.mutable_tesouro()->mutable_moedas();
    moedas->set_po(3);
    moedas->set_pp(30);
    moedas->set_pc(300);
    auto* armadura = doador_proto.mutable_tesouro()->add_armaduras();
    armadura->set_id("cota_malha");
    auto* arma = doador_proto.mutable_tesouro()->add_armas();
    arma->set_id("espada_longa");
    auto* varinha = doador_proto.mutable_tesouro()->add_varinhas();
    varinha->set_id("missil_magico_1");
  }
  std::unique_ptr<Entidade> doador(NovaEntidadeParaTestes(doador_proto, g_tabelas));
  ASSERT_FALSE(doador->Proto().evento().empty());

  EntidadeProto receptor_proto;
  {
    auto* anel = receptor_proto.mutable_tesouro()->add_aneis();
    anel->set_id("escalada");
    anel->set_em_uso(true);
    auto* moedas = receptor_proto.mutable_tesouro()->mutable_moedas();
    moedas->set_po(4);
    moedas->set_pp(40);
    moedas->set_pe(400);
    moedas->set_pl(4000);
    auto* varinhas = receptor_proto.mutable_tesouro()->add_varinhas();
    varinhas->set_id("missil_magico_2");
  }
  std::unique_ptr<Entidade> receptor(NovaEntidadeParaTestes(receptor_proto, g_tabelas));
  ASSERT_FALSE(receptor->Proto().evento().empty());

  ntf::Notificacao n_grupo;
  n_grupo.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  PreencheNotificacoesTransicaoTesouro(g_tabelas, *doador, *receptor, &n_grupo, nullptr);

  {
    // Aplica.
    ASSERT_GE(n_grupo.notificacao_size(), 2);
    doador->AtualizaParcial(n_grupo.notificacao(0).entidade());
    EXPECT_TRUE(doador->Proto().evento().empty());
    EXPECT_TRUE(doador->Proto().tesouro().aneis().empty());
    EXPECT_TRUE(doador->Proto().tesouro().armaduras().empty());
    EXPECT_TRUE(doador->Proto().tesouro().armas().empty());
    EXPECT_TRUE(doador->Proto().tesouro().varinhas().empty());
    EXPECT_EQ(doador->Proto().tesouro().moedas().po(), 0);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pp(), 0);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pc(), 0);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pe(), 0);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pl(), 0);

    receptor->AtualizaParcial(n_grupo.notificacao(1).entidade());
    EXPECT_EQ(receptor->Proto().evento().size(), 1);
    ASSERT_EQ(receptor->Proto().tesouro().aneis().size(), 2);
    EXPECT_TRUE(receptor->Proto().tesouro().aneis(0).em_uso());
    EXPECT_FALSE(receptor->Proto().tesouro().aneis(1).em_uso());
    EXPECT_EQ(receptor->Proto().tesouro().armaduras().size(), 1);
    EXPECT_EQ(receptor->Proto().tesouro().armas().size(), 1);
    EXPECT_EQ(receptor->Proto().tesouro().varinhas().size(), 2);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().po(), 7);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pp(), 70);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pc(), 300);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pe(), 400);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pl(), 4000);
  }
  {
    // Aplica desfazer.
    doador->AtualizaParcial(n_grupo.notificacao(0).entidade_antes());
    EXPECT_EQ(doador->Proto().evento().size(), 1);
    EXPECT_FALSE(doador->Proto().tesouro().aneis().empty());
    EXPECT_TRUE(doador->Proto().tesouro().aneis(0).em_uso());
    EXPECT_FALSE(doador->Proto().tesouro().armaduras().empty());
    EXPECT_FALSE(doador->Proto().tesouro().armas().empty());
    EXPECT_FALSE(doador->Proto().tesouro().varinhas().empty());
    EXPECT_EQ(doador->Proto().tesouro().moedas().po(), 3);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pp(), 30);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pc(), 300);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pl(), 0);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pe(), 0);

    receptor->AtualizaParcial(n_grupo.notificacao(1).entidade_antes());
    EXPECT_EQ(receptor->Proto().evento().size(), 1);
    ASSERT_EQ(receptor->Proto().tesouro().aneis().size(), 1);
    EXPECT_TRUE(receptor->Proto().tesouro().aneis(0).em_uso());
    EXPECT_TRUE(receptor->Proto().tesouro().armaduras().empty());
    EXPECT_TRUE(receptor->Proto().tesouro().armas().empty());
    EXPECT_EQ(receptor->Proto().tesouro().varinhas().size(), 1);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().po(), 4);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pp(), 40);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pc(), 0);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pe(), 400);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pl(), 4000);
  }
}

TEST(TesteTesouro, TesteTransicaoTipoAnel) {
  EntidadeProto doador_proto;
  {
    auto* anel = doador_proto.mutable_tesouro()->add_aneis();
    anel->set_id("protecao_1");
    anel->set_em_uso(true);
    auto* moedas = doador_proto.mutable_tesouro()->mutable_moedas();
    moedas->set_po(3);
    moedas->set_pp(30);
    moedas->set_pc(300);
    auto* armadura = doador_proto.mutable_tesouro()->add_armaduras();
    armadura->set_id("cota_malha");
    armadura->set_em_uso(true);
    auto* arma = doador_proto.mutable_tesouro()->add_armas();
    arma->set_id("espada_longa");
    arma->set_em_uso(true);
  }
  std::unique_ptr<Entidade> doador(NovaEntidadeParaTestes(doador_proto, g_tabelas));
  ASSERT_FALSE(doador->Proto().evento().empty());

  EntidadeProto receptor_proto;
  {
    auto* anel = receptor_proto.mutable_tesouro()->add_aneis();
    anel->set_id("escalada");
    anel->set_em_uso(true);
    auto* moedas = receptor_proto.mutable_tesouro()->mutable_moedas();
    moedas->set_po(4);
    moedas->set_pp(40);
    moedas->set_pe(400);
    moedas->set_pl(4000);
  }
  std::unique_ptr<Entidade> receptor(NovaEntidadeParaTestes(receptor_proto, g_tabelas));
  ASSERT_FALSE(receptor->Proto().evento().empty());

  ntf::Notificacao n_grupo;
  n_grupo.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  PreencheNotificacoesTransicaoUmTipoTesouro(g_tabelas, TT_ANEL, *doador, *receptor, &n_grupo, nullptr);

  {
    // Aplica.
    ASSERT_GE(n_grupo.notificacao_size(), 2);
    doador->AtualizaParcial(n_grupo.notificacao(0).entidade());
    EXPECT_TRUE(doador->Proto().evento().empty());
    EXPECT_TRUE(doador->Proto().tesouro().aneis().empty());
    EXPECT_FALSE(doador->Proto().tesouro().armaduras().empty());
    EXPECT_FALSE(doador->Proto().tesouro().armas().empty());
    EXPECT_EQ(doador->Proto().tesouro().moedas().po(), 3);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pp(), 30);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pc(), 300);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pe(), 0);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pl(), 0);

    receptor->AtualizaParcial(n_grupo.notificacao(1).entidade());
    EXPECT_EQ(receptor->Proto().evento().size(), 1);
    ASSERT_EQ(receptor->Proto().tesouro().aneis().size(), 2);
    EXPECT_TRUE(receptor->Proto().tesouro().aneis(0).em_uso());
    EXPECT_FALSE(receptor->Proto().tesouro().aneis(1).em_uso());
    EXPECT_EQ(receptor->Proto().tesouro().armaduras().size(), 0);
    EXPECT_EQ(receptor->Proto().tesouro().armas().size(), 0);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().po(), 4);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pp(), 40);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pc(), 0);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pe(), 400);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pl(), 4000);
  }
  {
    // Aplica desfazer.
    doador->AtualizaParcial(n_grupo.notificacao(0).entidade_antes());
    EXPECT_EQ(doador->Proto().evento().size(), 1);
    EXPECT_FALSE(doador->Proto().tesouro().aneis().empty());
    EXPECT_TRUE(doador->Proto().tesouro().aneis(0).em_uso());
    EXPECT_FALSE(doador->Proto().tesouro().armaduras().empty());
    EXPECT_FALSE(doador->Proto().tesouro().armas().empty());
    EXPECT_EQ(doador->Proto().tesouro().moedas().po(), 3);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pp(), 30);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pc(), 300);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pl(), 0);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pe(), 0);

    receptor->AtualizaParcial(n_grupo.notificacao(1).entidade_antes());
    EXPECT_EQ(receptor->Proto().evento().size(), 1);
    ASSERT_EQ(receptor->Proto().tesouro().aneis().size(), 1);
    EXPECT_TRUE(receptor->Proto().tesouro().aneis(0).em_uso());
    EXPECT_TRUE(receptor->Proto().tesouro().armaduras().empty());
    EXPECT_TRUE(receptor->Proto().tesouro().armas().empty());
    EXPECT_EQ(receptor->Proto().tesouro().moedas().po(), 4);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pp(), 40);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pc(), 0);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pe(), 400);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pl(), 4000);
  }
}

TEST(TesteTesouro, TesteTransicaoTipoArma) {
  EntidadeProto doador_proto;
  {
    auto* armadura = doador_proto.mutable_tesouro()->add_armaduras();
    armadura->set_id("cota_malha");
    auto* arma = doador_proto.mutable_tesouro()->add_armas();
    arma->set_id("espada_longa");
  }
  std::unique_ptr<Entidade> doador(NovaEntidadeParaTestes(doador_proto, g_tabelas));

  EntidadeProto receptor_proto;
  {
    auto* arma = receptor_proto.mutable_tesouro()->add_armas();
    arma->set_id("espada_curta");
  }
  std::unique_ptr<Entidade> receptor(NovaEntidadeParaTestes(receptor_proto, g_tabelas));

  ntf::Notificacao n_grupo;
  n_grupo.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  PreencheNotificacoesTransicaoUmTipoTesouro(g_tabelas, TT_ARMA, *doador, *receptor, &n_grupo, nullptr);

  {
    // Aplica.
    ASSERT_GE(n_grupo.notificacao_size(), 2);
    doador->AtualizaParcial(n_grupo.notificacao(0).entidade());
    EXPECT_FALSE(doador->Proto().tesouro().armaduras().empty());
    EXPECT_TRUE(doador->Proto().tesouro().armas().empty());

    receptor->AtualizaParcial(n_grupo.notificacao(1).entidade());
    EXPECT_EQ(receptor->Proto().tesouro().armaduras().size(), 0);
    EXPECT_EQ(receptor->Proto().tesouro().armas().size(), 2);
  }
  {
    // Aplica desfazer.
    doador->AtualizaParcial(n_grupo.notificacao(0).entidade_antes());
    EXPECT_FALSE(doador->Proto().tesouro().armaduras().empty());
    EXPECT_FALSE(doador->Proto().tesouro().armas().empty());

    receptor->AtualizaParcial(n_grupo.notificacao(1).entidade_antes());
    EXPECT_TRUE(receptor->Proto().tesouro().armaduras().empty());
    EXPECT_FALSE(receptor->Proto().tesouro().armas().empty());
  }
}

TEST(TesteTesouro, TesteDoacao) {
  EntidadeProto doador_proto;
  {
    auto* anel = doador_proto.mutable_tesouro()->add_aneis();
    anel->set_id("protecao_1");
    anel->set_em_uso(true);
    auto* amuleto = doador_proto.mutable_tesouro()->add_amuletos();
    amuleto->set_id("protecao_1");
    amuleto->set_em_uso(true);
    amuleto = doador_proto.mutable_tesouro()->add_amuletos();
    amuleto->set_id("protecao_1");
    amuleto->set_em_uso(false);
    auto* moedas = doador_proto.mutable_tesouro()->mutable_moedas();
    moedas->set_po(3);
    moedas->set_pp(30);
    moedas->set_pc(300);
  }
  std::unique_ptr<Entidade> doador(NovaEntidadeParaTestes(doador_proto, g_tabelas));
  ASSERT_FALSE(doador->Proto().evento().empty());

  EntidadeProto receptor_proto;
  {
    auto* anel = receptor_proto.mutable_tesouro()->add_aneis();
    anel->set_id("escalada");
    anel->set_em_uso(true);
    auto* moedas = receptor_proto.mutable_tesouro()->mutable_moedas();
    moedas->set_po(4);
    moedas->set_pp(40);
    moedas->set_pe(400);
    moedas->set_pl(4000);
  }
  std::unique_ptr<Entidade> receptor(NovaEntidadeParaTestes(receptor_proto, g_tabelas));
  ASSERT_FALSE(receptor->Proto().evento().empty());

  ntf::Notificacao notificacao_doacao;
  {
    auto* n_doador = notificacao_doacao.mutable_entidade();
    n_doador->set_id(doador->Id());
    auto* tesouro_doado = n_doador->mutable_tesouro();
    tesouro_doado->mutable_moedas()->set_po(3);  // qualquer valor funciona, so pra indicar que eh ouro.
    EXPECT_FALSE(doador->Proto().tesouro().aneis().empty());
    *tesouro_doado->add_aneis() = doador->Proto().tesouro().aneis(0);
    EXPECT_EQ(doador->Proto().tesouro().amuletos().size(), 2);
    *tesouro_doado->add_amuletos() = doador->Proto().tesouro().amuletos(1);
  }

  ntf::Notificacao n_grupo;
  ntf::Notificacao n_desfazer;
  n_grupo.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  PreencheNotificacoesDoacaoParcialTesouro(g_tabelas, notificacao_doacao, doador->Proto(), receptor->Proto(), &n_grupo, &n_desfazer);

  {
    // Aplica.
    ASSERT_GE(n_grupo.notificacao_size(), 2);
    doador->AtualizaParcial(n_grupo.notificacao(0).entidade());
    EXPECT_TRUE(doador->Proto().evento().empty());
    EXPECT_TRUE(doador->Proto().tesouro().aneis().empty());
    EXPECT_EQ(doador->Proto().tesouro().amuletos().size(), 1);
    EXPECT_EQ(doador->Proto().tesouro().moedas().po(), 0);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pp(), 30);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pc(), 300);

    receptor->AtualizaParcial(n_grupo.notificacao(1).entidade());
    EXPECT_EQ(receptor->Proto().evento().size(), 1);
    ASSERT_EQ(receptor->Proto().tesouro().aneis().size(), 2);
    EXPECT_TRUE(receptor->Proto().tesouro().aneis(0).em_uso());
    EXPECT_FALSE(receptor->Proto().tesouro().aneis(1).em_uso());
    ASSERT_FALSE(receptor->Proto().tesouro().amuletos().empty());
    EXPECT_FALSE(receptor->Proto().tesouro().amuletos(0).em_uso());
    EXPECT_EQ(receptor->Proto().tesouro().moedas().po(), 7);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pp(), 40);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pe(), 400);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pl(), 4000);
  }
  {
    // Aplica desfazer.
    doador->AtualizaParcial(n_grupo.notificacao(0).entidade_antes());
    EXPECT_EQ(doador->Proto().evento().size(), 1);
    EXPECT_FALSE(doador->Proto().tesouro().aneis().empty());
    EXPECT_TRUE(doador->Proto().tesouro().aneis(0).em_uso());
    EXPECT_EQ(doador->Proto().tesouro().amuletos().size(), 2);
    EXPECT_TRUE(doador->Proto().tesouro().amuletos(0).em_uso());
    EXPECT_FALSE(doador->Proto().tesouro().amuletos(1).em_uso());
    EXPECT_EQ(doador->Proto().tesouro().moedas().po(), 3);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pp(), 30);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pc(), 300);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pl(), 0);
    EXPECT_EQ(doador->Proto().tesouro().moedas().pe(), 0);

    receptor->AtualizaParcial(n_grupo.notificacao(1).entidade_antes());
    EXPECT_EQ(receptor->Proto().evento().size(), 1);
    ASSERT_EQ(receptor->Proto().tesouro().aneis().size(), 1);
    EXPECT_TRUE(receptor->Proto().tesouro().aneis(0).em_uso());
    EXPECT_TRUE(receptor->Proto().tesouro().amuletos().empty());
    EXPECT_EQ(receptor->Proto().tesouro().moedas().po(), 4);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pp(), 40);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pc(), 0);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pe(), 400);
    EXPECT_EQ(receptor->Proto().tesouro().moedas().pl(), 4000);
  }
}

TEST(TestTipoAtaqueReseta, TesteTipoAtaqueReseta) {
  EntidadeProto proto;
  {
    auto* da = proto.add_dados_ataque();
    da->set_tipo_ataque("Ataque Corpo a Corpo");
    da->set_ataque_toque(true);
    da->set_ataque_agarrar(true);
    da->set_grupo("teste");
  }
  RecomputaDependencias(g_tabelas, &proto);
  DadosAtaque* dat = nullptr;
  for (auto& da : *proto.mutable_dados_ataque()) {
    if (da.grupo() == "teste") {
      dat = &da;
      break;
    }
  }
  ASSERT_NE(dat, nullptr);
  EXPECT_FALSE(dat->ataque_agarrar());
  EXPECT_FALSE(dat->ataque_toque());
}

TEST(TestFeiticos, TesteFeiticosLink) {
  for (const auto& feitico : g_tabelas.todas().tabela_feiticos().armas()) {
    EXPECT_FALSE(feitico.link().empty()) << "feitico: " << feitico.id();
  }
}

TEST(TestFeiticos, TesteFeiticosComSalvacaoTemSalvacao) {
  for (const auto& feitico : g_tabelas.todas().tabela_feiticos().armas()) {
    if (!feitico.has_acao()) continue;
    auto acao = g_tabelas.Acao(feitico.acao().id());
    bool ou_nao_tem_id_ou_tem_id_valido = !feitico.acao().has_id() || acao.has_id();
    EXPECT_TRUE(ou_nao_tem_id_ou_tem_id_valido) 
        << "Feitiço tem id de acao invalido: " << feitico.nome();
    if (!ou_nao_tem_id_ou_tem_id_valido) continue;
    acao.MergeFrom(feitico.acao());
    if (acao.permite_salvacao()) {
      EXPECT_TRUE(acao.has_tipo_salvacao()) << "feitico sem tipo de salvacao: " << feitico.nome();
      EXPECT_TRUE(acao.has_dificuldade_salvacao_base() || acao.has_dificuldade_salvacao_por_nivel()) << "feitico sem CD de salvacao: " << feitico.nome();
    }
  }
}


TEST(TestFeiticos, RodadasBaseAnterior) {
  for (const auto& feitico : g_tabelas.todas().tabela_feiticos().armas()) {
    if (!feitico.acao().efeitos_adicionais().empty()) {
      EXPECT_FALSE(feitico.acao().efeitos_adicionais(0).rodadas_base_igual_efeito_anterior())
        << "rodadas_base_igual_efeito_anterior invalido para feitico " << feitico.id();
    }
    if (!feitico.acao().efeitos_adicionais_se_salvou().empty()) {
      EXPECT_FALSE(feitico.acao().efeitos_adicionais_se_salvou(0).rodadas_base_igual_efeito_anterior())
        << "rodadas_base_igual_efeito_anterior (se salvou) invalido para feitico " << feitico.id();
    }
  }
}

TEST(TesteRacas, TesteGnomo) {
  EntidadeProto proto;
  proto.set_raca("gnomo");
  {
    auto* ic = proto.add_info_classes();
    ic->set_nivel(1);
    ic->set_id("guerreiro");
  }
  std::unique_ptr<Entidade> gnomo(NovaEntidadeParaTestes(proto, g_tabelas));
  EXPECT_EQ(BonusTotal(gnomo->Proto().movimento().terrestre_q()), 4);
  EXPECT_TRUE(TalentoComArma(g_tabelas.Arma("martelo_gnomo_com_gancho"), gnomo->Proto()));
}


TEST(TesteRacas, TesteElfo) {
  EntidadeProto proto;
  AtribuiBaseAtributo(10, TA_DESTREZA, &proto);
  AtribuiBaseAtributo(10, TA_SABEDORIA, &proto);
  AtribuiBaseAtributo(10, TA_CONSTITUICAO, &proto);
  AtribuiBaseAtributo(10, TA_DESTREZA, &proto);
  proto.set_raca("elfo");
  {
    auto* ic = proto.add_info_classes();
    ic->set_nivel(1);
    ic->set_id("guerreiro");
  }
  std::unique_ptr<Entidade> elfo(NovaEntidadeParaTestes(proto, g_tabelas));
  EXPECT_EQ(BonusTotal(elfo->Proto().movimento().terrestre_q()), 6);
  EXPECT_EQ(ValorFinalPericia("ouvir", elfo->Proto()), 2);
  EXPECT_EQ(ValorFinalPericia("procurar", elfo->Proto()), 2);
  EXPECT_EQ(ValorFinalPericia("observar", elfo->Proto()), 2);
  EXPECT_EQ(ValorFinalPericia("diplomacia", elfo->Proto()), 0);
  EXPECT_EQ(ValorFinalPericia("obter_informacao", elfo->Proto()), 0);
  EXPECT_TRUE(elfo->ImuneEfeito(EFEITO_SONO));
  EXPECT_EQ(elfo->Proto().tipo_visao(), VISAO_BAIXA_LUMINOSIDADE);

  // Salvacoes contra alguns ataques.
  std::unique_ptr<Entidade> ea;
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(1);
    AtribuiBaseAtributo(20, TA_INTELIGENCIA, &proto);  // +5
    {
      // 16 de CD, mas nao é feitico.
      auto* da = proto.mutable_dados_ataque()->Add();
      da->set_dificuldade_salvacao(16);
      da->set_tipo_salvacao(TS_FORTITUDE);
    }
    {
      // 16 de CD reflexo.
      auto* da = proto.mutable_dados_ataque()->Add();
      da->set_tipo_ataque("Feitiço de Mago");
      da->set_id_arma("maos_flamejantes");
    }
    {
      // 13 de CD vontade.
      auto* da = proto.mutable_dados_ataque()->Add();
      da->set_tipo_ataque("Pergaminho Arcano");
      da->set_nivel_conjurador_pergaminho(1);
      da->set_modificador_atributo_pergaminho(2);
      da->set_id_arma("enfeiticar_pessoa");
    }
    ea = NovaEntidadeParaTestes(proto, g_tabelas);
    ASSERT_GE(ea->Proto().dados_ataque_size(), 3);
  }
  EXPECT_EQ(elfo->Salvacao(*ea, TS_FORTITUDE), 1);
  EXPECT_EQ(elfo->Salvacao(*ea, TS_REFLEXO), 1);
  EXPECT_EQ(elfo->Salvacao(*ea, TS_VONTADE), 0);
  EXPECT_EQ(elfo->SalvacaoVeneno(), 1);

  // Testa o ataque nao feitico, bonus 1 vs CD 16.
  g_dados_teste.push(14);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(0), *ea, *elfo)), false) << "cd: " << ea->Proto().dados_ataque(0).dificuldade_salvacao();
  g_dados_teste.push(15);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(0), *ea, *elfo)), true) << "cd: " << ea->Proto().dados_ataque(0).dificuldade_salvacao();
  // Testa o ataque feitico, bonus 1 vs CD 16.
  g_dados_teste.push(14);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(1), *ea, *elfo)), false)
      << "bonus: " << elfo->SalvacaoFeitico(*ea, ea->Proto().dados_ataque(1))
      << ", cd: " << ea->Proto().dados_ataque(1).dificuldade_salvacao();
  g_dados_teste.push(15);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(1), *ea, *elfo)), true)
      << "bonus: " << elfo->SalvacaoFeitico(*ea, ea->Proto().dados_ataque(1))
      << ", cd: " << ea->Proto().dados_ataque(1).dificuldade_salvacao();
  // Testa o pergaminho feitico, bonus 0+2 vs CD 13.
  g_dados_teste.push(10);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(2), *ea, *elfo)), false)
      << "bonus: " << elfo->SalvacaoFeitico(*ea, ea->Proto().dados_ataque(2))
      << ", cd: " << ea->Proto().dados_ataque(2).dificuldade_salvacao();
  g_dados_teste.push(11);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(2), *ea, *elfo)), true)
      << "bonus: " << elfo->SalvacaoFeitico(*ea, ea->Proto().dados_ataque(2))
      << ", cd: " << ea->Proto().dados_ataque(2).dificuldade_salvacao();

  EXPECT_FALSE(TalentoComArma(g_tabelas.Arma("machado_de_guerra_anao"), elfo->Proto()));
  EXPECT_TRUE(TalentoComArma(g_tabelas.Arma("espada_longa"), elfo->Proto()));
  EXPECT_TRUE(TalentoComArma(g_tabelas.Arma("espada_curta"), elfo->Proto()));
  EXPECT_TRUE(TalentoComArma(g_tabelas.Arma("arco_longo"), elfo->Proto()));
  EXPECT_TRUE(TalentoComArma(g_tabelas.Arma("arco_curto"), elfo->Proto()));
  EXPECT_TRUE(TalentoComArma(g_tabelas.Arma("arco_longo_composto"), elfo->Proto()));
  EXPECT_TRUE(TalentoComArma(g_tabelas.Arma("sabre"), elfo->Proto()));
}

TEST(TesteRacas, TesteMeioElfo) {
  EntidadeProto proto;
  AtribuiBaseAtributo(10, TA_SABEDORIA, &proto);
  AtribuiBaseAtributo(10, TA_CONSTITUICAO, &proto);
  AtribuiBaseAtributo(10, TA_DESTREZA, &proto);
  proto.set_raca("meio_elfo");
  {
    auto* ic = proto.add_info_classes();
    ic->set_nivel(1);
    ic->set_id("guerreiro");
  }
  std::unique_ptr<Entidade> ed(NovaEntidadeParaTestes(proto, g_tabelas));
  EXPECT_EQ(BonusTotal(ed->Proto().movimento().terrestre_q()), 6);
  EXPECT_EQ(ValorFinalPericia("ouvir", ed->Proto()), 1);
  EXPECT_EQ(ValorFinalPericia("procurar", ed->Proto()), 1);
  EXPECT_EQ(ValorFinalPericia("observar", ed->Proto()), 1);
  EXPECT_EQ(ValorFinalPericia("diplomacia", ed->Proto()), 2);
  EXPECT_EQ(ValorFinalPericia("obter_informacao", ed->Proto()), 2);
  EXPECT_TRUE(ed->ImuneEfeito(EFEITO_SONO));
  EXPECT_EQ(ed->Proto().tipo_visao(), VISAO_BAIXA_LUMINOSIDADE);

  // Salvacoes contra alguns ataques.
  std::unique_ptr<Entidade> ea;
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(1);
    AtribuiBaseAtributo(20, TA_INTELIGENCIA, &proto);  // +5
    {
      // 16 de CD, mas nao é feitico.
      auto* da = proto.mutable_dados_ataque()->Add();
      da->set_dificuldade_salvacao(16);
      da->set_tipo_salvacao(TS_FORTITUDE);
    }
    {
      // 16 de CD reflexo.
      auto* da = proto.mutable_dados_ataque()->Add();
      da->set_tipo_ataque("Feitiço de Mago");
      da->set_id_arma("maos_flamejantes");
    }
    {
      // 13 de CD vontade.
      auto* da = proto.mutable_dados_ataque()->Add();
      da->set_tipo_ataque("Pergaminho Arcano");
      da->set_nivel_conjurador_pergaminho(1);
      da->set_modificador_atributo_pergaminho(2);
      da->set_id_arma("enfeiticar_pessoa");
    }
    ea = NovaEntidadeParaTestes(proto, g_tabelas);
    ASSERT_GE(ea->Proto().dados_ataque_size(), 3);
  }
  EXPECT_EQ(ed->Salvacao(*ea, TS_FORTITUDE), 2);
  EXPECT_EQ(ed->Salvacao(*ea, TS_REFLEXO), 0);
  EXPECT_EQ(ed->Salvacao(*ea, TS_VONTADE), 0);
  EXPECT_EQ(ed->SalvacaoVeneno(), 2);

  // Testa o ataque nao feitico, bonus 2 vs CD 16.
  g_dados_teste.push(13);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(0), *ea, *ed)), false) << "cd: " << ea->Proto().dados_ataque(0).dificuldade_salvacao();
  g_dados_teste.push(14);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(0), *ea, *ed)), true) << "cd: " << ea->Proto().dados_ataque(0).dificuldade_salvacao();
  // Testa o ataque feitico, bonus 0 vs CD 16.
  g_dados_teste.push(15);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(1), *ea, *ed)), false)
      << "bonus: " << ed->SalvacaoFeitico(*ea, ea->Proto().dados_ataque(1))
      << ", cd: " << ea->Proto().dados_ataque(1).dificuldade_salvacao();
  g_dados_teste.push(16);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(1), *ea, *ed)), true)
      << "bonus: " << ed->SalvacaoFeitico(*ea, ea->Proto().dados_ataque(1))
      << ", cd: " << ea->Proto().dados_ataque(1).dificuldade_salvacao();
  // Testa o pergaminho feitico, bonus 0+2 vs CD 13.
  g_dados_teste.push(10);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(2), *ea, *ed)), false)
      << "bonus: " << ed->SalvacaoFeitico(*ea, ea->Proto().dados_ataque(2))
      << ", cd: " << ea->Proto().dados_ataque(2).dificuldade_salvacao();
  g_dados_teste.push(11);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(2), *ea, *ed)), true)
      << "bonus: " << ed->SalvacaoFeitico(*ea, ea->Proto().dados_ataque(2))
      << ", cd: " << ea->Proto().dados_ataque(2).dificuldade_salvacao();

}

TEST(TesteRacas, TesteAnao) {
  std::unique_ptr<Entidade> anao;
  {
    EntidadeProto proto;
    AtribuiBaseAtributo(15, TA_CONSTITUICAO, &proto);  // com bonus de anao, vai para 17.
    AtribuiBaseAtributo(10, TA_CARISMA, &proto);  // com penalidade de anao, vai para 8.
    proto.set_raca("anao");
    auto* ic = proto.add_info_classes();
    ic->set_nivel(1);
    ic->set_id("guerreiro");
    auto* da = proto.mutable_dados_ataque()->Add();
    da->set_id_arma("machado_de_batalha");
    anao = NovaEntidadeParaTestes(proto, g_tabelas);
  }

  std::unique_ptr<Entidade> gigante;
  {
    EntidadeProto proto;
    proto.add_tipo_dnd(TIPO_GIGANTE);
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(1);
    AtribuiBaseAtributo(20, TA_INTELIGENCIA, &proto);
    {
      // 16 de CD, mas nao é feitico.
      auto* da = proto.mutable_dados_ataque()->Add();
      da->set_dificuldade_salvacao(16);
      da->set_tipo_salvacao(TS_FORTITUDE);
    }
    {
      // 16 de CD reflexo, feitico.
      auto* da = proto.mutable_dados_ataque()->Add();
      da->set_tipo_ataque("Feitiço de Mago");
      da->set_id_arma("maos_flamejantes");
    }
    {
      // 13 de CD reflexo, feitico.
      auto* da = proto.mutable_dados_ataque()->Add();
      da->set_tipo_ataque("Pergaminho Arcano");
      da->set_nivel_conjurador_pergaminho(1);
      da->set_modificador_atributo_pergaminho(2);
      da->set_id_arma("maos_flamejantes");
    }

    gigante = NovaEntidadeParaTestes(proto, g_tabelas);
    ASSERT_GE(gigante->Proto().dados_ataque_size(), 3);
  }

  EXPECT_EQ(BonusTotal(BonusAtributo(TA_CONSTITUICAO, anao->Proto())), 17);
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_CARISMA, anao->Proto())), 8);
  EXPECT_EQ(anao->Salvacao(*gigante, TS_FORTITUDE), 5);  // 2 + 3.
  EXPECT_EQ(anao->SalvacaoVeneno(), 7);  // 2 + 3 + 2.
  EXPECT_EQ(anao->SalvacaoFeitico(*gigante, gigante->Proto().dados_ataque(1)), 2);  // 0 + 0 + 2.

  // Testa o ataque nao feitico, bonus 5 vs CD 16.
  g_dados_teste.push(10);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, gigante->Proto().dados_ataque(0), *gigante, *anao)), false) << "cd: " << gigante->Proto().dados_ataque(0).dificuldade_salvacao();
  g_dados_teste.push(11);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, gigante->Proto().dados_ataque(0), *gigante, *anao)), true) << "cd: " << gigante->Proto().dados_ataque(0).dificuldade_salvacao();
  // Testa o ataque feitico, bonus 2 vs CD 16.
  g_dados_teste.push(13);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, gigante->Proto().dados_ataque(1), *gigante, *anao)), false)
      << "bonus: " << anao->SalvacaoFeitico(*gigante, gigante->Proto().dados_ataque(1))
      << ", cd: " << gigante->Proto().dados_ataque(1).dificuldade_salvacao();
  g_dados_teste.push(14);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, gigante->Proto().dados_ataque(1), *gigante, *anao)), true)
      << "bonus: " << anao->SalvacaoFeitico(*gigante, gigante->Proto().dados_ataque(1))
      << ", cd: " << gigante->Proto().dados_ataque(1).dificuldade_salvacao();
  // Testa o pergaminho feitico, bonus 2 vs CD 13.
  g_dados_teste.push(10);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, gigante->Proto().dados_ataque(2), *gigante, *anao)), false)
      << "bonus: " << anao->SalvacaoFeitico(*gigante, gigante->Proto().dados_ataque(2))
      << ", cd: " << gigante->Proto().dados_ataque(2).dificuldade_salvacao();
  g_dados_teste.push(11);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, gigante->Proto().dados_ataque(2), *gigante, *anao)), true)
      << "bonus: " << anao->SalvacaoFeitico(*gigante, gigante->Proto().dados_ataque(2))
      << ", cd: " << gigante->Proto().dados_ataque(2).dificuldade_salvacao();

  EXPECT_EQ(anao->CA(*gigante, Entidade::CA_NORMAL), 14);
  {
    EntidadeProto a;
    a.set_surpreso(true);
    anao->AtualizaParcial(a);
  }
  EXPECT_EQ(anao->CA(*gigante, Entidade::CA_NORMAL), 10);

  for (SubTipoDnD sub_tipo : {SUBTIPO_ORC, SUBTIPO_GOBLINOIDE}) {
    EntidadeProto proto;
    proto.add_sub_tipo_dnd(sub_tipo);
    EXPECT_EQ(ModificadorAtaqueWrapper(TipoAtaque::CORPO_A_CORPO, anao->Proto(), proto), 1);
  }

  EXPECT_TRUE(TalentoComArma(g_tabelas.Arma("machado_de_guerra_anao"), anao->Proto()));
  EXPECT_TRUE(TalentoComArma(g_tabelas.Arma("urgrosh_anao"), anao->Proto()));
}

TEST(TesteRacas, TesteAasimar) {
  EntidadeProto proto;
  AtribuiBaseAtributo(12, TA_SABEDORIA, &proto);  // com bonus de aasimar fica 14, +2 de bonus.
  proto.set_raca("aasimar");
  RecomputaDependencias(g_tabelas, &proto);
  RecomputaDependencias(g_tabelas, &proto);  // duas vezes pra ver se num gera duas vezes o ataque.
  int c = 0;
  for (const auto& pericia : proto.info_pericias()) {
    if (pericia.id() == "observar" || pericia.id() == "ouvir") {
      EXPECT_EQ(BonusTotal(pericia.bonus()), 4) << "pericia: " << pericia.DebugString();
      ++c;
    }
  }
  ASSERT_EQ(c, 2);
  EXPECT_EQ(DadosAtaquePorGrupo("aasimar", proto).id_raca(), "aasimar") << proto.dados_ataque(0).DebugString();
}

TEST(TesteRacas, TesteFalcao) {
  Modelos modelos;
  EntidadeProto proto = g_tabelas.ModeloEntidade("Falcão").entidade();
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(ValorFinalPericia("ouvir", proto), 4);
  EXPECT_EQ(ValorFinalPericia("observar", proto), 16);
}

TEST(TesteDominios, TesteRenovacao) {
  EntidadeProto proto;
  proto.set_dados_vida("5");
  AtribuiBaseAtributo(14, TA_CARISMA, &proto);  // +2
  {
    auto* ic = proto.add_info_classes();
    ic->set_nivel(1);
    ic->set_id("clerigo");
    auto* ifc = FeiticosClasse("clerigo", &proto);
    ifc->add_dominios("renovacao");
  }
  std::unique_ptr<Entidade> e(NovaEntidadeParaTestes(proto, g_tabelas));
  {
    // Nada acontece.
    ntf::Notificacao n;
    auto [delta, texto] = RenovaSeTiverDominioRenovacao(e->Proto(), -5, TD_LETAL, &n, nullptr);
    EXPECT_FALSE(n.has_tipo()) << "delta: " << delta << ", texto: " << texto;
  }
  {
    // Ativa renovacao.
    g_dados_teste.push(5); // d8 + 2 = 7
    ntf::Notificacao n;
    auto [delta, texto] = RenovaSeTiverDominioRenovacao(e->Proto(), -6, TD_LETAL, &n, nullptr);
    EXPECT_TRUE(n.has_tipo());
    EXPECT_EQ(delta, 1) << ", texto: " << texto;  // mudou de -6 para 1 por causa da renovacao.
    EXPECT_FALSE(texto.empty());
    e->AtualizaParcial(n.entidade());
    EXPECT_EQ(PoderesDoDominio("renovacao", e->Proto()).usado(), true);
    EXPECT_EQ(PoderesDoDominio("renovacao", e->Proto()).disponivel_em(), DIA_EM_RODADAS);
  }
  {
    ntf::Notificacao n;
    auto [delta, texto] = RenovaSeTiverDominioRenovacao(e->Proto(), -6, TD_LETAL, &n, nullptr);
    EXPECT_EQ(delta, -6) << ", texto: " << texto << ", delta: " << delta;  // nao muda, ja usou.
    EXPECT_FALSE(n.has_tipo());
    EXPECT_TRUE(texto.empty());
  }
}

TEST(TesteDominios, TesteNobrezaMorte) {
  ntf::Notificacao n;
  EntidadeProto proto;
  std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto, g_tabelas));
  AtribuiBaseAtributo(14, TA_CARISMA, &proto);  // +2
  {
    auto* ic = proto.add_info_classes();
    ic->set_nivel(1);
    ic->set_id("clerigo");
    auto* ifc = FeiticosClasse("clerigo", &proto);
    ifc->add_dominios("nobreza");
    ifc->add_dominios("morte");
  }
  RecomputaDependencias(g_tabelas, &proto);
  RecomputaDependencias(g_tabelas, &proto);  // pra garantir que num vai adicionar duas vezes.

  ASSERT_EQ(proto.dados_ataque().size(), 3);
  {
    AcaoProto acao = DadosAtaquePorGrupo("dominio nobreza", proto).acao();
    ResolveEfeitosAdicionaisVariaveis(/*nivel_conjurador=*/1, proto, *alvo, &acao);
    ASSERT_FALSE(acao.efeitos_adicionais().empty()) << "acao: " << acao.DebugString();
    EXPECT_EQ(acao.efeitos_adicionais(0).efeito(), EFEITO_INSPIRAR_CORAGEM);
    EXPECT_EQ(acao.efeitos_adicionais(0).rodadas(), 2);
  }
  {
    AcaoProto acao = DadosAtaquePorGrupo("dominio morte", proto).acao();
    ResolveEfeitosAdicionaisVariaveis(/*nivel_conjurador=*/1, proto, *alvo, &acao);
    ASSERT_FALSE(acao.efeitos_adicionais().empty()) << "acao: " << acao.DebugString();
    EXPECT_EQ(acao.efeitos_adicionais(0).efeito(), EFEITO_MORTE);
  }
}

// Retorna true se o feitico for do dominio para o nivel passado.
bool FeiticoDominio(const ArmaProto& feitico_tabelado, int nivel, const std::string& dominio) {
  for (const auto& ic : feitico_tabelado.info_classes()) {
    if (ic.id() == dominio && ic.nivel() == nivel) return true;
  }
  return false;
}

TEST(TesteDominios, TesteProtecao) {
  EntidadeProto proto;
  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_DOMINIO_PROTECAO);
  }
  std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto, g_tabelas));
  AtribuiBaseAtributo(14, TA_SABEDORIA, &proto);  // +2
  AtribuiBaseAtributo(14, TA_CARISMA, &proto);  // +2
  {
    auto* ic = proto.add_info_classes();
    ic->set_nivel(3);
    ic->set_id("clerigo");
    auto* ifc = FeiticosClasse("clerigo", &proto);
    ifc->add_dominios("protecao");
  }
  RecomputaDependencias(g_tabelas, &proto);
  RecomputaDependencias(g_tabelas, &proto);  // pra garantir que num vai adicionar duas vezes.

  ASSERT_EQ(proto.dados_ataque().size(), 2);
  {
    AcaoProto acao = DadosAtaquePorGrupo("dominio protecao", proto).acao();
    ASSERT_FALSE(acao.efeitos_adicionais().empty()) << "da: " << acao.DebugString();
    EXPECT_EQ(acao.efeitos_adicionais(0).efeito(), EFEITO_DOMINIO_PROTECAO);
    EXPECT_EQ(acao.efeitos_adicionais(0).rodadas(), 600);  // 1 hora
    EXPECT_FALSE(acao.permite_ataque_vs_defesa());
    EXPECT_FALSE(acao.permite_salvacao());
    EXPECT_TRUE(acao.ignora_resistencia_magia());
  }
  ntf::Notificacao n = PreencheNotificacaoExpiracaoEventoPosSalvacao(*alvo);
  ASSERT_FALSE(alvo->Proto().evento().empty());
  EXPECT_EQ(alvo->Proto().evento(0).id_efeito(), EFEITO_DOMINIO_PROTECAO);
  alvo->AtualizaParcial(n.entidade());
  ASSERT_TRUE(alvo->Proto().evento().empty());

  // Nivel 3 tem progressao: 4, 2+1+1, 1+1+1
  {
    const auto& fn0 = FeiticosNivel("clerigo", 0, proto);
    ASSERT_EQ(fn0.para_lancar_size(), 4);
  }
  {
    const auto& fn1 = FeiticosNivel("clerigo", 1, proto);
    ASSERT_EQ(fn1.para_lancar_size(), 4);
    EXPECT_TRUE(c_any_of(fn1.conhecidos(), [](const EntidadeProto::InfoConhecido& ic) { return ic.id() == "santuario"; }))
        << "nao encontrei santuario, todos: " << fn1.DebugString();
  }
  {
    const auto& fn2 = FeiticosNivel("clerigo", 2, proto);
    ASSERT_EQ(fn2.para_lancar_size(), 3);
    EXPECT_TRUE(c_any_of(fn2.conhecidos(), [](const EntidadeProto::InfoConhecido& ic) { return ic.id() == "proteger_outro"; }))
        << "nao encontrei proteger_outro, todos: " << fn2.DebugString();
  }
}

TEST(TesteCondicoes, TesteIndefeso) {
  auto modelo = g_tabelas.ModeloEntidade("Humano Plebeu 1");
  modelo.mutable_entidade()->set_gerar_agarrar(false);
  modelo.mutable_entidade()->set_morta(true);

  auto plebeu = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
  EXPECT_EQ(plebeu->CA(*plebeu, Entidade::CA_NORMAL), 5);
}

TEST(TesteTabelas, TesteFeiticoAleatorioClerigo) {
  Tabelas::DadosParaFeiticoAleatorio dfa;
  dfa.id_classe = "clerigo";
  dfa.nivel = 1;
  dfa.descritores_proibidos.emplace({DESC_MAL, DESC_CAOS});
  dfa.feiticos_excluidos = {"bencao", "maldicao_menor", "infligir_ferimentos_leves"};
  for (int i = 0; i < 100; ++i) {
    const auto& id = g_tabelas.FeiticoAleatorio(dfa);
    EXPECT_FALSE(id.empty());
    EXPECT_NE(id, "bencao");
    EXPECT_NE(id, "maldicao_menor");
    EXPECT_NE(id, "infligir_ferimentos_leves");
    EXPECT_NE(g_tabelas.Feitico(id).acao().alinhamento_bem_mal(), DESC_MAL);
    EXPECT_NE(g_tabelas.Feitico(id).acao().alinhamento_bem_mal(), DESC_LEAL);
  }
}

TEST(TesteTabelas, TesteFeiticoAleatorioMago) {
  Tabelas::DadosParaFeiticoAleatorio dfa;
  dfa.id_classe = "clerigo";
  dfa.nivel = 1;
  dfa.escolas_proibidas.emplace(std::vector<std::string>{"encantamento", "evocacao"});
  dfa.feiticos_excluidos = {"escudo_arcano", "armadura_arcana"};
  for (int i = 0; i < 100; ++i) {
    const auto& id = g_tabelas.FeiticoAleatorio(dfa);
    EXPECT_FALSE(id.empty());
    EXPECT_NE(id, "escudo_arcano");
    EXPECT_NE(id, "armadura_arcana");
    EXPECT_NE(g_tabelas.Feitico(id).escola(), "encantamento");
    EXPECT_NE(g_tabelas.Feitico(id).escola(), "evocacao");
  }
}

TEST(TesteAtaqueVsDefesa, TesteCritico) {
  DadosAtaque da_ataque;
  da_ataque.set_bonus_ataque_final(12);
  auto proto = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_ABENCOAR_ARMA);
  evento->add_complementos_str("machado_grande");
  auto orc = NovaEntidadeParaTestes(proto, g_tabelas);
  auto aguia_celestial = NovaEntidadeParaTestes(g_tabelas.ModeloEntidade("Águia Celestial").entidade(), g_tabelas);
  {
    g_dados_teste.push(20);
    ResultadoAtaqueVsDefesa resultado =
        AtaqueVsDefesa(0.0f, DadosAtaquePorGrupo("ataque_total_machado", orc->Proto()).acao(), *orc, *orc, Posicao::default_instance(), /*ataque_oportunidade=*/false);
    EXPECT_TRUE(resultado.Sucesso()) << resultado.texto;
    EXPECT_EQ(resultado.vezes, 3) << resultado.texto;
  }
  {
    // Este caso é interessante pq o 1 pegaria, entao cobre o caso de verificacao de 1 no critico.
    // Nao pega automatico por causa de alinhamento.
    g_dados_teste.push(20);
    g_dados_teste.push(1);
    ResultadoAtaqueVsDefesa resultado =
        AtaqueVsDefesa(0.0f, DadosAtaquePorGrupo("ataque_total_machado", orc->Proto()).acao(), *orc, *aguia_celestial, Posicao::default_instance(), /*ataque_oportunidade=*/false);
    EXPECT_TRUE(resultado.Sucesso()) << resultado.texto;
    EXPECT_EQ(resultado.vezes, 1) << resultado.texto;
  }
}

TEST(TesteAtaqueVsDefesa, TesteDesarmar) {
  DadosAtaque da_ataque;
  da_ataque.set_bonus_ataque_final(12);
  auto proto = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  auto* da = DadosAtaquePorGrupoOuCria("ataque_total_machado", &proto);
  da->set_ataque_desarmar(true);
  auto orca = NovaEntidadeParaTestes(proto, g_tabelas);
  auto orcd = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    // Defesa primeiro.
    g_dados_teste.push(10);
    g_dados_teste.push(10);
    // Contra ataques.
    g_dados_teste.push(10);
    g_dados_teste.push(10);
    ResultadoAtaqueVsDefesa resultado =
        AtaqueVsDefesa(0.0f, DadosAtaquePorGrupo("ataque_total_machado", orca->Proto()).acao(), *orca, *orcd, Posicao::default_instance(), /*ataque_oportunidade=*/false);
    EXPECT_FALSE(resultado.Sucesso()) << resultado.texto;
  }
  {
    // Defesa primeiro.
    g_dados_teste.push(9);
    g_dados_teste.push(10);
    ResultadoAtaqueVsDefesa resultado =
        AtaqueVsDefesa(0.0f, DadosAtaquePorGrupo("ataque_total_machado", orca->Proto()).acao(), *orca, *orcd, Posicao::default_instance(), /*ataque_oportunidade=*/false);
    EXPECT_TRUE(resultado.Sucesso()) << resultado.texto;
  }
}

TEST(TesteAtaqueVsDefesa, TesteDesarmar2) {
  DadosAtaque da_ataque;
  da_ataque.set_bonus_ataque_final(12);
  auto proto = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  auto* da = DadosAtaquePorGrupoOuCria("ataque_total_machado", &proto);
  da->set_ataque_desarmar(true);
  auto orca = NovaEntidadeParaTestes(proto, g_tabelas);

  proto.set_tamanho(TM_GRANDE);  // +4.
  auto* dam = DadosAtaquePorGrupoOuCria("mangual_pesado", &proto);  // +2.
  dam->set_id_arma("mangual_pesado");
  proto.set_ultimo_grupo_acao("mangual_pesado");
  auto orcd = NovaEntidadeParaTestes(proto, g_tabelas);

  {
    // Defesa primeiro. Nao tem foco em mangual.
    // +7 bab, +5 for, -1 tam, +4 duas mãos, +2 mangual = 17.
    g_dados_teste.push(10);
    // +7 bab, +5 for, +1 foco, +4 duas mãos, +1 magico, -4 dif tamanho = 14.
    g_dados_teste.push(13);
    // Contra ataques.
    g_dados_teste.push(14);  // um a mais por causa do desempate.
    g_dados_teste.push(10);
    ResultadoAtaqueVsDefesa resultado =
        AtaqueVsDefesa(0.0f, DadosAtaquePorGrupo("ataque_total_machado", orca->Proto()).acao(), *orca, *orcd, Posicao::default_instance(), /*ataque_oportunidade=*/false);
    EXPECT_FALSE(resultado.Sucesso()) << resultado.texto;
  }
  {
    g_dados_teste.push(10);
    g_dados_teste.push(14);
    ResultadoAtaqueVsDefesa resultado =
        AtaqueVsDefesa(0.0f, DadosAtaquePorGrupo("ataque_total_machado", orca->Proto()).acao(), *orca, *orcd, Posicao::default_instance(), /*ataque_oportunidade=*/false);
    EXPECT_TRUE(resultado.Sucesso()) << resultado.texto;
  }
}

TEST(TesteAtaqueVsDefesa, TesteDesarmarContraAtaque) {
  auto proto = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  auto* da = DadosAtaquePorGrupoOuCria("ataque_total_machado", &proto);
  da->set_ataque_desarmar(true);
  auto orca = NovaEntidadeParaTestes(proto, g_tabelas);
  auto orcd = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    // Defesa primeiro.
    g_dados_teste.push(10);
    g_dados_teste.push(10);
    // Contra ataques.
    g_dados_teste.push(10);
    g_dados_teste.push(11);
    ResultadoAtaqueVsDefesa resultado =
        AtaqueVsDefesa(0.0f, DadosAtaquePorGrupo("ataque_total_machado", orca->Proto()).acao(), *orca, *orcd, Posicao::default_instance(), /*ataque_oportunidade=*/false);
    EXPECT_FALSE(resultado.Sucesso()) << resultado.texto;
    EXPECT_EQ(resultado.resultado, RA_FALHA_CONTRA_ATAQUE);
  }

}

TEST(TesteAtaqueVsDefesa, TesteDerrubarSucesso) {
  auto proto = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  auto orca = NovaEntidadeParaTestes(proto, g_tabelas);
  auto orcd = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    // Defesa primeiro.
    g_dados_teste.push(10);
    g_dados_teste.push(11);
    ResultadoAtaqueVsDefesa resultado = AtaqueVsDefesaDerrubar(*orca, *orcd);
    EXPECT_TRUE(resultado.Sucesso()) << resultado.texto;
  }
}

TEST(TesteAtaqueVsDefesa, TesteDerrubarSucessoDerrubarAprimorado) {
  auto proto = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  proto.mutable_info_talentos()->add_outros()->set_id("derrubar_aprimorado");
  auto orca = NovaEntidadeParaTestes(proto, g_tabelas);
  auto orcd = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    // Defesa primeiro.
    g_dados_teste.push(10);
    g_dados_teste.push(7);
    ResultadoAtaqueVsDefesa resultado = AtaqueVsDefesaDerrubar(*orca, *orcd);
    EXPECT_TRUE(resultado.Sucesso()) << resultado.texto;
  }
}

TEST(TesteAtaqueVsDefesa, TesteDerrubarAprimoradoSemContraAtaqu) {
  auto proto = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  proto.mutable_info_talentos()->add_outros()->set_id("derrubar_aprimorado");
  auto orca = NovaEntidadeParaTestes(proto, g_tabelas);
  auto orcd = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    // Defesa primeiro.
    g_dados_teste.push(10);
    g_dados_teste.push(6);
    // Contra ataque.
    g_dados_teste.push(2);
    g_dados_teste.push(19);
    ResultadoAtaqueVsDefesa resultado = AtaqueVsDefesaDerrubar(*orca, *orcd);
    EXPECT_NE(resultado.resultado, RA_FALHA_CONTRA_ATAQUE) << resultado.texto;
  }
}

TEST(TesteAtaqueVsDefesa, TesteDerrubarComTamanho) {
  auto proto = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  auto orcd = NovaEntidadeParaTestes(proto, g_tabelas);
  proto.mutable_info_talentos()->add_outros()->set_id("derrubar_aprimorado");
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_AUMENTAR_PESSOA);
  evento->set_rodadas(1);
  auto orca = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    // Defesa primeiro.
    g_dados_teste.push(10);
    g_dados_teste.push(3);
    ResultadoAtaqueVsDefesa resultado = AtaqueVsDefesaDerrubar(*orca, *orcd);
    EXPECT_EQ(resultado.resultado, RA_SUCESSO) << resultado.texto;
  }
}

TEST(TesteAtaqueVsDefesa, TesteDerrubarFalhaNormal) {
  auto proto = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  auto orca = NovaEntidadeParaTestes(proto, g_tabelas);
  auto orcd = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    // Defesa primeiro.
    g_dados_teste.push(11);
    g_dados_teste.push(10);
    // Contra ataque.
    g_dados_teste.push(11);
    g_dados_teste.push(10);

    ResultadoAtaqueVsDefesa resultado = AtaqueVsDefesaDerrubar(*orca, *orcd);
    EXPECT_EQ(resultado.resultado, RA_FALHA_NORMAL) << resultado.texto;
  }
}

TEST(TesteAtaqueVsDefesa, TesteDerrubarFalhaContraAtaque) {
  auto proto = g_tabelas.ModeloEntidade("Orc Capitão").entidade();
  auto orca = NovaEntidadeParaTestes(proto, g_tabelas);
  auto orcd = NovaEntidadeParaTestes(proto, g_tabelas);
  {
    // Defesa primeiro.
    g_dados_teste.push(11);
    g_dados_teste.push(10);
    // Contra ataque.
    g_dados_teste.push(10);
    g_dados_teste.push(11);

    ResultadoAtaqueVsDefesa resultado = AtaqueVsDefesaDerrubar(*orca, *orcd);
    EXPECT_EQ(resultado.resultado, RA_FALHA_CONTRA_ATAQUE) << resultado.texto;
  }
}

TEST(TesteAtaqueVsDefesa, TesteDerrubarNaoPermiteContraAtaque) {
  auto orc = NovaEntidadeParaTestes(g_tabelas.ModeloEntidade("Orc Capitão").entidade(), g_tabelas);
  auto guepardo = NovaEntidadeParaTestes(g_tabelas.ModeloEntidade("Guepardo").entidade(), g_tabelas);
  {
    // Defesa primeiro.
    g_dados_teste.push(19);
    g_dados_teste.push(2);
    // Contra ataque. Nao sera usado.
    g_dados_teste.push(2);
    g_dados_teste.push(19);

    ResultadoAtaqueVsDefesa resultado = AtaqueVsDefesaDerrubar(*guepardo, *orc, guepardo->DadoCorrenteNaoNull().ataque_derrubar());
    EXPECT_NE(resultado.resultado, RA_FALHA_CONTRA_ATAQUE) << resultado.texto;
  }
}


}  // namespace ent.

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
