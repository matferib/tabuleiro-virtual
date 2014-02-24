#include <cmath>
#include <google/protobuf/repeated_field.h>
#if __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include "ent/constantes.h"
#include "ent/entidade.pb.h"
#include "ent/util.h"

namespace ent {

void MudaCor(const float* cor) {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cor);
  glColor3fv(cor);
}

void MudaCorAlfa(const float* cor) {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cor);
  glColor4fv(cor);
}

void MudaCor(float r, float g, float b, float a) {
  const GLfloat cor_gl[] = { r, g, b, a };
  MudaCorAlfa(cor_gl);
}

void MudaCor(const ent::Cor& cor) {
  MudaCor(cor.r(), cor.g(), cor.b(), cor.a());
}

void CorAlfaParaProto(const float* cor, Cor* proto_cor) {
  proto_cor->set_r(cor[0]);
  proto_cor->set_g(cor[1]);
  proto_cor->set_b(cor[2]);
  proto_cor->set_a(cor[3]);
}

void CorParaProto(const float* cor, Cor* proto_cor) {
  proto_cor->set_r(cor[0]);
  proto_cor->set_g(cor[1]);
  proto_cor->set_b(cor[2]);
  proto_cor->clear_a();
}

const Cor EscureceCor(const Cor& cor) {
  Cor cret;
  cret.set_r(cor.r() * 0.5);
  cret.set_g(cor.g() * 0.5);
  cret.set_b(cor.b() * 0.5);
  cret.set_a(cor.a());
  return cret;
}

float VetorParaRotacaoGraus(float x, float y, float* tamanho) {
  float tam = sqrt(x * x + y * y);
  float angulo = acosf(x / tam) * RAD_PARA_GRAUS;
  if (tamanho != nullptr) {
    *tamanho = tam;
  }
  return (y >= 0 ? angulo : -angulo);
}

void DesenhaDisco(GLfloat raio, int num_faces) {
  glNormal3f(0, 0, 1.0f);
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(0.0, 0.0, 0.0);
  for (int i = 0; i <= num_faces; ++i) {
    float angulo = i * 2 * M_PI / num_faces;
    glVertex3f(cosf(angulo) * raio, sinf(angulo) * raio, 0.0f);
  }
  glEnd();
}

namespace {
template<class T>
void DesenhaLinha3dBase(const T& pontos, float largura) {
  if (pontos.size() == 0) {
    return;
  }
  for (auto it = pontos.begin(); it != pontos.end() - 1;) {
    const auto& ponto = *it;
    glPushMatrix();
    glTranslatef(ponto.x(), ponto.y(), ponto.z());
    // Disco do ponto corrente.
    DesenhaDisco(largura / 2.0f, 12);
    // Reta ate proximo ponto.
    const auto& proximo_ponto = *(++it);
    float tam;
    float graus = VetorParaRotacaoGraus(proximo_ponto.x() - ponto.x(), proximo_ponto.y() - ponto.y(), &tam);
    glRotatef(graus, 0.0f, 0.0f, 1.0f);
    glRectf(0, -largura / 2.0f, tam, largura / 2.0f);
    glPopMatrix();
  }
  const auto& ponto = *(pontos.end() - 1);
  glPushMatrix();
  glTranslatef(ponto.x(), ponto.y(), ponto.z());
  DesenhaDisco(largura / 2.0f, 12);
  glPopMatrix();
}

}  // namespace

void DesenhaLinha3d(const std::vector<Posicao>& pontos, float largura) {
  DesenhaLinha3dBase(pontos, largura);
}

void DesenhaLinha3d(const google::protobuf::RepeatedPtrField<Posicao>& pontos, float largura) {
  DesenhaLinha3dBase(pontos, largura);
}

void LigaStencil() {
  glPushAttrib(GL_ENABLE_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_BLEND);
  glEnable(GL_STENCIL_TEST);  // Habilita stencil.
  glClear(GL_STENCIL_BUFFER_BIT);  // stencil zerado.
  glStencilFunc(GL_ALWAYS, 0xFF, 0xFF);  // Sempre passa no stencil.
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  // Quando passar no stencil e no depth, escreve 0xFF.
  glColorMask(0, 0, 0, 0);  // Para nao desenhar nada de verdade, apenas no stencil.
}

void DesenhaStencil(const Cor& cor) {
  const float cor_float[] = { cor.r(), cor.g(), cor.b(), cor.a() };
  DesenhaStencil(cor_float);
}

void DesenhaStencil(const float* cor) {
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  int largura = viewport[2], altura = viewport[3];

  // Neste ponto, os pixels desenhados tem 0xFF no stencil. Reabilita o desenho.
  glColorMask(true, true, true, true);
  glStencilFunc(GL_EQUAL, 0xFF, 0xFF);  // So passara no teste quem tiver 0xFF.
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);  // Mantem os valores do stencil.
  // Desenha uma chapa na tela toda, preenchera so os buracos do stencil.
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  // Eixo com origem embaixo esquerda.
  glOrtho(0, largura, 0, altura, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  MudaCorAlfa(cor);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(false);
  // ATENCAO: Esse retangulo acaba com a operacao de picking (porque escreve na tela toda). Operacoes de picking nao devem usar stencil.
  glRectf(0.0f, 0.0f, largura, altura);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  // Restaura atributos antes do stencil.
  glPopAttrib();
  glMatrixMode(GL_MODELVIEW);
}

void ComputaDiferencaVetor(const Posicao& pos2, const Posicao& pos1, Posicao* pos_res) {
  pos_res->set_x(pos2.x() - pos1.x());
  pos_res->set_y(pos2.y() - pos1.y());
  pos_res->set_z(pos2.z()-  pos1.z());
}

void ComputaSomaVetor(const Posicao& pos2, const Posicao& pos1, Posicao* pos_res) {
  pos_res->set_x(pos2.x() + pos1.x());
  pos_res->set_y(pos2.y() + pos1.y());
  pos_res->set_z(pos2.z() + pos1.z());
}

void ComputaMultiplicacaoEscalar(float escala, const Posicao& pos, Posicao* pos_res) {
  pos_res->set_x(pos.x() * escala);
  pos_res->set_y(pos.y() * escala);
  pos_res->set_z(pos.z() * escala);
}

void ComputaVetorNormalizado(Posicao* pos) {
  float x = pos->x();
  float y = pos->y();
  float z = pos->z();
  float tam = sqrt(x * x + y * y + z * z);
  pos->set_x(x / tam);
  pos->set_y(y / tam);
  pos->set_z(z / tam);
}

void MultiplicaMatrizVetor(const GLfloat* matriz, GLfloat* vetor) {
  GLfloat res[4];
  for (int i = 0; i < 4; ++i) {
    res[i] = vetor[0] * matriz[i] +
             vetor[1] * matriz[i + 4] +
             vetor[2] * matriz[i + 8] +
             vetor[3] * matriz[i + 12];
  }
  vetor[0] = res[0];
  vetor[1] = res[1];
  vetor[2] = res[2];
  vetor[3] = res[3];
}

}  // namespace ent
