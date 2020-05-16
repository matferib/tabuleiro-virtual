#ifndef ENT_UTIL_H
#define ENT_UTIL_H

#include <algorithm>
#include <cctype>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include <google/protobuf/repeated_field.h>
#include "arq/arquivo.h"
#include "ent/controle_virtual.pb.h"
#include "ent/tabelas.pb.h"
#include "matrix/matrices.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

// Funcoes uteis de ent.
namespace ent {

class Acao;
class ArmaProto;
class Cor;
class Entidade;
class EntidadeProto_Evento;
class ParametrosDesenho;
class Posicao;
class Tabuleiro;
class Tabelas;

// Util para rodar codigo ao sair de escopo.
class RodaNoRetorno {
 public:
  explicit RodaNoRetorno(std::function<void()> f) : f(f) {}
  ~RodaNoRetorno() { f(); }
  void Cancela() { f = std::function<void()>(); }

  std::function<void()> f;
};

template <class C, class F>
bool c_any_of(const C& c, const F& f) {
  return std::any_of(c.begin(), c.end(), f);
}

template <class C, class F>
bool c_all_of(const C& c, const F& f) {
  return std::all_of(c.begin(), c.end(), f);
}

template <class C, class T>
bool c_any(const C& c, const T& t) {
  return std::find(c.begin(), c.end(), t) != c.end();
}

template <class C, class F>
bool c_none_of(const C& c, const F& f) {
  return std::none_of(c.begin(), c.end(), f);
}

template <class C, class T>
bool c_none(const C& c, const T& t) {
  return std::find(c.begin(), c.end(), t) == c.end();
}

// Remove os indices passados do container.
template <class T>
void RemoveIndices(const std::set<int, std::greater<int>>& indices, google::protobuf::RepeatedPtrField<T>* c) {
  for (int i : indices) {
    c->DeleteSubrange(i, 1);
  }
}

// Remove o item do container se bater com predicado.
template <class T>
void RemoveSe(const std::function<bool(const T& t)>& predicado, google::protobuf::RepeatedPtrField<T>* c) {
  for (int i = c->size() - 1; i >= 0; --i) {
    if (predicado(c->Get(i))) c->DeleteSubrange(i, 1);
  }
}

void IniciaUtil();

/** Cria uma notificacao do tipo TN_GRUPO_NOTIFICACOES. */
std::unique_ptr<ntf::Notificacao> NovoGrupoNotificacoes();
/** Cria uma nova notificacao do tipo passado para a entidade, preenchendo id antes e depois dela. */
std::unique_ptr<ntf::Notificacao> NovaNotificacao(ntf::Tipo tipo, const EntidadeProto& proto);
/** Retorna a notificacao filha, com proto antes e depois preenchidos pelo id de proto. */
std::tuple<ntf::Notificacao*, EntidadeProto*, EntidadeProto*> NovaNotificacaoFilha(
    ntf::Tipo tipo, const EntidadeProto& proto, ntf::Notificacao* pai);

/** Altera a cor corrente para cor. Nao considera alpha. */
void MudaCor(const float* cor);
void MudaCorAplicandoNevoa(const float* cor, const ParametrosDesenho* pd);

/** Liga a nevoa com os parametros passados, preenchendo pd. Posicao deve ser em coordenadas de mundo, que sera convertida em coordenadas
* de olho.
*/
void ConfiguraNevoa(float min, float max, float r, float g, float b, float* pos, ParametrosDesenho* pd);

/** Considera alpha. */
void MudaCorAlfa(const float* cor);

/** Outra forma de mudar a cor. */
void MudaCor(const Cor& cor);

/** Preenche proto_cor com cor. A entrada deve ter 4 componentes. */
void CorAlfaParaProto(const float* cor, Cor* cor_proto);

/** Igual CorAlfaParaProto mas para 3 componentes. */
void CorParaProto(const float* cor, Cor* cor_proto);
Cor CorParaProto(const float* cor);

/** Reduz cada componente RGB em 0.5f. */
const Cor EscureceCor(const Cor& cor);
void EscureceCor(Cor* cor);
void EscureceCor(float* cor);

/** Aumenta cada componente RGB em 0.5f. */
void ClareiaCor(Cor* cor);

/** Muda cada componente RGB de forma notavel. */
void RealcaCor(Cor* cor);
void RealcaCor(float* cor);
Cor CorRealcada(const Cor& cor);

/** Combina as duas cores, maximo 1 em cada componente. Nao usa alfa. */
void CombinaCor(const Cor& cor_origem, Cor* cor_destino);
/** Peso indica quanto cada cor contribui. */
void CombinaCorComPeso(float peso_origem, const Cor& cor_origem, Cor* cor_destino);

// retorna true se o tipo da forma for 2d: circulo, retangulo, triangulo, livre.
bool EhForma2d(int tipo_forma);

/** Desenha uma linha 3d com a largura passada, passando pelos pontos. Em cada ponto, sera desenhado um disco para conectar. */
void DesenhaLinha3d(const std::vector<Posicao>& pontos, float largura);
void DesenhaLinha3d(const google::protobuf::RepeatedPtrField<Posicao>& pontos, float largura);
void LimitesLinha3d(const google::protobuf::RepeatedPtrField<Posicao>& pontos, float largura, float* xi, float* yi, float *xs, float *ys);

/** Funcoes para ligar o stencil e depois desenhar a cor passada (ou a corrente) onde o stencil foi marcado. */
// ATENCAO: Esse retangulo acaba com a operacao de picking (porque escreve na tela toda). Operacoes de picking nao devem usar stencil.
void LigaStencil();
/** O stencil 2d desenha um retangulo sobre a tela inteira. */
void DesenhaStencil2d(const float* cor = nullptr);
void DesenhaStencil2d(const Cor& cor);
/** O stencil 3d desenha um retangulo paralelo ao tabuleiro. */
void DesenhaStencil3d(float tam_x, float tam_y, const Cor& cor);
void DesenhaStencil3d(float tam_x, float tam_y, const float* cor = nullptr);
void DesenhaStencil3d(float xi, float yi, float xs, float ys, const float* cor = nullptr);

/** Gera um aleatorio de 1 a nfaces. */
int RolaDado(unsigned int nfaces);
/** Gera um aleatorio entre [0.0 e 1.0]. Os valores tem precisao de duas casas. */
float Aleatorio();

/** Adiciona o delta ao dados_vida. */
void AtualizaStringDadosVida(int delta, std::string* dados_vida);

/** Gera pontos de vida baseado nos dados de vida, da forma 4d8+8 por exemplo.
* O valor de retorno é o total, e um vetor com o valor de cada dado rolado (o primeiro elemento eh o numero de faces, o segundo eh o valor).
* Da excecao se dados_vida for mal formado.
*/
std::tuple<int, std::vector<std::pair<int, int>>> GeraPontosVida(const std::string& dados_vida);
inline int RolaValor(const std::string& valor) { return std::get<0>(GeraPontosVida(valor)); }

/** Converte os dados convertido para string. */
std::string DadosParaString(int total, std::vector<std::pair<int, int>>& dados);

// Le o resultado de um dano de uma arma.
// Exemplos:
// 1d8+5 (19-20/x3) retorna: 1d8+5, 19, 3.
// 1d8 (x3) retorna: 1d8, 20, 3.
// 1d8 (19-20) retorna: 1d8, 19, 2.
// 1d8 retorna: 1d8, 20, 2.
struct DanoArma {
  std::string dano;
  int margem_critico;
  int multiplicador;
};
DanoArma LeDanoArma(const std::string& dano);

/** Gera o maximo de pontos de vida baseado no dados_vida.
* Da excecao se dados_vida for mal formado.
*/
int GeraMaxPontosVida(const std::string& dados_vida);

/** Computa pos2 - pos1 em pos_res. */
void ComputaDiferencaVetor(const Posicao& pos2, const Posicao& pos1, Posicao* pos_res);

/** Computa pos2 + pos1 em pos_res. */
void ComputaSomaVetor(const Posicao& pos2, const Posicao& pos1, Posicao* pos_res);

/** Computa pos_res = escala * pos. */
void ComputaMultiplicacaoEscalar(float escala, const Posicao& pos, Posicao* pos_res);

/** Computa o vetor normalizado. */
void ComputaVetorNormalizado(Posicao* pos);

/* Multiplica a matriz openGL pelo vetor. A matriz OpenGL tem formato col x linha (column major), portanto,
* ao inves de multiplicar matriz (4x4) pelo vetor (4x1), fazemos a inversao: vetor (1x4) pela matriz (4x4).
*/
void MultiplicaMatrizVetor(const float* matriz, float* vetor);

/** Retorna o valor de rotacao para se chegar a um vetor x,y. O valor vai de (-180, 180]. */
float VetorParaRotacaoGraus(const Posicao& vetor, float* tamanho = nullptr);
float VetorParaRotacaoGraus(float x, float y, float* tamanho = nullptr);

/** Matriz de rotacao para se chegar ao vetor v a partir do eixo X. */
Matrix4 MatrizRotacao(const Vector3& v);

/** @return quadrado da distancia entre as posicoes. */
float DistanciaEmMetrosAoQuadrado(const Posicao& pos1, const Posicao& pos2);

/** Roda o vetor no eixo Z. */
void RodaVetor2d(float graus, Posicao* vetor);

/** Retorna true se o ponto estiver dentro do poligono. */
bool PontoDentroDePoligono(const Posicao& ponto, const std::vector<Posicao>& vertices);

/** Posicionamento do raster em 2d. */
void PosicionaRaster2d(int x, int y);

/** Converte uma string para o efeito, se houver. Caso contrario retorna EFEITO_INVALIDO. */
TipoEfeito StringParaEfeito(const std::string& s);
// Devia se chamar EfeitoParaString.
std::string StringEfeito(TipoEfeito efeito);
inline std::string EfeitoParaString(TipoEfeito efeito) { return StringEfeito(efeito); }

// Trim functions from: http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring.
static inline std::string& ltrim(std::string& s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) { return !std::isspace(c); } ));
  return s;
}

