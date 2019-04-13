#include <gtest/gtest.h>

#include <google/protobuf/text_format.h>
#include <queue>
#include "arq/arquivo.h"
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/tabelas.h"
#include "ent/util.h"
#include "log/log.h"

namespace ent {

extern std::queue<int> g_dados_teste;

TEST(TesteBonus, TesteBonusCumulativo) {
  Tabelas tabelas(nullptr);
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
  Tabelas tabelas(nullptr);
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
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* anel = proto.mutable_tesouro()->add_aneis();
  anel->set_id("protecao_1");
  anel->set_em_uso(true);
  RecomputaDependencias(tabelas, &proto);
  ASSERT_FALSE(proto.evento().empty());
  const auto& evento = proto.evento(0);
  EXPECT_EQ(evento.id_efeito(), EFEITO_DEFLEXAO_CA);
  EXPECT_EQ(evento.id_unico(), 0);
  EXPECT_TRUE(evento.continuo());
  EXPECT_TRUE(evento.requer_pai());

  proto.mutable_tesouro()->clear_aneis();
  RecomputaDependencias(tabelas, &proto);
  ASSERT_TRUE(proto.evento().empty());
}

TEST(TesteArmas, TestePedrada) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Pedrada (gigante)");
  RecomputaDependencias(tabelas, &proto);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_arremesso()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_PROJETIL) << "acao: " << acao.DebugString();
}

TEST(TesteArmas, TesteFunda) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* da = proto.add_dados_ataque();
  da->set_id_arma("funda");
  RecomputaDependencias(tabelas, &proto);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_arremesso()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_PROJETIL) << "acao: " << acao.DebugString();
}

TEST(TesteArmas, TesteEspadaLaminaAfiada) {
  Tabelas tabelas(nullptr);
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
  RecomputaDependencias(tabelas, &proto);
  ASSERT_GE(proto.dados_ataque().size(), 2);
  EXPECT_EQ(proto.dados_ataque(0).margem_critico(), 19);

  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_LAMINA_AFIADA);
  evento->set_rodadas(1);
  evento->add_complementos_str("espada longa 1");
  RecomputaDependencias(tabelas, &proto);
  ASSERT_GE(proto.dados_ataque().size(), 2);
  EXPECT_EQ(proto.dados_ataque(0).margem_critico(), 17);
  EXPECT_EQ(proto.dados_ataque(1).margem_critico(), 19);
}

