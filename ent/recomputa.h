#ifndef ENT_RECOMPUTA_H
#define ENT_RECOMPUTA_H

#include "ent/entidade.h"
#include "ent/entidade.pb.h"
#include "ent/tabelas.h"
#include "ent/tabelas.pb.h"

// Recomputacao de dependencias do proto.

namespace ent {

typedef std::unordered_map<unsigned int, std::unique_ptr<Entidade>> MapaEntidades;

// Recomputa as dependencias do proto.
void RecomputaDependencias(
    const Tabelas& tabelas, TipoTerreno tipo_terreno, EntidadeProto* proto,
    Entidade* entidade = nullptr, const MapaEntidades* mapa_entidades = nullptr);

}  // namespace ent

#endif