// trim from end
static inline std::string& rtrim(std::string& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) { return !std::isspace(c); }).base(), s.end());
  return s;
}

// trim from both ends
static inline std::string& trim(std::string& s) {
  return ltrim(rtrim(s));
}

// Normaliza o texto passado, tirando sequencias UTF-8.
const std::string StringSemUtf8(const std::string& texto);

// Move o delta para uma entidade, garantindo que ela termine acima do solo.
void MoveDeltaRespeitandoChao(float dx, float dy, float dz, const Tabuleiro& tabuleiro, Entidade* entidade);

bool EhPng(const std::string& textura);
bool EhIcone(const std::string& textura);
bool EhTerreno(const std::string& textura);
bool EhCaixaCeu(const std::string& textura);
bool EhModelo3d(const std::string& textura);
// Filtros uteis.
bool FiltroModelo3d(const std::string& textura);
bool FiltroTexturaEntidade(const std::string& textura);
bool FiltroTexturaCaixaCeu(const std::string& textura);
bool FiltroTexturaTabuleiro(const std::string& textura);

// DEPRECATED
class AlteraBlendEscopo {
 public:
  // O valor alfa, se menor que 1.0, sera usado na transparencia no lugar do alfa_translucidos de pd.
  explicit AlteraBlendEscopo(const ParametrosDesenho* pd, const Cor& cor)
      : pd_(pd), restaurar_(AlteraBlendEntidadeComposta(pd, cor)) {}
  ~AlteraBlendEscopo() { if (restaurar_) RestauraBlend(pd_); }

 private:
  bool AlteraBlendEntidadeComposta(const ParametrosDesenho* pd, const Cor& cor) const;
  void RestauraBlend(const ParametrosDesenho* pd) const;

  const ParametrosDesenho* pd_;
  bool restaurar_;
};

// Blend de entidades compostas e modelos3d. O blend deve ser restaurado para o valor padrao depois para nao
// avacalhar os proximos.
struct MisturaPreNevoaEscopo {
  MisturaPreNevoaEscopo(const Cor& cor, const ParametrosDesenho* pd);
  MisturaPreNevoaEscopo(float r, float g, float b, float a);
  ~MisturaPreNevoaEscopo();

  float salvo_[4];
};

enum class TipoAtaque {
  CORPO_A_CORPO,
  DISTANCIA,
  AGARRAR
};
TipoAtaque DaParaTipoAtaque(const DadosAtaque& da);
// Retorna alguns modificadores de ataque para a entidade de acordo com seus status e do defensor.
// Alguns modificadores que seriam de CA tb vem para ca.
int ModificadorAtaque(TipoAtaque tipo_ataque, const EntidadeProto& ea, const EntidadeProto& ed);
// Retorna alguns modificadores de dano genericos para a entidade de acordo com seus status e o defensor.
int ModificadorDano(const DadosAtaque& da, const EntidadeProto& ea, const EntidadeProto& ed);

enum resultado_ataque_e {
  RA_SEM_ACAO = 0,            // acao nao realizada por algum problema com ataque.
  RA_SUCESSO = 1,             // sucesso normal, ver vezes para saber se eh critico.
  RA_FALHA_CRITICA = 3,       // falha critica.
  RA_FALHA_REFLEXO = 4,       // falhou porque acertou reflexo.
  RA_FALHA_NORMAL = 5,        // falha normal.
  RA_FALHA_TOQUE_AGARRAR = 6, // falha normal.
  RA_FALHA_CHANCE_FALHA = 7,  // falha por chance de falha.
  RA_FALHA_IMUNE = 8,         // falha por imunidade ao tipo de ataque.
  RA_FALHA_REDUCAO = 9,       // falha por reducao de dano (reduzido a zero).
};
struct ResultadoAtaqueVsDefesa {
  resultado_ataque_e resultado = RA_SEM_ACAO;
  int vezes = 0;  // para sucesso critico.
  std::string texto;

  bool Sucesso() const { return resultado == RA_SUCESSO; }
};
// Rola o dado de ataque vs defesa, retornando o numero de vezes que o dano deve ser aplicado e o texto da jogada.
// O ultimo parametro indica se a acao deve ser desenhada (em caso de distancia maxima atingida, retorna false).
// Caso haja falha critica, retorna vezes = -1;
// Posicao ataque eh para calculo de distancia.
ResultadoAtaqueVsDefesa AtaqueVsDefesa(
    float distancia_m, const AcaoProto& ap, const Entidade& ea, const Entidade& ed, const Posicao& pos_alvo);
ResultadoAtaqueVsDefesa AtaqueVsDefesa(
    float distancia_m, const AcaoProto& ap, const Entidade& ea, const DadosAtaque* da,
    const Entidade& ed, const Posicao& pos_alvo);

// Rola o dado de ataque da manobra de derrubar (forca vs (destreza ou forca))
ResultadoAtaqueVsDefesa AtaqueVsDefesaDerrubar(const Entidade& ea, const Entidade& ed);
// Rola o teste de agarrar, ja considerando que tudo antes (ataque de toque), funcionou.
ResultadoAtaqueVsDefesa AtaqueVsDefesaAgarrar(const Entidade& ea, const Entidade& ed);

// Retorna true se o personagem puder lancar o tipo de pergaminho do ataque.
std::pair<bool, std::string> PodeLancarPergaminho(const Tabelas& tabelas, const EntidadeProto& proto, const DadosAtaque& da);

