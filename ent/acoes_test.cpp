#include <memory>
#include <queue>
#include "ent/entidade.h"
#include "ent/entidade.pb.h"
#include "ent/acoes.h"
#include "ent/acoes.pb.h"
#include "ent/tabelas.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "log/log.h"

namespace ent {

extern std::queue<int> g_dados_teste;
namespace {
Tabelas g_tabelas(nullptr);
}  // namespace

// Teste basico gera dados.
TEST(TesteAcoes, TesteAreaAfetadaCone) {
  Posicao ponto;
  ponto.set_x(5.0f);
  Posicao origem;
  AcaoProto acao_proto;
  acao_proto.set_tipo(ACAO_DISPERSAO);
  acao_proto.set_geometria(ACAO_GEO_CONE);
  acao_proto.set_distancia_quadrados(4);
  acao_proto.mutable_pos_tabuleiro()->set_x(10.0f);
  EXPECT_TRUE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto, false));

  acao_proto.mutable_pos_tabuleiro()->set_x(-10.0f);
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto, false));

  acao_proto.mutable_pos_tabuleiro()->set_x(10.0f);
  acao_proto.set_distancia_quadrados(3);
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto, false));

  acao_proto.set_distancia_quadrados(4);
  ponto.set_y(1.0f);
  EXPECT_TRUE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto, false));

  // Dentro do alcance, fora do angulo do cone.
  ponto.set_y(3.1f);
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto, false));
}

TEST(TesteAcoes, TesteAreaAfetadaRaio) {
  Posicao origem;
  origem.set_x(1.0f);

  AcaoProto acao_proto;
  acao_proto.set_tipo(ACAO_RAIO);
  acao_proto.set_efeito_area(true);
  acao_proto.set_distancia_quadrados(10);
  // Raio paralelo ao eixo Y.
  acao_proto.mutable_pos_tabuleiro()->set_x(1.0f);  // direcao do raio
  acao_proto.mutable_pos_tabuleiro()->set_y(1.0f);  // direcao do raio

  // Ponto a frente do raio.
  Posicao ponto;
  ponto.set_x(1.0f);
  ponto.set_y(2.0f);
  EXPECT_TRUE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto, false));

  // Ponto a frente, menos de meio quadrado ao lado.
  ponto.set_x(1.5f);
  ponto.set_y(2.0f);
  EXPECT_TRUE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto, false));

  {
    // Dados de verdade.
    Posicao ponto;
    ponto.set_x(0.296404f);
    ponto.set_y(-7.3323421f);

    AcaoProto acao(acao_proto);
    acao.mutable_pos_tabuleiro()->set_x(-0.23085393);
    acao.mutable_pos_tabuleiro()->set_y(-10.466313);

    Posicao origem;
    origem.set_x(-5.25f);
    origem.set_y(-6.75f);
    EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao, false));
  }

  // Mais de meio quadrado com tolerancia de 10%.
  ponto.set_x(1.9f);
  ponto.set_y(2.0f);
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto, false));

  // Ponto fora do alcance do raio.
  ponto.set_x(1.0f);
  ponto.set_y(16.0f);
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto, false));

  // Ponto atras do raio.
  ponto.set_x(1.0f);
  ponto.set_y(-1.0f);
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto, false));

  // Ponto fora da linha do raio.
  ponto.set_x(5.0f);
  ponto.set_y(5.0f);
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto, false));
  ponto.set_x(-5.0f);
  ponto.set_y(5.0f);
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto, false));

  // Ponto na origem.
  ponto.set_x(1.0f);
  ponto.set_y(0.0f);
  EXPECT_TRUE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto, false));
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto, true));
}

