#ifndef ENT_UTIL_H
#define ENT_UTIL_H

// Funcoes uteis de ent.
namespace ent {

/** Altera a cor correnta para cor. Nao considera alpha. */
void MudaCor(const float* cor);

/** Considera alpha. */
void MudaCorAlfa(const float* cor);

/** Retorna o vetor de rotacao dado um vetor x,y. */
float VetorParaRotacaoGraus(float x, float y, float* tamanho = nullptr);

/** Desenha um disco no eixo x-y, com um determinado numero de faces. */
void DesenhaDisco(GLfloat raio, int num_faces);

}  // namespace ent

#endif  // ENT_UTIL_H
