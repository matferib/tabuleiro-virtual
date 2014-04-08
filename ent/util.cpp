#include <algorithm>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <google/protobuf/repeated_field.h>
#include <stdexcept>
#include <random>
#include "ent/constantes.h"
#include "ent/entidade.pb.h"
#include "ent/util.h"
#include "gltab/gl.h"  // TODO remover e passar desenhos para para gl
#include "log/log.h"

namespace ent {

void MudaCor(const float* cor) {
  gl::MudaCor(cor[0], cor[1], cor[2], 1.0f);
}

void MudaCorAlfa(const float* cor) {
  gl::MudaCor(cor[0], cor[1], cor[2], cor[3]);
}

void MudaCor(const ent::Cor& cor) {
  gl::MudaCor(cor.r(), cor.g(), cor.b(), cor.a());
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

void EscureceCor(Cor* cor) {
  cor->set_r(std::max(cor->r() - 0.5f, 0.0f));
  cor->set_g(std::max(cor->g() - 0.5f, 0.0f));
  cor->set_b(std::max(cor->b() - 0.5f, 0.0f));
}

const Cor EscureceCor(const Cor& cor) {
  Cor cret(cor);
  EscureceCor(&cret);
  return cret;
}

void ClareiaCor(Cor* cor) {
  cor->set_r(std::min(cor->r() + 0.5f, 1.0f));
  cor->set_g(std::min(cor->g() + 0.5f, 1.0f));
  cor->set_b(std::min(cor->b() + 0.5f, 1.0f));
}

float RealcaComponente(float c) {
  if (c < 0.5) {
    return std::min(c + 0.15f, 1.0f);
  } else {
    return std::max(c - 0.15f, 0.0f);
  }
}

void RealcaCor(Cor* cor) {
  cor->set_r(RealcaComponente(cor->r()));
  cor->set_g(RealcaComponente(cor->g()));
  cor->set_b(RealcaComponente(cor->b()));
}

float VetorParaRotacaoGraus(float x, float y, float* tamanho) {
  float tam = sqrt(x * x + y * y);
  float angulo = acosf(x / tam) * RAD_PARA_GRAUS;
  if (tamanho != nullptr) {
    *tamanho = tam;
  }
  return (y >= 0 ? angulo : -angulo);
}

void DesenhaDisco(float raio, int num_faces) {
  gl::Normal(0.0f, 0.0f, 1.0f);
  unsigned int num_vertices = 2 + (num_faces + 1) * 2;
  float vertices[num_vertices];
  unsigned short indices[num_vertices];
  vertices[0] = vertices[1] = 0.0f;
  for (int i = 0; i <= num_faces; ++i) {
    float angulo = i * 2 * M_PI / num_faces;
    vertices[2 + i * 2] = cosf(angulo) * raio;
    vertices[2 + i * 2 + 1] = sinf(angulo) * raio;
  }
  for (unsigned int i = 0; i < num_vertices; ++i) {
    indices[i] = i;
  }
  gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  gl::PonteiroVertices(2, GL_FLOAT, vertices);
  gl::DesenhaElementos(GL_TRIANGLE_FAN, num_vertices / 2, GL_UNSIGNED_SHORT, indices);
  gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}

namespace {
template<class T>
void DesenhaLinha3dBase(const T& pontos, float largura) {
  if (pontos.size() == 0) {
    return;
  }
  gl::Normal(0.0f, 0.0f, 1.0f);
  for (auto it = pontos.begin(); it != pontos.end() - 1;) {
    const auto& ponto = *it;
    gl::MatrizEscopo salva_matriz;
    gl::Translada(ponto.x(), ponto.y(), ponto.z());
    // Disco do ponto corrente.
    DesenhaDisco(largura / 2.0f, 12);
    // Reta ate proximo ponto.
    const auto& proximo_ponto = *(++it);
    float tam;
    float graus = VetorParaRotacaoGraus(proximo_ponto.x() - ponto.x(), proximo_ponto.y() - ponto.y(), &tam);
    gl::Roda(graus, 0.0f, 0.0f, 1.0f);
    gl::Retangulo(0, -largura / 2.0f, tam, largura / 2.0f);
  }
  const auto& ponto = *(pontos.end() - 1);
  gl::MatrizEscopo salva_matriz;
  gl::Translada(ponto.x(), ponto.y(), ponto.z());
  DesenhaDisco(largura / 2.0f, 12);
}

}  // namespace

void DesenhaLinha3d(const std::vector<Posicao>& pontos, float largura) {
  DesenhaLinha3dBase(pontos, largura);
}

void DesenhaLinha3d(const google::protobuf::RepeatedPtrField<Posicao>& pontos, float largura) {
  DesenhaLinha3dBase(pontos, largura);
}

void LigaStencil() {
  gl::Habilita(GL_STENCIL_TEST);  // Habilita stencil.
  gl::Limpa(GL_STENCIL_BUFFER_BIT);  // stencil zerado.
  gl::FuncaoStencil(GL_ALWAYS, 0xFF, 0xFF);  // Sempre passa no stencil.
  gl::OperacaoStencil(GL_KEEP, GL_KEEP, GL_REPLACE);  // Quando passar no stencil e no depth, escreve 0xFF.
  gl::MascaraCor(false);  // Para nao desenhar nada de verdade, apenas no stencil.
}

void DesenhaStencil(const Cor& cor) {
  const float cor_float[] = { cor.r(), cor.g(), cor.b(), cor.a() };
  DesenhaStencil(cor_float);
}

void DesenhaStencil(const float* cor) {
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  int largura = viewport[2], altura = viewport[3];

  // Neste ponto, os pixels desenhados tem 0xFF no stencil. Reabilita o desenho.
  gl::MascaraCor(true);
  gl::FuncaoStencil(GL_EQUAL, 0xFF, 0xFF);  // So passara no teste quem tiver 0xFF.
  gl::OperacaoStencil(GL_KEEP, GL_KEEP, GL_KEEP);  // Mantem os valores do stencil.
  // Desenha uma chapa na tela toda, preenchera so os buracos do stencil.
  {
    gl::MatrizEscopo salva_projecao(GL_PROJECTION);
    gl::CarregaIdentidade();
    // Eixo com origem embaixo esquerda.
    gl::Ortogonal(0, largura, 0, altura, 0, 1);
    {
      gl::MatrizEscopo salva_projecao(GL_MODELVIEW);
      gl::CarregaIdentidade();
      if (cor != nullptr) {
        MudaCorAlfa(cor);
      }
      gl::DesabilitaEscopo profundidade_escopo(GL_DEPTH_TEST);
      gl::DesligaTesteProfundidadeEscopo desliga_teste_profundidade_escopo;
      // ATENCAO: Esse retangulo acaba com a operacao de picking (porque escreve na tela toda).
      // Operacoes de picking nao devem usar stencil.
      gl::Retangulo(0.0f, 0.0f, largura, altura);
    }
  }
  // Desliga stencil.
  gl::Desabilita(GL_STENCIL_TEST);
}

namespace {

// A string de dados de vida desmembrada.
struct MultDadoSoma {
  unsigned int mult;
  unsigned int dado;
  int soma;
};

const MultDadoSoma DesmembraDadosVida(const std::string& dados_vida) {
  MultDadoSoma mult_dado_soma;
  // Procura o d.
  size_t indice_d = dados_vida.find('d');
  if (indice_d == std::string::npos || indice_d == dados_vida.size() - 1) {
    throw std::logic_error(std::string("Dados de vida mal formado, d nao encontrado: ") + dados_vida);
  }
  // Mult.
  std::string str_mult(dados_vida.substr(0, indice_d));
  mult_dado_soma.mult = atoi(str_mult.c_str());
  if (mult_dado_soma.mult == 0) {
    throw std::logic_error(std::string("Dados de vida mal formado, multiplicador invalido: ") + dados_vida);
  }
  // Dado.
  size_t indice_sinal = dados_vida.find_first_of("+-", indice_d + 1);
  if (indice_sinal == std::string::npos) {
    indice_sinal = dados_vida.size();
  }
  std::string str_dado(dados_vida.substr(indice_d + 1, indice_sinal - indice_d - 1));
  mult_dado_soma.dado = atoi(str_dado.c_str());
  if (mult_dado_soma.dado == 0) {
    throw std::logic_error(std::string("Dados de vida mal formado, dado invalido: ") + dados_vida);
  }
  // Soma.
  if (indice_sinal == dados_vida.size()) {
    mult_dado_soma.soma = 0;
    return mult_dado_soma;
  }
  std::string str_soma(dados_vida.substr(indice_sinal + 1));
  mult_dado_soma.soma = atoi(str_soma.c_str());
  if (dados_vida[indice_sinal] == '-') {
    mult_dado_soma.soma = -mult_dado_soma.soma;
  }
  return mult_dado_soma;
}

} // namespace

// Rola um dado de nfaces.
int RolaDado(unsigned int nfaces) {
  // TODO inicializacao do motor de baseada no timestamp.
  static std::minstd_rand motor_aleatorio;
  return (motor_aleatorio() % nfaces) + 1;
}

// Como gcc esta sem suporte a regex, vamos fazer na mao.
int GeraPontosVida(const std::string& dados_vida) {
  auto mds = DesmembraDadosVida(dados_vida);
  int res = 0;
  for (int i = 0; i < mds.mult; ++i) {
    res += RolaDado(mds.dado);
  }
  res += mds.soma;
  return res;
}

int GeraMaxPontosVida(const std::string& dados_vida) {
  auto mds = DesmembraDadosVida(dados_vida);
  return mds.mult * mds.dado + mds.soma;
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

void MultiplicaMatrizVetor(const float* matriz, float* vetor) {
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
