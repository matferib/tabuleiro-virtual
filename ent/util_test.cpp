#include <gtest/gtest.h>

#include <google/protobuf/text_format.h>
#include <queue>
#include "arq/arquivo.h"
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/recomputa.h"
#include "ent/tabelas.h"
#include "ent/util.h"
#include "log/log.h"
#include "ntf/notificacao.h"

namespace ent {

extern std::queue<int> g_dados_teste;
namespace {
Tabelas g_tabelas(nullptr);

class CentralColetora : public ntf::CentralNotificacoes {
 public:
  std::vector<std::unique_ptr<ntf::Notificacao>>& Notificacoes() { return notificacoes_; }
};

const DadosAtaque& DadosAtaquePorGrupo(const std::string& grupo, const EntidadeProto& proto) {
  for (const auto& da : proto.dados_ataque()) {
    if (da.grupo() == grupo) return da;
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

TEST(TesteArmas, TesteChicote) {
  auto modelo = g_tabelas.ModeloEntidade("Humano Plebeu 1");
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
  PreencheNotificacaoEventoEfeitoAdicional(proto.id(), nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
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
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
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
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_CEGO);
    EXPECT_EQ(evento.id_unico(), 0);
    EXPECT_EQ(evento.rodadas(), 20);
  }
  {
    proto_alvo.set_pontos_vida(101);
    std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto_alvo, g_tabelas));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    g_dados_teste.push(1);
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_CEGO);
    EXPECT_EQ(evento.id_unico(), 0);
    EXPECT_EQ(evento.rodadas(), 2);
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
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_ATORDOADO);
    EXPECT_EQ(evento.id_unico(), 0);
    EXPECT_EQ(evento.rodadas(), 4);
  }
  {
    proto_alvo.set_pontos_vida(51);
    std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto_alvo, g_tabelas));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    g_dados_teste.push(1);
    g_dados_teste.push(1);
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_ATORDOADO);
    EXPECT_EQ(evento.id_unico(), 0);
    EXPECT_EQ(evento.rodadas(), 2);
  }
  {
    proto_alvo.set_pontos_vida(101);
    std::unique_ptr<Entidade> alvo(NovaEntidadeParaTestes(proto_alvo, g_tabelas));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    g_dados_teste.push(1);
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_ATORDOADO);
    EXPECT_EQ(evento.id_unico(), 0);
    EXPECT_EQ(evento.rodadas(), 1);
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
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_PODER_DIVINO);
    alvo->AtualizaParcial(n.entidade());
    EXPECT_EQ(BonusTotal(BonusAtributo(TA_FORCA, alvo->Proto())), 16);
    EXPECT_EQ(alvo->Proto().bba().base(), 15);
    EXPECT_EQ(BonusTotal(alvo->Proto().pontos_vida_temporarios_por_fonte()), 10);
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
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_MORTE);
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
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
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
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
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
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
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
  EXPECT_EQ(da.alcance_q(), 2);
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
  {
    proto_ataque.set_tamanho(TM_GRANDE);
    proto_ataque.set_gerar_agarrar(false);
    proto_ataque.mutable_tesouro()->add_itens_mundanos()->set_id("fogo_alquimico");
    proto_ataque.mutable_tesouro()->add_itens_mundanos()->set_id("fogo_alquimico");
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
  // Consome 1.
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
    ASSERT_TRUE(e->Proto().dados_ataque().empty());
  }
}