// Resultado de um teste de lancar pergaminho. Se nao ok, pode ter fiasco.
struct ResultadoPergaminho {
  ResultadoPergaminho(bool ok, bool fiasco = false, const std::string& texto = "") : ok(ok), fiasco(fiasco), texto(texto) {}
  bool ok;
  bool fiasco;  // se falhar, pode ser fiasco.
  std::string texto;
};
// Realiza o teste de lancar pergaminho.
ResultadoPergaminho TesteLancarPergaminho(const Tabelas& tabelas, const EntidadeProto& proto, const DadosAtaque& da);

// Rola o dado de ataque contra a resistencia a magia e salvacao, retornando o dano, se salvou ou nao e o texto do resultado.
std::tuple<int, bool, std::string> AtaqueVsSalvacao(int delta_pv, const DadosAtaque& da, const Entidade& ea, const Entidade& ed);
// Caso a criatura possua RM, rola o dado e retorna true se passar na RM. Caso nao possua RM, retorna true e vazio.
std::tuple<bool, std::string> AtaqueVsResistenciaMagia(
    const Tabelas& tabelas, const DadosAtaque& da, const Entidade& ea, const Entidade& ed);

// Gera um resumo sobre a notificacao, ou vazio.
std::string ResumoNotificacao(const Tabuleiro& tabuleiro, const ntf::Notificacao& n);

inline Vector3 PosParaVector3(const Posicao& pos) { return Vector3(pos.x(), pos.y(), pos.z()); }
inline Vector4 PosParaVector4(const Posicao& pos) { return Vector4(pos.x(), pos.y(), pos.z(), 1.0f); }
inline Posicao Vector3ParaPosicao(const Vector3& v) {
  Posicao p;
  p.set_x(v.x); p.set_y(v.y); p.set_z(v.z);
  return p;
}
inline Posicao Vector4ParaPosicao(const Vector4& v) {
  Posicao p;
  p.set_x(v.x / v.w); p.set_y(v.y / v.w); p.set_z(v.z / v.w);
  return p;
}

// Preenche uma notificacao de dano letal ou nao letal, incluindo desfazer. n_desfazer nao eh obrigatorio.
enum tipo_dano_e {
  TD_LETAL = 0,
  TD_NAO_LETAL = 1
};

// Preenche a notificacao de forma alternativa para a entidade, alternando para a proxima, ou para a original se ja estiver na ultima.
void PreencheNotificacaoFormaAlternativa(const Tabelas& tabelas, const EntidadeProto& proto, ntf::Notificacao* n_grupo, ntf::Notificacao* n_desfazer);

void PreencheNotificacoesTransicaoTesouro(
    const Tabelas& tabelas, const Entidade& doador, const Entidade& receptor, ntf::Notificacao* n_grupo, ntf::Notificacao* n_desfazer);

// Botao de doacao: apenas alguns tesouros.
void PreencheNotificacoesDoacaoParcialTesouro(
    const Tabelas& tabelas, const ntf::Notificacao& notificacao_doacao, const EntidadeProto& proto_doador, const EntidadeProto& proto_receptor,
    ntf::Notificacao* n_grupo, ntf::Notificacao* n_desfazer);

// Para cura, usar delta positivo.
void PreencheNotificacaoAtualizacaoPontosVida(
    const EntidadeProto& proto, int delta_pontos_vida, tipo_dano_e td, ntf::Notificacao* n, ntf::Notificacao* n_desfazer);
void PreencheNotificacaoAtualizacaoPontosVida(
    const Entidade& entidade, int delta_pontos_vida, tipo_dano_e td, ntf::Notificacao* n, ntf::Notificacao* n_desfazer);
void PreencheNotificacaoCuraAcelerada(const Entidade& entidade, ntf::Notificacao* n);

// Preenche uma notificacao consumir o dado de ataque e/ou municao.
void PreencheNotificacaoConsumoAtaque(
    const Entidade& entidade, const DadosAtaque& da, ntf::Notificacao* n, ntf::Notificacao* n_desfazer);
// Preenche uma notificacao de carregamento da arma.
void PreencheNotificacaoRecarregamento(
    const Entidade& entidade, const DadosAtaque& da, ntf::Notificacao* n, ntf::Notificacao* n_desfazer);

// Adiciona um evento do tipo passado a entidade.
// Tem uma parte tricky aqui, que se mais de um efeito for adicionado de uma vez so, os id_unico irao se repetir.
void PreencheNotificacaoEvento(
    unsigned int id_entidade, const std::string& origem, TipoEfeito te, int rodadas,
    std::vector<int>* ids_unicos, ntf::Notificacao* n, ntf::Notificacao* n_desfazer);
void PreencheNotificacaoEventoComComplementoStr(
    unsigned int id_entidade, const std::string& origem, TipoEfeito te, const std::string& complemento_str, int rodadas,
    std::vector<int>* ids_unicos, ntf::Notificacao* n, ntf::Notificacao* n_desfazer);

// Dado um tipo de evento, remove todos daquele tipo.
void PreencheNotificacaoRemocaoEvento(
    const EntidadeProto& proto, TipoEfeito te, ntf::Notificacao* n);

// Retorna o id unico gerado (-1 em caso de erro).
void PreencheNotificacaoEventoEfeitoAdicionalComAtaque(
    unsigned int id_origem, const DadosAtaque& da, int nivel_conjurador, const Entidade& entidade_destino, const AcaoProto::EfeitoAdicional& efeito_adicional,
    std::vector<int>* ids_unicos, ntf::Notificacao* n, ntf::Notificacao* n_desfazer);
inline void PreencheNotificacaoEventoEfeitoAdicional(
    unsigned int id_origem, int nivel_conjurador, const Entidade& entidade_destino, const AcaoProto::EfeitoAdicional& efeito_adicional,
    std::vector<int>* ids_unicos, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  return PreencheNotificacaoEventoEfeitoAdicionalComAtaque(
      id_origem, DadosAtaque::default_instance(), nivel_conjurador,
      entidade_destino, efeito_adicional, ids_unicos, n, n_desfazer);
}

void PreencheNotificacaoEventoParaVenenoPrimario(
    unsigned int id_entidade, const VenenoProto& veneno, int rodadas,
    std::vector<int>* ids_unicos, ntf::Notificacao* n, ntf::Notificacao* n_desfazer);
void PreencheNotificacaoEventoParaVenenoSecundario(
    unsigned int id_entidade, const VenenoProto& veneno, int rodadas,
    std::vector<int>* ids_unicos, ntf::Notificacao* n, ntf::Notificacao* n_desfazer);

// Preenche n com o tipo passado, setando id da entidade antes e depois em n.
// Retorna entidade antes e depois dentro de n.
std::pair<EntidadeProto*, EntidadeProto*> PreencheNotificacaoEntidade(
    ntf::Tipo tipo, const Entidade& entidade, ntf::Notificacao* n);
std::pair<EntidadeProto*, EntidadeProto*> PreencheNotificacaoEntidadeProto(
    ntf::Tipo tipo, const EntidadeProto& proto, ntf::Notificacao* n);
std::pair<EntidadeProto*, EntidadeProto*> PreencheNotificacaoEntidadeComId(
    ntf::Tipo tipo, uint32_t id_entidade, ntf::Notificacao* n);

// Preenche notificacao de diminuicao do raio de luz da entidade e a consequencia da acao.
void PreencheNotificacaoReducaoLuzComConsequencia(int nivel, const Entidade& alvo, AcaoProto* acao_proto, ntf::Notificacao* n, ntf::Notificacao* n_desfazer);

// Preenche proto alvo com todos o itens magicos em uso por proto. Parametro manter uso ditara o estado deles em proto alvo.
void PreencheComTesourosEmUso(const EntidadeProto& proto, bool manter_uso, EntidadeProto* proto_alvo);

// Notificacao que a entidade desviou ou resetou o valor de desviar objetos.
void PreencheNotificacaoObjetoDesviado(bool valor, const Entidade& entidade, ntf::Notificacao* n, ntf::Notificacao* nd);

