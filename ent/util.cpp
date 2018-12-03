#include "ent/util.h"
#include <google/protobuf/repeated_field.h>
#include <algorithm>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>
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
#include <tuple>
#include <unordered_map>
#include "ent/acoes.h"
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/entidade.pb.h"
#include "ent/tabelas.h"
#include "ent/tabuleiro.h"
#include "gltab/gl.h"      // TODO remover e passar desenhos para para gl
#include "gltab/gl_vbo.h"  // TODO remover e passar desenhos para para gl
#include "goog/stringprintf.h"
#include "log/log.h"
#include "net/util.h"

namespace ent {

namespace {
using google::protobuf::StringPrintf;

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

void DobraMargemCritico(EntidadeProto::DadosAtaque* da) {
  int margem = 21 - da->margem_critico();
  margem *= 2;
  da->set_margem_critico(21 - margem);
}

// Distancia do ponto da matriz de modelagem para a nevoa.
float DistanciaPontoCorrenteParaNevoa(const ParametrosDesenho* pd) {
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
  return distancia;
}

}  // namespace

std::unique_ptr<ntf::Notificacao> NovaNotificacao(ntf::Tipo tipo, const EntidadeProto& proto) {
  auto n = std::unique_ptr<ntf::Notificacao>(ntf::NovaNotificacao(tipo));
  n->mutable_entidade_antes()->set_id(proto.id());
  n->mutable_entidade()->set_id(proto.id());
  return n;
}

std::tuple<ntf::Notificacao*, EntidadeProto*, EntidadeProto*> NovaNotificacaoFilha(
        ntf::Tipo tipo, const EntidadeProto& proto, ntf::Notificacao* pai) {
  auto* n = pai->add_notificacao();
  n->set_tipo(tipo);
  n->mutable_entidade_antes()->set_id(proto.id());
  n->mutable_entidade()->set_id(proto.id());
  return std::make_tuple(n, n->mutable_entidade_antes(), n->mutable_entidade());
}

void MudaCor(const float* cor) {
  gl::MudaCor(cor[0], cor[1], cor[2], 1.0f);
}

void MudaCorAplicandoNevoa(const float* cor, const ParametrosDesenho* pd) {
  if (!pd->has_nevoa()) {
    MudaCor(cor);
    return;
  }
  float distancia = DistanciaPontoCorrenteParaNevoa(pd);
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

namespace {
float CombinaComponenteCor(float c1, float c2) {
  return std::min(1.0f, c1 + c2);
}
}  // namespace

void CombinaCor(const Cor& cor_origem, Cor* cor_destino) {
  cor_destino->set_r(CombinaComponenteCor(cor_origem.r(), cor_destino->r()));
  cor_destino->set_g(CombinaComponenteCor(cor_origem.g(), cor_destino->g()));
  cor_destino->set_b(CombinaComponenteCor(cor_origem.b(), cor_destino->b()));
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
  //glClearStencil(1);
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

// Retorna a string sem os caracteres UTF-8 para desenho OpenGL.
const std::string StringSemUtf8(const std::string& texto) {
  std::string ret(texto);
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

TipoAtaque DaParaTipoAtaque(const EntidadeProto::DadosAtaque& da) {
  if (da.ataque_distancia()) return TipoAtaque::DISTANCIA;
  if (da.ataque_agarrar()) return TipoAtaque::AGARRAR;
  return TipoAtaque::CORPO_A_CORPO;
}

// Pode ser chamado com ed == default para ver alguns modificadores do atacante.
int ModificadorAtaque(TipoAtaque tipo_ataque, const EntidadeProto& ea, const EntidadeProto& ed) {
  int modificador = 0;
  // ataque.
  if (ea.caida() && tipo_ataque != TipoAtaque::DISTANCIA) {
    modificador -= 4;
  }
  if (PossuiEvento(EFEITO_INVISIBILIDADE, ea)) {
    modificador += 2;
  }
  if (tipo_ataque == TipoAtaque::AGARRAR) {
    if (PossuiTalento("agarrar_aprimorado", ea)) modificador += 4;
    if (PossuiTalento("agarrar_aprimorado", ed)) modificador -= 4;
  }
  // Nao aplica contra a entidade default.
  if (ed.has_pos() && ea.pos().z() - ed.pos().z() >= TAMANHO_LADO_QUADRADO && tipo_ataque != TipoAtaque::DISTANCIA) {
    modificador += 1;
  }

  if (ea.dados_ataque_global().ataque_menos_1()) {
    modificador -= 1;
  }
  if (ea.dados_ataque_global().ataque_menos_2()) {
    modificador -= 2;
  }
  if (ea.dados_ataque_global().ataque_menos_4()) {
    modificador -= 4;
  }
  if (ea.dados_ataque_global().ataque_menos_8()) {
    modificador -= 8;
  }
  if (ea.dados_ataque_global().ataque_mais_1()) {
    modificador += 1;
  }
  if (ea.dados_ataque_global().ataque_mais_2()) {
    modificador += 2;
  }
  if (ea.dados_ataque_global().ataque_mais_4()) {
    modificador += 4;
  }
  if (ea.dados_ataque_global().ataque_mais_8()) {
    modificador += 8;
  }

  // Defesa.
  if (ed.caida()) {
    if (tipo_ataque == TipoAtaque::DISTANCIA) modificador -= 4;
    else modificador += 4;
  }
  return modificador;
}

int ModificadorDano(const EntidadeProto& ea) {
  int modificador = 0;
  if (ea.dados_ataque_global().dano_menos_1()) {
    modificador -= 1;
  }
  if (ea.dados_ataque_global().dano_menos_2()) {
    modificador -= 2;
  }
  if (ea.dados_ataque_global().dano_menos_4()) {
    modificador -= 4;
  }
  if (ea.dados_ataque_global().dano_menos_8()) {
    modificador -= 8;
  }
  if (ea.dados_ataque_global().dano_mais_1()) {
    modificador += 1;
  }
  if (ea.dados_ataque_global().dano_mais_2()) {
    modificador += 2;
  }
  if (ea.dados_ataque_global().dano_mais_4()) {
    modificador += 4;
  }
  if (ea.dados_ataque_global().dano_mais_8()) {
    modificador += 8;
  }
  if (ea.dados_ataque_global().dano_mais_16()) {
    modificador += 16;
  }
  if (ea.dados_ataque_global().dano_mais_32()) {
    modificador += 32;
  }
  return modificador;
}

float DistanciaAcaoAoAlvoMetros(const Entidade& ea, const Entidade& ed, const Posicao& pos_alvo) {
  Posicao pos_acao_a = ea.PosicaoAcao();
  Posicao pos_acao_d = ed.PosicaoAcao();
  // Raio de acao da entidade. A partir do raio dela, numero de quadrados multiplicado pelo tamanho.
  float mult_tamanho = ea.MultiplicadorTamanho();
  float raio_a = mult_tamanho * TAMANHO_LADO_QUADRADO_2;
  VLOG(1) << "raio_a: " << raio_a;

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
  return distancia_m;
}

std::tuple<std::string, bool, float> VerificaAlcanceMunicao(const AcaoProto& ap, const Entidade& ea, const Entidade& ed, const Posicao& pos_alvo) {
  const auto* da = ea.DadoCorrente();
  if ((ap.tipo() == ACAO_PROJETIL || ap.tipo() == ACAO_PROJETIL_AREA) &&
      da!= nullptr && da->has_municao() && da->municao() == 0) {
    VLOG(1) << "Nao ha municao para ataque: " << da->DebugString();
    return std::make_tuple("Sem munição", false, 0.0f);
  }

  const float alcance_m = ea.AlcanceAtaqueMetros();
  float alcance_minimo_m = ea.AlcanceMinimoAtaqueMetros();
  float distancia_m = 0.0f;
  if (alcance_m >= 0) {
    distancia_m = DistanciaAcaoAoAlvoMetros(ea, ed, pos_alvo);
    if (distancia_m > alcance_m) {
      int total_incrementos = distancia_m / alcance_m;
      if (total_incrementos > ea.IncrementosAtaque()) {
        return std::make_tuple(
            google::protobuf::StringPrintf(
                "Fora de alcance: %0.1fm > %0.1fm, inc: %d, max: %d", distancia_m, alcance_m, total_incrementos, ea.IncrementosAtaque()),
            false, distancia_m);
      }
    } else if (alcance_minimo_m > 0 && distancia_m < alcance_minimo_m) {
      std::string texto =
          google::protobuf::StringPrintf("Alvo muito perto: alcance mínimo: %0.1fm, distância: %0.1f",
                                         alcance_minimo_m, distancia_m);
      return std::make_tuple(texto, false, distancia_m);
    }
    VLOG(1) << "alcance_m: " << alcance_m << ", distancia_m: " << distancia_m;
  }
  return std::make_tuple("", true, distancia_m);
}

int ModificadorAlcance(float distancia_m, const AcaoProto& ap, const Entidade& ea) {
  if (ap.tipo() != ACAO_PROJETIL && ap.tipo() != ACAO_PROJETIL_AREA) {
    return 0;
  }

  int modificador_incrementos = 0;
  const float alcance_m = ea.AlcanceAtaqueMetros();
  if (alcance_m >= 0) {
    int total_incrementos = distancia_m / alcance_m;
    modificador_incrementos = -2 * total_incrementos;
    VLOG(1) << "Modificador de incremento: " << modificador_incrementos << ", distancia_m: " << distancia_m << ", alcance_m: " << alcance_m;
  }
  assert(modificador_incrementos <= 0);
  return modificador_incrementos;
}

namespace {

Entidade::TipoCA CATipoAtaque(const EntidadeProto::DadosAtaque& da) {
  return da.ataque_toque() ? Entidade::CA_TOQUE : Entidade::CA_NORMAL;
}

// Retorna true se o ataque for bem sucedido, false com string caso contrario.
std::tuple<std::string, bool> AtaqueVsChanceFalha(const Entidade& ea, const Entidade& ed) {
  if (ea.IgnoraChanceFalha()) {
    VLOG(1) << "ataque ignorando chance de falha";
    return std::make_pair("", true);
  }
  VLOG(1) << "ataque: " << ea.ChanceFalhaAtaque();
  VLOG(1) << "defesa: " << ea.ChanceFalhaDefesa();
  const int chance_falha = std::max(ea.ChanceFalhaAtaque(), ed.ChanceFalhaDefesa());
  if (chance_falha > 0) {
    const int d100 = RolaDado(100);
    VLOG(1) << "Chance de falha: " << chance_falha << ", tirou: " << d100;
    if (d100 < chance_falha) {
      return std::make_tuple(google::protobuf::StringPrintf("Falhou, chance %d, tirou %d", chance_falha, d100), false);
    }
  }
  return std::make_pair("", true);
}

// Retorna o texto sinalizado para o modificador se diferente de zero, ou "".
std::string TextoOuNada(int modificador) {
  if (modificador != 0) {
    return google::protobuf::StringPrintf("%+d", modificador);
  }
  return "";
}

std::string TextoOuNada(const std::string& texto) {
  if (texto.empty()) return "";
  return texto;
}

// Retorna o numero de vezes que o critico da dano e o texto para o critico.
std::tuple<int, std::string> ComputaCritico(
    int d20, int ataque_origem, int modificador_incrementos, int outros_modificadores, int ca_destino, bool agarrar,
    const EntidadeProto::DadosAtaque* da, const Entidade& ea, const Entidade& ed) {
  assert(modificador_incrementos <= 0);
  int vezes = 1;
  std::string texto_critico;
  if (d20 >= ea.MargemCritico() && !agarrar) {
    if (ed.ImuneCritico()) {
      texto_critico = ", imune a critico";
    } else {
      int d20_critico = RolaDado(20);
      int total_critico = ataque_origem + d20_critico + modificador_incrementos + outros_modificadores;
      if (d20 == 1) {
        texto_critico = google::protobuf::StringPrintf(", critico falhou: 1");
      } else if (total_critico >= ca_destino) {
        texto_critico = google::protobuf::StringPrintf(
            ", critico %d+%d%s%s= %d",
            d20_critico, ataque_origem, TextoOuNada(modificador_incrementos).c_str(), TextoOuNada(outros_modificadores).c_str(), total_critico);
        vezes = ea.MultiplicadorCritico();
      } else {
        texto_critico = google::protobuf::StringPrintf(
            ", critico falhou: %d+%d%s%s= %d",
            d20_critico, ataque_origem, TextoOuNada(modificador_incrementos).c_str(), TextoOuNada(outros_modificadores).c_str(), total_critico);
      }
    }
  }
  return std::make_tuple(vezes, texto_critico);
}

// Retorna -1 para falha critica, 0, para falha e total para sucesso.
std::tuple<int, std::string, bool> ComputaAcertoOuErro(
    int d20, int ataque_origem, int modificador_incrementos, int outros_modificadores, int ca_destino, bool agarrar, const EntidadeProto& pea, const EntidadeProto& ped) {
  assert(modificador_incrementos <= 0);
  int total = d20 + ataque_origem + modificador_incrementos + outros_modificadores;
  if (d20 == 1) {
    VLOG(1) << "Falha critica";
    return std::make_tuple(-1, "falha critica", false);
  } else if ((d20 != 20 || agarrar) && total < ca_destino) {
    std::string texto = google::protobuf::StringPrintf(
        "falhou: %d%+d%s%s= %d < %d", d20, ataque_origem,
        TextoOuNada(modificador_incrementos).c_str(), TextoOuNada(outros_modificadores).c_str(), total, ca_destino);
    return std::make_tuple(0, texto, false);
  } else if (agarrar && total == ca_destino) {
    // Maior modificador ganha.
    if (pea.bba().agarrar() < ped.bba().agarrar()) {
      // TODO em case de empate, deveria rolar de novo. Mas vou considerar vantagem do ataque.
      return std::make_tuple(0, google::protobuf::StringPrintf("falhou: mod defesa %d > %d ataque", ped.bba().agarrar(), pea.bba().agarrar()).c_str(), false);
    }
  }
  return std::make_tuple(total, "", true);
}

// Retorna o resultado do ataque de toque o se acertou ou nao.
std::tuple<std::string, bool> AtaqueToquePreAgarrar(int outros_modificadores, const EntidadeProto::DadosAtaque* da, const Entidade& ea, const Entidade& ed) {
  // TODO: aqui to hackeando pra pular o toque se ambos estiverem agarrando.
  if (da->ignora_ataque_toque() || AgarradoA(ed.Id(), ea.Proto())) {
    return std::make_tuple("", true);
  }
  const int d20_toque = RolaDado(20);
  const int ca_destino_toque = ed.CA(ea, Entidade::CA_TOQUE);
  const int ataque_toque = ea.BonusAtaqueToque();
  std::string texto_erro;
  bool acertou;
  std::tie(std::ignore, texto_erro, acertou) = ComputaAcertoOuErro(d20_toque, ataque_toque, 0, outros_modificadores, ca_destino_toque, false, ea.Proto(), ed.Proto());
  if (!acertou) {
    std::string texto_falha_toque = google::protobuf::StringPrintf("Ataque de toque falhou: %s", texto_erro.c_str());
    return std::make_tuple(texto_falha_toque, false);
  }
  std::string texto = google::protobuf::StringPrintf(", toque ok: %d%+d < %d", d20_toque, ataque_toque, ca_destino_toque);
  return std::make_tuple(texto, true);
}

enum resultado_ataque_reflexos {
  RAR_ACERTOU = 0,
  RAR_FALHA_NORMAL = 1,
  RAR_FALHA_CRITICA = 2,
};
std::tuple<std::string, resultado_ataque_reflexos> AtaqueToqueReflexos(
    int outros_modificadores, const EntidadeProto::DadosAtaque* da, const Entidade& ea, const Entidade& ed) {
  const int d20 = RolaDado(20);
  if (d20 == 1) {
    return std::make_tuple("falha crítica", RAR_FALHA_CRITICA);
  }
  const int ca_reflexo = ed.CAReflexos();
  const int ataque = ea.BonusAtaque();
  std::string texto_erro;
  bool acertou;
  std::tie(std::ignore, texto_erro, acertou) =
      ComputaAcertoOuErro(d20, ataque, 0, outros_modificadores, ca_reflexo, false, ea.Proto(), ed.Proto());
  return std::make_tuple(
    acertou ? "acertou reflexo" : "errou reflexo", 
    acertou ? RAR_ACERTOU : RAR_FALHA_NORMAL);
}

}  // namespace

ResultadoAtaqueVsDefesa AtaqueVsDefesa(
    float distancia_m, const AcaoProto& ap, const Entidade& ea, const Entidade& ed, const Posicao& pos_alvo) {
  const auto* da = ea.DadoCorrente();
  if (da == nullptr) da = &EntidadeProto::DadosAtaque::default_instance();
  return AtaqueVsDefesa(distancia_m, ap, ea, da, ed, pos_alvo);
}

ResultadoAtaqueVsDefesa AtaqueVsDefesa(
    float distancia_m, const AcaoProto& ap, const Entidade& ea, const EntidadeProto::DadosAtaque* da,
    const Entidade& ed, const Posicao& pos_alvo) {
  const int ataque_origem = ea.BonusAtaque();

  const int d20_agarrar_defesa = RolaDado(20);
  const int bonus_agarrar_defesa = ed.Proto().bba().agarrar();

  const int ca_destino = da->ataque_agarrar() ? d20_agarrar_defesa + bonus_agarrar_defesa : ed.CA(ea, CATipoAtaque(*da));

  ResultadoAtaqueVsDefesa resultado;
  if (ataque_origem == Entidade::AtaqueCaInvalido || ca_destino == Entidade::AtaqueCaInvalido) {
    VLOG(1) << "Ignorando ataque vs defesa por falta de dados: ataque: " << ataque_origem
            << ", defesa: " << ca_destino;
    resultado.texto = "Ataque sem bonus ou defensor sem armadura";
    return resultado; 
  }
  if (EmDefesaTotal(ea.Proto())) {
    VLOG(1) << "Ignorando ataque vs defesa por causa de defesa total.";
    resultado.texto = "Atacante em defesa total";
    return resultado; 
  }
  int modificador_incrementos = ModificadorAlcance(distancia_m, ap, ea);
  const int outros_modificadores = ModificadorAtaque(DaParaTipoAtaque(*da), ea.Proto(), ed.Proto());

  const int numero_reflexos = NumeroReflexos(ed.Proto());
  if (numero_reflexos > 0) {
    int num_total = numero_reflexos + 1;
    int dref = RolaDado(num_total);
    VLOG(1) << StringPrintf("dado ref: %d em d%d", dref, num_total);
    if (dref != 1) {
      VLOG(1) << "Ataque no reflexo";
      // Ataque no reflexo.
      resultado_ataque_reflexos rar;
      std::string texto_reflexo;
      std::tie(texto_reflexo, rar) = AtaqueToqueReflexos(outros_modificadores, da, ea, ed);
      resultado.texto = StringPrintf("%s, %d de %d", texto_reflexo.c_str(), dref - 1, num_total - 1);
      if (rar == RAR_ACERTOU) {
        resultado.resultado = RA_FALHA_REFLEXO;
      } else {
        resultado.resultado = (rar == RAR_FALHA_CRITICA) ? RA_FALHA_CRITICA : RA_FALHA_NORMAL;
      }
      return resultado;
    }
    VLOG(1) << "Ataque acertou alvo mesmo com reflexo";
  }

  // Realiza um ataque de toque.
  std::string texto_toque_agarrar;
  if (da->ataque_agarrar()) {
    bool acertou;
    std::tie(texto_toque_agarrar, acertou) = AtaqueToquePreAgarrar(outros_modificadores, da, ea, ed);
    if (!acertou) {
      resultado.resultado = RA_FALHA_TOQUE_AGARRAR;
      resultado.texto = texto_toque_agarrar;
      return resultado;
    }
  }

  // Rola o dado de ataque!
  int d20 = RolaDado(20);

  // Acerto ou erro.
  int total;
  {
    bool acertou;
    std::tie(total, resultado.texto, acertou) =
        ComputaAcertoOuErro(
            d20, ataque_origem, modificador_incrementos, outros_modificadores, ca_destino, da->ataque_agarrar(),
            ea.Proto(), ed.Proto());
    if (!acertou) {
      resultado.resultado = total == -1 ? RA_FALHA_CRITICA : RA_FALHA_NORMAL;
      return resultado; 
    }
  }

  // Chance de falha.
  {
    bool passou_falha;
    std::tie(resultado.texto, passou_falha) = AtaqueVsChanceFalha(ea, ed);
    if (!passou_falha) {
      resultado.resultado = RA_FALHA_CHANCE_FALHA;
      return resultado; 
    }
  }

  // Imunidade: nao texta resistencia porque aqui eh so pra ver se acertou e como..
  if (EntidadeImuneElemento(ed.Proto(), ap.elemento())) {
    resultado.resultado = RA_FALHA_IMUNE;
    resultado.texto = StringPrintf("defensor imune a %s", TextoDescritor(ap.elemento()));
    return resultado; 
  }

  // Se chegou aqui acertou.
  resultado.resultado = RA_SUCESSO;
  std::string texto_critico;
  std::tie(resultado.vezes, texto_critico) =
    ComputaCritico(
        d20, ataque_origem, modificador_incrementos, outros_modificadores, ca_destino, da->ataque_agarrar(),
        da, ea, ed);

  const std::string str_ca_destino = da->ataque_agarrar()
      ? StringPrintf("%d=%d+d20", ca_destino, ed.Proto().bba().agarrar())
      : StringPrintf("%d", ca_destino);

  resultado.texto =
      StringPrintf("acertou: %d+%d%s%s= %d%s vs %s%s",
           d20, ataque_origem, TextoOuNada(modificador_incrementos).c_str(), TextoOuNada(outros_modificadores).c_str(), total,
           texto_critico.c_str(), str_ca_destino.c_str(), TextoOuNada(texto_toque_agarrar).c_str());
  VLOG(1) << "Resultado ataque vs defesa: " << resultado.texto << ", vezes: " << resultado.vezes;
  return resultado; 
}

// Retorna o delta pontos de vida e a string do resultado.
// A fracao eh para baixo mas com minimo de 1, segundo regra de rounding fractions, exception.
std::tuple<int, bool, std::string> AtaqueVsSalvacao(const AcaoProto& ap, const Entidade& ea, const Entidade& ed) {
  std::string descricao_resultado;
  int delta_pontos_vida = ap.delta_pontos_vida();
  bool salvou = false;

  if (ed.TemProximaSalvacao()) {
    if (ed.ProximaSalvacao() == RS_MEIO) {
      delta_pontos_vida = delta_pontos_vida == -1 ? -1 : delta_pontos_vida / 2;
      descricao_resultado = google::protobuf::StringPrintf("salvacao manual 1/2: dano %d", -delta_pontos_vida);
      salvou = true;
    } else if (ed.ProximaSalvacao() == RS_QUARTO) {
      delta_pontos_vida /= 4;
      descricao_resultado = google::protobuf::StringPrintf("salvacao manual 1/4: dano %d", -delta_pontos_vida);
      salvou = true;
    } else if (ed.ProximaSalvacao() == RS_ANULOU) {
      delta_pontos_vida = 0;
      descricao_resultado = "salvacao manual anulou";
      salvou = true;
    } else {
      descricao_resultado = "salvacao manual falhou";
    }
  } else if (ap.has_dificuldade_salvacao()) {
    int d20 = RolaDado(20);
    int bonus = ed.Salvacao(ea, ap.tipo_salvacao());
    int total = d20 + bonus;
    std::string str_evasao;
    if (total >= ap.dificuldade_salvacao()) {
      salvou = true;
      if (ap.resultado_salvacao() == RS_MEIO) {
        if (ap.tipo_salvacao() == TS_REFLEXO && PossuiHabilidadeEspecial("evasao", ed.Proto())) {
          delta_pontos_vida = 0;
          str_evasao = " (evasão)";
        } else {
          delta_pontos_vida = delta_pontos_vida == -1 ? -1 : delta_pontos_vida / 2;
        }
      } else if (ap.resultado_salvacao() == RS_QUARTO) {
        delta_pontos_vida /= 4;
      } else {
        delta_pontos_vida = 0;
      }
      descricao_resultado = google::protobuf::StringPrintf(
          "salvacao sucesso: %d%+d >= %d, dano: %d%s", d20, bonus, ap.dificuldade_salvacao(), -delta_pontos_vida, str_evasao.c_str());
    } else {
      if (ap.resultado_salvacao() == RS_MEIO && ap.tipo_salvacao() == TS_REFLEXO && PossuiHabilidadeEspecial("evasao_aprimorada", ed.Proto())) {
        delta_pontos_vida = delta_pontos_vida == 1 ? 1 : delta_pontos_vida / 2;
        str_evasao = " (evasão aprimorada)";
      }
      descricao_resultado = google::protobuf::StringPrintf(
          "salvacao falhou: %d%+d < %d, dano: %d%s", d20, bonus, ap.dificuldade_salvacao(), -delta_pontos_vida, str_evasao.c_str());
    }
  } else {
    salvou = true;
    descricao_resultado = google::protobuf::StringPrintf("salvacao: acao sem dificuldade e alvo sem salvacao, dano: %d", -delta_pontos_vida);
  }
  return std::make_tuple(delta_pontos_vida, salvou, descricao_resultado);
}

std::tuple<bool, std::string> AtaqueVsResistenciaMagia(const AcaoProto& ap, const Entidade& ea, const Entidade& ed) {
  const int rm = ed.Proto().dados_defesa().resistencia_magia();
  if (rm == 0) {
    return std::make_tuple(true, "");;
  }
  const int d20 = RolaDado(20);
  const int nivel_conjurador = ea.NivelConjurador();
  const int total = d20 + nivel_conjurador;

  if (d20 + nivel_conjurador < rm) {
    return std::make_tuple(false, google::protobuf::StringPrintf("RM falhou: %d < %d", total, rm));
  }
  return std::make_tuple(
      true, google::protobuf::StringPrintf("RM sucesso: %d >= %d", total, rm));
}

namespace {

std::string EntidadeNotificacao(const Tabuleiro& tabuleiro, const ntf::Notificacao& n) {
  auto* entidade = tabuleiro.BuscaEntidade(n.entidade().id());
  return RotuloEntidade(entidade);
}

}  // namespace

std::string RotuloEntidade(const Entidade* entidade) {
  if (entidade == nullptr) {
    return "null";
  }
  return RotuloEntidade(entidade->Proto());
}

std::string RotuloEntidade(const EntidadeProto& proto) {
  if (!proto.rotulo().empty()) {
    return proto.rotulo();
  }
  return net::to_string(proto.id());
}

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
    case ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL: {
      return std::string("entidade ") + EntidadeNotificacao(tabuleiro, n) + " atualizada: " + n.entidade().ShortDebugString();
    }
    default:
      return "";
  }
}

// O delta de pontos de vida afeta outros bits tambem.
void PreencheNotificacaoAtualizaoPontosVida(
    const Entidade& entidade, int delta_pontos_vida, tipo_dano_e td, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL);
  auto* entidade_depois = n->mutable_entidade();
  entidade_depois->set_id(entidade.Id());

  if (delta_pontos_vida > 0) {
    if (delta_pontos_vida >= entidade_depois->dano_nao_letal()) {
      entidade_depois->set_dano_nao_letal(0);
    } else {
      entidade_depois->set_dano_nao_letal(entidade_depois->dano_nao_letal() - delta_pontos_vida);
    }
  } else if (delta_pontos_vida < 0 && td == TD_NAO_LETAL) {
    entidade_depois->set_dano_nao_letal(entidade.DanoNaoLetal() - delta_pontos_vida);
    entidade_depois->set_pontos_vida(entidade.PontosVida());
  } else if (delta_pontos_vida < 0 && entidade.PontosVidaTemporarios() > 0) {
    // Tira dos temporarios.
    *entidade_depois->mutable_pontos_vida_temporarios_por_fonte() = entidade.Proto().pontos_vida_temporarios_por_fonte();
    auto* bpv = entidade_depois->mutable_pontos_vida_temporarios_por_fonte();
    auto* bi = BonusIndividualSePresente(TB_SEM_NOME, bpv);
    if (bi != nullptr) {
      for (int i_origem = 0; delta_pontos_vida < 0 && i_origem < bi->por_origem_size(); ++i_origem)  {
        auto* po = bi->mutable_por_origem(i_origem);
        if (po->valor() < 0) {
          LOG(WARNING) << "Valor de pv temporario invalido: " << po->valor();
        } else if (po->valor() >= abs(delta_pontos_vida)) {
          delta_pontos_vida = 0;
          po->set_valor(po->valor() - abs(delta_pontos_vida));
        } else {
          delta_pontos_vida += po->valor();
          po->set_valor(0);
        }
      }
    }
  }
  entidade_depois->set_pontos_vida(entidade.PontosVida() + (td == TD_NAO_LETAL ? 0 : delta_pontos_vida));

  if (n_desfazer != nullptr) {
    n_desfazer->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL);
    n_desfazer->mutable_entidade()->CopyFrom(*entidade_depois);
    auto* entidade_antes = n_desfazer->mutable_entidade_antes();
    entidade_antes->set_id(entidade.Id());
    *entidade_antes->mutable_pontos_vida_temporarios_por_fonte() = entidade.Proto().pontos_vida_temporarios_por_fonte();
    entidade_antes->set_pontos_vida(entidade.PontosVida());
    entidade_antes->set_dano_nao_letal(entidade.DanoNaoLetal());
    entidade_antes->set_morta(entidade.Proto().morta());
    entidade_antes->set_caida(entidade.Proto().caida());
    entidade_antes->set_voadora(entidade.Proto().voadora());
    entidade_antes->set_aura(entidade.Proto().aura());
    *entidade_antes->mutable_pos() = entidade.Pos();
    entidade_antes->mutable_direcao_queda()->CopyFrom(entidade.Proto().direcao_queda());
  }
}