TEST(TesteCA, TesteLutaDefensiva) {
  std::unique_ptr<Entidade> e;
  {
    EntidadeProto proto;
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("fogo_alquimico");
    e.reset(NovaEntidadeParaTestes(proto, g_tabelas));
  }

  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
  auto n = PreencheNotificacaoLutarDefensivamente(true, e->Proto());
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
    e.reset(NovaEntidadeParaTestes(proto, g_tabelas));
  }

  EXPECT_EQ(BonusTotal(e->Proto().dados_defesa().ca()), 10);
  auto n = PreencheNotificacaoLutarDefensivamente(true, e->Proto());
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
    e.reset(NovaEntidadeParaTestes(proto, g_tabelas));
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
    e.reset(NovaEntidadeParaTestes(proto, g_tabelas));
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
  EXPECT_EQ(CATotal(proto, /*escudo=*/true, Bonus()), 17);
  EXPECT_EQ(CASurpreso(proto, /*escudo=*/true, Bonus()), 17);

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
  EXPECT_FALSE(PodeLancarPergaminho(g_tabelas, proto, proto.dados_ataque(0)).first);
  // Fora da lista.
  EXPECT_FALSE(PodeLancarPergaminho(g_tabelas, proto, proto.dados_ataque(1)).first);
  // Atributo invalido.
  EXPECT_FALSE(PodeLancarPergaminho(g_tabelas, proto, proto.dados_ataque(2)).first);

  EXPECT_TRUE(PodeLancarPergaminho(g_tabelas, proto, proto.dados_ataque(3)).first);
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
  EXPECT_TRUE(PodeLancarPergaminho(g_tabelas, proto, proto.dados_ataque(0)).first);
  EXPECT_FALSE(PodeLancarPergaminho(g_tabelas, proto, proto.dados_ataque(1)).first);
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
  EXPECT_TRUE(PodeLancarPergaminho(g_tabelas, proto, proto.dados_ataque(0)).first);
  EXPECT_FALSE(PodeLancarPergaminho(g_tabelas, proto, proto.dados_ataque(1)).first);
  EXPECT_FALSE(PodeLancarPergaminho(g_tabelas, proto, proto.dados_ataque(2)).first);
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

TEST(TesteTalentoPericias, TesteTabeladoTalentos) {
  for (const auto& modelo : g_tabelas.TodosModelosEntidades().modelo()) {
    EntidadeProto proto = modelo.entidade();
    const int antes = proto.info_talentos().gerais().size();
    RecomputaDependencias(g_tabelas, &proto);
    const int depois = proto.info_talentos().gerais().size();
    EXPECT_EQ(antes, depois) << "falhou para: " << modelo.id();
  }
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
  int tamanho = proto.ByteSize();
  for (int i = 0; i < 100; ++i) {
    RecomputaDependencias(g_tabelas, &proto);
    EXPECT_EQ(tamanho, proto.ByteSize()) << ", iteração: " << i;
    break;
  }
}

TEST(TesteVezes, TesteVezes) {
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
  PreencheNotificacaoAtaqueAoPassarRodada(e->Proto(), grupo.get(), grupo_desfazer.get());
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
  PreencheNotificacaoAtaqueAoPassarRodada(e->Proto(), grupo.get(), grupo_desfazer.get());

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
  auto* ic = proto.add_info_classes();
  ic->set_id("ladino");
  ic->set_nivel(1);
  AtribuiBaseAtributo(12, TA_FORCA, &proto);
  AtribuiBaseAtributo(14, TA_DESTREZA, &proto);
  proto.mutable_info_talentos()->add_gerais()->set_id("maos_leves");
  auto* p = PericiaCriando("usar_cordas", &proto);
  p->set_pontos(5);
  RecomputaDependencias(g_tabelas, &proto);

  // 2 des, 2 talento, 5 pontos de classe.
  EXPECT_EQ(9, BonusTotal(p->bonus()));
  // 1 forca, 2 sinergia.
  EXPECT_EQ(3, BonusTotal(Pericia("escalar", proto).bonus()));

  // Pericia deixa de ser de classe.
  ic->set_id("guerreiro");
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(6, BonusTotal(p->bonus()));
  // Sem sinergia.
  EXPECT_EQ(1, BonusTotal(Pericia("escalar", proto).bonus()));
}