// Preenche a notificacao para entidade entrar em defesa total (aumenta CA, nao pode atacar).
ntf::Notificacao PreencheNotificacaoDefesaTotal(bool ativar, const EntidadeProto& proto);
// Preenche a notificacao para entidade entrar em luta defensiva (aumenta CA, ataque com penalidade).
ntf::Notificacao PreencheNotificacaoLutarDefensivamente(bool ativar, const EntidadeProto& proto);

// Alguns efeitos valem ate a proxima salvacao. Cria uma notificacao para expira-los.
// Caso nao haja eventos assim, retorna uma notificacao defaul (vazia).
ntf::Notificacao PreencheNotificacaoExpiracaoEventoPosSalvacao(const Entidade& entidade);

// Adiciona ao grupo uma notificacao de atualizacao dos ataques da entidade.
void PreencheNotificacaoAtaqueAoPassarRodada(const EntidadeProto& proto, ntf::Notificacao* grupo);

// Retorna uma string com o resumo do bonus.
std::string BonusParaString(const Bonus& bonus);
std::string NomeTipoBonus(TipoBonus tipo);
// Retorna o total de um tipo de bonus.
int BonusTotal(const Bonus& bonus);
int BonusTotalExcluindo(const Bonus& bonus, const std::vector<ent::TipoBonus>& bonus_excluidos);
BonusIndividual::PorOrigem* AtribuiBonus(int valor, TipoBonus tipo, const std::string& origem, Bonus* bonus);
BonusIndividual::PorOrigem* AtribuiBonusPenalidadeSeMaior(int valor, TipoBonus tipo, const std::string& origem, Bonus* bonus);
// Atribui o bonus se valor != 0, remove caso contrario.
void AtribuiOuRemoveBonus(int valor, TipoBonus tipo, const std::string& origem, Bonus* bonus);
void RemoveBonus(TipoBonus tipo, const std::string& origem, Bonus* bonus);
inline void LimpaBonus(TipoBonus tipo, const std::string& origem, Bonus* bonus) { RemoveBonus(tipo, origem, bonus); }
// Limpa todos os bonus individuais presentes em bonus.
void LimpaBonus(const Bonus& bonus_a_remover, Bonus* bonus);
// Retorna um bonus individual.
int BonusIndividualTotal(TipoBonus tipo, const Bonus& bonus);
// Acesso a bonus individual e origem. nullptr se nao achar.
int BonusIndividualTotal(const BonusIndividual& bonus_individual);
int BonusIndividualPorOrigem(TipoBonus tipo, const std::string& origem, const Bonus& bonus);
int BonusIndividualPorOrigem(const std::string& origem, const BonusIndividual& bonus_individual);

// Acessa um bonus especifico se existir.
BonusIndividual* BonusIndividualSePresente(TipoBonus tipo, Bonus* bonus);
BonusIndividual::PorOrigem* OrigemSePresente(const std::string& origem, BonusIndividual* bonus_individual);
BonusIndividual::PorOrigem* OrigemSePresente(TipoBonus tipo, const std::string& origem, Bonus* bonus);
// Retorna true se tipo estiver presente em bonus.
bool PossuiBonus(TipoBonus tipo, const Bonus& bonus);

// Combina os bonus_novos em bonus. Bonus de mesmo tipo e origem serao sobrescritos.
void CombinaBonus(const Bonus& bonus_novos, Bonus* bonus);
// Combina atributos_novos em atributos, sobrescrevendo os iguais (prioridade de atributos_depois).
void CombinaAtributos(const Atributos& atributos_novos, Atributos* atributos);

// Retorna o modificador do atributo.
int ModificadorAtributo(int atributo);
// Leva em consideracao a ausencia de bonus BASE, assumindo ser 10.
int ModificadorAtributo(const Bonus& atributo);
int ModificadorAtributo(TipoAtributo ta, const EntidadeProto& proto);
// Retorna o modificador de atributo de conjuracao para uma determinada classe.
int ModificadorAtributoConjuracao(const std::string& id_classe, const EntidadeProto& proto);
// Retorna bonus por TipoAtributo.
const Bonus& BonusAtributo(TipoAtributo ta, const EntidadeProto& proto);
Bonus* BonusAtributo(TipoAtributo ta, EntidadeProto* proto);
// Util para colocar um bonus base no atributo do proto.
void AtribuiBaseAtributo(int valor, TipoAtributo ta, EntidadeProto* proto);

// Modificador geral de tamanho.
int ModificadorTamanho(TamanhoEntidade tamanho);
int ModificadorTamanhoAgarrar(TamanhoEntidade tamanho);
int ModificadorTamanhoEsconderse(TamanhoEntidade tamanho);
// Dano da entidade por tamanho.
std::string DanoDesarmadoPorTamanho(TamanhoEntidade tamanho);
std::string ConverteDanoBasicoMedioParaTamanho(const std::string& dano_basico_medio, TamanhoEntidade tamanho);

// Retorna o alcance de acordo com tamanho.
int AlcanceTamanhoQuadrados(TamanhoEntidade tamanho);

// Funcoes auxiliares de CA.
int CATotal(const EntidadeProto& proto, bool permite_escudo, const Bonus& outros_bonus = Bonus());
int CASurpreso(const EntidadeProto& proto, bool permite_escudo, const Bonus& outros_bonus = Bonus());
int CAToque(const EntidadeProto& proto, const Bonus& outros_bonus = Bonus());
int CAToqueSurpreso(const EntidadeProto& proto, const Bonus& outros_bonus = Bonus());

bool ArmaDupla(const ArmaProto& arma);
bool ArmaDistancia(const ArmaProto& arma);

// Retorna true se os eventos forem identicos (campo a campo).
bool EventosIguais(const EntidadeProto::Evento& lhs, const EntidadeProto::Evento& rhs);

// Retorna true se a entidade tiver o modelo e estiver ativo.
bool PossuiModeloAtivo(TipoEfeitoModelo efeito_modelo, const EntidadeProto& proto);

// Retorna o evento pelo id unico. Retorna nullptr se nao houver.
EntidadeProto::Evento* AchaEvento(int id_unico, EntidadeProto* proto);
const EntidadeProto::Evento* AchaEvento(int id_unico, const EntidadeProto& proto);
// Retorna verdadeiro se a entidade tiver um evento do tipo passado.
bool PossuiEvento(TipoEfeito tipo, const EntidadeProto& proto);
// Retorna true se possuir evento tipo_sim e nao possui evento tipo_nao.
inline bool PossuiEventoNaoPossuiOutro(TipoEfeito tipo_sim, TipoEfeito tipo_nao, const EntidadeProto& proto) {
  return PossuiEvento(tipo_sim, proto) && !PossuiEvento(tipo_nao, proto);
}
// Retorna verdadeiro se tiver um dos tipo de evento passado.
bool PossuiUmDosEventos(const std::vector<TipoEfeito>& tipos, const EntidadeProto& proto);
bool PossuiEvento(TipoEfeito tipo, const std::string& complemento, const EntidadeProto& proto);
// Retorna verdadeiro se a entidade tiver um evento com mesmo id unico (ou todos campos identicos).
bool PossuiEventoEspecifico(const EntidadeProto& proto, const EntidadeProto::Evento& evento);
// Retorna true se a entidade possuir resistencia do mesmo tipo que o passado, com mesmo valor.
bool PossuiResistenciaEspecifica(const EntidadeProto& proto, const ResistenciaElementos& resistencia);
// Retorna a resistencia a elementos gerada pelo evento de id_unico ou cria se nao houver.
ResistenciaElementos* AchaOuCriaResistenciaElementoIdUnico(DescritorAtaque descritor, int id_unico, EntidadeProto* proto);
void LimpaResistenciaElementoIdUnico(DescritorAtaque descritor, int id_unico, EntidadeProto* proto);
ResistenciaElementos* AchaOuCriaResistenciaElementoEfeitoModelo(DescritorAtaque descritor, TipoEfeitoModelo id_efeito_modelo, EntidadeProto* proto);
void LimpaResistenciaElementoEfeitoModelo(DescritorAtaque descritor, TipoEfeitoModelo id_efeito_modelo, EntidadeProto* proto);

