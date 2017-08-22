#include "ent/entidade.h"
#include "ent/tabelas.h"
#include "ent/util.h"
#include "gtest/gtest.h"
#include "log/log.h"

namespace ent {

TEST(TesteDependencias, TesteDependencias) {
  Tabelas tabelas;
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

  auto* ea = NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr);
  auto* ed = NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr);
  // 16 normal +2 contra o bem.
  EXPECT_EQ(18, ed->CA(*ea, Entidade::CA_NORMAL));
  // 6 normal + 2 contra o bem.
  EXPECT_EQ(6, ed->Salvacao(*ea, TS_VONTADE));
}

TEST(TesteDependencias, TesteDependenciasTalentosSalvacoes) {
  Tabelas tabelas;
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
  Tabelas tabelas;
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
  Tabelas tabelas;
  EntidadeProto proto;
  auto* ev = AdicionaEvento(EFEITO_AJUDA, 10, &proto);
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
  Tabelas tabelas;
  EntidadeProto proto;
  auto* ev = AdicionaEvento(EFEITO_AJUDA, 10, &proto);
  ev = AdicionaEvento(EFEITO_AJUDA, 10, &proto);
  uint32_t id_segundo_evento = ev->id_unico();
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

}  // namespace ent.

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
