#include <set>
#include <stack>
#include <string>

#include "arq/arquivo.h"
#include "ent/tabelas.h"
#include "ent/tabuleiro.pb.h"
#include "ifg/modelos.pb.h"
#include "gtest/gtest.h"

// Teste quer verifica que todos os modelos no menu tem correspondencia em modelos.
TEST(TesteModelos, TesteModelos) {
  ent::Tabelas tabelas(nullptr);

  std::stack<const ifg::MenuModelos*> pilha;
  pilha.push(&tabelas.MenuModelos());
  do {
    const auto* menu = pilha.top();
    for (const auto& item : menu->item_menu()) {
      // verifica os ids.
      if (item.id() == "Padrão") {
        continue;
      }
      if (item.modelos().empty()) {
        if (!item.id().empty()) {
          EXPECT_FALSE(tabelas.ModeloEntidade(item.id()).id().empty()) << "id de modelo invalido para item.id: " << item.id();
        } else {
          EXPECT_FALSE(tabelas.ItemMenu(item.id_item_menu()).id().empty()) << "id de item de modelo invalido para item.id_item_menu: " << item.id_item_menu();
        }
      } else {
        for (const auto& modelo : item.modelos()) {
          if (modelo.id() != "NADA") {
            if (!modelo.id().empty()) {
              EXPECT_FALSE(tabelas.ModeloEntidade(modelo.id()).id().empty()) << ", id: " << modelo.id() << " não encontrado dentro de modelos de " << item.id();
            } else {
              EXPECT_FALSE(tabelas.ItemMenu(modelo.id_item_menu()).id().empty()) << ", id: " << modelo.id_item_menu() << " não encontrado dentro de modelos de " << item.id();
            }
          }
        }
      }
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