// Funcoes de reducao de dano.
ReducaoDano* AchaOuCriaReducaoDanoEfeitoModelo(TipoEfeitoModelo id_efeito_modelo, EntidadeProto* proto);
void LimpaReducaoDanoEfeitoModelo(TipoEfeitoModelo id_efeito_modelo, EntidadeProto* proto);


// Retorna os eventos do tipo passado.
std::vector<const EntidadeProto::Evento*> EventosTipo(TipoEfeito tipo, const EntidadeProto& proto);

// Retorna true se a classe possuir a salvacao forte do tipo passado.
bool ClassePossuiSalvacaoForte(TipoSalvacao ts, const InfoClasse& ic);

// Retorna a string de critico se for diferente de 20/x2. Exemplo: '(19-20/x3)'. Se for 20/x2, retorna vazio.
std::string StringCritico(const DadosAtaque& da);

// Retorna o resumo da arma, para display na UI. Nao inclui modificadores circunstanciais.
// Algo como id: rotulo, alcance: 10 q, 5 incrementos, bonus +3, dano: 1d8(x3), CA: 15, toque: 12, surpresa: 13.
std::string StringResumoArma(const Tabelas& tabelas, const DadosAtaque& da);

// Retorna os detalhes da arma para display na lista de acoes, incluindo alguns modificadores de circunstancia (como caido, por exemplo).
// Algo como: 'machado: +8+2, 1d8(x3)+2d6, CA 15/15/12'
std::string StringAtaque(const DadosAtaque& da, const EntidadeProto& proto);

// Retorna a string de dano para o ataque, sem critico e com modificadores.
// Exemplo: 1d8+5+2.
// Usado para gerar dano.
std::string StringDanoParaAcao(const DadosAtaque& da, const EntidadeProto& proto, const EntidadeProto& alvo);

// Retorna a string de dano para o ataque, com informacao de critico e sem modificadores.
// Exemplo: 1d8(19-20).
std::string StringDanoBasicoComCritico(const DadosAtaque& da);

// Retorna a string de CA para uma determinada configuracao de ataque. Inclui bonus circunstanciais.
// Exemplo: '(esc+surp) 16, tq: 12'
std::string StringCAParaAcao(const DadosAtaque& da, const EntidadeProto& proto);

// Retorna a string para o efeito passado.
std::string StringEfeito(TipoEfeito efeito);

//--------------------
// Formas Alternativas
//--------------------
// Dado um proto, retorna outro apenas com os campos de forma alternativa setados.
EntidadeProto ProtoFormaAlternativa(const EntidadeProto& proto);
// Adiciona a forma alternativa a proto. Se nao tiver, cria a padrao tb.
void AdicionaFormaAlternativa(const EntidadeProto& proto_forma, EntidadeProto* proto);
// Remove uma forma alternativa do proto. Parametro indice deve ser > 0.
void RemoveFormaAlternativa(int indice, EntidadeProto* proto);
//-------------------------
// Fim Formas Alternativas.
//-------------------------

// Retorna true se uma arma possui a categoria passada.
bool PossuiCategoria(CategoriaArma categoria, const ArmaProto& arma);

// Retorna true se o personagem tiver o talento.
bool PossuiTalento(const std::string& chave_talento, const EntidadeProto& proto);
bool PossuiTalento(const std::string& chave_talento, const std::string& chave_complemento, const EntidadeProto& proto);
// Retorna o talento do personagem, ou nullptr se nao tiver.
const TalentoProto* Talento(const std::string& chave_talento, const EntidadeProto& proto);
const TalentoProto* Talento(const std::string& chave_talento, const std::string& complemento, const EntidadeProto& proto);
TalentoProto* TalentoOuCria(const std::string& chave_talento, EntidadeProto* proto);

// Retorna true se possui a habilidade especial.
bool PossuiHabilidadeEspecial(const std::string& chave, const EntidadeProto& proto);

// Retorna se a pericia eh considerada de classe para o proto.
bool PericiaDeClasse(const Tabelas& tabelas, const std::string& chave_pericia, const EntidadeProto& proto);
// Retorna o total de pontos de pericia permitido para e entidade.
int TotalPontosPericiaPermitidos(const Tabelas& tabelas, const EntidadeProto& proto);

// Funcoes de tendencia.
inline bool Bom(const EntidadeProto& proto)     { return proto.tendencia().eixo_bem_mal() > 0.666f; }
inline bool Mal(const EntidadeProto& proto)     { return proto.tendencia().eixo_bem_mal() <= 0.333f; }
inline bool Ordeiro(const EntidadeProto& proto) { return proto.tendencia().eixo_ordem_caos() > 0.666f;  }
inline bool Caotico(const EntidadeProto& proto) { return proto.tendencia().eixo_ordem_caos() <= 0.333f; }
// Retorna o bonus contra tendencia de um atacante.
Bonus BonusContraTendenciaNaCA(const EntidadeProto& proto_ataque, const EntidadeProto& proto_defesa);
Bonus BonusContraTendenciaNaSalvacao(const EntidadeProto& proto_ataque, const EntidadeProto& proto_defesa);

// Retorna o nivel do id da classe do proto.
int Nivel(const std::string& id, const EntidadeProto& proto);
// Nivel total da entidade.
int Nivel(const EntidadeProto& proto);
// Retorna o nivel de conjurador da entidade para uma determinada acao. Caso tenha classe de conjuracao vazia, usara nivel do personagem.
int NivelConjuradorParaAcao(const AcaoProto& acao, const Entidade& entidade);
// Retorna o nivel de conjuracao para a classe.
int NivelConjurador(const std::string& id_classe, const EntidadeProto& proto);
// Retorna o nivel de conjurador do personagem para lancar um pergaminho. Caso nao haja, retorna -1.
int NivelConjuradorParaLancarPergaminho(const Tabelas& tabelas, TipoMagia tipo_magia, const std::string& id_feitico, const EntidadeProto& proto);
// Retorna o nivel da classe com modificadores para fins de calculo de numeros de magia.
int NivelParaCalculoMagiasPorDia(const Tabelas& tabelas, const std::string& id_classe, const EntidadeProto& proto);
// Retorna o nivel da classe para um tipo de ataque.
// Se o tipo de ataque pertecencer a mais de duas classes, usa a mais alta.
int NivelParaFeitico(const Tabelas& tabelas, const DadosAtaque& da, const EntidadeProto& proto);
// Retorna o nivel para testes de expulsar/fascinar mortos vivos.
int NivelExpulsao(const Tabelas& tabelas, const EntidadeProto& proto);
// Retorna o id de classe para um tipo de ataque. Por exemplo, 'Feitiço de Mago' retorna 'mago'.
// Vazio caso contrario.
std::string TipoAtaqueParaClasse(const Tabelas& tabelas, const std::string& tipo_ataque);
// Retorna a string de acao para a classe. Por exemplo, se for clerigo, retorna 'Feitiço de Clérigo'.
std::string ClasseParaTipoAtaqueFeitico(const Tabelas& tabelas, const std::string& id_classe);
// Retorna a classe que melhor casa com o tipo de ataque. Por exemplo, se o personagem tem nivel de feiticeiro,
// e o tipo eh 'Feitico de Mago', retorna o info de feiticeiro.
const InfoClasse& InfoClasseParaFeitico(
    const Tabelas& tabelas, const std::string& tipo_ataque, const EntidadeProto& proto);
// Retorna o id para magia de uma classe. Por exemplo, feiticeiro usa mago. Note que ranger, druida, paladino
// possuem sua propria lista de magias. Renomear para IdParaConjuracao?
const std::string IdParaMagia(const Tabelas& tabelas, const std::string& id_classe);

