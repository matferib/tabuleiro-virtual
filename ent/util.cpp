#include <algorithm>
#include <boost/tokenizer.hpp>
#include <chrono>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <google/protobuf/repeated_field.h>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <random>
#include <unordered_map>
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/entidade.pb.h"
#include "ent/tabuleiro.h"
#include "ent/util.h"
#include "gltab/gl.h"  // TODO remover e passar desenhos para para gl
#include "gltab/gl_vbo.h"  // TODO remover e passar desenhos para para gl
#include "log/log.h"
#include "net/util.h"

namespace ent {

namespace {
std::map<std::string, std::string> g_mapa_utf8;
}  // namespace

void IniciaUtil() {
  g_mapa_utf8 = {
    { "á", "a" },
    { "ã", "a" },
    { "â", "a" },
    { "é", "e" },
    { "ê", "e" },
    { "í", "i" },
    { "ç", "c" },
    { "ô", "o" },
    { "ó", "o" },
    { "õ", "o" },
    { "Á", "A" },
    { "Â", "A" },
    { "É", "E" },
    { "Ê", "E" },
    { "Í", "I" },
    { "Ç", "C" },
    { "Ô", "O" },
    { "Ó", "O" },
    { "Ú", "U" },
    { "ú", "u" },
  };

}

void MudaCor(const float* cor) {
  gl::MudaCor(cor[0], cor[1], cor[2], 1.0f);
}

void MudaCorAplicandoNevoa(const float* cor, const ParametrosDesenho* pd) {
  if (!pd->has_nevoa()) {
    MudaCor(cor);
    return;
  }
  // Distancia do ponto pra nevoa.
  GLfloat mv_gl[16];
  gl::Le(GL_MODELVIEW_MATRIX, mv_gl);
  Matrix4 mv(mv_gl);
  Vector4 ref = Vector4(pd->nevoa().referencia().x(),
                        pd->nevoa().referencia().y(),
                        pd->nevoa().referencia().z(),
                        1.0f);
  Vector4 ponto = mv * Vector4(0.0f, 0.0f, 0.0f, 1.0f);
  ponto -= Vector4(ref.x, ref.y, ref.z, 1.0f);
  float distancia = ponto.length();
  //VLOG(2) << "Distancia para nevoa: " << distancia;
  VLOG(3) << "Distancia para nevoa: " << distancia
          << ", ref: (" << pd->nevoa().referencia().x() << ", " << pd->nevoa().referencia().y() << ", " << pd->nevoa().referencia().z() << ")"
          << ", ponto: (" << ponto.x << ", " << ponto.y << ", " << ponto.z << ")"
          << ", minimo: " << pd->nevoa().minimo()
          << ", maximo: " << pd->nevoa().maximo();
  if (distancia > pd->nevoa().maximo()) {
    gl::MudaCor(pd->nevoa().cor().r(), pd->nevoa().cor().g(), pd->nevoa().cor().b(), 1.0f);
  } else if (distancia > pd->nevoa().minimo()) {
    float escala = (distancia - pd->nevoa().minimo()) / (pd->nevoa().maximo() - pd->nevoa().minimo());
    gl::MudaCor(pd->nevoa().cor().r() * escala + cor[0] * (1.0f - escala),
                pd->nevoa().cor().g() * escala + cor[1] * (1.0f - escala),
                pd->nevoa().cor().b() * escala + cor[2] * (1.0f - escala),
                1.0f);
  } else {
    MudaCor(cor);
  }
}

void ConfiguraNevoa(float min, float max, float r, float g, float b, float* pos, ParametrosDesenho* pd) {
  // A funcao nevoa ja converte a posicao para olho.
  gl::Nevoa(min, max, r, g, b, pos);
  // Aqui tem que ser feito nao mao.
  GLfloat mv_gl[16];
  gl::Le(GL_MODELVIEW_MATRIX, mv_gl);
  Matrix4 mv(mv_gl);
  Vector4 ref = mv * Vector4(pos[0], pos[1], pos[2], 1.0f);

  pd->mutable_nevoa()->mutable_referencia()->set_x(ref.x);
  pd->mutable_nevoa()->mutable_referencia()->set_y(ref.y);
  pd->mutable_nevoa()->mutable_referencia()->set_z(ref.z);
  pd->mutable_nevoa()->set_minimo(min);
  pd->mutable_nevoa()->set_maximo(max);
  pd->mutable_nevoa()->mutable_cor()->set_r(r);
  pd->mutable_nevoa()->mutable_cor()->set_g(g);
  pd->mutable_nevoa()->mutable_cor()->set_b(b);
  pd->mutable_nevoa()->mutable_cor()->set_a(1.0);
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

void EscureceCor(float* cor) {
  cor[0] = std::max(cor[0] - 0.5f, 0.0f);
  cor[1] = std::max(cor[1] - 0.5f, 0.0f);
  cor[2] = std::max(cor[2] - 0.5f, 0.0f);
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

void RealcaCor(float* cor) {
  cor[0] = RealcaComponente(cor[0]);
  cor[1] = RealcaComponente(cor[1]);
  cor[2] = RealcaComponente(cor[2]);
}

void RealcaCor(Cor* cor) {
  cor->set_r(RealcaComponente(cor->r()));
  cor->set_g(RealcaComponente(cor->g()));
  cor->set_b(RealcaComponente(cor->b()));
}

float VetorParaRotacaoGraus(const Posicao& vetor, float* tamanho) {
  return VetorParaRotacaoGraus(vetor.x(), vetor.y(), tamanho);
}

float VetorParaRotacaoGraus(float x, float y, float* tamanho) {
  float tam = sqrt(x * x + y * y);
  if (tam == 0) {
    if (tamanho != nullptr) {
      *tamanho = 0;
    }
    return 0;
  }
  float angulo = acosf(x / tam) * RAD_PARA_GRAUS;
  if (tamanho != nullptr) {
    *tamanho = tam;
  }
  return (y >= 0 ? angulo : -angulo);
}

Matrix4 MatrizRotacao(const Vector3& vn) {
  Vector3 a(1.0f, 0.0f, 0.0f);
  Vector3 b(vn.x, vn.y, vn.z);
  b.normalize();
  Vector3 axis = a.cross(b);
  if (fabs(axis.length()) == 0.0f) {
    return Matrix4();
  }
  axis.normalize();
  float cosang = a.dot(b);
  // aparentemente, nao precisa resolver o caso > PI porque o cross sempre vai retornar o menor angulo entre eles..
  return Matrix4().rotate(acosf(cosang) * RAD_PARA_GRAUS, axis);
}

float DistanciaQuadrado(const Posicao& pos1, const Posicao& pos2) {
  float distancia = powf(pos1.x() - pos2.x(), 2) + powf(pos1.y() - pos2.y(), 2) + powf(pos1.z() - pos2.z(), 2);
  VLOG(4) << "Distancia: " << distancia;
  return distancia;
}

void RodaVetor2d(float graus, Posicao* vetor) {
  float rad = graus * GRAUS_PARA_RAD;
  float sen_o = sinf(rad);
  float cos_o = cosf(rad);
  float x = vetor->x() * cos_o - vetor->y() * sen_o;
  float y = vetor->x() * sen_o + vetor->y() * cos_o;
  vetor->set_x(x);
  vetor->set_y(y);
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
    gl::MatrizEscopo salva_matriz(false);
    gl::Translada(ponto.x(), ponto.y(), ponto.z(), false);
    // Disco do ponto corrente.
    gl::Disco(largura / 2.0f, 8);
    // Reta ate proximo ponto.
    const auto& proximo_ponto = *(++it);
    float tam;
    float graus = VetorParaRotacaoGraus(proximo_ponto.x() - ponto.x(), proximo_ponto.y() - ponto.y(), &tam);
    gl::Roda(graus, 0.0f, 0.0f, 1.0f, false);
    gl::Retangulo(0, -largura / 2.0f, tam, largura / 2.0f);
  }
  const auto& ponto = *(pontos.end() - 1);
  gl::MatrizEscopo salva_matriz(false);
  gl::Translada(ponto.x(), ponto.y(), ponto.z(), false);
  gl::Disco(largura / 2.0f, 8);
}

}  // namespace

bool EhForma2d(int tipo_forma) {
  return
      (tipo_forma == TF_LIVRE || tipo_forma == TF_CIRCULO || tipo_forma == TF_LIVRE ||
       tipo_forma == TF_RETANGULO || tipo_forma == TF_TRIANGULO);
}

void DesenhaLinha3d(const std::vector<Posicao>& pontos, float largura) {
  DesenhaLinha3dBase(pontos, largura);
}

void DesenhaLinha3d(const google::protobuf::RepeatedPtrField<Posicao>& pontos, float largura) {
  DesenhaLinha3dBase(pontos, largura);
}

void LimitesLinha3d(const google::protobuf::RepeatedPtrField<Posicao>& pontos, float largura, float* xi, float* yi, float *xs, float *ys) {
  *xi = std::numeric_limits<float>::max();
  *xs = std::numeric_limits<float>::lowest();
  *yi = std::numeric_limits<float>::max();
  *ys = std::numeric_limits<float>::lowest();
  for (const auto& p : pontos) {
    if (p.x() < *xi) {
      *xi = p.x();
    }
    if (p.x() > *xs) {
      *xs = p.x();
    }
    if (p.y() < *yi) {
      *yi = p.y();
    }
    if (p.y() > *ys) {
      *ys = p.y();
    }
  }
  *xi -= largura;
  *xs += largura;
  *yi -= largura;
  *ys += largura;
}

void LigaStencil() {
  gl::Habilita(GL_STENCIL_TEST);  // Habilita stencil.
  gl::Limpa(GL_STENCIL_BUFFER_BIT);  // stencil zerado.
  gl::FuncaoStencil(GL_ALWAYS, 0xFF, 0xFF);  // Sempre passa no stencil.
  gl::OperacaoStencil(GL_KEEP, GL_KEEP, GL_REPLACE);  // Quando passar no stencil e no depth, escreve 0xFF.
  gl::MascaraCor(false);  // Para nao desenhar nada de verdade, apenas no stencil.
}

void DesenhaStencil2d(const Cor& cor) {
  const float cor_float[] = { cor.r(), cor.g(), cor.b(), cor.a() };
  DesenhaStencil2d(cor_float);
}

void DesenhaStencil2d(const float* cor) {
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
    gl::Ortogonal(0, largura, 0, altura, 0.0f, 1.0f);
    {
      gl::MatrizEscopo salva_projecao(GL_MODELVIEW);
      gl::CarregaIdentidade();
      if (cor != nullptr) {
        MudaCorAlfa(cor);
      }
      gl::DesabilitaEscopo profundidade_escopo(GL_DEPTH_TEST);
      gl::DesligaEscritaProfundidadeEscopo desliga_teste_profundidade_escopo;
      // ATENCAO: Esse retangulo acaba com a operacao de picking (porque escreve na tela toda).
      // Operacoes de picking nao devem usar stencil.
      gl::Retangulo(0.0f, 0.0f, largura, altura);
    }
  }
  // Desliga stencil.
  gl::Desabilita(GL_STENCIL_TEST);
}