TEST(TesteArmas, TesteMetamorfoseTorrida) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(20);
  AtribuiBaseAtributo(20, TA_INTELIGENCIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Mago");
  da->set_id_arma("metamorfose_torrida");
  RecomputaDependencias(tabelas, &proto);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->alcance_m(), 15 * QUADRADOS_PARA_METROS) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  EXPECT_FALSE(da->acao().permite_ataque_vs_defesa()) << "DA completo: " << da->DebugString();
  const AcaoProto& acao = da->acao();
  const int nivel_conjurador = NivelConjurador(TipoAtaqueParaClasse(tabelas, da->tipo_ataque()), proto);
  ASSERT_FALSE(acao.efeitos_adicionais().empty());
  EXPECT_TRUE(da->acao().permite_salvacao()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->acao().ignora_resistencia_magia()) << "DA completo: " << da->DebugString();

  EntidadeProto proto_alvo;
  std::unique_ptr<Entidade> alvo(NovaEntidade(proto_alvo, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
  std::vector<int> ids_unicos;
  ntf::Notificacao n;
  PreencheNotificacaoEventoEfeitoAdicional(nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
  alvo->AtualizaParcial(n.entidade());
  ASSERT_FALSE(alvo->Proto().evento().empty());
  const auto& evento = alvo->Proto().evento(0);
  EXPECT_EQ(evento.id_efeito(), EFEITO_OUTRO);
  EXPECT_EQ(evento.id_unico(), 0);
  EXPECT_TRUE(evento.continuo());
  EXPECT_EQ(evento.descricao(), "virou sapo");
}

TEST(TesteArmas, TestePalavraDoPoderCegar) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(20);
  AtribuiBaseAtributo(20, TA_INTELIGENCIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Mago");
  da->set_id_arma("palavra_poder_cegar");
  RecomputaDependencias(tabelas, &proto);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->alcance_m(), 15 * QUADRADOS_PARA_METROS) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  EXPECT_FALSE(da->acao().permite_ataque_vs_defesa()) << "DA completo: " << da->DebugString();
  const AcaoProto& acao = da->acao();
  const int nivel_conjurador = NivelConjurador(TipoAtaqueParaClasse(tabelas, da->tipo_ataque()), proto);
  ASSERT_FALSE(acao.efeitos_adicionais().empty());
  EXPECT_FALSE(da->acao().permite_salvacao()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->acao().ignora_resistencia_magia()) << "DA completo: " << da->DebugString();

  EntidadeProto proto_alvo;
  proto_alvo.set_max_pontos_vida(201);
  {
    proto_alvo.set_pontos_vida(50);
    std::unique_ptr<Entidade> alvo(NovaEntidade(proto_alvo, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_CEGO);
    EXPECT_EQ(evento.id_unico(), 0);
    EXPECT_TRUE(evento.continuo());
  }
  {
    proto_alvo.set_pontos_vida(51);
    std::unique_ptr<Entidade> alvo(NovaEntidade(proto_alvo, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    g_dados_teste.push(1);
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_CEGO);
    EXPECT_EQ(evento.id_unico(), 0);
    EXPECT_EQ(evento.rodadas(), 20);
  }
  {
    proto_alvo.set_pontos_vida(101);
    std::unique_ptr<Entidade> alvo(NovaEntidade(proto_alvo, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    g_dados_teste.push(1);
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_CEGO);
    EXPECT_EQ(evento.id_unico(), 0);
    EXPECT_EQ(evento.rodadas(), 2);
  }
  {
    proto_alvo.set_pontos_vida(201);
    std::unique_ptr<Entidade> alvo(NovaEntidade(proto_alvo, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
    EXPECT_FALSE(AcaoAfetaAlvo(acao, *alvo)) << "acao: " << acao.DebugString();
  }
}

TEST(TesteArmas, TestePalavraDoPoderAtordoar) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(20);
  AtribuiBaseAtributo(20, TA_INTELIGENCIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Mago");
  da->set_id_arma("palavra_poder_atordoar");
  RecomputaDependencias(tabelas, &proto);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->alcance_m(), 15 * QUADRADOS_PARA_METROS) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  EXPECT_FALSE(da->acao().permite_ataque_vs_defesa()) << "DA completo: " << da->DebugString();
  const AcaoProto& acao = da->acao();
  const int nivel_conjurador = NivelConjurador(TipoAtaqueParaClasse(tabelas, da->tipo_ataque()), proto);
  ASSERT_FALSE(acao.efeitos_adicionais().empty());
  EXPECT_FALSE(da->acao().permite_salvacao()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->acao().ignora_resistencia_magia()) << "DA completo: " << da->DebugString();

  EntidadeProto proto_alvo;
  proto_alvo.set_max_pontos_vida(200);
  {
    proto_alvo.set_pontos_vida(50);
    std::unique_ptr<Entidade> alvo(NovaEntidade(proto_alvo, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    g_dados_teste.push(1);
    g_dados_teste.push(1);
    g_dados_teste.push(1);
    g_dados_teste.push(1);
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_ATORDOADO);
    EXPECT_EQ(evento.id_unico(), 0);
    EXPECT_EQ(evento.rodadas(), 4);
  }
  {
    proto_alvo.set_pontos_vida(51);
    std::unique_ptr<Entidade> alvo(NovaEntidade(proto_alvo, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    g_dados_teste.push(1);
    g_dados_teste.push(1);
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_ATORDOADO);
    EXPECT_EQ(evento.id_unico(), 0);
    EXPECT_EQ(evento.rodadas(), 2);
  }
  {
    proto_alvo.set_pontos_vida(101);
    std::unique_ptr<Entidade> alvo(NovaEntidade(proto_alvo, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    g_dados_teste.push(1);
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_ATORDOADO);
    EXPECT_EQ(evento.id_unico(), 0);
    EXPECT_EQ(evento.rodadas(), 1);
  }
  {
    proto_alvo.set_pontos_vida(151);
    std::unique_ptr<Entidade> alvo(NovaEntidade(proto_alvo, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
    EXPECT_FALSE(AcaoAfetaAlvo(acao, *alvo));
  }
}

TEST(TesteArmas, TestePoderDivino) {
  Tabelas tabelas(nullptr);
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
  RecomputaDependencias(tabelas, &proto);
  const AcaoProto& acao = da->acao();
  const int nivel_conjurador = NivelConjurador(TipoAtaqueParaClasse(tabelas, da->tipo_ataque()), proto);
  ASSERT_FALSE(acao.efeitos_adicionais().empty());

  std::unique_ptr<Entidade> alvo(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
  {
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
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
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(20);
  AtribuiBaseAtributo(20, TA_INTELIGENCIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Mago");
  da->set_id_arma("palavra_poder_matar");
  RecomputaDependencias(tabelas, &proto);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->alcance_m(), 15 * QUADRADOS_PARA_METROS) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  EXPECT_FALSE(da->acao().permite_ataque_vs_defesa()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->acao().permite_salvacao()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->acao().ignora_resistencia_magia()) << "DA completo: " << da->DebugString();
  const AcaoProto& acao = da->acao();
  const int nivel_conjurador = NivelConjurador(TipoAtaqueParaClasse(tabelas, da->tipo_ataque()), proto);
  ASSERT_FALSE(acao.efeitos_adicionais().empty());

  EntidadeProto proto_alvo;
  proto_alvo.set_max_pontos_vida(200);
  {
    proto_alvo.set_pontos_vida(100);
    std::unique_ptr<Entidade> alvo(NovaEntidade(proto_alvo, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
    EXPECT_TRUE(AcaoAfetaAlvo(acao, *alvo));
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
    ASSERT_FALSE(n.entidade().evento().empty());
    const auto& evento = n.entidade().evento(0);
    EXPECT_EQ(evento.id_efeito(), EFEITO_MORTE);
  }
  {
    proto_alvo.set_pontos_vida(101);
    std::unique_ptr<Entidade> alvo(NovaEntidade(proto_alvo, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
    EXPECT_FALSE(AcaoAfetaAlvo(acao, *alvo));
  }
}

TEST(TesteArmas, TesteRaioEnfraquecimento) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(3);
  AtribuiBaseAtributo(15, TA_INTELIGENCIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Mago");
  da->set_id_arma("raio_enfraquecimento");
  RecomputaDependencias(tabelas, &proto);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->ataque_arremesso()) << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->alcance_m(), 6 * QUADRADOS_PARA_METROS) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_RAIO) << "acao: " << acao.DebugString();
  const int nivel_conjurador = NivelConjurador(TipoAtaqueParaClasse(tabelas, da->tipo_ataque()), proto);
  ASSERT_FALSE(acao.efeitos_adicionais().empty());

  EntidadeProto proto_alvo;
  AtribuiBaseAtributo(20, TA_FORCA, &proto_alvo);
  std::unique_ptr<Entidade> alvo(NovaEntidade(proto_alvo, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
  {
    // Primeiro ataque.
    g_dados_teste.push(3);
    std::vector<int> ids_unicos;
    ntf::Notificacao n;
    PreencheNotificacaoEventoEfeitoAdicional(nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
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
    PreencheNotificacaoEventoEfeitoAdicional(nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
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
    PreencheNotificacaoEventoEfeitoAdicional(nivel_conjurador, *alvo, acao.efeitos_adicionais(0), &ids_unicos, &n, nullptr);
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
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* da = proto.add_dados_ataque();
  da->set_id_arma("arco_longo");
  RecomputaDependencias(tabelas, &proto);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_PROJETIL) << "acao: " << acao.DebugString();
}

TEST(TesteArmas, TesteBoleadeira) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto_ataque;
  auto* da = proto_ataque.add_dados_ataque();
  da->set_id_arma("boleadeira");
  RecomputaDependencias(tabelas, &proto_ataque);
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_PROJETIL) << "acao: " << acao.DebugString();
}

TEST(TesteArmas, TesteEspada) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto_ataque;
  auto* da = proto_ataque.add_dados_ataque();
  da->set_id_arma("espada_curta");
  RecomputaDependencias(tabelas, &proto_ataque);
  EXPECT_FALSE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_FALSE(da->ataque_toque()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_CORPO_A_CORPO) << "acao: " << acao.DebugString();
}

TEST(TesteArmas, TesteDanoIgnoraSalvacao) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("clerigo");
  ic->set_nivel(3);
  AtribuiBaseAtributo(15, TA_SABEDORIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Clérigo");
  da->set_id_arma("explosao_sonora");
  RecomputaDependencias(tabelas, &proto);
  EXPECT_TRUE(da->dano_ignora_salvacao()) << "DA completo: " << da->DebugString();
}

TEST(TesteArmas, TesteDispersao) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(3);
  AtribuiBaseAtributo(15, TA_INTELIGENCIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Mago");
  da->set_id_arma("maos_flamejantes");
  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(da->dano(), "3d4") << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->dificuldade_salvacao(), 13) << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->resultado_ao_salvar(), RS_MEIO);
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_DISPERSAO) << "acao: " << acao.DebugString();
}

TEST(TesteArmas, TesteRaio) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(3);
  AtribuiBaseAtributo(15, TA_INTELIGENCIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Feitiço de Mago");
  da->set_id_arma("raio_ardente");
  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(da->dano(), "4d6") << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->acao().permite_ataque_vs_defesa()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_distancia()) << "DA completo: " << da->DebugString();
  EXPECT_TRUE(da->ataque_toque());
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_RAIO) << "acao: " << acao.DebugString();
}

TEST(TesteArmas, TestePergaminho) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(3);
  AtribuiBaseAtributo(15, TA_INTELIGENCIA, &proto);

  auto* da = proto.add_dados_ataque();
  da->set_tipo_ataque("Pergaminho Arcano");
  da->set_id_arma("maos_flamejantes");
  da->set_nivel_conjurador_pergaminho(1);
  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(da->tipo_pergaminho(), TM_ARCANA);
  EXPECT_EQ(da->dano(), "1d4") << "DA completo: " << da->DebugString();
  EXPECT_EQ(da->dificuldade_salvacao(), 11) << "DA completo: " << da->DebugString();
}

TEST(TesteArmas, TesteVeneno) {
  Tabelas tabelas(nullptr);
  Modelos modelos;
  try {
    arq::LeArquivoAsciiProto(arq::TIPO_DADOS, "modelos.asciiproto", &modelos);
  } catch (const std::logic_error& erro) {
    ASSERT_TRUE(false) <<  erro.what();
  }
  std::unordered_map<std::string, const Modelo*> modelos_por_id;
  for (const auto& m : modelos.modelo()) {
    modelos_por_id[m.id()] = &m;
  }
  EntidadeProto proto = modelos_por_id["Centopéia Enorme"]->entidade();
  RecomputaDependencias(tabelas, &proto);
  ASSERT_FALSE(proto.dados_ataque().empty());
  EXPECT_TRUE(proto.dados_ataque(0).has_veneno());
}

TEST(TesteArmas, TesteArmaTemAcao) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto_ataque;
  auto* da = proto_ataque.add_dados_ataque();
  da->set_id_arma("adaga");
  RecomputaDependencias(tabelas, &proto_ataque);
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_PROJETIL);
}

TEST(TesteArmas, TesteProjetilArea) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto_ataque;
  auto* da = proto_ataque.add_dados_ataque();
  da->set_id_arma("fogo_alquimico");
  RecomputaDependencias(tabelas, &proto_ataque);
  EXPECT_TRUE(da->ataque_toque());
  EXPECT_TRUE(da->ataque_distancia());
  EXPECT_TRUE(da->has_acao());
  const AcaoProto& acao = da->acao();
  EXPECT_EQ(acao.tipo(), ACAO_PROJETIL_AREA);
}

TEST(TesteCA, TesteDestrezaCA) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  RecomputaDependencias(tabelas, &proto);
  EXPECT_TRUE(DestrezaNaCA(proto));

  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_CEGO);
  RecomputaDependencias(tabelas, &proto);
  EXPECT_FALSE(DestrezaNaCA(proto));

  proto.mutable_info_talentos()->add_gerais()->set_id("lutar_as_cegas");
  RecomputaDependencias(tabelas, &proto);
  EXPECT_TRUE(DestrezaNaCA(proto));

  DadosAtaque da;
  da.set_ataque_distancia(true);
  EXPECT_FALSE(DestrezaNaCAContraAtaque(&da, proto));
}

TEST(TesteResistenciaMagia, TesteResistenciaMagia) {
  Tabelas tabelas(nullptr);
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

  std::unique_ptr<Entidade> ea(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
  std::unique_ptr<Entidade> ed(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));

  g_dados_teste.push(7);
  bool sucesso;
  std::string texto;
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(/*da=*/nullptr, *ea, *ed);
  EXPECT_TRUE(sucesso) << texto;

  g_dados_teste.push(6);
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(/*da=*/nullptr, *ea, *ed);
  EXPECT_FALSE(sucesso) << texto;

  proto.mutable_info_talentos()->add_gerais()->set_id("magia_penetrante");
  std::unique_ptr<Entidade> eamp(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
  g_dados_teste.push(5);
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(/*da=*/nullptr, *eamp, *ed);
  EXPECT_TRUE(sucesso) << texto;

  proto.mutable_info_talentos()->add_gerais()->set_id("magia_penetrante_maior");
  std::unique_ptr<Entidade> eampm(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
  g_dados_teste.push(3);
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(/*da=*/nullptr, *eampm, *ed);
  EXPECT_TRUE(sucesso) << texto;
}

TEST(TesteResistenciaMagia, TesteResistenciaMagia2) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("mago");
  ic->set_nivel(3);
  {
    // Sera ignorada.
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_RESISTENCIA_MAGIA);
    evento->set_rodadas(100);
    evento->add_complementos(10);
  }
  proto.mutable_dados_defesa()->set_resistencia_magia_racial(3);

  std::unique_ptr<Entidade> ea(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
  std::unique_ptr<Entidade> ed(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));

  g_dados_teste.push(7);
  bool sucesso;
  std::string texto;
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(/*da=*/nullptr, *ea, *ed);
  EXPECT_TRUE(sucesso) << texto;

  g_dados_teste.push(6);
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(/*da=*/nullptr, *ea, *ed);
  EXPECT_FALSE(sucesso) << texto;

  proto.mutable_info_talentos()->add_gerais()->set_id("magia_penetrante");
  std::unique_ptr<Entidade> eamp(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
  g_dados_teste.push(5);
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(/*da=*/nullptr, *eamp, *ed);
  EXPECT_TRUE(sucesso) << texto;

  proto.mutable_info_talentos()->add_gerais()->set_id("magia_penetrante_maior");
  std::unique_ptr<Entidade> eampm(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
  g_dados_teste.push(3);
  std::tie(sucesso, texto) = AtaqueVsResistenciaMagia(/*da=*/nullptr, *eampm, *ed);
  EXPECT_TRUE(sucesso) << texto;
}

TEST(TestePergaminho, PodeLancar) {
  Tabelas tabelas(nullptr);
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
  RecomputaDependencias(tabelas, &proto);
  // Tipo errado.
  EXPECT_FALSE(PodeLancarPergaminho(tabelas, proto, proto.dados_ataque(0)).first);
  // Fora da lista.
  EXPECT_FALSE(PodeLancarPergaminho(tabelas, proto, proto.dados_ataque(1)).first);
  // Atributo invalido.
  EXPECT_FALSE(PodeLancarPergaminho(tabelas, proto, proto.dados_ataque(2)).first);

  EXPECT_TRUE(PodeLancarPergaminho(tabelas, proto, proto.dados_ataque(3)).first);
}

TEST(TestePergaminho, PodeLancarDominio) {
  Tabelas tabelas(nullptr);
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
  RecomputaDependencias(tabelas, &proto);
  EXPECT_TRUE(PodeLancarPergaminho(tabelas, proto, proto.dados_ataque(0)).first);
  EXPECT_FALSE(PodeLancarPergaminho(tabelas, proto, proto.dados_ataque(1)).first);
}


TEST(TestePergaminho, TesteLancarPergaminhoSemRisco) {
  Tabelas tabelas(nullptr);
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
  RecomputaDependencias(tabelas, &proto);
  EXPECT_TRUE(TesteLancarPergaminho(tabelas, proto, proto.dados_ataque(0)).ok);
}

TEST(TestePergaminho, TesteLancarPergaminhoSucesso) {
  Tabelas tabelas(nullptr);
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
  RecomputaDependencias(tabelas, &proto);
  g_dados_teste.push(3);
  auto res = TesteLancarPergaminho(tabelas, proto, proto.dados_ataque(0));
  EXPECT_TRUE(res.ok) << res.texto;
}

TEST(TestePergaminho, TesteLancarPergaminhoFalhaSemFiasco) {
  Tabelas tabelas(nullptr);
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
  RecomputaDependencias(tabelas, &proto);
  g_dados_teste.push(1);  // 1 + 1 de nivel = 2 < 3 (nivel conjurador pergaminho  1);
  g_dados_teste.push(5);  // 4 + 0 de sabedoria >= 5.
  auto res = TesteLancarPergaminho(tabelas, proto, proto.dados_ataque(0));
  EXPECT_FALSE(res.ok) << res.texto;
  EXPECT_FALSE(res.fiasco) << res.texto;
}

TEST(TestePergaminho, TesteLancarPergaminhoFalhaComFiasco) {
  Tabelas tabelas(nullptr);
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
  RecomputaDependencias(tabelas, &proto);
  g_dados_teste.push(1);
  g_dados_teste.push(4);  // 4 + 0 sabedoria < 5.
  auto res = TesteLancarPergaminho(tabelas, proto, proto.dados_ataque(0));
  EXPECT_FALSE(res.ok) << res.texto;
  EXPECT_TRUE(res.fiasco) << res.texto;
}

TEST(TesteTalentoPericias, AumentaNivelDeDruida) {
  Tabelas tabelas(nullptr);
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
  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(NivelParaCalculoMagiasPorDia(tabelas, "druida", proto), 9);
  EXPECT_EQ(NivelConjurador("druida", proto), 9);
  EXPECT_EQ(NivelMaximoFeitico(tabelas, "druida", 9), 5);
  EXPECT_EQ(proto.classe_feitico_ativa(), "druida");
}

TEST(TesteTalentoPericias, AumentaNivelDeRanger) {
  Tabelas tabelas(nullptr);
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
  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(NivelParaCalculoMagiasPorDia(tabelas, "ranger", proto), 9);
  EXPECT_EQ(NivelConjurador("ranger", proto), 4);
  EXPECT_EQ(NivelMaximoFeitico(tabelas, "ranger", 9), 2);
}

TEST(TesteTalentoPericias, AumentaNivelConjuradorVitalidadeIlusoria) {
  Tabelas tabelas(nullptr);
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
    evento->set_id_efeito(EFEITO_VITALIDADE_ILUSORIA);
    evento->set_rodadas(100);
  }
  AtribuiBaseAtributo(12, TA_FORCA, &proto);

  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(NivelParaCalculoMagiasPorDia(tabelas, "clerigo", proto), 8);
  EXPECT_EQ(NivelConjurador("clerigo", proto), 9);
  EXPECT_EQ(ModificadorAtributo(proto.atributos().forca()), ModificadorAtributo(14));
}

TEST(TesteTalentoPericias, TesteHabilidadesEspeciais) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("monge");

  ic->set_nivel(1);
  proto.mutable_dados_defesa()->set_evasao_estatica(TE_EVASAO_APRIMORADA);
  RecomputaDependencias(tabelas, &proto);
  EXPECT_FALSE(PossuiHabilidadeEspecial("evasao", proto));
  EXPECT_FALSE(PossuiHabilidadeEspecial("evasao_aprimorada", proto));
  EXPECT_TRUE(TipoEvasaoPersonagem(proto) == TE_EVASAO_APRIMORADA);

  ic->set_nivel(2);
  proto.mutable_dados_defesa()->clear_evasao_estatica();
  RecomputaDependencias(tabelas, &proto);
  EXPECT_TRUE(PossuiHabilidadeEspecial("evasao", proto));
  EXPECT_FALSE(PossuiHabilidadeEspecial("evasao_aprimorada", proto));
  EXPECT_TRUE(TipoEvasaoPersonagem(proto) == TE_EVASAO);

  ic->set_nivel(9);
  RecomputaDependencias(tabelas, &proto);
  EXPECT_TRUE(PossuiHabilidadeEspecial("evasao", proto));
  EXPECT_TRUE(PossuiHabilidadeEspecial("evasao_aprimorada", proto));
  EXPECT_TRUE(TipoEvasaoPersonagem(proto) == TE_EVASAO_APRIMORADA);
}

TEST(TesteVazamento, TesteVazamento) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("monge");
  ic->set_nivel(2);
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_CURA_ACELERADA);
  evento->add_complementos(5);
  RecomputaDependencias(tabelas, &proto);
  int tamanho = proto.ByteSize();
  for (int i = 0; i < 100; ++i) {
    RecomputaDependencias(tabelas, &proto);
  }
  EXPECT_EQ(tamanho, proto.ByteSize());
}

TEST(TesteVezes, TesteVezes) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
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

  RecomputaDependencias(tabelas, &proto);
  // Vai ter criado o agarrar tb.
  ASSERT_EQ(proto.dados_ataque().size(), 2);
  EXPECT_EQ(proto.dados_ataque(0).id_arma(), "espada_curta");
}

