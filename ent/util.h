#ifndef ENT_UTIL_H
#define ENT_UTIL_H

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

/** Retorna o vetor de rotacao dado um vetor x,y. O valor do vetor vai de (-180, 180]. */
float VetorParaRotacaoGraus(float x, float y, float* tamanho = nullptr);

// Placeholder para retornar a altura do chao em determinado ponto do tabuleiro.
inline float ZChao(float x3d, float y3d) {
  return 0;
}

/** Desenha um disco no eixo x-y, com um determinado numero de faces. */
void DesenhaDisco(GLfloat raio, int num_faces);

/** Desenha uma linha 3d com a largura passada, passando pelos pontos. Em cada ponto, sera desenhado um disco para conectar. */
void DesenhaLinha3d(const std::vector<Posicao>& pontos, float largura);
void DesenhaLinha3d(const google::protobuf::RepeatedPtrField<Posicao>& pontos, float largura);

/** Par de funcoes para ligar o stencil e depois desenhar a cor passada onde o stencil foi marcado. */
// ATENCAO: Esse retangulo acaba com a operacao de picking (porque escreve na tela toda). Operacoes de picking nao devem usar stencil.
void LigaStencil();
void DesenhaStencil(const float* cor);
void DesenhaStencil(const Cor& cor);

/** Computa pos2 - pos1 em pos_res. */
void ComputaDiferencaVetor(const Posicao& pos2, const Posicao& pos1, Posicao* pos_res);

/** Computa pos2 + pos1 em pos_res. */
void ComputaSomaVetor(const Posicao& pos2, const Posicao& pos1, Posicao* pos_res);

/** Computa pos_res = escala * pos. */
void ComputaMultiplicacaoEscalar(float escala, const Posicao& pos, Posicao* pos_res);

/** Computa o vetor normalizado. */
void ComputaVetorNormalizado(Posicao* pos);

}  // namespace ent

#endif  // ENT_UTIL_H