void DesenhaStencil3d(float tam_x, float tam_y, const Cor& cor) {
  const float cor_float[] = { cor.r(), cor.g(), cor.b(), cor.a() };
  DesenhaStencil3d(tam_x, tam_y, cor_float);
}

void DesenhaStencil3d(float tam_x, float tam_y, const float* cor) {
  float tam_x_2 = tam_x / 2.0f;
  float tam_y_2 = tam_y / 2.0f;
  DesenhaStencil3d(-tam_x_2, -tam_y_2, tam_x_2, tam_y_2, cor);
}

void DesenhaStencil3d(float xi, float yi, float xs, float ys, const float* cor) {
  // Neste ponto, os pixels desenhados tem 0xFF no stencil. Reabilita o desenho.
  gl::MascaraCor(true);
  gl::FuncaoStencil(GL_EQUAL, 0xFF, 0xFF);  // So passara no teste quem tiver 0xFF.
  gl::OperacaoStencil(GL_KEEP, GL_KEEP, GL_KEEP);  // Mantem os valores do stencil.
  // Desenha uma chapa na tela toda, preenchera so os buracos do stencil.
  {
    if (cor != nullptr) {
      MudaCorAlfa(cor);
    }
    gl::DesabilitaEscopo profundidade_escopo(GL_DEPTH_TEST);
    gl::DesligaEscritaProfundidadeEscopo desliga_teste_profundidade_escopo;
    gl::Retangulo(xi, yi, xs, ys);
  }
  // Desliga stencil.
  gl::Desabilita(GL_STENCIL_TEST);
}

