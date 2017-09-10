#ifndef ENT_UTIL_H
#define ENT_UTIL_H

#include <algorithm>
#include <cctype>
#include <functional>
#include <string>
#include <tuple>
#include <vector>
#include <google/protobuf/repeated_field.h>
#include "ent/tabelas.pb.h"
#include "matrix/matrices.h"
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

void IniciaUtil();

/** Altera a cor correnta para cor. Nao considera alpha. */
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

/** Reduz cada componente RGB em 0.5f. */
const Cor EscureceCor(const Cor& cor);
void EscureceCor(Cor* cor);
void EscureceCor(float* cor);

/** Aumenta cada componente RGB em 0.5f. */
void ClareiaCor(Cor* cor);

/** Muda cada componente RGB de forma notavel. */
void RealcaCor(Cor* cor);
void RealcaCor(float* cor);

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
float DistanciaQuadrado(const Posicao& pos1, const Posicao& pos2);

/** Roda o vetor no eixo Z. */
void RodaVetor2d(float graus, Posicao* vetor);

/** Retorna true se o ponto estiver dentro do poligono. */
bool PontoDentroDePoligono(const Posicao& ponto, const std::vector<Posicao>& vertices);

/** Posicionamento do raster em 2d. */
void PosicionaRaster2d(int x, int y);

/** Converte uma string para o efeito, se houver. Caso contrario retorna EFEITO_INVALIDO. */
TipoEfeito StringParaEfeito(const std::string& s);

