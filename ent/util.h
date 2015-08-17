#ifndef ENT_UTIL_H
#define ENT_UTIL_H

#include <algorithm>
#include <string>
#include <vector>
#include <google/protobuf/repeated_field.h>

// Funcoes uteis de ent.
namespace ent {

class Cor;
class Posicao;

/** Altera a cor correnta para cor. Nao considera alpha. */
void MudaCor(const float* cor);

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

// Placeholder para retornar a altura do chao em determinado ponto do tabuleiro.
inline float ZChao(float x3d, float y3d) {
  return 0;
}

/** Desenha um disco no eixo x-y, com um determinado numero de faces. */
void DesenhaDisco(float raio, int num_faces);

/** Desenha uma linha 3d com a largura passada, passando pelos pontos. Em cada ponto, sera desenhado um disco para conectar. */
void DesenhaLinha3d(const std::vector<Posicao>& pontos, float largura);
void DesenhaLinha3d(const google::protobuf::RepeatedPtrField<Posicao>& pontos, float largura);

/** Funcoes para ligar o stencil e depois desenhar a cor passada (ou a corrente) onde o stencil foi marcado. */
// ATENCAO: Esse retangulo acaba com a operacao de picking (porque escreve na tela toda). Operacoes de picking nao devem usar stencil.
void LigaStencil();
void DesenhaStencil(const float* cor = nullptr);
void DesenhaStencil(const Cor& cor);

/** Gera um aleatorio de 1 a nfaces. */
int RolaDado(unsigned int nfaces);

/** Gera pontos de vida baseado nos dados de vida, da forma 4d8+8 por exemplo.
* Da excecao se dados_vida for mal formado.
*/
int GeraPontosVida(const std::string& dados_vida);

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

/** Roda o vetor no eixo Z. */
void RodaVetor2d(float graus, Posicao* vetor);

/** Retorna true se o ponto estiver dentro do poligono. */
bool PontoDentroDePoligono(const Posicao& ponto, const std::vector<Posicao>& vertices);

/** Posicionamento do raster em 2d. */
void PosicionaRaster2d(int x, int y, int largura_vp, int altura_vp);

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
google::protobuf::RepeatedPtrField<EntidadeProto::Evento> LeEventos(const std::string& eventos_str);

/** Converte uma string para o efeito, se houver. Caso contrario retorna EFEITO_INVALIDO. */
efeitos_e StringParaEfeito(const std::string& s);

// Trim functions from: http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring.
static inline std::string &ltrim(std::string& s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

// trim from end
static inline std::string &rtrim(std::string& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  return s;
}

// trim from both ends
static inline std::string &trim(std::string& s) {
  return ltrim(rtrim(s));
}

}  // namespace ent

#endif  // ENT_UTIL_H