namespace {

struct MultDadoSoma {
  unsigned int mult = 0;  // O numero antes do dado.
  unsigned int dado = 0;  // O numero de faces do dado
  int soma = 0;           // A soma fixa apos o dado.
  void Reset() {
    soma = 0;
    mult = 0;
    dado = 0;
  }
  void Imprime() const {
    LOG(INFO) << "MDS: " << mult << "d" << dado << "+-" << soma;
  }
};

// A string de dados tem o seguinte formato.
const std::vector<MultDadoSoma> DesmembraDadosVida(const std::string& dados_vida) {
  std::vector<MultDadoSoma> vetor_mult_dado_soma;
  boost::char_separator<char> sep("\t ", "+-d");
  boost::tokenizer<boost::char_separator<char>> tokenizador(dados_vida, sep);
  enum EstadoTokenizer {
    ET_INICIAL,
    ET_LEU_D,
    ET_LEU_DADO,
    ET_LEU_MAIS_MENOS,
    ET_LEU_NUM,
    ET_ERRO,
  } et = ET_INICIAL;
  int num = 1;
  MultDadoSoma corrente;
  for (const auto& token : tokenizador) {
    VLOG(2) << "token: " << token;
    switch (et) {
      case ET_INICIAL:
        if (token == "+" || token == "-" || token == "d") {
          LOG(ERROR) << "Estado inicial deve comecar com numero, encontrei: " << token;
          et = ET_ERRO;
        } else {
          num = atoi(token.c_str()) * num;
          et = ET_LEU_NUM;
        }
        break;
      case ET_LEU_NUM:
        if (token == "d") {
          if (corrente.mult != 0) {
           vetor_mult_dado_soma.push_back(corrente);
           corrente.Reset();
          }
          corrente.mult = num;
          et = ET_LEU_D;
        } else if (token == "+" || token == "-") {
          corrente.soma = num;
          vetor_mult_dado_soma.push_back(corrente);
          corrente.Reset();
          et = ET_INICIAL;
          num = (token == "+") ? 1 : -1;
        } else {
          LOG(ERROR) << "Esperava d ou +- apos numero, encontrei: " << token;
          et = ET_ERRO;
        }
        break;
      case ET_LEU_D:
        if (token == "+" || token == "-" || token == "d") {
          LOG(ERROR) << "Esperando numero apos d, encontrei: " << token;
          et = ET_ERRO;
        } else {
          corrente.dado = atoi(token.c_str());
          et = ET_LEU_DADO;
        }
        break;
      case ET_LEU_DADO:
        if (token == "+" || token == "-") {
          num = (token == "+") ? 1 : -1;
          et = ET_LEU_MAIS_MENOS;
        } else {
          LOG(ERROR) << "Esperando +- apos dado, encontrei: " << token;
          et = ET_ERRO;
        }
        break;
      case ET_LEU_MAIS_MENOS:
        if (token == "+" || token == "-") {
          LOG(ERROR) << "Esperando numero ou dado apos +-, encontrei: " << token;
          et = ET_ERRO;
        } else {
          num = atoi(token.c_str()) * num;
          et = ET_LEU_NUM;
        }
        break;
      case ET_ERRO:
        break;
      default:
        break;
    }
    if (et == ET_ERRO) {
      break;
    }
  }
  if (et == ET_LEU_NUM) {
    corrente.soma = num;
    vetor_mult_dado_soma.push_back(corrente);
  }
  if (et == ET_LEU_DADO) {
    vetor_mult_dado_soma.push_back(corrente);
  }
  return vetor_mult_dado_soma;
}

} // namespace

