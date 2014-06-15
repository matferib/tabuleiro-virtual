#include <cmath>
#include "ent/constantes.h"

// Constantes que precisam de definicao.

namespace ent {

const float SEN_60 = sinf(M_PI / 3.0f);
const float SEN_30 = sinf(M_PI / 6.0f);
const float COS_60 = cosf(M_PI / 3.0f);
const float COS_30 = cosf(M_PI / 6.0f);

const float COR_BRANCA[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const float COR_PRETA[]    = { 0.0f, 0.0f, 0.0f, 1.0f };
const float COR_VERMELHA[] = { 1.0f, 0.0f, 0.0f, 1.0f };
const float COR_VERDE[]    = { 0.0f, 1.0f, 0.0f, 1.0f };
const float COR_AZUL[]     = { 0.0f, 0.0f, 1.0f, 1.0f };
const float COR_AZUL_ALFA[]     = { 0.0f, 0.0f, 1.0f, 0.5f };
const float COR_AMARELA[]  = { 1.0f, 1.0f, 0.0f, 1.0f };

}  // namespace ent