TEST(TesteTalentoPericias, TesteTalentoPericias) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("ladino");
  ic->set_nivel(1);
  AtribuiBaseAtributo(12, TA_FORCA, &proto);
  AtribuiBaseAtributo(14, TA_DESTREZA, &proto);
  proto.mutable_info_talentos()->add_gerais()->set_id("maos_leves");
  auto* p = PericiaCriando("usar_cordas", &proto);
  p->set_pontos(5);
  RecomputaDependencias(tabelas, &proto);

  // 2 des, 2 talento, 5 pontos de classe.
  EXPECT_EQ(9, BonusTotal(p->bonus()));
  // 1 forca, 2 sinergia.
  EXPECT_EQ(3, BonusTotal(Pericia("escalar", proto).bonus()));

  // Pericia deixa de ser de classe.
  ic->set_id("guerreiro");
  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(6, BonusTotal(p->bonus()));
  // Sem sinergia.
  EXPECT_EQ(1, BonusTotal(Pericia("escalar", proto).bonus()));
}

TEST(TesteFormaAlternativa, TesteFormaAlternativa) {
  // TODO ignorar INT SAB CAR.
  Tabelas tabelas(nullptr);
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

  std::unique_ptr<Entidade> e(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
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

TEST(TesteDependencias, TestePericias) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  AtribuiBaseAtributo(12, TA_INTELIGENCIA, &proto);
  auto* ic = proto.add_info_classes();
  ic->set_id("ladino");
  ic->set_nivel(1);
  EXPECT_TRUE(PericiaDeClasse(tabelas, "observar", proto));
  EXPECT_FALSE(PericiaDeClasse(tabelas, "adestrar_animais", proto));
  EXPECT_EQ(TotalPontosPericiaPermitidos(tabelas, proto), 36);
}

TEST(TesteDependencias, TesteNiveisNegativos) {
  Tabelas tabelas(nullptr);
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

  RecomputaDependencias(tabelas, &proto);
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
  Tabelas tabelas(nullptr);
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

  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(proto.niveis_negativos(), 2);

  {
    // Multiplos se acumulam.
    auto* evento = proto.add_evento();
    evento->set_id_efeito(EFEITO_DRENAR_TEMPORARIO);
    evento->set_rodadas(1);
    evento->add_complementos(2);
    evento->set_id_unico(2);
  }

  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(proto.niveis_negativos(), 4);
}

TEST(TesteDependencias, TesteReducaoDanoBarbaro) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* ic = proto.add_info_classes();
  ic->set_id("barbaro");
  ic->set_nivel(3);
  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(0, proto.dados_defesa().reducao_dano_barbaro());
  ic->set_nivel(7);
  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(1, proto.dados_defesa().reducao_dano_barbaro());
  ic->set_nivel(10);
  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(2, proto.dados_defesa().reducao_dano_barbaro());
  ic->set_nivel(13);
  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(3, proto.dados_defesa().reducao_dano_barbaro());
  ic->set_nivel(16);
  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(4, proto.dados_defesa().reducao_dano_barbaro());
  ic->set_nivel(19);
  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(5, proto.dados_defesa().reducao_dano_barbaro());
}

