#include <memory>
#include "ent/entidade.h"
#include "ent/entidade.pb.h"
#include "ent/acoes.h"
#include "ent/acoes.pb.h"
#include "gtest/gtest.h"

namespace ent {

// Teste basico gera dados.
TEST(TesteAcoes, TesteAreaAfetada) {
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

}  // namespace ent.

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
