#include <memory>
#include "ent/entidade.h"
#include "ent/entidade.pb.h"
#include "ent/acoes.h"
#include "ent/acoes.pb.h"
#include "gtest/gtest.h"
#include "log/log.h"

namespace ent {

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
  EXPECT_TRUE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto));

  acao_proto.mutable_pos_tabuleiro()->set_x(-10.0f);
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto));

  acao_proto.mutable_pos_tabuleiro()->set_x(10.0f);
  acao_proto.set_distancia_quadrados(3);
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto));

  acao_proto.set_distancia_quadrados(4);
  ponto.set_y(1.0f);
  EXPECT_TRUE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto));

  // Dentro do alcance, fora do angulo do cone.
  ponto.set_y(3.1f);
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto));
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
  EXPECT_TRUE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto));

  // Ponto a frente, menos de meio quadrado ao lado.
  ponto.set_x(1.5f);
  ponto.set_y(2.0f);
  EXPECT_TRUE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto));

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
    EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao));
  }

  // Mais de meio quadrado.
  ponto.set_x(1.8f);
  ponto.set_y(2.0f);
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto));

  // Ponto fora do alcance do raio.
  ponto.set_x(1.0f);
  ponto.set_y(16.0f);
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto));

  // Ponto atras do raio.
  ponto.set_x(1.0f);
  ponto.set_y(-1.0f);
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto));

  // Ponto fora da linha do raio.
  ponto.set_x(5.0f);
  ponto.set_y(5.0f);
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto));
  ponto.set_x(-5.0f);
  ponto.set_y(5.0f);
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto));
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
  EXPECT_FALSE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto));

  ponto.set_x(5.4f);
  EXPECT_TRUE(Acao::PontoAfetadoPorAcao(ponto, origem, acao_proto));
}

TEST(TesteAcoes, TesteAjustePontoDispersao) {
  Posicao ponto;
  ponto.set_x(5.0);
  Posicao origem;
  AcaoProto acao_proto;
  acao_proto.set_tipo(ACAO_DISPERSAO);
  acao_proto.set_geometria(ACAO_GEO_ESFERA);
  acao_proto.set_efeito_area(true);
  acao_proto.set_raio_quadrados(3);  // 4.5m
  acao_proto.mutable_pos_tabuleiro()->set_x(1.0f);
  Posicao ponto_ajustado = Acao::AjustaPonto(ponto, 1.0f, origem, acao_proto);

  EXPECT_FLOAT_EQ(4.25, ponto_ajustado.x());
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

}  // namespace ent.

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