TEST(TesteDependencias, TesteInvestida) {
  Tabelas tabelas(nullptr);
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

  RecomputaDependencias(tabelas, &proto);
  // 0 + 2.
  EXPECT_EQ(2, proto.dados_ataque(0).bonus_ataque_final());
  // 10 - 2.
  EXPECT_EQ(8, BonusTotal(proto.dados_defesa().ca()));
}

TEST(TesteDependencias, TesteDependencias) {
  Tabelas tabelas(nullptr);
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
  RecomputaDependencias(tabelas, &proto);
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
  EXPECT_EQ(14, proto.dados_ataque(0).ca_surpreso());
  EXPECT_EQ(9, proto.dados_ataque(0).ca_toque());
  // Segundo ataque.
  // Espada longa grande com uma mao.
  EXPECT_EQ("2d6+3", proto.dados_ataque(1).dano());
  EXPECT_EQ(21, proto.dados_ataque(1).ca_normal());
  EXPECT_EQ(19, proto.dados_ataque(1).ca_surpreso());
  EXPECT_EQ(9, proto.dados_ataque(1).ca_toque());

  EXPECT_GE(proto.tendencia().eixo_bem_mal(), 0.6f);

  auto* ea = NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr);
  auto* ed = NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr);
  // 16 normal +2 contra o bem.
  EXPECT_EQ(18, ed->CA(*ea, Entidade::CA_NORMAL));
  // 6 normal + 2 contra o bem.
  EXPECT_EQ(6, ed->Salvacao(*ea, TS_VONTADE));
}