TEST(TesteAcoes, TesteAreaAfetadaEsfera) {
  Posicao ponto;
  ponto.set_x(5.6f);
  Posicao origem;
  AcaoProto acao_proto;
  acao_proto.set_tipo(ACAO_DISPERSAO);
  acao_proto.set_geometria(ACAO_GEO_ESFERA);
  acao_proto.set_efeito_area(true);
  acao_proto.set_raio_quadrados(3);  // 4.5m
  acao_proto.mutable_pos_tabuleiro()->set_x(1.0f);
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto, false));

  ponto.set_x(5.4f);
  EXPECT_TRUE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto, false));
}

TEST(TesteAcoes, TesteAjustePontoDispersao) {
  Posicao origem;  // nao faz diferenca para ESFERA.

  Posicao ponto;
  ponto.set_x(5.0);

  AcaoProto acao_proto;
  acao_proto.set_tipo(ACAO_DISPERSAO);
  acao_proto.set_geometria(ACAO_GEO_ESFERA);
  acao_proto.set_efeito_area(true);
  acao_proto.set_raio_quadrados(3);  // 4.5m
  acao_proto.mutable_pos_tabuleiro()->set_x(1.0f);

  Posicao ponto_ajustado = Acao::AjustaPonto(ponto, 1.0f, origem, acao_proto);
  EXPECT_FLOAT_EQ(4.25, ponto_ajustado.x());

  ponto.set_x(1.0f);
  ponto_ajustado = Acao::AjustaPonto(ponto, 1.0f, origem, acao_proto);
  EXPECT_FLOAT_EQ(1.0f, ponto_ajustado.x());

  ponto.set_x(1.3f);
  ponto_ajustado = Acao::AjustaPonto(ponto, 1.0f, origem, acao_proto);
  EXPECT_FLOAT_EQ(1.0f, ponto_ajustado.x());
}

TEST(TesteAcoes, TesteAjustePontoRaio) {
  Posicao origem;
  origem.set_x(1.0f);

  // Raio paralelo ao eixo y.
  AcaoProto acao_proto;
  acao_proto.set_tipo(ACAO_RAIO);
  acao_proto.set_efeito_area(true);
  acao_proto.set_distancia_quadrados(10);  // 15m
  acao_proto.mutable_pos_tabuleiro()->set_x(1.0f);
  acao_proto.mutable_pos_tabuleiro()->set_y(1.0f);

  Posicao ponto;

  // Ajuste vai aproximar ponto do eixo, na direcao -x.
  ponto.set_x(5.0f);
  Posicao ponto_ajustado = Acao::AjustaPonto(ponto, 1.0f, origem, acao_proto);
  EXPECT_FLOAT_EQ(4.25, ponto_ajustado.x());

  // Ajuste vai aproximar o ponto do eixo, na direcao x.
  ponto.set_x(-5.0f);
  ponto_ajustado = Acao::AjustaPonto(ponto, 1.0f, origem, acao_proto);
  EXPECT_FLOAT_EQ(-4.25, ponto_ajustado.x());

  // Entidade alinhada com raio, vai aproximar.
  ponto.set_x(1.0f);
  ponto.set_y(5.0f);
  ponto_ajustado = Acao::AjustaPonto(ponto, 1.0f, origem, acao_proto);
  EXPECT_FLOAT_EQ(4.25, ponto_ajustado.y());
}

TEST(TesteAcoes, TesteEntidadesAfetadasPorAcao) {
  AcaoProto acao;
  acao.set_tipo(ACAO_DISPERSAO);
  acao.set_geometria(ACAO_GEO_ESFERA);
  acao.set_raio_quadrados(1);
  acao.set_total_dv(3);
  acao.set_mais_fracos_primeiro(false);
  std::vector<const Entidade*> entidades;
  int id = 1;
  for (int nivel : { 1, 2, 1, 1}) {
    EntidadeProto proto;
    proto.set_id(id++);
    auto* ic = proto.add_info_classes();
    ic->set_id("guerreiro");
    ic->set_nivel(nivel);
    entidades.push_back(NovaEntidadeParaTestes(proto, g_tabelas));
  }

  const std::vector<unsigned int> ids_afetados = EntidadesAfetadasPorAcao(acao, nullptr, entidades);
  EXPECT_THAT(ids_afetados, testing::ElementsAre(1, 2));

  for (const auto* entidade : entidades) {
    delete entidade;
  }
}

