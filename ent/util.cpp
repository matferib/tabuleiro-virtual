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
#include <google/protobuf/text_format.h>
#include <limits>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>

#include "absl/strings/str_format.h"
#include "ent/acoes.h"
#include "ent/acoes.pb.h"
#include "ent/comum.pb.h"
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/entidade.pb.h"
#include "ent/tabelas.h"
#include "ent/tabuleiro.h"
#include "gltab/gl.h"      // TODO remover e passar desenhos para para gl
#include "gltab/gl_vbo.h"  // TODO remover e passar desenhos para para gl
#include "log/log.h"
#include "tex/texturas.h"

namespace ent {


namespace {
using EfeitoAdicional = ent::AcaoProto::EfeitoAdicional;
using google::protobuf::RepeatedPtrField;

const std::map<std::string, std::string> g_mapa_utf8 = {
    { "á", "a" },
    { "à", "a" },
    { "ã", "a" },
    { "â", "a" },
    { "é", "e" },
    { "ê", "e" },
    { "è", "e" },
    { "í", "i" },
    { "ì", "i" },
    { "ç", "c" },
    { "ô", "o" },
    { "ó", "o" },
    { "õ", "o" },
    { "ò", "o" },
    { "Á", "A" },
    { "Â", "A" },
    { "À", "A" },
    { "É", "E" },
    { "Ê", "E" },
    { "Í", "I" },
    { "Ç", "C" },
    { "Ô", "O" },
    { "Ó", "O" },
    { "Ú", "U" },
    { "ú", "u" },
    { "ù", "u" },
    { "ñ", "n" },
};

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

//------------
// Histograma.
//------------
void Histograma::Adiciona(float valor) {
  total_ += valor;
  if (valor < limite_superior_) {
    total_sem_ultimo_ += valor;
  }
  max_ = std::max(valor, max_);
  min_ = std::min(valor, min_);
  ++AchaIntervalo(valor);
}

void Histograma::Imprime() const {
  int maximo = 0;
  int num_valores = 0, num_valores_sem_ultimo = 0;
  for (const auto& [intervalo, valor] : intervalos_) {
    maximo = std::max(valor, maximo);
    num_valores += valor;
    if (intervalo.max != std::numeric_limits<float>::max()) {
      num_valores_sem_ultimo += valor;
    }
  }
  std::cout << "min: " << min_ << "s, max: " << max_ << "s, total: " << total_ << "s, num valores: " << num_valores << std::endl;
  std::cout << "média: " << (total_ / num_valores) << "s, média sem último: " << (total_sem_ultimo_ / num_valores_sem_ultimo) << "s" << std::endl;
  for (const auto& [intervalo, valor] : intervalos_) {
    const int porcentagem = valor * 50 / maximo;
    const std::string str_porcentagem = std::string(porcentagem, '#');
    std::string s = intervalo.max == std::numeric_limits<float>::max()
        ? absl::StrFormat("[%f, max) -> %d: %s", intervalo.min, valor, str_porcentagem.c_str())
        : absl::StrFormat("[%f, %f) -> %d: %s", intervalo.min, intervalo.max, valor, str_porcentagem.c_str());
    std::cout << s << std::endl;
  }
}
int& Histograma::AchaIntervalo(float valor) {
  int bucket = floor(valor / tam_intervalos_);
  float lim_inferior_intervalo = bucket * tam_intervalos_;
  float lim_superior_intervalo = lim_inferior_intervalo + tam_intervalos_;
  Intervalo intervalo = valor >= limite_superior_
      ? Intervalo(limite_superior_, std::numeric_limits<float>::max())
      : Intervalo(lim_inferior_intervalo, lim_superior_intervalo);
  return intervalos_[intervalo];
}
// Fim histograma.

std::optional<DadosIniciativa> DadosIniciativaEntidade(const Entidade& entidade) {
  if (!entidade.TemIniciativa()) return std::nullopt;
  return DadosIniciativa{entidade.Iniciativa(), entidade.ModificadorIniciativa()};
}

std::optional<DadosIniciativa> DadosIniciativaEvento(const EntidadeProto::Evento& evento) {
  if (!evento.has_iniciativa()) return std::nullopt;
  return DadosIniciativa{evento.iniciativa(), evento.modificador_iniciativa()};
}

std::unique_ptr<ntf::Notificacao> NovoGrupoNotificacoes() {
  auto n = std::unique_ptr<ntf::Notificacao>(ntf::NovaNotificacao(ntf::TN_GRUPO_NOTIFICACOES));
  return n;
}

std::unique_ptr<ntf::Notificacao> NovaNotificacao(ntf::Tipo tipo, const EntidadeProto& proto) {
  auto n = std::unique_ptr<ntf::Notificacao>(ntf::NovaNotificacao(tipo));
  n->mutable_entidade_antes()->set_id(proto.id());
  n->mutable_entidade()->set_id(proto.id());
  return n;
}

ntf::Notificacao* NovaNotificacaoFilha(ntf::Tipo tipo, ntf::Notificacao* pai) {
  if (pai->tipo() != ntf::TN_GRUPO_NOTIFICACOES) {
    LOG(WARNING) << "Notificacao pai nao eh grupo, conferir se chamou errado usando pai->add_notificacao().";
  }
  auto* n = pai->add_notificacao();
  n->set_tipo(tipo);
  return n;
}

std::tuple<ntf::Notificacao*, EntidadeProto*, EntidadeProto*> NovaNotificacaoFilha(
    ntf::Tipo tipo, const EntidadeProto& proto, ntf::Notificacao* pai) {
  auto* n = NovaNotificacaoFilha(tipo, pai);
  n->mutable_entidade_antes()->set_id(proto.id());
  n->mutable_entidade()->set_id(proto.id());
  return std::make_tuple(n, n->mutable_entidade_antes(), n->mutable_entidade());
}

std::tuple<ntf::Notificacao*, EntidadeProto*, EntidadeProto*> NovaNotificacaoFilha(
    ntf::Tipo tipo, const Entidade& entidade, ntf::Notificacao* pai) {
  return NovaNotificacaoFilha(tipo, entidade.Proto(), pai);
}

std::unique_ptr<ntf::Notificacao> NovaNotificacaoAcaoTextoLogada(const std::string& texto, const EntidadeProto& proto, const std::optional<float> atraso_s) {
  auto na = NovaNotificacao(ntf::TN_ADICIONAR_ACAO);
  auto* a = na->mutable_acao();
  a->set_tipo(ACAO_DELTA_PONTOS_VIDA);
  auto* por_entidade = a->add_por_entidade();
  por_entidade->set_id(proto.id());
  por_entidade->set_texto(texto);
  a->set_afeta_pontos_vida(false);
  a->set_local_apenas(false);
  a->set_adiciona_ao_log_se_local(true);
  if (atraso_s.has_value()) {
    a->set_atraso_s(*atraso_s);
  }
  return na;
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

void AplicaNevoa(float* cor, const ParametrosDesenho* pd) {
  if (!pd->has_nevoa()) {
    return;
  }
  float distancia = DistanciaPontoCorrenteParaNevoa(pd);
  if (distancia > pd->nevoa().maximo()) {
     cor[0] = pd->nevoa().cor().r();
     cor[1] = pd->nevoa().cor().g();
     cor[2] = pd->nevoa().cor().b();
     return;
  }
  if (distancia > pd->nevoa().minimo()) {
    float escala = (distancia - pd->nevoa().minimo()) / (pd->nevoa().maximo() - pd->nevoa().minimo());
    cor[0] = (pd->nevoa().cor().r() * escala + cor[0] * (1.0f - escala));
    cor[1] = (pd->nevoa().cor().g() * escala + cor[1] * (1.0f - escala));
    cor[2] = (pd->nevoa().cor().b() * escala + cor[2] * (1.0f - escala));
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

Cor CorParaProto(const float* cor) {
  Cor proto_cor;
  proto_cor.set_r(cor[0]);
  proto_cor.set_g(cor[1]);
  proto_cor.set_b(cor[2]);
  return proto_cor;
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

Cor CorRealcada(const Cor& cor) {
  Cor cr = cor;
  if (!cr.has_r()) {
    cr.set_r(1);
    cr.set_g(1);
    cr.set_b(1);
  }
  RealcaCor(&cr);
  return cr;
}

namespace {
float CombinaComponenteCor(float c1, float c2) {
  return std::min(1.0f, c1 + c2);
}
float CombinaComponenteCorComPeso(float peso_c1, float c1, float c2) {
  return std::min(1.0f, peso_c1 * c1 + (1 - peso_c1) * c2);
}

}  // namespace

void CombinaCor(const Cor& cor_origem, Cor* cor_destino) {
  cor_destino->set_r(CombinaComponenteCor(cor_origem.r(), cor_destino->r()));
  cor_destino->set_g(CombinaComponenteCor(cor_origem.g(), cor_destino->g()));
  cor_destino->set_b(CombinaComponenteCor(cor_origem.b(), cor_destino->b()));
}

void CombinaCorComPeso(float peso_origem, const Cor& cor_origem, Cor* cor_destino) {
  cor_destino->set_r(CombinaComponenteCorComPeso(peso_origem, cor_origem.r(), cor_destino->r()));
  cor_destino->set_g(CombinaComponenteCorComPeso(peso_origem, cor_origem.g(), cor_destino->g()));
  cor_destino->set_b(CombinaComponenteCorComPeso(peso_origem, cor_origem.b(), cor_destino->b()));
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

float DistanciaEmMetrosAoQuadrado(const Posicao& pos1, const Posicao& pos2) {
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

void DesenhaLinha3d(const RepeatedPtrField<Posicao>& pontos, float largura) {
  DesenhaLinha3dBase(pontos, largura);
}

void LimitesLinha3d(const RepeatedPtrField<Posicao>& pontos, float largura, float* xi, float* yi, float *xs, float *ys) {
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
    gl::MatrizEscopo salva_projecao(gl::MATRIZ_PROJECAO);
    gl::CarregaIdentidade();
    // Eixo com origem embaixo esquerda.
    gl::Ortogonal(0, largura, 0, altura, 0.0f, 1.0f);
    gl::AtualizaMatrizes();
    {
      gl::MatrizEscopo salva_projecao(gl::MATRIZ_MODELAGEM);
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
  int mult = 0;           // O numero antes do dado.
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
    ET_ESPERANDO_NUMERO,
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
        //LOG(INFO) << "inicial";
        if (token == "d") {
          LOG(ERROR) << "Estado inicial deve comecar com numero ou sinal, encontrei: 'd', string: " << dados_vida;
          et = ET_ERRO;
        } else if (token == "+" || token == "-") {
          num = (token == "+") ? 1 : -1;
          et = ET_LEU_MAIS_MENOS;
        } else {
          num = atoi(token.c_str());
          et = ET_LEU_NUM;
        }
        break;
      case ET_ESPERANDO_NUMERO:
        //LOG(INFO) << "esperando numero";
        if (token == "+" || token == "-" || token == "d") {
          LOG(ERROR) << "Estado inicial deve comecar com numero, encontrei: " << token << ", string: " << dados_vida;
          et = ET_ERRO;
        } else {
          num = atoi(token.c_str()) * num;
          et = ET_LEU_NUM;
        }
        break;
      case ET_LEU_NUM:
        //LOG(INFO) << "leu numero";
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
          et = ET_ESPERANDO_NUMERO;
          num = (token == "+") ? 1 : -1;
        } else {
          LOG(ERROR) << "Esperava d ou +- apos numero, encontrei: " << token;
          et = ET_ERRO;
        }
        break;
      case ET_LEU_D:
        //LOG(INFO) << "leu d";
        if (token == "+" || token == "-" || token == "d") {
          LOG(ERROR) << "Esperando numero apos d, encontrei: " << token;
          et = ET_ERRO;
        } else {
          corrente.dado = atoi(token.c_str());
          et = ET_LEU_DADO;
        }
        break;
      case ET_LEU_DADO:
        //LOG(INFO) << "leu dado";
        if (token == "+" || token == "-") {
          num = (token == "+") ? 1 : -1;
          et = ET_LEU_MAIS_MENOS;
        } else {
          LOG(ERROR) << "Esperando +- apos dado, encontrei: " << token;
          et = ET_ERRO;
        }
        break;
      case ET_LEU_MAIS_MENOS:
        //LOG(INFO) << "leu +-";
        if (token == "+" || token == "-") {
          LOG(ERROR) << "Esperando numero ou dado apos +-, encontrei: " << token;
          et = ET_ERRO;
        } else {
          num = atoi(token.c_str()) * num;
          et = ET_LEU_NUM;
        }
        break;
      case ET_ERRO:
        //LOG(INFO) << "erro";
        break;
      default:
        break;
    }
    if (et == ET_ERRO) {
      break;
    }
  }
  //LOG(INFO) << "fim!";
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

// Apenas para testes e dados forçados.
std::queue<int> g_dados_teste;
// Primeira dimensao: numero de faces do dado.
// Segunda: numero rolado.
// Terceira: quantidade.
std::unordered_map<int, std::map<int, int>> g_dados_rolados;

void AcumulaDado(int valor) {
  VLOG(1) << "acumulando: " << valor;
  g_dados_teste.push(valor);
}

void LimpaDadosAcumulados() {
  while (!g_dados_teste.empty()) g_dados_teste.pop();
}

// Rola um dado de nfaces.
int RolaDado(unsigned int nfaces, bool ignora_forcado) {
  // TODO inicializacao do motor de baseada no timestamp.
  static std::default_random_engine motor(std::chrono::system_clock::now().time_since_epoch().count());
  if (!g_dados_teste.empty() && !ignora_forcado) {
    int valor = g_dados_teste.front();
    g_dados_teste.pop();
    VLOG(1) << "retornando valor forcado: " << valor;
    return valor;
  }
  std::uniform_int_distribution<int> distribution(1, nfaces);
  //static int min = motor_aleatorio.min();
  //static int max = motor_aleatorio.max();
  int valor = distribution(motor);
  g_dados_rolados[nfaces][valor]++;
  return valor;
}

void ImprimeDadosRolados() {
  for (const auto& [nfaces, valores] : g_dados_rolados) {
    if (c_none<std::vector<int>>({3, 4, 6, 8, 10, 12, 20, 100}, nfaces)) {
      LOG(INFO) << "ignorando d" << nfaces;
      continue;
    }
    LOG(INFO) << "Imprimindo valores de d" << nfaces;
    for (const auto& [valor, vezes] : valores) {
      LOG(INFO) << "  Valor '" << valor << "' rolado '" << vezes << "' vezes";
    }
  }
}

float Aleatorio() {
  int val = RolaDado(10001, /*ignora_forcado=*/true) - 1;  // [0-10000]
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
  s += absl::StrFormat("%d, ", total);
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
    // mds.Imprime();
    for (int i = 0; i < std::abs(mds.mult); ++i) {
      int valor_dado = RolaDado(mds.dado) * ((mds.mult < 0) ? -1 : 1);
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
  //AplicaNevoa(rgb, pd);
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

MisturaPreNevoaEscopo::MisturaPreNevoaEscopo(float r, float g, float b, float a) {
  gl::LeCorMisturaPreNevoa(salvo_);
  gl::CorMisturaPreNevoa(r, g, b, a);
}

MisturaPreNevoaEscopo::MisturaPreNevoaEscopo(const Cor& cor, const ParametrosDesenho* pd)
    : MisturaPreNevoaEscopo(cor.has_r() ? cor.r() : 1.0f,
                            cor.has_g() ? cor.g() : 1.0f,
                            cor.has_b() ? cor.b() : 1.0f,
                            cor.a() < 1.0f ? cor.a() : pd->has_alfa_translucidos() ? pd->alfa_translucidos() : 1.0f) {}

MisturaPreNevoaEscopo::~MisturaPreNevoaEscopo() {
  gl::CorMisturaPreNevoa(salvo_[0], salvo_[1], salvo_[2], salvo_[3]);
}

TipoAtaque DaParaTipoAtaque(const DadosAtaque& da) {
  if (da.ataque_agarrar()) return TipoAtaque::AGARRAR;
  if (da.ataque_desarmar()) return TipoAtaque::DESARMAR;
  if (da.ataque_distancia()) return TipoAtaque::DISTANCIA;
  return TipoAtaque::CORPO_A_CORPO;
}

float DistanciaMetros(const Posicao& pos_acao_a, const Posicao& pos_acao_d) {
  Vector3 va(pos_acao_a.x(), pos_acao_a.y(), pos_acao_a.z());
  Vector3 vd;
  float distancia_m = 0;
  vd = Vector3(pos_acao_d.x(), pos_acao_d.y(), pos_acao_d.z());
  distancia_m += (va - vd).length();
  VLOG(2) << "distancia_m: " << distancia_m;
  return distancia_m;
}

// Esse modificador se aplica a ataque e dano.
int AplicaMaestriaElemental(int modificador, const EntidadeProto& ea, const EntidadeProto& ed) {
  if (!TemTipoDnD(TIPO_ELEMENTAL, ea)) return modificador;

  if (TemSubTipoDnD(SUBTIPO_TERRA, ea)) {
    if (ed.voadora() || ed.nadando()) {
      // Elemental tem dificuldade contra ar e agua.
      modificador -= 4;
    } else if (!ea.voadora() && !ea.nadando()) {
      // Tem facilidade contra terrestres.
      ++modificador;
    }
    return modificador;
  }
  if (TemSubTipoDnD(SUBTIPO_AR, ed)) {
    if (ea.voadora()) {
      // Aereos tem dificuldade contra elemental.
      --modificador;
    }
    return modificador;
  }
  if (TemSubTipoDnD(SUBTIPO_AGUA, ea)) {
    if (ea.nadando() && ed.nadando()) {
      // Ambos na agua.
      ++modificador;
    } else if (!ea.voadora() && !ea.nadando()) {
      // Elemental tem dificuldade em terra firme.
      modificador -= 4;
    } else if (!ed.voadora() && !ed.nadando()) {
      // Defensor em terra firme.
      modificador -= 4;
    }
    return modificador;
  }
  return modificador;
}

int ModificadoresDesarmarSemBonusAtaque(const DadosAtaque& da, const EntidadeProto& proto) {
  int modificador = 0;
  if (da.empunhadura() == EA_2_MAOS) {
    modificador += 4;
  }
  const auto& arma = Tabelas::Unica().Arma(da.id_arma());
  if (PossuiCategoria(CAT_LEVE, arma)) {
    modificador -= 4;
  }
  modificador += arma.bonus_desarmar();
  if (PossuiTalento("desarme_aprimorado", proto)) {
    modificador += 4;
  }
  // O modificador de tamanho sera computado em ModificadorAtaque, que tem o atacante e o alvo.
  return modificador;
}

// Pode ser chamado com ed == default para ver alguns modificadores do atacante.
int ModificadorAtaque(const DadosAtaque& da, const EntidadeProto& ea, const EntidadeProto& ed) {
  TipoAtaque tipo_ataque = DaParaTipoAtaque(da);
  int modificador = 0;
  if (tipo_ataque == TipoAtaque::DESARMAR) {
    modificador += ModificadoresDesarmarSemBonusAtaque(da, ea);
    modificador += 4 * (static_cast<int>(ea.tamanho()) - static_cast<int>(ed.tamanho()));
  }
  // ataque.
  if (ea.caida() && tipo_ataque != TipoAtaque::DISTANCIA) {
    modificador -= 4;
  }
  if (ed.em_corpo_a_corpo() && tipo_ataque == TipoAtaque::DISTANCIA && !PossuiTalento("tiro_preciso", ea)) {
    modificador -= 4;
  }
  if (tipo_ataque == TipoAtaque::DISTANCIA) {
    // era tiro_queima_roupa
    if (PossuiTalento("tiro_certeiro", ea) && DistanciaMetros(ea.pos(), ed.pos()) < 9) {
      modificador += 1;
    }
  }
  if (tipo_ataque == TipoAtaque::CORPO_A_CORPO && ea.dados_ataque_global().flanqueando() &&
      !PossuiHabilidadeEspecial("esquiva_sobrenatural_aprimorada", ed)) {
    modificador += 2;
  }
  // Invisibilidade da +2 ataque desde que alvo nao esteja cego. Neste caso sera tratado
  // na parte de modificadores de CA no final da funcao.
  if (PossuiEventoNaoPossuiOutro(EFEITO_INVISIBILIDADE, EFEITO_POEIRA_OFUSCANTE, ea) &&
      !PossuiEvento(EFEITO_VER_INVISIVEL, ed) && !PossuiEvento(EFEITO_CEGO, ed)) {
    if (!PossuiTalento("lutar_as_cegas", ed) || tipo_ataque == TipoAtaque::DISTANCIA) {
      modificador += 2;
    }
  }
  if (tipo_ataque == TipoAtaque::AGARRAR) {
    if (PossuiTalento("agarrar_aprimorado", ea)) modificador += 4;
    if (PossuiTalento("agarrar_aprimorado", ed)) modificador -= 4;
  }
  // Nao aplica contra a entidade default.
  if (ed.has_pos() && ea.pos().z() - ed.pos().z() >= TAMANHO_LADO_QUADRADO &&
      tipo_ataque != TipoAtaque::DISTANCIA) {
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
  if (ea.dados_ataque_global().ataque_mais_16()) {
    modificador += 16;
  }

  // Modificadores que se aplicariam na CA.
  if (PossuiEvento(EFEITO_CEGO, ed)) {
    if (!PossuiTalento("lutar_as_cegas", ed) || tipo_ataque == TipoAtaque::DISTANCIA) {
      modificador += 2;
    }
  }
  if (PossuiEvento(EFEITO_ATORDOADO, ed)) {
    modificador += 2;
  }
  if (ed.caida() || Indefeso(ed)) {
    if (tipo_ataque == TipoAtaque::DISTANCIA) modificador -= 4;
    else modificador += 4;
  }
  modificador = AplicaMaestriaElemental(modificador, ea, ed);
  for (const auto& [sub_tipo, valor] : Tabelas::Unica().Raca(ea.raca()).dados_ataque_global().bonus_ataque_por_sub_tipo()) {
    if (TemSubTipoDnD(static_cast<SubTipoDnD>(sub_tipo), ed)) {
      modificador += valor;
    }
  }
  return modificador;
}

int ModificadorDano(
    const DadosAtaque& da, const EntidadeProto& ea, const EntidadeProto& ed) {
  TipoAtaque tipo_ataque = DaParaTipoAtaque(da);
  int modificador = 0;
  if (tipo_ataque == TipoAtaque::DISTANCIA) {
    // Se houver alvo, verifica possibilidade de queima roupa.
    // nome melhor seria: tiro_queima_roupa.
    if (da.eh_arma() && PossuiTalento("tiro_certeiro", ea) &&
        ed.has_id() && DistanciaMetros(ea.pos(), ed.pos()) < 9) {
      modificador += 1;
    }
  }

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
  modificador = AplicaMaestriaElemental(modificador, ea, ed);
  return modificador;
}

float DistanciaAbsoluta(const Posicao& pos_atacante, const Posicao& pos_alvo) {
  const float resultado = (PosParaVector3(pos_atacante) - PosParaVector3(pos_alvo)).length();
  VLOG(1) << "Distancia absoluta: " << resultado;
  return resultado;
}

// Teste varias distancias diferentes e usa a menor.
// Entidade a entidade;
// Entidade ao ponto clique;
// Ponto de acao a entidade.
float DistanciaMinimaAcaoAlvoMetros(const Entidade& ea, const Posicao& pos_clique) {
  const Posicao pos_acao = ea.PosicaoAcao();
  // Raio de acao da entidade, de forma aproximada.
  const float raio = DistanciaAbsoluta(pos_acao, ea.Pos());
  // Essa é a ideal, mas nem sempre a entidade esta com giro correto.
  float distancia = DistanciaAbsoluta(pos_acao, pos_clique);
  // Posicao da entidade a posicao clicada descontando raio.
  distancia = std::min(distancia, std::max(0.0f, DistanciaAbsoluta(ea.Pos(), pos_clique) - raio));
  VLOG(1) << "Distancia minima: " << distancia;
  return distancia;
}

float DistanciaMaximaAcaoAlvoMetros(const Entidade& ea, const Posicao& pos_clique) {
  const Posicao pos_acao = ea.PosicaoAcao();
  // Raio de acao da entidade, de forma aproximada.
  const float raio = DistanciaAbsoluta(pos_acao, ea.Pos());
  // Essa é a ideal, mas nem sempre a entidade esta com giro correto.
  float distancia = DistanciaAbsoluta(pos_acao, pos_clique);
  // Posicao da entidade a posicao clicada descontando raio.
  distancia = std::max(distancia, std::max(0.0f, DistanciaAbsoluta(ea.Pos(), pos_clique) - raio));
  VLOG(1) << "Distancia maxima: " << distancia;
  return distancia;
}

std::tuple<std::string, bool, float> VerificaAlcanceMunicao(
    const AcaoProto& ap, const Entidade& ea, const Entidade& ed, const Posicao& pos_clique) {
  const auto& da = ea.DadoCorrenteNaoNull();
  if ((ap.tipo() == ACAO_PROJETIL || ap.tipo() == ACAO_PROJETIL_AREA) &&
      da.has_municao() && da.municao() == 0) {
    VLOG(1) << "Nao ha municao para ataque: " << da.DebugString();
    return std::make_tuple("Sem munição", false, 0.0f);
  }
  if (ea.Id() == ed.Id()) {
    return std::make_tuple("", true, 0.0f);
  }

  const float alcance_m = ea.AlcanceAtaqueMetros();
  if (alcance_m < 0) {
    return std::make_tuple("Ataque sem alcance", false, 0.0f);
  }
  // Usa a menor distancia para verificar alcance maximo.
  const float distancia_m = DistanciaMinimaAcaoAlvoMetros(ea, pos_clique);
  if (distancia_m > alcance_m) {
    int total_incrementos = distancia_m / alcance_m;
    if (total_incrementos > da.incrementos()) {
      return std::make_tuple(
          absl::StrFormat("Fora de alcance: %0.1fm > %0.1fm, inc: %d, max: %d", distancia_m, alcance_m, total_incrementos, da.incrementos()), false, distancia_m);
    }
  }
  // Se houver alcance minimo, verifica com a menor distancia.
  const float alcance_minimo_m = ea.AlcanceMinimoAtaqueMetros();
  if (float distancia_para_alcance_minimo_m = DistanciaMaximaAcaoAlvoMetros(ea, pos_clique); alcance_minimo_m > 0 && distancia_para_alcance_minimo_m < alcance_minimo_m) {
    return std::make_tuple(absl::StrFormat("Alvo muito perto: alcance mínimo: %0.1fm, distância: %0.1f", alcance_minimo_m, distancia_para_alcance_minimo_m), false, distancia_para_alcance_minimo_m);
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

Entidade::TipoCA CATipoAtaque(const DadosAtaque& da) {
  auto tipo = da.ataque_toque() ? Entidade::CA_TOQUE : Entidade::CA_NORMAL;
  VLOG(1) << "tipo CA: " << tipo;
  return tipo;
}

// Retorna true se o ataque for bem sucedido, false com string caso contrario.
std::tuple<std::string, bool> AtaqueVsChanceFalha(
    NivelIluminacao nivel_iluminacao, const DadosAtaque& da, const Entidade& ea, const Entidade& ed) {
  if (ea.IgnoraChanceFalha() || da.id_arma() == "missil_magico") {
    VLOG(1) << "ataque ignorando chance de falha";
    return std::make_pair("", true);
  }
  VLOG(1) << "chance falha ataque: " << ea.ChanceFalhaAtaque();
  VLOG(1) << "chance falha defesa: " << ea.ChanceFalhaDefesa();
  const int chance_falha = std::max(ea.ChanceFalhaAtaque(), ed.ChanceFalhaDefesa(da));
  if (chance_falha > 0) {
    const int d100 = RolaDado(100);
    VLOG(1) << "Chance de falha: " << chance_falha << ", tirou: " << d100;
    if (d100 < chance_falha) {
      return std::make_tuple(absl::StrFormat("Falhou, chance %d, tirou %d", chance_falha, d100), false);
    }
  }
  return std::make_pair("", true);
}

// Retorna o texto sinalizado para o modificador se diferente de zero, ou "".
std::string TextoOuNada(int modificador) {
  if (modificador != 0) {
    return absl::StrFormat("%+d", modificador);
  }
  return "";
}

std::string TextoOuNada(const std::string& texto) {
  if (texto.empty()) return "";
  return texto;
}

bool ArmaAbencoada(const DadosAtaque& da, const Entidade& ea) {
  // TODO vorpal tb nao se aplica aqui.
  return PossuiEvento(EFEITO_ABENCOAR_ARMA, da.rotulo(), ea.Proto()) && !PossuiEvento(EFEITO_LAMINA_AFIADA, da.rotulo(), ea.Proto());
}

// Retorna o numero de vezes que o critico da dano e o texto para o critico.
std::tuple<int, std::string> AplicaCriticoSeDentroMargem(
    int d20, int ataque_origem, int modificador_incrementos, int outros_modificadores, int ca_destino,
    const DadosAtaque& da, const Entidade& ea, const Entidade& ed) {
  assert(modificador_incrementos <= 0);
  TipoAtaque tipo_ataque = DaParaTipoAtaque(da);
  if (d20 < ea.MargemCritico() || tipo_ataque == TipoAtaque::AGARRAR || tipo_ataque == TipoAtaque::DESARMAR) return std::make_tuple(1, "");
  if (ed.ImuneCritico()) {
    return std::make_tuple(1, ", imune a critico");
  }
  std::string texto_critico;
  if (ArmaAbencoada(da, ea) && ed.Mau()) {
    texto_critico = ", critico: arma abençoada vs mal";
  } else {
    int d20_critico = RolaDado(20);
    if (d20_critico == 1) return std::make_tuple(1, ", critico falhou: 1");
    int total_critico = ataque_origem + d20_critico + modificador_incrementos + outros_modificadores;
    if (total_critico < ca_destino) {
      return std::make_tuple(
          1, absl::StrFormat(", critico falhou: %d+%d%s%s= %d < %d",
                          d20_critico, ataque_origem,
                          TextoOuNada(modificador_incrementos).c_str(),
                          TextoOuNada(outros_modificadores).c_str(),
                          total_critico, ca_destino));
    }
    texto_critico = absl::StrFormat(
        ", critico %d+%d%s%s= %d",
        d20_critico, ataque_origem, TextoOuNada(modificador_incrementos).c_str(), TextoOuNada(outros_modificadores).c_str(), total_critico);
  }
  return std::make_tuple(ea.MultiplicadorCritico(), texto_critico);
}

// Retorna o resultado e quanto tirou no dado.
std::tuple<resultado_ataque_e, int, std::string, bool> ComputaAcertoOuErro(
    int d20_ataque, int ataque_origem, int modificador_incrementos, int outros_modificadores, int d20_defesa, int modificadores_defesa, TipoAtaque tipo_ataque,
    const EntidadeProto& pea, const EntidadeProto& ped, bool confirmacao = false) {
  assert(modificador_incrementos <= 0);
  if (tipo_ataque == TipoAtaque::AGARRAR && PossuiEvento(EFEITO_MOVIMENTACAO_LIVRE, ped)) {
    return std::make_tuple(RA_FALHA_NORMAL, 0, "Defensor com movimentação livre", false);
  }

  int valor_defesa = d20_defesa + modificadores_defesa;
  int total = d20_ataque + ataque_origem + modificador_incrementos + outros_modificadores;
  if (d20_ataque == 1) {
    if (confirmacao) {
      return std::make_tuple(RA_FALHA_AUTOMATICA, 1, "outro 1", false);
    } else {
      VLOG(1) << "Possivel falha critica";
      // Confirma a falha.
      auto [resultadoc, totalc, textoc, acertouc] =
        ComputaAcertoOuErro(RolaDado(20), ataque_origem, modificador_incrementos, outros_modificadores, d20_defesa, modificadores_defesa, tipo_ataque, pea, ped,
            /*confirmacao=*/true);
      VLOG(1) << "Confirmacao de falha critica: resultado: " << resultadoc << ", total: " << totalc << ", texto: " << textoc << ", acertou: " << acertouc;
      if (acertouc) {
        return std::make_tuple(RA_FALHA_AUTOMATICA, 1, absl::StrFormat("falha critica nao confirmada: %s", textoc.c_str()), false);
      } else {
        return std::make_tuple(RA_FALHA_CRITICA, 1, absl::StrFormat("falha critica confirmada: %s", textoc.c_str()), false);
      }
    }
  }

  if ((d20_ataque != 20 || (tipo_ataque == TipoAtaque::AGARRAR || tipo_ataque == TipoAtaque::DESARMAR)) && total < valor_defesa) {
    std::string texto = absl::StrFormat(
        "falhou: %d%+d%s%s= %d < %d", d20_ataque, ataque_origem,
        TextoOuNada(modificador_incrementos).c_str(), TextoOuNada(outros_modificadores).c_str(), total, valor_defesa);
    return std::make_tuple(RA_FALHA_NORMAL, total, texto, false);
  } else if (tipo_ataque == TipoAtaque::AGARRAR && total == valor_defesa) {
    // Maior modificador ganha.
    if (pea.bba().agarrar() < ped.bba().agarrar()) {
      // TODO em case de empate, deveria rolar de novo. Mas vou considerar vantagem da defesa.
      return std::make_tuple(RA_FALHA_NORMAL, total, absl::StrFormat("falhou: mod defesa %d > %d ataque", ped.bba().agarrar(), pea.bba().agarrar()).c_str(), false);
    }
  } else if (tipo_ataque == TipoAtaque::DESARMAR) {
    if (total == valor_defesa) {
      // Maior modificador ganha.
      if (modificadores_defesa >= ataque_origem) {
        return std::make_tuple(RA_FALHA_NORMAL, total, absl::StrFormat("falhou: mod defesa %d >= %d ataque", modificadores_defesa, ataque_origem).c_str(), false);
      }
    } else {
      // Acertou agarrar.
      return std::make_tuple(RA_SUCESSO, total, absl::StrFormat("sucesso: mod defesa %d >= %d ataque", modificadores_defesa, ataque_origem).c_str(), true);
    }
  }
  return std::make_tuple(RA_SUCESSO, total, absl::StrFormat(
        "sucesso: %d%+d%s%s= %d >= %d", d20_ataque, ataque_origem,
        TextoOuNada(modificador_incrementos).c_str(), TextoOuNada(outros_modificadores).c_str(), total, valor_defesa), true);
}

// Retorna o resultado do ataque de toque o se acertou ou nao.
std::tuple<std::string, bool> AtaqueToquePreAgarrar(int outros_modificadores, const DadosAtaque& da, const Entidade& ea, const Entidade& ed) {
  // TODO: aqui to hackeando pra pular o toque se ambos estiverem agarrando.
  if (da.ignora_ataque_toque() || AgarradoA(ed.Id(), ea.Proto())) {
    return std::make_tuple("", true);
  }
  const int d20_toque = RolaDado(20);
  const int ca_destino_toque = ed.CA(ea, Entidade::CA_TOQUE);
  const int ataque_toque = ea.BonusAtaqueToque();
  std::string texto_erro;
  bool acertou;
  // O tipo desse ataque é normal, não é agarrar.
  std::tie(std::ignore, std::ignore, texto_erro, acertou) =
      ComputaAcertoOuErro(d20_toque, ataque_toque, 0, outros_modificadores, /*d20_defesa=*/0, ca_destino_toque, TipoAtaque::CORPO_A_CORPO, ea.Proto(), ed.Proto());
  if (!acertou) {
    std::string texto_falha_toque = absl::StrFormat("Ataque de toque falhou: %s", texto_erro.c_str());
    return std::make_tuple(texto_falha_toque, false);
  }
  std::string texto = absl::StrFormat(", toque ok: %d%+d < %d", d20_toque, ataque_toque, ca_destino_toque);
  return std::make_tuple(texto, true);
}

enum resultado_ataque_reflexos {
  RAR_ACERTOU = 0,
  RAR_FALHA_NORMAL = 1,
  RAR_FALHA_CRITICA = 2,
};
std::tuple<std::string, resultado_ataque_reflexos> AtaqueToqueReflexos(
    int outros_modificadores, const DadosAtaque& da, const Entidade& ea, const Entidade& ed) {
  const int d20 = RolaDado(20);
  if (d20 == 1) {
    return std::make_tuple("falha crítica", RAR_FALHA_CRITICA);
  }
  const int ca_reflexo = ed.CAReflexos();
  const int ataque = ea.BonusAtaque();
  std::string texto_erro;
  bool acertou;
  std::tie(std::ignore, std::ignore, texto_erro, acertou) =
      ComputaAcertoOuErro(d20, ataque, 0, outros_modificadores, /*d20_defesa=*/0, ca_reflexo, DaParaTipoAtaque(da), ea.Proto(), ed.Proto());
  return std::make_tuple(
    acertou ? "acertou reflexo" : "errou reflexo",
    acertou ? RAR_ACERTOU : RAR_FALHA_NORMAL);
}

}  // namespace

ResultadoAtaqueVsDefesa AtaqueVsDefesa(
    float distancia_m, const AcaoProto& ap, const Entidade& ea, const Entidade& ed, const Posicao& pos_alvo, bool ataque_oportunidade) {
  const auto& da = ea.DadoCorrenteNaoNull();
  return AtaqueVsDefesa(distancia_m, ap, ea, da, ed, pos_alvo, ataque_oportunidade);
}

// Retorna o d20 da defesa e os modificadores, incluindo o bonus de ataque.
std::optional<std::pair<unsigned int, unsigned int>> D20ModificadoresDefesa(const DadosAtaque& da, const Entidade& ea, const Entidade& ed, bool ataque_oportunidade) {
  if (da.ataque_agarrar()) {
    const int bonus_agarrar_defesa = ed.Proto().bba().agarrar();
    return std::make_pair(RolaDado(20), bonus_agarrar_defesa);
  } else if (da.ataque_desarmar()) {
    return std::make_pair(RolaDado(20), ed.BonusAtaque() + ModificadoresDesarmarSemBonusAtaque(ed.DadoCorrenteNaoNull(), ed.Proto()));
  }
  int ca = ed.CA(ea, CATipoAtaque(da), ataque_oportunidade);
  if (ca == Entidade::AtaqueCaInvalido) return std::nullopt;
  return std::make_pair(0, ca);
}

std::optional<int> BonusAtaque(const Entidade& ea) {
  const int modificadores_ataque = ea.BonusAtaque();
  if (modificadores_ataque == Entidade::AtaqueCaInvalido) return std::nullopt;
  return modificadores_ataque;
}

std::string StringValorDestino(TipoAtaque tipo_ataque, int d20_defesa, int valor_defesa) {
  switch (tipo_ataque) {
    case TipoAtaque::AGARRAR:
    case TipoAtaque::DESARMAR:
      return absl::StrFormat("%d=%d+%d", d20_defesa + valor_defesa, d20_defesa, valor_defesa);
    default:
      return absl::StrFormat("%d", valor_defesa);
  }
}

ResultadoAtaqueVsDefesa AtaqueVsDefesa(
    float distancia_m, const AcaoProto& ap, const Entidade& ea, const DadosAtaque& da,
    const Entidade& ed, const Posicao& pos_alvo, bool ataque_oportunidade) {
  ResultadoAtaqueVsDefesa resultado;
  if (EmDefesaTotal(ea.Proto())) {
    VLOG(1) << "Ignorando ataque vs defesa por causa de defesa total.";
    resultado.texto = "Atacante em defesa total";
    return resultado;
  }

  std::optional<int> opt_bonus_ataque = BonusAtaque(ea);
  if (!opt_bonus_ataque.has_value()) {
    VLOG(1) << "Ignorando ataque vs defesa por falta de dados de ataque";
    resultado.texto = "Ataque sem bonus";
    return resultado;
  }
  int bonus_ataque = *opt_bonus_ataque;

  std::optional<std::pair<unsigned int, unsigned int>> opt_modificadores_defesa = D20ModificadoresDefesa(da, ea, ed, ataque_oportunidade);
  if (!opt_modificadores_defesa.has_value()) {
    VLOG(1) << "Ignorando ataque vs defesa por falta de dados de defesa.";
    resultado.texto = "Ataque sem bonus ou defensor sem armadura";
    return resultado;
  }
  auto [d20_defesa, modificadores_defesa] = *opt_modificadores_defesa;

  int modificador_incrementos = ModificadorAlcance(distancia_m, ap, ea);
  const int modificadores_ataque = ModificadorAtaque(da, ea.Proto(), ed.Proto());
  if (ea.Id() != ed.Id() && !da.ignora_reflexos()) {
    const int numero_reflexos = NumeroReflexos(ed.Proto());
    if (numero_reflexos > 0) {
      int num_total = numero_reflexos + 1;
      int dref = RolaDado(num_total);
      VLOG(1) << absl::StrFormat("dado ref: %d em d%d", dref, num_total);
      if (dref != 1) {
        VLOG(1) << "Ataque no reflexo";
        // Ataque no reflexo.
        auto [texto_reflexo, rar] = AtaqueToqueReflexos(modificadores_ataque, da, ea, ed);
        resultado.texto = absl::StrFormat("%s, %d de %d", texto_reflexo.c_str(), dref - 1, num_total - 1);
        if (rar == RAR_ACERTOU) {
          resultado.resultado = RA_FALHA_REFLEXO;
        } else {
          resultado.resultado = (rar == RAR_FALHA_CRITICA) ? RA_FALHA_CRITICA : RA_FALHA_NORMAL;
        }
        return resultado;
      }
      VLOG(1) << "Ataque acertou alvo mesmo com reflexo";
    }
  } else {
    VLOG(1) << "Ataque ignorou reflexos " << (da.ignora_reflexos() ? "por flag." : "por ser pessoal.");
  }

  // Realiza um ataque de toque.
  std::string texto_toque_agarrar;
  if (da.ataque_agarrar()) {
    bool acertou;
    std::tie(texto_toque_agarrar, acertou) = AtaqueToquePreAgarrar(modificadores_ataque, da, ea, ed);
    if (!acertou) {
      resultado.resultado = RA_FALHA_TOQUE_AGARRAR;
      resultado.texto = texto_toque_agarrar;
      return resultado;
    }
  }

  // Rola o dado de ataque!
  int d20 = 0;
  // Acerto ou erro.
  int total = 1;
  if (ap.permite_ataque_vs_defesa()) {
    d20 = RolaDado(20);
    bool acertou;
    resultado_ataque_e resultado_ataque;
    std::tie(resultado_ataque, total, resultado.texto, acertou) =
        ComputaAcertoOuErro(
            d20, bonus_ataque, modificador_incrementos, modificadores_ataque, d20_defesa, modificadores_defesa, DaParaTipoAtaque(da),
            ea.Proto(), ed.Proto());
    if (!acertou) {
      resultado.resultado = resultado_ataque;
      if (da.ataque_desarmar() && !ea.PossuiTalento("desarme_aprimorado")) {
        // Manter a ordem de defesa primeiro por causa dos testes.
        int d20_contra_defesa = RolaDado(20);
        int d20_contra = RolaDado(20);
        // Modificadores da defesa ja inclui o bonus, por isso passamos 0.
        auto [resultado_contra_ataque, total_contra, texto_contra, acertou_contra] = ComputaAcertoOuErro(
            d20_contra, /*bonus_ataque=*/modificadores_defesa, 0, 0, d20_contra_defesa, bonus_ataque + modificadores_ataque, TipoAtaque::DESARMAR,
            ed.Proto(), ea.Proto());
        VLOG(1) << "compilador feliz: " << total_contra << " e resultado " << resultado_contra_ataque;
        if (acertou_contra) {
          resultado.resultado = RA_FALHA_CONTRA_ATAQUE;
        }
        resultado.texto = absl::StrFormat("%s, contra ataque: %s", resultado.texto.c_str(), texto_contra.c_str());
      }
      return resultado;
    }
  }

  // Chance de falha.
  if (ea.Id() != ed.Id()) {
    bool passou_falha;
    std::tie(resultado.texto, passou_falha) = AtaqueVsChanceFalha(NI_NORMAL, da, ea, ed);
    if (!passou_falha) {
      resultado.resultado = RA_FALHA_CHANCE_FALHA;
      return resultado;
    }
  }

  // Imunidade: nao testa resistencia porque aqui eh so pra ver se acertou e como..
  if (EntidadeImuneElemento(ed.Proto(), ap.elemento())) {
    resultado.resultado = RA_FALHA_IMUNE;
    resultado.texto = absl::StrFormat("defensor imune a %s", TextoDescritor(ap.elemento()));
    return resultado;
  }

  // Se chegou aqui acertou.
  resultado.resultado = RA_SUCESSO;
  std::string texto_critico;
  std::tie(resultado.vezes, texto_critico) =
      AplicaCriticoSeDentroMargem(
          d20, bonus_ataque, modificador_incrementos, modificadores_ataque, modificadores_defesa, da, ea, ed);
  resultado.valor_final_com_modificadores = total;

  if (ea.PossuiEfeito(EFEITO_INVESTIDA)) {
    if (da.id_arma() == "lanca_montada" && ea.Proto().has_montado_em()) {
      ++resultado.vezes;
    }
    if (ea.PossuiTalento("investida_implacavel")) {
      ++resultado.vezes;
    }
  }

  if (ap.permite_ataque_vs_defesa()) {
    resultado.texto =
        absl::StrFormat("acertou: %d+%d%s%s= %d%s vs %s%s",
             d20, bonus_ataque, TextoOuNada(modificador_incrementos).c_str(), TextoOuNada(modificadores_ataque).c_str(), total,
             texto_critico.c_str(), StringValorDestino(DaParaTipoAtaque(da), d20_defesa, modificadores_defesa).c_str(),
             TextoOuNada(texto_toque_agarrar).c_str());
  } else {
    resultado.texto = "acertou (automático)";
  }
  VLOG(1) << "Resultado ataque vs defesa: " << resultado.texto << ", vezes: " << resultado.vezes;
  return resultado;
}

ResultadoAtaqueVsDefesa AtaqueVsDefesaAgarrar(const Entidade& ea, const Entidade& ed) {
  ResultadoAtaqueVsDefesa resultado;
  const auto* da_ataque = ea.DadoAgarrar();
  if (da_ataque == nullptr) {
    resultado.texto = "atacante sem acao agarrar";
    resultado.resultado = RA_SEM_ACAO;
    return resultado;
  }

  const auto* da_defesa = ed.DadoAgarrar();
  if (da_defesa == nullptr) {
    resultado.texto = "agarrar bem sucedido, defensor sem acao agarrar";
    resultado.resultado = RA_SUCESSO;
    return resultado;
  }
  int d20_ataque = RolaDado(20);
  int total_ataque = d20_ataque + da_ataque->bonus_ataque_final();
  int d20_defesa = RolaDado(20);
  int total_defesa = d20_defesa + da_defesa->bonus_ataque_final();
  resultado.resultado =
      total_ataque >= total_defesa
      ? RA_SUCESSO : RA_FALHA_NORMAL;
  resultado.texto = absl::StrFormat("agarrar %s: %d%+d %s %d%+d",
      resultado.Sucesso() ? "sucesso" : "falhou",
      d20_ataque, da_ataque->bonus_ataque_final(),
      resultado.Sucesso() ? ">=" : "<",
      d20_defesa, da_defesa->bonus_ataque_final());
  return resultado;
}

namespace {
ResultadoAtaqueVsDefesa AtaqueVsDefesaDerrubarInterno(const Entidade& ea, const Entidade& ed, bool permite_contra_ataque, bool eh_contra_ataque) {
  TamanhoEntidade tamanho_atacante = ea.Proto().tamanho();
  TamanhoEntidade tamanho_defensor = ed.Proto().tamanho();
  ResultadoAtaqueVsDefesa resultado;
  if (tamanho_defensor > tamanho_atacante + 1) {
    resultado.texto = "derrubar falhou: defensor muito grande";
    return resultado;
  }
  const int diferenca_tamanho = tamanho_atacante - tamanho_defensor;
  const int modificadores_ataque = ea.ModificadorAtributo(TA_FORCA) + 4 * diferenca_tamanho + (ea.PossuiTalento("derrubar_aprimorado") ? 4 : 0);
  const int modificadores_defesa =
      std::max(ed.ModificadorAtributo(TA_FORCA), ed.ModificadorAtributo(TA_DESTREZA)) +
      (ea.MaisDeDuasPernas() || Tabelas::Unica().Raca(ed.Proto().raca()).estabilidade() ? 4 : 0);
  const int dado_defesa = RolaDado(20);
  const int dado_ataque = RolaDado(20);
  if (dado_ataque + modificadores_ataque >= dado_defesa + modificadores_defesa) {
    resultado.resultado = RA_SUCESSO;
    resultado.texto = absl::StrFormat("%s sucesso: %d%+d >= %d%+d",
        (eh_contra_ataque ? "contra ataque" : "derrubar"), dado_ataque, modificadores_ataque, dado_defesa, modificadores_defesa);
  } else {
    if (eh_contra_ataque) {
      // Retorna o resultado do contra ataque.
      resultado.resultado = RA_FALHA_NORMAL;
      resultado.texto = absl::StrFormat("contra ataque falhou: %d%+d < %d%+d", dado_ataque, modificadores_ataque, dado_defesa, modificadores_defesa);
    } else if (permite_contra_ataque) {
      // Realiza o contra ataque
      ResultadoAtaqueVsDefesa resultado_contra_ataque = AtaqueVsDefesaDerrubarInterno(ed, ea, /*permite_contra_ataque=*/false, /*eh_contra_ataque=*/true);
      resultado.resultado = resultado_contra_ataque.Sucesso() ? RA_FALHA_CONTRA_ATAQUE : RA_FALHA_NORMAL;
      resultado.texto = absl::StrFormat(
           "derrubar falhou: %d%+d < %d%+d, %s",
           dado_ataque, modificadores_ataque, dado_defesa, modificadores_defesa, resultado_contra_ataque.texto.c_str());
    } else {
      // Falha normal.
      resultado.resultado = RA_FALHA_NORMAL;
      resultado.texto = absl::StrFormat(
           "derrubar falhou: %d%+d < %d%+d sem contra ataque",
           dado_ataque, modificadores_ataque, dado_defesa, modificadores_defesa);

    }
  }
  return resultado;
}
}  // namespace

ResultadoAtaqueVsDefesa AtaqueVsDefesaDerrubar(const Entidade& ea, const Entidade& ed, bool permite_contra_ataque) {
  return AtaqueVsDefesaDerrubarInterno(ea, ed, permite_contra_ataque, /*eh_contra_ataque=*/false);
}

const char* NomeSalvacao(TipoSalvacao ts) {
  switch (ts) {
    case TS_FORTITUDE: return "fortitude";
    case TS_REFLEXO: return "reflexo";
    case TS_VONTADE: return "vontade";
    default: return "desconhecido";
  }
}

// Retorna o delta pontos de vida e a string do resultado.
// A fracao eh para baixo mas com minimo de 1, segundo regra de rounding fractions, exception.
std::tuple<int, bool, std::string> AtaqueVsSalvacao(
    int delta_pontos_vida_entrada, const DadosAtaque& da, const Entidade& ea, const Entidade& ed) {
  int delta_pontos_vida = delta_pontos_vida_entrada;
  std::string descricao_resultado;
  bool salvou = false;

  if (ed.TemProximaSalvacao()) {
    if (ed.ProximaSalvacao() == RS_MEIO) {
      delta_pontos_vida = delta_pontos_vida == -1 ? -1 : delta_pontos_vida / 2;
      descricao_resultado = absl::StrFormat("salvacao manual 1/2: dano %d", -delta_pontos_vida);
      salvou = true;
    } else if (ed.ProximaSalvacao() == RS_QUARTO) {
      delta_pontos_vida /= 4;
      descricao_resultado = absl::StrFormat("salvacao manual 1/4: dano %d", -delta_pontos_vida);
      salvou = true;
    } else if (ed.ProximaSalvacao() == RS_ANULOU) {
      delta_pontos_vida = 0;
      descricao_resultado = "salvacao manual anulou";
      salvou = true;
    } else {
      descricao_resultado = "salvacao manual falhou";
    }
  } else if (da.has_dificuldade_salvacao()) {
    int d20 = RolaDado(20);
    int bonus = da.eh_feitico() ? ed.SalvacaoFeitico(ea, da) : ed.Salvacao(ea, da.tipo_salvacao());
    int total = d20 + bonus;
    std::string str_evasao;
    if (total >= da.dificuldade_salvacao()) {
      // Aqui salvou.
      salvou = true;
      if (da.resultado_ao_salvar() == RS_MEIO) {
        auto tipo_evasao = TipoEvasaoPersonagem(ed.Proto());
        if (da.tipo_salvacao() == TS_REFLEXO && (tipo_evasao == TE_EVASAO || tipo_evasao == TE_EVASAO_APRIMORADA)) {
          delta_pontos_vida = 0;
          str_evasao = " (evasão)";
        } else {
          delta_pontos_vida = delta_pontos_vida == -1 ? -1 : delta_pontos_vida / 2;
        }
      } else if (da.resultado_ao_salvar() == RS_QUARTO) {
        delta_pontos_vida /= 4;
      } else {
        delta_pontos_vida = 0;
      }
      descricao_resultado = absl::StrFormat(
          "salvacao %s sucesso: %d%+d >= %d, dano: %d%s", NomeSalvacao(da.tipo_salvacao()), d20, bonus, da.dificuldade_salvacao(), -delta_pontos_vida, str_evasao.c_str());
    } else {
      // Nao salvou.
      if (da.resultado_ao_salvar() == RS_MEIO && da.tipo_salvacao() == TS_REFLEXO && TipoEvasaoPersonagem(ed.Proto()) == TE_EVASAO_APRIMORADA) {
        delta_pontos_vida = delta_pontos_vida == -1 ? -1 : delta_pontos_vida / 2;
        str_evasao = " (evasão aprimorada)";
      }
      descricao_resultado = absl::StrFormat(
          "salvacao %s falhou: %d%+d < %d, dano: %d%s", NomeSalvacao(da.tipo_salvacao()), d20, bonus, da.dificuldade_salvacao(), -delta_pontos_vida, str_evasao.c_str());
    }
  } else {
    salvou = true;
    descricao_resultado = absl::StrFormat("salvacao: acao sem dificuldade, dano: %d", -delta_pontos_vida);
  }

  // A gente ainda precisa de fazer tudo acima por causa dos efeitos. Mas o dano pode ser aplicado normalmente.
  if (da.dano_ignora_salvacao()) {
    delta_pontos_vida = delta_pontos_vida_entrada;
    descricao_resultado = absl::StrFormat("%s; dano ignora salvacao: %d", descricao_resultado.c_str(), delta_pontos_vida);
  }
  return std::make_tuple(delta_pontos_vida, salvou, descricao_resultado);
}

std::tuple<bool, std::string> AtaqueVsResistenciaMagia(
    const Tabelas& tabelas, const DadosAtaque& da, const Entidade& ea, const Entidade& ed) {
  const int rm = ed.Proto().dados_defesa().resistencia_magia();
  if (rm == 0) {
    return std::make_tuple(true, "");;
  }
  const int d20 = RolaDado(20);
  const auto& feitico = tabelas.Feitico(da.id_arma());
  const int nivel_conjurador = da.has_nivel_conjurador_pergaminho()
    ? da.nivel_conjurador_pergaminho()
    : ea.NivelConjuradorParaMagia(ea.Proto().classe_feitico_ativa(), feitico);
  int mod = nivel_conjurador;
  if (PossuiTalento("magia_penetrante", ea.Proto())) {
    mod += 2;
  }
  if (PossuiTalento("magia_penetrante_maior", ea.Proto())) {
    mod += 2;
  }
  if (PossuiTalento("magia_perniciosa", ea.Proto()) && EscolaBoaTramaDasSombras(feitico) &&
      !PossuiTalento("magia_trama_sombras", ed.Proto())) {
    mod += 4;
  }
  const int total = d20 + mod;
  if (total < rm) {
    return std::make_tuple(false, absl::StrFormat("RM: ataque anulado; %d < %d (d20=%d, mod=%d)", total, rm, d20, mod));
  }
  return std::make_tuple(
      true, absl::StrFormat("RM: ataque bem sucedido; %d >= %d (d20=%d, mod=%d)", total, rm, d20, mod));
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
  return absl::StrFormat("%s%s, id: %d%s",
      !proto.modelo_3d().id().empty() ? proto.modelo_3d().id().c_str() : "",
      proto.modelo_3d().id().empty() && !proto.info_textura().id().empty() ? proto.info_textura().id().c_str() : "",
      proto.id(),
      proto.selecionavel_para_jogador() ? ", [selecionável]" : "");
}

std::string RotuloEntidade(const Entidade& entidade) {
  return RotuloEntidade(entidade.Proto());
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

namespace {
void PreencheCargasVarinha(bool decrementa, int limite_vezes, const ItemMagicoProto& varinha_tabelada, const Entidade& entidade, EntidadeProto* proto) {
  bool atualizou = false;
  for (const auto& vp : entidade.Proto().tesouro().varinhas()) {
    auto* varinha = proto->mutable_tesouro()->mutable_varinhas()->Add();
    *varinha = vp;
    if (atualizou || vp.id() != varinha_tabelada.id()) continue;
    varinha->set_cargas(limite_vezes - (decrementa ? 1 : 0));
  }
}
}  // namespace

void PreencheNotificacaoAtaqueAoPassarRodada(const Entidade& entidade, ntf::Notificacao* grupo, ntf::Notificacao* grupo_desfazer) {
  const auto& proto = entidade.Proto();
  ntf::Notificacao* np = nullptr;
  bool reusando = false;
  for (auto& n : *grupo->mutable_notificacao()) {
    if (n.entidade().id() == proto.id() && !n.entidade().dados_ataque().empty()) {
      if (np != nullptr) {
        LOG(ERROR) << "Grupo contem mais de uma notificacao de ataque!!i Ignorando a primeira.";
      }
      np = &n;
      reusando = true;
    }
  }
  EntidadeProto *e_antes, *e_depois;
  if (np != nullptr) {
    e_antes = np->mutable_entidade_antes();
    e_depois = np->mutable_entidade();
  } else {
    std::tie(np, e_antes, e_depois) = NovaNotificacaoFilha(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, proto, grupo);
    *e_antes->mutable_dados_ataque() = proto.dados_ataque();
    *e_depois->mutable_dados_ataque() = proto.dados_ataque();
  }
  for (auto& da : *e_depois->mutable_dados_ataque()) {
    if (da.usos() > 0 && !da.varinha().empty()) {
      const auto& varinha_tabelada = Tabelas::Unica().Varinha(da.varinha());
      da.set_usos(0);
      PreencheCargasVarinha(
          /*decrementa=*/true, da.limite_vezes(), varinha_tabelada, entidade, e_depois);
      PreencheCargasVarinha(
          /*decrementa=*/false, da.limite_vezes(), varinha_tabelada, entidade, e_antes);
    } else if (da.has_taxa_refrescamento() && da.usado_rodada() &&
               (!da.has_limite_vezes() || da.limite_vezes() == 0)) {
      // Trata o caso de ataques consumidos so ao fim da rodada.
      int valor = 0;
      try {
        valor = RolaValor(da.taxa_refrescamento());
      } catch (const std::exception& e) {
        LOG(ERROR) << "valor mal formado: " << da.taxa_refrescamento() << ", excecao: " << e.what();
        valor = 0;
      }
      da.set_disponivel_em(valor);
      da.clear_usado_rodada();
    }
    // Decrementa numero de rodadas que faltam para disponibilizar ataque.
    if (da.disponivel_em() > 0) {
      da.set_disponivel_em(std::max<int>(0, da.disponivel_em() - 1));
      if (da.disponivel_em() == 0 && da.has_limite_vezes_original()) {
        da.set_limite_vezes(da.limite_vezes_original());
      }
    }
  }
  if (grupo_desfazer != nullptr && !reusando) {
    *grupo_desfazer->add_notificacao() = *np;
  }
}

// Retorna o complemento para atributos e salvacao.
std::pair<int, int> ComplementoFuria(int nivel_barbaro) {
  if (nivel_barbaro == 0) {
    return {0, 0};
  } else if (nivel_barbaro < 11) {
    return {4, 2};
  } else if (nivel_barbaro < 20) {
    return {6, 3};
  } else {
    return {8, 4};
  }
}

void PreencheNotificacaoFadigaFuria(const Tabelas& tabelas, const Entidade& entidade, ntf::Notificacao* n_grupo, ntf::Notificacao* n_desfazer) {
 if (Nivel("barbaro", entidade.Proto()) >= 17) return;
  std::vector<int> ids_unicos = IdsUnicosEntidade(entidade);
  PreencheNotificacaoEventoSemComplemento(
      entidade.Id(), DadosIniciativaEntidade(entidade), "fadiga_furia_barbaro", EFEITO_FADIGA, 100,
      &ids_unicos, n_grupo->add_notificacao(), n_desfazer != nullptr ? n_desfazer->add_notificacao() : nullptr);
}

void PreencheNotificacaoEntrarFuria(const Tabelas& tabelas, const Entidade& entidade, ntf::Notificacao* n_grupo, ntf::Notificacao* n_desfazer) {
  auto [complemento, complemento_salvacao] = ComplementoFuria(Nivel("barbaro", entidade.Proto()));
  // Para ver novo modificador.
  Bonus bc = BonusAtributo(TA_CONSTITUICAO, entidade.Proto());
  AtribuiBonus(complemento, TB_MORAL, "furia_barbaro", &bc);
  const int rodadas = 3 + ModificadorAtributo(bc);
  std::vector<int> ids_unicos = IdsUnicosEntidade(entidade);
  PreencheNotificacaoEventoComComplementos(
      entidade.Id(), DadosIniciativaEntidade(entidade), "furia_barbaro", EFEITO_FURIA_BARBARO, {complemento, complemento_salvacao}, rodadas,
      &ids_unicos, n_grupo->add_notificacao(), n_desfazer != nullptr ? n_desfazer->add_notificacao() : nullptr);
}


bool PreencheNotificacaoAlternarFuria(const Tabelas& tabelas, const Entidade& entidade, ntf::Notificacao* n_grupo, ntf::Notificacao* n_desfazer) {
  const int nivel_barbaro = Nivel("barbaro", entidade.Proto());
  if (nivel_barbaro < 1) return false;
  bool possui_furia = entidade.PossuiEfeito(EFEITO_FURIA_BARBARO);
  // TODO verificar usos.
  if (possui_furia) {
    PreencheNotificacaoRemocaoEvento(
        entidade.Proto(), EFEITO_FURIA_BARBARO, n_grupo->add_notificacao(),
        n_desfazer != nullptr ? n_desfazer->add_notificacao() : nullptr);
    PreencheNotificacaoFadigaFuria(tabelas, entidade, n_grupo, n_desfazer);
  } else {
    PreencheNotificacaoEntrarFuria(tabelas, entidade, n_grupo, n_desfazer);
  }
  return !possui_furia;
}

void PreencheNotificacaoFormaAlternativa(const Tabelas& tabelas, const EntidadeProto& proto, ntf::Notificacao* n_grupo, ntf::Notificacao* n_desfazer) {
  if (proto.formas_alternativas_size() < 2) {
    LOG(INFO) << "Nao foi possivel alterar forma, entidade " << RotuloEntidade(proto) << "nao possui formas alternativas.";
    return;
  }

  auto [n, e_antes, e_depois] = NovaNotificacaoFilha(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, proto, n_grupo);

  int indice = proto.forma_alternativa_corrente();
  int proximo_indice = (indice + 1) % proto.formas_alternativas_size();
  {
    e_antes->MergeFrom(ProtoFormaAlternativa(proto));
    e_antes->set_forma_alternativa_corrente(indice);
    *e_antes->mutable_formas_alternativas() = proto.formas_alternativas();
    //PreencheComTesourosEmUso(proto, /*manter_uso=*/true, e_antes);
  }
  {
    const auto& nova_forma = proto.formas_alternativas(proximo_indice);
    if (nova_forma.has_id_forma_alternativa()) {
      auto& modelo = tabelas.ModeloEntidade(nova_forma.id_forma_alternativa());
      e_depois->MergeFrom(ProtoFormaAlternativa(modelo.entidade()));
    } else {
      e_depois->MergeFrom(ProtoFormaAlternativa(nova_forma));
    }
    e_depois->set_forma_alternativa_corrente(proximo_indice);
    *e_depois->mutable_formas_alternativas() = proto.formas_alternativas();
    // Salva os dados da forma corrente na forma alternativa.
    *e_depois->mutable_formas_alternativas(indice) = ProtoFormaAlternativa(proto);
    VLOG(1) << "Alterando para forma " << proximo_indice
            << ", entidade " << RotuloEntidade(proto) << ", proto: " << e_depois->DebugString();
  }
  if (n_desfazer != nullptr) {
    *n_desfazer->add_notificacao() = *n;
  }
  if (Nivel("druida", proto) >= 5 && proximo_indice != 0) {
    int nivel = NivelPersonagem(proto);
    PreencheNotificacaoAtualizacaoPontosVida(proto, nivel, TD_LETAL, n_grupo->add_notificacao(), n_desfazer != nullptr ? n_desfazer->add_notificacao() : nullptr);
  }
}

void PreencheNotificacaoAtualizacaoPontosVida(
    const Entidade& entidade, int delta_pontos_vida, tipo_dano_e td, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  PreencheNotificacaoAtualizacaoPontosVida(entidade.Proto(), delta_pontos_vida, td, n, n_desfazer);
}

// O delta de pontos de vida afeta outros bits tambem.
void PreencheNotificacaoAtualizacaoPontosVida(
    const EntidadeProto& proto, int delta_pontos_vida, tipo_dano_e td, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  EntidadeProto *e_antes, *e_depois;
  std::tie(e_antes, e_depois) = PreencheNotificacaoEntidadeProto(
      ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, proto, n);

  // Aqui vamos tratar nao letais e temporarios. Depois tratamos os pontos de vida mesmo.
  int temporarios = proto.pontos_vida_temporarios();
  int dano_nao_letal = proto.dano_nao_letal();
  if (delta_pontos_vida > 0) {
    if (delta_pontos_vida >= e_depois->dano_nao_letal()) {
      e_depois->set_dano_nao_letal(0);
      dano_nao_letal = 0;
    } else {
      dano_nao_letal = proto.dano_nao_letal() - delta_pontos_vida;
      e_depois->set_dano_nao_letal(dano_nao_letal);
    }
  } else if (delta_pontos_vida < 0) {
    if (td == TD_NAO_LETAL) {
      dano_nao_letal -= delta_pontos_vida;
      e_depois->set_dano_nao_letal(dano_nao_letal);
    } else if (proto.pontos_vida_temporarios() > 0) {
      temporarios = 0;
      // Tira dos temporarios.
      *e_depois->mutable_pontos_vida_temporarios_por_fonte() = proto.pontos_vida_temporarios_por_fonte();
      auto* bpv = e_depois->mutable_pontos_vida_temporarios_por_fonte();
      auto* bi = BonusIndividualSePresente(TB_SEM_NOME, bpv);
      if (bi != nullptr) {
        // Tem que loopar tudo pra computar o novo valor de temporarios.
        for (int i_origem = 0; i_origem < bi->por_origem_size(); ++i_origem)  {
          auto* po = bi->mutable_por_origem(i_origem);
          if (po->valor() < 0) {
            LOG(WARNING) << "Valor de pv temporario invalido: " << po->valor();
          } else if (po->valor() >= abs(delta_pontos_vida)) {
            // Anulou o dano todo, mas continua pra computar temporarios.
            int novo_valor = po->valor() - abs(delta_pontos_vida);
            delta_pontos_vida = 0;
            po->set_valor(novo_valor);
            temporarios += novo_valor;
          } else {
            // Sobrou dano.
            delta_pontos_vida += po->valor();
            po->set_valor(0);
          }
        }
      }
    }
  }
  // Agora os pontos de vida. Se o dano foi nao letal, nada a fazer (foi feito acima).
  // Caso contrario, aplica aqui.
  const int pv = proto.pontos_vida() + (td == TD_NAO_LETAL ? 0 : delta_pontos_vida);
  e_depois->set_pontos_vida(std::min(pv, proto.max_pontos_vida()));
  e_depois->set_dano_nao_letal(dano_nao_letal);
  PreencheNotificacaoConsequenciaAlteracaoPontosVida(
      pv + temporarios, dano_nao_letal, proto, n);

  if (n_desfazer != nullptr) {
    n_desfazer->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL);
    *n_desfazer->mutable_entidade() = *e_depois;
    *n_desfazer->mutable_entidade_antes() = *e_antes;
    e_antes->set_pontos_vida(proto.pontos_vida());
    e_antes->set_dano_nao_letal(proto.dano_nao_letal());
    *e_antes->mutable_pontos_vida_temporarios_por_fonte() = proto.pontos_vida_temporarios_por_fonte();
  }
}

void PreencheNotificacaoCuraAcelerada(const Entidade& entidade, ntf::Notificacao* n) {
  const auto& proto = entidade.Proto();
  int cura = CuraAcelerada(proto);
  if (cura == 0) {
    return;
  }

  EntidadeProto *entidade_antes, *entidade_depois;
  std::tie(entidade_antes, entidade_depois) = PreencheNotificacaoEntidade(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, entidade, n);

  if (proto.dano_nao_letal() > 0) {
    if (cura >= proto.dano_nao_letal()) {
      // Cura tirou tudo e pode ate sobrar.
      cura -= proto.dano_nao_letal();
      entidade_depois->set_dano_nao_letal(0);
      VLOG(2) << "curando todo dano nao letal, sobra: " << cura;
    } else {
      // Cura so dano nao letal.
      entidade_depois->set_dano_nao_letal(proto.dano_nao_letal() - cura);
      cura = 0;
      VLOG(2) << "curando apenas dano nao letal";
    }
    entidade_antes->set_dano_nao_letal(proto.dano_nao_letal());
  }
  const int dano = entidade.MaximoPontosVida() - entidade.PontosVida();
  if (dano > 0 && cura > 0) {
    // Ainda sobrou cura, cura parte letal.
    if (cura >= dano) {
      entidade_depois->set_pontos_vida(entidade.MaximoPontosVida());
      VLOG(2) << "curando todo dano letal";
    } else {
      entidade_depois->set_pontos_vida(proto.pontos_vida() + cura);
      VLOG(2) << "curando parte do dano letal: " << cura;
    }
    entidade_antes->set_pontos_vida(proto.pontos_vida());
  }
}


std::pair<EntidadeProto*, EntidadeProto*> PreencheNotificacaoEntidade(
    ntf::Tipo tipo, const Entidade& entidade, ntf::Notificacao* n) {
  return PreencheNotificacaoEntidadeProto(tipo, entidade.Proto(), n);
}

std::pair<EntidadeProto*, EntidadeProto*> PreencheNotificacaoEntidadeProto(
    ntf::Tipo tipo, const EntidadeProto& proto, ntf::Notificacao* n) {
  return PreencheNotificacaoEntidadeComId(tipo, proto.id(), n);
}

std::pair<EntidadeProto*, EntidadeProto*> PreencheNotificacaoEntidadeComId(
    ntf::Tipo tipo, unsigned int id_entidade, ntf::Notificacao* n) {
  n->set_tipo(tipo);
  n->mutable_entidade_antes()->set_id(id_entidade);
  n->mutable_entidade()->set_id(id_entidade);
  return std::make_pair(n->mutable_entidade_antes(), n->mutable_entidade());
}


namespace {

bool AtaqueIgual(const DadosAtaque& lda, const DadosAtaque& rda) {
  return lda.rotulo() == rda.rotulo() &&
         lda.tipo_ataque() == rda.tipo_ataque() &&
         lda.grupo() == rda.grupo() &&
         lda.ordem_ataque() == rda.ordem_ataque();
}

}  // namespace

// Encontra determinado dado de ataque em um proto. Retorna nullptr caso nao encontre.
DadosAtaque* EncontraAtaque(const DadosAtaque& da, EntidadeProto* proto) {
  for (auto& pda : *proto->mutable_dados_ataque()) {
    if (AtaqueIgual(pda, da)) {
      return &pda;
    }
  }
  return nullptr;
}

std::pair<std::string, std::string> UltimaAcaoGrupo(const EntidadeProto& proto) {
  std::string ultima_acao = proto.ultima_acao();
  std::string ultimo_grupo = proto.ultimo_grupo_acao();
  if (ultima_acao.empty() && ultimo_grupo.empty()) {
    VLOG(3) << "sobrescrevendo ultima acao e grupo";
    ultima_acao = proto.dados_ataque().empty() ? "Ataque Corpo a Corpo" : proto.dados_ataque(0).tipo_ataque();
    ultimo_grupo = proto.dados_ataque().empty() ? "" : proto.dados_ataque(0).grupo();
  }

  VLOG(3) << "ultima_acao: " << ultima_acao << ", ultimo_grupo: " << ultimo_grupo;
  return {ultima_acao, ultimo_grupo};
}

const DadosAtaque* DadoCorrente(const EntidadeProto& proto) {
  auto [ultima_acao, ultimo_grupo] = UltimaAcaoGrupo(proto);
  for (const auto& da : proto.dados_ataque()) {
    if ((ultima_acao.empty() || da.tipo_ataque() == ultima_acao) && da.grupo() == ultimo_grupo) {
      return &da;
    }
  }
  return nullptr;
}

const DadosAtaque& DadoCorrenteNaoNull(const EntidadeProto& proto) {
  const auto* da = DadoCorrente(proto);
  return da == nullptr ? DadosAtaque::default_instance() : *da;
}

const std::vector<std::string>& ItemsQueGeramAtaques() {
  static const std::vector<std::string> v = {
    "fogo_alquimico", "agua_benta", "acido", "pedra_trovao", "bolsa_cola", "gas_alquimico_sono", "bomba" };
  return v;
}

bool AtaqueDeItemMundano(const DadosAtaque& da) {
  return c_any<std::vector<std::string>>(ItemsQueGeramAtaques(), da.id_arma()) || c_any<std::vector<std::string>>(ItemsQueGeramAtaques(), da.grupo());
}

void PreencheConsumoItemMundano(const std::string& id_item, const Entidade& entidade, EntidadeProto* proto) {
  bool removeu = false;
  for (auto& item : entidade.Proto().tesouro().itens_mundanos()) {
    if (item.id() != id_item || removeu) {
      *proto->mutable_tesouro()->mutable_itens_mundanos()->Add() = item;
    } else if (item.id() == id_item) {
      removeu = true;
    }
  }
  if (proto->tesouro().itens_mundanos().empty()) {
    // Cria vazio para remover.
    proto->mutable_tesouro()->add_itens_mundanos();
  }
}

void PreencheNotificacaoConsumoAtaque(
    const Entidade& entidade, const DadosAtaque& da, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  EntidadeProto *proto = nullptr, *proto_antes = nullptr;
  std::tie(proto_antes, proto) =
      PreencheNotificacaoEntidade(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, entidade, n);
  *proto_antes->mutable_dados_ataque() = entidade.Proto().dados_ataque();
  *proto->mutable_dados_ataque() = entidade.Proto().dados_ataque();
  auto* da_antes = EncontraAtaque(da, proto_antes);
  auto* da_depois = EncontraAtaque(da, proto);
  if (da_depois != nullptr) {
    if (da.requer_carregamento()) {
      da_depois->set_descarregada(true);
    }
    if (da_depois->has_limite_vezes()) {
      if (da_depois->has_consome_apos_n_usos() && (da_antes->usos() + 1) < da_antes->consome_apos_n_usos()) {
        da_depois->set_usos(da_antes->usos() + 1);
      } else {
        if (da_depois->has_consome_apos_n_usos()) {
          da_depois->set_usos(0);
        }
        if (!da_depois->varinha().empty()) {
          const auto& varinha_tabelada = Tabelas::Unica().Varinha(da_depois->varinha());
          PreencheCargasVarinha(
              /*decrementa=*/true, da_depois->limite_vezes(), varinha_tabelada, entidade, proto);
          PreencheCargasVarinha(
              /*decrementa=*/false, da_depois->limite_vezes(), varinha_tabelada, entidade, proto_antes);
        } else {
          da_depois->set_limite_vezes(std::max<int>(0, da_depois->limite_vezes() - 1));
        }
      }
    }
    if (da_depois->has_municao()) {
      if (AtaqueDeItemMundano(*da_depois)) {
        const std::string id_arma = da_depois->id_arma();
        PreencheConsumoItemMundano(da_depois->id_arma(), entidade, proto);
        // Hack: aqui so para criar vazio se num tiver nada. Nao vai remover nada mesmo.
        PreencheConsumoItemMundano("xxx_id_invalido_xxx", entidade, proto_antes);
      } else {
        da_depois->set_municao(std::max((int)(da_depois->municao() - 1), 0));
      }
    }
    if (!da_depois->taxa_refrescamento().empty()) {
      if (da_depois->refresca_apos_rodada()) {
        // Marca como usado para refrescar apos rodada.
        da_depois->set_usado_rodada(true);
        VLOG(1) << "refrescar apos rodada";
      } else if (!da_depois->has_limite_vezes() || da_depois->limite_vezes() == 0) {
        // Ja atualiza.
        int valor = 0;
        try {
          valor = RolaValor(da_depois->taxa_refrescamento());
        } catch (const std::exception& e) {
          LOG(ERROR) << "valor mal formado: " << da_depois->taxa_refrescamento() << ", excecao: " << e.what();
          valor = 0;
        }
        da_depois->set_disponivel_em(valor);
        da_antes->set_disponivel_em(0);
        VLOG(1) << "refrescando valor para " << valor;
      }
    }
  }
  if (n_desfazer != nullptr) {
    *n_desfazer  = *n;
  }
}

void PreencheNotificacaoRecarregamento(
    const Entidade& entidade, const DadosAtaque& da, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
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

void PreencheNotificacaoEventoSemComplemento(
    unsigned int id_entidade, const std::optional<DadosIniciativa>& dados_iniciativa, const std::string& origem, TipoEfeito tipo_efeito, int rodadas, std::vector<int>* ids_unicos,
    ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  EntidadeProto *e_antes, *e_depois;
  std::tie(e_antes, e_depois) = PreencheNotificacaoEntidadeComId(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, id_entidade, n);
  auto* evento = AdicionaEvento(dados_iniciativa, origem, tipo_efeito, rodadas, false, ids_unicos, e_depois);
  auto* evento_antes = e_antes->add_evento();
  *evento_antes = *evento;
  evento_antes->set_rodadas(-1);
  if (n_desfazer != nullptr) {
    *n_desfazer = *n;
  }
}

void PreencheNotificacaoEventoEfeitoAdicionalComAtaque(
    unsigned int id_origem, const std::optional<DadosIniciativa>& dados_iniciativa, const DadosAtaque& da, int nivel_conjurador, const Entidade& entidade_destino, const EfeitoAdicional& efeito_adicional,
    std::vector<int>* ids_unicos, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  EntidadeProto *e_antes, *e_depois;
  std::tie(e_antes, e_depois) = PreencheNotificacaoEntidade(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, entidade_destino, n);
  auto* evento = AdicionaEventoEfeitoAdicional(id_origem, dados_iniciativa, nivel_conjurador, efeito_adicional, da.acao(), ids_unicos, entidade_destino, e_depois);
  auto* evento_antes = e_antes->add_evento();
  *evento_antes = *evento;
  evento_antes->set_rodadas(-1);
  evento->set_tipo_salvacao(da.tipo_salvacao());
  evento->set_dificuldade_salvacao(da.dificuldade_salvacao());
  if (n_desfazer != nullptr) {
    *n_desfazer = *n;
  }
}

void PreencheNotificacaoEventoComComplementos(
    unsigned int id_entidade, const std::optional<DadosIniciativa>& dados_iniciativa, const std::string& origem, TipoEfeito tipo_efeito, const std::vector<int>& complementos, int rodadas,
    std::vector<int>* ids_unicos, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  auto [e_antes, e_depois] = PreencheNotificacaoEntidadeComId(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, id_entidade, n);
  auto* evento = AdicionaEvento(dados_iniciativa, /*origem=*/origem, tipo_efeito, rodadas, /*continuo=*/false, ids_unicos, e_depois);
  for (int c : complementos) {
    evento->add_complementos(c);
  }
  auto* evento_antes = e_antes->add_evento();
  *evento_antes = *evento;
  evento_antes->set_rodadas(-1);
  if (n_desfazer != nullptr) {
    *n_desfazer = *n;
  }
}


void PreencheNotificacaoEventoComComplementoStr(
    unsigned int id_entidade, const std::optional<DadosIniciativa>& dados_iniciativa, const std::string& origem, TipoEfeito tipo_efeito, const std::string& complemento_str, int rodadas,
    std::vector<int>* ids_unicos, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  auto [e_antes, e_depois] = PreencheNotificacaoEntidadeComId(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, id_entidade, n);
  auto* evento = AdicionaEvento(dados_iniciativa, /*origem=*/origem, tipo_efeito, rodadas, /*continuo=*/false, ids_unicos, e_depois);
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

bool PreencheNotificacaoEventoParaVenenoComum(
    unsigned int id_entidade, const std::optional<DadosIniciativa>& dados_iniciativa, const VenenoProto& veneno, int rodadas,
    std::vector<int>* ids_unicos, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  // A origem ficara veneno: %d, pois veneno é cumulativo.
  std::string origem = absl::StrFormat("%d", AchaIdUnicoEvento(*ids_unicos));
  if (veneno.sono()) {
    PreencheNotificacaoEventoSemComplemento(id_entidade, dados_iniciativa, origem, EFEITO_SONO, DIA_EM_RODADAS, ids_unicos, n, n_desfazer);
  } else {
    PreencheNotificacaoEventoSemComplemento(id_entidade, dados_iniciativa, origem, EFEITO_DANO_ATRIBUTO_VENENO, DIA_EM_RODADAS, ids_unicos, n, n_desfazer);
  }
  if (n->entidade().evento_size() != 1) {
    LOG(ERROR) << "Falha criando veneno: tamanho de evento invalido, " << n->entidade().evento_size();
    return false;
  }
  return true;
}
}  // namespace

void PreencheNotificacaoEventoParaVenenoPrimario(
    unsigned int id_entidade, const std::optional<DadosIniciativa>& dados_iniciativa, const VenenoProto& veneno,
    std::vector<int>* ids_unicos, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  if (!PreencheNotificacaoEventoParaVenenoComum(id_entidade, dados_iniciativa, veneno, /*rodadas=*/veneno.sono() ? 10 : DIA_EM_RODADAS, ids_unicos, n, n_desfazer)) return;
  if (!veneno.sono()) {
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
}

void PreencheNotificacaoEventoParaVenenoSecundario(
    unsigned int id_entidade, const std::optional<DadosIniciativa>& dados_iniciativa, const VenenoProto& veneno,
    std::vector<int>* ids_unicos, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  if (!PreencheNotificacaoEventoParaVenenoComum(id_entidade, dados_iniciativa, veneno, /*rodadas=*/veneno.sono() ? RolaDado(4) * MINUTOS_PARA_RODADAS : DIA_EM_RODADAS, ids_unicos, n, n_desfazer)) return;
  if (!veneno.sono()) {
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
  const int modificador_destreza = DestrezaNaCA(proto) ? 0 : ModificadorAtributo(proto.atributos().destreza());
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
#if 0
// TODO isso num ta muito certo. Tem armas que nao sao de distancia (lanca) que tem alcance > 0. Melhor deixar so as categorias.
bool ArmaDistancia(const ArmaProto& arma) {
  const bool distancia =
      c_any_of(arma.categoria(), [](int cat) { return cat == CAT_DISTANCIA; }) ||
      c_any_of(arma.categoria(), [](int cat) { return cat == CAT_ARREMESSO; });
  return distancia || arma.alcance_quadrados() > 0;
}
#endif

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

std::string ChaveMapaPorOrigem(const BonusIndividual::PorOrigem& por_origem) {
  if (por_origem.valor() >= 0) {
    return absl::StrFormat("%s:positivo", por_origem.origem().c_str());
  } else {
    return absl::StrFormat("%s:negativo", por_origem.origem().c_str());
  }
}

// Retorna o total de um bonus individual, contabilizando acumulo caso as origens sejam diferentes.
int BonusIndividualTotal(const BonusIndividual& bonus_individual) {
  if (BonusCumulativo(bonus_individual.tipo())) {
    std::unordered_map<std::string, int> mapa_por_origem;
    for (const auto& por_origem : bonus_individual.por_origem()) {
      std::string chave = ChaveMapaPorOrigem(por_origem);
      auto it = mapa_por_origem.find(chave);
      if (it == mapa_por_origem.end()) {
        // Origem nao existe no mapa, pode usar.
        mapa_por_origem[chave] = por_origem.valor();
      } else {
        // Origem existe no mapa. Usa o maior para positivos, menor para negativos.
        int valor = por_origem.valor();
        if ((valor < 0 && valor < it->second) || (valor >= 0 && valor > it->second)) {
          mapa_por_origem[chave] = por_origem.valor();
        }
      }
    }
    int total = 0;
    for (const auto& par : mapa_por_origem) {
      total += par.second;
    }
    return total;
  } else {
    // Bonus nao cumulativo, nao importa origem, apenas o maior e menor se aplicam.
    int maior = 0;
    int menor = 0;
    for (const auto& por_origem : bonus_individual.por_origem()) {
      if (por_origem.valor() > 0) {
        // Bonus.
        maior = std::max(maior, por_origem.valor());
      } else if (por_origem.valor() < 0) {
        // Penalidade.
        menor = std::min(menor, por_origem.valor());
      }
    }
    // Atencao, menor é negativo, entao aqui deve ser soma.
    return maior + menor;
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

bool PossuiCategoria(CategoriaArma categoria, const ArmaProto& arma) {
  return c_any(arma.categoria(), categoria);
}

bool ClassePossuiSalvacaoForte(TipoSalvacao ts, const InfoClasse& ic) {
  return c_any(ic.salvacoes_fortes(), ts);
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
    if (c_any_of(bonus_excluidos, [&bi](ent::TipoBonus tipo) { return tipo == bi.tipo(); })) {
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

void AtribuiOuRemoveBonus(int valor, TipoBonus tipo, const std::string& origem, Bonus* bonus) {
  if (valor != 0) {
    AtribuiBonus(valor, tipo, origem, bonus);
  } else {
    RemoveBonus(tipo, origem, bonus);
  }
}

void LimpaBonus(const Bonus& bonus_a_remover, Bonus* bonus) {
  for (const auto& bi : bonus_a_remover.bonus_individual()) {
    for (const auto& po : bi.por_origem()) {
      LimpaBonus(bi.tipo(), po.origem(), bonus);
    }
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

BonusIndividual::PorOrigem* AtribuiBonusPenalidadeIndividualSeMaior(int valor, const std::string& origem, BonusIndividual* bonus_individual) {
  for (auto& por_origem : *bonus_individual->mutable_por_origem()) {
    if (por_origem.origem() == origem) {
      if ((valor > 0 && valor > por_origem.valor()) || (valor < 0 && valor < por_origem.valor())) {
        por_origem.set_valor(valor);
      }
      return &por_origem;
    }
  }
  auto* po = bonus_individual->add_por_origem();
  po->set_valor(valor);
  po->set_origem(origem);
  return po;
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

BonusIndividual::PorOrigem* AtribuiBonusPenalidadeSeMaior(int valor, TipoBonus tipo, const std::string& origem, Bonus* bonus) {
  for (auto& bi : *bonus->mutable_bonus_individual()) {
    if (bi.tipo() == tipo) {
      return AtribuiBonusPenalidadeIndividualSeMaior(valor, origem, &bi);
    }
  }
  auto* bi = bonus->add_bonus_individual();
  bi->set_tipo(tipo);
  return AtribuiBonusIndividual(valor, origem, bi);
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

int ModificadorAtributoOriginal(TipoAtributo ta, const EntidadeProto& proto) {
  const auto& bonus = BonusAtributo(ta, proto);
  int valor_base = BonusIndividualTotal(TB_BASE, bonus);
  int valor_racial = BonusIndividualTotal(TB_RACIAL, bonus);
  return ModificadorAtributo(valor_base + valor_racial);
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

int ModificadorTamanhoEsconderse(TamanhoEntidade tamanho) {
  return -ModificadorTamanhoAgarrar(tamanho);
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

std::string ConverteDanoBasicoMedioParaTamanho(const std::string& dano_basico_medio, TamanhoEntidade tamanho) {
  // Chaveado por dano medio.
  const static std::unordered_map<std::string, std::unordered_map<int, std::string>> mapa_danos {
    { "1d2", {
      { TM_MINUSCULO, "" }, { TM_DIMINUTO, "" }, { TM_MIUDO, ""}, { TM_PEQUENO, "1" }, { TM_GRANDE, "1d3" }, { TM_ENORME, "1d4" }, { TM_IMENSO, "1d6" }, { TM_COLOSSAL, "1d8" }
    } },
    { "1d3", {
      { TM_MINUSCULO, "" }, { TM_DIMINUTO, "" }, { TM_MIUDO, "1" }, { TM_PEQUENO, "1d2" }, { TM_GRANDE, "1d4" }, { TM_ENORME, "1d6" }, { TM_IMENSO, "1d8" }, { TM_COLOSSAL, "2d6" }
    } },
    { "1d4", {
      { TM_MINUSCULO, "" }, { TM_DIMINUTO, "1" }, { TM_MIUDO, "1d2" }, { TM_PEQUENO, "1d3" }, { TM_GRANDE, "1d6" }, { TM_ENORME, "1d8" }, { TM_IMENSO, "2d6" },
      { TM_COLOSSAL, "3d6" }
    } },
    { "1d6", {
      { TM_MINUSCULO, "1" }, { TM_DIMINUTO, "1d2" },  { TM_MIUDO, "1d3" }, { TM_PEQUENO, "1d4" }, { TM_GRANDE, "1d8" }, { TM_ENORME, "2d6" },
      { TM_IMENSO, "3d6" }, { TM_COLOSSAL, "4d6" }
    } },
    { "1d8", {
      { TM_MINUSCULO, "1d2" }, { TM_DIMINUTO, "1d3" },  { TM_MIUDO, "1d4" }, { TM_PEQUENO, "1d6" },
      { TM_GRANDE, "2d6" }, { TM_ENORME, "3d6" }, { TM_IMENSO, "4d6" }, { TM_COLOSSAL, "6d6" }
    } },
    { "1d10", {
      { TM_MINUSCULO, "1d3" }, { TM_DIMINUTO, "1d4" },  { TM_MIUDO, "1d6" }, { TM_PEQUENO, "1d8" },
      { TM_GRANDE, "2d8" }, { TM_ENORME, "3d8" }, { TM_IMENSO, "4d8" }, { TM_COLOSSAL, "6d8" }
    } },
    { "1d12", {
      { TM_MINUSCULO, "1d4" }, { TM_DIMINUTO, "1d6" },  { TM_MIUDO, "1d8" }, { TM_PEQUENO, "1d10" },
      { TM_GRANDE, "3d6" }, { TM_ENORME, "4d6" }, { TM_IMENSO, "6d6" }, { TM_COLOSSAL, "8d6" }
    } },
    { "2d4", {
      { TM_MINUSCULO, "1d2" }, { TM_DIMINUTO, "1d3" },  { TM_MIUDO, "1d4" }, { TM_PEQUENO, "1d6" },
      { TM_GRANDE, "2d6" }, { TM_ENORME, "3d6" }, { TM_IMENSO, "4d6" }, { TM_COLOSSAL, "6d6" }
    } },
    { "2d6", {
      { TM_MINUSCULO, "1d4" }, { TM_DIMINUTO, "1d6" },  { TM_MIUDO, "1d8" }, { TM_PEQUENO, "1d10" },
      { TM_GRANDE, "3d6" }, { TM_ENORME, "4d6" }, { TM_IMENSO, "6d6" }, { TM_COLOSSAL, "8d6" }
    } },
    { "2d8", {
      { TM_MINUSCULO, "1d6" }, { TM_DIMINUTO, "1d8" },  { TM_MIUDO, "1d10" }, { TM_PEQUENO, "2d6" },
      { TM_GRANDE, "3d8" }, { TM_ENORME, "4d8" }, { TM_IMENSO, "6d8" }, { TM_COLOSSAL, "8d8" }
    } },
    { "2d10", {
      { TM_MINUSCULO, "1d8" }, { TM_DIMINUTO, "1d10" },  { TM_MIUDO, "2d6" }, { TM_PEQUENO, "2d8" },
      { TM_GRANDE, "4d8" }, { TM_ENORME, "6d8" }, { TM_IMENSO, "8d8" }, { TM_COLOSSAL, "12d8" }
    } }
  };
  if (tamanho == TM_MEDIO) {
    return dano_basico_medio;
  }
  auto it = mapa_danos.find(dano_basico_medio);
  if (it == mapa_danos.end()) {
    if (!dano_basico_medio.empty()) {
      LOG(ERROR) << absl::StrFormat("Dano basico medio nao tabelado: %s", dano_basico_medio.c_str());
    }
    return "";
  }
  auto it2 = it->second.find(tamanho);
  if (it2 == it->second.end()) {
    LOG(ERROR) << "Tamanho invalido (" << tamanho << ") para dano medio " << dano_basico_medio.c_str();
    return "";
  }
  return it2->second;
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

bool PossuiEvento(TipoEfeito tipo, const EntidadeProto& proto) {
  return c_any_of(proto.evento(), [tipo] (const EntidadeProto::Evento& evento) {
    return evento.id_efeito() == tipo;
  });
}

bool PossuiUmDosEventos(const std::vector<TipoEfeito>& tipos, const EntidadeProto& proto) {
  return c_any_of(proto.evento(), [&tipos] (const EntidadeProto::Evento& evento) {
    return c_any(tipos, evento.id_efeito());
  });
}

bool PossuiEvento(TipoEfeito tipo, const std::string& complemento, const EntidadeProto& entidade) {
  return c_any_of(entidade.evento(), [tipo, &complemento] (const EntidadeProto::Evento& evento) {
    return
        evento.id_efeito() == tipo &&
        c_any(evento.complementos_str(), complemento);
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
  for (int i = 0; i < lhs.complementos_size(); ++i) {
    if (lhs.complementos(i) != rhs.complementos(i)) return false;
  }
  return true;
}
}  // namespace

bool PossuiEventoEspecifico(const EntidadeProto& entidade, const EntidadeProto::Evento& evento) {
  return c_any_of(entidade.evento(), [&evento] (const EntidadeProto::Evento& evento_entidade) {
    if (evento.has_id_unico()) {
      return evento.id_unico() == evento_entidade.id_unico();
    }
    return EventosIguaisIgnorandoDuracao(evento, evento_entidade);
  });
}

bool PossuiResistenciaEspecifica(const EntidadeProto& entidade, const ResistenciaElementos& resistencia) {
  return c_any_of(entidade.dados_defesa().resistencia_elementos(),
      [&resistencia] (const ResistenciaElementos& resistencia_entidade) {
      return resistencia.valor() == resistencia_entidade.valor() && resistencia.descritor() == resistencia_entidade.descritor();
    });
}

const std::string IdParaMagia(const Tabelas& tabelas, const std::string& id_classe) {
  const auto& classe_tabelada = tabelas.Classe(id_classe);
  return classe_tabelada.has_id_para_magia() ? classe_tabelada.id_para_magia() : id_classe;
}

int NivelConjurador(const std::string& id_classe, const EntidadeProto& proto) {
  const auto& ic = InfoClasseProtoParaMagia(id_classe, proto);
  VLOG(2) << "classe para nivel de conjurador: " << ic.DebugString();
  if (ic.has_nivel_conjurador()) {
    return ic.nivel_conjurador();
  } else {
    return ic.nivel();
  }
}

int NivelConjuradorParaMagia(const std::string& id_classe, const ArmaProto& feitico_tabelado, const EntidadeProto& proto) {
  int nivel = NivelConjurador(id_classe, proto);
  if (PossuiTalento("magia_trama_sombras", proto)) {
    if (EscolaBoaTramaDasSombras(feitico_tabelado)) {
      ++nivel;
    } else if (EscolaRuimTramaDasSombras(feitico_tabelado)) {
      if (feitico_tabelado.acao().elemento() != DESC_ESCURIDAO) {
        --nivel;
      }
    }
  }
  return nivel;
}

int NivelConjuradorParaAcao(const AcaoProto& acao, const ArmaProto& feitico_tabelado, const Entidade& entidade) {
  if (!acao.classe_conjuracao().empty()) {
    VLOG(1) << "classe conjuracao: " << acao.classe_conjuracao();
    return EhFeitico(feitico_tabelado)
        ? entidade.NivelConjuradorParaMagia(acao.classe_conjuracao(), feitico_tabelado)
        : NivelConjurador(acao.classe_conjuracao(), entidade.Proto());
  }
  VLOG(1) << "sem classe conjuracao";
  return Nivel(entidade.Proto());
}

// Retorna o nivel aumentado de conjurador da classe, se houver.
std::string NivelAumentadoConjurador(const Tabelas& tabelas, const std::string& id_classe, const EntidadeProto& proto) {
  // Primeiro: ve se a classe estabelece qual nivel sera aumentado.
  const auto& classe_tabelada = tabelas.Classe(id_classe);
  if (classe_tabelada.has_aumenta_nivel_conjurador_de()) {
    return classe_tabelada.aumenta_nivel_conjurador_de();
  }
  if (!classe_tabelada.has_aumenta_nivel_conjurador()) {
    return "";
  }
  // A classe tabelada aumenta nivel de conjurador mas nao especificou.
  // Tenta o que o usuario passou.
  const auto& ic = InfoClasseProtoParaMagia(id_classe, proto);
  if (ic.has_aumenta_nivel_conjurador_de()) {
    return ic.aumenta_nivel_conjurador_de();
  }
  // Por ultimo, tenta achar a primeira classe com nivel de conjuracao.
  for (const auto& ic : proto.info_classes()) {
    const auto& classe_tabelada = tabelas.Classe(ic.id());
    if (classe_tabelada.progressao_conjurador() != PCONJ_ZERO) {
      return ic.id();
    }
  }
  return "";
}

int NivelParaCalculoMagiasPorDia(const Tabelas& tabelas, const std::string& id_classe, const EntidadeProto& proto) {
  int niveis_da_classe = 0;
  for (const auto& info_classe : proto.info_classes()) {
    if (info_classe.id() == id_classe) {
      niveis_da_classe += info_classe.nivel();
    } else {
      std::string classe_aumentada = NivelAumentadoConjurador(tabelas, info_classe.id(), proto);
      if (!classe_aumentada.empty() && classe_aumentada == id_classe) {
        niveis_da_classe += info_classe.nivel();
      }
    }
  }
  return niveis_da_classe;
}

// Nivel expulsao.
int NivelExpulsao(const Tabelas& tabelas, const ent::EntidadeProto& proto) {
  int total = 0;
  for (const auto& ic : proto.info_classes()) {
    const auto& classe_tabelada = tabelas.Classe(ic.id());
    switch (classe_tabelada.progressao_expulsao()) {
      case PEXP_UM:
        total += ic.nivel();
      break;
      case PEXP_UM_MENOS_TRES:
        if (ic.nivel() > 3) {
          total += ic.nivel() - 3;
        }
      break;
      default:
        continue;
    }
  }
  if (PossuiTalento("expulsao_aprimorada", proto)) {
    ++total;
  }
  return total - proto.niveis_negativos();
}

std::string StringCritico(const DadosAtaque& da) {
  if (da.multiplicador_critico() == 2 && da.margem_critico() == 20) return "";
  std::string critico = "(";
  if (da.margem_critico() < 20) {
    critico += absl::StrFormat("%d", da.margem_critico()) + "-20";
    if (da.multiplicador_critico() > 2) {
      critico += "/";
    }
  }
  if (da.multiplicador_critico() > 2) {
    critico += "x" + absl::StrFormat("%d", da.multiplicador_critico());
  }
  critico += ")";
  return critico;
}

std::string StringAtaque(const DadosAtaque& da, const EntidadeProto& proto) {
  int modificador = ModificadorAtaque(da, proto, EntidadeProto());
  std::string texto_modificador;
  if (modificador != 0) texto_modificador = absl::StrFormat("%+d", modificador);

  std::string texto_furtivo;
  if (proto.dados_ataque_global().furtivo() && !proto.dados_ataque_global().dano_furtivo().empty()) {
    texto_furtivo = absl::StrFormat("+%s", proto.dados_ataque_global().dano_furtivo().c_str());
  }

  std::string critico = StringCritico(da);
  auto [dano_normal, dano_adicional_opt] = StringDanoParaAcao(da, proto, EntidadeProto());
  std::string dano_adicional;
  if (dano_adicional_opt.has_value()) {
    dano_adicional = absl::StrFormat("+%s[%s]", dano_adicional_opt->c_str(), TextoDescritor(da.elemento_dano_adicional()));
  }
  return absl::StrFormat(
      "%s (%s)%s%s: %+d%s, %s%s%s%s, CA: %s",
      da.grupo().c_str(), da.rotulo().c_str(),
      da.descarregada() ? " [descarregado]" : "", da.ataque_toque() ? " (T)" : "",
      da.bonus_ataque_final(), texto_modificador.c_str(),
      dano_normal.c_str(), dano_adicional.c_str(), critico.c_str(), texto_furtivo.c_str(),
      StringCAParaAcao(da, proto).c_str());
}

std::string StringCAParaAcao(const DadosAtaque& da, const EntidadeProto& proto) {
  const bool permite_escudo = da.empunhadura() == EA_ARMA_ESCUDO && PermiteEscudo(proto);
  int normal, toque;
  std::string info = !permite_escudo && !proto.surpreso()
      ? "" : permite_escudo && proto.surpreso() ? "(esc+surp) " : permite_escudo ? "(escudo) " : "(surpreso) ";
  if (proto.dados_defesa().has_ca()) {
    normal = DestrezaNaCA(proto) ? CATotal(proto, permite_escudo) : CASurpreso(proto, permite_escudo);
    toque = DestrezaNaCA(proto) ? CAToque(proto) : CAToqueSurpreso(proto);
  } else {
    normal = proto.surpreso() ? da.ca_surpreso() : da.ca_normal();
    toque = da.ca_toque();
  }
  return absl::StrFormat("%s%d, tq: %d", info.c_str(), normal, toque);
}

std::string StringDescritores(const google::protobuf::RepeatedField<int>& descritores) {
  if (descritores.empty()) return "";
  if (descritores.size() == 1) return absl::StrFormat(" [%s] ", TextoDescritor(descritores.Get(0)));
  std::string ret;
  for (int descritor : descritores) {
    ret += TextoDescritor(descritores.Get(descritor));
    ret += ", ";
  }
  ret.resize(ret.size() - 2);
  return absl::StrFormat(" [%s] ", ret.c_str());
}

std::string StringResumoArma(const Tabelas& tabelas, const ent::DadosAtaque& da) {
  // Monta a string.
  std::string string_rotulo = absl::StrFormat("%s (%s), ", da.grupo().c_str(), da.rotulo().c_str());

  std::string string_nome_arma = da.id_arma().empty()
      ? ""
      : absl::StrFormat("%s, ", tabelas.Arma(da.id_arma()).nome().c_str());
  std::string string_alcance;
  if (da.has_alcance_m()) {
    char string_incrementos[40] = { '\0' };
    if (da.has_incrementos()) {
      snprintf(string_incrementos, 39, ", inc %d", da.incrementos());
    }
    string_alcance = absl::StrFormat("alcance: %0.0f q%s, ", da.alcance_m() * METROS_PARA_QUADRADOS, string_incrementos);
  }

  std::string texto_municao;
  if (da.has_municao()) texto_municao = absl::StrFormat(", municao: %d", da.municao());
  std::string texto_limite_vezes;
  if (da.has_limite_vezes()) texto_limite_vezes = absl::StrFormat(", limite vezes: %d", da.limite_vezes());
  std::string texto_descarregada;
  if (da.descarregada()) texto_descarregada = " [descarregada]";

  std::string texto_elementos;
  if (da.has_elemento()) texto_elementos = absl::StrFormat(" [%s] ", TextoDescritor(da.elemento()));

  std::string string_escudo = da.empunhadura() == ent::EA_ARMA_ESCUDO ? "(escudo)" : "";
  std::string string_salvacao;
  if (da.has_dificuldade_salvacao()) {
    string_salvacao = absl::StrFormat(", CD: %d", da.dificuldade_salvacao());
  }
  std::string texto_veneno;
  if (da.has_veneno()) {
    texto_veneno = absl::StrFormat(", veneno CD %d", da.veneno().cd());
  }
  return absl::StrFormat(
      "id: %s%s%s, %sbonus: %d, dano: %s%s%s%s%s%s%s%s, ca%s: %d toque: %d surpresa%s: %d",
      string_rotulo.c_str(), string_nome_arma.c_str(), da.tipo_ataque().c_str(),
      string_alcance.c_str(),
      da.bonus_ataque_final(),
      da.dano().c_str(), StringCritico(da).c_str(), texto_elementos.c_str(), texto_municao.c_str(), texto_descarregada.c_str(), texto_limite_vezes.c_str(), texto_veneno.c_str(),
      string_salvacao.c_str(),
      string_escudo.c_str(), da.ca_normal(),
      da.ca_toque(),
      string_escudo.c_str(), da.ca_surpreso());
}

int CalculaDanoAdicionalPorTendencia(const DadosAtaque& da, const EntidadeProto& alvo) {
  if (!da.has_dano_adicional_por_tendencia()) return 0;
  int dano = 0;
  const auto& dapt = da.dano_adicional_por_tendencia();
  if (dapt.bom() > 0 && Bom(alvo)) {
    dano += dapt.bom();
  }
  if (dapt.mau() > 0 && Mau(alvo)) {
    dano += dapt.mau();
  }
  if (dapt.leal() > 0 && Ordeiro(alvo)) {
    dano += dapt.leal();
  }
  if (dapt.caotico() > 0 && Caotico(alvo)) {
    dano += dapt.caotico();
  }
  return dano;
}

std::pair<std::string, std::optional<std::string>> StringDanoParaAcao(
    const DadosAtaque& da, const EntidadeProto& proto, const EntidadeProto& alvo) {
  int modificador_dano = ModificadorDano(da, proto, alvo);
  const std::string* dano = &da.dano();
  if (!da.dano_por_tipo().empty()) {
    for (const auto& dt : da.dano_por_tipo()) {
      if (TemTipoDnD(dt.tipo(), alvo)) {
        dano = &dt.dano();
        break;
      }
    }
  }
  int dapt = CalculaDanoAdicionalPorTendencia(da, alvo);
  const std::string dano_normal = absl::StrFormat(
      "%s%s%s",
      dano->c_str(),
      !dano->empty() &&  modificador_dano != 0 ? absl::StrFormat("%+d", modificador_dano).c_str() : "",
      dapt > 0 ? absl::StrFormat("+%d", dapt).c_str() : "");
  std::optional<std::string> dano_adicional_opt;
  if (da.has_dano_adicional()) {
    dano_adicional_opt = absl::StrFormat("%s", da.dano_adicional().c_str());
  }
  return { dano_normal, dano_adicional_opt };
}

// Monta a string de dano de uma arma de um ataque, como 1d6 (x3). Nao inclui modificadores.
std::string StringDanoBasicoComCritico(const ent::DadosAtaque& da) {
  std::string critico = StringCritico(da);
  return absl::StrFormat("%s%s", da.dano_basico().c_str(), critico.empty() ? "" : critico.c_str());
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
    const int racial = PossuiBonus(TB_RACIAL, bonus) ? BonusIndividualTotal(TB_RACIAL, bonus) : 0;
    // Tem que por o valor zero para sobrescrever.
    AtribuiBonus(racial, TB_RACIAL, "racial", BonusAtributo(ta, &ret));
  }
  ret.set_mais_de_duas_pernas(proto.mais_de_duas_pernas());
  ret.set_raca(proto.raca());
  *ret.mutable_info_textura() = proto.info_textura();
  *ret.mutable_modelo_3d() = proto.modelo_3d();
  if (!proto.dados_ataque().empty()) {
    *ret.mutable_dados_ataque() = proto.dados_ataque();
  } else {
    // Cria uma entrada dummy.
    ret.add_dados_ataque()->set_tipo_ataque("");
  }
  // Tamanho.
  if (proto.has_tamanho()) {
    AtribuiBonus(proto.tamanho(), TB_BASE, "base", ret.mutable_bonus_tamanho());
  } else {
    const int tam_base = BonusIndividualPorOrigem(TB_BASE, "base", proto.bonus_tamanho());
    AtribuiBonus(tam_base, TB_BASE, "base", ret.mutable_bonus_tamanho());
  }
  // Visao.
  // A forma alternativa nao ganha qualidades especiais (visao, faro).
  // Talento.
  // idem.

  // CA.
  const int ca_natural = BonusIndividualPorOrigem(TB_ARMADURA_NATURAL, "racial", proto.dados_defesa().ca());
  AtribuiBonus(ca_natural, TB_ARMADURA_NATURAL, "racial", ret.mutable_dados_defesa()->mutable_ca());
  ret.mutable_dados_defesa()->set_id_armadura(proto.dados_defesa().id_armadura());
  ret.mutable_dados_defesa()->set_armadura_obra_prima(proto.dados_defesa().armadura_obra_prima());
  ret.mutable_dados_defesa()->set_bonus_magico_armadura(proto.dados_defesa().bonus_magico_armadura());
  ret.mutable_dados_defesa()->set_material_armadura(proto.dados_defesa().material_armadura());
  ret.mutable_dados_defesa()->set_id_escudo(proto.dados_defesa().id_escudo());
  ret.mutable_dados_defesa()->set_escudo_obra_prima(proto.dados_defesa().escudo_obra_prima());
  ret.mutable_dados_defesa()->set_bonus_magico_escudo(proto.dados_defesa().bonus_magico_escudo());
  ret.mutable_dados_defesa()->set_material_escudo(proto.dados_defesa().material_escudo());

  AtribuiTesouroTodoOuCriaVazios(proto.tesouro(), ret.mutable_tesouro());
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

bool PreencheCondicoesBonus(const CondicoesBonus& condicoes, const EntidadeProto& proto) {
  if (condicoes.conjura_magia_arcana()) {
    if (c_none_of(proto.info_classes(), [] (const InfoClasse& ic) { return ic.tipo_magia() == TM_ARCANA; })) return false;
  }
  return true;
}

bool PossuiTalento(const std::string& chave_talento, const EntidadeProto& entidade) {
  return Talento(chave_talento, entidade) != nullptr;
}

bool PossuiTalento(const std::string& chave_talento, const std::string& complemento, const EntidadeProto& entidade) {
  return Talento(chave_talento, complemento, entidade) != nullptr;
}

TalentoProto* TalentoOuCria(const std::string& chave_talento, EntidadeProto* proto) {
  for (auto& t : *proto->mutable_info_talentos()->mutable_gerais()) {
    if (chave_talento == t.id()) return &t;
  }
  for (auto& t : *proto->mutable_info_talentos()->mutable_outros()) {
    if (chave_talento == t.id()) return &t;
  }
  for (auto& t : *proto->mutable_info_talentos()->mutable_automaticos()) {
    if (chave_talento == t.id()) return &t;
  }
  auto* nt = proto->mutable_info_talentos()->add_outros();
  nt->set_id(chave_talento);
  return nt;
}

const TalentoProto* Talento(const std::string& chave_talento, const EntidadeProto& entidade) {
  for (const auto& t : entidade.info_talentos().gerais()) {
    if (chave_talento == t.id()) return &t;
  }
  for (const auto& t : entidade.info_talentos().outros()) {
    if (chave_talento == t.id()) return &t;
  }
  for (const auto& t : entidade.info_talentos().automaticos()) {
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
  for (const auto& t : entidade.info_talentos().automaticos()) {
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

int ValorFinalPericia(const std::string& id, const EntidadeProto& proto) {
  return BonusTotal(BonusPericia(id, proto));
}

const Bonus& BonusPericia(const std::string& id, const EntidadeProto& proto) {
  for (const auto& info_pericia : proto.info_pericias()) {
    if (info_pericia.id() == id) {
      return info_pericia.bonus();
    }
  }
  return Bonus::default_instance();
}

bool PericiaDeClasse(const Tabelas& tabelas, const std::string& chave_pericia, const EntidadeProto& proto) {
  for (const auto& ic : proto.info_classes()) {
    const auto& ct = tabelas.Classe(ic.id());
    if (c_any(ct.pericias(), chave_pericia)) {
      return true;
    }
    if (c_any(ic.pericias_monstro(), chave_pericia)) {
      return true;
    }
  }
  return false;
}

bool PodeUsarPericia(const Tabelas& tabelas, const std::string& id_pericia, const EntidadeProto& proto) {
  auto* pt = &tabelas.Pericia(id_pericia);
  if (!pt->derivada_de().empty()) {
    pt = &tabelas.Pericia(pt->derivada_de());
  }
  if (pt->sem_treinamento()) return true;
  const auto& pp = Pericia(pt->id(), proto);
  return PericiaDeClasse(tabelas, pt->id(), proto) ? pp.pontos() > 0 : pp.pontos() > 1;
}

int TotalPontosPericiaPermitidos(const Tabelas& tabelas, const EntidadeProto& proto) {
  bool primeiro_nivel = true;
  int total = 0;
  int modificador = ModificadorAtributo(TA_INTELIGENCIA, proto);
  if (proto.raca() == "humano") {
    ++modificador;
  }
  for (const auto& ic : proto.info_classes()) {
    if (ic.nivel() <= 0) continue;
    const auto& ct = tabelas.Classe(ic.id());
    int por_nivel = std::max(ct.pericias_por_nivel() + modificador, 1);
    if (primeiro_nivel) {
      total += (por_nivel * 4) + por_nivel * (ic.nivel() - 1);
    } else {
      total += por_nivel * ic.nivel();
    }
    primeiro_nivel = false;
  }
  return total;
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
RepeatedPtrField<EntidadeProto::Evento> LeEventos(const std::string& eventos_str) {
  RepeatedPtrField<EntidadeProto::Evento> ret;
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
  Bonus b;
  if ((Bom(proto_ataque) && PossuiEvento(EFEITO_PROTECAO_CONTRA_BEM, proto_defesa)) ||
      (Mau(proto_ataque) && PossuiEvento(EFEITO_PROTECAO_CONTRA_MAL, proto_defesa)) ||
      (Ordeiro(proto_ataque) && PossuiEvento(EFEITO_PROTECAO_CONTRA_ORDEM, proto_defesa)) ||
      (Caotico(proto_ataque) && PossuiEvento(EFEITO_PROTECAO_CONTRA_CAOS, proto_defesa))) {
    AtribuiBonus(2, TB_DEFLEXAO, "protecao_contra_tendencia", &b);
  }
  if ((Bom(proto_ataque) && PossuiEvento(EFEITO_DISSIPAR_BEM, proto_defesa)) ||
      (Mau(proto_ataque) && PossuiEvento(EFEITO_DISSIPAR_MAL, proto_defesa)) ||
      (Ordeiro(proto_ataque) && PossuiEvento(EFEITO_DISSIPAR_ORDEM, proto_defesa)) ||
      (Caotico(proto_ataque) && PossuiEvento(EFEITO_DISSIPAR_CAOS, proto_defesa))) {
    Bonus b;
    AtribuiBonus(4, TB_DEFLEXAO, "dissipar_tendencia", &b);
  }
  return b;
}

Bonus BonusContraTendenciaNaSalvacao(const EntidadeProto& proto_ataque, const EntidadeProto& proto_defesa) {
  if ((Bom(proto_ataque) && PossuiEvento(EFEITO_PROTECAO_CONTRA_BEM, proto_defesa)) ||
      (Mau(proto_ataque) && PossuiEvento(EFEITO_PROTECAO_CONTRA_MAL, proto_defesa)) ||
      (Ordeiro(proto_ataque) && PossuiEvento(EFEITO_PROTECAO_CONTRA_ORDEM, proto_defesa)) ||
      (Caotico(proto_ataque) && PossuiEvento(EFEITO_PROTECAO_CONTRA_CAOS, proto_defesa))) {
    Bonus b;
    AtribuiBonus(2, TB_RESISTENCIA, "protecao_contra_tendencia", &b);
    return b;
  }
  return Bonus();
}

int Nivel(const EntidadeProto& proto) {
  int total = -proto.niveis_negativos();
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

const InfoClasse& InfoClasseProto(const std::string& id_classe, const EntidadeProto& proto) {
  for (const auto& ic : proto.info_classes()) {
    if (ic.id() == id_classe) return ic;
  }
  return InfoClasse::default_instance();
}

const InfoClasse& InfoClasseProtoParaMagia(const std::string& id_classe, const EntidadeProto& proto) {
  for (const auto& ic : proto.info_classes()) {
    if (ic.id() == id_classe || ic.id_para_magia() == id_classe) return ic;
  }
  return InfoClasse::default_instance();
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

int NivelParaFeitico(const Tabelas& tabelas, const DadosAtaque& da, const EntidadeProto& proto) {
  return InfoClasseParaFeitico(tabelas, da.tipo_ataque(), proto).nivel();
}

void RenovaFeiticos(EntidadeProto* proto) {
  for (auto& fc : *proto->mutable_feiticos_classes()) {
    for (auto& nivel_fn : *fc.mutable_mapa_feiticos_por_nivel()) {
      auto& fn = nivel_fn.second;
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

bool AtacandoPoderosamente(const EntidadeProto& proto) {
  for (const auto& da : proto.dados_ataque()) {
    if (da.ataque_corpo_a_corpo()) {
      return BonusIndividualPorOrigem(TB_SEM_NOME, "ataque_poderoso", proto.dados_ataque(0).bonus_ataque()) < 0;
    }
  }
  return false;
}

bool EventosIguais(const EntidadeProto::Evento& lhs, const EntidadeProto::Evento& rhs) {
  return lhs.SerializeAsString() == rhs.SerializeAsString();
}

std::vector<int> IdsUnicosProto(const EntidadeProto& proto) {
  std::vector<int> ids;
  for (const auto& evento : proto.evento()) {
    if (evento.has_id_unico()) ids.push_back(evento.id_unico());
  }
  return ids;
}

std::vector<int> IdsUnicosEntidade(const Entidade& entidade) {
  return IdsUnicosProto(entidade.Proto());;
}

template <class ProtoInt>
bool IdsUnicosIguais(const google::protobuf::RepeatedField<ProtoInt>& lhs, const google::protobuf::RepeatedField<ProtoInt>& rhs) {
  // Por algum motivo, usar rhs.end da erro no mac.
  if (lhs.size() != rhs.size()) return false;
  return std::equal(lhs.begin(), lhs.end(), rhs.begin());//, rhs.end());
}

template <class ProtoInt>
bool IdsUnicosIguaisSemOrdem(const google::protobuf::RepeatedField<ProtoInt>& lhs, const google::protobuf::RepeatedField<ProtoInt>& rhs) {
  std::set<int> lhss(lhs.begin(), lhs.end());
  std::set<int> rhss(rhs.begin(), rhs.end());
  return std::equal(lhss.begin(), lhss.end(), rhss.begin());
}

int AchaIdUnicoEvento(const std::vector<int>& ids_unicos) {
  int candidato = 0;
  bool existe = false;
  auto EhCandidato = [&candidato] (int id_unico) { return candidato == id_unico; };
  do {
    existe = c_any_of(ids_unicos, EhCandidato);
    if (!existe) break;
    ++candidato;
    if (candidato < 0) {
      // Se isso acontecer, acredito que a memoria vai explodir antes...
      LOG(ERROR) << "Cheguei ao limite de eventos";
      candidato = 0;
      break;
    }
  } while (1);
  VLOG(1) << "Retornando id unico: " << candidato;
  return candidato;
}

int AchaIdUnicoEvento(
    const RepeatedPtrField<EntidadeProto::Evento>& eventos,
    const RepeatedPtrField<EntidadeProto::Evento>& eventos_sendo_gerados) {
  int candidato = 0;
  bool existe = false;
  auto EhCandidato = [&candidato] (const EntidadeProto::Evento& evento) { return candidato == evento.id_unico(); };
  do {
    existe = c_any_of(eventos, EhCandidato) ||
             c_any_of(eventos_sendo_gerados, EhCandidato);
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
    const std::optional<DadosIniciativa>& dados_iniciativa, const std::string& origem, TipoEfeito id_efeito, int rodadas, bool continuo, std::vector<int>* ids_unicos, EntidadeProto* proto) {
  // Pega antes de criar o evento.
  int id_unico = AchaIdUnicoEvento(*ids_unicos);
  auto* e = proto->add_evento();
  e->set_id_efeito(id_efeito);
  e->set_rodadas(continuo ? 1 : rodadas);
  e->set_continuo(continuo);
  e->set_id_unico(id_unico);
  e->set_origem(origem);
  ids_unicos->push_back(id_unico);
  if (dados_iniciativa.has_value()) {
    e->set_iniciativa(dados_iniciativa->iniciativa);
    e->set_modificador_iniciativa(dados_iniciativa->modificador);
  }
  return e;
}

// Acha o efeito anterior para fins de efeitos que herdam atributos dos anteriores.
const EfeitoAdicional* EfeitoAnterior(const EfeitoAdicional& efeito_adicional, const AcaoProto& acao) {
  // Encontra o efeito para saber qual o anterior.
  for (int i = 1; i < acao.efeitos_adicionais().size(); ++i) {
    if (&acao.efeitos_adicionais(i) == &efeito_adicional) {
      return &acao.efeitos_adicionais(i - 1);
    }
  }
  for (int i = 1; i < acao.efeitos_adicionais_se_salvou().size(); ++i) {
    if (&acao.efeitos_adicionais_se_salvou(i) == &efeito_adicional) {
      return &acao.efeitos_adicionais_se_salvou(i - 1);
    }
  }
  return nullptr;
}

// Retorna o valor de rodadas ou kEfeitoContinuo se efeito for continuo. Caso nao haja alvo, nao preenchera rodadas que depende disso.
constexpr int kEfeitoContinuo = std::numeric_limits<int>::max();
int Rodadas(
    int nivel_conjurador, const EfeitoAdicional& efeito_adicional, const AcaoProto& acao, const EntidadeProto& lancador,
    const Entidade* alvo) {
  VLOG(1) << "Calculo de rodadas: nivel de conjurador: " << nivel_conjurador;
  if (efeito_adicional.has_rodadas()) {
    VLOG(1) << "Calculo de rodadas, valor de rodadas: " << efeito_adicional.rodadas();
    return efeito_adicional.rodadas();
  }
  int rodadas_base = efeito_adicional.rodadas_base();
  if (efeito_adicional.rodadas_base_igual_efeito_anterior()) {
    // Encontra o efeito para saber qual o anterior.
    const EfeitoAdicional* efeito_anterior = EfeitoAnterior(efeito_adicional, acao);
    if (efeito_anterior == nullptr) {
      LOG(WARNING) << "nao encontrei efeito adicional anterior: "  << efeito_adicional.ShortDebugString();
      rodadas_base = 0;
    } else {
      // Usa o valor final do anterior.
      rodadas_base = efeito_anterior->rodadas();
      //LOG(INFO) << "efeito_anterior: " << efeito_anterior->DebugString();
    }
  }
  if (efeito_adicional.has_dado_modificador_rodadas()) {
    int modificador = RolaValor(efeito_adicional.dado_modificador_rodadas());
    VLOG(1) << "Calculo de rodadas por string, valor final: " << (rodadas_base + modificador);
    return rodadas_base + modificador;
  }
  if (efeito_adicional.has_modificador_rodadas()) {
    int modificador = 0;
    switch (efeito_adicional.modificador_rodadas()) {
      case MR_DRENAR_FORCA_VITAL:
        return 10 * MINUTOS_PARA_RODADAS * std::max(1, alvo == nullptr ? 1 : alvo->NivelPersonagem());
      break;
      case MR_MOD_CARISMA:
        modificador = ModificadorAtributo(TA_CARISMA, lancador);
      break;
      case MR_CONTINUO:
        modificador = kEfeitoContinuo;
      break;
      case MR_PALAVRA_PODER_ATORDOAR: {
        if (alvo == nullptr) break;
        const int pv = alvo->PontosVida();
        if (pv <= 50) {
          modificador = RolaValor("4d4");;
        } else if (pv <= 100) {
          modificador = RolaValor("2d4");
        } else {
          modificador = RolaValor("1d4");
        }
      }
      break;
      case MR_PALAVRA_PODER_CEGAR: {
        if (alvo == nullptr) break;
        const int pv = alvo->PontosVida();
        if (pv <= 50) {
          modificador = kEfeitoContinuo;
        } else if (pv <= 100) {
          modificador = RolaValor("1d4+1") * MINUTOS_PARA_RODADAS;
        } else {
          modificador = RolaValor("1d4+1");
        }
      }
      break;
      case MR_DIAS_POR_NIVEL:
        modificador = nivel_conjurador * 24 * HORAS_PARA_RODADAS;
        break;
      case MR_2_MINUTOS_NIVEL:
        modificador = nivel_conjurador * 2 * MINUTOS_PARA_RODADAS;
        break;
      case MR_RODADAS_NIVEL:
        modificador = nivel_conjurador;
        break;
      case MR_MINUTOS_NIVEL:
        modificador = MINUTOS_PARA_RODADAS * nivel_conjurador;
        break;
      case MR_10_MINUTOS_NIVEL:
        modificador = 10 * MINUTOS_PARA_RODADAS * nivel_conjurador;
        break;
      case MR_HORAS_NIVEL:
        modificador = HORAS_PARA_RODADAS * nivel_conjurador;
        break;
      case MR_HORAS_NIVEL_MAX_15:
        modificador = HORAS_PARA_RODADAS * std::min(nivel_conjurador, 15);
        break;
      case MR_2_HORAS_NIVEL:
        modificador = 2 * HORAS_PARA_RODADAS * nivel_conjurador;
        break;
      case MR_1_RODADA_A_CADA_3_NIVEIS_MAX_6:
        modificador = std::min(nivel_conjurador / 3, 6);
        break;
      case MR_1_RODADA_A_CADA_2_NIVEIS:
        modificador = nivel_conjurador / 2;
        break;
      case MR_10_RODADAS_MAIS_UMA_POR_NIVEL_MAX_15:
        modificador = std::min(15, 10 + nivel_conjurador);
        break;
      default:
        break;
    }
    VLOG(1) << "Calculo de rodadas, valor final: " << (rodadas_base + modificador);
    return modificador == kEfeitoContinuo ? kEfeitoContinuo : rodadas_base + modificador;
  }
  if (efeito_adicional.has_rodadas_base() || efeito_adicional.rodadas_base_igual_efeito_anterior()) {
    VLOG(1) << "Calculo de rodadas base, valor final: " << rodadas_base;
    return rodadas_base;
  }
  VLOG(1) << "Sem dado para rodadas, retornando efeito contínuo";
  return kEfeitoContinuo;
}

void PreencheComplementos(unsigned int id_origem, int nivel_conjurador, const EfeitoAdicional& efeito_adicional, const Entidade* alvo, EntidadeProto::Evento* evento) {
  *evento->mutable_complementos_str() = efeito_adicional.complementos_str();
  if (efeito_adicional.has_dado_complementos_str()) {
    evento->add_complementos(RolaValor(efeito_adicional.dado_complementos_str()));
    return;
  }
  switch (efeito_adicional.modificador_complementos()) {
    case MC_PELE_ARVORE: {
      int valor = 2 + ((nivel_conjurador - 3) / 3);
      evento->add_complementos(std::min(5, valor));
      break;
    }
    case MC_ID_ENTIDADE: {
      evento->add_complementos(id_origem);
      break;
    }
    case MC_1D6_MAIS_1_CADA_2_NIVEIS_MAX_5_NEGATIVO: {
      int adicionais = std::min(5, nivel_conjurador / 2);
      evento->add_complementos(-RolaValor(absl::StrFormat("1d6+%d", adicionais)));
      break;
    }
    case MC_1_POR_NIVEL: {
      evento->add_complementos(nivel_conjurador);
      break;
    }
    case MC_10_POR_NIVEL: {
      evento->add_complementos(nivel_conjurador * 10);
      break;
    }
    case MC_1_POR_NIVEL_MAX_10: {
      evento->add_complementos(std::min(10, nivel_conjurador));
      break;
    }
    case MC_1D8_MAIS_1_POR_NIVEL_MAX_10: {
      evento->add_complementos(RolaValor("1d8") + std::min(10, nivel_conjurador));
      break;
    }
    case MC_1D10_MAIS_1_POR_NIVEL_MAX_10: {
      evento->add_complementos(RolaValor("1d10") + std::min(10, nivel_conjurador));
      break;
    }

    case MC_1D4_MAIS_1_CADA_TRES_MAX_8: {
      int adicionais = std::min(4, nivel_conjurador / 3);
      evento->add_complementos(RolaValor(absl::StrFormat("1d4+%d", adicionais)));
      break;
    }
    case MC_1_CADA_3_MAX_3: {
      evento->add_complementos(std::min(3, nivel_conjurador / 3));
      break;
    }
    case MC_2_MAIS_1_CADA_6_MAX_5: {
      evento->add_complementos(std::min(5, (nivel_conjurador / 6) + 2));
      break;
    }
    case MC_NIVEL_CLERIGO: {
      evento->add_complementos(nivel_conjurador);
      break;
    }
    default:
      *evento->mutable_complementos() = efeito_adicional.complementos();
  }
}

EntidadeProto::Evento* AdicionaEventoEfeitoAdicional(
    unsigned int id_origem, const std::optional<DadosIniciativa>& dados_iniciativa, int nivel_conjurador, const EfeitoAdicional& efeito_adicional, const AcaoProto& acao,
    std::vector<int>* ids_unicos,  const Entidade& alvo, EntidadeProto* proto) {
  const int rodadas = Rodadas(nivel_conjurador, efeito_adicional, acao, *proto, &alvo);
  const bool continuo = rodadas == kEfeitoContinuo ||
                        (!efeito_adicional.has_rodadas() &&
                         !efeito_adicional.has_modificador_rodadas() &&
                         !efeito_adicional.has_dado_modificador_rodadas());
  auto* e = AdicionaEvento(dados_iniciativa, efeito_adicional.origem(), efeito_adicional.efeito(), rodadas, continuo, ids_unicos, proto);
  if (efeito_adicional.has_requer_modelo_ativo()) {
    e->set_requer_modelo_ativo(efeito_adicional.requer_modelo_ativo());
  }
  PreencheComplementos(id_origem, nivel_conjurador, efeito_adicional, &alvo, e);
  if (efeito_adicional.has_descricao()) e->set_descricao(efeito_adicional.descricao());
  return e;
}

void ExpiraEventoItemMagico(int id_unico, EntidadeProto* proto) {
  for (auto& evento : *proto->mutable_evento()) {
    if (evento.id_unico() == id_unico) {
      evento.set_rodadas(-1);
      return;
    }
  }
}

void ExpiraEventosItemMagico(ItemMagicoProto* item, EntidadeProto* proto) {
  for (int id_unico : item->ids_efeitos()) {
    ExpiraEventoItemMagico(id_unico, proto);
  }
  item->clear_ids_efeitos();
}

EntidadeProto::Evento* AchaEvento(int id_unico, EntidadeProto* proto) {
  for (auto& evento : *proto->mutable_evento()) {
    if (evento.id_unico() == id_unico) {
      return &evento;
    }
  }
  return nullptr;
}

const EntidadeProto::Evento* AchaEvento(int id_unico, const EntidadeProto& proto) {
  for (auto& evento : proto.evento()) {
    if (evento.id_unico() == id_unico) {
      return &evento;
    }
  }
  return nullptr;
}

ResistenciaElementos* AchaOuCriaResistenciaElementoIdUnico(DescritorAtaque descritor, int id_unico, EntidadeProto* proto) {
  for (auto& re : *proto->mutable_dados_defesa()->mutable_resistencia_elementos()) {
    if (re.descritor() == descritor && re.id_unico() == id_unico) {
      return &re;
    }
  }
  return nullptr;
}

void LimpaResistenciaElementoIdUnico(DescritorAtaque descritor, int id_unico, EntidadeProto* proto) {
  int i = 0;
  for (auto& re : *proto->mutable_dados_defesa()->mutable_resistencia_elementos()) {
    if (re.descritor() == descritor && re.id_unico() == id_unico) {
      proto->mutable_dados_defesa()->mutable_resistencia_elementos()->DeleteSubrange(i, 1);
      return;
    }
    ++i;
  }
}

ResistenciaElementos* AchaOuCriaResistenciaElementoEfeitoModelo(DescritorAtaque descritor, TipoEfeitoModelo id_efeito_modelo, EntidadeProto* proto) {
  for (auto& re : *proto->mutable_dados_defesa()->mutable_resistencia_elementos()) {
    if (re.descritor() == descritor && re.id_efeito_modelo() == id_efeito_modelo) {
      return &re;
    }
  }
  return proto->mutable_dados_defesa()->add_resistencia_elementos();
}

void LimpaResistenciaElementoEfeitoModelo(DescritorAtaque descritor, TipoEfeitoModelo id_efeito_modelo, EntidadeProto* proto) {
  int i = 0;
  for (auto& re : *proto->mutable_dados_defesa()->mutable_resistencia_elementos()) {
    if (re.descritor() == descritor && re.id_efeito_modelo() == id_efeito_modelo) {
      proto->mutable_dados_defesa()->mutable_resistencia_elementos()->DeleteSubrange(i, 1);
      return;
    }
    ++i;
  }
}

ReducaoDano* AchaOuCriaReducaoDanoEfeitoModelo(TipoEfeitoModelo id_efeito_modelo, EntidadeProto* proto) {
  for (auto& rd : *proto->mutable_dados_defesa()->mutable_reducao_dano()) {
    if (rd.id_efeito_modelo() == id_efeito_modelo) {
      return &rd;
    }
  }
  return proto->mutable_dados_defesa()->add_reducao_dano();
}

void LimpaReducaoDanoEfeitoModelo(TipoEfeitoModelo id_efeito_modelo, EntidadeProto* proto) {
  int i = 0;
  for (auto& rd : *proto->mutable_dados_defesa()->mutable_reducao_dano()) {
    if (rd.id_efeito_modelo() == id_efeito_modelo) {
      proto->mutable_dados_defesa()->mutable_reducao_dano()->DeleteSubrange(i, 1);
      return;
    }
    ++i;
  }
}

void AdicionaEventoItemMagico(
    const ItemMagicoProto& item, int indice, int rodadas, bool continuo,
    std::vector<int>* ids_unicos, EntidadeProto* proto) {
  std::vector<std::pair<TipoEfeito, std::string>> efeitos_origens;
  if (item.combinacao_efeitos() == COMB_EXCLUSIVO) {
    if (indice < 0 || indice >= item.tipo_efeito().size()) {
      LOG(ERROR) << "indice de efeito de item invalido para " << item.DebugString();
    } else {
      efeitos_origens.push_back(std::make_pair(item.tipo_efeito(indice), indice < item.origens_size() ? item.origens(indice) : ""));
    }
  } else {
    for (int indice = 0; indice < item.tipo_efeito().size(); ++indice) {
      auto tipo_efeito = item.tipo_efeito(indice);
      efeitos_origens.push_back(std::make_pair((TipoEfeito)tipo_efeito, indice < item.origens_size() ? item.origens(indice) : ""));
    }
  }

  for (unsigned int i = 0; i < efeitos_origens.size(); ++i) {
    auto tipo_efeito = efeitos_origens[i].first;
    const std::string& origem = efeitos_origens[i].second;
    auto* evento = AdicionaEvento(std::nullopt, origem, tipo_efeito, rodadas, continuo, ids_unicos, proto);
    if (item.complementos().empty() && !item.complementos_variavel().empty()) {
      for (const auto& cv : item.complementos_variavel()) {
        int valor = 0;
        try {
          valor = RolaValor(cv);
        } catch (...) {
          LOG(ERROR) << "Valor de complemento mal formado: " << cv;
        }
        evento->add_complementos(valor);
      }
    } else if (!item.complementos().empty()) {
      *evento->mutable_complementos() = item.complementos();
    }
    if (!item.complementos_str().empty()) {
      *evento->mutable_complementos_str() = item.complementos_str();
    }
    if (continuo) {
      evento->set_requer_pai(true);
    }
    evento->set_descricao(item.descricao().empty() ? item.nome() : item.descricao());
  }
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
  return c_any(proto.agarrado_a(), id);
}

//----------
// Dominios.
//----------
const EntidadeProto::PoderesDominio& PoderesDoDominio(const std::string& id_dominio, const EntidadeProto& proto) {
  const auto& ifc = FeiticosClasse("clerigo", proto);
  if (c_none(ifc.dominios(), id_dominio)) {
    return EntidadeProto::PoderesDominio::default_instance();
  }
  const auto& pd = ifc.poderes_dominio();
  if (auto it = pd.find(id_dominio); it == pd.end()) {
    return EntidadeProto::PoderesDominio::default_instance();
  } else {
    return it->second;
  }
}

EntidadeProto::PoderesDominio* PoderesDoDominio(const std::string& id_dominio, EntidadeProto* proto) {
  auto* ifc = FeiticosClasse("clerigo", proto);
  auto& pd = *ifc->mutable_poderes_dominio();
  if (auto it = pd.find(id_dominio); it == pd.end()) {
    return &pd.insert({id_dominio, EntidadeProto::PoderesDominio::default_instance()}).first->second;
  } else {
    return &it->second;
  }
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
  if (auto it = fc.mapa_feiticos_por_nivel().find(nivel); it == fc.mapa_feiticos_por_nivel().end()) {
    return EntidadeProto::FeiticosPorNivel::default_instance();
  } else {
    return it->second; 
  }
}

EntidadeProto::FeiticosPorNivel* FeiticosNivel(const std::string& id_classe, int nivel, EntidadeProto* proto) {
  auto* fc = FeiticosClasse(id_classe, proto);
  if (nivel < 0 || nivel > 9) return nullptr;
  return &(*fc->mutable_mapa_feiticos_por_nivel())[nivel];
}

EntidadeProto::FeiticosPorNivel* FeiticosNivelOuNullptr(
    const std::string& id_classe, int nivel, EntidadeProto* proto) {
  if (nivel < 0 || nivel > 9) return nullptr;
  auto* fc = FeiticosClasseOuNullptr(id_classe, proto);
  if (fc == nullptr) return nullptr;
  if (auto it = fc->mutable_mapa_feiticos_por_nivel()->find(nivel);
      it == fc->mutable_mapa_feiticos_por_nivel()->end()) {
    return nullptr;
  } else {
    return &it->second; 
  }
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

bool TemFeiticoDisponivel(const std::string& id_classe, int nivel, const EntidadeProto& proto) {
  const auto& fn = FeiticosNivel(id_classe, nivel, proto);
  for (int i = 0; i < fn.para_lancar().size(); ++i) {
    if (!fn.para_lancar(i).usado()) return true;
  }
  return false;
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
    auto& fn = (*fc->mutable_mapa_feiticos_por_nivel())[nivel];
    fn.set_nivel(nivel);
    auto* pl = fn.add_para_lancar();
    pl->set_usado(usado);
    pl->set_indice(indice);
  }
  {
    auto* e_antes = n->mutable_entidade_antes();
    auto* fc = e_antes->add_feiticos_classes();
    fc->set_id_classe(id_classe);
    auto& fn = (*fc->mutable_mapa_feiticos_por_nivel())[nivel];
    fn.set_nivel(nivel);
    auto* pl = fn.add_para_lancar();
    pl->set_usado(FeiticoParaLancar(id_classe, nivel, indice, proto).usado());
    pl->set_indice(indice);
  }

  return n;
}

int ComputaLimiteVezes(ModeloGenerico modelo, int nivel_conjurador) {
  switch (modelo) {
    case MG_UM_MAIS_UM_CADA_4_APOS_TERCEIRO_MAXIMO_3: {
      return std::min(3, (1 + nivel_conjurador - 3) / 4);
    }
    break;
    case MG_UM_CADA_3_NIVEIS: {
      return nivel_conjurador / 3;
    }
    break;
    case MG_UM_CADA_NIVEL_IMPAR_MAX_5: {
      return std::min(5, (nivel_conjurador + 1) / 2);
    }
    break;
    case MG_UM_POR_NIVEL: {
      return nivel_conjurador;
    }
    break;
    case MG_UM: ;
    default:
      return 1;
  }
}

int ComputaTotalDadosVida(ArmaProto::ModeloTotalDadosVida modelo_total_dv,
                          int nivel_conjurador) {
  switch (modelo_total_dv) {
    case ArmaProto::TDV_2D4_MAIS_NIVEL: {
      return RolaValor("2d4") + nivel_conjurador;
    }
    break;
    case ArmaProto::TDV_1D4_POR_NIVEL_MAX_20D4: {
      return RolaValor(absl::StrFormat("%dd4", std::min(20, nivel_conjurador)));
    }
    break;

    default:
      return 1000;
  }
}


void ComputaDano(ArmaProto::ModeloDano modelo_dano, int nivel_conjurador, DadosAtaque* da) {
  switch (modelo_dano) {
    case ArmaProto::DANO_3D6_MAIS_1_POR_NIVEL: {
      da->set_dano_basico_fixo(absl::StrFormat("3d6+%d", nivel_conjurador));
      return;
    }
    case ArmaProto::DANO_1D4_POR_NIVEL_MAX_5D4: {
      da->set_dano_basico_fixo(absl::StrFormat("%dd4", std::min(5, nivel_conjurador)));
      return;
    }
    case ArmaProto::CURA_1: {
      da->set_dano_basico_fixo("1");
      da->set_cura(true);
      return;
    }
    case ArmaProto::DANO_1: {
      da->set_dano_basico_fixo("1");
      return;
    }
    case ArmaProto::CURA_1D8_MAIS_1_POR_NIVEL_MAX_5: {
      da->set_dano_basico_fixo(absl::StrFormat("1d8+%d", std::min(5, nivel_conjurador)));
      da->set_cura(true);
      return;
    }
    case ArmaProto::CURA_2D8_MAIS_1_POR_NIVEL_MAX_10: {
      da->set_dano_basico_fixo(absl::StrFormat("2d8+%d", std::min(10, nivel_conjurador)));
      da->set_cura(true);
      return;
    }
    case ArmaProto::CURA_3D8_MAIS_1_POR_NIVEL_MAX_15: {
      da->set_dano_basico_fixo(absl::StrFormat("3d8+%d", std::min(15, nivel_conjurador)));
      da->set_cura(true);
      return;
    }
    case ArmaProto::CURA_4D8_MAIS_1_POR_NIVEL_MAX_20: {
      da->set_dano_basico_fixo(
          absl::StrFormat("4d8+%d", std::min(20, nivel_conjurador)));
      da->set_cura(true);
      return;
    }
    case ArmaProto::DANO_1D8_MAIS_1_POR_NIVEL_MAX_5: {
      da->set_dano_basico_fixo(absl::StrFormat("1d8+%d", std::min(5, nivel_conjurador)));
      return;
    }
    case ArmaProto::DANO_2D8_MAIS_1_POR_NIVEL_MAX_10: {
      da->set_dano_basico_fixo(absl::StrFormat("2d8+%d", std::min(10, nivel_conjurador)));
      return;
    }
    case ArmaProto::DANO_3D8_MAIS_1_POR_NIVEL_MAX_15: {
      da->set_dano_basico_fixo(absl::StrFormat("3d8+%d", std::min(15, nivel_conjurador)));
      return;
    }
    case ArmaProto::DANO_4D8_MAIS_1_POR_NIVEL_MAX_20: {
      da->set_dano_basico_fixo(absl::StrFormat("4d8+%d", std::min(20, nivel_conjurador)));
      return;
    }
    case ArmaProto::DANO_1D6_POR_NIVEL_MAX_10D6: {
      da->set_dano_basico_fixo(absl::StrFormat("%dd6", std::min(10, nivel_conjurador)));
      return;
    }
    case ArmaProto::DANO_1D6_POR_NIVEL_MAX_15D6: {
      da->set_dano_basico_fixo(absl::StrFormat("%dd6", std::min(15, nivel_conjurador)));
      return;
    }
    case ArmaProto::DANO_1D6_POR_NIVEL_MAX_20D6: {
      da->set_dano_basico_fixo(absl::StrFormat("%dd6", std::min(20, nivel_conjurador)));
      return;
    }
    case ArmaProto::DANO_1D8_CADA_2_NIVEIS_MAX_5D8: {
      da->set_dano_basico_fixo(absl::StrFormat("%dd8", std::min(5, nivel_conjurador / 2)));
      return;
    }
    case ArmaProto::DANO_1D8_CADA_2_NIVEIS_MAX_10D8: {
      da->set_dano_basico_fixo(absl::StrFormat("%dd8", std::min(10, nivel_conjurador / 2)));
      return;
    }
    case ArmaProto::DANO_1D8_POR_NIVEL_MAX_10D8: {
      da->set_dano_basico_fixo(absl::StrFormat("%dd8", std::min(10, nivel_conjurador)));
      return;
    }
    case ArmaProto::DANO_1D6_CADA_2_NIVEIS_MAX_5D6: {
      da->set_dano_basico_fixo(absl::StrFormat("%dd6", std::min(5, nivel_conjurador / 2)));
      return;
    }
    case ArmaProto::CURA_10_POR_NIVEL_MAX_150: {
      da->set_dano_basico_fixo(absl::StrFormat("%d", std::min(150, nivel_conjurador * 10)));
      da->set_cura(true);
      return;
    }
    case ArmaProto::DANO_10_POR_NIVEL_MAX_150: {
      da->set_dano_basico_fixo(absl::StrFormat("%d", std::min(150, nivel_conjurador * 10)));
      da->set_cura(false);
      return;
    }
    default:
      ;
  }
}

bool FeiticoGeraAcao(const Tabelas& tabelas, const ArmaProto& feitico_tabelado) {
  // Ha varias condicoes que fazem o feitico gerar uma acao. Vamos tentar varias.
  switch (feitico_tabelado.acao().tipo()) {
    case ACAO_INVALIDA:
      return tabelas.Acao(feitico_tabelado.acao().id()).tipo() != ACAO_INVALIDA;
      return false;
    default:
      return true;
  }
}

void CriaNovoAtaqueComFeitico(
    const Tabelas& tabelas, const ArmaProto& feitico_tabelado, const std::string& id_classe, int nivel_slot, const EntidadeProto& proto,
    ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  auto [e_antes, e_depois] = PreencheNotificacaoEntidadeProto(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, proto, n);
  {
    *e_depois->mutable_dados_ataque() = proto.dados_ataque();
    //std::string acao_str = ClasseParaTipoAtaqueFeitico(tabelas, IdParaMagia(tabelas, id_classe));
    std::string acao_str = ClasseParaTipoAtaqueFeitico(tabelas, id_classe);
    std::string grupo_str = feitico_tabelado.nome();
    e_depois->set_ultima_acao(acao_str);
    e_depois->set_ultimo_grupo_acao(grupo_str);
    auto* da = e_depois->add_dados_ataque();
    da->set_tipo_ataque(acao_str);
    da->set_grupo(grupo_str);
    const int nivel_conjurador = NivelConjuradorParaMagia(id_classe, feitico_tabelado, proto);
    int limite_vezes = ComputaLimiteVezes(feitico_tabelado.modelo_limite_vezes(), nivel_conjurador);
    da->set_rotulo(absl::StrFormat("%s x%d", feitico_tabelado.nome().c_str(), limite_vezes));
    da->set_id_arma(feitico_tabelado.id());
    da->set_limite_vezes(limite_vezes);
    da->set_nivel_slot(nivel_slot);
    if (feitico_tabelado.has_modelo_total_dv()) {
      da->mutable_acao()->set_total_dv(ComputaTotalDadosVida(feitico_tabelado.modelo_total_dv(), nivel_conjurador));
    }
    if (feitico_tabelado.has_modelo_dano()) {
      ComputaDano(feitico_tabelado.modelo_dano(), NivelConjurador(id_classe, proto), da);
    }
  }
  {
    e_antes->set_ultima_acao(proto.ultima_acao());
    e_antes->set_ultimo_grupo_acao(proto.ultimo_grupo_acao());
    *e_antes->mutable_dados_ataque() = proto.dados_ataque();
    if (e_antes->dados_ataque().empty()) {
      e_antes->add_dados_ataque();   // ataque invalido para sinalizar para apagar.
    }
  }
  if (n_desfazer != nullptr) {
    *n_desfazer = *n;
  }
}

void PreencheNotificacaoAcaoFeiticoPessoal(
    unsigned int id, const std::string& string_efeitos, float atraso_s, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  n->set_tipo(ntf::TN_ADICIONAR_ACAO);
  auto* a = n->mutable_acao();
  a->set_id_entidade_origem(id);
  a->set_tipo(ACAO_FEITICO_PESSOAL);
  a->set_pessoal(true);
  auto* por_entidade = a->add_por_entidade();
  por_entidade->set_id(id);
  por_entidade->set_texto(string_efeitos);
  a->set_afeta_pontos_vida(false);
  a->set_gera_outras_acoes(true);
  if (atraso_s != 0.0f) a->set_atraso_s(atraso_s);
  if (n_desfazer != nullptr) {
    *n_desfazer = *n;
  }
}

void PreencheNotificacaoAcaoFeiticoPessoalDispersao(
    const Tabelas& tabelas, const ArmaProto& feitico_tabelado, const std::string& id_classe, const EntidadeProto& proto,
    ntf::Notificacao* n) {
  // Gera a acao na entidade.
  AcaoProto acao = feitico_tabelado.acao().id().empty() ? AcaoProto::default_instance() : tabelas.Acao(feitico_tabelado.acao().id());
  acao.MergeFrom(feitico_tabelado.acao());
  n->set_tipo(ntf::TN_ADICIONAR_ACAO);
  *n->mutable_acao() = acao;
  n->mutable_acao()->set_id_entidade_origem(proto.id());
  n->mutable_acao()->set_id_entidade_destino(proto.id());
  *n->mutable_acao()->mutable_pos_tabuleiro() = proto.pos();
  *n->mutable_acao()->mutable_pos_entidade() = proto.pos();
}

bool FeiticoPessoalDispersao(const Tabelas& tabelas, const ArmaProto& feitico_tabelado) {
  AcaoProto acao = feitico_tabelado.acao().id().empty() ? AcaoProto::default_instance() : tabelas.Acao(feitico_tabelado.acao().id());
  acao.MergeFrom(feitico_tabelado.acao());
  return acao.pessoal() && acao.tipo() == ACAO_DISPERSAO;
}

// Retorna true se uma acao foi criada.
bool ExecutaFeitico(
    const Tabelas& tabelas, const ArmaProto& feitico_tabelado, int nivel_conjurador, const std::string& id_classe, int nivel_slot,
    const std::optional<DadosIniciativa>& dados_iniciativa, const Entidade& entidade,
    ntf::Notificacao* grupo, ntf::Notificacao* grupo_desfazer) {
  // Fazer a dispersão funcionar é complicado, porque tem que Chamar a funcao EntidadesAfetadasPorAcao para preencher os ids da acao
  // e isso depende do tabuleiro. Deixar desligado por agora.
  //if (FeiticoPessoalDispersao(tabelas, feitico_tabelado)) {
  //PreencheNotificacaoAcaoFeiticoPessoalDispersao(tabelas, feitico_tabelado, id_classe, proto, grupo->add_notificacao());
  //  return false;
  //} else if (FeiticoPessoal(tabelas, feitico_tabelado)) {
  if (FeiticoPessoal(tabelas, feitico_tabelado) && !FeiticoPessoalDispersao(tabelas, feitico_tabelado)) {
    // Aplica o efeito do feitico no personagem diretamente.
    std::vector<int> ids_unicos = IdsUnicosEntidade(entidade);
    std::string string_efeitos;
    for (const auto& efeito_adicional : feitico_tabelado.acao().efeitos_adicionais()) {
       PreencheNotificacaoEventoEfeitoAdicional(
           entidade.Id(), dados_iniciativa, nivel_conjurador, entidade, efeito_adicional, &ids_unicos, grupo->add_notificacao(), nullptr);
       string_efeitos += absl::StrFormat("%s, ", tabelas.Efeito(efeito_adicional.efeito()).nome().c_str());
    }
    if (!string_efeitos.empty()) {
      string_efeitos.pop_back();
      string_efeitos.pop_back();
    }
    // Adiciona uma acao de feitico pessoal.
    PreencheNotificacaoAcaoFeiticoPessoal(
        entidade.Id(), string_efeitos, /*atraso_s=*/0,
        grupo->add_notificacao(), grupo_desfazer != nullptr ? grupo_desfazer->add_notificacao() : nullptr);
    return false;
  } else if (FeiticoGeraAcao(tabelas, feitico_tabelado)) {
    CriaNovoAtaqueComFeitico(
        tabelas, feitico_tabelado, id_classe, nivel_slot, entidade.Proto(),
        grupo->add_notificacao(), grupo_desfazer != nullptr ? grupo_desfazer->add_notificacao() : nullptr);
    return true;
  }
  return false;

}

std::optional<std::pair<bool, std::string>> TestaConcentracao(const Tabelas& tabelas, int cd, const EntidadeProto& proto) {
  auto resultado = RolaPericia(tabelas, "concentracao", Bonus::default_instance(), proto);
  if (!resultado.has_value()) {
    LOG(ERROR) << "Pericia concentracao invalida, nunca deveria acontecer";
    return std::nullopt;
  }
  auto [rolou, valor, modificadores, texto] = *resultado;
  if (!rolou) {
    LOG(ERROR) << "Pericia concentracao nao rolada, nunca deveria acontecer";
    return std::nullopt;
  }
  const bool passou = valor >= cd;
  VLOG(1) << "compilador feliz: valor " << valor << ", modificadores: " << modificadores;
  return std::make_pair(passou, absl::StrFormat("%s: %s", passou ? "passou" : "falhou", texto.c_str()));
}

std::optional<std::pair<bool, std::string>> TestaConcentracaoSeConjurando(const Tabelas& tabelas, int delta_pv, const EntidadeProto& proto) {
  if (delta_pv >= 0) return std::nullopt;
  if (!PossuiEvento(EFEITO_CONJURANDO, proto)) return std::nullopt;
  return TestaConcentracao(tabelas, 15 - delta_pv, proto);
}

std::pair<bool, std::string> VerificaChancesDeFalha(const Tabelas& tabelas, const ArmaProto& feitico_tabelado, int nivel, const EntidadeProto& proto) {
  std::string texto;
  if (PossuiEvento(EFEITO_ENREDADO, proto)) {
    auto resultado = TestaConcentracao(tabelas, 15 + nivel, proto);
    if (resultado.has_value()) {
      auto [passou, texto_enredado] = *resultado;
      if (!passou) {
        return *resultado;
      }
      texto = texto_enredado;
    }
  }
  // TODO: efeitos metamagicos como magia silenciosa anulam isso.
  if (PossuiEvento(EFEITO_ENSURDECIDO, proto) && feitico_tabelado.componente().verbal()) {
    int d100 = RolaDado(100);
    if (d100 <= 20) {
      return std::make_pair(false, absl::StrFormat("falha por surdez%s%s", texto.empty() ? "" : ", ", texto.c_str()));
    }
    texto = absl::StrFormat("passou em falha por surdez%s%s", texto.empty() ? "" : ", ", texto.c_str());
  }
  return std::make_pair(true, texto);
}

const ArmaProto& FeiticoTabeladoOuConvertido(
    const Tabelas& tabelas, const std::string& id_classe, bool conversao_espontanea, int nivel, int indice, const EntidadeProto& proto) {
  const EntidadeProto::InfoConhecido& ic =
      ClassePrecisaMemorizar(tabelas, id_classe)
        ? FeiticoConhecido(id_classe, FeiticoParaLancar(id_classe, nivel, indice, proto), proto)
        : FeiticoConhecido(id_classe, nivel, indice, proto);
  // Busca feitico. Se precisa memorizar, busca de ParaLancar, caso contrario, os valores que vem aqui ja sao
  // dos feiticos conhecidos.
  std::string id_conversao_espontanea;
  if (conversao_espontanea) {
    id_conversao_espontanea = tabelas.FeiticoConversaoEspontanea(
        id_classe, nivel, Mau(proto) ? Tabelas::COI_INFLIGIR : Tabelas::COI_CURA);
  }
  const auto& feitico_tabelado = tabelas.Feitico(id_conversao_espontanea.empty() ? ic.id() : id_conversao_espontanea);
  if (!feitico_tabelado.has_id()) {
    // Nao ha entrada.
    LOG(ERROR) << "Nao ha feitico id '" << ic.id() << "' tabelado: InfoConhecido: " << ic.ShortDebugString()
               << ". id_classe: " << id_classe << ", nivel: " << nivel << ", indice: " << indice;
  }
  return feitico_tabelado;
}

bool NotificacaoConsequenciaFeitico(
    const Tabelas& tabelas, const std::optional<DadosIniciativa>& dados_iniciativa, const std::string& id_classe, bool conversao_espontanea, int nivel, int indice, const Entidade& entidade, ntf::Notificacao* grupo) {
  const auto& proto = entidade.Proto();
  float atraso_s = 0.0f;
  const auto& feitico_tabelado = FeiticoTabeladoOuConvertido(tabelas, id_classe, conversao_espontanea, nivel, indice, entidade.Proto());
  if (!feitico_tabelado.has_id()) {
    return false;
  }
  auto [passou_falha, texto_falha] = VerificaChancesDeFalha(tabelas, feitico_tabelado, nivel, proto);
  if (!passou_falha || !texto_falha.empty()) {
    grupo->add_notificacao()->Swap(NovaNotificacaoAcaoTextoLogada(texto_falha, proto, atraso_s).get());
    atraso_s += 2.0f;
    if (!passou_falha) return false;
  }
  const int nivel_conjurador = NivelConjurador(id_classe, proto);
  if (feitico_tabelado.tempo_execucao_rodadas() > 0) {
    EntidadeProto ep;
    ep.set_id(entidade.Id());
    auto* ic = ep.add_info_classes();
    ic->set_id(id_classe);
    ic->set_nivel_conjurador(nivel_conjurador);
    if (dados_iniciativa.has_value()) {
      ep.set_iniciativa(dados_iniciativa->iniciativa);
      ep.set_modificador_iniciativa(dados_iniciativa->modificador);
    }
    auto* eda = ep.add_dados_ataque();
    eda->set_id_arma(feitico_tabelado.id());
    eda->set_nivel_slot(nivel);

    std::string entidade_str;
    google::protobuf::TextFormat::PrintToString(ep, &entidade_str);
    auto ids_unicos = IdsUnicosEntidade(entidade);
    PreencheNotificacaoEventoComComplementoStr(
        entidade.Id(), std::nullopt, /*origem=*/feitico_tabelado.nome(), EFEITO_CONJURANDO, entidade_str,
        feitico_tabelado.tempo_execucao_rodadas(), &ids_unicos, grupo->add_notificacao(), nullptr);
    grupo->add_notificacao()->Swap(NovaNotificacaoAcaoTextoLogada(absl::StrFormat(
          "conjurando por %d rodada%s",
          feitico_tabelado.tempo_execucao_rodadas(), feitico_tabelado.tempo_execucao_rodadas() > 1 ? "s" : ""), proto, atraso_s).get());
    return false;
  }
  return ExecutaFeitico(tabelas, feitico_tabelado, nivel_conjurador, id_classe, nivel, dados_iniciativa, entidade, grupo, nullptr);
}

// Retorna: id_classe, nivel, indice slot, usado e id entidade na notificacao de alterar feitico. Em caso de
// erro, retorna nivel negativo.
std::tuple<std::string, int, int, bool, unsigned int> DadosNotificacaoAlterarFeitico(const ntf::Notificacao& n) {
  // Bizarramente, make_tuple da pau de linker se usar Entidade::IdInvalido.
  unsigned int id_invalido = Entidade::IdInvalido;
  if (n.entidade().feiticos_classes().size() != 1) {
    return std::make_tuple("", -1, 0, false, id_invalido);
  }
  const auto& fc = n.entidade().feiticos_classes(0);
  if (fc.mapa_feiticos_por_nivel().size() != 1) {
    return std::make_tuple("", -1, 0, false, id_invalido);
  }
  const auto& [nivel, fn] = *fc.mapa_feiticos_por_nivel().begin();
  if (fc.mapa_feiticos_por_nivel().begin()->second.para_lancar().size() != 1) {
    return std::make_tuple("", -1, 0, false, id_invalido);
  }
  const auto& pl = fn.para_lancar(0);
  return std::make_tuple(
      fc.id_classe(), fn.has_nivel() ? fn.nivel() : nivel, pl.indice(), pl.usado(), n.entidade().id());
}

std::unique_ptr<ntf::Notificacao> NotificacaoEscolherFeitico(
    bool conversao_espontanea, const std::string& id_classe, int nivel, const EntidadeProto& proto) {
  const auto& fc = FeiticosClasse(id_classe, proto);
  std::unique_ptr<ntf::Notificacao> n(new ntf::Notificacao);
  if (fc.id_classe().empty()) {
    n->set_tipo(ntf::TN_ERRO);
    n->set_erro(absl::StrFormat("Classe '%s' nao encontrada no proto", id_classe.c_str()));
    return n;
  }
  n->set_tipo(ntf::TN_ABRIR_DIALOGO_ESCOLHER_FEITICO);
  n->mutable_entidade()->set_id(proto.id());
  *n->mutable_entidade()->mutable_info_classes() = proto.info_classes();
  *n->mutable_entidade()->mutable_dados_ataque() = proto.dados_ataque();  // pro ataque criado.
  auto* nfc = n->mutable_entidade()->add_feiticos_classes();
  *nfc = fc;
  nfc->set_conversao_espontanea(conversao_espontanea);

  for (int i = nivel + 1; i <= 9; ++i) {
    n->mutable_entidade()->mutable_feiticos_classes(0)->mutable_mapa_feiticos_por_nivel()->erase(i);
  }
  return n;
}

const EntidadeProto::InfoConhecido& FeiticoConhecido(
    const std::string& id_classe, int nivel, int indice, const EntidadeProto& proto) {
  const auto& fn = FeiticosNivel(id_classe, nivel, proto);
  if (indice < 0 || indice >= fn.conhecidos().size()) return EntidadeProto::InfoConhecido::default_instance();
  return fn.conhecidos(indice);
}

const EntidadeProto::InfoLancar& FeiticoParaLancar(
    const std::string& id_classe, int nivel, int indice, const EntidadeProto& proto) {
  const auto& fn = FeiticosNivel(id_classe, nivel, proto);
  if (indice < 0 || indice >= fn.para_lancar().size()) return EntidadeProto::InfoLancar::default_instance();
  return fn.para_lancar(indice);
}

bool EntidadeVulneravelElemento(const EntidadeProto& proto, DescritorAtaque elemento) {
  return c_any(proto.dados_defesa().vulnerabilidades(), (int)elemento);
}

bool EntidadeImuneEfeito(const EntidadeProto& proto, TipoEfeito efeito) {
  return c_any(proto.dados_defesa().imunidade_efeitos(), (int)efeito);
}

bool EntidadeImuneElemento(const EntidadeProto& proto, DescritorAtaque elemento) {
  if (elemento == DESC_NENHUM) return false;
  if ((elemento == DESC_MENTAL || elemento == DESC_MENTAL_PADRAO_VISIVEL) && ImuneAcaoMental(proto)) return true; 
  if (elemento == DESC_MENTAL_PADRAO_VISIVEL && NaoEnxerga(proto)) return true;
  const auto& dd = proto.dados_defesa();
  return c_any(dd.imunidades(), (int)elemento);
}

bool EntidadeImuneFeitico(const EntidadeProto& proto, const std::string& id) {
  return c_any_of(proto.dados_defesa().imunidade_feiticos(), [&id] (const DadosDefesa::ImunidadeFeitico& imf) { return imf.id_feitico() == id; } );
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
    case DESC_MAGICO: return "mágico";
    case DESC_ESTOURANTE: return "estourante";
    case DESC_PERFURANTE: return "perfurante";
    case DESC_CORTANTE: return "cortante";
  }
  LOG(WARNING) << "descritor desconhecido: " << descritor;
  return "desconhecido";
}

ResultadoImunidadeOuResistencia ImunidadeOuResistenciaParaElemento(
    int delta_pv, const DadosAtaque& da, const EntidadeProto& proto, DescritorAtaque elemento,
    std::unique_ptr<ntf::Notificacao>* n_efeito, ntf::Notificacao* grupo_desfazer) {
  ResultadoImunidadeOuResistencia resultado;
  if (delta_pv >= 0) {
    return resultado;
  }
  if (da.id_arma() == "missil_magico" && PossuiEvento(EFEITO_ESCUDO_ARCANO, proto)) {
    resultado.resistido = std::abs(delta_pv);
    resultado.texto = "imunidade por escudo arcano";
    resultado.causa = ALT_IMUNIDADE;
    return resultado;
  }
  if (elemento == DESC_NENHUM) {
    return resultado;
  }
  if (EntidadeImuneElemento(proto, elemento)) {
    resultado.resistido = std::abs(delta_pv);
    resultado.texto = absl::StrFormat("imunidade: %s", TextoDescritor(elemento));
    resultado.causa = ALT_IMUNIDADE;
    return resultado;
  }

  if (da.id_arma() == "missil_magico" && PossuiEvento(EFEITO_BROCHE_ESCUDO, proto)) {
    if (auto eventos = EventosTipo(EFEITO_BROCHE_ESCUDO, proto); !eventos.empty() && !eventos[0]->complementos().empty()) {
      const auto* evento = eventos[0];
      resultado.resistido = std::min(std::abs(delta_pv), evento->complementos(0));
      if (resultado.resistido > 0) {
        n_efeito->reset(new ntf::Notificacao);
        auto [proto_antes, proto_depois] =
            PreencheNotificacaoEntidadeProto(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, proto, n_efeito->get());
        *proto_antes->add_evento() = *evento;
        auto* evento_depois = proto_depois->add_evento();
        *evento_depois = *evento;
        evento_depois->mutable_complementos()->Set(0, std::max(0, evento_depois->complementos(0) - resultado.resistido));
        *grupo_desfazer->add_notificacao() = *n_efeito->get();
      }

      if (resultado.resistido == std::abs(delta_pv)) {
        resultado.texto = "imunidade por broche do escudo";
        resultado.causa = ALT_IMUNIDADE;
        // Cria uma atualizacao do efeito.
        return resultado;
      }

      if (resultado.resistido > 0) {
        resultado.texto = "resistência por broche do escudo";
        resultado.causa = ALT_RESISTENCIA;
        return resultado;
      }
    }
  }

  // Resistencia ao tipo de ataque.
  const ResistenciaElementos* resistencia = EntidadeResistenciaElemento(proto, elemento);
  if (resistencia == nullptr) {
    return resultado;
  }

  resultado.causa = ALT_RESISTENCIA;
  const int valor_efetivo = resistencia->valor();
  resultado.resistido = valor_efetivo > std::abs(delta_pv) ? std::abs(delta_pv) : valor_efetivo;
  resultado.texto = absl::StrFormat("resistência: %s: %d", TextoDescritor(elemento), valor_efetivo);
  resultado.resistencia = resistencia;
  return resultado;
}

std::optional<std::pair<int, std::string>> VulnerabilidadeParaElemento(
    int delta_pv, const EntidadeProto& proto, DescritorAtaque elemento) {
  if (delta_pv >= 0) {
    return std::nullopt;
  }
  if (elemento == DESC_NENHUM) {
    return std::nullopt;
  }
  std::pair<int, std::string> resultado;
  if (EntidadeVulneravelElemento(proto, elemento)) {
    int novo_delta = delta_pv * 0.5;
    return std::make_pair(delta_pv * 0.5, absl::StrFormat("vulnerável a: %s, aumentando em %d", TextoDescritor(elemento), std::abs(novo_delta)));
  }
  return std::nullopt;
}

std::tuple<int, std::string, int> AlteraDeltaPontosVidaPorUmaReducao(
    int delta_pv, const ReducaoDano& rd, const google::protobuf::RepeatedField<int>& descritores) {
  VLOG(1) << "rd: " << rd.DebugString();
  for (int d : descritores) {
    VLOG(1) << "descritor ataque: " << TextoDescritor(d);
  }
  if (rd.tipo_combinacao() == COMB_E) {
    for (const auto& descritor_defesa : rd.descritores()) {
      if (c_none(descritores, descritor_defesa)) {
        delta_pv += rd.valor();
        return std::make_tuple(std::min(0, delta_pv), absl::StrFormat("redução de dano: %d", rd.valor()), rd.has_id_unico() ? rd.id_unico() : -1);
      } else {
        VLOG(1) << "descritor defesa: " << TextoDescritor(descritor_defesa) << " bateu";
      }
    }
    return std::make_tuple(delta_pv, "redução de dano: não aplicada", -1);
  } else {
    for (const auto& descritor : rd.descritores()) {
      if (c_any(descritores, descritor)) {
        return std::make_tuple(delta_pv, "redução de dano: não aplicada", -1);
      }
    }
    delta_pv += rd.valor();
    return std::make_tuple(std::min(0, delta_pv), absl::StrFormat("redução de dano: %d", rd.valor()), rd.has_id_unico() ? rd.id_unico() : -1);
  }
}

std::tuple<int, std::string, int> AlteraDeltaPontosVidaPorReducao(
    int delta_pv, const EntidadeProto& proto, const google::protobuf::RepeatedField<int>& descritores) {
  const auto& dd = proto.dados_defesa();
  if (dd.reducao_dano().empty()) {
    return std::make_tuple(delta_pv, "", -1);
  }
  std::tuple<int, std::string, int> melhor_reducao = std::make_tuple(delta_pv, "", -1);
  for (const auto& rd : dd.reducao_dano()) {
    int delta_alterado;
    std::string texto;
    int id_unico;
    std::tie(delta_alterado, texto, id_unico) = AlteraDeltaPontosVidaPorUmaReducao(delta_pv, rd, descritores);
    if (delta_alterado > delta_pv) {
      melhor_reducao = std::tie(delta_alterado, texto, id_unico);
    }
  }
  return melhor_reducao;
}

std::tuple<int, std::string> AlteraDeltaPontosVidaPorReducaoBarbaro(int delta_pv, const EntidadeProto& proto) {
  if (proto.dados_defesa().reducao_dano_barbaro() == 0) return std::make_tuple(delta_pv, "");
  return std::make_tuple(std::min(0, delta_pv + proto.dados_defesa().reducao_dano_barbaro()),
      absl::StrFormat("redução de dano de bárbaro: %d", proto.dados_defesa().reducao_dano_barbaro()));
}

ResultadoReducaoDano AlteraDeltaPontosVidaPorMelhorReducao(
    int delta_pv, const EntidadeProto& proto, const google::protobuf::RepeatedField<int>& descritores) {
  int delta_barbaro;
  std::string texto_barbaro;
  std::tie(delta_barbaro, texto_barbaro) = AlteraDeltaPontosVidaPorReducaoBarbaro(delta_pv, proto);
  int delta_outros;
  std::string texto_outros;
  int id_unico;
  std::tie(delta_outros, texto_outros, id_unico) = AlteraDeltaPontosVidaPorReducao(delta_pv, proto, descritores);
  std::string texto_final;
  if (delta_barbaro != delta_pv && delta_barbaro >= delta_outros) {
    delta_pv = delta_barbaro;
    texto_final = texto_barbaro;
    id_unico = -1;
  } else if (delta_outros != delta_pv && delta_outros > delta_barbaro) {
    delta_pv = delta_outros;
    texto_final = texto_outros;
  }
  ResultadoReducaoDano resultado;
  resultado.delta_pv = delta_pv;
  resultado.texto = texto_final;
  resultado.id_unico = id_unico;
  return resultado;
}

bool AcaoAfetaAlvo(const AcaoProto& acao_proto, const Entidade& entidade, std::string* texto) {
  if (!entidade.PodeSerAfetadoPorAcoes()) {
    return false;
  }
  if (acao_proto.nao_afeta_origem() && entidade.Id() == acao_proto.id_entidade_origem()) {
    return false;
  }
  if (c_any_of(acao_proto.nao_afeta_tipo(), [&entidade](const int tipo) {
    return entidade.TemTipoDnD(static_cast<TipoDnD>(tipo));
  })) {
    return false;
  }
  if (c_any_of(acao_proto.nao_afeta_sub_tipo(), [&entidade](const int sub_tipo) {
    return entidade.TemSubTipoDnD(static_cast<SubTipoDnD>(sub_tipo));
  })) {
    return false;
  }
  if (acao_proto.has_afeta_apenas_tamanhos_menores_ou_igual_a() &&
      entidade.Proto().tamanho() > acao_proto.afeta_apenas_tamanhos_menores_ou_igual_a()) {
    *texto = "alvo muito grande";
    return false;
  }
  if (acao_proto.has_dv_mais_alto() && Nivel(entidade.Proto()) > acao_proto.dv_mais_alto()) {
    if (texto != nullptr) {
      *texto = absl::StrFormat("alvo tem mais dv que %d", acao_proto.dv_mais_alto());
    }
    return false;
  }
  if (acao_proto.has_pv_mais_alto() && entidade.PontosVida() > acao_proto.pv_mais_alto()) {
    if (texto != nullptr) {
      *texto = absl::StrFormat("criatura tem mais pv que %d", acao_proto.pv_mais_alto());
    }
    return false;
  }

  // Aqui tem que testar o vazio pra afetar, caso contrario o return da false e
  // nao afeta ninguem.
  if (acao_proto.afeta_apenas().empty()) return true;
  return c_any_of(acao_proto.afeta_apenas(), [&entidade] (const int tipo) {
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

const ItemMagicoProto& ItemTabela(
    const Tabelas& tabelas, TipoItem tipo, const std::string& id) {
  switch (tipo) {
    case TipoItem::TIPO_ANEL: return tabelas.Anel(id);
    case TipoItem::TIPO_MANTO: return tabelas.Manto(id);
    case TipoItem::TIPO_LUVAS: return tabelas.Luvas(id);
    case TipoItem::TIPO_BRACADEIRAS: return tabelas.Bracadeiras(id);
    case TipoItem::TIPO_POCAO: return tabelas.Pocao(id);
    case TipoItem::TIPO_AMULETO: return tabelas.Amuleto(id);
    case TipoItem::TIPO_BOTAS: return tabelas.Botas(id);
    case TipoItem::TIPO_CHAPEU: return tabelas.Chapeu(id);
    case TipoItem::TIPO_PERGAMINHO_ARCANO: return tabelas.PergaminhoArcano(id);
    case TipoItem::TIPO_PERGAMINHO_DIVINO: return tabelas.PergaminhoDivino(id);
    case TipoItem::TIPO_ITEM_MUNDANO: return tabelas.ItemMundano(id);
    case TipoItem::TIPO_VARINHA: return tabelas.Varinha(id);
    default: ;
  }
  LOG(ERROR) << "tipo de item invalido: " << TipoItem_Name(tipo);
  return ItemMagicoProto::default_instance();
}

const ItemMagicoProto& ItemTabela(const Tabelas& tabelas, const ItemMagicoProto& item) {
  return ItemTabela(tabelas, item.tipo(), item.id());
}

void AdicionaEventosItemMagicoContinuo(
    const Tabelas& tabelas, ItemMagicoProto* item, std::vector<int>* ids_unicos, EntidadeProto* proto) {
  int tam_antes = ids_unicos->size();
  AdicionaEventoItemMagicoContinuo(ItemTabela(tabelas, *item), ids_unicos, proto);
  std::vector<int> ids_adicionados(ids_unicos->begin() + tam_antes, ids_unicos->end());
  for (int id_unico : ids_adicionados) {
    item->add_ids_efeitos(id_unico);
  }
}

void PreencheComTesourosEmUso(const EntidadeProto& proto, bool manter_uso, EntidadeProto* proto_alvo) {
  const std::vector<const ItemMagicoProto*> todos_itens = TodosItensExcetoPocoes(proto);
  for (const auto* item : todos_itens) {
    if (!item->em_uso()) continue;
    auto* item_novo = ItensProtoMutavel(item->tipo(), proto_alvo)->Add();
    *item_novo = *item;
    item_novo->set_em_uso(manter_uso);
  }
}

std::string NomeTipoItem(TipoItem tipo) {
  std::string nome = TipoItem_Name(tipo);
  return nome.find("TIPO_") == 0 ? nome.substr(5) : nome;
}

const RepeatedPtrField<ent::EntidadeProto::ArmaArmaduraOuEscudoPersonagem>& ArmasArmadurasOuEscudosProto(
    TipoTesouro tipo, const EntidadeProto& proto) {
  switch (tipo) {
    case TT_ARMA: return proto.tesouro().armas();
    case TT_ARMADURA: return proto.tesouro().armaduras();
    case TT_ESCUDO: return proto.tesouro().escudos();
    default: ;
  }
  LOG(ERROR) << "Tipo de item invalido (" << (int)tipo << "), retornando armas";
  return proto.tesouro().armas();
}

const RepeatedPtrField<ent::ItemMagicoProto>& ItensProto(
    TipoItem tipo, const EntidadeProto& proto) {
  switch (tipo) {
    case TipoItem::TIPO_ANEL: return proto.tesouro().aneis();
    case TipoItem::TIPO_MANTO: return proto.tesouro().mantos();
    case TipoItem::TIPO_LUVAS: return proto.tesouro().luvas();
    case TipoItem::TIPO_BRACADEIRAS: return proto.tesouro().bracadeiras();
    case TipoItem::TIPO_POCAO: return proto.tesouro().pocoes();
    case TipoItem::TIPO_AMULETO: return proto.tesouro().amuletos();
    case TipoItem::TIPO_BOTAS: return proto.tesouro().botas();
    case TipoItem::TIPO_CHAPEU: return proto.tesouro().chapeus();
    case TipoItem::TIPO_PERGAMINHO_ARCANO: return proto.tesouro().pergaminhos_arcanos();
    case TipoItem::TIPO_PERGAMINHO_DIVINO: return proto.tesouro().pergaminhos_divinos();
    case TipoItem::TIPO_ITEM_MUNDANO: return proto.tesouro().itens_mundanos();
    case TipoItem::TIPO_VARINHA: return proto.tesouro().varinhas();
    default: ;
  }
  LOG(ERROR) << "Tipo de item invalido (" << (int)tipo << "), retornando anel";
  return proto.tesouro().aneis();
}

RepeatedPtrField<ent::EntidadeProto::ArmaArmaduraOuEscudoPersonagem>* ArmasArmadurasOuEscudosProtoMutavel(
    TipoTesouro tipo, EntidadeProto* proto) {
  switch (tipo) {
    case TT_ARMA: return proto->mutable_tesouro()->mutable_armas();
    case TT_ARMADURA: return proto->mutable_tesouro()->mutable_armaduras();
    case TT_ESCUDO: return proto->mutable_tesouro()->mutable_escudos();
    default: ;
  }
  LOG(ERROR) << "Tipo de item invalido (" << (int)tipo << "), retornando armas";
  return proto->mutable_tesouro()->mutable_armas();
}

RepeatedPtrField<ent::ItemMagicoProto>* ItensProtoMutavel(
    TipoItem tipo, EntidadeProto* proto) {
  switch (tipo) {
    case TipoItem::TIPO_ANEL: return proto->mutable_tesouro()->mutable_aneis();
    case TipoItem::TIPO_MANTO: return proto->mutable_tesouro()->mutable_mantos();
    case TipoItem::TIPO_LUVAS: return proto->mutable_tesouro()->mutable_luvas();
    case TipoItem::TIPO_BRACADEIRAS: return proto->mutable_tesouro()->mutable_bracadeiras();
    case TipoItem::TIPO_POCAO: return proto->mutable_tesouro()->mutable_pocoes();
    case TipoItem::TIPO_AMULETO: return proto->mutable_tesouro()->mutable_amuletos();
    case TipoItem::TIPO_BOTAS: return proto->mutable_tesouro()->mutable_botas();
    case TipoItem::TIPO_CHAPEU: return proto->mutable_tesouro()->mutable_chapeus();
    case TipoItem::TIPO_PERGAMINHO_ARCANO: return proto->mutable_tesouro()->mutable_pergaminhos_arcanos();
    case TipoItem::TIPO_PERGAMINHO_DIVINO: return proto->mutable_tesouro()->mutable_pergaminhos_divinos();
    case TipoItem::TIPO_ITEM_MUNDANO: return proto->mutable_tesouro()->mutable_itens_mundanos();
    case TipoItem::TIPO_VARINHA: return proto->mutable_tesouro()->mutable_varinhas();
    default: ;
  }
  LOG(ERROR) << "Tipo de item invalido (" << (int)tipo << "), retornando anel";
  return proto->mutable_tesouro()->mutable_aneis();
}

// Retorna true se os itens forem do mesmo tipo, tiverem os mesmos efeitos e complementos.
bool MesmoItem(const ItemMagicoProto& item1, const ItemMagicoProto& item2) {
  if (item1.id() != item2.id() || item1.tipo() != item2.tipo() ||
      item1.ids_efeitos().size() != item2.ids_efeitos().size() ||
      item1.complementos().size() != item2.complementos().size() ||
      item1.complementos_str().size() != item2.complementos_str().size()) {
    return false;
  }
  for (int i = 0; i < item1.ids_efeitos().size(); ++i) {
    if (item1.ids_efeitos(i) != item2.ids_efeitos(i)) return false;
  }
  for (int i = 0; i < item1.complementos().size(); ++i) {
    if (item1.complementos(i) != item2.complementos(i)) return false;
  }
  for (int i = 0; i < item1.complementos_str().size(); ++i) {
    if (item1.complementos_str(i) != item2.complementos_str(i)) return false;
  }
  return true;
}

void RemoveItem(const ItemMagicoProto& item, EntidadeProto* proto) {
  auto* itens_do_tipo = ItensProtoMutavel(item.tipo(), proto);
  for (int i = 0; i < itens_do_tipo->size(); ++i) {
    const auto& item_proto = itens_do_tipo->Get(i);
    if (MesmoItem(item_proto, item)) {
      itens_do_tipo->DeleteSubrange(i, 1);
      return;
    }
  }
}

std::vector<const ItemMagicoProto*> TodosItensExcetoPocoes(const EntidadeProto& proto) {
  const auto& tesouro = proto.tesouro();
  std::vector<const RepeatedPtrField<ItemMagicoProto>*> itens_agrupados = {
    &tesouro.aneis(), &tesouro.mantos(), &tesouro.luvas(), &tesouro.bracadeiras(), &tesouro.amuletos(), &tesouro.botas(), &tesouro.chapeus(), &tesouro.itens_mundanos(), &tesouro.varinhas(), &tesouro.pergaminhos_divinos(), &tesouro.pergaminhos_arcanos(),
  };
  std::vector<const ItemMagicoProto*> itens;
  for (const auto* itens_grupo : itens_agrupados) {
    std::copy(itens_grupo->pointer_begin(), itens_grupo->pointer_end(), std::back_inserter(itens));
  }
  return itens;
}

std::vector<const ItemMagicoProto*> TodosItens(const EntidadeProto& proto) {
  std::vector<const ItemMagicoProto*> itens = TodosItensExcetoPocoes(proto);
  const auto& tesouro = proto.tesouro();
  std::copy(tesouro.pocoes().pointer_begin(), tesouro.pocoes().pointer_end(), std::back_inserter(itens));
  return itens;
}

std::vector<ItemMagicoProto*> TodosItensExcetoPocoes(EntidadeProto* proto) {
  auto* tesouro = proto->mutable_tesouro();
  std::vector<RepeatedPtrField<ItemMagicoProto>*> itens_agrupados = {
    tesouro->mutable_aneis(), tesouro->mutable_mantos(), tesouro->mutable_luvas(), tesouro->mutable_bracadeiras(),
    tesouro->mutable_amuletos(), tesouro->mutable_botas(), tesouro->mutable_chapeus(), tesouro->mutable_itens_mundanos(),
    tesouro->mutable_varinhas(), tesouro->mutable_pergaminhos_divinos(), tesouro->mutable_pergaminhos_arcanos(),
  };
  std::vector<ItemMagicoProto*> itens;
  for (auto* itens_grupo : itens_agrupados) {
    std::copy(itens_grupo->pointer_begin(), itens_grupo->pointer_end(), std::back_inserter(itens));
  }
  return itens;
}

int CuraAcelerada(const EntidadeProto& proto) {
  // Rodar manualmente os bonus e retornar o maior?
  return BonusTotal(proto.dados_defesa().cura_acelerada());
}

Vector3 RotationMatrixToAngles(const Matrix3& matrix) {
  float sy = sqrt(matrix[0] * matrix[0] + matrix[1] * matrix[1]);
  const bool singular = sy < 1e-6; // If
  float x, y, z;
  if (!singular) {
    x = atan2(matrix[5], matrix[8]);
    y = atan2(-matrix[2], sy);
    z = atan2(matrix[1], matrix[0]);
  } else {
    x = atan2(-matrix[7], matrix[4]);
    y = atan2(-matrix[2], sy);
    z = 0;
  }
  return Vector3(x, y, z);
}

Matrix4 MatrizDecomposicaoPai(const EntidadeProto& pai) {
  Matrix4  m_translacao_pai;
  m_translacao_pai.translate(pai.pos().x(), pai.pos().y(), pai.pos().z());
  Matrix4 m_rotacao_pai;
  m_rotacao_pai.rotateX(pai.rotacao_x_graus());
  m_rotacao_pai.rotateY(pai.rotacao_y_graus());
  m_rotacao_pai.rotateZ(pai.rotacao_z_graus());
  Matrix4 m_escala_pai;
  m_escala_pai.scale(pai.escala().x(), pai.escala().y(), pai.escala().z());
  return m_translacao_pai * m_rotacao_pai * m_escala_pai;
}

void DecompoeFilho(const Matrix4& matriz_pai, EntidadeProto* filho) {
  Matrix4 m_translacao_sub;
  m_translacao_sub.translate(filho->pos().x(), filho->pos().y(), filho->pos().z());
  Matrix4 m_rotacao_sub;
  m_rotacao_sub.rotateX(filho->rotacao_x_graus());
  m_rotacao_sub.rotateY(filho->rotacao_y_graus());
  m_rotacao_sub.rotateZ(filho->rotacao_z_graus());
  Matrix4 m_escala_sub;
  m_escala_sub.scale(filho->escala().x(), filho->escala().y(), filho->escala().z());
  // final.
  Matrix4 m_final = matriz_pai  * m_translacao_sub * m_rotacao_sub * m_escala_sub ;

  auto* pos = filho->mutable_pos();
  pos->set_x(m_final[12]);
  pos->set_y(m_final[13]);
  pos->set_z(m_final[14]);
  auto* escala = filho->mutable_escala();
  escala->set_x(Vector3(m_final[0], m_final[1], m_final[2]).length());
  escala->set_y(Vector3(m_final[4], m_final[5], m_final[6]).length());
  escala->set_z(Vector3(m_final[8], m_final[9], m_final[10]).length());

  Matrix3 m_final_rotacao;
  m_final_rotacao[0] = m_final[0] / escala->x();
  m_final_rotacao[1] = m_final[1] / escala->x();
  m_final_rotacao[2] = m_final[2] / escala->x();
  m_final_rotacao[3] = m_final[4] / escala->y();
  m_final_rotacao[4] = m_final[5] / escala->y();
  m_final_rotacao[5] = m_final[6] / escala->y();
  m_final_rotacao[6] = m_final[8] / escala->z();
  m_final_rotacao[7] = m_final[9] / escala->z();
  m_final_rotacao[8] = m_final[10] / escala->z();

  Vector3 vr = RotationMatrixToAngles(m_final_rotacao);
  filho->set_rotacao_x_graus(vr.x * RAD_PARA_GRAUS);
  filho->set_rotacao_y_graus(vr.y * RAD_PARA_GRAUS);
  filho->set_rotacao_z_graus(vr.z * RAD_PARA_GRAUS);
}

bool ModeloDesligavel(const Tabelas& tabelas, const ModeloDnD& modelo) {
  return tabelas.EfeitoModelo(modelo.id_efeito()).desligavel();
}

ModeloDnD* EncontraModelo(TipoEfeitoModelo id_efeito, EntidadeProto* proto) {
  for (auto& modelo : *proto->mutable_modelos()) {
    if (modelo.id_efeito() == id_efeito) return &modelo;
  }
  return nullptr;
}

bool EntidadeTemModeloDesligavelLigado(const Tabelas& tabelas, const EntidadeProto& proto) {
  return c_any_of(proto.modelos(), [&tabelas] (const ModeloDnD& modelo) {
    return ModeloDesligavel(tabelas, modelo) && modelo.ativo();
  });
}

bool FeiticoPessoal(const Tabelas& tabelas, const ArmaProto& feitico_tabelado) {
  AcaoProto acao = feitico_tabelado.acao().id().empty() ? AcaoProto::default_instance() : tabelas.Acao(feitico_tabelado.acao().id());
  acao.MergeFrom(feitico_tabelado.acao());
  return acao.pessoal();
}

int NivelMaximoFeitico(const Tabelas& tabelas, const std::string& id_classe, int nivel_para_conjuracao) {
  if (nivel_para_conjuracao <= 0) return 0;
  nivel_para_conjuracao = std::min(nivel_para_conjuracao, 20);
  const auto& classe_tabelada = tabelas.Classe(id_classe);
  const auto& progressao_feitico = classe_tabelada.progressao_feitico();
  if (progressao_feitico.para_nivel().empty()) return 0;

  int modificador = progressao_feitico.nao_possui_nivel_zero() ? 0 : 1;
  if (nivel_para_conjuracao < progressao_feitico.para_nivel().size()) {
    return progressao_feitico.para_nivel(nivel_para_conjuracao).magias_por_dia().size() - modificador;
  } else {
    LOG(WARNING) << "Nao deveria acontecer, nivel de conjuracao maior que tabelado, id: " << id_classe << ", nivel: " << nivel_para_conjuracao;
    return progressao_feitico.para_nivel(progressao_feitico.para_nivel().size() - 1).magias_por_dia().size() - modificador;
  }
}

bool PodeConjurarFeitico(const ArmaProto& feitico, int nivel_maximo_feitico, const std::string& id_classe_para_magia) {
  for (const auto& ic : feitico.info_classes()) {
    if (ic.id() == id_classe_para_magia) {
      if (ic.nivel() <= nivel_maximo_feitico) return true;
      else return false;
    }
  }
  return false;
}

bool TemFeiticoLista(const std::string& id_feitico, const EntidadeProto& proto) {
  const auto& feitico = Tabelas::Unica().Feitico(id_feitico);
  for (const auto& ic : feitico.info_classes()) {
    if (Nivel(ic.id(), proto) > 0) return true;
  }
  return false;
}

std::string NomeTipoBonus(TipoBonus tipo) {
  std::string nome = TipoBonus_Name(tipo);
  if (nome.size() > 3) {
    nome = nome.substr(3);
  }
  return nome;
}

std::string BonusParaString(const Bonus& bonus) {
  std::string resumo;
  for (const auto& bi : bonus.bonus_individual()) {
    for (const auto& po : bi.por_origem()) {
      if (po.valor() == 0) continue;
      resumo += absl::StrFormat("%s (%s): %d\n", NomeTipoBonus(bi.tipo()).c_str(), po.origem().c_str(), po.valor());
    }
  }
  if (!resumo.empty()) {
    resumo.pop_back();
  }
  return resumo;
}

bool DestrezaNaCA(const EntidadeProto& proto) {
  const bool possui_esquiva_sobrenatural = PossuiHabilidadeEspecial("esquiva_sobrenatural", proto);
  if (!possui_esquiva_sobrenatural && proto.surpreso()) {
    return false;
  }
  if (PossuiEvento(EFEITO_ATORDOADO, proto)) {
    return false;
  }
  if (!possui_esquiva_sobrenatural && PossuiEvento(EFEITO_CEGO, proto) && !PossuiTalento("lutar_as_cegas", proto)) {
    return false;
  }
  if (PossuiEventoNaoPossuiOutro(EFEITO_PARALISIA, EFEITO_MOVIMENTACAO_LIVRE, proto)) {
    return false;
  }
  return true;
}

bool DestrezaNaCAContraAtaque(
    const DadosAtaque* da, const EntidadeProto& proto_defesa, const EntidadeProto& proto_ataque) {
  if (!DestrezaNaCA(proto_defesa)) return false;
  if (da == nullptr) return true;
  if (!proto_defesa.agarrado_a().empty() && c_none(proto_defesa.agarrado_a(), proto_ataque.id())) {
    // Defensor agarrado e atacante não.
    return false;
  }

  const bool atacante_invisivel_para_defensor = PossuiEvento(EFEITO_CEGO, proto_defesa) ||
    (PossuiEventoNaoPossuiOutro(EFEITO_INVISIBILIDADE, EFEITO_POEIRA_OFUSCANTE, proto_ataque) && !PossuiEvento(EFEITO_VER_INVISIVEL, proto_defesa));
  if (atacante_invisivel_para_defensor &&
      (!PossuiTalento("lutar_as_cegas", proto_defesa) || DaParaTipoAtaque(*da) == TipoAtaque::DISTANCIA)) {
    return false;
  }
  return true;
}

bool PermiteEscudo(const EntidadeProto& proto) {
  if (PossuiEvento(EFEITO_ATORDOADO, proto)) {
    return false;
  }
  return true;
}

bool TalentoComEscudo(const std::string& id_escudo, const EntidadeProto& proto) {
  if (id_escudo == "broquel" || id_escudo.find("leve_") == 0 || id_escudo.find("pesado_") == 0) {
    return PossuiTalento("usar_escudo", proto);
  } else if (id_escudo == "corpo") {
    return PossuiTalento("usar_escudo_corpo", proto);
  }
  return true;
}

bool TalentoComArmadura(const ArmaduraOuEscudoProto& armadura_tabelada, const EntidadeProto& proto) {
  if (armadura_tabelada.id().empty()) return true;
  if (armadura_tabelada.has_tipo()) {
    if (armadura_tabelada.tipo() == TA_LEVE) {
      return PossuiTalento("usar_armadura_leve", proto);
    } else if (armadura_tabelada.tipo() == TA_MEDIO) {
      return PossuiTalento("usar_armadura_media", proto);
    } else if (armadura_tabelada.tipo() == TA_PESADO) {
      return PossuiTalento("usar_armadura_pesada", proto);
    }
  }
  return true;
}

const std::string& IdArmaBase(const ArmaProto& arma_tabelada) {
  return arma_tabelada.has_id_arma_base() ? arma_tabelada.id_arma_base() : arma_tabelada.id();
}

bool TalentoComArma(const ArmaProto& arma_tabelada, const EntidadeProto& proto) {
  const std::string& id = IdArmaBase(arma_tabelada);
  if (id.empty()) return true;
  if (c_any(arma_tabelada.categoria(), CAT_ARMA_NATURAL)) return true;
  if (arma_tabelada.has_categoria_pericia()) {
    auto categoria = arma_tabelada.categoria_pericia();
    bool familiaridade = false;
    if (c_any(Tabelas::Unica().Raca(proto.raca()).familiaridade_com_arma(), arma_tabelada.id())) {
      familiaridade = true;
    }
    if (categoria == CATPER_SIMPLES) {
      return PossuiTalento("usar_armas_simples", proto) || PossuiTalento("usar_uma_arma_simples", id, proto);
    } else if (categoria == CATPER_COMUM || familiaridade) {
      return PossuiTalento("usar_armas_comuns", proto) || PossuiTalento("usar_arma_comum", id, proto);
    } else if (categoria == CATPER_EXOTICA) {
      return PossuiTalento("usar_arma_exotica", id, proto);
    } else if (categoria == CATPER_EXOTICA_ARMA_FOGO) {
      return PossuiTalento("usar_arma_exotica_fogo", proto);
    }
  }
  return true;
}

int NivelFeiticoParaClasse(const std::string& id_classe, const ArmaProto& feitico) {
  for (const auto& ic : feitico.info_classes()) {
    if (ic.id() == id_classe) return ic.nivel();
  }
  return -1;
}

namespace {

void PassaAtributosReferencia(const ArmaProto& feitico, const Entidade& referencia, EntidadeProto* modelo) {
  if (referencia.TemIniciativa()) {
    modelo->set_iniciativa(referencia.Iniciativa());
  }
  modelo->set_rotulo(absl::StrFormat("%s (conjurado por %s)", modelo->rotulo().c_str(), RotuloEntidade(referencia).c_str()));
  if (referencia.Proto().has_cor()) {
    Cor cor_destino = !modelo->has_cor() ? CorParaProto(COR_BRANCA) : modelo->cor();
    CombinaCorComPeso(0.3, referencia.Proto().cor(), &cor_destino);
    *modelo->mutable_cor() = cor_destino;
  }
  if (feitico.escola() == "conjuracao") {
    modelo->set_conjurada(true);
    if (referencia.PossuiTalento("potencializar_invocacao")) {
      AtribuiBonus(4, TB_MELHORIA, "potencializar_invocacao", BonusAtributo(TA_FORCA, modelo));
      AtribuiBonus(4, TB_MELHORIA, "potencializar_invocacao", BonusAtributo(TA_CONSTITUICAO, modelo));
      if (!modelo->dados_vida().empty()) {
        const int bonus_pv = NivelPersonagem(*modelo) * 2;
        modelo->set_dados_vida(absl::StrFormat("%s+%d", modelo->dados_vida().c_str(), bonus_pv));
      }
    }
  }
}

}  // namespace

void PreencheModeloComParametros(const ArmaProto& feitico, const Modelo::Parametros& parametros, const Entidade& referencia, EntidadeProto* modelo) {
  const auto& classe_feitico_ativa = referencia.Proto().classe_feitico_ativa();
  const int nivel_conjurador = referencia.NivelConjuradorParaMagia(classe_feitico_ativa, feitico);
  const int nivel_feitico = NivelFeiticoParaClasse(classe_feitico_ativa, feitico);
  VLOG(1) << "usando nivel: " << nivel_conjurador << " para classe: " << referencia.Proto().classe_feitico_ativa();
  PassaAtributosReferencia(feitico, referencia, modelo);
  if (parametros.has_tipo_duracao()) {
    int duracao_rodadas = -1;
    switch (parametros.tipo_duracao()) {
      case TD_RODADAS_NIVEL:
        duracao_rodadas = nivel_conjurador;
        break;
      case TD_MINUTOS_NIVEL:
        duracao_rodadas = nivel_conjurador * MINUTOS_PARA_RODADAS;
        break;
      case TD_10_MINUTOS_NIVEL:
        duracao_rodadas = 10 * nivel_conjurador * MINUTOS_PARA_RODADAS;
        break;
      case TD_HORAS_NIVEL:
        duracao_rodadas = nivel_conjurador * HORAS_PARA_RODADAS;
        break;
      case TD_2_HORAS_NIVEL:
        duracao_rodadas = 2 * nivel_conjurador * HORAS_PARA_RODADAS;
        break;
      default:
        break;
    }
    if (duracao_rodadas > 0) {
      // Procura a duracao no modelo, caso nao haja, coloca.
      for (int i = 0; i < modelo->evento_size(); ++i) {
        if (StringSemUtf8(modelo->evento(i).descricao()) == "duracao") {
          modelo->mutable_evento()->DeleteSubrange(i, 1);
          break;
        }
      }
      auto* evento = modelo->add_evento();
      evento->set_descricao("duração");
      evento->set_id_efeito(EFEITO_OUTRO);
      evento->set_rodadas(duracao_rodadas);
      // Acha o id para a referencia, ja que o modelo nao tem nada.
      evento->set_id_unico(AchaIdUnicoEvento(referencia.Proto().evento()));
    }
  }
  if (parametros.multiplicador_nivel_dano() > 0 && nivel_conjurador > 0) {
    int modificador = nivel_conjurador * parametros.multiplicador_nivel_dano();
    if (parametros.has_maximo_modificador_dano()) {
      modificador = std::min(parametros.maximo_modificador_dano(), modificador);
    }
    std::string dano_str;
    if (!parametros.dano_fixo().empty()) {
      dano_str = absl::StrFormat("%s%+d", parametros.dano_fixo().c_str(), modificador);
    } else {
      dano_str = absl::StrFormat("%dd%d", modificador, parametros.dado_dano_por_nivel());
    }
    for (auto& da : *modelo->mutable_dados_ataque()) {
      da.set_dano_basico(dano_str);
    }
  }
  if (parametros.has_tipo_modificador_ataque()) {
    int modificador_ataque = -100;
    int num_ataques = 1;
    switch (parametros.tipo_modificador_ataque()) {
      case TMA_BBA_MAIS_ATRIBUTO_CONJURACAO: {
        int ref_bba = referencia.BonusBaseAtaque();
        num_ataques = 1 + (ref_bba / 6);
        modificador_ataque = ref_bba + referencia.ModificadorAtributoConjuracao();
        break;
      }
      case TMA_BBA_NIVEL_CONJURADOR:
        modificador_ataque = nivel_conjurador;
        break;
      default:
        break;
    }
    // Hack para quando o personagem nao tiver modificadores.
    if (modificador_ataque > -50) {
      // Adiciona uma classe ficticia com o bba. Os ataques adicionais receberao o bonus dentro de outros_bonus_ataque.
      auto* ic = modelo->add_info_classes();
      ic->set_id("modelo");
      ic->set_nivel(modificador_ataque);
      ic->set_bba(modificador_ataque);
      // Salva o ataque como modelo para usar ao criar os demais.
      auto da = modelo->dados_ataque().empty() ? DadosAtaque() : modelo->dados_ataque(0);
      modelo->clear_dados_ataque();
      int ordem_ataque = 0;
      while (num_ataques-- > 0) {
        auto* nda = modelo->add_dados_ataque();
        *nda = da;
        if (ordem_ataque > 0) AtribuiBonus(ordem_ataque * -5, TB_SEM_NOME, "ataque_adicional", nda->mutable_bonus_ataque());
        ++ordem_ataque;
      }
    }
  }
  if (parametros.has_tipo_modificador_salvacao()) {
    switch (parametros.tipo_modificador_salvacao()) {
      case TMS_MODIFICADOR_CONJURACAO:
        for (auto& da : *modelo->mutable_dados_ataque()) {
          da.set_dificuldade_salvacao(
              nivel_feitico >= 0
              ? 10 + nivel_feitico + referencia.ModificadorAtributoConjuracao()
              : da.dificuldade_salvacao() + referencia.ModificadorAtributoConjuracao());
          if (!da.acao_fixa().efeitos_adicionais().empty()) {
            for (auto& ea : *da.mutable_acao_fixa()->mutable_efeitos_adicionais()) {
              if (ea.has_modificador_rodadas()) {
                ea.set_rodadas(Rodadas(nivel_conjurador, ea, da.acao_fixa(), referencia.Proto(), /*alvo=*/nullptr));
              }
            }
          }
        }
        break;
      case TMS_NENHUM:
      default:
        break;
    }
  }
  if (!parametros.rotulo_especial().empty()) {
    *modelo->mutable_rotulo_especial() = parametros.rotulo_especial();
  }
  VLOG(1) << "Modelo parametrizado: " << modelo->DebugString();
}

std::vector<std::string> DominiosClasse(const std::string& id_classe, const EntidadeProto& proto) {
  const auto& fc = ent::FeiticosClasse(id_classe, proto);
  std::vector<std::string> dominios;
  for (const auto& dominio : fc.dominios()) {
    dominios.push_back(dominio);
  }
  return dominios;
}

const InfoClasse& ClasseParaLancarPergaminho(
    const Tabelas& tabelas, TipoMagia tipo_magia, const std::string& id_feitico, const EntidadeProto& proto) {
  const auto& feitico_tabelado = tabelas.Feitico(id_feitico);
  std::vector<const InfoClasse*> candidatos;
  for (const auto& ic : proto.info_classes()) {
    const auto& classe_tabelada = tabelas.Classe(ic.id());
    if (tipo_magia != TM_NENHUMA && classe_tabelada.tipo_magia() != tipo_magia) continue;
    std::vector<std::string> dominios = DominiosClasse(ic.id(), proto);
    bool de_dominio = false;
    for (const auto& dominio : dominios) {
      if (NivelFeiticoParaClasse(dominio, feitico_tabelado) > -1) {
        de_dominio = true;
        break;
      }
    }
    const bool feitico_de_classe = de_dominio || NivelFeiticoParaClasse(ic.has_id_para_magia() ? ic.id_para_magia() : ic.id(), feitico_tabelado) > -1;
    if (!feitico_de_classe) continue;

    candidatos.push_back(&ic);
  }
  if (candidatos.empty()) return InfoClasse::default_instance();
  if (candidatos.size() == 1) return *candidatos[0];
  int maior_nivel = 0;
  const InfoClasse* melhor_candidato = nullptr;
  for (const auto* ic : candidatos) {
    if (ic->id() == proto.classe_feitico_ativa()) return *ic;
    int nivel_conjurador = NivelConjurador(ic->id(), proto);
    if (nivel_conjurador > maior_nivel) {
      maior_nivel = nivel_conjurador;
      melhor_candidato = ic;
    }
  }
  return melhor_candidato == nullptr ? InfoClasse::default_instance() : *melhor_candidato;
}

int NivelConjuradorParaLancarPergaminho(const Tabelas& tabelas, TipoMagia tipo_magia, const std::string& id_feitico, const EntidadeProto& proto) {
  const auto& ic = ClasseParaLancarPergaminho(tabelas, tipo_magia, id_feitico, proto);
  return ic.has_nivel_conjurador() ? ic.nivel_conjurador() : -1;
}

ResultadoPergaminho TesteLancarPergaminho(const Tabelas& tabelas, const EntidadeProto& proto, const DadosAtaque& da) {
  int nc = NivelConjuradorParaLancarPergaminho(tabelas, da.tipo_pergaminho(), da.id_arma(), proto);
  if (nc == -1) {
    // TODO pericia usar itens magicos.
    return
        ResultadoPergaminho(/*ok=*/false, /*fiasco=*/false,
        absl::StrFormat("não pode ler pergaminho do tipo %s", da.tipo_pergaminho() == TM_ARCANA ? "arcano" : "divino"));
  }
  if (nc >= da.nivel_conjurador_pergaminho()) {
    return ResultadoPergaminho(/*ok=*/true);
  }
  // Teste para lançar.
  const int dc = da.nivel_conjurador_pergaminho() + 1;
  const int d20 = RolaDado(20);
  if (d20 + nc >= dc) {
    return ResultadoPergaminho(/*ok=*/true, false, absl::StrFormat("Teste conjuração ok: %d >= %d", d20 + nc, dc));
  }
  // Fiascos com pergaminho (scroll mishaps).
  const int d20_sab = RolaDado(20);
  const int modificador_sabedoria = ModificadorAtributo(TA_SABEDORIA, proto);
  if (d20_sab + modificador_sabedoria >= 5) {
    return ResultadoPergaminho(
        /*ok=*/false, false,
        absl::StrFormat("Teste conjuração falhou: %d < %d, sem fiasco %d >= 5", d20 + nc, dc, d20_sab + modificador_sabedoria));
  }
  return ResultadoPergaminho(
      /*ok=*/false, /*fiasco=*/true,
      absl::StrFormat("FIASCO! Teste conjuração: %d < %d, teste sabedoria: %d < 5", d20 + nc, dc, d20_sab + modificador_sabedoria));
}

std::pair<bool, std::string> PodeLancarItemMagico(const Tabelas& tabelas, const EntidadeProto& proto, const DadosAtaque& da) {
  // Tipo correto.
  const auto& feitico = tabelas.Feitico(da.id_arma());
  TipoMagia tipo_magia = da.tipo_pergaminho();
  if (tipo_magia != TM_NENHUMA) {
    bool tipo_correto = tipo_magia == TM_NENHUMA;  // se nao tiver tipo, nao tem essa restricao.
    for (const auto& classe_proto : proto.info_classes()) {
      if (tabelas.Classe(classe_proto.id()).tipo_magia() == tipo_magia) {
        tipo_correto = true;
        break;
      }
    }
    if (!tipo_correto) {
      return std::make_pair(false, absl::StrFormat("incapaz de lançar magias %s", tipo_magia == TM_DIVINA ? "divina" : "arcana"));
    }
  }
  // Caso nao precise de estar na lista, o resto nao faz sentido.
  if (da.nao_precisa_ter_na_lista()) return std::make_pair(true, "");

  // Esta na lista de feiticos.
  const auto& ic = ClasseParaLancarPergaminho(tabelas, tipo_magia, da.id_arma(), proto);
  if (!ic.has_nivel_conjurador()) {
    return std::make_pair(false, absl::StrFormat("feitiço %s não está na lista do personagem", feitico.nome().c_str()));
  }
  // Se tem especializacao, é de escola permitida.
  const auto& fc = FeiticosClasse(ic.id(), proto);
  if (!fc.especializacao().empty()) {
    if (c_any(fc.escolas_proibidas(), feitico.escola())) {
      return std::make_pair(false, absl::StrFormat("feitiço %s é de escola não permitida (%s).", da.id_arma().c_str(), feitico.escola().c_str()));
    }
  }
  // Atributo minimo de conjuracao.
  if (da.varinha().empty()) {
    if (da.has_modificador_atributo_pergaminho() &&
        ModificadorAtributoConjuracao(ic.id(), proto) < da.modificador_atributo_pergaminho()) {
      return std::make_pair(
          false,
          absl::StrFormat(
            "modificador atributo de conjuração do personagem abaixo do minimo: %d < %d",
            ModificadorAtributoConjuracao(ic.id(), proto),
            da.modificador_atributo_pergaminho()));
    }
    if (const Bonus& bonus = BonusAtributo(ic.atributo_conjuracao(), proto); BonusTotal(bonus) < 10 + NivelFeiticoParaClasse(ic.id(), feitico)) {
      return std::make_pair(
          false,
          absl::StrFormat(
            "atributo de conjuração abaixo do mínimo: %d < %d",
            BonusTotal(bonus), 10 + NivelFeiticoParaClasse(ic.id(), feitico)));
    }
  }
  return std::make_pair(true, "");
}

TipoEvasao TipoEvasaoPersonagem(const EntidadeProto& proto) {
  return proto.dados_defesa().evasao();
}

void PreencheNotificacaoRemocaoEvento(const EntidadeProto& proto, TipoEfeito te, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  EntidadeProto *e_antes, *e_depois;
  std::tie(e_antes, e_depois) = PreencheNotificacaoEntidadeProto(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, proto, n);
  for (const auto& evento_proto : proto.evento()) {
    if (evento_proto.id_efeito() != te) continue;
    *e_antes->add_evento() = evento_proto;
    auto* evento_depois = e_depois->add_evento();
    *evento_depois = evento_proto;
    evento_depois->set_rodadas(-1);
  }
  if (n_desfazer != nullptr) {
    *n_desfazer = *n;
  }
}

bool PreencheInfoTextura(
    const std::string& nome, arq::tipo_e tipo, InfoTextura* info_textura,
    unsigned int* plargura, unsigned int* paltura) {
  unsigned int largura = 0, altura = 0;
  if (plargura == nullptr) {
    plargura = &largura;
  }
  if (paltura == nullptr) {
    paltura = &altura;
  }
  try {
    tex::Texturas::LeDecodificaImagemTipo(tipo, nome, info_textura, plargura, paltura);
    return true;
  } catch (...) {
    LOG(ERROR) << "Textura inválida: " << info_textura->id();
    return false;
  }
}

bool Indefeso(const EntidadeProto& proto) {
  if (PossuiEventoNaoPossuiOutro(EFEITO_PARALISIA, EFEITO_MOVIMENTACAO_LIVRE, proto)) {
    return true;
  }
  if (PossuiEvento(EFEITO_SONO, proto)) {
    return true;
  }
  if (proto.morta() || proto.inconsciente()) {
    return true;
  }
  return false;
}

void ConcatenaString(const std::string& s, std::string* alvo) {
  if (alvo == nullptr) return;
  if (alvo->empty()) *alvo = s;
  else *alvo = absl::StrFormat("%s\n%s", alvo->c_str(), s.c_str());
}

int DesviaObjetoSeAplicavel(
    const Tabelas& tabelas, int delta_pontos_vida, const Entidade& alvo, const DadosAtaque& da, Tabuleiro* tabuleiro,
    AcaoProto::PorEntidade* por_entidade, ntf::Notificacao* grupo_desfazer) {
  //LOG(INFO) << "delta_pontos_vida: " << delta_pontos_vida << ", " << da.eh_arma() << ", " << da.id_arma();
  if (delta_pontos_vida >= 0 || !da.eh_arma() || da.id_arma().empty()) return delta_pontos_vida;
  const auto& arma = tabelas.Arma(da.id_arma());
  //LOG(INFO) << "arma: " << arma.DebugString();
  if (!PossuiCategoria(CAT_DISTANCIA, arma) || !da.ataque_distancia()) return delta_pontos_vida;
  if (!DestrezaNaCA(alvo.Proto())) return delta_pontos_vida;

  const auto* talento = Talento("desviar_objetos", alvo.Proto());
  if (talento == nullptr || talento->usado_na_rodada() || alvo.Proto().surpreso()) return delta_pontos_vida;
  ConcatenaString("projétil desviado", por_entidade->mutable_texto());
  ntf::Notificacao n;
  PreencheNotificacaoObjetoDesviado(true, alvo, &n, grupo_desfazer->add_notificacao());
  tabuleiro->TrataNotificacao(n);
  return 0;
}

int DesviaMontariaSeAplicavel(
    const Tabelas& tabelas, int delta_pontos_vida, int total_ataque, const Entidade& alvo, const DadosAtaque& da,
    Tabuleiro* tabuleiro, AcaoProto::PorEntidade* por_entidade, ntf::Notificacao* grupo_desfazer) {
  if (delta_pontos_vida >= 0 || !da.acao().permite_ataque_vs_defesa() || alvo.Proto().entidades_montadas().size() != 1) return delta_pontos_vida;
  const auto* montador = tabuleiro->BuscaEntidade(alvo.Proto().entidades_montadas(0));
  if (montador == nullptr) {
    LOG(INFO) << "montador de " << RotuloEntidade(alvo) << " é null";
    return delta_pontos_vida;
  }
  const auto* talento = Talento("combate_montado", montador->Proto());
  if (talento == nullptr || talento->usado_na_rodada()) return delta_pontos_vida;
  auto pericia_cavalgar_opt = RolaPericia(tabelas, "cavalgar", Bonus::default_instance(), montador->Proto());
  if (!pericia_cavalgar_opt.has_value()) return delta_pontos_vida;
  auto [rolou, pericia_cavalgar, modificadores_cavalgar, texto] = *pericia_cavalgar_opt;
  VLOG(1) << "compilador feliz: rolou pericia: " << rolou << ", total: " << pericia_cavalgar << ", modificadores: " << modificadores_cavalgar << ", " << texto;
  if (pericia_cavalgar <= total_ataque) {
    ConcatenaString(absl::StrFormat("cavalgar %d <= %d ataque", pericia_cavalgar, total_ataque), por_entidade->mutable_texto());
    return delta_pontos_vida;
  }
  ConcatenaString(
      absl::StrFormat("ataque na montaria desviado: cavalgar %d > %d ataque", pericia_cavalgar, total_ataque), por_entidade->mutable_texto());
  ntf::Notificacao n;
  PreencheNotificacaoMontariaDesviada(true, *montador, &n, grupo_desfazer->add_notificacao());
  tabuleiro->TrataNotificacao(n);
  return 0;
}

std::pair<int, std::string> RenovaSeTiverDominioRenovacao(
    const EntidadeProto& proto, int delta_pv, tipo_dano_e tipo_dano, ntf::Notificacao* n, ntf::Notificacao* grupo_desfazer) {
  if (delta_pv >= 0) {
    return std::make_pair(delta_pv, "");
  }
  const EntidadeProto::InfoFeiticosClasse& ic = FeiticosClasse("clerigo", proto);
  if (c_none(ic.dominios(), "renovacao")) {
    return std::make_pair(delta_pv, "");
  }
  auto it = ic.poderes_dominio().find("renovacao");
  if (it != ic.poderes_dominio().end() && it->second.usado()) {
    return std::make_pair(delta_pv, "");
  }
  const EntidadeProto::PoderesDominio pd = it == ic.poderes_dominio().end() ? EntidadeProto::PoderesDominio::default_instance() : it->second;
  ntf::Notificacao nfake;
  PreencheNotificacaoAtualizacaoPontosVida(proto, delta_pv, tipo_dano, &nfake, nullptr /*desfazer*/);
  const auto& e_antes = nfake.entidade_antes();
  const auto& e_depois = nfake.entidade();
  if (!e_depois.morta() && !e_antes.inconsciente() && e_depois.inconsciente()) {
    const int d8 = RolaDado(8);
    const int mod_carisma = ModificadorAtributo(BonusAtributo(TA_CARISMA, proto));
    const int total = std::max(0, d8 + mod_carisma);
    auto [e_antes, e_depois] = PreencheNotificacaoEntidadeProto(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, proto, n);
    auto* ic_antes = FeiticosClasse("clerigo", e_antes);
    (*ic_antes->mutable_poderes_dominio())["renovacao"].set_usado(false);
    auto* ic_depois = FeiticosClasse("clerigo", e_depois);
    (*ic_depois->mutable_poderes_dominio())["renovacao"].set_usado(true);
    int disponivel_em = DIA_EM_RODADAS;
    if (!pd.taxa_refrescamento().empty()) {
      try {
        RolaValor(pd.taxa_refrescamento());
      } catch (...) {
        disponivel_em = DIA_EM_RODADAS;
      }
    }
    (*ic_depois->mutable_poderes_dominio())["renovacao"].set_disponivel_em(disponivel_em);
    if (grupo_desfazer != nullptr) {
      *grupo_desfazer->add_notificacao() = *n;
    }
    return std::make_pair(
        delta_pv + total,
        absl::StrFormat("dominio renovação %d = 1d8 + mod carisma = %d %+d", total, d8, mod_carisma));
  }
  return std::make_pair(delta_pv, "");
}

int CompartilhaDanoSeAplicavel(
    int delta_pv, const EntidadeProto& alvo, const Tabuleiro& tabuleiro, tipo_dano_e tipo_dano,
    AcaoProto::PorEntidade* por_entidade, AcaoProto* acao_proto, ntf::Notificacao* grupo_desfazer) {
  std::vector<const EntidadeProto::Evento*> evento_divisao = EventosTipo(EFEITO_PROTEGER_OUTRO, alvo);
  if (delta_pv >= 0 || evento_divisao.empty() || evento_divisao[0]->complementos().empty()) return delta_pv;

  const auto* entidade_solidaria = tabuleiro.BuscaEntidade(evento_divisao[0]->complementos(0));
  if (entidade_solidaria == nullptr) {
    ConcatenaString("dano não dividido, sem entidade", por_entidade->mutable_texto());
    return delta_pv;
  }

  // Dano deve ser dividido.
  int sobra = delta_pv - (delta_pv / 2);
  delta_pv = delta_pv / 2;
  ConcatenaString("dano dividido por 2", por_entidade->mutable_texto());

  auto* por_entidade_compartilhada = acao_proto->add_por_entidade();
  por_entidade_compartilhada->set_id(entidade_solidaria->Id());
  por_entidade_compartilhada->set_delta(sobra);
  ConcatenaString("dano solidário", por_entidade_compartilhada->mutable_texto());

  auto* nd = grupo_desfazer->add_notificacao();
  PreencheNotificacaoAtualizacaoPontosVida(*entidade_solidaria, sobra, tipo_dano, nd, nd);

  return delta_pv;
}

void PreencheNotificacaoReducaoLuzComConsequencia(int nivel, const Entidade& entidade, AcaoProto* acao_proto, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  EntidadeProto *e_antes, *e_depois;
  const float fator = 1 - std::min(1.0f, ((nivel * 10) / 100.0f));
  std::tie(e_antes, e_depois) = PreencheNotificacaoEntidade(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, entidade, n);
  *e_antes->mutable_luz() = entidade.Proto().luz();
  // Essa brincadeira do valor padrao da luz quebra tudo.
  e_antes->mutable_luz()->set_raio_m(entidade.RaioLuzMetros());
  *e_depois->mutable_luz() = entidade.Proto().luz();
  e_depois->mutable_luz()->set_raio_m(entidade.RaioLuzMetros() * fator);
  if (n_desfazer) {
    *n_desfazer = *n;
  }
  acao_proto->set_consequencia(TC_REDUZ_LUZ_ALVO);
  acao_proto->set_reducao_luz(fator);
}

void PreencheNotificacaoObjetoDesviado(bool valor, const Entidade& entidade, ntf::Notificacao* n, ntf::Notificacao* nd) {
  EntidadeProto *e_antes, *e_depois;
  std::tie(e_antes, e_depois) = PreencheNotificacaoEntidade(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, entidade, n);
  auto* talento_proto = Talento("desviar_objetos", entidade.Proto());
  if (talento_proto == nullptr) {
    LOG(ERROR) << "PreencheNotificacaoObjetoDesviado para entidade sem 'desviar_objetos'";
    return;
  }
  {
    auto* talento = e_antes->mutable_info_talentos()->add_outros();
    talento->set_id("desviar_objetos");
    talento->set_usado_na_rodada(talento_proto->usado_na_rodada());
  }
  {
    auto* talento = e_depois->mutable_info_talentos()->add_outros();
    talento->set_id("desviar_objetos");
    talento->set_usado_na_rodada(valor);
  }
  if (nd != nullptr) {
    *nd = *n;
  }
}

void PreencheNotificacaoMontariaDesviada(bool valor, const Entidade& entidade, ntf::Notificacao* n, ntf::Notificacao* nd) {
  auto [e_antes, e_depois] = PreencheNotificacaoEntidade(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, entidade, n);
  auto* talento_proto = Talento("combate_montado", entidade.Proto());
  if (talento_proto == nullptr) {
    LOG(ERROR) << "PreencheNotificacaoMontariaDesviada para entidade sem 'combate_montado'";
    return;
  }
  {
    auto* talento = e_antes->mutable_info_talentos()->add_outros();
    talento->set_id("combate_montado");
    talento->set_usado_na_rodada(talento_proto->usado_na_rodada());
  }
  {
    auto* talento = e_depois->mutable_info_talentos()->add_outros();
    talento->set_id("combate_montado");
    talento->set_usado_na_rodada(valor);
  }
  if (nd != nullptr) {
    *nd = *n;
  }
}

bool PossuiModeloAtivo(TipoEfeitoModelo efeito_modelo, const EntidadeProto& proto) {
  return c_any_of(proto.modelos(), [efeito_modelo](const ModeloDnD& modelo) {
    return modelo.id_efeito() == efeito_modelo && modelo.ativo();
  });
}

bool EscolaBoaTramaDasSombras(const ArmaProto& feitico) {
  std::vector<std::string> escolas = {"encantamento", "ilusao", "necromancia"};
  return c_any(escolas, feitico.escola());
}

bool EscolaRuimTramaDasSombras(const ArmaProto& feitico) {
  std::vector<std::string> escolas = {"evocacao", "transmutacao"};
  return c_any(escolas, feitico.escola());
}

void PreencheNotificacaoConsequenciaAlteracaoPontosVida(int pontos_vida, int dano_nao_letal, const EntidadeProto& proto, ntf::Notificacao* n) {
  EntidadeProto *e_antes, *e_depois;
  std::tie(e_antes, e_depois) = PreencheNotificacaoEntidadeProto(
      ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, proto, n);

  e_antes->set_morta(proto.morta());
  e_antes->set_caida(proto.caida());
  e_antes->set_inconsciente(proto.inconsciente());
  e_antes->set_caida(proto.caida());
  e_antes->set_incapacitada(proto.incapacitada());
  e_antes->set_nocauteada(proto.nocauteada());
  e_antes->set_aura_m(proto.aura_m());
  e_antes->set_pontos_vida(proto.pontos_vida());
  e_antes->set_dano_nao_letal(proto.dano_nao_letal());
  *e_antes->mutable_pos() = proto.pos();
  *e_antes->mutable_direcao_queda() = proto.direcao_queda();

  if (pontos_vida > dano_nao_letal) {
    e_depois->set_morta(false);
    e_depois->set_inconsciente(false);
    e_depois->set_incapacitada(false);
    e_depois->set_nocauteada(false);
    return;
  }
  if (pontos_vida == dano_nao_letal) {
    e_depois->set_morta(false);
    e_depois->set_inconsciente(false);
    if (dano_nao_letal > 0) {
      e_depois->set_nocauteada(true);
    } else {
      e_depois->set_incapacitada(true);
    }
    return;
  }
  // pv < dano_nao_letal.
  const int constituicao = BonusTotal(BonusAtributo(TA_CONSTITUICAO, proto));
  const int limiar_morte = std::min(-10, -constituicao);

  // Morto incondicional.
  if (pontos_vida <= limiar_morte) {
    e_depois->set_morta(true);
    e_depois->set_caida(true);
    e_depois->set_voadora(false);
    e_depois->set_inconsciente(true);
    e_depois->set_incapacitada(true);
    e_depois->set_nocauteada(true);
    e_depois->set_aura_m(0.0f);
    return;
  }
  // Se contar o nao letal, passa do limiar, quase morto.
  if (pontos_vida - dano_nao_letal <= limiar_morte) {
    //LOG(INFO) << "pv: " << pontos_vida << ", dano nl: " << dano_nao_letal << ", limiar: " << limiar_morte;
    // A regra nao eh clara aqui, entao extrapolei. Se contando com o dano nao letal passar do limiar, fica inconsciente.
    e_depois->set_morta(false);
    e_depois->set_caida(true);
    e_depois->set_voadora(false);
    e_depois->set_inconsciente(true);
    e_depois->set_incapacitada(true);
    e_depois->set_nocauteada(true);
    e_depois->set_aura_m(0.0f);
    return;
  }

  // Aqui é inconsciente, mas depende de talento.
  const bool duro_de_matar = PossuiTalento("duro_de_matar", proto);
  e_depois->set_nocauteada(true);
  e_depois->set_incapacitada(true);
  e_depois->set_inconsciente(!duro_de_matar);
  e_depois->set_voadora(proto.voadora() && duro_de_matar);
  e_depois->set_caida(proto.caida() || !duro_de_matar);
}

int PontosVida(const EntidadeProto& proto) {
  return proto.pontos_vida() + proto.pontos_vida_temporarios();
}

template <typename T>
void AtribuiTesouroOuCriaVazio(const T& tesouro_receber, T* tesouro_final) {
  if (tesouro_receber.empty()) {
    // Cria vazio para limpar na atualizacao.
    tesouro_final->Add();
  } else {
    // Copia os tesouros, limpando ids dos efeitos para serem recomputados.
    *tesouro_final = tesouro_receber;
    for (auto& item : *tesouro_final) {
      item.clear_ids_efeitos();
    }
  }
}

void AtribuiMoedasOuZeraSeVazio(const Moedas& moedas_receber, Moedas* moedas_final) {
  moedas_final->set_po(moedas_receber.po());
  moedas_final->set_pp(moedas_receber.pp());
  moedas_final->set_pc(moedas_receber.pc());
  moedas_final->set_pe(moedas_receber.pe());
  moedas_final->set_pl(moedas_receber.pl());
}

void AtribuiTesouroTodoOuCriaVazios(
    const EntidadeProto::DadosTesouro& tesouro_receber, EntidadeProto::DadosTesouro* tesouro_final) {
  AtribuiTesouroOuCriaVazio(tesouro_receber.pocoes(), tesouro_final->mutable_pocoes());
  AtribuiTesouroOuCriaVazio(tesouro_receber.aneis(), tesouro_final->mutable_aneis());
  AtribuiTesouroOuCriaVazio(tesouro_receber.mantos(), tesouro_final->mutable_mantos());
  AtribuiTesouroOuCriaVazio(tesouro_receber.luvas(), tesouro_final->mutable_luvas());
  AtribuiTesouroOuCriaVazio(tesouro_receber.bracadeiras(), tesouro_final->mutable_bracadeiras());
  AtribuiTesouroOuCriaVazio(tesouro_receber.amuletos(), tesouro_final->mutable_amuletos());
  AtribuiTesouroOuCriaVazio(tesouro_receber.botas(), tesouro_final->mutable_botas());
  AtribuiTesouroOuCriaVazio(tesouro_receber.chapeus(), tesouro_final->mutable_chapeus());
  AtribuiTesouroOuCriaVazio(tesouro_receber.armas(), tesouro_final->mutable_armas());
  AtribuiTesouroOuCriaVazio(tesouro_receber.armaduras(), tesouro_final->mutable_armaduras());
  AtribuiTesouroOuCriaVazio(tesouro_receber.escudos(), tesouro_final->mutable_escudos());
  AtribuiTesouroOuCriaVazio(tesouro_receber.municoes(), tesouro_final->mutable_municoes());
  AtribuiTesouroOuCriaVazio(tesouro_receber.pergaminhos_arcanos(), tesouro_final->mutable_pergaminhos_arcanos());
  AtribuiTesouroOuCriaVazio(tesouro_receber.pergaminhos_divinos(), tesouro_final->mutable_pergaminhos_divinos());
  AtribuiTesouroOuCriaVazio(tesouro_receber.itens_mundanos(), tesouro_final->mutable_itens_mundanos());
  AtribuiTesouroOuCriaVazio(tesouro_receber.varinhas(), tesouro_final->mutable_varinhas());
  AtribuiMoedasOuZeraSeVazio(tesouro_receber.moedas(), tesouro_final->mutable_moedas());
}

template <typename T>
void MergeTesouro(const T& tesouro_atual, const T& tesouro_receber, T* tesouro_final) {
  *tesouro_final = tesouro_atual;
  tesouro_final->MergeFrom(tesouro_receber);
  for (int i = tesouro_atual.size(); i < tesouro_final->size(); ++i) {
    auto* item = tesouro_final->Mutable(i);
    item->set_em_uso(false);
    item->clear_ids_efeitos();
  }
}

void MergeMoedas(const Moedas& atual, const Moedas& receber, Moedas* moedas_final) {
  moedas_final->set_po(atual.po() + receber.po());
  moedas_final->set_pp(atual.pp() + receber.pp());
  moedas_final->set_pc(atual.pc() + receber.pc());
  moedas_final->set_pl(atual.pl() + receber.pl());
  moedas_final->set_pe(atual.pe() + receber.pe());
}

void MergeTesouroTodo(const EntidadeProto::DadosTesouro& tesouro_receptor, const EntidadeProto::DadosTesouro& tesouro_receber, EntidadeProto::DadosTesouro* tesouro_final) {
  MergeTesouro(tesouro_receptor.pocoes(), tesouro_receber.pocoes(), tesouro_final->mutable_pocoes());
  MergeTesouro(tesouro_receptor.aneis(), tesouro_receber.aneis(), tesouro_final->mutable_aneis());
  MergeTesouro(tesouro_receptor.mantos(), tesouro_receber.mantos(), tesouro_final->mutable_mantos());
  MergeTesouro(tesouro_receptor.luvas(), tesouro_receber.luvas(), tesouro_final->mutable_luvas());
  MergeTesouro(tesouro_receptor.bracadeiras(), tesouro_receber.bracadeiras(), tesouro_final->mutable_bracadeiras());
  MergeTesouro(tesouro_receptor.amuletos(), tesouro_receber.amuletos(), tesouro_final->mutable_amuletos());
  MergeTesouro(tesouro_receptor.botas(), tesouro_receber.botas(), tesouro_final->mutable_botas());
  MergeTesouro(tesouro_receptor.chapeus(), tesouro_receber.chapeus(), tesouro_final->mutable_chapeus());
  MergeTesouro(tesouro_receptor.pergaminhos_arcanos(), tesouro_receber.pergaminhos_arcanos(), tesouro_final->mutable_pergaminhos_arcanos());
  MergeTesouro(tesouro_receptor.pergaminhos_divinos(), tesouro_receber.pergaminhos_divinos(), tesouro_final->mutable_pergaminhos_divinos());
  MergeTesouro(tesouro_receptor.itens_mundanos(), tesouro_receber.itens_mundanos(), tesouro_final->mutable_itens_mundanos());
  MergeTesouro(tesouro_receptor.varinhas(), tesouro_receber.varinhas(), tesouro_final->mutable_varinhas());
  MergeTesouro(tesouro_receptor.armas(), tesouro_receber.armas(), tesouro_final->mutable_armas());
  MergeTesouro(tesouro_receptor.armaduras(), tesouro_receber.armaduras(), tesouro_final->mutable_armaduras());
  MergeTesouro(tesouro_receptor.escudos(), tesouro_receber.escudos(), tesouro_final->mutable_escudos());
  MergeTesouro(tesouro_receptor.municoes(), tesouro_receber.municoes(), tesouro_final->mutable_municoes());
  MergeMoedas(tesouro_receptor.moedas(), tesouro_receber.moedas(), tesouro_final->mutable_moedas());
}

// T normalmente é ItemMagicoProto, mas pode ser outros tipo, como armas e armaduras.
template <class T>
void RemoveTesouroDoado(const RepeatedPtrField<T>& tesouro_doado,
                        RepeatedPtrField<T>* tesouro_final) {
  if (tesouro_doado.empty()) return;
  std::set<int, std::greater<int>> indices_a_remover;
  for (const auto& td : tesouro_doado) {
    //LOG(INFO) << "Removendo: " << td.ShortDebugString() << " do tesouro final";
    int candidato = -1;
    for (int i = 0; i < tesouro_final->size(); ++i) {
      //LOG(INFO) << "candidato a remocao: " << tesouro_final->Get(i).ShortDebugString() << " do tesouro final";
      if (tesouro_final->Get(i).id() != td.id()) continue;
      //LOG(INFO) << "encontrei possivel";
      if (IdsUnicosIguais(tesouro_final->Get(i).ids_efeitos(), td.ids_efeitos())) {
        // Achou sem duvidas.
        indices_a_remover.insert(i);
        candidato = -1;
        //LOG(INFO) << "match exato";
        break;
      }
      // É um candidato.
      candidato = i;
    }
    // Nao achou exato, usa o candidato.
    if (candidato != -1) {
      //LOG(WARNING) << "Nao deu match exato, provavelmente houve mudanca de estado na janela da doacao";
      indices_a_remover.insert(candidato);
    }
  }
  for (int i : indices_a_remover) {
    //LOG(INFO) << "Removendo tesouro final indice " << i;
    tesouro_final->DeleteSubrange(i, 1);
  }
  if (!indices_a_remover.empty() && tesouro_final->empty()) {
    tesouro_final->Add();  // para sinalizar que ta vazio.
  }
}

void RemoveMoedasDoadas(const Moedas& moedas_doadas, Moedas* moedas_final) {
  if (moedas_doadas.has_po()) moedas_final->set_po(0);
  if (moedas_doadas.has_pp()) moedas_final->set_pp(0);
  if (moedas_doadas.has_pc()) moedas_final->set_pc(0);
  if (moedas_doadas.has_pl()) moedas_final->set_pl(0);
  if (moedas_doadas.has_pe()) moedas_final->set_pe(0);
}

void RemoveTesourosDoados(const EntidadeProto::DadosTesouro& tesouro_doado, EntidadeProto::DadosTesouro* tesouro_final) {
  RemoveTesouroDoado(tesouro_doado.pocoes(), tesouro_final->mutable_pocoes());
  RemoveTesouroDoado(tesouro_doado.aneis(), tesouro_final->mutable_aneis());
  RemoveTesouroDoado(tesouro_doado.mantos(), tesouro_final->mutable_mantos());
  RemoveTesouroDoado(tesouro_doado.luvas(), tesouro_final->mutable_luvas());
  RemoveTesouroDoado(tesouro_doado.bracadeiras(), tesouro_final->mutable_bracadeiras());
  RemoveTesouroDoado(tesouro_doado.amuletos(), tesouro_final->mutable_amuletos());
  RemoveTesouroDoado(tesouro_doado.botas(), tesouro_final->mutable_botas());
  RemoveTesouroDoado(tesouro_doado.chapeus(), tesouro_final->mutable_chapeus());
  RemoveTesouroDoado(tesouro_doado.pergaminhos_arcanos(), tesouro_final->mutable_pergaminhos_arcanos());
  RemoveTesouroDoado(tesouro_doado.pergaminhos_divinos(), tesouro_final->mutable_pergaminhos_divinos());
  RemoveTesouroDoado(tesouro_doado.itens_mundanos(), tesouro_final->mutable_itens_mundanos());
  RemoveTesouroDoado(tesouro_doado.armas(), tesouro_final->mutable_armas());
  RemoveTesouroDoado(tesouro_doado.armaduras(), tesouro_final->mutable_armaduras());
  RemoveTesouroDoado(tesouro_doado.escudos(), tesouro_final->mutable_escudos());
  RemoveTesouroDoado(tesouro_doado.municoes(), tesouro_final->mutable_municoes());
  RemoveTesouroDoado(tesouro_doado.varinhas(), tesouro_final->mutable_varinhas());
  RemoveMoedasDoadas(tesouro_doado.moedas(), tesouro_final->mutable_moedas());
}

void MergeMensagemTesouro(TipoItem tipo, const RepeatedPtrField<ItemMagicoProto>& tesouro, std::string* texto) {
  for (const auto& item : tesouro) {
    texto->append("\n");
    texto->append(item.nome().empty() ? ItemTabela(Tabelas::Unica(), tipo, item.id()).nome() : item.nome());
  }
}

void MergeMensagemArmaArmaduraOuEscudo(
    TipoTesouro tipo, const RepeatedPtrField<EntidadeProto::ArmaArmaduraOuEscudoPersonagem>& itens, std::string* texto) {
  for (const auto& item : itens) {
    texto->append("\n");
    texto->append(item.nome().empty() ? item.id_tabela() : item.nome());
  }
}

void MergeMensagemMoedas(const Moedas& moedas, std::string* texto) {
  if (moedas.po() <= 0 && moedas.pp() <= 0 && moedas.pc() <= 0 && moedas.pe() <= 0 && moedas.pl() <= 0) return;
  texto->append("\n");
  if (moedas.po() > 0) { texto->append(absl::StrFormat("%d PO, ", moedas.pl())); }
  if (moedas.pl() > 0) { texto->append(absl::StrFormat("%d PL, ", moedas.pl())); }
  if (moedas.pp() > 0) { texto->append(absl::StrFormat("%d PP, ", moedas.pp())); }
  if (moedas.pc() > 0) { texto->append(absl::StrFormat("%d PC, ", moedas.pc())); }
  if (moedas.pe() > 0) { texto->append(absl::StrFormat("%d PE, ", moedas.pe())); }
  texto->resize(texto->size() - 2);
}

void MergeMensagensTesouro(const EntidadeProto::DadosTesouro& tesouro, const Tabelas& tabelas, std::string* texto) {
  // Os lambdas aqui devem ter o retorno explicit, caso contrario o C++ assumira copia e a referencia retornada pela funcao ficara invalida.
  MergeMensagemTesouro(TIPO_POCAO, tesouro.pocoes(), texto);
  MergeMensagemTesouro(TIPO_ANEL, tesouro.aneis(), texto);
  MergeMensagemTesouro(TIPO_MANTO, tesouro.mantos(), texto);
  MergeMensagemTesouro(TIPO_LUVAS, tesouro.luvas(), texto);
  MergeMensagemTesouro(TIPO_BRACADEIRAS, tesouro.bracadeiras(), texto);
  MergeMensagemTesouro(TIPO_AMULETO, tesouro.amuletos(), texto);
  MergeMensagemTesouro(TIPO_BOTAS, tesouro.botas(), texto);
  MergeMensagemTesouro(TIPO_CHAPEU, tesouro.chapeus(), texto);
  MergeMensagemTesouro(TIPO_PERGAMINHO_ARCANO, tesouro.pergaminhos_arcanos(), texto);
  MergeMensagemTesouro(TIPO_PERGAMINHO_DIVINO, tesouro.pergaminhos_divinos(), texto);
  MergeMensagemTesouro(TIPO_VARINHA, tesouro.varinhas(), texto);
  MergeMensagemTesouro(TIPO_ITEM_MUNDANO, tesouro.itens_mundanos(), texto);
  MergeMensagemTesouro(TIPO_VARINHA, tesouro.varinhas(), texto);
  MergeMensagemArmaArmaduraOuEscudo(TT_ARMA, tesouro.armas(), texto);
  MergeMensagemArmaArmaduraOuEscudo(TT_ARMADURA, tesouro.armaduras(), texto);
  MergeMensagemArmaArmaduraOuEscudo(TT_ESCUDO, tesouro.escudos(), texto);
  MergeMensagemMoedas(tesouro.moedas(), texto);
}

void LimpaMoedas(Moedas* moedas) {
  moedas->set_po(0);
  moedas->set_pp(0);
  moedas->set_pc(0);
  moedas->set_pl(0);
  moedas->set_pe(0);
}

void CriaTesouroTodoVazio(EntidadeProto::DadosTesouro* tesouro) {
  tesouro->set_tesouro("");
  tesouro->add_pocoes();
  tesouro->add_aneis();
  tesouro->add_mantos();
  tesouro->add_luvas();
  tesouro->add_bracadeiras();
  tesouro->add_amuletos();
  tesouro->add_botas();
  tesouro->add_chapeus();
  tesouro->add_pergaminhos_arcanos();
  tesouro->add_pergaminhos_divinos();
  tesouro->add_armas();
  tesouro->add_armaduras();
  tesouro->add_escudos();
  tesouro->add_municoes();
  tesouro->add_itens_mundanos();
  tesouro->add_varinhas();
  LimpaMoedas(tesouro->mutable_moedas());
}

void DoadorPerdeTesouroTipo(TipoTesouro tipo, const Entidade& doador, EntidadeProto* e_antes, EntidadeProto* e_depois) {
  if (TipoItem_IsValid(tipo)) {
    auto ti = static_cast<TipoItem>(tipo);
    AtribuiTesouroOuCriaVazio(ItensProto(ti, doador.Proto()), ItensProtoMutavel(ti, e_antes));
    auto* itens_perdidos = ItensProtoMutavel(ti, e_depois);
    itens_perdidos->Clear();
    itens_perdidos->Add();
  } else {
    AtribuiTesouroOuCriaVazio(ArmasArmadurasOuEscudosProto(tipo, doador.Proto()), ArmasArmadurasOuEscudosProtoMutavel(tipo, e_antes));
    auto* itens_perdidos = ArmasArmadurasOuEscudosProtoMutavel(tipo, e_depois);
    itens_perdidos->Clear();
    itens_perdidos->Add();
  }
}

void ReceptorGanhaTesouroDoadoTipo(TipoTesouro tipo, const Entidade& receptor, const Entidade& doador, EntidadeProto* e_antes, EntidadeProto* e_depois) {
  if (TipoItem_IsValid(tipo)) {
    auto ti = static_cast<TipoItem>(tipo);
    AtribuiTesouroOuCriaVazio(ItensProto(ti, receptor.Proto()), ItensProtoMutavel(ti, e_antes));
    MergeTesouro(ItensProto(ti, receptor.Proto()), ItensProto(ti, doador.Proto()), ItensProtoMutavel(ti, e_depois));
  } else {
    AtribuiTesouroOuCriaVazio(ArmasArmadurasOuEscudosProto(tipo, receptor.Proto()), ArmasArmadurasOuEscudosProtoMutavel(tipo, e_antes));
    MergeTesouro(ArmasArmadurasOuEscudosProto(tipo, receptor.Proto()), ArmasArmadurasOuEscudosProto(tipo, doador.Proto()), ArmasArmadurasOuEscudosProtoMutavel(tipo, e_depois));
  }
}

std::string GeraMensagemTesouroDoado(TipoTesouro tipo, const Entidade& doador) {
  std::string texto;
  if (TipoItem_IsValid(tipo)) {
    const auto& tesouro_doador = ItensProto(static_cast<TipoItem>(tipo), doador.Proto());
    MergeMensagemTesouro(static_cast<TipoItem>(tipo), tesouro_doador, &texto);
  } else {
    MergeMensagemArmaArmaduraOuEscudo(tipo, ArmasArmadurasOuEscudosProto(tipo, doador.Proto()), &texto);
  }
  return texto;
}

void PreencheNotificacoesTransicaoUmTipoTesouro(
    const Tabelas& tabelas, TipoTesouro tipo, const Entidade& doador, const Entidade& receptor, ntf::Notificacao* n_grupo, ntf::Notificacao* n_desfazer) {
  {
    // Doador perde so o tipo passado.
    auto [n_perdeu, e_antes, e_depois] = NovaNotificacaoFilha(
        ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, doador.Proto(), n_grupo);
    DoadorPerdeTesouroTipo(tipo, doador, e_antes, e_depois);
    if (n_desfazer != nullptr) {
      *n_desfazer->add_notificacao() = *n_perdeu;
    }
  }
  {
    // Receptor ganha alem do que ja tem.
    auto [n_ganhou, e_antes, e_depois] = NovaNotificacaoFilha(
        ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, receptor.Proto(), n_grupo);
    ReceptorGanhaTesouroDoadoTipo(tipo, receptor, doador, e_antes, e_depois);
    if (n_desfazer != nullptr) {
      *n_desfazer->add_notificacao() = *n_ganhou;
    }
  }
  {
    // Texto de transicao.
    auto* acao = NovaNotificacaoFilha(ntf::TN_ADICIONAR_ACAO, n_grupo)->mutable_acao();
    acao->set_tipo(ACAO_DELTA_PONTOS_VIDA);
    std::string texto = GeraMensagemTesouroDoado(tipo, doador);
    acao->set_texto(texto);
    acao->add_por_entidade()->set_id(receptor.Id());
  }
}

void PreencheNotificacoesTransicaoTesouro(
    const Tabelas& tabelas, const Entidade& doador, const Entidade& receptor, ntf::Notificacao* n_grupo, ntf::Notificacao* n_desfazer) {
  {
    // Doador perde tudo.
    auto [n_perdeu, e_antes, e_depois] = NovaNotificacaoFilha(
        ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, doador.Proto(), n_grupo);

    auto* tesouro_perdeu_antes = e_antes->mutable_tesouro();
    const auto& tesouro_transicao_antes = doador.Proto().tesouro();
    tesouro_perdeu_antes->set_tesouro(tesouro_transicao_antes.tesouro());
    AtribuiTesouroTodoOuCriaVazios(tesouro_transicao_antes, tesouro_perdeu_antes);

    auto* tesouro_perdeu = e_depois->mutable_tesouro();
    CriaTesouroTodoVazio(tesouro_perdeu);

    if (n_desfazer != nullptr) {
      *n_desfazer->add_notificacao() = *n_perdeu;
    }
  }
  {
    // Receptor ganha alem do que ja tem.
    auto [n_ganhou, e_antes, e_depois] = NovaNotificacaoFilha(
        ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, receptor.Proto(), n_grupo);
    auto* tesouro_ganhou_antes = e_antes->mutable_tesouro();
    const auto& tesouro_receptor = receptor.Proto().tesouro();
    const std::string& tesouro_str_corrente = tesouro_receptor.tesouro();
    tesouro_ganhou_antes->set_tesouro(tesouro_str_corrente);
    AtribuiTesouroTodoOuCriaVazios(tesouro_receptor, tesouro_ganhou_antes);

    auto* tesouro_ganhou = e_depois->mutable_tesouro();
    const auto& tesouro_receber = doador.Proto().tesouro();
    tesouro_ganhou->set_tesouro(
        tesouro_str_corrente + (tesouro_str_corrente.empty() ? "" : "\n") + tesouro_receber.tesouro());
    MergeTesouroTodo(tesouro_receptor, tesouro_receber, tesouro_ganhou);

    if (n_desfazer != nullptr) {
      *n_desfazer->add_notificacao() = *n_ganhou;
    }
  }
  {
    // Texto de transicao.
    auto* acao = NovaNotificacaoFilha(ntf::TN_ADICIONAR_ACAO, n_grupo)->mutable_acao();
    acao->set_tipo(ACAO_DELTA_PONTOS_VIDA);
    const auto& tesouro_doador = doador.Proto().tesouro();
    std::string texto = tesouro_doador.tesouro();
    // Os lambdas aqui devem ter o retorno explicit, caso contrario o C++ assumira copia e a referencia retornada pela funcao ficara invalida.
    MergeMensagensTesouro(tesouro_doador, tabelas, &texto);
    acao->set_texto(texto);
    acao->add_por_entidade()->set_id(receptor.Id());
    //*acao->mutable_pos_entidade() = receptor->Pos();
  }
}

void PreencheNotificacaoArrumarIds(
    const EntidadeProto& proto, const std::unordered_map<unsigned int, unsigned int>& mapa_ids_adicionados, ntf::Notificacao* grupo_notificacoes) {
  EntidadeProto proto_novo;
  proto_novo.set_id(proto.id());
  bool mudou = false;
  if (proto.has_montado_em()) {
    if (auto it = mapa_ids_adicionados.find(proto.montado_em()); it != mapa_ids_adicionados.end()) {
      proto_novo.set_montado_em(it->second);
      mudou = true;
    }
  }
  if (proto.has_familiar()) {
    if (auto it = mapa_ids_adicionados.find(proto.familiar()); it != mapa_ids_adicionados.end()) {
      proto_novo.set_familiar(it->second);
      mudou = true;
    }
  }
  if (c_any_of(proto.entidades_montadas(), [&mapa_ids_adicionados](unsigned int id) { return mapa_ids_adicionados.find(id) != mapa_ids_adicionados.end(); } )) {
    for (const auto& id : proto.entidades_montadas()) {
      if (auto it = mapa_ids_adicionados.find(id); it != mapa_ids_adicionados.end()) {
        proto_novo.add_entidades_montadas(it->second);
      } else {
        proto_novo.add_entidades_montadas(id);
      }
    }
    mudou = true;
  }
  if (c_any_of(proto.agarrado_a(), [&mapa_ids_adicionados](unsigned int id) { return mapa_ids_adicionados.find(id) != mapa_ids_adicionados.end(); } )) {
    for (const auto& id : proto.agarrado_a()) {
      if (auto it = mapa_ids_adicionados.find(id); it != mapa_ids_adicionados.end()) {
        proto_novo.add_agarrado_a(it->second);
      } else {
        proto_novo.add_agarrado_a(id);
      }
    }
    mudou = true;
  }
  if (mudou) {
    auto* n_filha = grupo_notificacoes->add_notificacao();
    n_filha->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL);
    *n_filha->mutable_entidade() = proto_novo;
    VLOG(2) << "PreencheNotificacaoArrumarIds: " << proto_novo.DebugString();
  }
}

void PreencheNotificacoesDoacaoParcialTesouro(
    const Tabelas& tabelas, const ntf::Notificacao& notificacao_doacao,
    const EntidadeProto& proto_doador, const EntidadeProto& proto_receptor, ntf::Notificacao* n_grupo, ntf::Notificacao* n_desfazer) {
  EntidadeProto::DadosTesouro tesouro_ganho;
  {
    // Doador fica como na notificacao.
    ntf::Notificacao* n_perdeu;
    EntidadeProto* e_antes;
    EntidadeProto* e_depois;
    std::tie(n_perdeu, e_antes, e_depois) = NovaNotificacaoFilha(
        ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, proto_doador, n_grupo);
    AtribuiTesouroTodoOuCriaVazios(proto_doador.tesouro(), e_antes->mutable_tesouro());
    // Remove os tesouros doados.
    *e_depois->mutable_tesouro() = proto_doador.tesouro();
    RemoveTesourosDoados(notificacao_doacao.entidade().tesouro(), e_depois->mutable_tesouro());
    if (n_desfazer != nullptr) {
      *n_desfazer->add_notificacao() = *n_perdeu;
    }
  }
  {
    // Receptor: recebe o tesouro.
    ntf::Notificacao* n_ganhou;
    EntidadeProto* e_antes;
    EntidadeProto* e_depois;
    std::tie(n_ganhou, e_antes, e_depois) = NovaNotificacaoFilha(
        ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, proto_receptor, n_grupo);
    MergeTesouroTodo(proto_receptor.tesouro(), notificacao_doacao.entidade().tesouro(), e_depois->mutable_tesouro());
    AtribuiTesouroTodoOuCriaVazios(proto_receptor.tesouro(), e_antes->mutable_tesouro());
    if (n_desfazer != nullptr) {
      *n_desfazer->add_notificacao() = *n_ganhou;
    }
  }
  {
    // Texto de doacao.
    auto* n_texto = n_grupo->add_notificacao();
    n_texto->set_tipo(ntf::TN_ADICIONAR_ACAO);
    auto* acao = n_texto->mutable_acao();
    acao->set_tipo(ACAO_DELTA_PONTOS_VIDA);
    std::string texto;
    // Os lambdas aqui devem ter o retorno explicit, caso contrario o C++ assumira copia e a referencia retornada pela funcao ficara invalida.
    MergeMensagensTesouro(notificacao_doacao.entidade().tesouro(), tabelas, &texto);
    acao->set_texto(texto);
    acao->add_por_entidade()->set_id(proto_receptor.id());
  }
}

int NivelMagia(const ArmaProto& magia, const InfoClasse& ic) {
  for (const auto& icf : magia.info_classes()) {
    if (icf.id() == ic.id()) return icf.nivel();
  }
  return 0;
}

int NivelMaisAltoMagia(const ArmaProto& magia) {
  int nivel = 0;
  for (const auto& icf : magia.info_classes()) {
    nivel = std::max(nivel, icf.nivel());
  }
  return nivel;
}

int NivelPersonagem(const EntidadeProto& proto) {
  int total = 0;
  for (const auto& info_classe : proto.info_classes()) {
    total += info_classe.nivel();
  }
  return total - proto.niveis_negativos();
}

bool TendenciaSimples(TendenciaSimplificada tendencia, const EntidadeProto& proto) {
  return tendencia == proto.tendencia().simples();
}

bool ImuneAcaoMental(const EntidadeProto& proto) {
  return TemTipoDnD(TIPO_MORTO_VIVO, proto) || TemTipoDnD(TIPO_LIMO, proto) || 
         TemTipoDnD(TIPO_CONSTRUCTO, proto) || TemTipoDnD(TIPO_PLANTA, proto) ||
         TemTipoDnD(TIPO_VERME, proto);
}

bool NaoEnxerga(const EntidadeProto& proto) {
  return proto.naturalmente_cego() || TemTipoDnD(TIPO_LIMO, proto) || PossuiEvento(EFEITO_CEGO, proto);
}

bool MesmaTendencia(TendenciaSimplificada tendencia, const EntidadeProto& proto) {
  return tendencia == proto.tendencia().simples();
}

void ResolveEfeitoAdicional(int nivel_conjurador, const EntidadeProto& lancador, const Entidade& alvo, EfeitoAdicional* efeito_adicional, AcaoProto* acao_proto) {
  if (!efeito_adicional->has_rodadas()) {
    efeito_adicional->set_rodadas(Rodadas(nivel_conjurador, *efeito_adicional, *acao_proto, lancador, &alvo));
  }
}

void ResolveEfeitosAdicionaisVariaveis(int nivel_conjurador, const EntidadeProto& lancador, const Entidade& alvo, AcaoProto* acao_proto) {
  for (auto& efeito_adicional : *acao_proto->mutable_efeitos_adicionais()) {
    ResolveEfeitoAdicional(nivel_conjurador, lancador, alvo, &efeito_adicional, acao_proto);
  }
  for (auto& efeito_adicional : *acao_proto->mutable_efeitos_adicionais_se_salvou()) {
    ResolveEfeitoAdicional(nivel_conjurador, lancador, alvo, &efeito_adicional, acao_proto);
  }
  //LOG(INFO) << "resolvido: " << acao_proto->DebugString();
}

float AplicaEfeitosAdicionais(
    const Tabelas& tabelas,
    float atraso_s, bool salvou, const Entidade& entidade_origem, const Entidade& entidade_destino, aliado_e tipo_aliado, const DadosAtaque& da,
    AcaoProto::PorEntidade* por_entidade, AcaoProto* acao_proto, std::vector<int>* ids_unicos_origem, std::vector<int>* ids_unicos_destino,
    ntf::Notificacao* grupo_desfazer, ntf::CentralNotificacoes* central) {
  const int nivel_conjurador =
      da.has_nivel_conjurador_pergaminho()
        ? da.nivel_conjurador_pergaminho()
        : NivelConjuradorParaAcao(*acao_proto, tabelas.Feitico(da.id_arma()), entidade_origem);
  ResolveEfeitosAdicionaisVariaveis(nivel_conjurador, entidade_origem.Proto(), entidade_destino, acao_proto);
  for (const auto& efeito_adicional : salvou ? acao_proto->efeitos_adicionais_se_salvou() : acao_proto->efeitos_adicionais()) {
    if ((efeito_adicional.afeta_aliados_apenas() && tipo_aliado != TAL_ALIADO) || (efeito_adicional.afeta_inimigos_apenas() && tipo_aliado != TAL_INIMIGO)) {
      VLOG(1) << "pulando efeito: " << efeito_adicional.DebugString();
      continue;
    }
    if (!EntidadeAfetadaPorEfeito(tabelas, nivel_conjurador, efeito_adicional, entidade_destino.Proto())) {
      ConcatenaString(absl::StrFormat("imune ou não afetada por %s", EfeitoParaString(efeito_adicional.efeito()).c_str()), por_entidade->mutable_texto());
      continue;
    }
    std::unique_ptr<ntf::Notificacao> n_efeito(new ntf::Notificacao);
    PreencheNotificacaoEventoEfeitoAdicionalComAtaque(
        entidade_origem.Id(), DadosIniciativaEntidade(entidade_origem), da, nivel_conjurador, entidade_destino, efeito_adicional,
        ids_unicos_destino, n_efeito.get(), grupo_desfazer->add_notificacao());
    central->AdicionaNotificacao(n_efeito.release());
    atraso_s += 0.5f;
    // TODO criar tabela de nome dos efeitos.
    ConcatenaString(EfeitoParaString(efeito_adicional.efeito()), por_entidade->mutable_texto());
  }
  if (!salvou && !acao_proto->efeitos_adicionais_conjurador().empty()) {
    auto* por_entidade = acao_proto->add_por_entidade();
    por_entidade->set_id(entidade_origem.Id());
    por_entidade->set_delta(0);
    for (const auto& efeito_adicional : acao_proto->efeitos_adicionais_conjurador()) {
      std::unique_ptr<ntf::Notificacao> n_efeito(new ntf::Notificacao);
      PreencheNotificacaoEventoEfeitoAdicionalComAtaque(
          entidade_origem.Id(), DadosIniciativaEntidade(entidade_origem), da, nivel_conjurador, entidade_origem, efeito_adicional,
          ids_unicos_origem, n_efeito.get(), grupo_desfazer->add_notificacao());
      central->AdicionaNotificacao(n_efeito.release());
      atraso_s += 0.5f;
      ConcatenaString(StringEfeito(efeito_adicional.efeito()), por_entidade->mutable_texto());
    }
  }
  if (acao_proto->reduz_luz() && entidade_destino.TemLuz()) {
    // Apenas desfazer, pois o efeito fara a luz diminuir.
    auto* nd = grupo_desfazer->add_notificacao();
    PreencheNotificacaoReducaoLuzComConsequencia(nivel_conjurador, entidade_destino, acao_proto, nd, nd);
  }
  return atraso_s;
}

namespace {
void PreencheEntidadeDefesaTotal(bool ativar, const EntidadeProto& proto, EntidadeProto* proto_ntf) {
  proto_ntf->set_id(proto.id());
  auto* ca = proto_ntf->mutable_dados_defesa()->mutable_ca();
  *proto_ntf->mutable_dados_ataque() = proto.dados_ataque();
  if (ativar) {
    // Entra em defesa total (sai da luta defensiva).
    AtribuiBonus(ValorFinalPericia("acrobacias", proto) >= 5 ? 6 : 4, TB_ESQUIVA, "defesa_total", ca);
    AtribuiBonus(0, TB_ESQUIVA, "luta_defensiva", ca);
    for (auto& da : *proto_ntf->mutable_dados_ataque()) {
      AtribuiBonus(0, TB_SEM_NOME, "luta_defensiva", da.mutable_bonus_ataque());
    }
  } else {
    // Sai da defesa total.
    AtribuiBonus(0, TB_ESQUIVA, "defesa_total", ca);
  }
}
}  // namespace

ntf::Notificacao PreencheNotificacaoDefesaTotal(bool ativar, const EntidadeProto& proto) {
  ntf::Notificacao n;
  n.set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL);
  PreencheEntidadeDefesaTotal(ativar, proto, n.mutable_entidade());
  PreencheEntidadeDefesaTotal(!ativar, proto, n.mutable_entidade_antes());
  return n;
}

namespace {
void PreencheEntidadeLutarDefensivamente(bool ativar, const Entidade& entidade, EntidadeProto* proto_ntf) {
  const auto& proto = entidade.Proto();
  proto_ntf->set_id(proto.id());
  auto* ca = proto_ntf->mutable_dados_defesa()->mutable_ca();
  *proto_ntf->mutable_dados_ataque() = proto.dados_ataque();
  if (ativar) {
    int bonus_defesa = ValorFinalPericia("acrobacias", proto) >= 5 ? 3 : 2;
    int penalidade_ataque = -4;
    auto* t = Talento("especializacao_em_combate", proto);
    if (t != nullptr) {
      // Se tiver complemento, respeita ate o limite de 5.
      if (t->has_complemento()) {
        int complemento = atoi(t->complemento().c_str());
        if (complemento > 0) {
          bonus_defesa = std::min(complemento, 5);
        }
      }
      // Respeita o limite de BBA. Se nao teve complemento, vai usar o valor padrao.
      bonus_defesa = std::min(bonus_defesa, entidade.BonusBaseAtaque());
      penalidade_ataque = -bonus_defesa;
    }
    // Entra em luta defensiva (exclui defesa total).
    AtribuiBonus(bonus_defesa, TB_ESQUIVA, "luta_defensiva", ca);
    AtribuiBonus(0, TB_ESQUIVA, "defesa_total", ca);
    for (auto& da : *proto_ntf->mutable_dados_ataque()) {
      AtribuiBonus(penalidade_ataque, TB_SEM_NOME, "luta_defensiva", da.mutable_bonus_ataque());
    }
  } else {
    // Sai da luta defensiva.
    AtribuiBonus(0, TB_ESQUIVA, "luta_defensiva", ca);
    for (auto& da : *proto_ntf->mutable_dados_ataque()) {
      AtribuiBonus(0, TB_SEM_NOME, "luta_defensiva", da.mutable_bonus_ataque());
    }
  }
}

void PreencheEntidadeAtacandoPoderosamente(bool ativar, const Entidade& entidade, EntidadeProto* proto_ntf) {
  const auto& proto = entidade.Proto();
  proto_ntf->set_id(proto.id());
  *proto_ntf->mutable_dados_ataque() = proto.dados_ataque();
  if (ativar) {
    const auto* talento = Talento("ataque_poderoso", entidade.Proto()); 
    LOG(INFO) << "complemento: " << talento->complemento();
    const int bba = entidade.BonusBaseAtaque();
    int valor = talento == nullptr || talento->complemento().empty() ? bba / 2 : atoi(talento->complemento().c_str());
    valor = std::max(1, std::min(valor, bba));
    for (auto& da : *proto_ntf->mutable_dados_ataque()) {
      if (da.ataque_corpo_a_corpo()) {
        AtribuiBonus(da.empunhadura() == EA_2_MAOS ? 2 * valor : valor, TB_SEM_NOME, "ataque_poderoso", da.mutable_bonus_dano());
        AtribuiBonus(-valor, TB_SEM_NOME, "ataque_poderoso", da.mutable_bonus_ataque());
      }
    }
  } else {
    for (auto& da : *proto_ntf->mutable_dados_ataque()) {
      RemoveBonus(TB_SEM_NOME, "ataque_poderoso", da.mutable_bonus_dano());
      RemoveBonus(TB_SEM_NOME, "ataque_poderoso", da.mutable_bonus_ataque());
    }
  }
}

}  // namespace

ntf::Notificacao PreencheNotificacaoLutarDefensivamente(bool ativar, const Entidade& entidade) {
  ntf::Notificacao n;
  n.set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL);
  PreencheEntidadeLutarDefensivamente(ativar, entidade, n.mutable_entidade());
  PreencheEntidadeLutarDefensivamente(!ativar, entidade, n.mutable_entidade_antes());
  return n;
}

ntf::Notificacao PreencheNotificacaoAtacandoPoderosamente(bool ativar, const Entidade& entidade) {
  ntf::Notificacao n;
  n.set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL);
  PreencheEntidadeAtacandoPoderosamente(ativar, entidade, n.mutable_entidade());
  PreencheEntidadeAtacandoPoderosamente(!ativar, entidade, n.mutable_entidade_antes());
  return n;
}

ntf::Notificacao PreencheNotificacaoExpiracaoEventoPosSalvacao(const Entidade& entidade) {
  ntf::Notificacao n;
  PreencheNotificacaoRemocaoEvento(entidade.Proto(), EFEITO_DOMINIO_PROTECAO, &n);
  return n.entidade().evento().empty() ? ntf::Notificacao::default_instance() : n;
}

int Salvacao(const EntidadeProto& proto, const Bonus& outros_bonus, const EntidadeProto& proto_atacante, TipoSalvacao tipo) {
  Bonus b(BonusContraTendenciaNaSalvacao(proto_atacante, proto));
  CombinaBonus(outros_bonus, &b);
  const auto& dd = proto.dados_defesa();
  switch (tipo) {
    case TS_FORTITUDE: CombinaBonus(dd.salvacao_fortitude(), &b); break;
    case TS_REFLEXO: CombinaBonus(dd.salvacao_reflexo(), &b); break;
    case TS_VONTADE: CombinaBonus(dd.salvacao_vontade(), &b); break;
    default:
      LOG(ERROR) << "Tipo de salvacao invalido: " << (int)tipo;
  }
  return BonusTotal(b);
}

int SalvacaoVeneno(const EntidadeProto& proto) {
  return Salvacao(proto, proto.dados_defesa().bonus_salvacao_veneno(), EntidadeProto::default_instance(), TS_FORTITUDE);
}

// Retorna true caso haja algum tipo do defensor que o atacante nao tem.
bool TipoDnDDiferentes(const EntidadeProto& proto_atacante, const EntidadeProto& proto_defensor) {
  for (auto tipo_defensor : proto_defensor.tipo_dnd()) {
    if (c_none(proto_atacante.tipo_dnd(), tipo_defensor)) return true;
  }
  return false;
}

int SalvacaoFeitico(const ArmaProto& feitico_tabelado, const EntidadeProto& proto, const EntidadeProto& proto_atacante, TipoSalvacao tipo) {
  Bonus outros_bonus = proto.dados_defesa().bonus_salvacao_feitico();
  if (feitico_tabelado.escola() == "encantamento") {
    CombinaBonus(proto.dados_defesa().bonus_salvacao_encantamento(), &outros_bonus);
  } else if (feitico_tabelado.escola() == "ilusao") {
    CombinaBonus(proto.dados_defesa().bonus_salvacao_ilusao(), &outros_bonus);
  }
  if (feitico_tabelado.acao().elemento() == DESC_MEDO) {
    CombinaBonus(proto.dados_defesa().bonus_salvacao_medo(), &outros_bonus);
  }
  if (feitico_tabelado.acao().has_bonus_salvacao_por_tipo_diferente() &&
      TipoDnDDiferentes(proto_atacante, proto)) {
    Bonus b;
    AtribuiBonus(feitico_tabelado.acao().bonus_salvacao_por_tipo_diferente(), TB_SEM_NOME, "tipos_diferentes", &b);
    CombinaBonus(b, &outros_bonus);
  }
  return Salvacao(proto, outros_bonus, proto_atacante, tipo);
}

bool ArmaNatural(const ArmaProto& arma) {
  return c_any(arma.categoria(), CAT_ARMA_NATURAL);
}

bool FeiticoDominio(const std::vector<std::string>& dominios, const ArmaProto& feitico_tabelado) {
  return c_any_of(feitico_tabelado.info_classes(), [&dominios](const ArmaProto::InfoClasseParaFeitico& classe) {
    return classe.dominio() && c_any(dominios, classe.id());
  });
}

bool FeiticoEscolaProibida(const std::vector<std::string>& escolas_proibidas, const ArmaProto& feitico_tabelado) {
  return c_any(escolas_proibidas, feitico_tabelado.escola());
}

std::optional<std::tuple<bool, int, int, std::string>> RolaTesteAtributo(
    TipoAtributo atributo, const Bonus& outros_bonus, const EntidadeProto& proto) {
  std::unordered_map<TipoAtributo, std::string> nomes = {
    { TA_FORCA, "força"},
    { TA_DESTREZA, "destreza"},
    { TA_CONSTITUICAO, "constituição"},
    { TA_INTELIGENCIA, "inteligência"},
    { TA_SABEDORIA, "sabedoria"},
    { TA_CARISMA, "carisma"},
  };
  const int bonus = ModificadorAtributo(atributo, proto);
  const int outros_bonus_int = BonusTotal(outros_bonus);
  const int dado = RolaDado(20);
  const int total_modificadores = bonus + outros_bonus_int;
  const int total = dado + total_modificadores;
  std::string outros_bonus_str = outros_bonus_int != 0 ? absl::StrFormat(" %+d", outros_bonus_int) : std::string("");
  return std::make_tuple(true, total, total_modificadores, absl::StrFormat("%s: %d %+d%s = %d", nomes[atributo].c_str(), dado, bonus, outros_bonus_str.c_str(), total));
}

std::optional<std::tuple<bool, int, int, std::string>> RolaPericia(
    const Tabelas& tabelas, const std::string& id_pericia, const Bonus& outros_bonus, const EntidadeProto& proto) {
  const auto& pericia_personagem = Pericia(id_pericia, proto);
  if (!pericia_personagem.has_id()) {
    return std::nullopt;
  }
  const auto& pericia_tabelada = tabelas.Pericia(pericia_personagem.id());
  bool treinado = pericia_personagem.pontos() > 0;
  bool sem_treinamento = pericia_tabelada.sem_treinamento();
  if (!pericia_tabelada.derivada_de().empty()) {
    const auto& pericia_personagem_origem = Pericia(pericia_tabelada.derivada_de(), proto);
    treinado = pericia_personagem_origem.pontos() > 0;
    const auto& pericia_tabelada_origem = tabelas.Pericia(pericia_tabelada.derivada_de());
    sem_treinamento = pericia_tabelada_origem.sem_treinamento();
  }
  if (treinado || sem_treinamento) {
    const int bonus = BonusTotal(pericia_personagem.bonus());
    const int outros_bonus_int = BonusTotal(outros_bonus);
    const int dado = RolaDado(20);
    const int total_modificadores = bonus + outros_bonus_int;
    const int total = dado + total_modificadores;
    std::string outros_bonus_str = outros_bonus_int != 0 ? absl::StrFormat(" %+d", outros_bonus_int) : std::string("");
    return std::make_tuple(true, total, total_modificadores, absl::StrFormat("%s: %d %+d%s = %d", pericia_tabelada.nome().c_str(), dado, bonus, outros_bonus_str.c_str(), total));
  } else {
    return std::make_tuple(false, 0, 0, absl::StrFormat("Pericia %s requer treinamento", pericia_tabelada.nome().c_str()));
  }
}

bool EhFeitico(const ArmaProto& arma) {
  return !arma.info_classes().empty();
}

std::string PrecoString(const Moedas& moedas) {
  std::string preco;
  if (moedas.po() > 0) {
    absl::StrAppendFormat(&preco, "%d PO, ", moedas.po());
  }
  if (moedas.pp() > 0) {
    absl::StrAppendFormat(&preco, "%d PP, ", moedas.pp());
  }
  if (moedas.pc() > 0) {
    absl::StrAppendFormat(&preco, "%d PC, ", moedas.pc());
  }
  if (moedas.pl() > 0) {
    absl::StrAppendFormat(&preco, "%d PP, ", moedas.pl());
  }
  if (moedas.pe() > 0) {
    absl::StrAppendFormat(&preco, "%d PE, ", moedas.pp());
  }
  if (preco.size() >= 2) {
    preco = preco.substr(0, preco.size() - 2);
  }
  return preco;
}

int PrecoItemPo(const ItemMagicoProto& item_tabelado) {
  if (item_tabelado.custo_po() > 0) {
    return item_tabelado.custo_po();
  }
  return item_tabelado.custo().po();
}

std::string PrecoItem(const ItemMagicoProto& item_tabelado) {
  if (item_tabelado.custo_po() > 0) {
    return absl::StrFormat("%d PO", item_tabelado.custo_po());
  }
  return PrecoString(item_tabelado.custo());
}

Moedas ConvertePreco(const std::string& preco_const) {
  Moedas moedas;
  std::string preco = preco_const;
  std::transform(preco.begin(), preco.end(), preco.begin(), [](unsigned char c){ return std::tolower(c); });
  char tipo = 'o';
  auto posp = preco.find('p');
  if (posp + 1 < preco.size()) tipo = preco[posp + 1];
  int valor = atoi(preco.c_str());
  switch (tipo) {
    case 'c': moedas.set_pc(valor); break; 
    case 'p': moedas.set_pp(valor); break; 
    case 'e': moedas.set_pe(valor); break; 
    case 'l': moedas.set_pl(valor); break; 
    case 'o':
    default:
      moedas.set_po(valor);
      break; 
  }
  return moedas;
}

int PrecoArmaPo(const EntidadeProto::ArmaArmaduraOuEscudoPersonagem& arma_personagem) {
  const auto& arma_tabelada = Tabelas::Unica().Arma(arma_personagem.id_tabela());
  int valor = ConvertePreco(arma_tabelada.preco()).po();
  if (arma_personagem.obra_prima()) {
    valor += 300;
    if (PossuiCategoria(CAT_ARMA_DUPLA, arma_tabelada)) valor += 300;
  }
  valor += 2 * pow(arma_personagem.bonus_magico(), 2) * 1000;
  valor += 2 * pow(arma_personagem.bonus_magico_secundario(), 2) * 1000;
  return valor;
}

int PrecoArmaduraOuEscudoPo(const EntidadeProto::ArmaArmaduraOuEscudoPersonagem& aoe_personagem) {
  const ArmaduraOuEscudoProto* aoe_tabela = nullptr;
  {
    aoe_tabela = &Tabelas::Unica().Armadura(aoe_personagem.id_tabela());
    if (aoe_tabela->id().empty()) {
      aoe_tabela = &Tabelas::Unica().Escudo(aoe_personagem.id_tabela());
    } 
  }

  int valor = aoe_tabela->preco().po();
  if (aoe_personagem.obra_prima()) {
    valor += 150;
  }
  valor += pow(aoe_personagem.bonus_magico(), 2) * 1000;
  return valor;
}

}  // namespace ent
