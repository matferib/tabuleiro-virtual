#include <memory>
#include "ent/entidade.h"
#include "ent/entidade.pb.h"
#include "ent/tabelas.h"
#include "gtest/gtest.h"

namespace ent {

namespace {
Tabelas g_tabelas(nullptr);
}  // namespace

TEST(TesteEntidade, TesteAtualizaAcao) {
  std::unique_ptr<Entidade> e(NovaEntidadeParaTestes(EntidadeProto(), g_tabelas));
  e->AdicionaAcaoExecutada("teste1");
  EXPECT_EQ("teste1", e->TipoAcaoExecutada(0, {}));
  e->AdicionaAcaoExecutada("teste2");
  EXPECT_EQ("teste2", e->TipoAcaoExecutada(0, {}));
  EXPECT_EQ("teste1", e->TipoAcaoExecutada(1, {}));
  EXPECT_EQ("", e->TipoAcaoExecutada(2, {}));
  e->AdicionaAcaoExecutada("teste1");
  EXPECT_EQ("teste1", e->TipoAcaoExecutada(0, {}));
  EXPECT_EQ("teste2", e->TipoAcaoExecutada(1, {}));
  EXPECT_EQ("", e->TipoAcaoExecutada(2, {}));
  e->AdicionaAcaoExecutada("teste3");
  EXPECT_EQ("teste3", e->TipoAcaoExecutada(0, {}));
  EXPECT_EQ("teste1", e->TipoAcaoExecutada(1, {}));
  EXPECT_EQ("teste2", e->TipoAcaoExecutada(2, {}));
  EXPECT_EQ("", e->TipoAcaoExecutada(3, {}));
  e->AdicionaAcaoExecutada("teste4");
  EXPECT_EQ("teste4", e->TipoAcaoExecutada(0, {}));
  EXPECT_EQ("teste3", e->TipoAcaoExecutada(1, {}));
  EXPECT_EQ("teste1", e->TipoAcaoExecutada(2, {}));
  EXPECT_EQ("", e->TipoAcaoExecutada(3, {}));
}

}  // namespace ent.

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