// Retorna se o feitico pode ser conjurado no nivel passado pela classe cujo id para magia foi passado.
bool PodeConjurarFeitico(const ArmaProto& feitico, int nivel_maximo, const std::string& id_classe_para_magia);

// Retorna true se o feitico for pessoal.
bool FeiticoPessoal(const Tabelas& tabelas, const ArmaProto& feitico_tabelado);
bool FeiticoPessoalDispersao(const Tabelas& tabelas, const ArmaProto& feitico_tabelado);

// Renova todos os feiticos do proto (ficam prontos para serem usados).
void RenovaFeiticos(EntidadeProto* proto);

// Retorna o nivel maximo de feitico que a classe pode conjurar com o nivel de conjurador passado.
int NivelMaximoFeitico(const Tabelas& tabelas, const std::string& id_classe, int nivel_conjurador);

// Retorna o nivel do feitico para determinada classe, -1 se nao houver.
int NivelFeiticoParaClasse(const ArmaProto& feitico, const std::string& id_classe);

// Hack para android!
/** Realiza a leitura de uma string de eventos, um por linha, formato:
* descricao [(complemento)] : rodadas.
*/
google::protobuf::RepeatedPtrField<EntidadeProto_Evento> LeEventos(const std::string& eventos_str);

// Funcoes que retornam o estado da entidade de acordo com a origem e valor dos bonus de esquiva.
bool EmDefesaTotal(const EntidadeProto& proto);
bool LutandoDefensivamente(const EntidadeProto& proto);

// Retorna um rotulo para a entidade. Tenta o rotulo, id e se for null, retorna null.
std::string RotuloEntidade(const Entidade* entidade);
std::string RotuloEntidade(const EntidadeProto& proto);

// Retorna os ids unicos da entidade.
std::vector<int> IdsUnicosProto(const EntidadeProto& proto);
std::vector<int> IdsUnicosEntidade(const Entidade& entidade);

// Acha um id unico de evento para o proto passado. Normalmente o eventos_entidade vem do proto da entidade e o outro vem do proto que esta sendo
// gerado para atualizar a entidade.
int AchaIdUnicoEvento(const std::vector<int>& ids_unicos);
int AchaIdUnicoEvento(
    const google::protobuf::RepeatedPtrField<EntidadeProto::Evento>& eventos_entidade,
    const google::protobuf::RepeatedPtrField<EntidadeProto::Evento>& eventos_sendo_gerados);
inline int AchaIdUnicoEvento(const google::protobuf::RepeatedPtrField<EntidadeProto::Evento>& eventos) {
  return AchaIdUnicoEvento(eventos, google::protobuf::RepeatedPtrField<EntidadeProto::Evento>());
}

// Adiciona um evento ao proto, gerando o id do efeito automaticamente. Os eventos devem vir da entidade, para correto preenchimento do id unico
// (normalmente proto preenchido nao contem tudo).
// Params ids_unicos é in/out.
EntidadeProto::Evento* AdicionaEvento(
    const std::string& origem, TipoEfeito id_efeito, int rodadas, bool continuo, std::vector<int>* ids_unicos, EntidadeProto* proto);
EntidadeProto::Evento* AdicionaEventoEfeitoAdicional(
    unsigned int id_origem, int nivel_conjurador,
    const AcaoProto::EfeitoAdicional& efeito_adicional, const AcaoProto& acao,
    std::vector<int>* ids_unicos, const Entidade& alvo, EntidadeProto* proto);

// Dado um item magico, adiciona o efeito dele ao proto.
// Retorna os ids unicos dos eventos criados.
// Indice eh usado para itens com multiplos efeito de combinacao exclusiva. Ignorado para outros tipos.
// TODO: rodadas automatico?
void AdicionaEventoItemMagico(
    const ItemMagicoProto& item, int indice, int rodadas, bool continuo,
    std::vector<int>* ids_unicos, EntidadeProto* proto);

inline void AdicionaEventoItemMagicoEfeitoSimples(
    const ItemMagicoProto& item, int rodadas, bool continuo,
    std::vector<int>* ids_unicos, EntidadeProto* proto) {
  AdicionaEventoItemMagico(item, /*indice=*/-1, rodadas, continuo, ids_unicos, proto);
}
inline void AdicionaEventoItemMagicoContinuo(
    const ItemMagicoProto& item, std::vector<int>* ids_unicos, EntidadeProto* proto) {
  AdicionaEventoItemMagico(item, /*indice=*/-1, /*rodadas=*/1, /*continuo=*/true, ids_unicos, proto);
}
// Aqui o item eh do proto, e nao da tabela.
void AdicionaEventosItemMagicoContinuo(
    const Tabelas& tabelas, ItemMagicoProto* item, std::vector<int>* ids_unicos, EntidadeProto* proto);

// Marca a duracao do evento para -1.
void ExpiraEventoItemMagico(int id_unico, EntidadeProto* proto);
// Expira todos os eventos do item.
void ExpiraEventosItemMagico(ItemMagicoProto* item, EntidadeProto* proto);

// Retorna todos os talentos da entidade em um vector, para facilitar.
std::vector<const TalentoProto*> TodosTalentos(const EntidadeProto& proto);

// Retorna false se nao houver alcance ou municao, com texto descritivo. O valor float eh a distancia computada.
std::tuple<std::string, bool, float> VerificaAlcanceMunicao(
    const AcaoProto& ap, const Entidade& ea, const Entidade& ed, const Posicao& pos_alvo);

// Retorna o modificador de incrementos. Assume alcance e municao.
int ModificadorAlcance(float distancia_m, const AcaoProto& ap, const Entidade& ea);
// Distancia da acao da ea para a ed. Pos alvo indica a posicao exata no alvo.
float DistanciaAcaoAoAlvoMetros(const Entidade& ea, const Entidade& ed, const Posicao& pos_alvo);

// Acesso a pericias do proto.
InfoPericia* PericiaCriando(const std::string& id, EntidadeProto* proto);
InfoPericia* PericiaOuNullptr(const std::string& id, EntidadeProto* proto);
// Retorna default caso nao encontre.
const InfoPericia& Pericia(const std::string& id, const EntidadeProto& proto);
// Retorna o valor final da pericia para o personagem.
int ValorFinalPericia(const std::string& id, const EntidadeProto& proto);

// Retorna se o proto esta agarrado ao id.
bool AgarradoA(unsigned int id, const EntidadeProto& proto);

// Acesso aos poderes de dominios de clerigo.
const EntidadeProto::PoderesDominio& PoderesDoDominio(
    const std::string& id_dominio, const EntidadeProto& proto);
EntidadeProto::PoderesDominio* PoderesDoDominio(
    const std::string& id_dominio, EntidadeProto* proto);
// Retorna os feiticos da classe. Para as versoes mutaveis, uma cria e a outra retorna nullptr.
const EntidadeProto::InfoFeiticosClasse& FeiticosClasse(
    const std::string& id_classe, const EntidadeProto& proto);
EntidadeProto::InfoFeiticosClasse* FeiticosClasse(const std::string& id_classe, EntidadeProto* proto);
EntidadeProto::InfoFeiticosClasse* FeiticosClasseOuNullptr(const std::string& id_classe, EntidadeProto* proto);

// Retorna os feiticos de nivel da classe. Ditto sobre mutaveis.
const EntidadeProto::FeiticosPorNivel& FeiticosNivel(
    const std::string& id_classe, int nivel, const EntidadeProto& proto);
EntidadeProto::FeiticosPorNivel* FeiticosNivel(
    const std::string& id_classe, int nivel, EntidadeProto* proto);
EntidadeProto::FeiticosPorNivel* FeiticosNivelOuNullptr(
    const std::string& id_classe, int nivel, EntidadeProto* proto);