TEST(TesteFormaAlternativa, TesteFormaAlternativa) {
  EntidadeProto proto;
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
    EXPECT_EQ(17, BonusTotal(proto_pos_forma.dados_defesa().ca()));
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
  EXPECT_EQ(resultado.delta_pv, 0) << resultado.texto;

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
  // Ataques.
  {
    auto* da = proto.add_dados_ataque();
    da->set_id_arma("espada_longa");
  }
  // Eventos.
  {
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_INVESTIDA);
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
    da->set_id_arma("espada_longa");
    da->set_obra_prima(true);
    da->set_empunhadura(EA_2_MAOS);
  }
  {
    auto* da = proto.add_dados_ataque();
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
  EXPECT_EQ("espada longa", proto.dados_ataque(0).rotulo());
  // Espada longa grande com duas maos.
  EXPECT_EQ("2d6+4", proto.dados_ataque(0).dano());
  EXPECT_EQ(19, proto.dados_ataque(0).margem_critico());
  // 3 base, 3 forca, -1 tamanho, 1 obra prima.
  EXPECT_EQ(6, proto.dados_ataque(0).bonus_ataque_final());
  EXPECT_EQ(16, proto.dados_ataque(0).ca_normal());
  EXPECT_EQ(16, proto.dados_ataque(0).ca_surpreso());  // esquiva sobrenatural
  EXPECT_EQ(9, proto.dados_ataque(0).ca_toque());
  // Segundo ataque.
  // Espada longa grande com uma mao.
  EXPECT_EQ("2d6+3", proto.dados_ataque(1).dano());
  EXPECT_EQ(21, proto.dados_ataque(1).ca_normal());
  EXPECT_EQ(21, proto.dados_ataque(1).ca_surpreso());  // esquiva sobrenatural.
  EXPECT_EQ(9, proto.dados_ataque(1).ca_toque());

  EXPECT_GE(proto.tendencia().eixo_bem_mal(), 0.6f);

  auto* ea = NovaEntidadeParaTestes(proto, g_tabelas);
  auto* ed = NovaEntidadeParaTestes(proto, g_tabelas);
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

  proto.mutable_info_talentos()->add_gerais()->set_id("fortitude_maior");

  RecomputaDependencias(g_tabelas, &proto);
  // 3 + 4 con + 2 fortitude maior;
  EXPECT_EQ(9, BonusTotal(proto.dados_defesa().salvacao_fortitude()));
  // 1 + 3 destreza.
  EXPECT_EQ(4, BonusTotal(proto.dados_defesa().salvacao_reflexo()));
  // 1 de vontade, +2 bonus.
  EXPECT_EQ(3, BonusTotal(proto.dados_defesa().salvacao_vontade()));

  // Adiciona reflexos rapidos.
  proto.mutable_info_talentos()->add_gerais()->set_id("reflexos_rapidos");
  RecomputaDependencias(g_tabelas, &proto);
  // 3 + 3 con + 2 fortitude maior;
  EXPECT_EQ(9, BonusTotal(proto.dados_defesa().salvacao_fortitude()));
  // 1 + 3 destreza + 2 reflexos rapidos.
  EXPECT_EQ(6, BonusTotal(proto.dados_defesa().salvacao_reflexo()));
  // 1 de vontade, +2 bonus.
  EXPECT_EQ(3, BonusTotal(proto.dados_defesa().salvacao_vontade()));

  // Adiciona vontade de ferro.
  proto.mutable_info_talentos()->add_outros()->set_id("vontade_ferro");
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
  proto.mutable_info_talentos()->mutable_gerais()->DeleteSubrange(1, 1);
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
  auto* ev = AdicionaEvento(/*origem*/"virtude", EFEITO_VIRTUDE, /*rodadas=*/10, /*continuo=*/false, &ids_unicos, &proto);
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
  auto* ev = AdicionaEvento(/*origem*/"ajuda", EFEITO_AJUDA, 10, false, &ids_unicos, &proto);
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
  auto* ev = AdicionaEvento(/*origem*/"ajuda", EFEITO_AJUDA, 10, false, &ids_unicos, &proto);
  ev->add_complementos(5);
  ev = AdicionaEvento(/*origem*/"ajuda", EFEITO_AJUDA, 10, false, &ids_unicos, &proto);
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
  auto* ev = AdicionaEvento(/*origem*/"ajuda", EFEITO_AJUDA, 10, false, &ids_unicos, &proto);
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

TEST(TesteModificadorAtaque, TesteModificadorAtaque) {
  {
    EntidadeProto ea;
    EntidadeProto ed;
    ed.set_caida(true);

    EXPECT_EQ(ModificadorAtaque(TipoAtaque::CORPO_A_CORPO, ea, ed), 4);
    EXPECT_EQ(ModificadorAtaque(TipoAtaque::DISTANCIA, ea, ed), -4);
  }
  {
    EntidadeProto ea;
    EntidadeProto ed;
    ed.set_em_corpo_a_corpo(true);

    EXPECT_EQ(ModificadorAtaque(TipoAtaque::CORPO_A_CORPO, ea, ed), 0);
    EXPECT_EQ(ModificadorAtaque(TipoAtaque::DISTANCIA, ea, ed), -4);
  }
  {
    EntidadeProto ea;
    auto* tp = ea.mutable_info_talentos()->add_outros();
    tp->set_id("tiro_preciso");
    EntidadeProto ed;
    ed.set_em_corpo_a_corpo(true);

    EXPECT_EQ(ModificadorAtaque(TipoAtaque::CORPO_A_CORPO, ea, ed), 0);
    EXPECT_EQ(ModificadorAtaque(TipoAtaque::DISTANCIA, ea, ed), 0);
  }
  {
    EntidadeProto ea;
    EntidadeProto ed;

    EXPECT_EQ(ModificadorAtaque(TipoAtaque::AGARRAR, ea, ed), 0);
    ea.mutable_info_talentos()->add_gerais()->set_id("agarrar_aprimorado");
    EXPECT_EQ(ModificadorAtaque(TipoAtaque::AGARRAR, ea, ed), 4);
    ed.mutable_info_talentos()->add_gerais()->set_id("agarrar_aprimorado");
    EXPECT_EQ(ModificadorAtaque(TipoAtaque::AGARRAR, ea, ed), 0);
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

TEST(TesteSalvacaoDinamica, TesteRodadasDinamico) {
  ntf::Notificacao n;
  EntidadeProto proto;
  std::unique_ptr<Entidade> e(NovaEntidadeParaTestes(proto, g_tabelas));
  std::vector<int> ids_unicos = IdsUnicosEntidade(*e);
  PreencheNotificacaoEventoEfeitoAdicional(
      proto.id(), /*nivel*/3, *e, g_tabelas.Feitico("sono").acao().efeitos_adicionais(0), &ids_unicos, &n, nullptr);
  ASSERT_FALSE(n.entidade().evento().empty());
  EXPECT_EQ(n.entidade().evento(0).rodadas(), 30);
  ASSERT_EQ(ids_unicos.size(), 1ULL);
  EXPECT_EQ(n.entidade().evento(0).id_unico(), ids_unicos[0]);
}

TEST(TesteSalvacaoDinamica, TesteRodadasDinamicoDependenciaAnterior) {
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

// Este teste simula mais ou menos a forma como os efeitos adicionais de feiticos sao aplicados.
TEST(TesteSalvacaoDinamica, TesteEfeitosAdicionaisMultiplos) {
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(3);
    std::unique_ptr<Entidade> e(NovaEntidadeParaTestes(proto, g_tabelas));
    ntf::Notificacao n;
    std::vector<int> ids_unicos = IdsUnicosEntidade(*e);
    PreencheNotificacaoEventoEfeitoAdicional(
        proto.id(), /*nivel*/3, *e, g_tabelas.Feitico("teia").acao().efeitos_adicionais(0), &ids_unicos, n.add_notificacao(), nullptr);
    PreencheNotificacaoEventoEfeitoAdicional(
        proto.id(), /*nivel*/ 3, *e,
        g_tabelas.Feitico("teia").acao().efeitos_adicionais(1), &ids_unicos,
        n.add_notificacao(), nullptr);
    e->AtualizaParcial(n.notificacao(0).entidade());
    e->AtualizaParcial(n.notificacao(1).entidade());
    ASSERT_EQ(e->Proto().evento().size(), 2);
    EXPECT_EQ(e->Proto().evento(0).id_efeito(), EFEITO_ENREDADO);
    EXPECT_EQ(e->Proto().evento(1).id_efeito(), EFEITO_OUTRO);
    ASSERT_EQ(ids_unicos.size(), 2ULL);
    EXPECT_EQ(ids_unicos[0], 0);
    EXPECT_EQ(ids_unicos[1], 1);
  }
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
    referencia.reset(NovaEntidadeParaTestes(proto, g_tabelas));
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
    referencia.reset(NovaEntidadeParaTestes(proto, g_tabelas));
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
    AtribuiBaseAtributo(12, TA_FORCA, &proto);
    AtribuiBaseAtributo(16, TA_DESTREZA, &proto);
    AtribuiBaseAtributo(14, TA_SABEDORIA, &proto);
    auto* da = proto.add_dados_ataque();
    da->set_tipo_ataque("Feitiço de Clérigo");
    da->set_id_arma("oracao");
    da = proto.add_dados_ataque();
    da->set_id_arma("espada_curta");

    proto.mutable_tesouro()->add_itens_mundanos()->set_id("pedra_trovao");
    referencia.reset(NovaEntidadeParaTestes(proto, g_tabelas));
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
    ASSERT_EQ(proto.feiticos_classes(0).feiticos_por_nivel().size(), 3);
    // Nivel 0: Fixo em 4.
    ASSERT_EQ(proto.feiticos_classes(0).feiticos_por_nivel(0).para_lancar().size(), 4);
    // Nivel 1: 2 + 1 de bonus de atributo.
    ASSERT_EQ(proto.feiticos_classes(0).feiticos_por_nivel(1).para_lancar().size(), 3);
    // Nivel 2: 1 + 1 de bonus de atributo.
    ASSERT_EQ(proto.feiticos_classes(0).feiticos_por_nivel(2).para_lancar().size(), 2);
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
    ASSERT_EQ(proto.feiticos_classes(0).feiticos_por_nivel().size(), 2);
    // Nivel 0: Fixo em 3.
    ASSERT_EQ(proto.feiticos_classes(0).feiticos_por_nivel(0).para_lancar().size(), 3);
    // Nivel 1: 1 + 1 dominio + 1 de bonus de atributo.
    ASSERT_EQ(proto.feiticos_classes(0).feiticos_por_nivel(1).para_lancar().size(), 3);
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
    PreencheNotificacaoEventoEfeitoAdicional(proto.id(), nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_PROTEGER_OUTRO);
    ASSERT_FALSE(evento.complementos().empty());
    EXPECT_EQ(evento.complementos(0), 123);
  }

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
  ASSERT_EQ(acao.parametros_lancamento().parametros().size(), 4);
  ASSERT_EQ(acao.parametros_lancamento().parametros(2).id_modelo_entidade(), "Arma Espiritual Espada");

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
  celestial->set_id_efeito(EFEITO_MODELO_CELESTIAL);
  celestial->add_complementos(6);  // RM
  celestial->add_complementos(7);  // frio acido eletricidade
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(proto.dados_defesa().resistencia_magia(), 6);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos().size(), 4);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(0).valor(), 10);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(0).descritor(), DESC_ACIDO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).valor(), 7);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).descritor(), DESC_FRIO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).id_efeito_modelo(), EFEITO_MODELO_CELESTIAL);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).valor(), 7);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).descritor(), DESC_ACIDO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).id_efeito_modelo(), EFEITO_MODELO_CELESTIAL);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).valor(), 7);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).descritor(), DESC_ELETRICIDADE);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).id_efeito_modelo(), EFEITO_MODELO_CELESTIAL);
  // De novo pra ver se num ta quebrando nada.
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(proto.dados_defesa().resistencia_magia(), 6);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos().size(), 4);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(0).valor(), 10);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(0).descritor(), DESC_ACIDO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).valor(), 7);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).descritor(), DESC_FRIO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).valor(), 7);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).descritor(), DESC_ACIDO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).valor(), 7);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).descritor(), DESC_ELETRICIDADE);
}

