#ifndef SOM_SOM_H
#define SOM_SOM_H

#include <string>
#include "ent/tabuleiro.pb.h"

namespace som {

void Inicia(const ent::OpcoesProto& opcoes);
void Toca(const std::string& nome);
void Finaliza();

}  // namespace som

#endif
