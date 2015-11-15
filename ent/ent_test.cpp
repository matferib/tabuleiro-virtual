#include <memory>
#include "ent/entidade.h"
#include "ent/entidade.pb.h"
#include "gtest/gtest.h"

namespace ent {

// Teste basico gera dados.
TEST(TesteEntidade, TesteAtualizaAcao) {
  std::unique_ptr<Entidade> e(NovaEntidade(EntidadeProto(), nullptr, nullptr, nullptr));
  e->AdicionaAcaoExecutada("teste1");
  EXPECT_EQ("teste1", e->AcaoExecutada(0));
  e->AdicionaAcaoExecutada("teste2");
  EXPECT_EQ("teste2", e->AcaoExecutada(0));
  EXPECT_EQ("teste1", e->AcaoExecutada(1));
  EXPECT_EQ("", e->AcaoExecutada(2));
  e->AdicionaAcaoExecutada("teste1");
  EXPECT_EQ("teste1", e->AcaoExecutada(0));
  EXPECT_EQ("teste2", e->AcaoExecutada(1));
  EXPECT_EQ("", e->AcaoExecutada(2));
  e->AdicionaAcaoExecutada("teste3");
  EXPECT_EQ("teste3", e->AcaoExecutada(0));
  EXPECT_EQ("teste1", e->AcaoExecutada(1));
  EXPECT_EQ("teste2", e->AcaoExecutada(2));
  EXPECT_EQ("", e->AcaoExecutada(3));
  e->AdicionaAcaoExecutada("teste4");
  EXPECT_EQ("teste4", e->AcaoExecutada(0));
  EXPECT_EQ("teste3", e->AcaoExecutada(1));
  EXPECT_EQ("teste1", e->AcaoExecutada(2));
  EXPECT_EQ("", e->AcaoExecutada(3));
}

}  // namespace ent.

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
