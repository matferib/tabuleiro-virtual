#ifndef ENT_CONSTANTES_H
#define ENT_CONSTANTES_H

#if __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

/** Constantes comuns ao tabuleiro e entidades. */

/** tamanho do lado do quadrado no 3D. */
#define TAMANHO_LADO_QUADRADO 1.5
/** tamanho do lado do quadrado / 2. */
#define TAMANHO_LADO_QUADRADO_2 (TAMANHO_LADO_QUADRADO / 2.0)
/** tamanho do lado do quadrado / 10. */
#define TAMANHO_LADO_QUADRADO_10 (TAMANHO_LADO_QUADRADO / 10.0)

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

#define RAD_PARA_GRAUS (180.0f / M_PI)
#define GRAUS_PARA_RAD (M_PI / 180.0f)

#define DIR_DADOS "dados" 
#define ARQUIVO_MODELOS "modelos.asciiproto"
#define ARQUIVO_ACOES "acoes.asciiproto"

namespace ent {

extern const double SEN_60;
extern const double SEN_30;
extern const double COS_60;
extern const double COS_30;
extern const GLfloat COR_PRETA[];
extern const GLfloat COR_BRANCA[];
extern const GLfloat COR_VERMELHA[];
extern const GLfloat COR_VERDE[];
extern const GLfloat COR_AZUL[];
extern const GLfloat COR_AMARELA[];

/** Altera a cor correnta para cor. Nao considera alpha. */
void MudaCor(const GLfloat* cor);
/** Considera alpha. */
void MudaCorAlfa(const GLfloat* cor);

}  // namespace ent

#endif  // ENT_CONSTANTES_H
