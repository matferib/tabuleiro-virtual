#ifndef ENT_CONSTANTES_H
#define ENT_CONSTANTES_H

/** Constantes comuns ao tabuleiro e entidades. */

/** tamanho do lado do quadrado no 3D. */
#define TAMANHO_LADO_QUADRADO 1.5f
#define TAMANHO_LADO_QUADRADO_EXP_2 (TAMANHO_LADO_QUADRADO * TAMANHO_LADO_QUADRADO)
/** tamanho do lado do quadrado / 2. */
#define TAMANHO_LADO_QUADRADO_2 (TAMANHO_LADO_QUADRADO / 2.0f)
/** tamanho do lado do quadrado / 10. */
#define TAMANHO_LADO_QUADRADO_10 (TAMANHO_LADO_QUADRADO / 10.0f)
/** Altura dos objetos. */
#define ALTURA TAMANHO_LADO_QUADRADO
/** Altura do voo dos objetos. */
#define ALTURA_VOO ALTURA
/** Numero de faces do cone. */
#define NUM_FACES 10
/** Numero de divisoes do eixo Z do cone. */
#define NUM_LINHAS 1

#define DIR_DADOS "dados" 
#define ARQUIVO_MODELOS "modelos.asciiproto"
#define ARQUIVO_ACOES "acoes.asciiproto"

namespace ent {

extern const float SEN_60;
extern const float SEN_30;
extern const float COS_60;
extern const float COS_30;
extern const float COR_PRETA[];
extern const float COR_BRANCA[];
extern const float COR_VERMELHA[];
extern const float COR_VERDE[];
extern const float COR_AZUL[];
extern const float COR_AZUL_ALFA[];
extern const float COR_AMARELA[];

}  // namespace ent

#endif  // ENT_CONSTANTES_H