TEST(TesteAcoes, TesteEntidadesAfetadasPorAcaoMaisFracosPrimeiro) {
  AcaoProto acao;
  acao.set_tipo(ACAO_DISPERSAO);
  acao.set_geometria(ACAO_GEO_ESFERA);
  acao.set_raio_quadrados(1);
  acao.set_total_dv(3);
  acao.set_mais_fracos_primeiro(true);
  std::vector<const Entidade*> entidades;
  int id = 1;
  for (int nivel : { 1, 2, 1, 1}) {
    EntidadeProto proto;
    proto.set_id(id++);
    auto* ic = proto.add_info_classes();
    ic->set_id("guerreiro");
    ic->set_nivel(nivel);
    entidades.push_back(NovaEntidadeParaTestes(proto, g_tabelas));
  }

  const std::vector<unsigned int> ids_afetados = EntidadesAfetadasPorAcao(acao, nullptr, entidades);
  EXPECT_THAT(ids_afetados, testing::ElementsAre(1, 3, 4));

  for (const auto* entidade : entidades) {
    delete entidade;
  }
}

TEST(TesteAcoes, TesteMaximoAfetados) {
  AcaoProto acao;
  acao.set_tipo(ACAO_DISPERSAO);
  acao.set_geometria(ACAO_GEO_ESFERA);
  acao.set_raio_quadrados(1);
  acao.set_maximo_criaturas_afetadas(2);
  std::vector<const Entidade*> entidades;
  for (int id = 0; id < 3; ++id) {
    EntidadeProto proto;
    proto.set_id(id);
    entidades.push_back(NovaEntidadeParaTestes(proto, g_tabelas));
  }

  const std::vector<unsigned int> ids_afetados = EntidadesAfetadasPorAcao(acao, nullptr, entidades);
  EXPECT_THAT(ids_afetados, testing::ElementsAre(0, 1));

  for (const auto* entidade : entidades) {
    delete entidade;
  }
}

TEST(TesteAcoes, TesteEfeitoAfetaApenas) {
  AcaoProto acao;
  auto* efeito = acao.add_efeitos_adicionais();
  efeito->set_afeta_apenas_dados_vida_menor_igual_a(5);
  efeito->set_afeta_apenas_dados_vida_maior_igual_a(4);
  std::map<int, bool> esperado = { {3, false}, {4, true}, {5, true}, {6, false}};
  for (int nivel = 3; nivel <= 6; ++nivel) {
    EntidadeProto proto;
    proto.add_info_classes()->set_nivel(nivel);
    EXPECT_EQ(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/20, *efeito, proto), esperado[nivel]) << "nivel: " << nivel;
  }
}

TEST(TesteAcoes, TesteEfeitoNaoAfeta) {
  AcaoProto acao;
  auto* efeito = acao.add_efeitos_adicionais();
  efeito->set_efeito(EFEITO_ATORDOADO);
  {
    EntidadeProto proto;
    proto.add_tipo_dnd(TIPO_HUMANOIDE);
    EXPECT_EQ(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/20, *efeito, proto), true);
  }
  {
    EntidadeProto proto;
    proto.add_tipo_dnd(TIPO_MORTO_VIVO);
    EXPECT_EQ(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/20, *efeito, proto), false);
  }
}