// Trim functions from: http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring.
static inline std::string& ltrim(std::string& s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

// trim from end
static inline std::string& rtrim(std::string& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  return s;
}

// trim from both ends
static inline std::string& trim(std::string& s) {
  return ltrim(rtrim(s));
}

const std::string StringSemUtf8(const std::string& id_acao);

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

// Blend de entidades compostas e modelos3d. O blend deve ser restaurado para o valor padrao depois para nao
// avacalhar os proximos.
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

// Retorna alguns modificadores de ataque para a entidade de acordo com seus status e do defensor.
int ModificadorAtaque(bool distancia, const EntidadeProto& ea, const EntidadeProto& ed);
// Retorna alguns modificadores de dano genericos para a entidade de acordo com seus status.
int ModificadorDano(const EntidadeProto& ea);

// Rola o dado de ataque vs defesa, retornando o numero de vezes que o dano deve ser aplicado e o texto da jogada.
// O ultimo parametro indica se a acao deve ser desenhada (em caso de distancia maxima atingida, retorna false).
// Caso haja falha critica, retorna vezes = -1;
// Posicao ataque eh para calculo de distancia.
std::tuple<int, std::string, bool> AtaqueVsDefesa(const Entidade& ea, const Entidade& ed, const Posicao& pos_alvo);

// Rola o dado de ataque contra a salvacao, retornando o dano, texto do resultado.
std::tuple<int, std::string> AtaqueVsSalvacao(const AcaoProto& ap, const Entidade& ea, const Entidade& ed);

// Gera um resumo sobre a notificacao, ou vazio.
std::string ResumoNotificacao(const Tabuleiro& tabuleiro, const ntf::Notificacao& n);

inline Vector3 PosParaVector3(const Posicao& pos) { return Vector3(pos.x(), pos.y(), pos.z()); }

void PreencheNotificacaoAtualizaoPontosVida(
    const Entidade& entidade, int delta_pontos_vida, bool nao_letal, ntf::Notificacao* n, ntf::Notificacao* n_desfazer = nullptr);

// Recomputa as dependencias do proto.
void RecomputaDependencias(const Tabelas& tabelas, EntidadeProto* proto);

// Retorna o total de um tipo de bonus.
int BonusTotal(const Bonus& bonus);
int BonusTotalExcluindo(const Bonus& bonus, const std::vector<ent::TipoBonus>& bonus_excluidos);
BonusIndividual::PorOrigem* AtribuiBonus(int valor, TipoBonus tipo, const std::string& origem, Bonus* bonus);
void AtribuiBonusSeMaior(int valor, TipoBonus tipo, const std::string& origem, Bonus* bonus);
void RemoveBonus(TipoBonus tipo, const std::string& origem, Bonus* bonus);
inline void LimpaBonus(TipoBonus tipo, const std::string& origem, Bonus* bonus) { RemoveBonus(tipo, origem, bonus); }
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
// Retorna bonus por TipoAtributo.
const Bonus& BonusAtributo(TipoAtributo ta, const EntidadeProto& proto);
Bonus* BonusAtributo(TipoAtributo ta, EntidadeProto* proto);
// Util para colocar um bonus base no atributo do proto.
void AtribuiBaseAtributo(int valor, TipoAtributo ta, EntidadeProto* proto);

// Modificador geral de tamanho.
int ModificadorTamanho(TamanhoEntidade tamanho);
int ModificadorTamanhoAgarrar(TamanhoEntidade tamanho);
// Dano da entidade por tamanho.
std::string DanoDesarmadoPorTamanho(TamanhoEntidade tamanho);

// Retorna o alcance de acordo com tamanho.
int AlcanceTamanhoQuadrados(TamanhoEntidade tamanho);

// Funcoes auxiliares de CA.
int CATotal(const EntidadeProto& proto, bool permite_escudo, const Bonus& outros_bonus = Bonus());
int CASurpreso(const EntidadeProto& proto, bool permite_escudo, const Bonus& outros_bonus = Bonus());
int CAToque(const EntidadeProto& proto, const Bonus& outros_bonus = Bonus());
int CAToqueSurpreso(const EntidadeProto& proto, const Bonus& outros_bonus = Bonus());

bool ArmaDupla(const ArmaProto& arma);
bool ArmaDistancia(const ArmaProto& arma);

// Retorna verdadeiro se a entidade tiver um evento do tipo passado.
bool PossuiEvento(TipoEfeito tipo, const EntidadeProto& entidade);
// Retorna verdadeiro se a entidade tiver um evento com mesmo id e descricao.
bool PossuiEventoEspecifico(const EntidadeProto::Evento& evento, const EntidadeProto& entidade);

// Passa alguns dados de acao proto para dados ataque. Preenche o tipo com o tipo da arma se nao houver.
void AcaoParaAtaque(const ArmaProto& arma, const AcaoProto& acao_proto, EntidadeProto::DadosAtaque* dados_ataque);

// Retorna true se a classe possuir a salvacao forte do tipo passado.
bool ClassePossuiSalvacaoForte(TipoSalvacao ts, const InfoClasse& ic);

// Retorna a string de critico se for diferente de 20/x2. Exemplo: '(19-20/x3)'. Se for 20/x2, retorna vazio.
std::string StringCritico(const EntidadeProto::DadosAtaque& da);

// Retorna o resumo da arma, para display na UI. Nao inclui modificadores circunstanciais.
// Algo como id: rotulo, alcance: 10 q, 5 incrementos, bonus +3, dano: 1d8(x3), CA: 15, toque: 12, surpresa: 13.
std::string StringResumoArma(const Tabelas& tabelas, const EntidadeProto::DadosAtaque& da);

// Retorna os detalhes da arma para display na lista de acoes, incluindo alguns modificadores de circunstancia (como caido, por exemplo).
// Algo como: 'machado: +8+2, 1d8(x3)+2d6, CA 15/15/12'
std::string StringAtaque(const EntidadeProto::DadosAtaque& da, const EntidadeProto& proto);

// Retorna a string de dano para o ataque, sem critico e com modificadores.
// Exemplo: 1d8+5+2.
// Usado para gerar dano.
std::string StringDanoParaAcao(const EntidadeProto::DadosAtaque& da, const EntidadeProto& proto);

// Retorna a string de dano para o ataque, com informacao de critico e sem modificadores.
// Exemplo: 1d8(19-20).
std::string StringDanoBasicoComCritico(const EntidadeProto::DadosAtaque& da);

// Retorna a strinf de CA para uma determinada configuracao de ataque. Inclui bonus circunstanciais.
// Exemplo: '(esc+surp) 16, tq: 12'
std::string StringCAParaAcao(const EntidadeProto::DadosAtaque& da, const EntidadeProto& proto);

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
bool PossuiTalento(const std::string& chave_talento, const EntidadeProto& entidade);
bool PossuiTalento(const std::string& chave_talento, const std::string& chave_complemento, const EntidadeProto& entidade);
// Retorna o talento do personagem, ou nullptr se nao tiver.
const TalentoProto* Talento(const std::string& chave_talento, const EntidadeProto& entidade);
const TalentoProto* Talento(const std::string& chave_talento, const std::string& complemento, const EntidadeProto& entidade);

// Retorna se a pericia eh considerada de classe para o proto.
bool PericiaDeClasse(const Tabelas& tabelas, const std::string& chave_pericia, const EntidadeProto& proto);

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

// Remove os campos para os quais predicado retornar true.
template <class T>
void RemoveSe(const std::function<bool(const T& t)>& predicado, google::protobuf::RepeatedPtrField<T>* c);

// Acha um id unico de evento para o proto passado.
uint32_t AchaIdUnicoEvento(const google::protobuf::RepeatedPtrField<EntidadeProto::Evento>& eventos);
inline uint32_t AchaIdUnicoEvento(const EntidadeProto& proto) { return AchaIdUnicoEvento(proto.evento()); }

// Adiciona um evento ao proto, gerando o id do efeito automaticamente.
EntidadeProto::Evento* AdicionaEvento(TipoEfeito id_efeito, int rodadas, EntidadeProto* proto);

// Retorna todos os talentos da entidade em um vector, para facilitar.
std::vector<const TalentoProto*> TodosTalentos(const EntidadeProto& proto);

// Retorna o modificador de incrementos (-inf, 0], string de erro e se tem alcance.
std::tuple<int, std::string, bool> ModificadorAlcanceMunicao(const Entidade& ea, const Entidade& ed, const Posicao& pos_alvo);

// Acesso a pericias do proto.
EntidadeProto::InfoPericia* PericiaCriando(const std::string& id, EntidadeProto* proto);
EntidadeProto::InfoPericia* PericiaOuNullptr(const std::string& id, EntidadeProto* proto);
// Retorna default caso nao encontre.
const EntidadeProto::InfoPericia& Pericia(const std::string& id, const EntidadeProto& proto);

}  // namespace ent

#endif  // ENT_UTIL_H