TEST(TesteImunidades, TesteAbissal) {
  EntidadeProto proto;
  {
    auto* ic = proto.add_info_classes();
    ic->set_id("druida");
    ic->set_nivel(1);
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
  celestial->set_id_efeito(EFEITO_MODELO_ABISSAL);
  celestial->add_complementos(6);  // RM
  celestial->add_complementos(7);  // frio fogo 
  celestial->add_complementos(10);  // reducao dano.
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(proto.dados_defesa().resistencia_magia(), 6);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos().size(), 4);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(0).valor(), 10);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(0).descritor(), DESC_ACIDO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).valor(), 11);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).descritor(), DESC_FOGO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).valor(), 7);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).descritor(), DESC_FRIO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).id_efeito_modelo(), EFEITO_MODELO_ABISSAL);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).valor(), 7);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).descritor(), DESC_FOGO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).id_efeito_modelo(), EFEITO_MODELO_ABISSAL);
  ASSERT_FALSE(proto.dados_defesa().reducao_dano().empty());
  ASSERT_EQ(proto.dados_defesa().reducao_dano(0).id_efeito_modelo(), EFEITO_MODELO_ABISSAL);
  ASSERT_TRUE(c_any(proto.dados_defesa().reducao_dano(0).descritores(), DESC_MAGICO));
  // De novo pra ver se num ta quebrando nada.
  RecomputaDependencias(g_tabelas, &proto);
  EXPECT_EQ(proto.dados_defesa().resistencia_magia(), 6);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos().size(), 4);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(0).valor(), 10);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(0).descritor(), DESC_ACIDO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).valor(), 11);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(1).descritor(), DESC_FOGO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).valor(), 7);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).descritor(), DESC_FRIO);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(2).id_efeito_modelo(), EFEITO_MODELO_ABISSAL);
  ASSERT_EQ(proto.dados_defesa().resistencia_elementos(3).valor(), 7);
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

