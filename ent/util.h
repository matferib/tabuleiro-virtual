#ifndef ENT_UTIL_H
#define ENT_UTIL_H

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

/** Retorna o vetor de rotacao dado um vetor x,y. O valor do vetor vai de (-180, 180]. */
float VetorParaRotacaoGraus(float x, float y, float* tamanho = nullptr);

// Placeholder para retornar a altura do chao em determinado ponto do tabuleiro.
inline float ZChao(float x3d, float y3d) {
  return 0;
}

/** Desenha um disco no eixo x-y, com um determinado numero de faces. */
void DesenhaDisco(GLfloat raio, int num_faces);

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