TEST(TesteDependencias, TesteDependenciasTalentosSalvacoes) {
  Tabelas tabelas(nullptr);
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

  RecomputaDependencias(tabelas, &proto);
  // 3 + 4 con + 2 fortitude maior;
  EXPECT_EQ(9, BonusTotal(proto.dados_defesa().salvacao_fortitude()));
  // 1 + 3 destreza.
  EXPECT_EQ(4, BonusTotal(proto.dados_defesa().salvacao_reflexo()));
  // 1 de vontade, +2 bonus.
  EXPECT_EQ(3, BonusTotal(proto.dados_defesa().salvacao_vontade()));

  // Adiciona reflexos rapidos.
  proto.mutable_info_talentos()->add_gerais()->set_id("reflexos_rapidos");
  RecomputaDependencias(tabelas, &proto);
  // 3 + 3 con + 2 fortitude maior;
  EXPECT_EQ(9, BonusTotal(proto.dados_defesa().salvacao_fortitude()));
  // 1 + 3 destreza + 2 reflexos rapidos.
  EXPECT_EQ(6, BonusTotal(proto.dados_defesa().salvacao_reflexo()));
  // 1 de vontade, +2 bonus.
  EXPECT_EQ(3, BonusTotal(proto.dados_defesa().salvacao_vontade()));

  // Adiciona vontade de ferro.
  proto.mutable_info_talentos()->add_gerais()->set_id("vontade_ferro");
  RecomputaDependencias(tabelas, &proto);
  // 3 + 3 con + 2 fortitude maior;
  EXPECT_EQ(9, BonusTotal(proto.dados_defesa().salvacao_fortitude()));
  // 1 + 3 destreza + 2 reflexos rapidos.
  EXPECT_EQ(6, BonusTotal(proto.dados_defesa().salvacao_reflexo()));
  // 1 de vontade, +2 bonus + 2 vontade ferro.
  EXPECT_EQ(5, BonusTotal(proto.dados_defesa().salvacao_vontade()));
  // Adiciona de novo (sem efeito).
  proto.mutable_info_talentos()->add_gerais()->set_id("vontade_ferro");
  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(9, BonusTotal(proto.dados_defesa().salvacao_fortitude()));

  // Remove reflexos rapidos.
  proto.mutable_info_talentos()->mutable_gerais()->DeleteSubrange(1, 1);
  RecomputaDependencias(tabelas, &proto);
  // 1 + 3 destreza.
  EXPECT_EQ(4, BonusTotal(proto.dados_defesa().salvacao_reflexo()));
}

TEST(TesteDependencias, TesteAgarrar) {
  Tabelas tabelas(nullptr);
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


  RecomputaDependencias(tabelas, &proto);
  // 3 + 2 forca + 4 tamanho.
  EXPECT_EQ(9, proto.bba().agarrar());

  proto.mutable_info_talentos()->add_gerais()->set_id("agarrar_aprimorado");
  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(13, proto.bba().agarrar());
}

TEST(TesteDependencias, TesteAjuda) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  std::vector<int> ids_unicos;
  auto* ev = AdicionaEvento(/*origem*/"ajuda", EFEITO_AJUDA, 10, false, &ids_unicos, &proto);
  ev->add_complementos(3);
  RecomputaDependencias(tabelas, &proto);
  // Neste ponto, espera-se uma entrada em pontos de vida temporario SEM_NOME, "ajuda".
  auto* po = OrigemSePresente(TB_SEM_NOME, "ajuda", proto.mutable_pontos_vida_temporarios_por_fonte());
  ASSERT_NE(po, nullptr);
  EXPECT_GT(proto.pontos_vida_temporarios(), 3);
  const int valor = po->valor();

  // Nova chamada, mantem o mesmo valor. Verifica duplicatas.
  RecomputaDependencias(tabelas, &proto);
  ASSERT_EQ(1, proto.pontos_vida_temporarios_por_fonte().bonus_individual().size());
  ASSERT_EQ(1, proto.pontos_vida_temporarios_por_fonte().bonus_individual(0).por_origem().size());
  po = OrigemSePresente(TB_SEM_NOME, "ajuda", proto.mutable_pontos_vida_temporarios_por_fonte());
  ASSERT_NE(po, nullptr);
  EXPECT_EQ(proto.pontos_vida_temporarios(), valor);

  // Termina o efeito.
  ev->set_rodadas(-1);
  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(proto.pontos_vida_temporarios(), 0);
}

