#include <set>
#include <stack>
#include <string>

#include "arq/arquivo.h"
#include "gtest/gtest.h"

// Teste para verificacao de arquivo. Para funcionar, deve-se criar o arquivo de teste no diretorio de usuario + teste.
TEST(TesteModelos, TesteModelos) {
  arq::Inicializa();
  const std::vector<std::string> listagem(arq::ConteudoDiretorio(arq::TIPO_TESTE));
  EXPECT_FALSE(listagem.empty());
  std::string dados;
  arq::LeArquivo(arq::TIPO_TESTE, "teste.txt", &dados);
  EXPECT_GT(dados.size(), 0);
  arq::LeArquivo(arq::TIPO_DADOS, "modelos.asciiproto", &dados);
  EXPECT_GT(dados.size(), 0);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
