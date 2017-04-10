#ifndef ENT_UTIL_H
#define ENT_UTIL_H

#include <algorithm>
#include <cctype>
#include <functional>
#include <string>
#include <tuple>
#include <vector>
#include <google/protobuf/repeated_field.h>
#include "matrix/matrices.h"
#include "ntf/notificacao.pb.h"

// Funcoes uteis de ent.
namespace ent {

class Cor;
class Entidade;
class EntidadeProto_Evento;
class ParametrosDesenho;
class Posicao;
class Tabuleiro;

void IniciaUtil();

/** Altera a cor correnta para cor. Nao considera alpha. */
void MudaCor(const float* cor);
void MudaCorAplicandoNevoa(const float* cor, const ParametrosDesenho* pd);

/** Liga a nevoa com os parametros passados, preenchendo pd. Posicao deve ser em coordenadas de mundo, que sera convertida em coordenadas
* de olho.
*/
void ConfiguraNevoa(float min, float max, float r, float g, float b, float* pos, ParametrosDesenho* pd);

/** Considera alpha. */
void MudaCorAlfa(const float* cor);

/** Outra forma de mudar a cor. */
void MudaCor(const Cor& cor);

/** Preenche proto_cor com cor. A entrada deve ter 4 componentes. */
void CorAlfaParaProto(const float* cor, Cor* cor_proto);

/** Igual CorAlfaParaProto mas para 3 componentes. */
void CorParaProto(const float* cor, Cor* cor_proto);

/** Reduz cada componente RGB em 0.5f. */
const Cor EscureceCor(const Cor& cor);
void EscureceCor(Cor* cor);
void EscureceCor(float* cor);

/** Aumenta cada componente RGB em 0.5f. */
void ClareiaCor(Cor* cor);

/** Muda cada componente RGB de forma notavel. */
void RealcaCor(Cor* cor);
void RealcaCor(float* cor);

// retorna true se o tipo da forma for 2d: circulo, retangulo, triangulo, livre.
bool EhForma2d(int tipo_forma);

/** Desenha uma linha 3d com a largura passada, passando pelos pontos. Em cada ponto, sera desenhado um disco para conectar. */
void DesenhaLinha3d(const std::vector<Posicao>& pontos, float largura);
void DesenhaLinha3d(const google::protobuf::RepeatedPtrField<Posicao>& pontos, float largura);
void LimitesLinha3d(const google::protobuf::RepeatedPtrField<Posicao>& pontos, float largura, float* xi, float* yi, float *xs, float *ys);

/** Funcoes para ligar o stencil e depois desenhar a cor passada (ou a corrente) onde o stencil foi marcado. */
// ATENCAO: Esse retangulo acaba com a operacao de picking (porque escreve na tela toda). Operacoes de picking nao devem usar stencil.
void LigaStencil();
/** O stencil 2d desenha um retangulo sobre a tela inteira. */
void DesenhaStencil2d(const float* cor = nullptr);
void DesenhaStencil2d(const Cor& cor);
/** O stencil 3d desenha um retangulo paralelo ao tabuleiro. */
void DesenhaStencil3d(float tam_x, float tam_y, const Cor& cor);
void DesenhaStencil3d(float tam_x, float tam_y, const float* cor = nullptr);
void DesenhaStencil3d(float xi, float yi, float xs, float ys, const float* cor = nullptr);

/** Gera um aleatorio de 1 a nfaces. */
int RolaDado(unsigned int nfaces);
/** Gera um aleatorio entre [0.0 e 1.0]. Os valores tem precisao de duas casas. */
float Aleatorio();

/** Adiciona o delta ao dados_vida. */
void AtualizaStringDadosVida(int delta, std::string* dados_vida);

/** Gera pontos de vida baseado nos dados de vida, da forma 4d8+8 por exemplo.
* O valor de retorno é o total, e um vetor com o valor de cada dado rolado (o primeiro elemento eh o numero de faces, o segundo eh o valor).
* Da excecao se dados_vida for mal formado.
*/
std::tuple<int, std::vector<std::pair<int, int>>> GeraPontosVida(const std::string& dados_vida);

/** Converte a resposta para string. */
std::string DadosParaString(int total, std::vector<std::pair<int, int>>& dados);

// Le o resultado de um dano de uma arma.
// Exemplos:
// 1d8+5 (19-20/x3) retorna: 1d8+5, 19, 3.
// 1d8 (x3) retorna: 1d8, 20, 3.
// 1d8 (19-20) retorna: 1d8, 19, 2.
// 1d8 retorna: 1d8, 20, 2.
struct DanoArma {
  std::string dano;
  int margem_critico;
  int multiplicador;
};
DanoArma LeDanoArma(const std::string& dano);

/** Gera o maximo de pontos de vida baseado no dados_vida.
* Da excecao se dados_vida for mal formado.
*/
int GeraMaxPontosVida(const std::string& dados_vida);

/** Computa pos2 - pos1 em pos_res. */
void ComputaDiferencaVetor(const Posicao& pos2, const Posicao& pos1, Posicao* pos_res);

/** Computa pos2 + pos1 em pos_res. */
void ComputaSomaVetor(const Posicao& pos2, const Posicao& pos1, Posicao* pos_res);

/** Computa pos_res = escala * pos. */
void ComputaMultiplicacaoEscalar(float escala, const Posicao& pos, Posicao* pos_res);

/** Computa o vetor normalizado. */
void ComputaVetorNormalizado(Posicao* pos);

/* Multiplica a matriz openGL pelo vetor. A matriz OpenGL tem formato col x linha (column major), portanto,
* ao inves de multiplicar matriz (4x4) pelo vetor (4x1), fazemos a inversao: vetor (1x4) pela matriz (4x4).
*/
void MultiplicaMatrizVetor(const float* matriz, float* vetor);

/** Retorna o vetor de rotacao dado um vetor x,y. O valor do vetor vai de (-180, 180]. */
float VetorParaRotacaoGraus(const Posicao& vetor, float* tamanho = nullptr);
float VetorParaRotacaoGraus(float x, float y, float* tamanho = nullptr);

/** Matriz de rotacao para se chegar ao vetor v a partir do eixo X. */
Matrix4 MatrizRotacao(const Vector3& v);

/** @return quadrado da distancia entre as posicoes. */
float DistanciaQuadrado(const Posicao& pos1, const Posicao& pos2);

/** Roda o vetor no eixo Z. */
void RodaVetor2d(float graus, Posicao* vetor);

/** Retorna true se o ponto estiver dentro do poligono. */
bool PontoDentroDePoligono(const Posicao& ponto, const std::vector<Posicao>& vertices);

/** Posicionamento do raster em 2d. */
void PosicionaRaster2d(int x, int y);

// Tipos de efeitos possiveis.
enum efeitos_e {
  EFEITO_INVALIDO = -1,
  EFEITO_BORRAR = 0,
  EFEITO_REFLEXOS = 1,  // complemento: numero de imagens.
  EFEITO_PISCAR = 2,
};

/** Realiza a leitura de uma string de eventos, um por linha, formato:
* descricao [(complemento)] : rodadas.
*/
google::protobuf::RepeatedPtrField<EntidadeProto_Evento> LeEventos(const std::string& eventos_str);

/** Converte uma string para o efeito, se houver. Caso contrario retorna EFEITO_INVALIDO. */
efeitos_e StringParaEfeito(const std::string& s);

// Trim functions from: http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring.
static inline std::string& ltrim(std::string& s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

// trim from end
static inline std::string& rtrim(std::string& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  return s;
}

// trim from both ends
static inline std::string& trim(std::string& s) {
  return ltrim(rtrim(s));
}

const std::string StringSemUtf8(const std::string& id_acao);

// Move o delta para uma entidade, garantindo que ela termine acima do solo.
void MoveDeltaRespeitandoChao(float dx, float dy, float dz, const Tabuleiro& tabuleiro, Entidade* entidade);

bool EhPng(const std::string& textura);
bool EhIcone(const std::string& textura);
bool EhTerreno(const std::string& textura);
bool EhCaixaCeu(const std::string& textura);
bool EhModelo3d(const std::string& textura);
// Filtros uteis.
bool FiltroModelo3d(const std::string& textura);
bool FiltroTexturaEntidade(const std::string& textura);
bool FiltroTexturaCaixaCeu(const std::string& textura);
bool FiltroTexturaTabuleiro(const std::string& textura);

// Blend de entidades compostas e modelos3d. O blend deve ser restaurado para o valor padrao depois para nao
// avacalhar os proximos.
class AlteraBlendEscopo {
 public:
  // O valor alfa, se menor que 1.0, sera usado na transparencia no lugar do alfa_translucidos de pd.
  explicit AlteraBlendEscopo(const ParametrosDesenho* pd, const Cor& cor)
      : pd_(pd), restaurar_(AlteraBlendEntidadeComposta(pd, cor)) {}
  ~AlteraBlendEscopo() { if (restaurar_) RestauraBlend(pd_); }

 private:
  bool AlteraBlendEntidadeComposta(const ParametrosDesenho* pd, const Cor& cor) const;
  void RestauraBlend(const ParametrosDesenho* pd) const;

  const ParametrosDesenho* pd_;
  bool restaurar_;
};

// Retorna alguns modificadores de ataque para a entidade de acordo com seus status e do defensor.
int ModificadorAtaque(bool distancia, const EntidadeProto& ea, const EntidadeProto& ed);
// Retorna alguns modificadores de dano genericos para a entidade de acordo com seus status.
int ModificadorDano(const EntidadeProto& ea);

// Rola o dado de ataque vs defesa, retornando o numero de vezes que o dano deve ser aplicado e o texto da jogada.
// O ultimo parametro indica se a acao deve ser desenhada (em caso de distancia maxima atingida, retorna false).
// Caso haja falha critica, retorna vezes = -1;
// Posicao ataque eh para calculo de distancia.
std::tuple<int, std::string, bool> AtaqueVsDefesa(const Entidade& ea, const Entidade& ed, const Posicao& pos_alvo);

// Gera um resumo sobre a notificacao, ou vazio.
std::string ResumoNotificacao(const Tabuleiro& tabuleiro, const ntf::Notificacao& n);

inline Vector3 PosParaVector3(const Posicao& pos) { return Vector3(pos.x(), pos.y(), pos.z()); }

void PreencheNotificacaoAtualizaoPontosVida(
    const Entidade& entidade, int delta_pontos_vida, ntf::Notificacao* n, ntf::Notificacao* n_desfazer = nullptr);
}  // namespace ent

#endif  // ENT_UTIL_H