// Retorna se a classe tem feitico para o nivel.
bool TemFeiticoDisponivel(const std::string& id_classe, int nivel, const EntidadeProto& proto);
// Retorna o indice do feitico disponivel da classe (para saber qual gastar, para feiticeiros e bardos por exemplo).
// Retorna -1 se nao houver.
int IndiceFeiticoDisponivel(const std::string& id_classe, int nivel, const EntidadeProto& proto);

// Retorna true se a classe tiver que conhecer feiticos para lancar, como bardos e feiticeiros.
bool ClasseDeveConhecerFeitico(const Tabelas& tabelas, const std::string& id_classe);

// Retorna se a classe tiver que memorizar feiticos antes de lancar (mago, clerigo, paladino etc).
// Retorna false para feiticeiro e bardo, por exemplo.
bool ClassePrecisaMemorizar(const Tabelas& tabelas, const std::string& id_classe);

// Retorna a classe de feitico ativa para o personagem.
const ent::EntidadeProto::InfoFeiticosClasse& InfoClasseFeiticoAtiva(const EntidadeProto& proto);
inline const std::string& ClasseFeiticoAtiva(const EntidadeProto& proto) {
  return InfoClasseFeiticoAtiva(proto).id_classe();
}
// Retorna a proxima classe de feitico ativa para o proto (vazio se nao houver, a mesma se houver apenas uma).
const std::string& ProximaClasseFeiticoAtiva(const EntidadeProto& proto);

// Retorna o nome do feitico conhecido ou default_instance se nao houver.
const EntidadeProto::InfoConhecido& FeiticoConhecido(
    const std::string& id_classe, int nivel, int indice, const EntidadeProto& proto);
inline const EntidadeProto::InfoConhecido& FeiticoConhecido(
    const std::string& id_classe, const ent::EntidadeProto::InfoLancar& para_lancar,
    const EntidadeProto& proto) {
  return FeiticoConhecido(id_classe, para_lancar.nivel_conhecido(), para_lancar.indice_conhecido(), proto);
}

// Retorna o feitico para lancar ou default_instance se nao houver.
const EntidadeProto::InfoLancar& FeiticoParaLancar(
    const std::string& id_classe, int nivel, int indice, const EntidadeProto& proto);

// Retorna uma notificacao de alterar feitico para um personagem.
std::unique_ptr<ntf::Notificacao> NotificacaoAlterarFeitico(
    const std::string& id_classe, int nivel, int indice, bool usado, const EntidadeProto& proto);

// Preenche as notificacoes de consequencia de um feitico. Para feiticos pessoais, o efeito sera aplicado.
// Para os demais, cria um ataque com o efeito do feitico.
// Retorna true se criou um ataque.
bool NotificacaoConsequenciaFeitico(
    const Tabelas& tabelas, const std::string& id_classe, bool conversao_espontanea, int nivel, int indice, const Entidade& entidade, ntf::Notificacao* grupo);

std::tuple<std::string, int, int, bool, unsigned int> DadosNotificacaoAlterarFeitico(const ntf::Notificacao& n);

// Cria uma notificacao de dialogo de escolher feitico. A notificacao tera a entidade com apenas a classe
// de feitico com todos ate o nivel desejado.
std::unique_ptr<ntf::Notificacao> NotificacaoEscolherFeitico(bool conversao_espontanea, const std::string& id_classe, int nivel, const EntidadeProto& proto);

// Retorna true se a entidade for imune a todos os descritores do elemento.
bool EntidadeImuneElemento(const EntidadeProto& proto, int elementos);
// Retorna a melhor resistencia da entidade contra o elemento ou nullptr se nao houver.
const ResistenciaElementos* EntidadeResistenciaElemento(const EntidadeProto& proto, DescritorAtaque elemento);
// Retorna true se a entidade for imune ao feitico.
bool EntidadeImuneFeitico(const EntidadeProto& proto, const std::string& id);

// retorna o descritor em formato texto.
const char* TextoDescritor(int descritor);

enum alteracao_delta_e {
  ALT_NENHUMA = 0,
  ALT_IMUNIDADE = 1,
  ALT_RESISTENCIA = 2,
};
struct ResultadoImunidadeOuResistencia {
  int resistido = 0;  // nunca excedera o valor absoluto de delta_pv.
  std::string texto;  // texto da alteracao.
  alteracao_delta_e causa = ALT_NENHUMA;  // o que causou a alteracao (imunidade, resistencia, nenhum).
  const ResistenciaElementos* resistencia = nullptr;  // qual resistencia barrou.
};
// Retorna se o ataque foi resistido, por que tipo de defesa e qual o valor resistido, que nunca passara de -delta_pv.
ResultadoImunidadeOuResistencia ImunidadeOuResistenciaParaElemento(int delta_pv, const DadosAtaque& da, const EntidadeProto& proto, DescritorAtaque elemento);

// Altera o delta_pv de acordo com as reducoes do alvo e tipo de ataque.
std::tuple<int, std::string> AlteraDeltaPontosVidaPorReducaoNormal(
    int delta_pv, const EntidadeProto& proto, const google::protobuf::RepeatedField<int>& descritores);

// Altera o delta_pv de acordo com reducoes de barbaro.
std::tuple<int, std::string> AlteraDeltaPontosVidaPorReducaoBarbaro(
    int delta_pv, const EntidadeProto& proto);

// Altera o delta_pv de acordo com a melhor reducao de dano. Retorna a string de reducao e o id_unico, se houver.
struct ResultadoReducaoDano {
  int delta_pv = 0;
  std::string texto;
  int id_unico = -1;
};
ResultadoReducaoDano AlteraDeltaPontosVidaPorMelhorReducao(
    int delta_pv, const EntidadeProto& proto, const google::protobuf::RepeatedField<int>& descritores);

// Return true se a acao ignora reducao de dano.
inline bool IgnoraReducaoDano(const DadosAtaque* da, const AcaoProto& acao) {
  return (da != nullptr && da->has_elemento()) || acao.ignora_reducao_dano_barbaro();
}

// Retorna true se a acao afetar o alvo. Alguns tipos de acoes afetam tipos de
// alvos especificos, como agua benta, que afeta apenas mortos-vivos e
// extraplanares.
//
// Caso a acao seja generica (nao tem afeta_apenas), retornara true.
bool AcaoAfetaAlvo(const AcaoProto& acao_proto, const Entidade& entidade, std::string* texto = nullptr);

// Retorna o numero de reflexos da entidade (feitico reflexos).
int NumeroReflexos(const EntidadeProto& proto);

// Retorna o item tabelado pelo tipo e id.
const ItemMagicoProto& ItemTabela(
    const Tabelas& tabelas, TipoItem tipo, const std::string& id);
const ItemMagicoProto& ItemTabela(const Tabelas& tabelas, const ItemMagicoProto& item);

// Retorna o repeated do tipo passado para o proto.
const google::protobuf::RepeatedPtrField<ent::ItemMagicoProto>& ItensProto(TipoItem tipo, const EntidadeProto& proto);
google::protobuf::RepeatedPtrField<ent::ItemMagicoProto>* ItensProtoMutavel(TipoItem tipo, EntidadeProto* proto);
// Retorna todos os itens do proto, exceto pocoes.
std::vector<const ItemMagicoProto*> TodosItensExcetoPocoes(const EntidadeProto& proto);
std::vector<ItemMagicoProto*> TodosItensExcetoPocoes(EntidadeProto* proto);
// Remove o item de proto.
void RemoveItem(const ItemMagicoProto& item, EntidadeProto* proto);

// Retorna a cura acelerada do alvo, ou 0 se houver.
int CuraAcelerada(const EntidadeProto& proto);

// https://www.learnopencv.com/rotation-matrix-to-euler-angles/
Vector3 RotationMatrixToAngles(const Matrix3& matrix);

// Gera a matriz de decomposicao do pai para filhos.
Matrix4 MatrizDecomposicaoPai(const EntidadeProto& pai);
// Decompoe o filho, usando a matriz do pai e a dele proprio.
void DecompoeFilho(const Matrix4& matriz_pai, EntidadeProto* filho);

