#include <cmath>
#include "ent/constantes.h"

// Constantes que precisam de definicao.

namespace ent {

const double SEN_60 = sin(M_PI / 3.0);
const double SEN_30 = sin(M_PI / 6.0);
const double COS_60 = cos(M_PI / 3.0);
const double COS_30 = cos(M_PI / 6.0);

const float COR_BRANCA[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const float COR_PRETA[]    = { 0.0f, 0.0f, 0.0f, 1.0f };
const float COR_VERMELHA[] = { 1.0f, 0.0f, 0.0f, 1.0f };
const float COR_VERDE[]    = { 0.0f, 1.0f, 0.0f, 1.0f };
const float COR_AZUL[]     = { 0.0f, 0.0f, 1.0f, 1.0f };
const float COR_AZUL_ALFA[]     = { 0.0f, 0.0f, 1.0f, 0.5f };
const float COR_AMARELA[]  = { 1.0f, 1.0f, 0.0f, 1.0f };

}  // namespace ent
