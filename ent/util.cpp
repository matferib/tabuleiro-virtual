#include "ent/util.h"
#include <google/protobuf/repeated_field.h>
#include <algorithm>
#include <boost/algorithm/string/replace.hpp>
#include <boost/tokenizer.hpp>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include "ent/acoes.h"
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/entidade.pb.h"
#include "ent/tabuleiro.h"
#include "gltab/gl.h"      // TODO remover e passar desenhos para para gl
#include "gltab/gl_vbo.h"  // TODO remover e passar desenhos para para gl
#include "goog/stringprintf.h"
#include "log/log.h"
#include "net/util.h"

namespace ent {

namespace {
const std::map<std::string, std::string> g_mapa_utf8 = {
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
}  // namespace

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
    gl::MatrizEscopo salva_matriz;
    gl::Translada(ponto.x(), ponto.y(), ponto.z());
    // Disco do ponto corrente.
    gl::Disco(largura / 2.0f, 8);
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
  static std::default_random_engine motor(std::chrono::system_clock::now().time_since_epoch().count());
  std::uniform_int_distribution<int> distribution(1, nfaces);
  //static int min = motor_aleatorio.min();
  //static int max = motor_aleatorio.max();
  return distribution(motor);
}

float Aleatorio() {
  int val = RolaDado(10001) - 1;  // [0-10000]
  return val / 10000.0f;
}

DanoArma LeDanoArma(const std::string& dano_arma) {
  DanoArma ret;
  auto pos_parentesis = dano_arma.find('(');
  ret.dano = dano_arma.substr(0, pos_parentesis);
  trim(ret.dano);
  ret.margem_critico = 20;
  ret.multiplicador = 2;

  if (pos_parentesis == std::string::npos) {
    return ret;
  }

  std::string entre_parentesis = dano_arma.substr(pos_parentesis + 1);
  boost::replace_all(entre_parentesis, "×", "x");
  boost::replace_all(entre_parentesis, "X", "x");
  boost::replace_all(entre_parentesis, "–", "-");

  boost::char_separator<char> sep("\t ", "-)x/");
  boost::tokenizer<boost::char_separator<char>> tokenizador(entre_parentesis, sep);
  enum EstadoTokenizer {
    ET_INICIAL,
    ET_LEU_MARGEM_INICIAL,
    ET_LEU_SEPARADOR_MARGEM,
    ET_LEU_MARGEM,
    ET_FIM,
    ET_ERRO,
  } et = ET_INICIAL;
  for (const auto& token : tokenizador) {
    VLOG(2) << "token: " << token << ", estado: " << et;
    switch (et) {
      case ET_INICIAL:
        if (token == "x") {
          et = ET_LEU_MARGEM;
          break;
        } else if (std::all_of(token.begin(), token.end(), [] (char c) { return isdigit(c); })) {
          int margem = 0;
          if ((margem = atoi(token.c_str())) > 0 || margem <= 20) {
            ret.margem_critico = margem;
            et = ET_LEU_MARGEM_INICIAL;
            break;
          }
        }
        et = ET_ERRO;
        break;
      case ET_LEU_MARGEM_INICIAL:
        if (token == "-") {
          et = ET_LEU_SEPARADOR_MARGEM;
        } else if (token == "/") {
          et = ET_LEU_MARGEM;
        } else if (token == "x") {
          et = ET_LEU_MARGEM;
        } else if (token == ")") {
          et = ET_FIM;
        }
        break;
      case ET_LEU_SEPARADOR_MARGEM:
        if (std::all_of(token.begin(), token.end(), [] (char c) { return isdigit(c); })) {
          // pode ignorar o resto.
          et = ET_LEU_MARGEM;
        } else {
          et = ET_ERRO;
        }
        break;
      case ET_LEU_MARGEM:
        if (token == "/") {
          break;
        } else if (token == "x") {
          break;
        } else if (token == ")") {
          et = ET_FIM;
        } else if (std::all_of(token.begin(), token.end(), [] (char c) { return isdigit(c); })) {
          int multiplicador = 0;
          if ((multiplicador = atoi(token.c_str())) != 0) {
            ret.multiplicador = multiplicador;
            et = ET_FIM;
            break;
          }
        } else {
          et = ET_ERRO;
        }
        break;
      case ET_ERRO:
      case ET_FIM:
      default:
        return ret;
        break;
    }
  }
  return ret;
}

std::string DadosParaString(int total, std::vector<std::pair<int, int>>& dados) {
  std::string s("rolado total: ");
  s += net::to_string(total) + ", ";
  for (const auto& dado_valor : dados) {
    char ds[100];
    snprintf(ds, 99, "d%d=%d, ", dado_valor.first, dado_valor.second);
    s += ds;
  }
  return s;
}

void AtualizaStringDadosVida(int delta, std::string* dados_vida) {
  std::vector<MultDadoSoma> vetor_mds = DesmembraDadosVida(*dados_vida);
  if (vetor_mds.empty()) {
    vetor_mds.resize(1);
  }
  vetor_mds[0].soma += delta;
  std::string s;
  bool first = true;
  for (const auto& mds : vetor_mds) {
    char texto[100];
    if (first) {
      if (mds.mult == 0) {
        snprintf(texto, 99, "%d", mds.soma);
      } else {
        snprintf(texto, 99, "%dd%d%+d", mds.mult, mds.dado, mds.soma);
      }
    } else {
      if (mds.mult == 0) {
        snprintf(texto, 99, "%+d", mds.soma);
      } else {
        snprintf(texto, 99, "%+dd%d%+d", mds.mult, mds.dado, mds.soma);
      }
    }
    s += texto;
  }
  *dados_vida = s;
}

std::tuple<int, std::vector<std::pair<int, int>>> GeraPontosVida(const std::string& dados_vida) {
  const std::vector<MultDadoSoma> vetor_mds = DesmembraDadosVida(dados_vida);
  std::vector<std::pair<int, int>> dados;
  int res = 0;
  for (const auto& mds : vetor_mds) {
    //mds.Imprime();
    for (unsigned int i = 0; i < mds.mult; ++i) {
      int valor_dado = RolaDado(mds.dado);
      dados.push_back(std::make_pair(mds.dado, valor_dado));
      res += valor_dado;
    }
    res += mds.soma;
  }
  return std::make_tuple(res, dados);
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

TipoEvento StringParaEfeito(const std::string& s) {
  static std::unordered_map<std::string, TipoEvento> mapa = {
    { "borrar", EFEITO_BORRAR },
    { "reflexos", EFEITO_REFLEXOS },
    { "piscar", EFEITO_PISCAR },
  };
  const auto& ret = mapa.find(s);
  return ret == mapa.end() ? EFEITO_INVALIDO : ret->second;
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
  // As texturas de terreno sao uteis para formas.
  return EhCaixaCeu(textura) || /*EhTerreno(textura) || */EhIcone(textura) || !EhPng(textura);
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
  if (cor.has_r() || cor.has_g() || cor.has_b()) {
    rgb[0] = cor.has_r() ? cor.r() : 1.0f;
    rgb[1] = cor.has_g() ? cor.g() : 1.0f;
    rgb[2] = cor.has_b() ? cor.b() : 1.0f;
    misturar_cor_raiz = true;
  }
  if (pd->has_picking_x()) {
    // Durante picking, nao altera o blending.
    return false;
  } else if (pd->has_alfa_translucidos()) {
    // Blend ja ta ligado.
    float a = cor.a() < 1.0f ? cor.a() : pd->alfa_translucidos();
    if (misturar_cor_raiz) {
      // realiza a mistura na mao da cor fonte na cor de mistura. A cor de destino escala pelo alfa normalmente.
      gl::FuncaoMistura(GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_ALPHA);
      gl::CorMistura(rgb[0] * a, rgb[1] * a, rgb[2] * a, a);
    } else {
      // A cor fonte eh afetada apenas pelo alfa.
      gl::FuncaoMistura(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
      gl::CorMistura(rgb[0], rgb[1], rgb[2], a);
    }
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

int ModificadorAtaque(bool distancia, const EntidadeProto& ea, const EntidadeProto& ed) {
  int modificador = 0;
  // ataque.
  if (ea.caida() && !distancia) {
    modificador -= 4;
  }
  if (ea.dados_ataque_globais().ataque_menos_1()) {
    modificador -= 1;
  }
  if (ea.dados_ataque_globais().ataque_menos_2()) {
    modificador -= 2;
  }
  if (ea.dados_ataque_globais().ataque_menos_4()) {
    modificador -= 4;
  }
  if (ea.dados_ataque_globais().ataque_menos_8()) {
    modificador -= 8;
  }
  if (ea.dados_ataque_globais().ataque_mais_1()) {
    modificador += 1;
  }
  if (ea.dados_ataque_globais().ataque_mais_2()) {
    modificador += 2;
  }
  if (ea.dados_ataque_globais().ataque_mais_4()) {
    modificador += 4;
  }
  if (ea.dados_ataque_globais().ataque_mais_8()) {
    modificador += 8;
  }

  // Defesa.
  if (ed.caida()) {
    if (distancia) modificador -= 4;
    else modificador += 4;
  }
  return modificador;
}

int ModificadorDano(const EntidadeProto& ea) {
  int modificador = 0;
  if (ea.dados_ataque_globais().dano_menos_1()) {
    modificador -= 1;
  }
  if (ea.dados_ataque_globais().dano_menos_2()) {
    modificador -= 2;
  }
  if (ea.dados_ataque_globais().dano_menos_4()) {
    modificador -= 4;
  }
  if (ea.dados_ataque_globais().dano_menos_8()) {
    modificador -= 8;
  }
  if (ea.dados_ataque_globais().dano_mais_1()) {
    modificador += 1;
  }
  if (ea.dados_ataque_globais().dano_mais_2()) {
    modificador += 2;
  }
  if (ea.dados_ataque_globais().dano_mais_4()) {
    modificador += 4;
  }
  if (ea.dados_ataque_globais().dano_mais_8()) {
    modificador += 8;
  }
  if (ea.dados_ataque_globais().dano_mais_16()) {
    modificador += 16;
  }
  if (ea.dados_ataque_globais().dano_mais_32()) {
    modificador += 32;
  }
  return modificador;
}

namespace {

Entidade::TipoCA CATipoAtaque(const EntidadeProto::DadosAtaque& da) {
  return da.ataque_toque() ? Entidade::CA_TOQUE : Entidade::CA_NORMAL;
}

}  // namespace

// Rola o dado de ataque vs defesa, retornando o numero de vezes que o dano deve ser aplicado e o texto da jogada.
// O ultimo parametro indica se a acao deve ser desenhada (em caso de distancia maxima atingida, retorna false).
// Caso haja falha critica, retorna vezes = -1;
// Posicao ataque eh para calculo de distancia.
std::tuple<int, std::string, bool> AtaqueVsDefesa(const Entidade& ea, const Entidade& ed, const Posicao& pos_alvo) {
  const auto* da = ea.DadoCorrente();
  if (da == nullptr) da = &EntidadeProto::DadosAtaque::default_instance();
  const int ataque_origem = ea.BonusAtaque();
  const int d20_defesa_agarrar = RolaDado(20);
  const int ca_destino = da->ataque_agarrar() ? (d20_defesa_agarrar + ed.Proto().bba().agarrar()) : ed.CA(CATipoAtaque(*da));
  const std::string str_ca_destino = da->ataque_agarrar()
      ? google::protobuf::StringPrintf("%d=%d+d20", ca_destino, ed.Proto().bba().agarrar())
      : google::protobuf::StringPrintf("%d", ca_destino);
  int modificador_incrementos = 0;
  if (ataque_origem == Entidade::AtaqueCaInvalido || ca_destino == Entidade::AtaqueCaInvalido) {
    VLOG(1) << "Ignorando ataque vs defesa por falta de dados: ataque: " << ataque_origem
            << ", defesa: " << ca_destino;
    return std::make_tuple(0, "Ataque sem bonus ou defensor sem armadura", true);
  }
  float alcance_m = ea.AlcanceAtaqueMetros();
  if (alcance_m >= 0) {
    Posicao pos_acao_a = ea.PosicaoAcao();
    Posicao pos_acao_d = ed.PosicaoAcao();

    // Raio de acao da entidade. A partir do raio dela, numero de quadrados multiplicado pelo tamanho.
    float mult_tamanho = ea.MultiplicadorTamanho();
    float raio_a = mult_tamanho * TAMANHO_LADO_QUADRADO_2;
    VLOG(1) << "raio_a: " << raio_a;

    // Distancia da acao ao alvo. Poderia usar o raio da defesa, mas é mais legal fazer a distancia precisa.
    Vector3 va(pos_acao_a.x(), pos_acao_a.y(), pos_acao_a.z());
    Vector3 vd;
    float distancia_m = - raio_a;
    if (pos_alvo.has_x()) {
      vd = Vector3(pos_alvo.x(), pos_alvo.y(), pos_alvo.z());
      const float MARGEM_ERRO = TAMANHO_LADO_QUADRADO_2;
      distancia_m -= MARGEM_ERRO;
      VLOG(1) << "Usando posicao real, descontando " << MARGEM_ERRO;
    } else {
      vd = Vector3(pos_acao_d.x(), pos_acao_d.y(), pos_acao_d.z());
      distancia_m -= ed.MultiplicadorTamanho() * TAMANHO_LADO_QUADRADO_2;
      VLOG(1) << "Usando posicao fixa, descontando" << (ed.MultiplicadorTamanho() * TAMANHO_LADO_QUADRADO_2);
    }
    distancia_m += (va - vd).length();
    VLOG(1) << "distancia_m: " << distancia_m;

    if (distancia_m > alcance_m) {
      int total_incrementos = distancia_m / alcance_m;
      if (total_incrementos > ea.IncrementosAtaque()) {
        char texto[50] = {'\0'};
        snprintf(texto, 49, "Fora de alcance: %0.1fm > %0.1fm, inc: %d, max: %d", distancia_m, alcance_m, total_incrementos, ea.IncrementosAtaque());
        return std::make_tuple(0, texto, false);
      } else {
        modificador_incrementos = total_incrementos * 2;
        VLOG(1) << "modificador_incrementos: " << modificador_incrementos;
      }
    }
    VLOG(1) << "alcance_m: " << alcance_m << ", distancia_m: " << distancia_m;
  }

  auto tipo_ataque = da->tipo_ataque();
  int outros_modificadores = ModificadorAtaque(da->ataque_distancia(), ea.Proto(), ed.Proto());
  int d20 = RolaDado(20);
  std::string texto;
  char texto_critico[50] = {'\0'};
  char texto_incremento[50] = {'\0'};
  char texto_outros_modificadores[50] = {'\0'};
  if (modificador_incrementos) {
    snprintf(texto_incremento, 49, "-%d", modificador_incrementos);
  }
  if (outros_modificadores != 0) {
    snprintf(texto_outros_modificadores, 49, "%+d", outros_modificadores);
  }

  int total = ataque_origem + d20 - modificador_incrementos + outros_modificadores;
  if (d20 == 1) {
    VLOG(1) << "Falha critica";
    return std::make_tuple(-1, "falha critica", false);
  } else if ((d20 != 20 || da->ataque_agarrar()) && total < ca_destino) {
    texto = google::protobuf::StringPrintf(
        "falhou: %d+%d%s%s= %d vs %s", d20, ataque_origem, texto_incremento, texto_outros_modificadores, total, str_ca_destino.c_str());
    return std::make_tuple(0, texto, true);
  }
  // Chance de falha.
  {
    const int chance_falha = std::max(ea.ChanceFalhaAtaque(), ed.ChanceFalhaDefesa());
    if (chance_falha > 0) {
      const int d100 = RolaDado(100);
      VLOG(1) << "Chance de falha: " << chance_falha << ", tirou: " << d100;
      if (d100 < chance_falha) {
        return std::make_tuple(0, google::protobuf::StringPrintf("Falhou, chance %d, tirou %d", chance_falha, d100), true);
      }
    }
  }

  // Se chegou aqui acertou.
  int vezes = 1;
  if (d20 >= ea.MargemCritico() && !da->ataque_agarrar()) {
    if (ed.ImuneCritico()) {
      snprintf(texto_critico, 49, ", imune a critico");
    } else {
      int d20_critico = RolaDado(20);
      int total_critico = ataque_origem + d20_critico - modificador_incrementos + outros_modificadores;
      if (d20 == 1) {
        snprintf(texto_critico, 49, ", critico falhou: 1");
      } else if (total_critico >= ca_destino) {
        snprintf(
            texto_critico, 49, ", critico %d+%d%s%s= %d",
            d20_critico, ataque_origem, texto_incremento, texto_outros_modificadores, total_critico);
        vezes = ea.MultiplicadorCritico();
      } else {
        snprintf(
            texto_critico, 49, ", critico falhou: %d+%d%s%s= %d",
            d20_critico, ataque_origem, texto_incremento, texto_outros_modificadores, total_critico);
      }
    }
  }
  texto = google::protobuf::StringPrintf("acertou: %d+%d%s%s= %d%s vs %s",
           d20, ataque_origem, texto_incremento, texto_outros_modificadores, total, texto_critico, str_ca_destino.c_str());
  VLOG(1) << "Resultado ataque vs defesa: " << texto << ", vezes: " << vezes;
  return std::make_tuple(vezes, texto, true);
}

std::tuple<int, std::string> AtaqueVsSalvacao(const AcaoProto& ap, const Entidade& ed) {
  std::string descricao_resultado;
  int delta_pontos_vida = ap.delta_pontos_vida();
  if (ed.TemProximaSalvacao()) {
    if (ed.ProximaSalvacao() == RS_MEIO) {
      delta_pontos_vida /= 2;
      descricao_resultado = google::protobuf::StringPrintf("salvou metade (manual), dano: %d", delta_pontos_vida);
    } else if (ed.ProximaSalvacao() == RS_QUARTO) {
      delta_pontos_vida /= 4;
      descricao_resultado = google::protobuf::StringPrintf("salvou um quarto (manual), dano: %d", delta_pontos_vida);
    } else if (ed.ProximaSalvacao() == RS_ANULOU) {
      delta_pontos_vida = 0;
      descricao_resultado = "salvou tudo (manual)";
    }
  } else if (ap.has_dificuldade_salvacao()) {
    // TODO evasao e evasao aprimorada.
    int d20 = RolaDado(20);
    int bonus = ed.Salvacao(ap.tipo_salvacao());
    int total = d20 + bonus;
    if (total >= ap.dificuldade_salvacao()) {
      if (ap.resultado_salvacao() == RS_MEIO) {
        delta_pontos_vida /= 2;
      } else if (ap.resultado_salvacao() == RS_QUARTO) {
        delta_pontos_vida /= 4;
      } else {
        delta_pontos_vida = 0;
      }
      descricao_resultado = google::protobuf::StringPrintf("%d%+d >= %d, Salvou, dano: %d", d20, bonus, ap.dificuldade_salvacao(), delta_pontos_vida);
    } else {
      descricao_resultado = google::protobuf::StringPrintf("%d%+d < %d, Nao salvou, dano: %d", d20, bonus, ap.dificuldade_salvacao(), delta_pontos_vida);
    }
  } else {
    descricao_resultado = google::protobuf::StringPrintf("Acao sem dificuldade e alvo sem salvacao, dano: %d", delta_pontos_vida);
  }
  return std::make_tuple(delta_pontos_vida, descricao_resultado);
}

namespace {

std::string EntidadeNotificacao(const Tabuleiro& tabuleiro, const ntf::Notificacao& n) {
  auto* entidade = tabuleiro.BuscaEntidade(n.entidade().id());
  if (entidade != nullptr && !entidade->Proto().rotulo().empty()) {
    return entidade->Proto().rotulo() + " ";
  }
  return std::string("Entidade ");
}

}  // namespace

std::string ResumoNotificacao(const Tabuleiro& tabuleiro, const ntf::Notificacao& n) {
  switch (n.tipo()) {
    case ntf::TN_GRUPO_NOTIFICACOES: {
      std::string resumo;
      for (const auto& nf : n.notificacao()) {
        auto resumo_parcial = ResumoNotificacao(tabuleiro, nf);
        if (!resumo_parcial.empty()) {
          resumo += resumo_parcial + ", ";
        }
      }
      return resumo;
    }
    case ntf::TN_ADICIONAR_ACAO: {
      return "";
    }
    case ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE: {
      return EntidadeNotificacao(tabuleiro, n) + " atualizada: " + n.entidade().ShortDebugString();
    }
    default:
      return "";
  }
}

// O delta de pontos de vida afeta outros bits tambem.
void PreencheNotificacaoAtualizaoPontosVida(
    const Entidade& entidade, int delta_pontos_vida, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
  auto* entidade_depois = n->mutable_entidade();
  entidade_depois->set_id(entidade.Id());
  if (delta_pontos_vida < 0 && entidade.PontosVidaTemporarios() > 0) {
    int temp = entidade.PontosVidaTemporarios();
    int minpv = std::min(std::abs(delta_pontos_vida), temp);
    delta_pontos_vida += minpv;
    temp -= minpv;
    entidade_depois->set_pontos_vida_temporarios(temp);
  }
  entidade_depois->set_pontos_vida(entidade.PontosVida() + delta_pontos_vida);

  if (n_desfazer != nullptr) {
    n_desfazer->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
    n_desfazer->mutable_entidade()->CopyFrom(*entidade_depois);
    auto* entidade_antes = n_desfazer->mutable_entidade_antes();
    entidade_antes->set_id(entidade.Id());
    entidade_antes->set_pontos_vida(entidade.PontosVida());
    entidade_antes->set_morta(entidade.Proto().morta());
    entidade_antes->set_caida(entidade.Proto().caida());
    entidade_antes->set_voadora(entidade.Proto().voadora());
    entidade_antes->set_aura(entidade.Proto().aura());
    entidade_antes->mutable_pos()->CopyFrom(entidade.Pos());
    entidade_antes->mutable_direcao_queda()->CopyFrom(entidade.Proto().direcao_queda());
  }
}

// Retorna se os bonus sao cumulativos.
bool BonusCumulativo(TipoBonus tipo) {
  switch (tipo) {
    case TB_CIRCUNSTANCIA:
    case TB_CLASSE:
    case TB_ESQUIVA:
    case TB_FAMILIAR:
    case TB_NIVEIS_NEGATIVOS:
    case TB_NIVEL:
    case TB_RACIAL:
    case TB_TEMPLATE:
    case TB_TALENTO:
    case TB_SEM_NOME:
      return true;
    default: return false;
  }
}

std::vector<TipoBonus> ExclusaoEscudo(bool permite_escudo) {
  std::vector<TipoBonus> exclusao;
  if (!permite_escudo) {
    exclusao.push_back(TB_ESCUDO);
    exclusao.push_back(TB_ESCUDO_MELHORIA);
  }
  return exclusao;
}

int CATotal(const EntidadeProto& proto, bool permite_escudo) {
  return BonusTotalExcluindo(proto.dados_defesa().ca(), ExclusaoEscudo(permite_escudo));
}

int CASurpreso(const EntidadeProto& proto, bool permite_escudo) {
  const int modificador_destreza = ModificadorAtributo(proto.atributos().destreza());
  return BonusTotalExcluindo(proto.dados_defesa().ca(), ExclusaoEscudo(permite_escudo)) - std::max(modificador_destreza, 0);
}

int CAToque(const EntidadeProto& proto) {
  return BonusTotalExcluindo(proto.dados_defesa().ca(),
         { TB_ARMADURA, TB_ESCUDO, TB_ARMADURA_NATURAL, TB_ARMADURA_MELHORIA, TB_ESCUDO_MELHORIA });
}

int CAToqueSurpreso(const EntidadeProto& proto) {
  const int modificador_destreza = ModificadorAtributo(proto.atributos().destreza());
  return BonusTotalExcluindo(proto.dados_defesa().ca(),
         { TB_ARMADURA, TB_ESCUDO, TB_ARMADURA_NATURAL, TB_ARMADURA_MELHORIA, TB_ESCUDO_MELHORIA }) - std::max(modificador_destreza, 0);
}

bool PossuiBonus(TipoBonus tipo, const Bonus& bonus) {
  for (const auto& bi : bonus.bonus_individual()) {
    if (bi.tipo() == tipo) {
      return true;
    }
  }
  return false;
}

// Retorna o total de um bonus individual, contabilizando acumulo caso as origens sejam diferentes.
int BonusIndividualTotal(const BonusIndividual& bonus_individual) {
  if (BonusCumulativo(bonus_individual.tipo())) {
    int total = 0;
    std::unordered_map<std::string, int> mapa_por_origem;
    for (const auto& por_origem : bonus_individual.por_origem()) {
      auto it = mapa_por_origem.find(por_origem.origem());
      if (it == mapa_por_origem.end() || por_origem.valor() > it->second) {
        total += por_origem.valor();
        mapa_por_origem[por_origem.origem()] = por_origem.valor();
      }
    }
    return total;
  } else {
    // TODO pensar no caso de origens dando penalidade e bonus. Acontece?
    int maior = bonus_individual.por_origem().empty() ? 0 : std::numeric_limits<int>::min();
    for (const auto& por_origem : bonus_individual.por_origem()) {
      maior = std::max(maior, por_origem.valor());
    }
    return maior;
  }
}

// Retorna o total para um tipo de bonus.
int BonusIndividualTotal(TipoBonus tipo, const Bonus& bonus) {
  for (const auto& bi : bonus.bonus_individual()) {
    if (bi.tipo() == tipo) {
      return BonusIndividualTotal(bi);
    }
  }
  return 0;
}

int BonusIndividualPorOrigem(TipoBonus tipo, const std::string& origem, const Bonus& bonus) {
  for (const auto& bi : bonus.bonus_individual()) {
    if (bi.tipo() == tipo) {
      return BonusIndividualPorOrigem(origem, bi);
    }
  }
  return 0;
}

int BonusIndividualPorOrigem(const std::string& origem, const BonusIndividual& bonus_individual) {
  for (const auto& por_origem : bonus_individual.por_origem()) {
    if (por_origem.origem() == origem) {
      return por_origem.valor();
    }
  }
  return 0;
}

namespace {
int CalculaBonusBaseAtaque(const EntidadeProto& proto) {
  int bba = 0;
  for (const auto& info_classe : proto.info_classes()) {
    bba += info_classe.bba();
  }
  return bba;
}

// Retorna o bonus de ataque para uma determinada arma.
int CalculaBonusBaseParaAtaque(const EntidadeProto::DadosAtaque& da, const EntidadeProto& proto) {
  return BonusTotal(da.outros_bonus_ataque());
}

// Retorna a string de dano para uma arma.
std::string CalculaDanoParaAtaque(const EntidadeProto::DadosAtaque& da, const EntidadeProto& proto) {
  const int mod_final = BonusTotal(da.outros_bonus_dano());
  return da.dano_basico_arma().c_str() + (mod_final != 0 ? google::protobuf::StringPrintf("%+d", mod_final) : "");
}

bool PossuiCategoria(CategoriaArma categoria, const ArmaProto& arma) {
  return std::any_of(arma.categoria().begin(), arma.categoria().end(), [categoria] (int c) { return c == categoria; });
}

std::string DanoBasicoPorTamanho(TamanhoEntidade tamanho, const StringPorTamanho& dano) {
  switch (tamanho) {
    case TM_MEDIO: return dano.medio();
    case TM_PEQUENO: return dano.pequeno();
    case TM_GRANDE: return dano.grande();
    default: return "";
  }
}

void RecomputaDependenciasArma(const Tabelas& tabelas, EntidadeProto::DadosAtaque* da, const EntidadeProto& proto) {
  // Passa alguns campos da acao para o ataque.
  AcaoParaAtaque(tabelas.Acao(da->tipo_ataque()), da);

  const auto& arma = tabelas.Arma(da->id_arma());
  if (arma.has_id()) {
    if (da->rotulo().empty()) da->set_rotulo(arma.nome());
    da->set_acuidade(false);
    if (proto.dados_ataque_globais().acuidade() &&
        (PossuiCategoria(CAT_LEVE, arma) ||
         arma.id() == "sabre" || arma.id() == "chicote" || arma.id() == "corrente_com_cravos")) {
      da->set_acuidade(true);
    }

    // tipo certo de ataque.
    bool distancia = PossuiCategoria(CAT_DISTANCIA, arma);
    bool cac = PossuiCategoria(CAT_CAC, arma);
    if (distancia && cac) {
      // Se tipo nao estiver selecionado, pode ser qualquer um dos dois. Preferencia para distancia.
      if (da->tipo_ataque() != "Ataque Corpo a Corpo" && da->tipo_ataque() != "Ataque a Distância") {
        da->set_tipo_ataque("Ataque a Distância");
      }
    } else if (cac) {
      da->set_tipo_ataque("Ataque Corpo a Corpo");
    } else {
      da->set_tipo_ataque("Ataque a Distância");
    }
    da->set_ataque_distancia(distancia && da->tipo_ataque() != "Ataque Corpo a Corpo");

    if (arma.has_alcance_quadrados()) {
      da->set_alcance_m(arma.alcance_quadrados() * QUADRADOS_PARA_METROS);
    }
    if (da->empunhadura() == EA_MAO_RUIM && PossuiCategoria(CAT_ARMA_DUPLA, arma)) {
      da->set_dano_basico_arma(DanoBasicoPorTamanho(proto.tamanho(), arma.dano_secundario()));
      da->set_margem_critico(arma.margem_critico_secundario());
      da->set_multiplicador_critico(arma.multiplicador_critico_secundario());
    } else {
      da->set_dano_basico_arma(DanoBasicoPorTamanho(proto.tamanho(), arma.dano()));
      da->set_margem_critico(arma.margem_critico());
      da->set_multiplicador_critico(arma.multiplicador_critico());
    }
    if (PossuiCategoria(CAT_ARREMESSO, arma)) {
      da->set_incrementos(5);
    } else if (PossuiCategoria(CAT_DISTANCIA, arma)) {
      da->set_incrementos(10);
    }
  }

  int bba = 0;
  const int bba_cac = proto.bba().cac();
  const int bba_distancia = proto.bba().distancia();
  const std::string& tipo_str = da->tipo_ataque();
  bool usar_forca_dano = false;
  const int modificador_forca = ModificadorAtributo(proto.atributos().forca());
  if (da->ataque_distancia()) {
    bba = bba_distancia;
    if (PossuiCategoria(CAT_ARCO, arma)) {
      if (arma.has_max_forca()) {
        // Ajuste de arcos compostos.
        if (modificador_forca < arma.max_forca()) bba -= 2;
        usar_forca_dano = true;
      }
    } else if (PossuiCategoria(CAT_ARREMESSO, arma)) {
      usar_forca_dano = true;
    }
  } else if (da->ataque_agarrar()) {
    bba = proto.bba().agarrar();
    usar_forca_dano = true;
  } else if (tipo_str == "Ataque Corpo a Corpo") {
    bba = da->acuidade() && bba_distancia > bba_cac ? bba_distancia : bba_cac;
    usar_forca_dano = true;
  } else if (da->ataque_toque()) {
    bba = proto.dados_ataque_globais().acuidade() && bba_distancia > bba_cac ? bba_distancia : bba_cac;
  }
  AtribuiBonus(bba, TB_BASE, "base", da->mutable_outros_bonus_ataque());
  if (usar_forca_dano) {
    int modificador_forca_dano = arma.has_max_forca() ? std::min(modificador_forca, arma.max_forca()) : modificador_forca;
    int dano_forca = 0;
    EmpunhaduraArma ea = da->empunhadura();
    if (modificador_forca_dano < 0) {
      dano_forca = modificador_forca;
    } else if (ea == EA_2_MAOS) {
      dano_forca = floorf(modificador_forca_dano * 1.5f);
    } else if (ea == EA_MAO_RUIM) {
      dano_forca = modificador_forca_dano / 2;
    } else {
      dano_forca = modificador_forca_dano;
    }
    AtribuiBonus(dano_forca, TB_ATRIBUTO, "forca", da->mutable_outros_bonus_dano());
  } else {
    RemoveBonus(TB_ATRIBUTO, "forca", da->mutable_outros_bonus_dano());
  }
  // Estes dois sao os mais importantes, porque eh o que vale.
  da->set_bonus_ataque(CalculaBonusBaseParaAtaque(*da, proto));
  da->set_dano(CalculaDanoParaAtaque(*da, proto));

  VLOG(1) << "Proto apos RecomputaDependencias: " << proto.DebugString();
}

void AplicaBonus(const Bonus& bonus, Bonus* alvo) {
  for (const auto& bi : bonus.bonus_individual()) {
    for (const auto& po : bi.por_origem()) {
      AtribuiBonus(po.valor(), bi.tipo(), po.origem(), alvo);
    }
  }
}

void AplicaEfeito(const ConsequenciaEvento& consequencia, EntidadeProto* proto) {
  AplicaBonus(consequencia.atributos().forca(), proto->mutable_atributos()->mutable_forca());
  AplicaBonus(consequencia.atributos().destreza(), proto->mutable_atributos()->mutable_destreza());
  AplicaBonus(consequencia.atributos().constituicao(), proto->mutable_atributos()->mutable_constituicao());
  AplicaBonus(consequencia.atributos().inteligencia(), proto->mutable_atributos()->mutable_inteligencia());
  AplicaBonus(consequencia.atributos().sabedoria(), proto->mutable_atributos()->mutable_sabedoria());
  AplicaBonus(consequencia.atributos().carisma(), proto->mutable_atributos()->mutable_carisma());
  AplicaBonus(consequencia.dados_defesa().ca(), proto->mutable_dados_defesa()->mutable_ca());
  AplicaBonus(consequencia.dados_defesa().salvacao_reflexo(), proto->mutable_dados_defesa()->mutable_salvacao_reflexo());
  for (auto& da : *proto->mutable_dados_ataque()) {
    AplicaBonus(consequencia.jogada_ataque(), da.mutable_outros_bonus_ataque());
  }
}

void RecomputaDependenciasEfeitos(const Tabelas& tabelas, EntidadeProto* proto) {
  std::set<int, std::greater<int>> eventos_a_remover;
  int i = 0;
  // Primeiro desfaz o que tem para depois sobrescrever com os ativos.
  for (const auto& evento : proto->evento()) {
    if (evento.rodadas() < 0) {
      const auto& efeito = tabelas.Efeito(evento.id_efeito());
      AplicaEfeito(efeito.consequencia_fim(), proto);
      eventos_a_remover.insert(i);
    }
    ++i;
  }
  for (int i : eventos_a_remover) {
    proto->mutable_evento()->DeleteSubrange(i, 1);
  }
  for (const auto& evento : proto->evento()) {
    const auto& efeito = tabelas.Efeito(evento.id_efeito());
    AplicaEfeito(efeito.consequencia(), proto);
  }
}

// Recomputa as dependencias de atributos (destreza com armadura).
void RecomputaDependenciasDestreza(const Tabelas& tabelas, EntidadeProto* proto) {
  auto* dd = proto->mutable_dados_defesa();
  // Ajusta a destreza de acordo com a armadura. Primeiro limpa para calcular a penalidade de armadura ou escudo.
  AtribuiBonus(0, TB_ARMADURA, "armadura_escudo", proto->mutable_atributos()->mutable_destreza());
  const int mod_antes = ModificadorAtributo(proto->atributos().destreza());
  int bonus_maximo = 100;
  if (dd->has_id_armadura()) {
    bonus_maximo = std::min(tabelas.Armadura(dd->id_armadura()).max_bonus_destreza(), bonus_maximo);
  }
  if (dd->has_id_escudo()) {
    bonus_maximo = std::min(tabelas.Escudo(dd->id_escudo()).max_bonus_destreza(), bonus_maximo);
  }
  int penalidade = mod_antes > bonus_maximo ? bonus_maximo - mod_antes : 0;
  AtribuiBonus(penalidade, TB_ARMADURA, "armadura_escudo", proto->mutable_atributos()->mutable_destreza());
}

// Retorna o bonus base de uma salvacao, dado o nivel. Forte indica que a salvacao eh forte.
int CalculaBaseSalvacao(bool forte, int nivel) {
  if (forte) {
    return 2 + (nivel / 2);
  } else {
    return nivel / 3;
  }
}

Bonus* BonusSalvacao(TipoSalvacao ts, EntidadeProto* proto) {
  switch (ts) {
    case TS_FORTITUDE: return proto->mutable_dados_defesa()->mutable_salvacao_fortitude();
    case TS_REFLEXO: return proto->mutable_dados_defesa()->mutable_salvacao_reflexo();
    case TS_VONTADE: return proto->mutable_dados_defesa()->mutable_salvacao_vontade();
    default:
      LOG(ERROR) << "Tipo de salvacao invalido: " << (int)ts;
      return proto->mutable_dados_defesa()->mutable_salvacao_fortitude();
  }
}

// Recomputa os modificadores de conjuracao.
void RecomputaDependenciasClasses(const Tabelas& tabelas, EntidadeProto* proto) {
  int salvacao_fortitude = 0;
  int salvacao_reflexo = 0;
  int salvacao_vontade = 0;
  // Para evitar recomputar quando nao tiver base.
  bool recomputa_base = false;
  for (auto& ic : *proto->mutable_info_classes()) {
    if (ic.has_atributo_conjuracao()) {
      ic.set_modificador_atributo_conjuracao(ModificadorAtributo(ic.atributo_conjuracao(), *proto));
    }

    if (ic.salvacoes_fortes_size() > 0) {
      recomputa_base = true;
      salvacao_fortitude += CalculaBaseSalvacao(ClassePossuiSalvacaoForte(TS_FORTITUDE, ic), ic.nivel());
      salvacao_reflexo += CalculaBaseSalvacao(ClassePossuiSalvacaoForte(TS_REFLEXO, ic), ic.nivel());
      salvacao_vontade += CalculaBaseSalvacao(ClassePossuiSalvacaoForte(TS_VONTADE, ic), ic.nivel());
    }
  }
  if (recomputa_base) {
    AtribuiBonus(salvacao_fortitude, TB_BASE, "base", BonusSalvacao(TS_FORTITUDE, proto));
    AtribuiBonus(salvacao_reflexo, TB_BASE, "base", BonusSalvacao(TS_REFLEXO, proto));
    AtribuiBonus(salvacao_vontade, TB_BASE, "base", BonusSalvacao(TS_VONTADE, proto));
  } else {
    RemoveBonus(TB_BASE, "base", BonusSalvacao(TS_FORTITUDE, proto));
    RemoveBonus(TB_BASE, "base", BonusSalvacao(TS_REFLEXO, proto));
    RemoveBonus(TB_BASE, "base", BonusSalvacao(TS_VONTADE, proto));
  }
}

void RecomputaDependenciasCA(const ent::Tabelas& tabelas, EntidadeProto* proto_retornado) {
  auto* dd = proto_retornado->mutable_dados_defesa();
  const int modificador_destreza = ModificadorAtributo(proto_retornado->atributos().destreza());
  AtribuiBonus(modificador_destreza, ent::TB_ATRIBUTO, "destreza", dd->mutable_ca());
  const int modificador_tamanho = ModificadorTamanho(proto_retornado->tamanho());
  ent::AtribuiBonus(10, ent::TB_BASE, "base",  dd->mutable_ca());
  AtribuiBonus(modificador_tamanho, ent::TB_TAMANHO, "tamanho", dd->mutable_ca());
  ent::AtribuiBonus(dd->has_id_armadura() ? tabelas.Armadura(dd->id_armadura()).bonus() : 0, ent::TB_ARMADURA, "armadura", dd->mutable_ca());
  ent::AtribuiBonus(dd->has_id_escudo() ? tabelas.Escudo(dd->id_escudo()).bonus() : 0, ent::TB_ESCUDO, "escudo", dd->mutable_ca());
  // CA dos ataques.
  for (auto& da : *proto_retornado->mutable_dados_ataque()) {
    bool permite_escudo = da.empunhadura() == EA_ARMA_ESCUDO;
    da.set_ca_normal(CATotal(*proto_retornado, permite_escudo));
    da.set_ca_surpreso(CASurpreso(*proto_retornado, permite_escudo));
    da.set_ca_toque(CAToque(*proto_retornado));
  }
}

}  // namespace

bool ClassePossuiSalvacaoForte(TipoSalvacao ts, const InfoClasse& ic) {
  return std::any_of(ic.salvacoes_fortes().begin(), ic.salvacoes_fortes().end(), [ts] (int icts) { return icts == ts; });
}

void RecomputaDependencias(const Tabelas& tabelas, EntidadeProto* proto) {
  RecomputaDependenciasEfeitos(tabelas, proto);
  RecomputaDependenciasDestreza(tabelas, proto);
  RecomputaDependenciasClasses(tabelas, proto);

  int modificador_destreza           = ModificadorAtributo(proto->atributos().destreza());
  const int modificador_constituicao = ModificadorAtributo(proto->atributos().constituicao());
  //const int modificador_inteligencia = ModificadorAtributo(ent::BonusTotal(proto->atributos().inteligencia()));
  const int modificador_sabedoria    = ModificadorAtributo(proto->atributos().sabedoria());
  //const int modificador_carisma      = ModificadorAtributo(ent::BonusTotal(proto->atributos().carisma()));

  // Iniciativa.
  AtribuiBonus(modificador_destreza, ent::TB_ATRIBUTO, "destreza", proto->mutable_bonus_iniciativa());
  proto->set_modificador_iniciativa(BonusTotal(proto->bonus_iniciativa()));

  auto* dd = proto->mutable_dados_defesa();
  // Testes de resistencia.
  AtribuiBonus(modificador_constituicao, ent::TB_ATRIBUTO, "constituicao", dd->mutable_salvacao_fortitude());
  AtribuiBonus(modificador_destreza, ent::TB_ATRIBUTO, "destreza", dd->mutable_salvacao_reflexo());
  AtribuiBonus(modificador_sabedoria, ent::TB_ATRIBUTO, "sabedoria", dd->mutable_salvacao_vontade());

  // CA.
  RecomputaDependenciasCA(tabelas, proto);

  // BBA: tenta atualizar por classe, se nao houver, pelo bba base, senao apenas modificadores.
  const int modificador_forca = ModificadorAtributo(proto->atributos().forca());
  const int modificador_tamanho = ModificadorTamanho(proto->tamanho());
  const int bba = proto->info_classes_size() > 0 ? CalculaBonusBaseAtaque(*proto) : proto->bba().has_base() ? proto->bba().base() : 0;
  proto->mutable_bba()->set_base(bba);
  proto->mutable_bba()->set_cac(modificador_forca + modificador_tamanho + bba);
  proto->mutable_bba()->set_distancia(modificador_destreza + modificador_tamanho + bba);
  proto->mutable_bba()->set_agarrar(modificador_forca + ModificadorTamanhoAgarrar(proto->tamanho()) + bba);

  // Atualiza os bonus de ataques.
  for (auto& da : *proto->mutable_dados_ataque()) {
    RecomputaDependenciasArma(tabelas, &da, *proto);
  }
}

int BonusTotal(const Bonus& bonus) {
  int total = 0;
  for (const auto& bi : bonus.bonus_individual()) {
    total += BonusIndividualTotal(bi);
  }
  return total;
}

// Computa o total de bonus, excluindo alguns tipos.
int BonusTotalExcluindo(const Bonus& bonus, const std::vector<ent::TipoBonus>& bonus_excluidos) {
  int total = 0;
  for (const auto& bi : bonus.bonus_individual()) {
    if (std::any_of(bonus_excluidos.begin(), bonus_excluidos.end(), [&bi](ent::TipoBonus tipo) { return tipo == bi.tipo(); })) {
      continue;
    }
    total += BonusIndividualTotal(bi);
  }
  return total;
}

void RemoveBonus(TipoBonus tipo, const std::string& origem, Bonus* bonus) {
  for (int ib = 0; ib < bonus->bonus_individual_size(); ++ib) {
    auto* bi = bonus->mutable_bonus_individual(ib);
    if (bi->tipo() != tipo) continue;
    for (int ipo = 0; ipo < bi->por_origem_size(); ++ipo) {
      const auto& po = bi->por_origem(ipo);
      if (po.origem() != origem) continue;
      bi->mutable_por_origem()->DeleteSubrange(ipo, 1);
      break;
    }
    if (bi->por_origem().empty()) {
      bonus->mutable_bonus_individual()->DeleteSubrange(ib, 1);
    }
    break;
  }
}

// Se origem existir, ira sobrescrever.
void AtribuiBonusIndividual(int valor, const std::string& origem, BonusIndividual* bonus_individual) {
  for (auto& por_origem : *bonus_individual->mutable_por_origem()) {
    if (por_origem.origem() == origem) {
      por_origem.set_valor(valor);
      return;
    }
  }
  auto* po = bonus_individual->add_por_origem();
  po->set_valor(valor);
  po->set_origem(origem);
}

// Atribui um tipo de bonus individual a bonus.
void AtribuiBonus(int valor, TipoBonus tipo, const std::string& origem, Bonus* bonus) {
  for (auto& bi : *bonus->mutable_bonus_individual()) {
    if (bi.tipo() == tipo) {
      AtribuiBonusIndividual(valor, origem, &bi);
      return;
    }
  }
  auto* bi = bonus->add_bonus_individual();
  bi->set_tipo(tipo);
  AtribuiBonusIndividual(valor, origem, bi);
}

int ModificadorAtributo(int atributo) {
  return (atributo / 2) - 5;
}

int ModificadorAtributo(const Bonus& atributo) {
  int total_atributo = BonusTotal(atributo);
  if (!PossuiBonus(TB_BASE, atributo)) {
    total_atributo += 10;
  }
  return ModificadorAtributo(total_atributo);
}

int ModificadorAtributo(TipoAtributo ta, const EntidadeProto& proto) {
  switch (ta) {
    case TA_FORCA: return ModificadorAtributo(proto.atributos().forca());
    case TA_DESTREZA: return ModificadorAtributo(proto.atributos().destreza());
    case TA_CONSTITUICAO: return ModificadorAtributo(proto.atributos().constituicao());
    case TA_INTELIGENCIA: return ModificadorAtributo(proto.atributos().inteligencia());
    case TA_SABEDORIA: return ModificadorAtributo(proto.atributos().sabedoria());
    case TA_CARISMA: return ModificadorAtributo(proto.atributos().carisma());
    default:
      LOG(ERROR) << "Tipo atributo invalido: " << (int)ta;
  }
  return 0;
}

int ModificadorTamanho(TamanhoEntidade tamanho) {
  switch (tamanho) {
    case TM_MINUSCULO: return 8;
    case TM_DIMINUTO: return 4;
    case TM_MIUDO: return 2;
    case TM_PEQUENO: return 1;
    case TM_MEDIO: return 0;
    case TM_GRANDE: return -1;
    case TM_ENORME: return -2;
    case TM_IMENSO: return -4;
    case TM_COLOSSAL: return -8;
  }
  return 0;
}

int ModificadorTamanhoAgarrar(TamanhoEntidade tamanho) {
  switch (tamanho) {
    case TM_MINUSCULO: return -16;
    case TM_DIMINUTO: return -12;
    case TM_MIUDO: return -8;
    case TM_PEQUENO: return -4;
    case TM_MEDIO: return 0;
    case TM_GRANDE: return 4;
    case TM_ENORME: return 8;
    case TM_IMENSO: return 12;
    case TM_COLOSSAL: return 16;
  }
  return 0;
}

bool PossuiEvento(TipoEvento tipo, const EntidadeProto& entidade) {
  return std::any_of(entidade.evento().begin(), entidade.evento().end(), [tipo] (const EntidadeProto::Evento& evento) {
    return evento.id_efeito() == tipo;
  });
}

void AcaoParaAtaque(const AcaoProto& acao_proto, EntidadeProto::DadosAtaque* da) {
  da->set_tipo_ataque(acao_proto.id());
  if (acao_proto.has_ataque_toque()) {
    da->set_ataque_toque(acao_proto.ataque_toque());
  } else {
    da->clear_ataque_toque();
  } 
  if (acao_proto.has_ataque_distancia()) {
    da->set_ataque_distancia(acao_proto.ataque_distancia());
  } else {
    da->clear_ataque_distancia();
  }
  if (acao_proto.has_ataque_agarrar()) {
    da->set_ataque_agarrar(acao_proto.ataque_agarrar());
  } else {
    da->clear_ataque_agarrar();
  }

}

std::string StringCritico(const EntidadeProto::DadosAtaque& da) {
  if (da.multiplicador_critico() == 2 && da.margem_critico() == 20) return "";
  std::string critico = "(";
  if (da.margem_critico() < 20) {
    critico += net::to_string(da.margem_critico()) + "-20";
    if (da.multiplicador_critico() > 2) {
      critico += "/";
    }
  }
  if (da.multiplicador_critico() > 2) {
    critico += "x" + net::to_string(da.multiplicador_critico());
  }
  critico += ")";
  return critico;
}

std::string StringAtaque(const EntidadeProto::DadosAtaque& da, const EntidadeProto& proto) {
  int modificador = ModificadorAtaque(da.tipo_ataque() != "Ataque Corpo a Corpo", proto, EntidadeProto());
  char texto_modificador[100] = { '\0' };
  if (modificador != 0) snprintf(texto_modificador, 99, "%+d", modificador);

  char texto_furtivo[100] = { '\0' };
  if (proto.furtivo() && !proto.dados_ataque_globais().dano_furtivo().empty()) {
    snprintf(texto_furtivo, 99, "+%s", proto.dados_ataque_globais().dano_furtivo().c_str());
  }

  std::string critico = StringCritico(da);
  return google::protobuf::StringPrintf(
      "%s%s: %+d%s, %s%s%s, CA: %s",
      da.rotulo().c_str(),
      da.ataque_toque() ? " (T)" : "",
      da.bonus_ataque(),
      texto_modificador,
      StringDanoParaAcao(da, proto).c_str(),
      critico.c_str(),
      texto_furtivo,
      StringCAParaAcao(da, proto).c_str());
}

std::string StringCAParaAcao(const EntidadeProto::DadosAtaque& da, const EntidadeProto& proto) {
  const bool permite_escudo = da.empunhadura() == EA_ARMA_ESCUDO;
  int normal, toque;
  std::string info = !permite_escudo && !proto.surpreso()
      ? "" : permite_escudo && proto.surpreso() ? "(esc+surp) " : permite_escudo ? "(escudo) " : "(surpreso) ";
  if (proto.dados_defesa().has_ca()) {
    normal = proto.surpreso() ? CASurpreso(proto, permite_escudo) : CATotal(proto, permite_escudo);
    toque = proto.surpreso() ? CAToqueSurpreso(proto) : CAToque(proto);
  } else {
    normal = proto.surpreso() ? da.ca_surpreso() : da.ca_normal();
    toque = da.ca_toque();
  }
  return google::protobuf::StringPrintf("%s%d, tq: %d", info.c_str(), normal, toque);
}

std::string StringResumoArma(const ent::EntidadeProto::DadosAtaque& da) {
  // Monta a string.
  char string_rotulo[40] = { '\0' };
  if (!da.rotulo().empty()) {
    snprintf(string_rotulo, 39, "%s, ", da.rotulo().c_str());
  }
  char string_alcance[40] = { '\0' };
  if (da.has_alcance_m()) {
    char string_incrementos[40] = { '\0' };
    if (da.has_incrementos()) {
      snprintf(string_incrementos, 39, ", inc %d", da.incrementos());
    }
    snprintf(string_alcance, 39, "alcance: %0.0f q%s, ", da.alcance_m() * METROS_PARA_QUADRADOS, string_incrementos);
  }
  std::string string_escudo = da.empunhadura() == ent::EA_ARMA_ESCUDO ? "(escudo)" : "";
  return google::protobuf::StringPrintf(
      "id: %s%s, %sbonus: %d, dano: %s%s, ca%s: %d toque: %d surpresa%s: %d",
      string_rotulo, da.tipo_ataque().c_str(), string_alcance, da.bonus_ataque(), da.dano().c_str(), StringCritico(da).c_str(),
      string_escudo.c_str(), da.ca_normal(),
      da.ca_toque(), string_escudo.c_str(), da.ca_surpreso());
}

std::string StringDanoParaAcao(const EntidadeProto::DadosAtaque& da, const EntidadeProto& proto) {
  int modificador_dano = ModificadorDano(proto);
  return google::protobuf::StringPrintf(
      "%s%s",
      da.dano().c_str(),
      modificador_dano != 0 ? google::protobuf::StringPrintf("%+d", modificador_dano).c_str() : "");

}

// Monta a string de dano de uma arma de um ataque, como 1d6 (x3). Nao inclui modificadores.
std::string StringDanoBasicoComCritico(const ent::EntidadeProto::DadosAtaque& da) {
  std::string critico = StringCritico(da);
  return google::protobuf::StringPrintf("%s%s", da.dano_basico_arma().c_str(), critico.empty() ? "" : critico.c_str());
}

}  // namespace ent
