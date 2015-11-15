#include <memory>
#include "ent/entidade.h"
#include "ent/entidade.pb.h"
#include "gtest/gtest.h"

namespace ent {

// Teste basico gera dados.
TEST(TesteEntidade, TesteAtualizaAcao) {
  std::unique_ptr<Entidade> e(NovaEntidade(EntidadeProto(), nullptr, nullptr, nullptr));
  const EntidadeProto& ep = e->Proto();
  e->AtualizaAcao("teste1");
  EXPECT_EQ("teste1", e->Acao());
  e->AtualizaAcao("teste2");
  EXPECT_EQ("teste2", e->Acao());
  e->AtualizaAcao("teste1");
  EXPECT_EQ(2, ep.ultima_acao_size());
  EXPECT_EQ("teste1", ep.ultima_acao(0));
  EXPECT_EQ("teste2", ep.ultima_acao(1));
  e->AtualizaAcao("teste3");
  EXPECT_EQ(3, ep.ultima_acao_size());
  EXPECT_EQ("teste3", ep.ultima_acao(0));
  EXPECT_EQ("teste1", ep.ultima_acao(1));
  EXPECT_EQ("teste2", ep.ultima_acao(2));
  e->AtualizaAcao("teste4");
  EXPECT_EQ(3, ep.ultima_acao_size());
  EXPECT_EQ("teste4", ep.ultima_acao(0));
  EXPECT_EQ("teste3", ep.ultima_acao(1));
  EXPECT_EQ("teste1", ep.ultima_acao(2));
}

}  // namespace ent.

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
