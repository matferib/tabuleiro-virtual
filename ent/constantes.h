#ifndef ENT_CONSTANTES_H
#define ENT_CONSTANTES_H

/** Constantes comuns ao tabuleiro e entidades. */

/** tamanho do lado do quadrado no 3D. */
#define TAMANHO_LADO_QUADRADO 1.5
/** tamanho do lado do quadrado / 2. */
#define TAMANHO_LADO_QUADRADO_2 (TAMANHO_LADO_QUADRADO / 2.0)

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

#define RAD_PARA_GRAUS (180.0f / M_PI)
#define GRAUS_PARA_RAD (M_PI / 180.0f)

#define DIR_DADOS "dados" 
#define ARQUIVO_MODELOS "modelos.asciiproto"
#define ARQUIVO_ACOES "acoes.asciiproto"

// Identificador da acao de sinalizacao.
#define ACAO_SINALIZACAO "Sinalização"
// As acoes de baixo devem bater com as do arquivo de acoes.
#define ACAO_MISSIL_MAGICO "Míssil Mágico"

#endif