// Retorna true se o modelo for desligavel.
bool ModeloDesligavel(const Tabelas& tabelas, const ModeloDnD& modelo);

// Encontra o modelo de entidade com o efeito passado. Retorna nullptr se nao houver.
ModeloDnD* EncontraModelo(TipoEfeitoModelo id_efeito, EntidadeProto* proto);

// Retorna true se a entidade tiver algum modelo desligavel que esta ligado.
bool EntidadeTemModeloDesligavelLigado(const Tabelas& tabelas, const EntidadeProto& proto);

// Retorna a classe dentro do proto, ou default se nao houver.
const InfoClasse& InfoClasseProto(const std::string& id_classe, const EntidadeProto& proto);
// Como InfoClasseProto, mas considera tb classes com id_para_magia.
const InfoClasse& InfoClasseProtoParaMagia(const std::string& id_classe, const EntidadeProto& proto);

// Retorna true se a entidade pode agir. Alguns efeitos nao permite (pasmar, atordoado, etc).
std::pair<bool, std::string> PodeAgir(const EntidadeProto& proto);

// Retorna true se puder usar destreza na CA. Algumas condicoes impedem isso (surpresa, atordoado).
bool DestrezaNaCA(const EntidadeProto& proto);
bool DestrezaNaCAContraAtaque(
    const DadosAtaque* da, const EntidadeProto& proto, const EntidadeProto& proto_ataque = EntidadeProto::default_instance());

// Retorna true se puder usar escudo. Algumas condicoes impedem isso (atordoado).
bool PermiteEscudo(const EntidadeProto& proto);
// Retorna true se o personagem puder usar o escudo passado (por chave).
bool TalentoComEscudo(const std::string& escudo, const EntidadeProto& proto);

// Dado o feitico que originou, os parametros e a entidade de referencia, preenche `modelo`.
void PreencheModeloComParametros(const ArmaProto& feitico, const Modelo::Parametros& parametros, const Entidade& referencia, EntidadeProto* modelo);

// Computa o dano do dado de ataque baseado no modelo e nivel passado.
void ComputaDano(ArmaProto::ModeloDano modelo_dano, int nivel_conjurador, DadosAtaque* da);

// Retorna o tipo de evasao da entidade (ja computado).
TipoEvasao TipoEvasaoPersonagem(const EntidadeProto& proto);

// Retorna true se o proto tiver o tipo passado.
inline bool TemTipoDnD(TipoDnD tipo, const EntidadeProto& proto) { return c_any(proto.tipo_dnd(), tipo); }
inline bool TemSubTipoDnD(SubTipoDnD sub_tipo, const EntidadeProto& proto) { return c_any(proto.sub_tipo_dnd(), sub_tipo); }

bool PreencheInfoTextura(
    const std::string& nome, arq::tipo_e tipo, InfoTextura* info_textura,
    unsigned int* plargura = nullptr, unsigned int* paltura = nullptr);

// Retorna true se proto estiver indefeso.
bool Indefeso(const EntidadeProto& proto);

// Concatena 's' ao string alvo. Se o texto era vazio, passa a ser 's'. Caso contrario, adiciona '\n' seguido de 's'.
void ConcatenaString(const std::string& s, std::string* alvo);

// Se alvo tiver efeito de compartilhamento de dano com alguem, aplica.
int CompartilhaDanoSeAplicavel(
    int delta_pontos_vida, const EntidadeProto& alvo, const Tabuleiro& tabuleiro, tipo_dano_e tipo_dano,
    AcaoProto::PorEntidade* por_entidade, AcaoProto* acao, ntf::Notificacao* grupo_desfazer);

// Caso a entidade tenha dominio renovar, ativa caso fique negativo.
// Caso haja uso do poder, adiciona ao grupo a notificacao.
std::pair<int, std::string> RenovaSeTiverDominioRenovar(const EntidadeProto& proto, int delta_pontos_vida, ntf::Notificacao* n, ntf::Notificacao* grupo_desfazer);

// Se alvo possuir desviar objetos e puder usar, anula o ataque.
int DesviaObjetoSeAplicavel(
    const Tabelas& tabelas, int delta_pontos_vida, const Entidade& alvo, const DadosAtaque& da, Tabuleiro* tabuleiro,
    AcaoProto::PorEntidade* por_entidade, ntf::Notificacao* grupo_desfazer);

bool EscolaBoaTramaDasSombras(const ArmaProto& feitico);
bool EscolaRuimTramaDasSombras(const ArmaProto& feitico);

// Computa o limite de vezes de um ataque gerado por feitico ou pergaminho.
int ComputaLimiteVezes(ModeloGenerico modelo_limite_vezes, int nivel_conjurador);

// Mexe nos bits da entidade depois de uma alteracao de pontos de vida.
// Parametros 'pontos_vida' deve incluir temporarios.
void PreencheNotificacaoConsequenciaAlteracaoPontosVida(int pontos_vida, int dano_nao_letal, const EntidadeProto& proto, ntf::Notificacao* n);

// Retorna o total de pontos de vida da entidade (incluindo temporarios).
int PontosVida(const EntidadeProto& proto);

// Retorna a classe do proto que lanca o feitico passado para o tipo de pergaminho.
const InfoClasse& ClasseParaLancarPergaminho(
    const Tabelas& tabelas, TipoMagia tipo_magia, const std::string& id_feitico, const EntidadeProto& proto);

// Retorna o nivel da magia para uma determinada classe.
int NivelMagia(const ArmaProto& magia, const InfoClasse& ic);
// Retorna o nivel mais alto para a magia.
int NivelMaisAltoMagia(const ArmaProto& magia);

int NivelPersonagem(const EntidadeProto& proto);

bool MesmaTendencia(TendenciaSimplificada tendencia, const EntidadeProto& proto);

bool ImuneAcaoMental(const EntidadeProto& proto);

bool NaoEnxerga(const EntidadeProto& proto);

// Antes de aplicar os efeitos adicionais, resolve a parte variavel que for possivel. Exemplo: rodadas.
void ResolveEfeitosAdicionaisVariaveis(int nivel_conjurador, const EntidadeProto& lancador, const Entidade& alvo, AcaoProto* acao_proto);

float AplicaEfeitosAdicionais(
    const Tabelas& tabelas,
    float atraso_s, bool salvou, const Entidade& entidade_origem, const Entidade& entidade_destino, const DadosAtaque& da,
    AcaoProto::PorEntidade* por_entidade, AcaoProto* acao_proto, std::vector<int>* ids_unicos_origem, std::vector<int>* ids_unicos_destino,
    ntf::Notificacao* grupo_desfazer, ntf::CentralNotificacoes* central);

// Encontra um ataque no proto igual a da. Compara rotulo, tipo e grupo.
DadosAtaque* EncontraAtaque(const DadosAtaque& da, EntidadeProto* proto);

// Retorna o bonus de salvacao contra veneno.
int Salvacao(const EntidadeProto& proto, const Bonus& outros_bonus, const EntidadeProto& proto_atacante, TipoSalvacao tipo);
int SalvacaoVeneno(const EntidadeProto& proto);
int SalvacaoFeitico(const ArmaProto& feitico_tabelado, const EntidadeProto& proto, const EntidadeProto& proto_atacante, TipoSalvacao tipo);

// Retorna true se ataque for uma arma natural.
bool ArmaNatural(const ArmaProto& arma);

// Retorna true se o feitico for de um dos dominios passados.
bool FeiticoDominio(const std::vector<std::string>& dominios, const ArmaProto& feitico_tabelado);

// Retorna true se o feitico for de uma escola proibida.
bool FeiticoEscolaProibida(const std::vector<std::string>& escolas_proibidas, const ArmaProto& feitico_tabelado);

}  // namespace ent

#endif  // ENT_UTIL_H
