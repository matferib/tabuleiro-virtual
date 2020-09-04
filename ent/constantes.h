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
/** Altura da posicao da acao, mais baixo que o objeto. */
#define FATOR_ALTURA 0.6f
#define ALTURA_ACAO (ALTURA * FATOR_ALTURA)
/** Numero de faces do cone. */
#define NUM_FACES 8
/** Numero de divisoes do eixo Z do cone. */
#define NUM_LINHAS 1

/** Os clipping planes. Isso afeta diretamente a precisao do Z buffer. */
#if ZBUFFER_16_BITS
#define DISTANCIA_PLANO_CORTE_PROXIMO 2.0f
#define DISTANCIA_PLANO_CORTE_DISTANTE 80.0f
#else
#define DISTANCIA_PLANO_CORTE_PROXIMO 0.5f
#define DISTANCIA_PLANO_CORTE_DISTANTE 160.0f
#endif
#define DISTANCIA_PLANO_CORTE_PROXIMO_PRIMEIRA_PESSOA 0.1f

#define ARQUIVO_MODELOS "modelos.asciiproto"
#define ARQUIVO_MODELOS_NAO_SRD "modelos_nao_srd.asciiproto"
#define ARQUIVO_MODELOS_HOMEBREW "modelos_homebrew.asciiproto"
#define ARQUIVO_ACOES "acoes.asciiproto"

// Constantes para picking do tabuleiro. Limite: TODO
#define OBJ_INVALIDO 0
#define OBJ_TABULEIRO 1
#define OBJ_ENTIDADE 2
#define OBJ_ENTIDADE_LISTA 3
#define OBJ_ROLAGEM 4
#define OBJ_CONTROLE_VIRTUAL 5
#define OBJ_EVENTO_ENTIDADE 6

// Constantes de cenario.
#define CENARIO_PRINCIPAL -1
#define CENARIO_INVALIDO -2

#define INICIATIVA_INVALIDA (-1000)

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
extern const float COR_LARANJA[];
extern const float COR_MARROM[];

// Offset de profundidade para formas 2d sobre tabuleiro.
constexpr float OFFSET_RASTRO_ESCALA_DZ  = -2.0f;
constexpr float OFFSET_RASTRO_ESCALA_R  = -20.0f;
constexpr float OFFSET_FORMAS_2D_ESCALA_DZ  = -4.0f;
constexpr float OFFSET_FORMAS_2D_ESCALA_R  = -40.0f;

constexpr float CAMPO_VISAO_PADRAO = 70.0f;
constexpr float CAMPO_VISAO_MIN = 50.0f;
constexpr float CAMPO_VISAO_MAX = 90.0f;

/** Tempo que o detalhamento mostra os detalhes no hover. */
constexpr int TEMPO_DETALHAMENTO_MS = 500;
/** Tempo entre mensagens. */
constexpr float TEMPO_ENTRE_MENSAGENS_S = 2.0f;

/** Distancia minima entre pontos no desenho livre. */
constexpr float DELTA_MINIMO_DESENHO_LIVRE = TAMANHO_LADO_QUADRADO / 2.0f;

/** A Translacao e a rotacao de objetos so ocorre depois que houver essa distancia de pixels percorrida pelo mouse. */
constexpr int DELTA_MINIMO_TRANSLACAO_ROTACAO = 5;

constexpr const char* ID_ACAO_ATAQUE_CORPO_A_CORPO = "Ataque Corpo a Corpo";

/** sensibilidade da rodela do mouse. */
constexpr double SENSIBILIDADE_RODA = 0.01;
/** sensibilidade da rotacao lateral do olho. */
constexpr double SENSIBILIDADE_ROTACAO_X = 0.01;
/** sensibilidade da altura do olho. */
constexpr double SENSIBILIDADE_ROTACAO_Y = 0.08;

/** altura inicial do olho. */
constexpr double OLHO_ALTURA_INICIAL = 10.0;
/** altura maxima do olho. */
constexpr double OLHO_ALTURA_MAXIMA = 90.0;
/** altura minima do olho. */
constexpr double OLHO_ALTURA_MINIMA = 0.3;

/** raio (distancia) inicial do olho. */
constexpr double OLHO_RAIO_INICIAL = 20.0;
/** raio maximo do olho. */
constexpr double OLHO_RAIO_MAXIMO = 40.0;
/** raio minimo do olho. */
constexpr double OLHO_RAIO_MINIMO = 1.5;

/** Distancia maxima do olho da entidade de referencia. */
constexpr float OLHO_DISTANCIA_MAXIMA_CAMERA_PRESA = 5.0f * TAMANHO_LADO_QUADRADO;

constexpr int MINUTOS_PARA_RODADAS = 10;
constexpr int HORAS_PARA_RODADAS = 60 * MINUTOS_PARA_RODADAS;
constexpr float QUADRADOS_PARA_METROS = 1.5f;
constexpr float METROS_PARA_QUADRADOS = 1.0f / 1.5f;
constexpr int DIV_NANO_PARA_MS = 1000000ULL;
constexpr int DIV_NANO_PARA_SEGUNDOS = 1000000000ULL;

constexpr int MINUTOS_EM_RODADAS = 10;
constexpr int HORAS_EM_RODADAS = 60 * MINUTOS_EM_RODADAS;
constexpr int DIA_EM_RODADAS = HORAS_EM_RODADAS * 24;

constexpr float DISTANCIA_LUZ_DIRECIONAL_METROS = 150.0f;

}  // namespace ent

#endif  // ENT_CONSTANTES_H
