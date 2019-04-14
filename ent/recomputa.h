#ifndef ENT_RECOMPUTA_H
#define ENT_RECOMPUTA_H

#include "ent/entidade.h"
#include "ent/entidade.pb.h"
#include "ent/tabelas.h"
#include "ent/tabelas.pb.h"

// Recomputacao de dependencias do proto.

namespace ent {

// Recomputa as dependencias do proto.
void RecomputaDependencias(const Tabelas& tabelas, EntidadeProto* proto, Entidade* entidade = nullptr);

}  // namespace ent

#endif