TEST(TesteDependencias, TesteAjuda2) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  std::vector<int> ids_unicos;
  auto* ev = AdicionaEvento(/*origem*/"ajuda", EFEITO_AJUDA, 10, false, &ids_unicos, &proto);
  ev = AdicionaEvento(/*origem*/"ajuda", EFEITO_AJUDA, 10, false, &ids_unicos, &proto);
  int id_segundo_evento = ev->id_unico();
  RecomputaDependencias(tabelas, &proto);
  // Neste ponto, espera-se uma entrada em pontos de vida temporario SEM_NOME, "ajuda".
  ASSERT_EQ(1, proto.pontos_vida_temporarios_por_fonte().bonus_individual().size());
  ASSERT_EQ(1, proto.pontos_vida_temporarios_por_fonte().bonus_individual(0).por_origem().size());
  EXPECT_EQ("ajuda", proto.pontos_vida_temporarios_por_fonte().bonus_individual(0).por_origem(0).origem());

  // Forcar o temporario a vir do segundo evento (porque sao aleatorios os valores).
  auto* po = OrigemSePresente(TB_SEM_NOME, "ajuda", proto.mutable_pontos_vida_temporarios_por_fonte());
  ASSERT_NE(po, nullptr);
  po->set_id_unico(id_segundo_evento);
  ev->set_rodadas(-1);

  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(proto.pontos_vida_temporarios(), 0);
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

    EXPECT_EQ(ModificadorAtaque(TipoAtaque::AGARRAR, ea, ed), 0);
    ea.mutable_info_talentos()->add_gerais()->set_id("agarrar_aprimorado");
    EXPECT_EQ(ModificadorAtaque(TipoAtaque::AGARRAR, ea, ed), 4);
    ed.mutable_info_talentos()->add_gerais()->set_id("agarrar_aprimorado");
    EXPECT_EQ(ModificadorAtaque(TipoAtaque::AGARRAR, ea, ed), 0);
  }
}

TEST(TesteModificadorAlcance, TesteModificadorAlcance) {
  Tabelas tabelas(nullptr);
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(3);
    auto* da = proto.add_dados_ataque();
    da->set_tipo_ataque("Feitiço de Mago");
    da->set_id_arma("missil_magico");
    RecomputaDependencias(tabelas, &proto);

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
    RecomputaDependencias(tabelas, &proto);

    EXPECT_EQ(da->alcance_m(), 24 * TAMANHO_LADO_QUADRADO);
  }
}

TEST(TesteSalvacaoDinamica, TesteRodadasDinamico) {
  Tabelas tabelas(nullptr);
  ntf::Notificacao n;
  EntidadeProto proto;
  std::unique_ptr<Entidade> e(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
  std::vector<int> ids_unicos = IdsUnicosEntidade(*e);
  PreencheNotificacaoEventoEfeitoAdicional(/*nivel*/3, *e, tabelas.Feitico("sono").acao().efeitos_adicionais(0), &ids_unicos, &n, nullptr);
  ASSERT_FALSE(n.entidade().evento().empty());
  EXPECT_EQ(n.entidade().evento(0).rodadas(), 30);
  ASSERT_EQ(ids_unicos.size(), 1);
  EXPECT_EQ(n.entidade().evento(0).id_unico(), ids_unicos[0]);
}

// Este teste simula mais ou menos a forma como os efeitos adicionais de feiticos sao aplicados.
TEST(TesteSalvacaoDinamica, TesteEfeitosAdicionaisMultiplos) {
  Tabelas tabelas(nullptr);
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(3);
    std::unique_ptr<Entidade> e(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
    ntf::Notificacao n;
    std::vector<int> ids_unicos = IdsUnicosEntidade(*e);
    PreencheNotificacaoEventoEfeitoAdicional(/*nivel*/3, *e, tabelas.Feitico("teia").acao().efeitos_adicionais(0), &ids_unicos, n.add_notificacao(), nullptr);
    PreencheNotificacaoEventoEfeitoAdicional(/*nivel*/3, *e, tabelas.Feitico("teia").acao().efeitos_adicionais(1), &ids_unicos, n.add_notificacao(), nullptr);
    e->AtualizaParcial(n.notificacao(0).entidade());
    e->AtualizaParcial(n.notificacao(1).entidade());
    ASSERT_EQ(e->Proto().evento().size(), 2);
    EXPECT_EQ(e->Proto().evento(0).id_efeito(), EFEITO_ENREDADO);
    EXPECT_EQ(e->Proto().evento(1).id_efeito(), EFEITO_OUTRO);
    ASSERT_EQ(ids_unicos.size(), 2);
    EXPECT_EQ(ids_unicos[0], 0);
    EXPECT_EQ(ids_unicos[1], 1);
  }
}

TEST(TesteSalvacaoDinamica, TesteSalvacaoDinamica) {
  Tabelas tabelas(nullptr);
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(3);
    AtribuiBaseAtributo(12, TA_INTELIGENCIA, &proto);
    auto* da = proto.add_dados_ataque();
    da->set_tipo_ataque("Feitiço de Mago");
    da->set_id_arma("bola_fogo");
    RecomputaDependencias(tabelas, &proto);

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
    RecomputaDependencias(tabelas, &proto);

    EXPECT_EQ(da->dificuldade_salvacao(), 15);
  }
}