std::pair<EntidadeProto*, EntidadeProto*> PreencheNotificacaoEntidade(
    ntf::Tipo tipo, const Entidade& entidade, ntf::Notificacao* n) {
  n->set_tipo(tipo);
  n->mutable_entidade_antes()->set_id(entidade.Id());
  n->mutable_entidade()->set_id(entidade.Id());
  return std::make_pair(n->mutable_entidade_antes(), n->mutable_entidade());
}

namespace {

bool AtaqueIgual(const EntidadeProto::DadosAtaque& lda, const EntidadeProto::DadosAtaque& rda) {
  return lda.rotulo() == rda.rotulo() &&
         lda.tipo_ataque() == rda.tipo_ataque() &&
         lda.grupo() == rda.grupo();
}

// Encontra determinado dado de ataque em um proto. Retorna nullptr caso nao encontre.
EntidadeProto::DadosAtaque* EncontraAtaque(const EntidadeProto::DadosAtaque& da, EntidadeProto* proto) {
  for (auto& pda : *proto->mutable_dados_ataque()) {
    if (AtaqueIgual(pda, da)) {
      return &pda;
    }
  }
  return nullptr;
}
}  // namespace

void PreencheNotificacaoConsumoAtaque(
    const Entidade& entidade, const EntidadeProto::DadosAtaque& da, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  EntidadeProto *proto = nullptr, *proto_antes = nullptr;
  std::tie(proto_antes, proto) =
      PreencheNotificacaoEntidade(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, entidade, n);
  *proto_antes->mutable_dados_ataque() = entidade.Proto().dados_ataque();
  *proto->mutable_dados_ataque() = entidade.Proto().dados_ataque();
  auto* da_depois = EncontraAtaque(da, proto);
  if (da_depois != nullptr) {
    if (da.requer_carregamento()) {
      da_depois->set_descarregada(true);
    }
    if (da_depois->has_limite_vezes()) {
      da_depois->set_limite_vezes(da_depois->limite_vezes() - 1);
    }
    if (da_depois->has_municao()) {
      da_depois->set_municao(std::max((int)(da_depois->municao() - 1), 0));
    }
  }
  if (n_desfazer != nullptr) {
    *n_desfazer  = *n;
  }
}

void PreencheNotificacaoRecarregamento(
    const Entidade& entidade, const EntidadeProto::DadosAtaque& da, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  EntidadeProto *proto = nullptr, *proto_antes = nullptr;
  std::tie(proto_antes, proto) =
      PreencheNotificacaoEntidade(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, entidade, n);
  *proto_antes->mutable_dados_ataque() = entidade.Proto().dados_ataque();
  *proto->mutable_dados_ataque() = entidade.Proto().dados_ataque();
  auto* da_depois = EncontraAtaque(da, proto);
  if (da_depois != nullptr) {
    da_depois->set_descarregada(false);
  }
  if (n_desfazer != nullptr) {
    *n_desfazer  = *n;
  }
}

void PreencheNotificacaoEventoContinuo(const Entidade& entidade, TipoEfeito tipo_efeito, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  EntidadeProto *e_antes, *e_depois;
  std::tie(e_antes, e_depois) = PreencheNotificacaoEntidade(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, entidade, n);
  auto* evento = AdicionaEvento(entidade.Proto().evento(), tipo_efeito, /*rodadas=*/0, /*continuo*/true, e_depois);
  auto* evento_antes = e_antes->add_evento();
  *evento_antes = *evento;
  evento_antes->set_rodadas(-1);
  if (n_desfazer != nullptr) {
    *n_desfazer = *n;
  }
}

void PreencheNotificacaoEvento(const Entidade& entidade, TipoEfeito tipo_efeito, int rodadas, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  EntidadeProto *e_antes, *e_depois;
  std::tie(e_antes, e_depois) = PreencheNotificacaoEntidade(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, entidade, n);
  auto* evento = AdicionaEvento(entidade.Proto().evento(), tipo_efeito, rodadas, false, e_depois);
  auto* evento_antes = e_antes->add_evento();
  *evento_antes = *evento;
  evento_antes->set_rodadas(-1);
  if (n_desfazer != nullptr) {
    *n_desfazer = *n;
  }
}

void PreencheNotificacaoEvento(
    const Entidade& entidade, TipoEfeito tipo_efeito, const std::string& complemento_str, int rodadas, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  EntidadeProto *e_antes, *e_depois;
  std::tie(e_antes, e_depois) = PreencheNotificacaoEntidade(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, entidade, n);
  auto* evento = AdicionaEvento(entidade.Proto().evento(), tipo_efeito, rodadas, false, e_depois);
  evento->add_complementos_str(complemento_str);
  auto* evento_antes = e_antes->add_evento();
  *evento_antes = *evento;
  evento_antes->set_rodadas(-1);
  if (n_desfazer != nullptr) {
    *n_desfazer = *n;
  }
}

namespace {
// Mapeia o tipo de dano de veneno para o indice de complemento.
// Retorna -1 se tipo de dano nao for de atributo.
int TipoDanoParaComplemento(TipoDanoVeneno tipo) {
  switch (tipo) {
    case TDV_FORCA: return 0;
    case TDV_DESTREZA: return 1;
    case TDV_CONSTITUICAO: return 2;
    case TDV_INTELIGENCIA: return 3;
    case TDV_SABEDORIA: return 4;
    case TDV_CARISMA: return 5;
    default: return -1;
  }
}

bool PreencheNotificacaoEventoParaVenenoComum(const Entidade& entidade, const VenenoProto& veneno, int rodadas, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  PreencheNotificacaoEvento(entidade, EFEITO_DANO_ATRIBUTO_VENENO, DIA_EM_RODADAS, n, n_desfazer);
  if (n->entidade().evento_size() != 1) {
    LOG(ERROR) << "Falha criando veneno: tamanho de evento invalido, " << n->entidade().evento_size();
    return false;
  }
  return true;
}
}  // namespace

void PreencheNotificacaoEventoParaVenenoPrimario(const Entidade& entidade, const VenenoProto& veneno, int rodadas, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  if (!PreencheNotificacaoEventoParaVenenoComum(entidade, veneno, rodadas, n, n_desfazer)) return;
  auto* e_depois = n->mutable_entidade();
  auto* evento = e_depois->mutable_evento(0);
  evento->mutable_complementos()->Resize(6, 0);
  for (int i = 0; i < veneno.tipo_dano().size(); ++i) {
    int indice_complemento = TipoDanoParaComplemento(veneno.tipo_dano(i));
    if (indice_complemento < 0 || indice_complemento >= 6) continue;
    if (veneno.dano_inicial().size() != veneno.tipo_dano().size()) {
      LOG(ERROR) << "Veneno mal formado: tamanho de dano incial e tipo dano diferem: " << veneno.dano_inicial().size() << ", " << veneno.tipo_dano().size();
      continue;
    }
    evento->mutable_complementos()->Set(indice_complemento, -RolaValor(veneno.dano_inicial(i)));
  }
}