TEST(TesteModelo, TestePlebeu1) {
  auto modelo = g_tabelas.ModeloEntidade("Humano Plebeu 1");
  auto plebeu = NovaEntidadeParaTestes(modelo.entidade(), g_tabelas);
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


  const auto* da = druida->DadoAtaque("relampago", 0);
  ASSERT_NE(da, nullptr);
  // Nao pode aplicar modificador de tiro certeiro.
  EXPECT_EQ(StringDanoParaAcao(*da, druida->Proto(), druida->Proto()), "3d6");
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
  vulto->add_complementos(13);
  vulto->set_ativo(true);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_GE(proto.dados_ataque().size(), 1);
  EXPECT_EQ(proto.dados_ataque(0).bonus_ataque_final(), 2);
  EXPECT_EQ(proto.dados_defesa().resistencia_magia(), 13);

  vulto->set_ativo(false);
  RecomputaDependencias(g_tabelas, &proto);
  ASSERT_GE(proto.dados_ataque().size(), 1);
  EXPECT_EQ(proto.dados_ataque(0).bonus_ataque_final(), 0);
  EXPECT_EQ(proto.dados_defesa().resistencia_magia(), 0);
}

TEST(TesteComposicaoEntidade, TesteCachorroCelestial) {
  const auto& modelo_cc = g_tabelas.ModeloEntidade("Cachorro Celestial");
  EntidadeProto proto = modelo_cc.entidade();
  RecomputaDependencias(g_tabelas, &proto);

  EXPECT_EQ(BonusTotal(BonusAtributo(TA_INTELIGENCIA, proto)), 3);
  EXPECT_EQ(Nivel(proto), 1);
  ASSERT_EQ(proto.info_classes().size(), 1);
  EXPECT_EQ(proto.info_classes(0).id(), "besta_magica");
  EXPECT_EQ(proto.info_classes(0).nivel(), 1);
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
      proto.id(), NivelConjuradorParaAcao(acao, *entidade), *entidade, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
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
    PreencheNotificacaoReducaoLuzComConsequencia(NivelConjuradorParaAcao(acao, *entidade), *alvo, &acao, &n, nullptr);
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
    PreencheNotificacaoReducaoLuzComConsequencia(NivelConjuradorParaAcao(acao, *entidade), *alvo, &acao, &n, nullptr);
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
  EXPECT_EQ(proto.dados_ataque(0).bonus_ataque_final(), 5) << proto.dados_ataque(0).DebugString();
  EXPECT_EQ(proto.dados_ataque(0).dano(), "1d8+2");
  EXPECT_EQ(proto.dados_ataque(0).dificuldade_salvacao(), 15);
  // Kama +1: derrubar, +3 ataque, +2 força, +1 arma, +1 foco em arma, -1 rajada. Dano: +2 de força, +1 arma.
  EXPECT_EQ(proto.dados_ataque(1).bonus_ataque_final(), 6) << proto.dados_ataque(0).DebugString();
  EXPECT_TRUE(proto.dados_ataque(1).dano().empty());
  EXPECT_TRUE(proto.dados_ataque(1).ataque_derrubar());
  // Desarmado: +3 ataque, +2 força, -1 rajada. Dano: +2 de força.
  EXPECT_EQ(proto.dados_ataque(3).bonus_ataque_final(), 4) << proto.dados_ataque(2).DebugString();
  EXPECT_EQ(proto.dados_ataque(3).dano(), "1d8+2") << proto.dados_ataque(2).DebugString();
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
    da->set_id_arma("azagaia");
    da->set_empunhadura(EA_ARMA_ESCUDO);
  }
  RecomputaDependencias(g_tabelas, &proto);

  EXPECT_EQ(-1, proto.dados_ataque(0).bonus_ataque_final());
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
  PreencheNotificacoesTransicaoTesouro(g_tabelas, *doador, *receptor, &n_grupo, nullptr);

  {
    // Aplica.
    ASSERT_GE(n_grupo.notificacao_size(), 2);
    doador->AtualizaParcial(n_grupo.notificacao(0).entidade());
    EXPECT_TRUE(doador->Proto().evento().empty());
    EXPECT_TRUE(doador->Proto().tesouro().aneis().empty());
    EXPECT_TRUE(doador->Proto().tesouro().armaduras().empty());
    EXPECT_TRUE(doador->Proto().tesouro().armas().empty());
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

TEST(TesteRacas, TesteAnao) {
  EntidadeProto proto;
  AtribuiBaseAtributo(15, TA_CONSTITUICAO, &proto);  // com bonus de anao, vai para 17.
  AtribuiBaseAtributo(10, TA_CARISMA, &proto);  // com penalidade de anao, vai para 8.
  proto.set_raca("anao");
  {
    auto* ic = proto.add_info_classes();
    ic->set_nivel(1);
    ic->set_id("guerreiro");
  }
  std::unique_ptr<Entidade> ed(NovaEntidadeParaTestes(proto, g_tabelas));

  std::unique_ptr<Entidade> ea;
  {
    EntidadeProto proto;
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

    ea.reset(NovaEntidadeParaTestes(proto, g_tabelas));
    ASSERT_GE(ea->Proto().dados_ataque_size(), 3);
  }

  EXPECT_EQ(BonusTotal(BonusAtributo(TA_CONSTITUICAO, ed->Proto())), 17);
  EXPECT_EQ(BonusTotal(BonusAtributo(TA_CARISMA, ed->Proto())), 8);
  EXPECT_EQ(ed->Salvacao(*ea, TS_FORTITUDE), 5);  // 2 + 3.
  EXPECT_EQ(ed->SalvacaoVeneno(), 7);  // 2 + 3 + 2.
  EXPECT_EQ(ed->SalvacaoFeitico(*ea, TS_REFLEXO), 2);  // 0 + 0 + 2.

  // Testa o ataque nao feitico, bonus 5 vs CD 16.
  g_dados_teste.push(10);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(0), *ea, *ed)), false) << "cd: " << ea->Proto().dados_ataque(0).dificuldade_salvacao();
  g_dados_teste.push(11);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(0), *ea, *ed)), true) << "cd: " << ea->Proto().dados_ataque(0).dificuldade_salvacao();
  // Testa o ataque feitico, bonus 2 vs CD 16.
  g_dados_teste.push(13);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(1), *ea, *ed)), false)
      << "bonus: " << ed->SalvacaoFeitico(*ea, TS_REFLEXO)
      << ", cd: " << ea->Proto().dados_ataque(1).dificuldade_salvacao();
  g_dados_teste.push(14);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(1), *ea, *ed)), true)
      << "bonus: " << ed->SalvacaoFeitico(*ea, TS_REFLEXO)
      << ", cd: " << ea->Proto().dados_ataque(1).dificuldade_salvacao();
  // Testa o pergaminho feitico, bonus 2 vs CD 13.
  g_dados_teste.push(10);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(2), *ea, *ed)), false)
      << "bonus: " << ed->SalvacaoFeitico(*ea, TS_REFLEXO)
      << ", cd: " << ea->Proto().dados_ataque(2).dificuldade_salvacao();
  g_dados_teste.push(11);
  EXPECT_EQ(std::get<1>(AtaqueVsSalvacao(0, ea->Proto().dados_ataque(2), *ea, *ed)), true)
      << "bonus: " << ed->SalvacaoFeitico(*ea, TS_REFLEXO)
      << ", cd: " << ea->Proto().dados_ataque(2).dificuldade_salvacao();
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
  int c = 0;
  for (const auto& pericia : proto.info_pericias()) {
    if (pericia.id() == "observar") {
      EXPECT_EQ(BonusTotal(pericia.bonus()), 16) << "pericia: " << pericia.DebugString();
      ++c;
    }
  }
  ASSERT_EQ(c, 1);
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
    auto [delta, texto] = RenovaSeTiverDominioRenovar(e->Proto(), -5, TD_LETAL, &n, nullptr);
    EXPECT_FALSE(n.has_tipo());
  }
  {
    // Ativa renovacao.
    g_dados_teste.push(5); // d8 + 2 = 7
    ntf::Notificacao n;
    auto [delta, texto] = RenovaSeTiverDominioRenovar(e->Proto(), -6, TD_LETAL, &n, nullptr);
    EXPECT_TRUE(n.has_tipo());
    EXPECT_EQ(delta, 1) << ", texto: " << texto;  // mudou de -6 para 1 por causa da renovacao.
    EXPECT_FALSE(texto.empty());
    e->AtualizaParcial(n.entidade());
    EXPECT_EQ(PoderesDoDominio("renovacao", e->Proto()).usado(), true);
    EXPECT_EQ(PoderesDoDominio("renovacao", e->Proto()).disponivel_em(), DIA_EM_RODADAS);
  }
  {
    ntf::Notificacao n;
    auto [delta, texto] = RenovaSeTiverDominioRenovar(e->Proto(), -6, TD_LETAL, &n, nullptr);
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

}  // namespace ent.

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
