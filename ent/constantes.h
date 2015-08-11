#ifndef ENT_CONSTANTES_H
#define ENT_CONSTANTES_H

/** Constantes comuns ao tabuleiro e entidades. */
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

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
#define NUM_FACES 8
/** Numero de divisoes do eixo Z do cone. */
#define NUM_LINHAS 1

#define ARQUIVO_MODELOS "modelos.asciiproto"
#define ARQUIVO_MODELOS_NAO_SRD "modelos_nao_srd.asciiproto"
#define ARQUIVO_ACOES "acoes.asciiproto"

#if USAR_OPENGL_ES
#define ATUALIZACOES_POR_SEGUNDO 30.0f
#else
#define ATUALIZACOES_POR_SEGUNDO 60.0f
#endif
#define INTERVALO_NOTIFICACAO_MS (1000.0f / ATUALIZACOES_POR_SEGUNDO)
// Converte a velocidade por segundo para distancia por atualizacao.
#define POR_SEGUNDO_PARA_ATUALIZACAO (1.0f / ATUALIZACOES_POR_SEGUNDO)

// Taxa de envio de movimentos parciais para clientes.
#define CICLOS_PARA_ATUALIZAR_MOVIMENTOS_PARCIAIS (ATUALIZACOES_POR_SEGUNDO / 3)

// Por quantos frames um botao fica pressionado. 240ms.
#define ATUALIZACOES_BOTAO_PRESSIONADO (static_cast<int>(ATUALIZACOES_POR_SEGUNDO * 0.240f))

// Constantes para picking do tabuleiro.
#define OBJ_TABULEIRO 1
#define OBJ_ENTIDADE 2
#define OBJ_ROLAGEM 3
#define OBJ_CONTROLE_VIRTUAL 4
#define OBJ_EVENTO_ENTIDADE 5

// Constantes de cenario.
#define CENARIO_PRINCIPAL -1
#define CENARIO_INVALIDO -2

// Nao ha um ENTIDADE_ID_INVALIDO pq a constante esta no proto: Entidade::IdInvalido.

namespace ent {

extern const float SEN_60;
extern const float SEN_30;
extern const float COS_60;
extern const float COS_30;
extern const float COR_PRETA[];
extern const float COR_BRANCA[];
extern const float COR_CINZA[];
extern const float COR_CINZA_CLARO[];
extern const float COR_VERMELHA[];
extern const float COR_VERDE[];
extern const float COR_AZUL[];
extern const float COR_AZUL_ALFA[];
extern const float COR_AMARELA[];

}  // namespace ent

#endif  // ENT_CONSTANTES_H