TEST(TesteFeiticos, TesteFeiticos) {
  Tabelas tabelas(nullptr);
  {
    EntidadeProto proto;
    auto* ic = proto.add_info_classes();
    ic->set_id("mago");
    ic->set_nivel(3);
    AtribuiBaseAtributo(14, TA_INTELIGENCIA, &proto);
    RecomputaDependencias(tabelas, &proto);

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
    RecomputaDependencias(tabelas, &proto);

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

TEST(TesteImunidades, TesteImunidadeElemento) {
  Tabelas tabelas(nullptr);
  {
    EntidadeProto proto;
    RecomputaDependencias(tabelas, &proto);
    EXPECT_FALSE(EntidadeImuneElemento(proto, DESC_ACIDO));
  }
  {
    EntidadeProto proto;
    proto.mutable_dados_defesa()->add_imunidades(DESC_ACIDO);
    RecomputaDependencias(tabelas, &proto);
    EXPECT_TRUE(EntidadeImuneElemento(proto, DESC_ACIDO));
  }
}

TEST(TesteImunidades, TesteReducaoDanoFormaGasosa) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* evento = proto.add_evento();
  evento->set_id_efeito(EFEITO_FORMA_GASOSA);
  evento->set_rodadas(1);
  auto* dd = proto.mutable_dados_defesa();
  dd->set_id_armadura("cota_malha");
  dd->set_bonus_magico_armadura(2);
  std::unique_ptr<Entidade> e(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
  proto = e->Proto();

  EntidadeProto proto_ataque;
  auto* da = proto_ataque.add_dados_ataque();
  da->set_id_arma("adaga");
  da->set_obra_prima(true);
  RecomputaDependencias(tabelas, &proto_ataque);
  std::unique_ptr<Entidade> ea(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
  EXPECT_EQ(e->CA(*ea, Entidade::CA_NORMAL), 10);

  int delta;
  std::string msg;
  std::tie(delta, msg) = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, da->descritores());
  EXPECT_EQ(delta, 0) << msg;

  da->set_bonus_magico(1);
  RecomputaDependencias(tabelas, &proto_ataque);
  std::tie(delta, msg) = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, da->descritores());
  EXPECT_EQ(delta, -10) << msg;
}

TEST(TesteImunidades, TesteReducaoDanoSimples) {
  Tabelas tabelas(nullptr);
  {
    google::protobuf::RepeatedField<int> descritores;
    descritores.Add(DESC_FERRO_FRIO);
    EntidadeProto proto;
    RecomputaDependencias(tabelas, &proto);
    int delta;
    std::string msg;
    std::tie(delta, msg) = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(delta, -10);
  }
  {
    google::protobuf::RepeatedField<int> descritores;
    EntidadeProto proto;
    auto* rd = proto.mutable_dados_defesa()->add_reducao_dano();
    rd->set_valor(6);
    rd->add_descritores(DESC_FERRO_FRIO);
    RecomputaDependencias(tabelas, &proto);
    int delta;
    std::string msg;
    std::tie(delta, msg) = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(delta, -4) << msg;
  }
  {
    google::protobuf::RepeatedField<int> descritores;
    descritores.Add(DESC_FERRO_FRIO);
    EntidadeProto proto;
    auto* rd = proto.mutable_dados_defesa()->add_reducao_dano();
    rd->set_valor(6);
    rd->add_descritores(DESC_FERRO_FRIO);
    RecomputaDependencias(tabelas, &proto);
    int delta;
    std::string msg;
    std::tie(delta, msg) = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(delta, -10) << msg;
  }
}

TEST(TesteImunidades, TesteReducaoDanoCombinacaoE) {
  Tabelas tabelas(nullptr);
  {
    google::protobuf::RepeatedField<int> descritores;
    descritores.Add(DESC_FERRO_FRIO);
    EntidadeProto proto;
    auto* rd = proto.mutable_dados_defesa()->add_reducao_dano();
    rd->set_valor(6);
    rd->set_tipo_combinacao(COMB_E);
    rd->add_descritores(DESC_FERRO_FRIO);
    rd->add_descritores(DESC_BEM);
    RecomputaDependencias(tabelas, &proto);
    int delta;
    std::string msg;
    std::tie(delta, msg) = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(delta, -4) << msg;
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
    RecomputaDependencias(tabelas, &proto);
    int delta;
    std::string msg;
    std::tie(delta, msg) = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(delta, -4) << msg;
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
    RecomputaDependencias(tabelas, &proto);
    int delta;
    std::string msg;
    std::tie(delta, msg) = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(delta, -10) << msg;
  }
}

TEST(TesteImunidades, TesteReducaoDanoCombinacaoOu) {
  Tabelas tabelas(nullptr);
  {
    google::protobuf::RepeatedField<int> descritores;
    EntidadeProto proto;
    auto* rd = proto.mutable_dados_defesa()->add_reducao_dano();
    rd->set_valor(6);
    rd->set_tipo_combinacao(COMB_OU);
    rd->add_descritores(DESC_FERRO_FRIO);
    rd->add_descritores(DESC_BEM);
    RecomputaDependencias(tabelas, &proto);
    int delta;
    std::string msg;
    std::tie(delta, msg) = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(delta, -4) << msg;
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
    RecomputaDependencias(tabelas, &proto);
    int delta;
    std::string msg;
    std::tie(delta, msg) = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(delta, -10) << msg;
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
    RecomputaDependencias(tabelas, &proto);
    int delta;
    std::string msg;
    std::tie(delta, msg) = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto, descritores);
    EXPECT_EQ(delta, -10) << msg;
  }
}

TEST(TesteImunidades, TesteReducaoDanoCombinacaoOuProtoAtaqueSucesso) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto_defesa;
  auto* rd = proto_defesa.mutable_dados_defesa()->add_reducao_dano();
  rd->set_valor(6);
  rd->set_tipo_combinacao(COMB_OU);
  rd->add_descritores(DESC_FERRO_FRIO);
  rd->add_descritores(DESC_BEM);
  RecomputaDependencias(tabelas, &proto_defesa);

  EntidadeProto proto_ataque;
  auto* da = proto_ataque.add_dados_ataque();
  da->set_material_arma(DESC_FERRO_FRIO);
  RecomputaDependencias(tabelas, &proto_ataque);

  int delta;
  std::string msg;
  std::tie(delta, msg) = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto_defesa, proto_ataque.dados_ataque(0).descritores());
  EXPECT_EQ(delta, -10) << msg;
}

TEST(TesteImunidades, TesteReducaoDanoCombinacaoEProtoAtaqueFalhou) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto_defesa;
  auto* rd = proto_defesa.mutable_dados_defesa()->add_reducao_dano();
  rd->set_valor(6);
  rd->set_tipo_combinacao(COMB_E);
  rd->add_descritores(DESC_FERRO_FRIO);
  rd->add_descritores(DESC_BEM);
  RecomputaDependencias(tabelas, &proto_defesa);

  EntidadeProto proto_ataque;
  auto* da = proto_ataque.add_dados_ataque();
  da->set_material_arma(DESC_FERRO_FRIO);
  RecomputaDependencias(tabelas, &proto_ataque);

  int delta;
  std::string msg;
  std::tie(delta, msg) = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto_defesa, proto_ataque.dados_ataque(0).descritores());
  EXPECT_EQ(delta, -4) << msg;
}

TEST(TesteImunidades, TesteReducaoDanoCombinacaoEProtoAtaqueSucesso) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto_defesa;
  auto* rd = proto_defesa.mutable_dados_defesa()->add_reducao_dano();
  rd->set_valor(6);
  rd->set_tipo_combinacao(COMB_E);
  rd->add_descritores(DESC_FERRO_FRIO);
  rd->add_descritores(DESC_PERFURANTE);
  rd->add_descritores(DESC_CORTANTE);
  RecomputaDependencias(tabelas, &proto_defesa);

  EntidadeProto proto_ataque;
  auto* da = proto_ataque.add_dados_ataque();
  da->set_material_arma(DESC_FERRO_FRIO);
  da->set_id_arma("adaga");  // implica DESC_PERFURANTE e DESC_CORTANTE
  RecomputaDependencias(tabelas, &proto_ataque);

  int delta;
  std::string msg;
  std::tie(delta, msg) = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto_defesa, proto_ataque.dados_ataque(0).descritores());
  EXPECT_EQ(delta, -10) << msg;
}

