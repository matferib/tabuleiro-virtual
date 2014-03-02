#ifndef ENT_UTIL_H
#define ENT_UTIL_H

#include <vector>

namespace google {
namespace protobuf {
template <class T> class RepeatedPtrField;
}  // namespace protobuf
}  // namespace google

// Funcoes uteis de ent.
namespace ent {

class Cor;
class Posicao;

/** Altera a cor correnta para cor. Nao considera alpha. */
void MudaCor(const float* cor);

/** Considera alpha. */
void MudaCorAlfa(const float* cor);

/** Outra forma de mudar a cor. */
void MudaCor(float r, float g, float b, float a);
void MudaCor(const Cor& cor);

/** Preenche proto_cor com cor. A entrada deve ter 4 componentes. */
void CorAlfaParaProto(const float* cor, Cor* cor_proto);

/** Igual CorAlfaParaProto mas para 3 componentes. */
void CorParaProto(const float* cor, Cor* cor_proto);

/** Reduz cada componente RGB em 0.5f. */
const Cor EscureceCor(const Cor& cor);
void EscureceCor(Cor* cor);

/** Aumenta cada componente RGB em 0.5f. */
void ClareiaCor(Cor* cor);

/** Muda cada componente RGB de forma notavel. */
void RealcaCor(Cor* cor);

/** Retorna o vetor de rotacao dado um vetor x,y. O valor do vetor vai de (-180, 180]. */
float VetorParaRotacaoGraus(float x, float y, float* tamanho = nullptr);

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

/** Computa pos2 - pos1 em pos_res. */
void ComputaDiferencaVetor(const Posicao& pos2, const Posicao& pos1, Posicao* pos_res);

/** Computa pos2 + pos1 em pos_res. */
void ComputaSomaVetor(const Posicao& pos2, const Posicao& pos1, Posicao* pos_res);

/** Computa pos_res = escala * pos. */
void ComputaMultiplicacaoEscalar(float escala, const Posicao& pos, Posicao* pos_res);

/** Computa o vetor normalizado. */
void ComputaVetorNormalizado(Posicao* pos);

/* Multiplica a matriz openGL matriz pelo vetor. A matriz OpenGL tem formato col x linha (column major), portanto,
* ao inves de multiplicar matriz (4x4) pelo vetor (4x1), fazemos a inversao: vetor (1x4) pela matriz (4x4).
*/
void MultiplicaMatrizVetor(const float* matriz, float* vetor);

}  // namespace ent

#endif  // ENT_UTIL_H