// Rola um dado de nfaces.
int RolaDado(unsigned int nfaces) {
  // TODO inicializacao do motor de baseada no timestamp.
  static std::minstd_rand motor_aleatorio(std::chrono::system_clock::now().time_since_epoch().count());
  return (motor_aleatorio() % nfaces) + 1;
}

float Aleatorio() {
  int val = RolaDado(10001) - 1;  // [0-10000]
  return val / 10000.0f;
}

// Como gcc esta sem suporte a regex, vamos fazer na mao.
int GeraPontosVida(const std::string& dados_vida) {
  const std::vector<MultDadoSoma> vetor_mds = DesmembraDadosVida(dados_vida);
  int res = 0;
  for (const auto& mds : vetor_mds) {
    //mds.Imprime();
    for (unsigned int i = 0; i < mds.mult; ++i) {
      res += RolaDado(mds.dado);
    }
    res += mds.soma;
  }
  return res;
}

int GeraMaxPontosVida(const std::string& dados_vida) {
  const std::vector<MultDadoSoma> vetor_mds = DesmembraDadosVida(dados_vida);
  int res = 0;
  for (const auto& mds : vetor_mds) {
    res +=  mds.mult * mds.dado + mds.soma;
  }
  return res;
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

// Ray casting do ponto para uma direcao qualquer. A cada intersecao, inverte o resultado.
// Referencia:
// http://stackoverflow.com/questions/11716268/point-in-polygon-algorithm
bool PontoDentroDePoligono(const Posicao& ponto, const std::vector<Posicao>& vertices) {
  // TODO verificar divisao por 0 aqui.
  float ponto_x = ponto.x();
  float ponto_y = ponto.y();
  bool dentro = false;
  VLOG(1) << "Ponto: (" << ponto_x << ", " << ponto_y << ")";
  for (unsigned int i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++) {
    float vix = vertices[i].x();
    float viy = vertices[i].y();
    float vjx = vertices[j].x();
    float vjy = vertices[j].y();
    float vjy_viy = vjy - viy;
    VLOG(1) << "Vertice: (" << vix << ", " << viy << ")";
    if (fabs(vjy_viy) < 0.01) {
      return true;
    }
    if (((viy > ponto_y) != (vjy > ponto_y)) &&
        (ponto_x < ((vjx - vix) * ((ponto_y - viy) / vjy_viy) + vix))) {
      // A cada intersecao com os vertices, inverte sinal.
      dentro = !dentro;
    }
  }
  return dentro;
}

// Posiciona o raster no pixel.
void PosicionaRaster2d(int x, int y) {
  gl::PosicaoRasterAbsoluta(x, y);
}

efeitos_e StringParaEfeito(const std::string& s) {
  static std::unordered_map<std::string, efeitos_e> mapa = {
    { "borrar", EFEITO_BORRAR },
    { "reflexos", EFEITO_REFLEXOS },
    { "piscar", EFEITO_PISCAR },
  };
  const auto& ret = mapa.find(s);
  return ret == mapa.end() ? EFEITO_INVALIDO : ret->second;
}

google::protobuf::RepeatedPtrField<EntidadeProto::Evento> LeEventos(const std::string& eventos_str) {
  google::protobuf::RepeatedPtrField<EntidadeProto::Evento> ret;
  std::istringstream ss(eventos_str);
  while (1) {
    std::string linha;
    if (!std::getline(ss, linha)) {
      break;
    }
    // Cada linha.
    size_t pos_dois_pontos = linha.find(':');
    if (pos_dois_pontos == std::string::npos) {
      LOG(ERROR) << "Ignorando evento: " << linha;
      continue;
    }
    std::string descricao(linha.substr(0, pos_dois_pontos));
    std::string complemento;
    size_t pos_par = descricao.find("(");
    if (pos_par != std::string::npos) {
      complemento = descricao.substr(pos_par + 1);
      descricao = descricao.substr(0, pos_par);
    }
    std::string rodadas(linha.substr(pos_dois_pontos + 1));
    EntidadeProto::Evento evento;
    evento.set_descricao(ent::trim(descricao));
    evento.set_rodadas(atoi(rodadas.c_str()));
    if (!complemento.empty()) {
      evento.set_complemento(atoi(complemento.c_str()));
    }
    efeitos_e id_efeito = StringParaEfeito(evento.descricao());
    if (id_efeito != EFEITO_INVALIDO) {
      evento.set_id_efeito(id_efeito);
    }
    ret.Add()->Swap(&evento);
  }
  return ret;
}

// Retorna a string sem os caracteres UTF-8 para desenho OpenGL.
const std::string StringSemUtf8(const std::string& id_acao) {
  std::string ret(id_acao);
  for (const auto& it_mapa : g_mapa_utf8) {
    auto it = ret.find(it_mapa.first);
    while (it != std::string::npos) {
      ret.replace(it, it_mapa.first.size(), it_mapa.second);
      it = ret.find(it_mapa.first);
    }
  }
  return ret;
}

void MoveDeltaRespeitandoChao(float dx, float dy, float dz, const Tabuleiro& tabuleiro, Entidade* entidade) {
  float novo_x = entidade->X() + dx;
  float novo_y = entidade->Y() + dy;
  float zchao = tabuleiro.ZChao(novo_x, novo_y);
  float novo_z = std::max(zchao, entidade->Z() + dz);
  entidade->MovePara(novo_x, novo_y, novo_z);
}

bool EhPng(const std::string& textura) {
  return textura.find(".png") == (textura.size() - 4);
}

bool EhTerreno(const std::string& textura) {
  return EhPng(textura) && (textura.find("tile_") == 0 || textura.find("terrain_") == 0);
}

bool EhCaixaCeu(const std::string& textura) {
  return EhPng(textura) && textura.find("skybox") == 0;
}

bool EhIcone(const std::string& textura) {
  return EhPng(textura) && textura.find("icon_") == 0;
}

bool EhModelo3d(const std::string& nome) {
  return nome.size() > 9 && nome.find(".binproto") != std::string::npos;
}

bool FiltroTexturaEntidade(const std::string& textura) {
  return EhCaixaCeu(textura) || EhTerreno(textura) || EhIcone(textura) || !EhPng(textura);
}

bool FiltroTexturaCaixaCeu(const std::string& textura) {
  return !EhCaixaCeu(textura);
}

bool FiltroTexturaTabuleiro(const std::string& textura) {
  return !EhTerreno(textura);
}

bool FiltroModelo3d(const std::string& nome) {
  return !EhModelo3d(nome);
}

bool AlteraBlendEscopo::AlteraBlendEntidadeComposta(const ParametrosDesenho* pd, const Cor& cor) const {
  float rgb[3] = { 1.0f, 1.0f, 1.0f};
  bool misturar_cor_raiz = false;
  if (cor.has_r() && (cor.r() < 1.0f || cor.g() < 1.0f || cor.b() < 1.0f)) {
    rgb[0] = cor.r();
    rgb[1] = cor.g();
    rgb[2] = cor.b();
    misturar_cor_raiz = true;
  }
  if (pd->has_picking_x()) {
    // Durante picking, nao altera o blending.
    return false;
  } else if (pd->has_alfa_translucidos()) {
    // Blend ja ta ligado.
    gl::FuncaoMistura(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
    gl::CorMistura(rgb[0], rgb[1], rgb[2], cor.a() < 1.0f ? cor.a() : pd->alfa_translucidos());
    return true;
  } else if (pd->entidade_selecionada()) {
    // Ignora a cor de destino (que ja esta la) e escurence a cor fonte (sendo escrita).
    gl::Habilita(GL_BLEND);
    gl::FuncaoMistura(GL_CONSTANT_COLOR, GL_ZERO);
    gl::CorMistura(rgb[0] * 0.9f, rgb[1] * 0.9f, rgb[2] * 0.9f, 1.0f);
    return true;
  } else if (misturar_cor_raiz) {
    // Mistura as cores com a cor do objeto raiz.
    gl::Habilita(GL_BLEND);
    gl::FuncaoMistura(GL_CONSTANT_COLOR, GL_ZERO);
    gl::CorMistura(rgb[0], rgb[1], rgb[2], 1.0f);
    return true;
  }
  return false;
}

void AlteraBlendEscopo::RestauraBlend(const ParametrosDesenho* pd) const {
  if (pd->has_alfa_translucidos()) {
    gl::CorMistura(1.0f, 1.0f, 1.0f, pd->alfa_translucidos());
  } else {
    gl::Desabilita(GL_BLEND);
    gl::CorMistura(0.0f, 0.0f, 0.0f, 0.0f);
  }
  gl::FuncaoMistura(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

}  // namespace ent