TEST(TesteImunidades, TesteReducaoDanoCombinacaoEProtoAtaqueAlinhadoSucesso) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto_defesa;
  auto* rd = proto_defesa.mutable_dados_defesa()->add_reducao_dano();
  rd->set_valor(6);
  rd->set_tipo_combinacao(COMB_E);
  rd->add_descritores(DESC_FERRO_FRIO);
  rd->add_descritores(DESC_BEM);
  RecomputaDependencias(tabelas, &proto_defesa);

  EntidadeProto proto_ataque;
  auto* evento = proto_ataque.add_evento();
  evento->set_id_efeito(EFEITO_ABENCOAR_ARMA);
  evento->add_complementos_str("rotulo_teste");
  evento->set_rodadas(1);
  auto* da = proto_ataque.add_dados_ataque();
  da->set_material_arma(DESC_FERRO_FRIO);
  da->set_rotulo("rotulo_teste");
  RecomputaDependencias(tabelas, &proto_ataque);

  int delta;
  std::string msg;
  std::tie(delta, msg) = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto_defesa, proto_ataque.dados_ataque(0).descritores());
  EXPECT_EQ(delta, -10) << msg;
}

TEST(TesteImunidades, TesteReducaoDanoCombinacaoEProtoAtaqueAlinhado2Sucesso) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto_defesa;
  auto* rd = proto_defesa.mutable_dados_defesa()->add_reducao_dano();
  rd->set_valor(6);
  rd->set_tipo_combinacao(COMB_E);
  rd->add_descritores(DESC_FERRO_FRIO);
  rd->add_descritores(DESC_BEM);
  RecomputaDependencias(tabelas, &proto_defesa);

  EntidadeProto proto_ataque;
  auto* evento = proto_ataque.add_evento();
  evento->set_id_efeito(EFEITO_TENDENCIA_EM_ARMA);
  evento->add_complementos_str("rotulo_teste");
  evento->add_complementos_str("bem");
  evento->set_rodadas(1);
  auto* da = proto_ataque.add_dados_ataque();
  da->set_material_arma(DESC_FERRO_FRIO);
  da->set_rotulo("rotulo_teste");
  RecomputaDependencias(tabelas, &proto_ataque);

  int delta;
  std::string msg;
  std::tie(delta, msg) = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto_defesa, proto_ataque.dados_ataque(0).descritores());
  EXPECT_EQ(delta, -10) << msg;
}

TEST(TesteImunidades, TesteReducaoDanoCombinacaoEProtoAtaqueAlinhadoFalha) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto_defesa;
  auto* rd = proto_defesa.mutable_dados_defesa()->add_reducao_dano();
  rd->set_valor(6);
  rd->set_tipo_combinacao(COMB_E);
  rd->add_descritores(DESC_FERRO_FRIO);
  rd->add_descritores(DESC_BEM);
  RecomputaDependencias(tabelas, &proto_defesa);

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
  RecomputaDependencias(tabelas, &proto_ataque);

  int delta;
  std::string msg;
  std::tie(delta, msg) = AlteraDeltaPontosVidaPorMelhorReducao(-10, proto_defesa, proto_ataque.dados_ataque(0).descritores());
  EXPECT_EQ(delta, -4) << msg;
}


TEST(TesteImunidades, TesteImunidadesNada) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(ImunidadeOuResistenciaParaElemento(-10, proto, DESC_ACIDO).causa, ALT_NENHUMA);
}

TEST(TesteImunidades, TesteImunidade) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  proto.mutable_dados_defesa()->add_imunidades(DESC_ACIDO);
  RecomputaDependencias(tabelas, &proto);
  auto resultado = ImunidadeOuResistenciaParaElemento(-10, proto, DESC_ACIDO);
  EXPECT_EQ(resultado.causa, ALT_IMUNIDADE);
  EXPECT_EQ(resultado.resistido, 10);
}

TEST(TesteImunidades, TesteResistencia) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* resistencia = proto.mutable_dados_defesa()->add_resistencia_elementos();
  resistencia->set_valor(10);
  resistencia->set_descritor(DESC_ACIDO);
  RecomputaDependencias(tabelas, &proto);
  auto resultado = ImunidadeOuResistenciaParaElemento(-10, proto, DESC_ACIDO);
  EXPECT_EQ(resultado.causa, ALT_RESISTENCIA);
  EXPECT_EQ(resultado.resistido, 10);
}

TEST(TesteImunidades, TesteMultiplasResistencia) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* resistencia = proto.mutable_dados_defesa()->add_resistencia_elementos();
  resistencia->set_valor(10);
  resistencia->set_descritor(DESC_ACIDO);
  auto* resistencia2 = proto.mutable_dados_defesa()->add_resistencia_elementos();
  resistencia2->set_valor(12);
  resistencia2->set_descritor(DESC_ACIDO);

  RecomputaDependencias(tabelas, &proto);
  auto resultado = ImunidadeOuResistenciaParaElemento(-15, proto, DESC_ACIDO);
  EXPECT_EQ(resultado.causa, ALT_RESISTENCIA);
  EXPECT_EQ(resultado.resistido, 12);
}

TEST(TesteImunidades, TesteResistenciaNaoBate) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  auto* resistencia = proto.mutable_dados_defesa()->add_resistencia_elementos();
  resistencia->set_valor(10);
  resistencia->set_descritor(DESC_ACIDO);

  RecomputaDependencias(tabelas, &proto);
  auto resultado = ImunidadeOuResistenciaParaElemento(-10, proto, DESC_FOGO);
  EXPECT_EQ(resultado.causa, ALT_NENHUMA);
}

TEST(TesteAfetaApenas, TesteAfetaApenasNegativo) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  std::unique_ptr<Entidade> e(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));

  AcaoProto acao;
  acao.add_afeta_apenas(TIPO_MORTO_VIVO);

  EXPECT_FALSE(AcaoAfetaAlvo(acao, *e));
}

TEST(TesteAfetaApenas, TesteAfetaApenasPositivo) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  proto.add_tipo_dnd(TIPO_MORTO_VIVO);
  std::unique_ptr<Entidade> e(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));

  AcaoProto acao;
  acao.add_afeta_apenas(TIPO_MORTO_VIVO);

  EXPECT_TRUE(AcaoAfetaAlvo(acao, *e));
}

TEST(TesteAfetaApenas, TesteAfetaApenasGenerico) {
  Tabelas tabelas(nullptr);
  EntidadeProto proto;
  std::unique_ptr<Entidade> e(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));

  AcaoProto acao;

  EXPECT_TRUE(AcaoAfetaAlvo(acao, *e));
}

TEST(TesteCuraAcelerada, TesteCuraAcelerada) {
  Tabelas tabelas(nullptr);
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
  RecomputaDependencias(tabelas, &proto);
  EXPECT_EQ(5, CuraAcelerada(proto));
}

TEST(TesteCuraAcelerada, TesteCuraAcelerada2) {
  Tabelas tabelas(nullptr);
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
  std::unique_ptr<Entidade> e(NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr));
  PreencheNotificacaoCuraAcelerada(*e, &n);
  e->AtualizaParcial(n.entidade());

  EXPECT_EQ(e->DanoNaoLetal(), 0);
  EXPECT_EQ(e->PontosVida(), 15);
  EXPECT_EQ(e->MaximoPontosVida(), 15);
}

}  // namespace ent.

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