void PreencheNotificacaoEventoParaVenenoSecundario(const Entidade& entidade, const VenenoProto& veneno, int rodadas, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  if (!PreencheNotificacaoEventoParaVenenoComum(entidade, veneno, rodadas, n, n_desfazer)) return;
  auto* e_depois = n->mutable_entidade();
  auto* evento = e_depois->mutable_evento(0);
  evento->mutable_complementos()->Resize(6, 0);
  for (int i = 0; i < veneno.tipo_dano_secundario().size(); ++i) {
    int indice_complemento = TipoDanoParaComplemento(veneno.tipo_dano_secundario(i));
    if (indice_complemento < 0 || indice_complemento >= 6) continue;
    if (veneno.dano_secundario().size() != veneno.tipo_dano_secundario().size()) {
      LOG(ERROR) << "Veneno mal formado: tamanho de dano secundario e tipo dano secundario diferem: "
                 << veneno.dano_secundario().size() << ", " << veneno.tipo_dano_secundario().size();
      continue;
    }
    evento->mutable_complementos()->Set(indice_complemento, -RolaValor(veneno.dano_secundario(i)));
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
    case TB_SINERGIA:
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

int CATotal(const EntidadeProto& proto, bool permite_escudo, const Bonus& outros_bonus) {
  Bonus ca(outros_bonus);
  CombinaBonus(proto.dados_defesa().ca(), &ca);
  return BonusTotalExcluindo(ca, ExclusaoEscudo(permite_escudo));
}

int CASurpreso(const EntidadeProto& proto, bool permite_escudo, const Bonus& outros_bonus) {
  Bonus ca(outros_bonus);
  CombinaBonus(proto.dados_defesa().ca(), &ca);
  std::vector<TipoBonus> exclusao = ExclusaoEscudo(permite_escudo);
  exclusao.push_back(TB_ESQUIVA);
  const int modificador_destreza = ModificadorAtributo(proto.atributos().destreza());
  return BonusTotalExcluindo(ca, exclusao) - std::max(modificador_destreza, 0);
}

int CAToque(const EntidadeProto& proto, const Bonus& outros_bonus) {
  Bonus ca(outros_bonus);
  CombinaBonus(proto.dados_defesa().ca(), &ca);
  return BonusTotalExcluindo(ca,
         { TB_ARMADURA, TB_ESCUDO, TB_ARMADURA_NATURAL,
           TB_ARMADURA_MELHORIA, TB_ESCUDO_MELHORIA, TB_ARMADURA_NATURAL_MELHORIA });
}

int CAToqueSurpreso(const EntidadeProto& proto, const Bonus& outros_bonus) {
  Bonus ca(outros_bonus);
  CombinaBonus(proto.dados_defesa().ca(), &ca);
  const int modificador_destreza = ModificadorAtributo(proto.atributos().destreza());
  return BonusTotalExcluindo(ca,
         { TB_ARMADURA, TB_ESCUDO, TB_ARMADURA_NATURAL,
           TB_ARMADURA_MELHORIA, TB_ESCUDO_MELHORIA, TB_ARMADURA_NATURAL_MELHORIA,
           TB_ESQUIVA }) - std::max(modificador_destreza, 0);
}

bool ArmaDupla(const ArmaProto& arma) { return arma.has_dano_secundario(); }
bool ArmaDistancia(const ArmaProto& arma) { return arma.alcance_quadrados() > 0; }

bool PossuiBonus(TipoBonus tipo, const Bonus& bonus) {
  for (const auto& bi : bonus.bonus_individual()) {
    if (bi.tipo() == tipo) {
      return true;
    }
  }
  return false;
}

void CombinaBonus(const Bonus& bonus_novos, Bonus* bonus) {
  for (const auto& bi : bonus_novos.bonus_individual()) {
    for (const auto& po : bi.por_origem()) {
      AtribuiBonus(po.valor(), bi.tipo(), po.origem(), bonus);
    }
  }
}

void CombinaAtributos(const Atributos& atributos_novos, Atributos* atributos) {
  CombinaBonus(atributos_novos.forca(), atributos->mutable_forca());
  CombinaBonus(atributos_novos.destreza(), atributos->mutable_destreza());
  CombinaBonus(atributos_novos.constituicao(), atributos->mutable_constituicao());
  CombinaBonus(atributos_novos.inteligencia(), atributos->mutable_inteligencia());
  CombinaBonus(atributos_novos.sabedoria(), atributos->mutable_sabedoria());
  CombinaBonus(atributos_novos.carisma(), atributos->mutable_carisma());
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

// Acesso.
BonusIndividual* BonusIndividualSePresente(TipoBonus tipo, Bonus* bonus) {
  if (bonus == nullptr) return nullptr;
  for (auto& bi : *bonus->mutable_bonus_individual()) {
    if (bi.tipo() == tipo) {
      return &bi;
    }
  }
  return nullptr;
}

BonusIndividual::PorOrigem* OrigemSePresente(const std::string& origem, BonusIndividual* bonus_individual) {
  if (bonus_individual == nullptr) return nullptr;
  for (auto& po : *bonus_individual->mutable_por_origem()) {
    if (po.origem() == origem) return &po;
  }
  return nullptr;
}

BonusIndividual::PorOrigem* OrigemSePresente(TipoBonus tipo, const std::string& origem, Bonus* bonus) {
  return OrigemSePresente(origem, BonusIndividualSePresente(tipo, bonus));
}
// Fim acesso.

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

// Atribui o bonus se valor != 0, remove caso contrario.
void AtribuiOuRemoveBonus(int valor, TipoBonus tipo, const std::string& origem, Bonus* bonus) {
  if (valor != 0) {
    AtribuiBonus(valor, tipo, origem, bonus);
  } else {
    RemoveBonus(tipo, origem, bonus);
  }
}

int CalculaBonusBaseAtaque(const EntidadeProto& proto) {
  int bba = 0;
  for (const auto& info_classe : proto.info_classes()) {
    bba += info_classe.bba();
  }
  return bba;
}

// Retorna o bonus de ataque para uma determinada arma.
int CalculaBonusBaseParaAtaque(const EntidadeProto::DadosAtaque& da, const EntidadeProto& proto) {
  return BonusTotal(da.bonus_ataque());
}

// Retorna a string de dano para uma arma.
std::string CalculaDanoParaAtaque(const EntidadeProto::DadosAtaque& da, const EntidadeProto& proto) {
  const int mod_final = BonusTotal(da.bonus_dano());
  return da.dano_basico().c_str() + (mod_final != 0 ? google::protobuf::StringPrintf("%+d", mod_final) : "");
}

std::string DanoBasicoPorTamanho(TamanhoEntidade tamanho, const StringPorTamanho& dano) {
  if (dano.has_invariavel()) {
    return dano.invariavel();
  }
  switch (tamanho) {
    case TM_MEDIO: return dano.medio();
    case TM_PEQUENO: return dano.pequeno();
    case TM_GRANDE: return dano.grande();
    default: return "";
  }
}

// Retorna a arma da outra mao.
const ArmaProto& ArmaOutraMao(
    const Tabelas& tabelas, const EntidadeProto::DadosAtaque& da_mao, const EntidadeProto& proto) {
  const EntidadeProto::DadosAtaque* da_outra_mao = &da_mao;
  for (const auto& da : proto.dados_ataque()) {
    if (da.rotulo() == da_mao.rotulo() && da.empunhadura() != da_mao.empunhadura()) {
      da_outra_mao = &da;
      break;
    }
  }
  if (da_outra_mao == &da_mao) LOG(WARNING) << "Nao encontrei a arma na outra mao, fallback pro mesmo tipo";
  for (const auto& da : proto.dados_ataque()) {
    if (da.tipo_ataque() == da_mao.tipo_ataque() && da.empunhadura() != da_mao.empunhadura()) {
      da_outra_mao = &da;
      break;
    }
  }
  if (da_outra_mao == &da_mao) {
    LOG(ERROR) << "Nao encontrei a arma na outra mao, retornando a mesma: " << da_mao.id_arma();
  }
  return tabelas.Arma(da_outra_mao->id_arma());
}

namespace {
google::protobuf::RepeatedField<int> TiposDanoParaAtaqueFisico(const google::protobuf::RepeatedField<int>& tipos_dano) {
  google::protobuf::RepeatedField<int> tipos_ataque_fisico;
  for (int td : tipos_dano) {
    switch (td) {
      case TD_CORTANTE: tipos_ataque_fisico.Add(DESC_CORTANTE); break;
      case TD_PERFURANTE: tipos_ataque_fisico.Add(DESC_PERFURANTE); break;
      case TD_CONCUSSAO: tipos_ataque_fisico.Add(DESC_ESTOURANTE); break;
      default: ;
    }
  }
  return tipos_ataque_fisico;
}
}  // namespace

void RecomputaDependenciasArma(const Tabelas& tabelas, const EntidadeProto& proto, EntidadeProto::DadosAtaque* da) {
  // Passa alguns campos da acao para o ataque.
  const auto& arma = tabelas.ArmaOuFeitico(da->id_arma());
  ArmaParaDadosAtaque(tabelas, arma, proto, da);
  if (arma.has_id()) {
    if (da->rotulo().empty()) da->set_rotulo(arma.nome());
    da->set_acuidade(false);
    da->set_nao_letal(arma.nao_letal());
    if (PossuiTalento("acuidade_arma", proto) &&
        (PossuiCategoria(CAT_LEVE, arma) ||
         arma.id() == "sabre" || arma.id() == "chicote" || arma.id() == "corrente_com_cravos")) {
      da->set_acuidade(true);
    }
    da->set_requer_carregamento(arma.carregamento().requer_carregamento());

    // tipo certo de ataque.
    const bool projetil_area = PossuiCategoria(CAT_PROJETIL_AREA, arma);
    const bool distancia = PossuiCategoria(CAT_DISTANCIA, arma);
    const bool cac = PossuiCategoria(CAT_CAC, arma);
    if (distancia && cac) {
      // Se tipo nao estiver selecionado, pode ser qualquer um dos dois. Preferencia para distancia.
      if (da->tipo_ataque() != "Ataque Corpo a Corpo" && da->tipo_ataque() != "Ataque a Distância") {
        da->set_tipo_ataque("Ataque a Distância");
        da->set_tipo_acao(ACAO_PROJETIL);
      }
    } else if (cac) {
      da->set_tipo_ataque("Ataque Corpo a Corpo");
      da->set_tipo_acao(ACAO_CORPO_A_CORPO);
    } else if (projetil_area) {
      da->set_tipo_ataque("Projétil de Área");
      da->set_tipo_acao(ACAO_PROJETIL_AREA);
    } else if (distancia) {
      da->set_tipo_ataque("Ataque a Distância");
      da->set_tipo_acao(ACAO_PROJETIL);
    }
    da->set_ataque_distancia(distancia && da->tipo_ataque() != "Ataque Corpo a Corpo");

    if (da->empunhadura() == EA_MAO_RUIM && PossuiCategoria(CAT_ARMA_DUPLA, arma) && arma.has_dano_secundario()) {
      da->set_dano_basico(DanoBasicoPorTamanho(proto.tamanho(), arma.dano_secundario()));
      da->set_margem_critico(arma.margem_critico_secundario());
      da->set_multiplicador_critico(arma.multiplicador_critico_secundario());
    } else if (arma.has_dano()) {
      da->set_dano_basico(DanoBasicoPorTamanho(proto.tamanho(), arma.dano()));
      da->set_margem_critico(arma.margem_critico());
      da->set_multiplicador_critico(arma.multiplicador_critico());
    }
    if (PossuiTalento("sucesso_decisivo_aprimorado", da->id_arma(), proto)) {
      DobraMargemCritico(da);
    }
    if (PossuiCategoria(CAT_ARREMESSO, arma) || PossuiCategoria(CAT_PROJETIL_AREA, arma)) {
      da->set_incrementos(5);
    } else if (PossuiCategoria(CAT_DISTANCIA, arma)) {
      da->set_incrementos(10);
    }
    *da->mutable_tipo_ataque_fisico() = TiposDanoParaAtaqueFisico(arma.tipo_dano());
  } else if (da->ataque_agarrar()) {
    if (!da->has_dano_basico()) {
      da->set_dano_basico(DanoDesarmadoPorTamanho(proto.tamanho()));
    }
    if (PossuiTalento("agarrar_aprimorado", proto) || da->acao().ignora_ataque_toque()) {
      da->set_ignora_ataque_toque(true);
    } else {
      da->clear_ignora_ataque_toque();
    }
  }
  if (da->has_dano_basico_fixo()) {
    da->set_dano_basico(da->dano_basico_fixo());
  }
  // Tenta achar o primeiro da lista com mesmo rotulo para obter coisas derivadas do primeiro (municao, descritores).
  const EntidadeProto::DadosAtaque* primeiro = nullptr;
  for (const auto& dda : proto.dados_ataque()) {
    if (dda.rotulo() == da->rotulo()) {
      primeiro = &dda;
      break;
    }
  }

  if (da != primeiro && primeiro != nullptr) {
    // municao.
    if (primeiro->has_municao()) da->set_municao(primeiro->municao());
    da->set_descarregada(primeiro->descarregada());
    // Elemento.
    if (primeiro->has_acao()) *da->mutable_acao() = primeiro->acao();
    // material.
    if (primeiro->material_arma() == DESC_NENHUM) da->clear_material_arma();
    else da->set_material_arma(primeiro->material_arma());
    // tipo ataque fisico.
    if (primeiro->tipo_ataque_fisico().empty()) da->tipo_ataque_fisico();
    else *da->mutable_tipo_ataque_fisico() = primeiro->tipo_ataque_fisico();
    // Alinhamento.
    if (primeiro->has_alinhamento()) da->set_alinhamento(primeiro->alinhamento());
    else da->clear_alinhamento();
  }
  // Descritores de ataque.

  auto* acao = da->mutable_acao();
  acao->clear_descritores_ataque();
  if (da->material_arma() != DESC_NENHUM) acao->add_descritores_ataque(da->material_arma());
  if (da->alinhamento() != DESC_NENHUM) acao->add_descritores_ataque(da->alinhamento());
  if (!da->tipo_ataque_fisico().empty()) {
    std::copy(da->tipo_ataque_fisico().begin(),
              da->tipo_ataque_fisico().end(),
              google::protobuf::RepeatedFieldBackInserter(acao->mutable_descritores_ataque()));
  }
  // Alcance do ataque. Se a arma tiver alcance, respeita o que esta nela (armas a distancia). Caso contrario, usa o tamanho.
  if (arma.has_alcance_quadrados()) {
    int mod_distancia_quadrados = 0;
    const int nivel = NivelParaFeitico(tabelas, *da, proto);
    switch (arma.modificador_alcance()) {
      case ArmaProto::MOD_2_QUAD_NIVEL:
        mod_distancia_quadrados = 2 * nivel;
        break;
      case ArmaProto::MOD_8_QUAD_NIVEL:
        mod_distancia_quadrados = 8 * nivel;
        break;
      default:
        ;
    }
    da->set_alcance_m((arma.alcance_quadrados() + mod_distancia_quadrados) * QUADRADOS_PARA_METROS);
    da->set_alcance_minimo_m(0);
  } else if (da->tipo_ataque() == "Ataque Corpo a Corpo") {
    // Regra para alcance. Criaturas com alcance zero nao se beneficiam de armas de haste.
    // https://rpg.stackexchange.com/questions/47227/do-creatures-with-inappropriately-sized-reach-weapons-threaten-different-areas/47338#47338
    int alcance = AlcanceTamanhoQuadrados(proto.tamanho());
    int alcance_minimo = 0;
    if (arma.haste()) {
      alcance_minimo = alcance;
      alcance *= 2;
    }
    da->set_alcance_m(alcance * QUADRADOS_PARA_METROS);
    da->set_alcance_minimo_m(alcance_minimo * QUADRADOS_PARA_METROS);
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
    bba = PossuiTalento("acuidade_arma", proto) && bba_distancia > bba_cac ? bba_distancia : bba_cac;
  }

  {
    auto* bonus_ataque = da->mutable_bonus_ataque();
    auto* bonus_dano = da->mutable_bonus_dano();
    // Obra prima e bonus magico.
    AtribuiBonus(bba, TB_BASE, "base", bonus_ataque);
    if (da->bonus_magico()) {
      da->set_obra_prima(true);  // Toda arma magica eh obra prima.
      AtribuiBonus(da->bonus_magico(), TB_MELHORIA, "arma_magica", bonus_ataque);
      AtribuiBonus(da->bonus_magico(), TB_MELHORIA, "arma_magica", bonus_dano);
    } else {
      RemoveBonus(TB_MELHORIA, "arma_magica", bonus_ataque);
      RemoveBonus(TB_MELHORIA, "arma_magica", bonus_dano);
    }
    AtribuiOuRemoveBonus(da->obra_prima() ? 1 : 0, TB_MELHORIA, "obra_prima", bonus_ataque);
    // Talentos.
    AtribuiOuRemoveBonus(PossuiTalento("foco_em_arma", da->id_arma(), proto) ? 1 : 0, TB_SEM_NOME, "foco_em_arma", bonus_ataque);
    AtribuiOuRemoveBonus(PossuiTalento("foco_em_arma_maior", da->id_arma(), proto) ? 1 : 0, TB_SEM_NOME, "foco_em_arma_maior", bonus_ataque);
    // Duas maos ou armas naturais.
    switch (da->empunhadura()) {
      case EA_MAO_BOA: {
        // TODO detectar a arma da outra mao.
        const auto& arma_outra_mao = ArmaOutraMao(tabelas, *da, proto);
        int penalidade = PossuiCategoria(CAT_LEVE, arma_outra_mao) || PossuiCategoria(CAT_ARMA_DUPLA, arma) ? -4 : -6;
        if (PossuiTalento("combater_duas_armas", proto)) penalidade += 2;
        AtribuiBonus(penalidade, TB_SEM_NOME, "empunhadura", bonus_ataque);
        break;
      }
      case EA_MAO_RUIM: {
        int penalidade = PossuiCategoria(CAT_LEVE, arma) || PossuiCategoria(CAT_ARMA_DUPLA, arma) ? -8 : -10;
        if (PossuiTalento("combater_duas_armas", proto)) penalidade += 6;
        AtribuiBonus(penalidade, TB_SEM_NOME, "empunhadura", bonus_ataque);
        break;
      }
      case EA_MONSTRO_ATAQUE_SECUNDARIO: {
        int penalidade = PossuiTalento("ataques_multiplos", proto) ? -2 : -5;
        AtribuiBonus(penalidade, TB_SEM_NOME, "empunhadura", bonus_ataque);
        break;
      }
      default:
        RemoveBonus(TB_SEM_NOME, "empunhadura", bonus_ataque);
    }
    // Outros ataques.
    AtribuiOuRemoveBonus(-da->ordem_ataque() * 5, TB_SEM_NOME, "multiplos_ataque", bonus_ataque);
  }
  // Forca no dano.
  if (usar_forca_dano) {
    int modificador_forca_dano = arma.has_max_forca() ? std::min(modificador_forca, arma.max_forca()) : modificador_forca;
    int dano_forca = 0;
    EmpunhaduraArma ea = da->empunhadura();
    if (modificador_forca_dano < 0) {
      dano_forca = modificador_forca;
    } else if (ea == EA_2_MAOS) {
      dano_forca = floorf(modificador_forca_dano * 1.5f);
    } else if (ea == EA_MAO_RUIM || ea == EA_MONSTRO_ATAQUE_SECUNDARIO) {
      dano_forca = modificador_forca_dano / 2;
    } else {
      dano_forca = modificador_forca_dano;
    }
    AtribuiBonus(dano_forca, TB_ATRIBUTO, "forca", da->mutable_bonus_dano());
  } else {
    RemoveBonus(TB_ATRIBUTO, "forca", da->mutable_bonus_dano());
  }
  AtribuiOuRemoveBonus(
      PossuiTalento("especializacao_arma", da->id_arma(), proto) ? 2 : 0, TB_SEM_NOME, "especializacao_arma", da->mutable_bonus_dano());
  AtribuiOuRemoveBonus(
      PossuiTalento("especializacao_arma_maior", da->id_arma(), proto) ? 2 : 0, TB_SEM_NOME, "especializacao_arma_maior", da->mutable_bonus_dano());
  // Estes dois campos (bonus_ataque_final e dano) sao os mais importantes, porque sao os que vale.
  // So atualiza o BBA se houver algo para atualizar. Caso contrario deixa como esta.
  if (proto.has_bba() || !da->has_bonus_ataque_final()) da->set_bonus_ataque_final(CalculaBonusBaseParaAtaque(*da, proto));
  if (da->has_dano_basico() || !da->has_dano()) da->set_dano(CalculaDanoParaAtaque(*da, proto));
  if (da->grupo().empty()) da->set_grupo(google::protobuf::StringPrintf("%s|%s", da->tipo_ataque().c_str(), da->rotulo().c_str()));

  // CA do ataque.
  bool permite_escudo = da->empunhadura() == EA_ARMA_ESCUDO;
  da->set_ca_normal(CATotal(proto, permite_escudo));
  da->set_ca_surpreso(CASurpreso(proto, permite_escudo));
  da->set_ca_toque(CAToque(proto));
}

void RecomputaDependenciasDadosAtaque(const Tabelas& tabelas, EntidadeProto* proto) {
  // Remove ataques cujo numero de vezes exista e seja zero.
  std::vector<int> indices_a_remover;
  for (int i = proto->dados_ataque().size() - 1; i >= 0; --i) {
    const auto& da = proto->dados_ataque(i);
    if (da.has_limite_vezes() && da.limite_vezes() <= 0) {
      indices_a_remover.push_back(i);
    }
  }
  for (int indice : indices_a_remover) {
    proto->mutable_dados_ataque()->DeleteSubrange(indice, 1);
  }

  // Preenche os tipos de ataque automaticamente a partir do tipo_acao.
  // Remover isso quando nao existir mais tipo_ataque.
  // Apenas para as correspondencias 1x1.
  for (auto& da : *proto->mutable_dados_ataque()) {
    if (da.has_tipo_ataque()) continue;
    switch (da.tipo_acao()) {
      case ACAO_AGARRAR: da.set_tipo_ataque("Agarrar"); break;
      case ACAO_CORPO_A_CORPO: da.set_tipo_ataque("Ataque Corpo a Corpo"); break;
      case ACAO_PROJETIL: da.set_tipo_ataque("Ataque a Distância"); break;
      case ACAO_FEITICO_TOQUE: da.set_tipo_ataque("Feitiço de Toque"); break;
      case ACAO_PROJETIL_AREA: da.set_tipo_ataque("Projétil de Área"); break;
      case ACAO_RAIO: da.set_tipo_ataque("Raio"); break;
      default:
        ;
    }
  }

  // Se nao tiver agarrar, cria um.
  if (std::none_of(proto->dados_ataque().begin(), proto->dados_ataque().end(),
        [] (const EntidadeProto::DadosAtaque& da) { return da.tipo_ataque() == "Agarrar" ||
                                                           da.tipo_acao() == ACAO_AGARRAR; })) {
    auto* da = proto->mutable_dados_ataque()->Add();
    da->set_tipo_acao(ACAO_AGARRAR);
    da->set_tipo_ataque("Agarrar");
    da->set_rotulo("agarrar");
  }

  for (auto& da : *proto->mutable_dados_ataque()) {
    RecomputaDependenciasArma(tabelas, *proto, &da);
  }
}


// Aplica o bonus ou remove, se for 0. Bonus vazios sao ignorados.
void AplicaBonusOuRemove(const Bonus& bonus, Bonus* alvo) {
  for (const auto& bi : bonus.bonus_individual()) {
    for (const auto& po : bi.por_origem()) {
      if (po.valor() != 0) {
        AtribuiBonus(po.valor(), bi.tipo(), po.origem(), alvo);
      } else {
        RemoveBonus(bi.tipo(), po.origem(), alvo);
      }
    }
  }
}

// Aplica o efeito. Alguns especificos serao feitos aqui. Alguns efeitos sao aplicados apenas uma vez e usam processado como controle.
void AplicaEfeitoComum(const ConsequenciaEvento& consequencia, EntidadeProto* proto) {
  AplicaBonusOuRemove(consequencia.atributos().forca(), proto->mutable_atributos()->mutable_forca());
  AplicaBonusOuRemove(consequencia.atributos().destreza(), proto->mutable_atributos()->mutable_destreza());
  AplicaBonusOuRemove(consequencia.atributos().constituicao(), proto->mutable_atributos()->mutable_constituicao());
  AplicaBonusOuRemove(consequencia.atributos().inteligencia(), proto->mutable_atributos()->mutable_inteligencia());
  AplicaBonusOuRemove(consequencia.atributos().sabedoria(), proto->mutable_atributos()->mutable_sabedoria());
  AplicaBonusOuRemove(consequencia.atributos().carisma(), proto->mutable_atributos()->mutable_carisma());
  AplicaBonusOuRemove(consequencia.dados_defesa().ca(), proto->mutable_dados_defesa()->mutable_ca());
  AplicaBonusOuRemove(consequencia.dados_defesa().salvacao_fortitude(), proto->mutable_dados_defesa()->mutable_salvacao_fortitude());
  AplicaBonusOuRemove(consequencia.dados_defesa().salvacao_reflexo(), proto->mutable_dados_defesa()->mutable_salvacao_reflexo());
  AplicaBonusOuRemove(consequencia.dados_defesa().salvacao_vontade(), proto->mutable_dados_defesa()->mutable_salvacao_vontade());
  for (auto& da : *proto->mutable_dados_ataque()) {
    AplicaBonusOuRemove(consequencia.jogada_ataque(), da.mutable_bonus_ataque());
  }
  AplicaBonusOuRemove(consequencia.tamanho(), proto->mutable_bonus_tamanho());
}

// Retorna o dado de ataque que contem a arma, ou nullptr;
const EntidadeProto::DadosAtaque* DadosAtaque(const std::string& id_arma, const EntidadeProto& proto) {
  for (const auto& da : proto.dados_ataque()) {
    if (da.id_arma() == id_arma) return &da;
  }
  return nullptr;
}

// Retorna os dado de ataque com o mesmo rotulo.
std::vector<EntidadeProto::DadosAtaque*> DadosAtaquePorRotulo(const std::string& rotulo, EntidadeProto* proto) {
  std::vector<EntidadeProto::DadosAtaque*> das;
  for (auto& da : *proto->mutable_dados_ataque()) {
    if (da.rotulo() == rotulo) das.push_back(&da);
  }
  return das;
}

#if 0
bool PossuiArma(const std::string& id_arma, const EntidadeProto& proto) {
  return std::any_of(proto.dados_ataque().begin(), proto.dados_ataque().end(), [&id_arma] (
        const EntidadeProto::DadosAtaque& da) {
      return da.id_arma() == id_arma;
  });
}
#endif

// Poe e na primeira posicao de rf, movendo todos uma posicao para tras. Parametro 'e' fica invalido.
template <class T>
void InsereInicio(T* e, google::protobuf::RepeatedPtrField<T>* rf) {
  rf->Add()->Swap(e);
  for (int i = static_cast<int>(rf->size()) - 1; i > 0; --i) {
    rf->SwapElements(i, i-1);
  }
}

namespace {
DescritorAtaque StringParaDescritorAlinhamento(const std::string& alinhamento_str) {
  std::string normalizado = alinhamento_str;
  std::transform(alinhamento_str.begin(), alinhamento_str.end(), normalizado.begin(), ::tolower);
  if (normalizado == "bom" || normalizado == "bem") return DESC_BEM;
  if (normalizado == "mal" || normalizado == "mau") return DESC_MAL;
  if (normalizado == "caos") return DESC_CAOS;
  if (normalizado == "lei" || normalizado == "leal") return DESC_LEAL;
  return DESC_NENHUM;
}


DescritorAtaque StringParaDescritorElemento(const std::string& elemento_str) {
  std::string normalizado = elemento_str;
  std::transform(elemento_str.begin(), elemento_str.end(), normalizado.begin(), ::tolower);
  if (normalizado == "fogo") return DESC_FOGO;
  if (normalizado == "frio") return DESC_FRIO;
  if (normalizado == "acido") return DESC_ACIDO;
  if (normalizado == "sonico") return DESC_SONICO;
  if (normalizado == "eletricidade") return DESC_ELETRICIDADE;
  return DESC_NENHUM;

}

}  // namespace

void AplicaEfeito(const EntidadeProto::Evento& evento, const ConsequenciaEvento& consequencia, EntidadeProto* proto) {
  AplicaEfeitoComum(consequencia, proto);
  switch (evento.id_efeito()) {
    case EFEITO_VENENO:
      break;
    case EFEITO_INVISIBILIDADE:
      proto->set_visivel(false);
      break;
    case EFEITO_COMPETENCIA_PERICIA: {
      if (evento.complementos_str().empty()) return;
      // Encontra a pericia do efeito.
      auto* pericia_proto = PericiaCriando(evento.complementos_str(0), proto);
      Bonus bonus;
      auto* bi = bonus.add_bonus_individual();
      bi->set_tipo(TB_COMPETENCIA);
      auto* po = bi->add_por_origem();
      po->set_valor(evento.complementos(0));
      po->set_origem(google::protobuf::StringPrintf("competencia (id: %d)", evento.id_unico()));
      AplicaBonusOuRemove(bonus, pericia_proto->mutable_bonus());
    }
    break;
    case EFEITO_AJUDA:
      if (!evento.processado()) {
        // Gera os pontos de vida temporarios.
        int complemento = evento.complementos().empty() ? 3 : evento.complementos(0);
        if (complemento < 3) complemento = 3;
        else if (complemento > 10) complemento = 10;
        const int tmp = RolaDado(8) + complemento;
        auto* po = AtribuiBonus(tmp, TB_SEM_NOME, "ajuda", proto->mutable_pontos_vida_temporarios_por_fonte());
        if (evento.has_id_unico()) po->set_id_unico(evento.id_unico());
      }
    break;
    case EFEITO_PEDRA_ENCANTADA:
      if (!evento.processado()) {
        const auto& funda = DadosAtaque("funda", *proto);
        EntidadeProto::DadosAtaque da;
        da.set_id_unico_efeito(evento.id_unico());
        da.set_bonus_magico(1);
        da.set_dano_basico_fixo("1d6");
        if (funda != nullptr) {
          da.set_id_arma("funda");
          da.set_empunhadura(funda->empunhadura());
        } else {
          da.set_empunhadura(EA_ARMA_ESCUDO);
        }
        da.set_rotulo(funda != nullptr ? "pedra encantada com funda" : "pedra encantada");
        da.set_tipo_ataque("Ataque a Distância");
        da.set_tipo_acao(ACAO_PROJETIL);
        da.set_municao(3);
        InsereInicio(&da, proto->mutable_dados_ataque());
      }
    break;
    case EFEITO_ABENCOAR_ARMA: {
      if (evento.complementos_str().empty()) return; 
      std::vector<EntidadeProto::DadosAtaque*> das = DadosAtaquePorRotulo(evento.complementos_str(0), proto);
      for (auto* da : das) {
        da->set_alinhamento(DESC_BEM);
      }
    }
    break;
    case EFEITO_ALINHAR_ARMA: {
      if (evento.complementos_str().size() != 2) return;
      DescritorAtaque desc = StringParaDescritorAlinhamento(evento.complementos_str(1));
      if (desc == DESC_NENHUM) return;
      std::vector<EntidadeProto::DadosAtaque*> das = DadosAtaquePorRotulo(evento.complementos_str(0), proto);
      for (auto* da : das) {
        da->set_alinhamento(desc);
      }
    }
    break;
    case EFEITO_SUPORTAR_ELEMENTOS: {
      if (evento.complementos_str().empty()) return;
      DescritorAtaque descritor = StringParaDescritorElemento(evento.complementos_str(0));
      if (descritor == DESC_NENHUM) return; 
      ResistenciaElementos re;
      re.set_valor(10);
      re.set_descritor(descritor);
      re.set_id_unico(evento.id_unico());
      auto* re_corrente = AchaResistenciaElemento(evento.id_unico(), proto);
      if (re_corrente == nullptr) {
        re_corrente = proto->mutable_dados_defesa()->add_resistencia_elementos();
      }
      re_corrente->Swap(&re);
    }
    break;
    case EFEITO_RESISTENCIA_ELEMENTOS: {      
      if (evento.complementos_str().size() != 2) return;
      DescritorAtaque descritor = StringParaDescritorElemento(evento.complementos_str(0));
      if (descritor == DESC_NENHUM) return;
      int valor = atoi(evento.complementos_str(1).c_str());
      if (valor <= 0 || valor > 1000) return;
      ResistenciaElementos re;
      re.set_valor(valor);
      re.set_descritor(descritor);
      re.set_id_unico(evento.id_unico());
      auto* re_corrente = AchaResistenciaElemento(evento.id_unico(), proto);
      if (re_corrente == nullptr) {
        re_corrente = proto->mutable_dados_defesa()->add_resistencia_elementos();
      }
      re_corrente->Swap(&re);
    }
    break;
    case EFEITO_SONO: {      
      proto->set_caida(true);
    }
    break;
    default: ;
  }
}

void AplicaFimFuriaBarbaro(EntidadeProto* proto) {
  if (Nivel("barbaro", *proto) >= 17) return;
  auto* evento = proto->add_evento();
  evento->set_id_efeito(EFEITO_FADIGA);
  evento->set_descricao("fadiga_furia_barbaro");
  // Dura pelo resto do encontro.
  evento->set_rodadas(100);
}

void AplicaFimPedraEncantada(unsigned int id_unico, EntidadeProto* proto) {
  // Encontra o dado de ataque.
  int i = 0;
  for (const auto& da : proto->dados_ataque()) {
    if (da.id_unico_efeito() == id_unico) {
      proto->mutable_dados_ataque()->DeleteSubrange(i, 1);
      return;
    }
  }
}

void AplicaFimAlinhamentoArma(const std::string& rotulo, EntidadeProto* proto) {
  // Encontra o dado de ataque.
  for (auto& da : *proto->mutable_dados_ataque()) {
    if (da.rotulo() == rotulo) {
      da.clear_alinhamento();
    }
  }
}


void AplicaFimEfeito(const EntidadeProto::Evento& evento, const ConsequenciaEvento& consequencia, EntidadeProto* proto) {
  AplicaEfeitoComum(consequencia, proto);
  switch (evento.id_efeito()) {
    case EFEITO_VENENO:
    break;
    case EFEITO_INVISIBILIDADE:
      proto->set_visivel(true);
    break;
    case EFEITO_COMPETENCIA_PERICIA: {
      if (evento.complementos_str().empty()) return;
      // Encontra a pericia do efeito.
      auto* pericia_proto = PericiaCriando(evento.complementos_str(0), proto);
      Bonus bonus;
      auto* bi = bonus.add_bonus_individual();
      bi->set_tipo(TB_COMPETENCIA);
      auto* po = bi->add_por_origem();
      po->set_valor(0);
      po->set_origem(google::protobuf::StringPrintf("competencia (id: %d)", evento.id_unico()));
      AplicaBonusOuRemove(bonus, pericia_proto->mutable_bonus());
    }
    break;
    case EFEITO_AJUDA: {
      auto* bi = BonusIndividualSePresente(TB_SEM_NOME, proto->mutable_pontos_vida_temporarios_por_fonte());
      auto* po = OrigemSePresente("ajuda", bi);
      if (po == nullptr) {
        break;
      }
      if (evento.has_id_unico()) {
        // Se tiver id unico, respeita o id.
        RemoveSe<BonusIndividual::PorOrigem>([&evento] (const BonusIndividual::PorOrigem& ipo) {
          return ipo.id_unico() == evento.id_unico();
        }, bi->mutable_por_origem());
      } else {
        // Nao tem id, remove a ajuda por completo. Pode dar merda.
        LOG(WARNING) << "Removendo ajuda sem id unico.";
        RemoveBonus(TB_SEM_NOME, "ajuda", proto->mutable_pontos_vida_temporarios_por_fonte());
      }
    }
    break;
    case EFEITO_FURIA_BARBARO:
      AplicaFimFuriaBarbaro(proto);
    break;
    case EFEITO_PEDRA_ENCANTADA:
      AplicaFimPedraEncantada(evento.id_unico(), proto);
    break;
    case EFEITO_ABENCOAR_ARMA:
    case EFEITO_ALINHAR_ARMA: {
      if (evento.complementos_str().size() >= 1) {
        AplicaFimAlinhamentoArma(evento.complementos_str(0), proto);
      }
    }
    break;
    case EFEITO_SUPORTAR_ELEMENTOS:
    case EFEITO_RESISTENCIA_ELEMENTOS: {
      LimpaResistenciaElemento(evento.id_unico(), proto);
    }
    break;
    default: ;
  }
}

// Adiciona o id unico a cada origem de bonus.
// Preenche os bonus de acordo com o complemento se houver.
void PreencheOrigemValor(
    int id_unico, const google::protobuf::RepeatedField<int>& complementos, Bonus* bonus) {
  for (auto& bi : *bonus->mutable_bonus_individual()) {
    for (auto& po : *bi.mutable_por_origem()) {
      po.set_origem(google::protobuf::StringPrintf("%s, id: %d", po.origem().c_str(), id_unico));
      if (po.has_indice_complemento() && po.indice_complemento() >= 0 && po.indice_complemento() < complementos.size()) {
        po.set_valor(complementos.Get(po.indice_complemento()));
      }
    }
  }
}

void PreencheOrigemZeraValor(int id_unico, Bonus* bonus) {
  for (auto& bi : *bonus->mutable_bonus_individual()) {
    for (auto& po : *bi.mutable_por_origem()) {
      po.set_origem(google::protobuf::StringPrintf("%s, id: %d", po.origem().c_str(), id_unico));
      po.set_valor(0);
    }
  }
}


// Caso a consequencia use complemento, preenchera os valores existentes com ela.
ConsequenciaEvento PreencheConsequencia(
    int id_unico,
    const google::protobuf::RepeatedField<int>& complementos, const ConsequenciaEvento& consequencia_original) {
  ConsequenciaEvento c(consequencia_original);
  if (c.atributos().has_forca())        PreencheOrigemValor(id_unico, complementos, c.mutable_atributos()->mutable_forca());
  if (c.atributos().has_destreza())     PreencheOrigemValor(id_unico, complementos, c.mutable_atributos()->mutable_destreza());
  if (c.atributos().has_constituicao()) PreencheOrigemValor(id_unico, complementos, c.mutable_atributos()->mutable_constituicao());
  if (c.atributos().has_inteligencia()) PreencheOrigemValor(id_unico, complementos, c.mutable_atributos()->mutable_inteligencia());
  if (c.atributos().has_sabedoria())    PreencheOrigemValor(id_unico, complementos, c.mutable_atributos()->mutable_sabedoria());
  if (c.atributos().has_carisma())      PreencheOrigemValor(id_unico, complementos, c.mutable_atributos()->mutable_carisma());
  if (c.dados_defesa().has_ca())        PreencheOrigemValor(id_unico, complementos, c.mutable_dados_defesa()->mutable_ca());
  if (c.dados_defesa().has_salvacao_fortitude()) PreencheOrigemValor(id_unico, complementos, c.mutable_dados_defesa()->mutable_salvacao_fortitude());
  if (c.dados_defesa().has_salvacao_vontade())   PreencheOrigemValor(id_unico, complementos, c.mutable_dados_defesa()->mutable_salvacao_vontade());
  if (c.dados_defesa().has_salvacao_reflexo())   PreencheOrigemValor(id_unico, complementos, c.mutable_dados_defesa()->mutable_salvacao_reflexo());
  if (c.has_jogada_ataque())            PreencheOrigemValor(id_unico, complementos, c.mutable_jogada_ataque());
  if (c.has_tamanho())                  PreencheOrigemValor(id_unico, complementos, c.mutable_tamanho());
  return c;
}

// Caso a consequencia use complemento, preenchera os valores existentes com ela.
ConsequenciaEvento PreencheConsequenciaFim(int id_unico, const ConsequenciaEvento& consequencia_original) {
  ConsequenciaEvento c(consequencia_original);
  if (c.atributos().has_forca())        PreencheOrigemZeraValor(id_unico, c.mutable_atributos()->mutable_forca());
  if (c.atributos().has_destreza())     PreencheOrigemZeraValor(id_unico, c.mutable_atributos()->mutable_destreza());
  if (c.atributos().has_constituicao()) PreencheOrigemZeraValor(id_unico, c.mutable_atributos()->mutable_constituicao());
  if (c.atributos().has_inteligencia()) PreencheOrigemZeraValor(id_unico, c.mutable_atributos()->mutable_inteligencia());
  if (c.atributos().has_sabedoria())    PreencheOrigemZeraValor(id_unico, c.mutable_atributos()->mutable_sabedoria());
  if (c.atributos().has_carisma())      PreencheOrigemZeraValor(id_unico, c.mutable_atributos()->mutable_carisma());
  if (c.dados_defesa().has_ca())        PreencheOrigemZeraValor(id_unico, c.mutable_dados_defesa()->mutable_ca());
  if (c.dados_defesa().has_salvacao_fortitude()) PreencheOrigemZeraValor(id_unico, c.mutable_dados_defesa()->mutable_salvacao_fortitude());
  if (c.dados_defesa().has_salvacao_vontade())   PreencheOrigemZeraValor(id_unico, c.mutable_dados_defesa()->mutable_salvacao_vontade());
  if (c.dados_defesa().has_salvacao_reflexo())   PreencheOrigemZeraValor(id_unico, c.mutable_dados_defesa()->mutable_salvacao_reflexo());
  if (c.has_jogada_ataque())            PreencheOrigemZeraValor(id_unico, c.mutable_jogada_ataque());
  if (c.has_tamanho())                  PreencheOrigemZeraValor(id_unico, c.mutable_tamanho());
  return c;
}

void RecomputaAlteracaoConstituicao(int total_antes, int total_depois, EntidadeProto* proto) {
  if (total_antes == total_depois) return;
  VLOG(1) << "Recomputando alteracao de constituicao";
  VLOG(1) << "max original: " << proto->max_pontos_vida();
  VLOG(1) << "pv original: " << proto->pontos_vida();
  if (total_depois > total_antes) {
    // Incremento de constituicao.
    const int delta_con = total_depois - total_antes;
    const int delta_pv = Nivel(*proto) * (((total_antes % 2) == 1 && (delta_con % 2) == 1) ? (delta_con / 2) + 1 : delta_con / 2);
    VLOG(1) << "aumentando CON de: " << total_antes << " para: " << total_depois << ", delta_pv: " << delta_pv;
    proto->set_max_pontos_vida(proto->max_pontos_vida() + delta_pv);
    proto->set_pontos_vida(proto->pontos_vida() + delta_pv);
    if (proto->pontos_vida() >= 0) {
      proto->set_morta(false);
    }
  } else if (total_antes > total_depois) {
    // Decremento de constituicao.
    const int delta_con = total_antes - total_depois;
    const int delta_pv = Nivel(*proto) * (((total_antes % 2) == 0 && (delta_con % 2) == 1) ? (delta_con / 2) + 1 : delta_con / 2);
    VLOG(1) << "diminuindo CON de: " << total_antes << " para: " << total_depois << ", delta_pv: " << delta_pv;
    proto->set_max_pontos_vida(proto->max_pontos_vida() - delta_pv);
    proto->set_pontos_vida(proto->pontos_vida() - delta_pv);
    if (proto->pontos_vida() < 0) {
      proto->set_caida(true);
      proto->set_morta(true);
    }
  }
  VLOG(1) << "max modificado: " << proto->max_pontos_vida();
  VLOG(1) << "pv modificado: " << proto->pontos_vida();
}

void RecomputaDependenciasEfeitos(const Tabelas& tabelas, EntidadeProto* proto) {
  std::set<int, std::greater<int>> eventos_a_remover;
  int i = 0;
  // Verifica eventos acabados.
  const int total_constituicao_antes = BonusTotal(proto->atributos().constituicao());
  for (auto& evento : *proto->mutable_evento()) {
    if (evento.rodadas() < 0) {
      const auto& efeito = tabelas.Efeito(evento.id_efeito());
      VLOG(1) << "removendo efeito: " << TipoEfeito_Name(efeito.id());
      if (efeito.has_consequencia_fim()) {
        AplicaFimEfeito(evento, PreencheConsequencia(evento.id_unico(), evento.complementos(), efeito.consequencia_fim()), proto);
      } else {
        AplicaFimEfeito(evento, PreencheConsequenciaFim(evento.id_unico(), efeito.consequencia()), proto);
      }
      eventos_a_remover.insert(i);
    }
    ++i;
  }
  for (int i : eventos_a_remover) {
    proto->mutable_evento()->DeleteSubrange(i, 1);
  }
  // Computa os eventos ainda ativos. Os que nao se acumulam sao ignorados.
  std::unordered_set<int> efeitos_computados;
  for (auto& evento : *proto->mutable_evento()) {
    const bool computado = efeitos_computados.find(evento.id_efeito()) != efeitos_computados.end();
    efeitos_computados.insert(evento.id_efeito());
    const auto& efeito = tabelas.Efeito(evento.id_efeito());
    if (computado && efeito.nao_cumulativo()) {
      VLOG(1) << "ignorando efeito: " << TipoEfeito_Name(efeito.id());
      continue;
    }
    VLOG(1) << "aplicando efeito: " << efeito.DebugString();
    AplicaEfeito(evento, PreencheConsequencia(evento.id_unico(), evento.complementos(), efeito.consequencia()), proto);
    evento.set_processado(true);
  }
  const int total_constituicao_depois = BonusTotal(proto->atributos().constituicao());
  RecomputaAlteracaoConstituicao(total_constituicao_antes, total_constituicao_depois, proto);
}

void RecomputaDependenciasDestrezaLegado(const Tabelas& tabelas, EntidadeProto* proto) {
  // Legado, apenas para limpar o que foi feito errado. O bonus maximo de destreza afeta apenas a CA.
  AtribuiBonus(0, TB_ARMADURA, "armadura_escudo", proto->mutable_atributos()->mutable_destreza());
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

int ReducaoDanoBarbaro(int nivel) {
  if (nivel < 6) return 0;
  else if (nivel < 10) return 1;
  else if (nivel < 13) return 2;
  else if (nivel < 16) return 3;
  else if (nivel < 19) return 4;
  return 5;
}

// Recomputa os modificadores de conjuracao.
void RecomputaDependenciasClasses(const Tabelas& tabelas, EntidadeProto* proto) {
  int salvacao_fortitude = 0;
  int salvacao_reflexo = 0;
  int salvacao_vontade = 0;
  // Para evitar recomputar quando nao tiver base.
  bool recomputa_base = false;
  proto->mutable_dados_defesa()->clear_reducao_dano_barbaro();
  for (auto& ic : *proto->mutable_info_classes()) {
    {
      const auto& classe_tabelada = tabelas.Classe(ic.id());
      if (classe_tabelada.has_nome()) {
        ic.clear_salvacoes_fortes();
        ic.clear_habilidades_por_nivel();
        ic.clear_pericias();
        ic.clear_progressao_feitico();
        ic.MergeFrom(classe_tabelada);
      }
    }
    if (ic.has_atributo_conjuracao() || ic.has_id_para_progressao_de_magia()) {
      const auto& classe_tabelada_conjuracao =
          tabelas.Classe(ic.has_id_para_progressao_de_magia() ? ic.id_para_progressao_de_magia() : ic.id());
      ic.set_atributo_conjuracao(classe_tabelada_conjuracao.atributo_conjuracao());
      ic.set_modificador_atributo_conjuracao(ModificadorAtributo(ic.atributo_conjuracao(), *proto));
      int nc = 0;
      ProgressaoConjurador pconj =
          ic.has_progressao_conjurador() ? ic.progressao_conjurador() : classe_tabelada_conjuracao.progressao_conjurador();
      switch (pconj) {
        case PCONJ_UM:
          nc = ic.nivel(); break;
        case PCONJ_MEIO_MIN_4:
          nc = ic.nivel() < 4 ? 0 : ic.nivel() / 2; break;
        default:
          nc = ic.nivel_conjurador();
      }
      ic.set_nivel_conjurador(nc);
    }
    if (ic.has_progressao_bba()) {
      switch (ic.progressao_bba()) {
        case PBBA_ZERO: ic.set_bba(0); break;
        case PBBA_MEIO: ic.set_bba(ic.nivel() / 2); break;
        case PBBA_TRES_QUARTOS: ic.set_bba((ic.nivel() * 3) / 4); break;
        case PBBA_UM: ic.set_bba(ic.nivel()); break;
      }
    }

    if (ic.salvacoes_fortes_size() > 0) {
      recomputa_base = true;
      salvacao_fortitude += CalculaBaseSalvacao(ClassePossuiSalvacaoForte(TS_FORTITUDE, ic), ic.nivel());
      salvacao_reflexo += CalculaBaseSalvacao(ClassePossuiSalvacaoForte(TS_REFLEXO, ic), ic.nivel());
      salvacao_vontade += CalculaBaseSalvacao(ClassePossuiSalvacaoForte(TS_VONTADE, ic), ic.nivel());
    }
    if (ic.id() == "barbaro") {
      proto->mutable_dados_defesa()->set_reducao_dano_barbaro(ReducaoDanoBarbaro(ic.nivel()));
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
  int bonus_maximo = std::numeric_limits<int>::max();
  if (dd->has_id_armadura()) {
    bonus_maximo = std::min(tabelas.Armadura(dd->id_armadura()).max_bonus_destreza(), bonus_maximo);
  }
  if (dd->has_id_escudo()) {
    bonus_maximo = std::min(tabelas.Escudo(dd->id_escudo()).max_bonus_destreza(), bonus_maximo);
  }
  const int modificador_destreza = std::min(ModificadorAtributo(proto_retornado->atributos().destreza()), bonus_maximo);
  AtribuiBonus(modificador_destreza, ent::TB_ATRIBUTO, "destreza", dd->mutable_ca());
  const int modificador_tamanho = ModificadorTamanho(proto_retornado->tamanho());
  ent::AtribuiBonus(10, ent::TB_BASE, "base",  dd->mutable_ca());
  AtribuiOuRemoveBonus(modificador_tamanho, ent::TB_TAMANHO, "tamanho", dd->mutable_ca());
  AtribuiOuRemoveBonus(dd->has_id_armadura() ? tabelas.Armadura(dd->id_armadura()).bonus() : 0, ent::TB_ARMADURA, "armadura", dd->mutable_ca());
  AtribuiOuRemoveBonus(dd->has_bonus_magico_armadura()
      ? dd->bonus_magico_armadura() : 0, ent::TB_ARMADURA_MELHORIA, "armadura_melhoria", dd->mutable_ca());
  AtribuiOuRemoveBonus(dd->has_id_escudo() ? tabelas.Escudo(dd->id_escudo()).bonus() : 0, ent::TB_ESCUDO, "escudo", dd->mutable_ca());
  AtribuiOuRemoveBonus(dd->has_bonus_magico_escudo()
      ? dd->bonus_magico_escudo() : 0, ent::TB_ESCUDO_MELHORIA, "escudo_melhoria", dd->mutable_ca());
}

void RecomputaDependenciasSalvacoes(
    int modificador_constituicao, int modificador_destreza, int modificador_sabedoria, const ent::Tabelas& tabelas, EntidadeProto* proto_retornado) {
  auto* dd = proto_retornado->mutable_dados_defesa();

  // Testes de resistencia.
  AtribuiBonus(modificador_constituicao, ent::TB_ATRIBUTO, "constituicao", dd->mutable_salvacao_fortitude());
  AtribuiBonus(modificador_destreza, ent::TB_ATRIBUTO, "destreza", dd->mutable_salvacao_reflexo());
  AtribuiBonus(modificador_sabedoria, ent::TB_ATRIBUTO, "sabedoria", dd->mutable_salvacao_vontade());

  // Talentos: tem que ser dessa forma (manual por talento), porque se nao ao retirar um talento do personagem tem que descobrir o que
  // foi tirado para remover os bonus. E isso eh bem mais complicado.
  // OBS: nao sao cumulativos.
  AtribuiOuRemoveBonus(PossuiTalento("fortitude_maior", *proto_retornado) ? 2 : 0, TB_SEM_NOME, "fortitude_maior", dd->mutable_salvacao_fortitude());
  AtribuiOuRemoveBonus(PossuiTalento("reflexos_rapidos", *proto_retornado) ? 2 : 0, TB_SEM_NOME, "reflexos_rapidos", dd->mutable_salvacao_reflexo());
  AtribuiOuRemoveBonus(PossuiTalento("vontade_ferro", *proto_retornado) ? 2 : 0, TB_SEM_NOME, "vontade_ferro", dd->mutable_salvacao_vontade());

  const int mod_nivel_negativo = -proto_retornado->niveis_negativos();
  AtribuiBonus(mod_nivel_negativo, ent::TB_SEM_NOME, "niveis_negativos", dd->mutable_salvacao_fortitude());
  AtribuiBonus(mod_nivel_negativo, ent::TB_SEM_NOME, "niveis_negativos", dd->mutable_salvacao_reflexo());
  AtribuiBonus(mod_nivel_negativo, ent::TB_SEM_NOME, "niveis_negativos", dd->mutable_salvacao_vontade());
}

void RecomputaDependenciaTamanho(EntidadeProto* proto) {
  // Aplica efeito cria isso, entao melhor ver se tem algum bonus individual.
  if (!PossuiBonus(TB_BASE, proto->bonus_tamanho())) {
    AtribuiBonus(proto->tamanho(), TB_BASE, "base", proto->mutable_bonus_tamanho());
  }
  int total = BonusTotal(proto->bonus_tamanho());
  if (total > TM_COLOSSAL) { total = TM_COLOSSAL; }
  else if (total < TM_MINUSCULO) { total = TM_MINUSCULO; }
  proto->set_tamanho(TamanhoEntidade(total));
}

void RecomputaDependenciasIniciativa(int modificador_destreza, EntidadeProto* proto) {
  AtribuiBonus(modificador_destreza, ent::TB_ATRIBUTO, "destreza", proto->mutable_bonus_iniciativa());
  AtribuiOuRemoveBonus(
      PossuiTalento("iniciativa_aprimorada", *proto) ? 4 : 0, TB_TALENTO, "talento", proto->mutable_bonus_iniciativa());
  proto->set_modificador_iniciativa(BonusTotal(proto->bonus_iniciativa()));
}

void RecomputaDependenciasTendencia(EntidadeProto* proto) {
  if (proto->tendencia().has_simples()) {
    float bem_mal = 0.5f;
    float ordem_caos = 0.5f;
    switch (proto->tendencia().simples()) {
      case TD_LEAL_BOM:    bem_mal = ordem_caos = 1.0f;       break;
      case TD_LEAL_NEUTRO: bem_mal = 0.5f; ordem_caos = 1.0f; break;
      case TD_LEAL_MAU:    bem_mal = 0.0f; ordem_caos = 1.0f; break;
      case TD_NEUTRO_BOM:  bem_mal = 1.0f; ordem_caos = 0.5f; break;
      case TD_NEUTRO:      bem_mal = ordem_caos = 0.5f;       break;
      case TD_NEUTRO_MAU:  bem_mal = 0.0f; ordem_caos = 0.5f; break;
      case TD_CAOTICO_BOM:    bem_mal = 1.0f; ordem_caos = 0.0f; break;
      case TD_CAOTICO_NEUTRO: bem_mal = 0.5f; ordem_caos = 0.0f; break;
      case TD_CAOTICO_MAU:    bem_mal = 0.0f; ordem_caos = 0.0f; break;
    }
    proto->mutable_tendencia()->clear_simples();
    proto->mutable_tendencia()->set_eixo_bem_mal(bem_mal);
    proto->mutable_tendencia()->set_eixo_ordem_caos(ordem_caos);
  }
}

}  // namespace

bool PossuiCategoria(CategoriaArma categoria, const ArmaProto& arma) {
  return std::any_of(arma.categoria().begin(), arma.categoria().end(), [categoria] (int c) { return c == categoria; });
}

bool ClassePossuiSalvacaoForte(TipoSalvacao ts, const InfoClasse& ic) {
  return std::any_of(ic.salvacoes_fortes().begin(), ic.salvacoes_fortes().end(), [ts] (int icts) { return icts == ts; });
}

void RecomputaDependenciasPontosVidaTemporarios(EntidadeProto* proto) {
  auto* bpv = proto->mutable_pontos_vida_temporarios_por_fonte();
  // Remove todos que nao sao sem nome.
  RemoveSe<BonusIndividual>([] (const BonusIndividual& bi) {
    return bi.tipo() != TB_SEM_NOME;
  }, bpv->mutable_bonus_individual());
  if (!bpv->bonus_individual().empty()) {
    // Mantem apenas o maximo de cada um.
    auto* bi = BonusIndividualSePresente(TB_SEM_NOME, bpv);
    if (bi != nullptr) {
      struct Info {
        int max = -1;
        const BonusIndividual::PorOrigem* ptr = nullptr;
      };
      std::unordered_map<std::string, Info> infos;
      for (int i = 0; i < bi->por_origem().size(); ++i) {
        const auto& po = bi->por_origem(i);
        auto& info = infos[po.origem()];
        if (info.ptr == nullptr || po.valor() > info.max) {
          info.max = po.valor();
          info.ptr = &po;
        }
      }
      // Remove os que nao sao maximo.
      RemoveSe<BonusIndividual::PorOrigem>([&infos](const BonusIndividual::PorOrigem& po) {
        return po.valor() == 0 || &po != infos[po.origem()].ptr;
      }, bi->mutable_por_origem());
    }
  }
  // So pode haver um de cada tipo. Todos tem TB_SEM_NOME, difere eh a origem.
  // Apenas o maior de cada origem eh mantido.
  proto->set_pontos_vida_temporarios(BonusTotal(*bpv));
}

void RecomputaDependenciasPontosVida(EntidadeProto* proto) {
  const int max_pontos_vida = proto->max_pontos_vida() - proto->niveis_negativos() * 5;
  if (proto->pontos_vida() > max_pontos_vida) {
    proto->set_pontos_vida(max_pontos_vida);
  }
}

// Retorna o bonus do talento para a pericia ou zero caso nao haja.
int BonusTalento(const std::string& id_pericia, const TalentoProto& talento) {
  for (const auto& bp : talento.bonus_pericias()) {
    if (bp.id() == id_pericia) return bp.valor();
  }
  return 0;
}

void RecomputaDependenciasPericias(const Tabelas& tabelas, EntidadeProto* proto) {
  // Pericias afetadas por talentos.
  std::unordered_map<std::string, std::vector<const TalentoProto*>> talentos_por_pericia;
  for (const auto& talento : tabelas.todas().tabela_talentos().talentos()) {
    for (const auto& bp : talento.bonus_pericias()) {
      talentos_por_pericia[bp.id()].push_back(&talento);
    }
  }

  // Mapa do proto do personagem, porque iremos iterar nas pericias existentes na tabela.
  std::unordered_map<std::string, InfoPericia*> mapa_pericias_proto;
  for (auto& ip : *proto->mutable_info_pericias()) {
    mapa_pericias_proto[ip.id()] = &ip;
  }

  // Cria todas as pericias do personagem.
  for (const auto& pt : tabelas.todas().tabela_pericias().pericias()) {
    // Acha a pericia no personagem se houver para pegar os pontos e calcular a graduacao.
    auto it = mapa_pericias_proto.find(pt.id());
    if (it == mapa_pericias_proto.end()) {
      auto* pericia_proto = proto->add_info_pericias();
      pericia_proto->set_id(pt.id());
      mapa_pericias_proto[pt.id()] = pericia_proto;
    } else {
      it->second->mutable_restricoes_sinergia()->Clear();
    }
  }

  // Iteracao.
  for (const auto& pt : tabelas.todas().tabela_pericias().pericias()) {
    // Graduacoes.
    auto* pericia_proto = mapa_pericias_proto[pt.id()];
    int graduacoes = PericiaDeClasse(tabelas, pt.id(), *proto) ? pericia_proto->pontos() : pericia_proto->pontos() / 2;
    AtribuiOuRemoveBonus(graduacoes, TB_BASE, "graduacao", pericia_proto->mutable_bonus());

    // Sinergia.
    for (const auto& s : pt.sinergias()) {
      auto* pericia_alvo = mapa_pericias_proto[s.id()];
      AtribuiOuRemoveBonus(graduacoes >= 5 ? 2 : 0, TB_SINERGIA, google::protobuf::StringPrintf("sinergia_%s", pt.id().c_str()), pericia_alvo->mutable_bonus());
      if (!s.restricao().empty()) {
        pericia_alvo->add_restricoes_sinergia(s.restricao());
      }
    }

    // Atributo.
    AtribuiOuRemoveBonus(ModificadorAtributo(pt.atributo(), *proto), TB_ATRIBUTO, "atributo", pericia_proto->mutable_bonus());

    // Talento.
    auto par_pericia_talentos = talentos_por_pericia.find(pt.id());
    if (par_pericia_talentos != talentos_por_pericia.end()) {
      for (const auto* talento : par_pericia_talentos->second) {
        const int bonus_talento = PossuiTalento(talento->id(), *proto) ? BonusTalento(pt.id(), *talento) : 0;
        AtribuiOuRemoveBonus(bonus_talento, TB_TALENTO, "talento", pericia_proto->mutable_bonus());
      }
    }
    //LOG(INFO) << "pericia_proto: " << pericia_proto->ShortDebugString();
  }
  // TODO sinergia e talentos.
}

void RecomputaDependenciasMagiasConhecidas(const Tabelas& tabelas, EntidadeProto* proto) {
  for (auto& ic : *proto->mutable_info_classes()) {
    if (!ic.has_progressao_conjurador() || ic.nivel() <= 0) continue;
    // Encontra a entrada da classe, ou cria se nao houver.
    auto* fc = FeiticosClasse(ic.id(), proto);
    // Le a progressao.
    const int nivel = std::min(ic.nivel(), 20);
    const auto& classe_tabelada = tabelas.Classe(ic.has_id_para_progressao_de_magia() ? ic.id_para_progressao_de_magia() : ic.id());
    // Esse caso deveria dar erro. O cara tem nivel acima do que esta na tabela.
    if (nivel >= classe_tabelada.progressao_feitico().para_nivel_size()) continue;
    const std::string& magias_conhecidas = classe_tabelada.progressao_feitico().para_nivel(nivel).conhecidos();
    // Classe nao tem magias conhecidas.
    if (magias_conhecidas.empty()) continue;
    // Inclui o nivel 0. Portanto, se o nivel maximo eh 2, deve haver 3 elementos.
    Redimensiona(magias_conhecidas.size(), fc->mutable_feiticos_por_nivel());

    for (int nivel_magia = 0; nivel_magia < magias_conhecidas.size(); ++nivel_magia) {
      const char magias_conhecidas_do_nivel = magias_conhecidas[nivel_magia] - '0';
      Redimensiona(magias_conhecidas_do_nivel, fc->mutable_feiticos_por_nivel(nivel_magia)->mutable_conhecidos());
    }
  }
}

namespace {

int FeiticosBonusPorAtributoPorNivel(int nivel, const Bonus& atributo) {
  int modificador_atributo = ModificadorAtributo(atributo);
  // Nunca ha bonus de nivel 0.
  if (nivel <= 0 || nivel > 9) return 0;
  if (modificador_atributo < nivel) return 0;
  return static_cast<int>(floor(((modificador_atributo - nivel) / 4) + 1));
}

}  // namespace

void RecomputaDependenciasMagiasPorDia(const Tabelas& tabelas, EntidadeProto* proto) {
  for (auto& ic : *proto->mutable_info_classes()) {
    if (!ic.has_progressao_conjurador() || ic.nivel() <= 0) continue;
    // Encontra a entrada da classe, ou cria se nao houver.
    auto* fc = FeiticosClasse(ic.id(), proto);
    // Le a progressao.
    const int nivel = std::min(ic.nivel(), 20);
    const auto& classe_tabelada = tabelas.Classe(ic.has_id_para_progressao_de_magia() ? ic.id_para_progressao_de_magia() : ic.id());
    // Esse caso deveria dar erro. O cara tem nivel acima do que esta na tabela.
    if (nivel >= classe_tabelada.progressao_feitico().para_nivel_size()) continue;
    const std::string& magias_por_dia = classe_tabelada.progressao_feitico().para_nivel(nivel).magias_por_dia();

    // Inclui o nivel 0. Portanto, se o nivel maximo eh 2, deve haver 3 elementos.
    Redimensiona(magias_por_dia.size(), fc->mutable_feiticos_por_nivel());

    for (int nivel_magia = 0; nivel_magia < magias_por_dia.size(); ++nivel_magia) {
      int magias_do_nivel =
        (magias_por_dia[nivel_magia] - '0') +
        FeiticosBonusPorAtributoPorNivel(
            nivel_magia,
            BonusAtributo(classe_tabelada.atributo_conjuracao(), *proto)) +
        (classe_tabelada.possui_dominio() && nivel_magia > 0 && nivel_magia <= 9 ? 1 : 0);
      Redimensiona(magias_do_nivel, fc->mutable_feiticos_por_nivel(nivel_magia)->mutable_para_lancar());
    }
  }
}

void RecomputaDependencias(const Tabelas& tabelas, EntidadeProto* proto) {
  VLOG(2) << "Proto antes RecomputaDependencias: " << proto->ShortDebugString();
  RecomputaDependenciasTendencia(proto);
  RecomputaDependenciasEfeitos(tabelas, proto);
  RecomputaDependenciasDestrezaLegado(tabelas, proto);
  RecomputaDependenciasClasses(tabelas, proto);
  RecomputaDependenciaTamanho(proto);
  RecomputaDependenciasPontosVidaTemporarios(proto);
  RecomputaDependenciasPontosVida(proto);

  int modificador_destreza           = ModificadorAtributo(proto->atributos().destreza());
  const int modificador_constituicao = ModificadorAtributo(proto->atributos().constituicao());
  //const int modificador_inteligencia = ModificadorAtributo(ent::BonusTotal(proto->atributos().inteligencia()));
  const int modificador_sabedoria    = ModificadorAtributo(proto->atributos().sabedoria());
  //const int modificador_carisma      = ModificadorAtributo(ent::BonusTotal(proto->atributos().carisma()));

  // Iniciativa.
  RecomputaDependenciasIniciativa(modificador_destreza, proto);

  // CA.
  RecomputaDependenciasCA(tabelas, proto);
  // Salvacoes.
  RecomputaDependenciasSalvacoes(modificador_constituicao, modificador_destreza, modificador_sabedoria, tabelas, proto);

  // BBA: tenta atualizar por classe, se nao houver, pelo bba base, senao nao faz nada.
  if (proto->info_classes_size() > 0 ||  proto->bba().has_base()) {
    const int modificador_forca = ModificadorAtributo(proto->atributos().forca());
    const int modificador_tamanho = ModificadorTamanho(proto->tamanho());
    const int bba = proto->info_classes_size() > 0 ? CalculaBonusBaseAtaque(*proto) : proto->bba().base();
    const int niveis_negativos = proto->niveis_negativos();
    proto->mutable_bba()->set_base(bba);
    proto->mutable_bba()->set_cac(modificador_forca + modificador_tamanho + bba - niveis_negativos);
    proto->mutable_bba()->set_distancia(modificador_destreza + modificador_tamanho + bba - niveis_negativos);
    int total_agarrar = modificador_forca + ModificadorTamanhoAgarrar(proto->tamanho()) + bba - niveis_negativos;
    if (PossuiTalento("agarrar_aprimorado", *proto)) {
      total_agarrar += 4;
    }
    proto->mutable_bba()->set_agarrar(total_agarrar);
  }

  // Atualiza os bonus de ataques.
  RecomputaDependenciasDadosAtaque(tabelas, proto);

  RecomputaDependenciasPericias(tabelas, proto);

  RecomputaDependenciasMagiasConhecidas(tabelas, proto);
  RecomputaDependenciasMagiasPorDia(tabelas, proto);

  VLOG(2) << "Proto depois RecomputaDependencias: " << proto->ShortDebugString();
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
BonusIndividual::PorOrigem* AtribuiBonusIndividual(int valor, const std::string& origem, BonusIndividual* bonus_individual) {
  for (auto& por_origem : *bonus_individual->mutable_por_origem()) {
    if (por_origem.origem() == origem) {
      por_origem.set_valor(valor);
      return &por_origem;
    }
  }
  auto* po = bonus_individual->add_por_origem();
  po->set_valor(valor);
  po->set_origem(origem);
  return po;
}

void AtribuiBonusIndividualSeMaior(int valor, const std::string& origem, BonusIndividual* bonus_individual) {
  for (auto& por_origem : *bonus_individual->mutable_por_origem()) {
    if (por_origem.origem() == origem) {
      if (valor > por_origem.valor()) {
        por_origem.set_valor(valor);
      }
      return;
    }
  }
  auto* po = bonus_individual->add_por_origem();
  po->set_valor(valor);
  po->set_origem(origem);
}


// Atribui um tipo de bonus individual a bonus.
BonusIndividual::PorOrigem* AtribuiBonus(int valor, TipoBonus tipo, const std::string& origem, Bonus* bonus) {
  for (auto& bi : *bonus->mutable_bonus_individual()) {
    if (bi.tipo() == tipo) {
      return AtribuiBonusIndividual(valor, origem, &bi);
    }
  }
  auto* bi = bonus->add_bonus_individual();
  bi->set_tipo(tipo);
  return AtribuiBonusIndividual(valor, origem, bi);
}

void AtribuiBonusSeMaior(int valor, TipoBonus tipo, const std::string& origem, Bonus* bonus) {
  for (auto& bi : *bonus->mutable_bonus_individual()) {
    if (bi.tipo() == tipo) {
      AtribuiBonusIndividualSeMaior(valor, origem, &bi);
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

// Melhor fazer sobre o proto do personagem do que para tabela por causa de classes nao tabeladas.
// A consequencia eh que o personagem deve ter a classe.
int ModificadorAtributoConjuracao(const std::string& id_classe, const EntidadeProto& proto) {
  for (const auto& info_classe : proto.info_classes()) {
    if (info_classe.id() == id_classe) {
      return info_classe.modificador_atributo_conjuracao();
    }
  }
  return 0;
}

void AtribuiBaseAtributo(int valor, TipoAtributo ta, EntidadeProto* proto) {
  auto* bonus = BonusAtributo(ta, proto);
  AtribuiBonus(valor, TB_BASE, "base", bonus);
}

Bonus* BonusAtributo(TipoAtributo ta, EntidadeProto* proto) {
  switch (ta) {
    case TA_FORCA: return proto->mutable_atributos()->mutable_forca();
    case TA_DESTREZA: return proto->mutable_atributos()->mutable_destreza();
    case TA_CONSTITUICAO: return proto->mutable_atributos()->mutable_constituicao();
    case TA_INTELIGENCIA: return proto->mutable_atributos()->mutable_inteligencia();
    case TA_SABEDORIA: return proto->mutable_atributos()->mutable_sabedoria();
    case TA_CARISMA: return proto->mutable_atributos()->mutable_carisma();
    default:
      LOG(ERROR) << "Tipo atributo invalido: " << (int)ta;
  }
  return proto->mutable_atributos()->mutable_forca();
}

const Bonus& BonusAtributo(TipoAtributo ta, const EntidadeProto& proto) {
  switch (ta) {
    case TA_FORCA: return proto.atributos().forca();
    case TA_DESTREZA: return proto.atributos().destreza();
    case TA_CONSTITUICAO: return proto.atributos().constituicao();
    case TA_INTELIGENCIA: return proto.atributos().inteligencia();
    case TA_SABEDORIA: return proto.atributos().sabedoria();
    case TA_CARISMA: return proto.atributos().carisma();
    default:
      LOG(ERROR) << "Tipo atributo invalido: " << (int)ta;
  }
  return proto.atributos().forca();
}

int ModificadorAtributo(TipoAtributo ta, const EntidadeProto& proto) {
  return ModificadorAtributo(BonusAtributo(ta, proto));
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

std::string DanoDesarmadoPorTamanho(TamanhoEntidade tamanho) {
  switch (tamanho) {
    case TM_MIUDO: return "1";
    case TM_PEQUENO: return "1d2";
    case TM_MEDIO: return "1d3";
    case TM_GRANDE: return "1d4";
    case TM_ENORME: return "1d6";
    case TM_IMENSO: return "1d8";
    case TM_COLOSSAL: return "2d6";
    default: return "0";
  }
}

int AlcanceTamanhoQuadrados(TamanhoEntidade tamanho) {
  switch (tamanho) {
    case TM_MINUSCULO:
    case TM_DIMINUTO:
    case TM_MIUDO:
      return 0;
    case TM_PEQUENO:
    case TM_MEDIO:
      return 1;
    case TM_GRANDE:
      return 2;
    case TM_ENORME:
      return 3;
    case TM_IMENSO:
      return 4;
    case TM_COLOSSAL:
      return 5;
    default:
      return 1;
  }
}

bool PossuiEvento(TipoEfeito tipo, const EntidadeProto& entidade) {
  return std::any_of(entidade.evento().begin(), entidade.evento().end(), [tipo] (const EntidadeProto::Evento& evento) {
    return evento.id_efeito() == tipo;
  });
}

std::vector<const EntidadeProto::Evento*> EventosTipo(TipoEfeito tipo, const EntidadeProto& entidade) {
  std::vector<const EntidadeProto::Evento*> eventos;
  for (const auto& evento : entidade.evento()) {
    if (evento.id_efeito() == tipo) eventos.push_back(&evento);
  }
  return eventos;
}

namespace {
// Usado para comparar eventos sem id unico.
bool EventosIguaisIgnorandoDuracao(const EntidadeProto::Evento& lhs, const EntidadeProto::Evento& rhs) {
  if (lhs.id_efeito() != rhs.id_efeito() || lhs.descricao() != rhs.descricao() ||
      lhs.complementos_size() != rhs.complementos_size()) {
    return false;
  }
  for (unsigned int i = 0; i < lhs.complementos_size(); ++i) {
    if (lhs.complementos(i) != rhs.complementos(i)) return false;
  }
  return true;
}
}  // namespace

bool PossuiEventoEspecifico(const EntidadeProto& entidade, const EntidadeProto::Evento& evento) {
  return std::any_of(entidade.evento().begin(), entidade.evento().end(), [&evento] (const EntidadeProto::Evento& evento_entidade) {
    if (evento.has_id_unico()) {
      return evento.id_unico() == evento_entidade.id_unico();
    }
    return EventosIguaisIgnorandoDuracao(evento, evento_entidade);
  });
}

bool PossuiResistenciaEspecifica(const EntidadeProto& entidade, const ResistenciaElementos& resistencia) {
  return std::any_of(entidade.dados_defesa().resistencia_elementos().begin(), entidade.dados_defesa().resistencia_elementos().end(),
      [&resistencia] (const ResistenciaElementos& resistencia_entidade) {
      return resistencia.valor() == resistencia_entidade.valor() && resistencia.descritor() == resistencia_entidade.descritor();
    });
}

const std::string IdParaMagia(const Tabelas& tabelas, const std::string& id_classe) {
  const auto& classe_tabelada = tabelas.Classe(id_classe);
  return classe_tabelada.has_id_para_magia() ? classe_tabelada.id_para_magia() : id_classe;
}

// Retorna o nivel do feitico para determinada classe.
int NivelFeitico(const Tabelas& tabelas, const std::string& id_classe, const ArmaProto& arma) {
  const auto& id = IdParaMagia(tabelas, id_classe);
  for (const auto& ic : arma.info_classes()) {
    if (ic.id() == id) return ic.nivel();
  }
  return 0;
}

void ArmaParaDadosAtaque(const Tabelas& tabelas, const ArmaProto& arma, const EntidadeProto& proto, EntidadeProto::DadosAtaque* da) {
  const auto& acao_proto = tabelas.Acao(da->tipo_ataque());
  da->clear_acao();
  // Aplica acao da arma.
  if (arma.has_acao()) {
    *da->mutable_acao() = arma.acao();
  }
  // Aplica acao fixa.
  if (da->has_acao_fixa()) {
    da->mutable_acao()->MergeFrom(da->acao_fixa());
  }
  if (da->acao().has_dificuldade_salvacao_base() || da->acao().has_dificuldade_salvacao_por_nivel()) {
    // Essa parte eh tricky. Algumas coisas tem que ser a classe mesmo: tipo atributo (feiticeiro usa carisma).
    // Outras tem que ser a classe de feitico, por exemplo, nivel de coluna de chama para mago.
    // A chamada InfoClasseParaFeitico busca a classe do personagem (feiticeiro)
    // enquanto TipoAtaqueParaClasse busca a classe para feitico (mago).
    const auto& ic = InfoClasseParaFeitico(tabelas, da->tipo_ataque(), proto);
    const int base = da->acao().has_dificuldade_salvacao_base()
        ? da->acao().dificuldade_salvacao_base()
        : 10 + NivelFeitico(tabelas, TipoAtaqueParaClasse(tabelas, da->tipo_ataque()), arma);
    da->mutable_acao()->set_dificuldade_salvacao(base + ModificadorAtributoConjuracao(ic.id(), proto));
  }

  if (acao_proto.ignora_municao() || da->acao().ignora_municao()) {
    da->clear_municao();
  }
  da->set_tipo_ataque(acao_proto.id());
  da->set_tipo_acao(acao_proto.has_tipo() ? acao_proto.tipo() : da->acao().tipo());
  if (da->tipo_ataque().empty() && da->has_id_arma()) {
    da->set_tipo_ataque(PossuiCategoria(CAT_PROJETIL_AREA, arma)
        ? "Projétil de Área"
        : PossuiCategoria(CAT_DISTANCIA, arma) ? "Ataque a Distância" : "Ataque Corpo a Corpo");
    da->set_tipo_acao(PossuiCategoria(CAT_PROJETIL_AREA, arma)
        ? ACAO_PROJETIL_AREA
        : PossuiCategoria(CAT_DISTANCIA, arma) ? ACAO_PROJETIL : ACAO_CORPO_A_CORPO);
  }
  if (PossuiCategoria(CAT_PROJETIL_AREA, arma)) {
    da->set_ataque_toque(true);
  } else if (acao_proto.has_ataque_toque()) {
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
  int modificador = ModificadorAtaque(DaParaTipoAtaque(da), proto, EntidadeProto());
  std::string texto_modificador;
  if (modificador != 0) texto_modificador = google::protobuf::StringPrintf("%+d", modificador);

  std::string texto_furtivo;
  if (proto.furtivo() && !proto.dados_ataque_global().dano_furtivo().empty()) {
    texto_furtivo = google::protobuf::StringPrintf("+%s", proto.dados_ataque_global().dano_furtivo().c_str());
  }

  std::string critico = StringCritico(da);
  return google::protobuf::StringPrintf(
      "%s (%s)%s%s: %+d%s, %s%s%s, CA: %s",
      da.grupo().c_str(), da.rotulo().c_str(), da.descarregada() ? " [descarregado]" : "", da.ataque_toque() ? " (T)" : "",
      da.bonus_ataque_final(), texto_modificador.c_str(),
      StringDanoParaAcao(da, proto).c_str(), critico.c_str(), texto_furtivo.c_str(),
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

std::string StringDescritores(const google::protobuf::RepeatedField<int>& descritores) {
  if (descritores.empty()) return "";
  if (descritores.size() == 1) return google::protobuf::StringPrintf(" [%s] ", TextoDescritor(descritores.Get(0)));
  std::string ret;
  for (int descritor : descritores) {
    ret += TextoDescritor(descritores.Get(descritor));
    ret += ", ";
  }
  ret.resize(ret.size() - 2);
  return google::protobuf::StringPrintf(" [%s] ", ret.c_str());
}

std::string StringResumoArma(const Tabelas& tabelas, const ent::EntidadeProto::DadosAtaque& da) {
  // Monta a string.
  std::string string_rotulo = StringPrintf("%s (%s), ", da.grupo().c_str(), da.rotulo().c_str());

  std::string string_nome_arma = da.id_arma().empty()
      ? ""
      : StringPrintf("%s, ", tabelas.Arma(da.id_arma()).nome().c_str());
  char string_alcance[40] = { '\0' };
  if (da.has_alcance_m()) {
    char string_incrementos[40] = { '\0' };
    if (da.has_incrementos()) {
      snprintf(string_incrementos, 39, ", inc %d", da.incrementos());
    }
    snprintf(string_alcance, 39, "alcance: %0.0f q%s, ", da.alcance_m() * METROS_PARA_QUADRADOS, string_incrementos);
  }

  std::string texto_municao;
  if (da.has_municao()) texto_municao = google::protobuf::StringPrintf(", municao: %d", da.municao());
  std::string texto_descarregada;
  if (da.descarregada()) texto_descarregada = " [descarregada]"; 

  std::string texto_elementos;
  if (da.acao().has_elemento()) texto_elementos = StringPrintf(" [%s] ", TextoDescritor(da.acao().elemento()));
  
  std::string string_escudo = da.empunhadura() == ent::EA_ARMA_ESCUDO ? "(escudo)" : "";
  return StringPrintf(
      "id: %s%s%s, %sbonus: %d, dano: %s%s%s%s%s, ca%s: %d toque: %d surpresa%s: %d",
      string_rotulo.c_str(), string_nome_arma.c_str(), da.tipo_ataque().c_str(),
      string_alcance,
      da.bonus_ataque_final(),
      da.dano().c_str(), StringCritico(da).c_str(), texto_elementos.c_str(), texto_municao.c_str(), texto_descarregada.c_str(),
      string_escudo.c_str(), da.ca_normal(),
      da.ca_toque(),  
      string_escudo.c_str(), da.ca_surpreso());
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
  return google::protobuf::StringPrintf("%s%s", da.dano_basico().c_str(), critico.empty() ? "" : critico.c_str());
}

std::string StringEfeito(TipoEfeito efeito) {
  std::string ret = TipoEfeito_Name(efeito);
  if (ret.find("EFEITO_") == 0) ret = ret.substr(7);
  return ret;
}

//--------------------
// Formas Alternativas
//--------------------

// Gera um proto de forma alternativa a partir da forma alternativa. Apenas alguns campos sao usados.
EntidadeProto ProtoFormaAlternativa(const EntidadeProto& proto) {
  EntidadeProto ret;
  ret.set_rotulo(proto.rotulo());
  // Forma alternativa afeta apena forca destreza e constituicao.
  std::vector<TipoAtributo> av = { TA_FORCA, TA_DESTREZA, TA_CONSTITUICAO /*, TA_INTELIGENCIA, TA_SABEDORIA, TA_CARISMA */};
  for (TipoAtributo ta : av) {
    const auto& bonus = BonusAtributo(ta, proto);
    const int base = PossuiBonus(TB_BASE, bonus) ? BonusIndividualTotal(TB_BASE, bonus) : 10;
    AtribuiBonus(base, TB_BASE, "base", BonusAtributo(ta, &ret));
    const int nivel = PossuiBonus(TB_NIVEL, bonus) ? BonusIndividualTotal(TB_NIVEL, bonus) : 0;
    // Tem que por o valor zero para sobrescrever.
    AtribuiBonus(nivel, TB_NIVEL, "nivel", BonusAtributo(ta, &ret));
  }
  *ret.mutable_info_textura() = proto.info_textura();
  *ret.mutable_modelo_3d() = proto.modelo_3d();
  if (!proto.dados_ataque().empty()) {
    *ret.mutable_dados_ataque() = proto.dados_ataque();
  } else {
    // Cria uma entrada dummy.
    ret.add_dados_ataque()->set_tipo_ataque("");
  }
  // Tamanho.
  const int tam_base = BonusIndividualPorOrigem(TB_BASE, "base", proto.bonus_tamanho());
  AtribuiBonus(tam_base, TB_BASE, "base", ret.mutable_bonus_tamanho());
  // Visao.
  // A forma alternativa nao ganha qualidades especiais (visao, faro).
  // Talento.
  // idem.

  // CA.
  const int ca_natural = BonusIndividualPorOrigem(TB_ARMADURA_NATURAL, "racial", proto.dados_defesa().ca());
  AtribuiBonus(ca_natural, TB_ARMADURA_NATURAL, "racial", ret.mutable_dados_defesa()->mutable_ca());
  ret.mutable_dados_defesa()->set_id_armadura(proto.dados_defesa().id_armadura());
  ret.mutable_dados_defesa()->set_bonus_magico_armadura(proto.dados_defesa().bonus_magico_armadura());
  ret.mutable_dados_defesa()->set_id_escudo(proto.dados_defesa().id_escudo());
  ret.mutable_dados_defesa()->set_bonus_magico_escudo(proto.dados_defesa().bonus_magico_escudo());
  return ret;
}

void AdicionaFormaAlternativa(const EntidadeProto& proto_forma, EntidadeProto* proto) {
  if (proto->formas_alternativas().empty()) {
    *proto->add_formas_alternativas() = ProtoFormaAlternativa(*proto);
  }
  *proto->add_formas_alternativas() = ProtoFormaAlternativa(proto_forma);
}

void RemoveFormaAlternativa(int indice, EntidadeProto* proto) {
  // Deve ser maior que zero.
  if (indice <= 0 || indice >= proto->formas_alternativas_size()) return;
  proto->mutable_formas_alternativas()->DeleteSubrange(indice, 1);
  if (proto->formas_alternativas_size() <= 1) {
    proto->clear_formas_alternativas();
  }
}
// Fim Formas Alternativas

bool PossuiTalento(const std::string& chave_talento, const EntidadeProto& entidade) {
  return Talento(chave_talento, entidade) != nullptr;
}

bool PossuiTalento(const std::string& chave_talento, const std::string& complemento, const EntidadeProto& entidade) {
  return Talento(chave_talento, complemento, entidade) != nullptr;
}

const TalentoProto* Talento(const std::string& chave_talento, const EntidadeProto& entidade) {
  for (const auto& t : entidade.info_talentos().gerais()) {
    if (chave_talento == t.id()) return &t;
  }
  for (const auto& t : entidade.info_talentos().outros()) {
    if (chave_talento == t.id()) return &t;
  }
  return nullptr;
}

const TalentoProto* Talento(const std::string& chave_talento, const std::string& complemento, const EntidadeProto& entidade) {
  for (const auto& t : entidade.info_talentos().gerais()) {
    if (chave_talento == t.id() && complemento == t.complemento()) return &t;
  }
  for (const auto& t : entidade.info_talentos().outros()) {
    if (chave_talento == t.id() && complemento == t.complemento()) return &t;
  }
  return nullptr;
}

bool PossuiHabilidadeEspecial(const std::string& chave, const EntidadeProto& proto) {
  for (const auto& ic : proto.info_classes()) {
    for (const auto& he : ic.habilidades_por_nivel()) {
      if (chave == he.id() && ic.nivel() >= he.nivel()) return true;
    }
  }
  return false;
}

bool PericiaDeClasse(const Tabelas& tabelas, const std::string& chave_pericia, const EntidadeProto& proto) {
  for (const auto& ic : proto.info_classes()) {
    const auto& ct = tabelas.Classe(ic.id());
    if (std::any_of(ct.pericias().begin(), ct.pericias().end(),
          [&chave_pericia] (const std::string& id) { return id == chave_pericia;} )) {
      return true;
    }
    if (std::any_of(ic.pericias_monstro().begin(), ic.pericias_monstro().end(),
          [&chave_pericia] (const std::string& id) { return id == chave_pericia;} )) {
      return true;
    }
  }
  return false;
}

TipoEfeito StringParaEfeito(const std::string& s) {
  for (int i = TipoEfeito_MIN; i < TipoEfeito_MAX; ++i) {
    if (!TipoEfeito_IsValid(i)) continue;
    if (boost::iequals(s, TipoEfeito_Name((TipoEfeito)i).substr(strlen("EFEITO_")))) {
      return (TipoEfeito)i;
    }
  }
  return EFEITO_INVALIDO;
}

// Ta quebrado!!!!! Nao tem o id do efeito.
// Funcao hack do android.
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
    std::string complementos;
    size_t pos_par = descricao.find_last_of("(");
    if (pos_par != std::string::npos) {
      complementos = descricao.substr(pos_par + 1);
      descricao = descricao.substr(0, pos_par);
    }
    std::string rodadas(linha.substr(pos_dois_pontos + 1));
    EntidadeProto::Evento evento;
    evento.set_descricao(ent::trim(descricao));
    evento.set_rodadas(atoi(rodadas.c_str()));
    boost::char_separator<char> sep(" ");
    boost::tokenizer<boost::char_separator<char>> tokenizador(complementos, sep);
    for (const auto& token : tokenizador) {
      evento.add_complementos(atoi(token.c_str()));
    }
    auto id_efeito = StringParaEfeito(evento.descricao());
    if (id_efeito != EFEITO_INVALIDO) {
      evento.set_id_efeito(id_efeito);
    }
    ret.Add()->Swap(&evento);
  }
  return ret;
}

Bonus BonusContraTendenciaNaCA(const EntidadeProto& proto_ataque, const EntidadeProto& proto_defesa) {
  if ((Bom(proto_ataque) && PossuiEvento(EFEITO_PROTECAO_CONTRA_BEM, proto_defesa)) ||
      (Mal(proto_ataque) && PossuiEvento(EFEITO_PROTECAO_CONTRA_MAL, proto_defesa)) ||
      (Ordeiro(proto_ataque) && PossuiEvento(EFEITO_PROTECAO_CONTRA_ORDEM, proto_defesa)) ||
      (Caotico(proto_ataque) && PossuiEvento(EFEITO_PROTECAO_CONTRA_CAOS, proto_defesa))) {
    Bonus b;
    AtribuiBonus(2, TB_DEFLEXAO, "protecao_contra_tendencia", &b);
    return b;
  }
  return Bonus();
}

Bonus BonusContraTendenciaNaSalvacao(const EntidadeProto& proto_ataque, const EntidadeProto& proto_defesa) {
  if ((Bom(proto_ataque) && PossuiEvento(EFEITO_PROTECAO_CONTRA_BEM, proto_defesa)) ||
      (Mal(proto_ataque) && PossuiEvento(EFEITO_PROTECAO_CONTRA_MAL, proto_defesa)) ||
      (Ordeiro(proto_ataque) && PossuiEvento(EFEITO_PROTECAO_CONTRA_ORDEM, proto_defesa)) ||
      (Caotico(proto_ataque) && PossuiEvento(EFEITO_PROTECAO_CONTRA_CAOS, proto_defesa))) {
    Bonus b;
    AtribuiBonus(2, TB_RESISTENCIA, "protecao_contra_tendencia", &b);
    return b;
  }
  return Bonus();
}

int Nivel(const EntidadeProto& proto) {
  int total = 0;
  for (const auto& ic : proto.info_classes()) {
    total += ic.nivel();
  }
  return total;
}

int Nivel(const std::string& id, const EntidadeProto& proto) {
  for (const auto& ic : proto.info_classes()) {
    if (ic.id() == id) return ic.nivel();
  }
  return 0;
}

std::string TipoAtaqueParaClasse(const Tabelas& tabelas, const std::string& tipo_ataque) {
  for (const auto& acao : tabelas.TodasAcoes().acao()) {
    if (acao.id() == tipo_ataque) {
      return acao.classe_conjuracao();
    }
  }
  return "";
}

std::string ClasseParaTipoAtaqueFeitico(const Tabelas& tabelas, const std::string& id_classe) {
  for (const auto& acao : tabelas.TodasAcoes().acao()) {
    if (acao.has_classe_conjuracao() && id_classe == acao.classe_conjuracao()) {
      return acao.id();
    }
  }
  return "";
}

const InfoClasse& InfoClasseParaFeitico(
    const Tabelas& tabelas, const std::string& tipo_ataque, const EntidadeProto& proto) {
  const auto& id = TipoAtaqueParaClasse(tabelas, tipo_ataque);
  const InfoClasse* ret = &InfoClasse::default_instance();
  // Evita comparacao com ids vazios, ja que id_para_magia pode ser vazio tb.
  if (id.empty()) return *ret;
  int max = 0;
  for (const auto& ic : proto.info_classes()) {
    if ((ic.id() == id || ic.id_para_magia() == id) && ic.nivel() > max) {
      max = ic.nivel();
      ret = &ic;
    }
  }
  return *ret;
}

int NivelParaFeitico(const Tabelas& tabelas, const EntidadeProto::DadosAtaque& da, const EntidadeProto& proto) {
  return InfoClasseParaFeitico(tabelas, da.tipo_ataque(), proto).nivel();
}

void RenovaFeiticos(EntidadeProto* proto) {
  for (auto& fc : *proto->mutable_feiticos_classes()) {
    for (auto& fn : *fc.mutable_feiticos_por_nivel()) {
      for (auto& pl : *fn.mutable_para_lancar()) {
        pl.set_usado(false);
      }
    }
  }
}

bool EmDefesaTotal(const EntidadeProto& proto) {
  for (const auto& bi : proto.dados_defesa().ca().bonus_individual()) {
    if (bi.tipo() == TB_ESQUIVA) {
      for (const auto& po : bi.por_origem()) {
        if (po.origem() == "defesa_total" && po.valor() > 0) return true;
      }
      return false;
    }
  }
  return false;
}

bool LutandoDefensivamente(const EntidadeProto& proto) {
  for (const auto& bi : proto.dados_defesa().ca().bonus_individual()) {
    if (bi.tipo() == TB_ESQUIVA) {
      for (const auto& po : bi.por_origem()) {
        if (po.origem() == "luta_defensiva" && po.valor() > 0) return true;
      }
      return false;
    }
  }
  return false;
}

template <class T>
void RemoveSe(const std::function<bool(const T& t)>& predicado, google::protobuf::RepeatedPtrField<T>* c) {
  for (int i = c->size() - 1; i >= 0; --i) {
    if (predicado(c->Get(i))) c->DeleteSubrange(i, 1);
  }
}

template <class T>
void Redimensiona(int tam, google::protobuf::RepeatedPtrField<T>* c) {
  if (tam == c->size()) return;
  if (tam < c->size()) {
    c->DeleteSubrange(tam, c->size());
    return;
  }
  while (c->size() < tam) c->Add();
}

uint32_t AchaIdUnicoEvento(
    const google::protobuf::RepeatedPtrField<EntidadeProto::Evento>& eventos,
    const google::protobuf::RepeatedPtrField<EntidadeProto::Evento>& eventos_sendo_gerados) {
  uint32_t candidato = 0;
  bool existe = false;
  auto EhCandidato = [&candidato] (const EntidadeProto::Evento& evento) { return candidato == evento.id_unico(); };
  do {
    existe = std::any_of(eventos.begin(), eventos.end(), EhCandidato) ||
             std::any_of(eventos_sendo_gerados.begin(), eventos_sendo_gerados.end(), EhCandidato);
    if (!existe) break;
    ++candidato;
    if (candidato == 0) {
      // Se isso acontecer, acredito que a memoria vai explodir antes...
      LOG(WARNING) << "Cheguei ao limite de eventos";
      break;
    }
  } while (1);
  VLOG(1) << "Retornando id unico: " << candidato;
  return candidato;
}

EntidadeProto::Evento* AdicionaEvento(
    const google::protobuf::RepeatedPtrField<EntidadeProto::Evento>& eventos,
    TipoEfeito id_efeito, int rodadas, bool continuo, EntidadeProto* proto) {
  // Pega antes de criar o evento.
  uint32_t id_unico = AchaIdUnicoEvento(eventos, proto->evento());
  auto* e = proto->add_evento();
  e->set_id_efeito(id_efeito);
  e->set_rodadas(continuo ? 1 : rodadas);
  e->set_continuo(continuo);
  e->set_id_unico(id_unico);
  return e;
}

void ExpiraEventoItemMagico(uint32_t id_unico, EntidadeProto* proto) {
  for (auto& evento : *proto->mutable_evento()) {
    if (evento.id_unico() == id_unico) {
      evento.set_rodadas(-1);
      return;
    }
  }
}

EntidadeProto::Evento* AchaEvento(uint32_t id_unico, EntidadeProto* proto) {
  for (auto& evento : *proto->mutable_evento()) {
    if (evento.id_unico() == id_unico) {
      return &evento;
    }
  }
  return nullptr;
}

ResistenciaElementos* AchaResistenciaElemento(uint32_t id_unico, EntidadeProto* proto) {
  for (auto& re : *proto->mutable_dados_defesa()->mutable_resistencia_elementos()) {
    if (re.id_unico() == id_unico) {
      return &re;
    }
  }
  return nullptr;
}

void LimpaResistenciaElemento(uint32_t id_unico, EntidadeProto* proto) {
  int i = 0;
  for (auto& re : *proto->mutable_dados_defesa()->mutable_resistencia_elementos()) {
    if (re.id_unico() == id_unico) {
      proto->mutable_dados_defesa()->mutable_resistencia_elementos()->DeleteSubrange(i, 1);
      return;
    }
    ++i;
  }
}

std::vector<int> AdicionaEventoItemMagico(
    const google::protobuf::RepeatedPtrField<EntidadeProto::Evento>& eventos,
    const ItemMagicoProto& item, int indice, int rodadas, bool continuo, EntidadeProto* proto) {
  std::vector<int> ids_unicos;
  std::vector<TipoEfeito> efeitos;
  if (item.combinacao_efeitos() == COMB_EXCLUSIVO) {
    if (indice < 0 || indice >= item.tipo_efeito().size()) {
      LOG(ERROR) << "indice de efeito de item invalido para " << item.DebugString();
    } else {
      efeitos.push_back(item.tipo_efeito(indice));
    }
  } else {
    for (auto tipo_efeito : item.tipo_efeito()) {
      efeitos.push_back((TipoEfeito)tipo_efeito);
    }
  }

  for (auto tipo_efeito : efeitos) {
    auto* evento = AdicionaEvento(eventos, tipo_efeito, rodadas, continuo, proto);
    ids_unicos.push_back(evento->id_unico());
    if (!item.complementos().empty()) {
      *evento->mutable_complementos() = item.complementos();
    }
    if (!item.complementos_str().empty()) {
      *evento->mutable_complementos_str() = item.complementos_str();
    }
    evento->set_descricao(item.descricao().empty() ? item.nome() : item.descricao());
  }
  return ids_unicos;
}

std::vector<const TalentoProto*> TodosTalentos(const EntidadeProto& proto) {
  std::vector<const TalentoProto*> todos_talentos;
  for (const auto& t : proto.info_talentos().gerais()) {
    todos_talentos.push_back(&t);
  }
  for (const auto& t : proto.info_talentos().outros()) {
    todos_talentos.push_back(&t);
  }
  return todos_talentos;
}

InfoPericia* PericiaOuNullptr(const std::string& id, EntidadeProto* proto) {
  for (auto& pericia : *proto->mutable_info_pericias()) {
    if (pericia.id() == id) return &pericia;
  }
  return nullptr;
}

InfoPericia* PericiaCriando(const std::string& id, EntidadeProto* proto) {
  for (auto& pericia : *proto->mutable_info_pericias()) {
    if (pericia.id() == id) return &pericia;
  }
  auto* pericia = proto->add_info_pericias();
  pericia->set_id(id);
  return pericia;
}

const InfoPericia& Pericia(const std::string& id, const EntidadeProto& proto) {
  for (auto& pericia : proto.info_pericias()) {
    if (pericia.id() == id) return pericia;
  }
  return InfoPericia::default_instance();
}

bool AgarradoA(unsigned int id, const EntidadeProto& proto) {
  return std::any_of(proto.agarrado_a().begin(), proto.agarrado_a().end(), [id] (unsigned int tid) { return id == tid; });
}


// ---------
// Feiticos.
// ---------

const EntidadeProto::InfoFeiticosClasse& FeiticosClasse(
    const std::string& id_classe, const EntidadeProto& proto) {
  for (const auto& fc : proto.feiticos_classes()) {
    if (fc.id_classe() == id_classe) return fc;
  }
  return EntidadeProto::InfoFeiticosClasse::default_instance();
}

EntidadeProto::InfoFeiticosClasse* FeiticosClasse(const std::string& id_classe, EntidadeProto* proto) {
  for (auto& fc : *proto->mutable_feiticos_classes()) {
    if (fc.id_classe() == id_classe) return &fc;
  }
  auto* fc = proto->add_feiticos_classes();
  fc->set_id_classe(id_classe);
  return fc;
}

EntidadeProto::InfoFeiticosClasse* FeiticosClasseOuNullptr(const std::string& id_classe, EntidadeProto* proto) {
  for (auto& fc : *proto->mutable_feiticos_classes()) {
    if (fc.id_classe() == id_classe) return &fc;
  }
  return nullptr;
}

const EntidadeProto::FeiticosPorNivel& FeiticosNivel(
    const std::string& id_classe, int nivel, const EntidadeProto& proto) {
  nivel = std::min(nivel, 9);
  const auto& fc = FeiticosClasse(id_classe, proto);
  if (nivel < 0 || nivel >= fc.feiticos_por_nivel().size()) return EntidadeProto::FeiticosPorNivel::default_instance();
  return fc.feiticos_por_nivel(nivel);
}

EntidadeProto::FeiticosPorNivel* FeiticosNivel(const std::string& id_classe, int nivel, EntidadeProto* proto) {
  nivel = std::min(nivel, 9);
  auto* fc = FeiticosClasse(id_classe, proto);
  if (nivel < 0) return nullptr;
  while (nivel >= fc->feiticos_por_nivel().size()) {
    fc->add_feiticos_por_nivel();
  }
  return fc->mutable_feiticos_por_nivel(nivel);
}

EntidadeProto::FeiticosPorNivel* FeiticosNivelOuNullptr(
    const std::string& id_classe, int nivel, EntidadeProto* proto) {
  nivel = std::min(nivel, 9);
  if (nivel < 0) return nullptr;
  auto* fc = FeiticosClasseOuNullptr(id_classe, proto);
  if (fc == nullptr) return nullptr;
  if (nivel >= fc->feiticos_por_nivel().size()) {
    return nullptr;
  }
  return fc->mutable_feiticos_por_nivel(nivel);
}

bool ClasseDeveConhecerFeitico(const Tabelas& tabelas, const std::string& id_classe) {
  const auto& ic = tabelas.Classe(id_classe);
  if (ic.progressao_feitico().para_nivel().size() < 2) return false;
  return !ic.progressao_feitico().para_nivel(1).conhecidos().empty();
}

bool ClassePrecisaMemorizar(const Tabelas& tabelas, const std::string& id_classe) {
  return tabelas.Classe(id_classe).precisa_memorizar();
}

// Fim feiticos.

const ent::EntidadeProto::InfoFeiticosClasse& InfoClasseFeiticoAtiva(const EntidadeProto& proto) {
  std::string id_classe = proto.classe_feitico_ativa();
  if (id_classe.empty()) {
    int nivel = 0;
    for (const auto& ic : proto.info_classes()) {
      if (ic.nivel_conjurador() > 0 && ic.nivel() > nivel) {
        nivel = ic.nivel();
        id_classe = ic.id();
      }
    }
  }
  return FeiticosClasse(id_classe, proto);
}

const std::string& ProximaClasseFeiticoAtiva(const EntidadeProto& proto) {
  std::vector<const std::string*> classes;
  for (const auto& ic : proto.info_classes()) {
    if (ic.nivel_conjurador() > 0) {
      classes.push_back(&ic.id());
    }
  }
  if (classes.empty()) return InfoClasse::default_instance().id();
  if (classes.size() == 1) return *classes[0];
  // encontra o indice corrente.
  auto it = std::find_if(classes.begin(), classes.end(), [&proto] (const std::string* c) {
    return proto.classe_feitico_ativa() == *c;
  });
  if (it == classes.end() || *it == classes.back()) return *classes[0];
  // Duas dereferencias: uma do iterador, outra do ponteiro
  return *(*(it + 1));
}

int IndiceFeiticoDisponivel(const std::string& id_classe, int nivel, const EntidadeProto& proto) {
  const auto& fn = FeiticosNivel(id_classe, nivel, proto);
  for (int i = 0; i < fn.para_lancar().size(); ++i) {
    if (!fn.para_lancar(i).usado()) return i;
  }
  return -1;
}

std::unique_ptr<ntf::Notificacao> NotificacaoAlterarFeitico(
    const std::string& id_classe, int nivel, int indice, bool usado, const EntidadeProto& proto) {
  // Consome o slot.
  auto n = NovaNotificacao(ntf::TN_ALTERAR_FEITICO_NOTIFICANDO, proto);
  {
    auto* e_depois = n->mutable_entidade();
    auto* fc = e_depois->add_feiticos_classes();
    fc->set_id_classe(id_classe);
    auto* fn = fc->add_feiticos_por_nivel();
    fn->set_nivel(nivel);
    auto* pl = fn->add_para_lancar();
    pl->set_usado(usado);
    pl->set_indice(indice);
  }
  {
    auto* e_antes = n->mutable_entidade_antes();
    auto* fc = e_antes->add_feiticos_classes();
    fc->set_id_classe(id_classe);
    auto* fn = fc->add_feiticos_por_nivel();
    fn->set_nivel(nivel);
    auto* pl = fn->add_para_lancar();
    pl->set_usado(FeiticoParaLancar(id_classe, nivel, indice, proto).usado());
    pl->set_indice(indice);
  }

  return n;
}

int ComputaLimiteVezes(
    ArmaProto::ModeloLimiteVezes modelo_limite_vezes, const std::string& id_classe, int nivel_conjurador) {
  switch (modelo_limite_vezes) {
    case ArmaProto::LIMITE_UM_CADA_NIVEL_IMPAR_MAX_5: {
      return std::min(5, (nivel_conjurador + 1) / 2);
    }
    break;
    default:
      return 1;
  }
}

std::unique_ptr<ntf::Notificacao> NotificacaoUsarFeitico(
    const Tabelas& tabelas, const std::string& id_classe, int nivel, int indice, const EntidadeProto& proto) {
  // Busca feitico. Se precisa memorizar, busca de ParaLancar, caso contrario, os valores que vem aqui ja sao
  // dos feiticos conhecidos.
  const EntidadeProto::InfoConhecido& ic =
      ClassePrecisaMemorizar(tabelas, id_classe)
      ? FeiticoConhecido(id_classe, FeiticoParaLancar(id_classe, nivel, indice, proto), proto)
      : FeiticoConhecido(id_classe, nivel, indice, proto);
  const auto& feitico_tabelado = tabelas.Feitico(ic.id());
  if (!feitico_tabelado.has_id()) {
    // Nao ha entrada.
    LOG(ERROR) << "Nao ha feitico id '" << ic.id() << "' tabelado: InfoConhecido: " << ic.ShortDebugString()
               << ". id_classe: " << id_classe << ", nivel: " << nivel << ", indice: " << indice;
    return nullptr;
  }
  auto n = NovaNotificacao(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, proto);
  {
    auto* e_depois = n->mutable_entidade();
    *e_depois->mutable_dados_ataque() = proto.dados_ataque();
    auto* da = e_depois->add_dados_ataque();
    da->set_tipo_ataque(ClasseParaTipoAtaqueFeitico(tabelas, IdParaMagia(tabelas, id_classe)));
    da->set_rotulo(feitico_tabelado.nome());
    da->set_id_arma(feitico_tabelado.id());
    da->set_limite_vezes(ComputaLimiteVezes(feitico_tabelado.modelo_limite_vezes(), id_classe, Nivel(id_classe, proto)));
  }
  {
    auto* e_antes = n->mutable_entidade_antes();
    *e_antes->mutable_dados_ataque() = proto.dados_ataque();
    if (e_antes->dados_ataque().empty()) {
      e_antes->add_dados_ataque();   // ataque invalido para sinalizar para apagar.
    }
  }
  return n;
}

// Retorna: id_classe, nivel, indice slot, usado e id entidade na notificacao de alterar feitico. Em caso de
// erro, retorna nivel negativo.
std::tuple<std::string, int, int, bool, unsigned int> DadosNotificacaoAlterarFeitico(const ntf::Notificacao& n) {
  if (n.entidade().feiticos_classes().empty() ||
      n.entidade().feiticos_classes().size() > 1 ||
      n.entidade().feiticos_classes(0).feiticos_por_nivel().empty() ||
      n.entidade().feiticos_classes(0).feiticos_por_nivel().size() > 1 ||
      n.entidade().feiticos_classes(0).feiticos_por_nivel(0).para_lancar().empty() ||
      n.entidade().feiticos_classes(0).feiticos_por_nivel(0).para_lancar().size() > 1) {
    // Bizarramente, make_tuple da pau de linker se usar Entidade::IdInvalido.
    unsigned int id_invalido = Entidade::IdInvalido;
    return std::make_tuple("", -1, 0, false, id_invalido);
  }
  const auto& fc = n.entidade().feiticos_classes(0);
  const auto& fn = fc.feiticos_por_nivel(0);
  const auto& pl = fn.para_lancar(0);
  return std::make_tuple(
      fc.id_classe(), fn.has_nivel() ? fn.nivel() : -1, pl.indice(), pl.usado(), n.entidade().id());
}

std::unique_ptr<ntf::Notificacao> NotificacaoEscolherFeitico(
    const std::string& id_classe, int nivel, const EntidadeProto& proto) {
  const auto& fc = FeiticosClasse(id_classe, proto);
  std::unique_ptr<ntf::Notificacao> n(new ntf::Notificacao);
  if (fc.id_classe().empty()) {
    n->set_tipo(ntf::TN_ERRO);
    n->set_erro(google::protobuf::StringPrintf("Classe '%s' nao encontrada no proto", id_classe.c_str()));
    return n;
  }
  n->set_tipo(ntf::TN_ABRIR_DIALOGO_ESCOLHER_FEITICO);
  n->mutable_entidade()->set_id(proto.id());
  *n->mutable_entidade()->mutable_info_classes() = proto.info_classes();
  *n->mutable_entidade()->add_feiticos_classes() = fc;
  if ((nivel + 1) < fc.feiticos_por_nivel().size()) {
    n->mutable_entidade()->mutable_feiticos_classes(0)->mutable_feiticos_por_nivel()->DeleteSubrange(
        nivel + 1, (fc.feiticos_por_nivel().size() - nivel - 1));
  }
  return n;
}

const EntidadeProto::InfoConhecido& FeiticoConhecido(
    const std::string& id_classe, int nivel, int indice, const EntidadeProto& proto) {
  const auto& fn = FeiticosNivel(id_classe, nivel, proto);
  if (indice >= fn.conhecidos().size()) return EntidadeProto::InfoConhecido::default_instance();
  return fn.conhecidos(indice);
}

const EntidadeProto::InfoLancar& FeiticoParaLancar(
    const std::string& id_classe, int nivel, int indice, const EntidadeProto& proto) {
  const auto& fn = FeiticosNivel(id_classe, nivel, proto);
  if (indice >= fn.para_lancar().size()) return EntidadeProto::InfoLancar::default_instance();
  return fn.para_lancar(indice);
}

bool EntidadeImuneElemento(const EntidadeProto& proto, int elemento) {
  if (elemento == DESC_NENHUM) return false;
  const auto& dd = proto.dados_defesa();
  return std::any_of(dd.imunidades().begin(), dd.imunidades().end(), [elemento](int descritor_imunidade) { return elemento == descritor_imunidade; });
}

const ResistenciaElementos* EntidadeResistenciaElemento(const EntidadeProto& proto, DescritorAtaque elemento) {
  if (elemento == DESC_NENHUM) return nullptr;
  const ResistenciaElementos* maior_resistencia = nullptr;
  for (const auto& resistencia : proto.dados_defesa().resistencia_elementos()) {
    if (resistencia.descritor() == elemento && ((maior_resistencia == nullptr) || (resistencia.valor() > maior_resistencia->valor()))) {
      maior_resistencia = &resistencia;;
    }
  }
  return maior_resistencia;
}

const char* TextoDescritor(int descritor) {
  switch (descritor) {
    case DESC_ACIDO: return "ácido";
    case DESC_AR: return "ar";
    case DESC_CAOS: return "caos";
    case DESC_FRIO: return "frio";
    case DESC_ESCURIDAO: return "escuridão";
    case DESC_MORTE: return "morte";
    case DESC_TERRA: return "terra";
    case DESC_ELETRICIDADE: return "eletricidade";
    case DESC_MAL: return "mal";
    case DESC_MEDO: return "medo";
    case DESC_FOGO: return "fogo";
    case DESC_FORCA: return "forca";
    case DESC_BEM: return "bem";
    case DESC_LEAL: return "leal";
    case DESC_LUZ: return "luz";
    case DESC_MENTAL: return "mental";
    case DESC_SONICO: return "sônico";
    case DESC_AGUA: return "água";
    case DESC_VENENO: return "veneno";
    case DESC_DEPENDENTE_IDIOMA: return "dependente de idioma";
    case DESC_FERRO_FRIO: return "ferro frio";
    case DESC_MADEIRA_NEGRA: return "madeira negra";
    case DESC_MITRAL: return "mitral";
    case DESC_PRATA_ALQUIMICA: return "prata alquímica";
    case DESC_COURO_DRAGAO: return "couro de dragão";
  }
  return "nenhum";
}

ResultadoImunidadeOuResistencia ImunidadeOuResistenciaParaElemento(int delta_pv, const EntidadeProto& proto, DescritorAtaque elemento) {
  ResultadoImunidadeOuResistencia resultado;
  if (delta_pv >= 0 || elemento == DESC_NENHUM) {
    return resultado;
  }
  if (EntidadeImuneElemento(proto, elemento)) {
    resultado.resistido = std::abs(delta_pv);
    resultado.texto = StringPrintf("imunidade: %s", TextoDescritor(elemento));
    resultado.causa = ALT_IMUNIDADE;
    return resultado;
  }

  // Resistencia ao tipo de ataque.
  const ResistenciaElementos* resistencia = EntidadeResistenciaElemento(proto, elemento);
  if (resistencia == nullptr) return resultado;

  resultado.causa = ALT_RESISTENCIA;
  const int valor_efetivo = resistencia->valor();
  resultado.resistido = valor_efetivo > std::abs(delta_pv) ? std::abs(delta_pv) : valor_efetivo;
  resultado.texto = StringPrintf("resistência: %s: %d", TextoDescritor(elemento), valor_efetivo);
  resultado.resistencia = resistencia;
  return resultado;
}

std::tuple<int, std::string> AlteraDeltaPontosVidaPorReducao(
    int delta_pv, const EntidadeProto& proto, const google::protobuf::RepeatedField<int>& descritores) {
  const auto& dd = proto.dados_defesa();
  if (!dd.has_reducao_dano()) {
    return std::make_tuple(delta_pv, "");
  }
  const auto& rd = dd.reducao_dano();
  if (rd.tipo_combinacao() == COMB_E) {
    for (const auto& descritor : rd.descritores()) {
      if (std::none_of(descritores.begin(), descritores.end(), [descritor] (int descritor_ataque) { return descritor_ataque == descritor; } )) {
        delta_pv += dd.reducao_dano().valor();
        return std::make_tuple(std::min(0, delta_pv), google::protobuf::StringPrintf("redução de dano: %d", rd.valor()));
      }
    }
    return std::make_tuple(delta_pv, "redução de dano: não aplicada");
  } else {
    for (const auto& descritor : rd.descritores()) {
      if (std::any_of(descritores.begin(), descritores.end(), [descritor] (int descritor_ataque) { return descritor_ataque == descritor; } )) {
        return std::make_tuple(delta_pv, "redução de dano: não aplicada");
      }
    }
    delta_pv += rd.valor();
    return std::make_tuple(std::min(0, delta_pv), google::protobuf::StringPrintf("redução de dano: %d", rd.valor()));
  }
}

std::tuple<int, std::string> AlteraDeltaPontosVidaPorReducaoBarbaro(int delta_pv, const EntidadeProto& proto) {
  if (proto.dados_defesa().reducao_dano_barbaro() == 0) return std::make_tuple(delta_pv, "");
  return std::make_tuple(std::min(0, delta_pv + proto.dados_defesa().reducao_dano_barbaro()),
      google::protobuf::StringPrintf("redução de dano de bárbaro: %d", proto.dados_defesa().reducao_dano_barbaro()));
}

std::tuple<int, std::string> AlteraDeltaPontosVidaPorMelhorReducao(
    int delta_pv, const EntidadeProto& proto, const google::protobuf::RepeatedField<int>& descritores) {
  int delta_barbaro;
  std::string texto_barbaro;
  std::tie(delta_barbaro, texto_barbaro) = AlteraDeltaPontosVidaPorReducaoBarbaro(delta_pv, proto);
  int delta_outros;
  std::string texto_outros;
  std::tie(delta_outros, texto_outros) = AlteraDeltaPontosVidaPorReducao(delta_pv, proto, descritores);
  std::string texto_final;
  if (delta_barbaro != delta_pv && delta_barbaro >= delta_outros) {
    delta_pv = delta_barbaro;
    texto_final = texto_barbaro;
  } else if (delta_outros != delta_pv && delta_outros > delta_barbaro) {
    delta_pv = delta_outros;
    texto_final = texto_outros;
  }
  return std::make_tuple(delta_pv, texto_final);
}

bool AcaoAfetaAlvo(const AcaoProto& acao_proto, const Entidade& entidade) {
  if (acao_proto.afeta_apenas().empty()) return true;
  return std::any_of(acao_proto.afeta_apenas().begin(), acao_proto.afeta_apenas().end(), [&entidade] (const int tipo) {
    return entidade.TemTipoDnD(static_cast<TipoDnD>(tipo));
  });
}


int NumeroReflexos(const EntidadeProto& proto) {
  int num_reflexos = 0;
  for (const auto& evento : proto.evento()) {
    if (evento.id_efeito() != EFEITO_REFLEXOS) continue;
    if (evento.complementos().empty()) continue;
    num_reflexos = std::max(num_reflexos, evento.complementos(0));
  }
  return num_reflexos;
}

}  // namespace ent
