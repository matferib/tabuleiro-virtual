#ifndef SOM_SOM_H
#define SOM_SOM_H

#include <string>
#include "ent/tabuleiro.pb.h"

namespace som {

void Inicia(const ent::OpcoesProto& opcoes);
// Apenas arquivos wav permitidos.
void Toca(const std::string& nome);
// Permite wav e ogg.
void TocaSomFundo(const std::string& nome);
void ParaSomFundo(const std::string& nome);
void Finaliza();

}  // namespace som

#endif
