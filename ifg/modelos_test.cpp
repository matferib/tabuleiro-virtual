#include <set>
#include <stack>
#include <string>

#include "arq/arquivo.h"
#include "ent/tabuleiro.pb.h"
#include "ifg/modelos.pb.h"
#include "gtest/gtest.h"

// Teste para verificacao dosmodelos.
TEST(TesteModelos, TesteModelos) {
  ent::Modelos ent_modelos;
  LeArquivoAsciiProto(arq::TIPO_DADOS, "modelos.asciiproto", &ent_modelos);
  ent::Modelos ent_modelos_nao_srd;
  LeArquivoAsciiProto(arq::TIPO_DADOS, "modelos_nao_srd.asciiproto", &ent_modelos_nao_srd);
  ent_modelos.MergeFrom(ent_modelos_nao_srd);
  std::set<std::string> ids_modelos;
  for (const auto& m : ent_modelos.modelo()) {
    ids_modelos.insert(m.id());
  }

  ifg::MenuModelos menu_modelos;
  LeArquivoAsciiProto(arq::TIPO_DADOS, "menumodelos.asciiproto", &menu_modelos);
  ifg::MenuModelos menu_modelos_nao_srd;
  LeArquivoAsciiProto(arq::TIPO_DADOS, "menumodelos_nao_srd.asciiproto", &menu_modelos_nao_srd);
  menu_modelos.MergeFrom(menu_modelos_nao_srd);

  std::stack<const ifg::MenuModelos*> pilha;
  pilha.push(&menu_modelos);
  do {
    const auto* menu = pilha.top();
    for (const auto& m : menu->modelo()) {
      // verifica os ids.
      if (m.id() == "Padrão") {
        continue;
      }
      EXPECT_NE(ids_modelos.end(), ids_modelos.find(m.id())) << ", id: " << m.id();
    }
    pilha.pop();
    for (const auto& sub_menu : menu->sub_menu()) {
      pilha.push(&sub_menu);
    }
  } while (!pilha.empty());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