TEST(TesteAcoes, TesteEntidadeAfetadaPorEfeito) {
  auto modelo_druida = g_tabelas.ModeloEntidade("Halfling Druida 10");
  auto alvo_10 = NovaEntidadeParaTestes(modelo_druida.entidade(), g_tabelas);
  auto modelo_clerigo_bom_5 = g_tabelas.ModeloEntidade("Humano Clérigo 5 (Leal Bom)");
  auto alvo_5_bom = NovaEntidadeParaTestes(modelo_clerigo_bom_5.entidade(), g_tabelas);
  const auto& feitico_tabelado = g_tabelas.Feitico("palavra_sagrada");
  ASSERT_EQ(feitico_tabelado.acao().efeitos_adicionais_size(), 5);
  {
    // Surdez: nivel de conjurador.
    EXPECT_TRUE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/11, feitico_tabelado.acao().efeitos_adicionais(0), alvo_10->Proto()));
    EXPECT_TRUE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/10, feitico_tabelado.acao().efeitos_adicionais(0), alvo_10->Proto()));
    EXPECT_FALSE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/9, feitico_tabelado.acao().efeitos_adicionais(0), alvo_10->Proto()));
    // bom nao é afetado.
    EXPECT_FALSE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/20, feitico_tabelado.acao().efeitos_adicionais(0), alvo_5_bom->Proto()));
  }
  {
    // Cegueira: nivel de conjurador -1.
    EXPECT_TRUE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/12, feitico_tabelado.acao().efeitos_adicionais(1), alvo_10->Proto()));
    EXPECT_TRUE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/11, feitico_tabelado.acao().efeitos_adicionais(1), alvo_10->Proto()));
    EXPECT_FALSE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/10, feitico_tabelado.acao().efeitos_adicionais(1), alvo_10->Proto()));
    // bom nao é afetado.
    EXPECT_FALSE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/20, feitico_tabelado.acao().efeitos_adicionais(0), alvo_5_bom->Proto()));
  }
  {
    // Paralisia: nivel de conjurador -5.
    EXPECT_TRUE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/16, feitico_tabelado.acao().efeitos_adicionais(2), alvo_10->Proto()));
    EXPECT_TRUE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/15, feitico_tabelado.acao().efeitos_adicionais(2), alvo_10->Proto()));
    EXPECT_FALSE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/14, feitico_tabelado.acao().efeitos_adicionais(2), alvo_10->Proto()));
    // bom nao é afetado.
    EXPECT_FALSE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/20, feitico_tabelado.acao().efeitos_adicionais(0), alvo_5_bom->Proto()));
  }
  {
    // Morte: nivel de conjurador -10.
    EXPECT_TRUE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/21, feitico_tabelado.acao().efeitos_adicionais(3), alvo_10->Proto()));
    EXPECT_TRUE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/20, feitico_tabelado.acao().efeitos_adicionais(3), alvo_10->Proto()));
    EXPECT_FALSE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/19, feitico_tabelado.acao().efeitos_adicionais(3), alvo_10->Proto()));
    // bom nao é afetado.
    EXPECT_FALSE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/20, feitico_tabelado.acao().efeitos_adicionais(0), alvo_5_bom->Proto()));
  }
  {
    // Destruicao de morto vivo: nivel de conjurador -10.
    // Druida nao eh afetado.
    EXPECT_FALSE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/21, feitico_tabelado.acao().efeitos_adicionais(4), alvo_10->Proto()));
    auto modelo_morto_vivo_6 = g_tabelas.ModeloEntidade("Esqueleto (Lobo 6)");
    auto alvo_6 = NovaEntidadeParaTestes(modelo_morto_vivo_6.entidade(), g_tabelas);
    EXPECT_TRUE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/17, feitico_tabelado.acao().efeitos_adicionais(4), alvo_6->Proto()));
    EXPECT_TRUE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/16, feitico_tabelado.acao().efeitos_adicionais(4), alvo_6->Proto()));
    EXPECT_FALSE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/15, feitico_tabelado.acao().efeitos_adicionais(4), alvo_6->Proto()));
    // bom nao é afetado.
    EXPECT_FALSE(EntidadeAfetadaPorEfeito(g_tabelas, /*nivel_conjurador=*/20, feitico_tabelado.acao().efeitos_adicionais(0), alvo_5_bom->Proto()));
  }
}

}  // namespace ent.

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
