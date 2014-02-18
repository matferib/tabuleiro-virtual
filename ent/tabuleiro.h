#ifndef ENT_TABULEIRO_H
#define ENT_TABULEIRO_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <vector>
#include "ent/acoes.pb.h"
#include "ent/entidade.pb.h"
#include "ent/tabuleiro.pb.h"
#include "ntf/notificacao.h"

namespace ntf {
class Notificacao;
}  // namespace ntf

namespace ent {

class Acao;
class Entidade;
class InfoTextura;
class Texturas;

/** botoes do mouse. */
enum botao_e {
  BOTAO_NENHUM,
  BOTAO_ESQUERDO,
  BOTAO_DIREITO,
  BOTAO_MEIO
};

/** teclas reconhecidas. Mesmo valor do QT para simplificar. */
enum tecla_e {
  TECLA_DEL = 0x01000007,
};

/** Estados possiveis do tabuleiro. */
enum etab_t {
  ETAB_OCIOSO,
  ETAB_ROTACAO,
  ETAB_DESLIZANDO,
  ETAB_ENT_PRESSIONADA,
  ETAB_ENT_SELECIONADA,
  ETAB_ENTS_SELECIONADAS,
  ETAB_QUAD_PRESSIONADO,
  ETAB_QUAD_SELECIONADO,
  ETAB_SELECIONANDO_ENTIDADES,
};

struct Sinalizador {
  ent::Posicao pos;
  double estado;
};

typedef std::unordered_map<unsigned int, std::unique_ptr<Entidade>> MapaEntidades;
typedef std::unordered_set<unsigned int> MapaClientes;

/** Responsavel pelo mundo do jogo. O sistema de coordenadas tera X apontando para o leste, 
* Y para o norte e Z para alto. Cada unidade corresponde a um metro, portanto os quadrados 
* sao de tamanho 1,5m.
*/
class Tabuleiro : public ntf::Receptor {
 public:
  explicit Tabuleiro(Texturas* texturas, ntf::CentralNotificacoes* central);

  /** libera os recursos do tabuleiro, inclusive entidades. */
  virtual ~Tabuleiro();

  /** @return numero de quadrados no eixo E-W. */
  int TamanhoX() const;

  /** @return numero de quadrados no eixo N-S. */
  int TamanhoY() const;

  /** adiciona a entidade ao tabuleiro, através de uma notificação.
  * @throw logic_error se o limite de entidades for alcançado.
  */
  void AdicionaEntidade(const ntf::Notificacao& notificacao);

  /** remove entidade do tabuleiro, pelo id da entidade passada ou a selecionada se nao houver id de entidade. 
  */
  void RemoveEntidade(const ntf::Notificacao& notificacao);

  /** Inverte o bit da entidade. */
  enum bit_e {
    BIT_VISIBILIDADE     = 0x1,
    BIT_ILUMINACAO       = 0x2,
    BIT_VOO              = 0x4,
    BIT_MORTA            = 0x8,
    BIT_CAIDA            = 0x10,
  };
  /** Atualiza algum campo booleano da entidade selecionada, invertendo-o.
  * O valor eh uma mascara de OUs de bit_e.
  */
  void AtualizaBitsEntidade(int bits);

  /** Adiciona delta_pontos_vida aos pontos de vida da entidade selecionada. */
  void AtualizaPontosVidaEntidade(int delta_pontos_vida);
  /** Atualiza os pontos de vida de uma entidade. */
  void AtualizaPontosVidaEntidade(unsigned int id, int delta_pontos_vida);

  /** Poe pv no final da lista de pontos de vida acumuladors. */
  void AcumulaPontosVida(int pv);
  /** Limpa a lista de pontos de vida. */
  void LimpaListaPontosVida();
  /** Limpa a ultima entrada da lista de pontos de vida. */
  void LimpaUltimoListaPontosVida();

  /** desenha o mundo. */
  void Desenha();

  /** Interface receptor. */
  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

  /** Trata teclado. */
  void TrataTeclaPressionada(int tecla);

  /** trata evento de rodela de mouse. */
  void TrataRodela(int delta);

  /** trata movimento do mouse (y ja em coordenadas opengl). */
  void TrataMovimento(botao_e botao, int x, int y);

  /** Trata o movimento do mouse apos ficar em repouso. */
  void TrataMovimento();

  /** trata o botao do mouse liberado. */
  void TrataBotaoLiberado(botao_e botao);

  /** trata o botao pressionado, recebendo x, y (ja em coordenadas opengl). */
  void TrataBotaoPressionado(botao_e botao, int x, int y);
  void TrataBotaoAlternarSelecaoEntidadePressionado(int x, int y);

  /** trata o botao pressionado em modo de acao, recebendo x, y (ja em coordenadas opengl). 
  * Acao com botao esquerdo respeita a selecao de padrao, botao direito usa sinalizacao.
  * Algumas acoes podem causar um delta de pontos de vida no alvo, indicado por delta_pontos_vida.
  */
  void TrataBotaoAcaoPressionado(botao_e botao, int x, int y);

  /** Trata o click duplo, recebendo x, y (coordenadas opengl). */
  void TrataDuploClique(botao_e botao, int x, int y);

  /** Trata o evento de mouse parado em um determinado local (coordenadas opengl). */
  void TrataMouseParadoEm(int x, int y);

  /** trata a redimensao da janela. */
  void TrataRedimensionaJanela(int largura, int altura);

  /** inicializa os parametros do openGL. */
  static void InicializaGL();

  /** Seleciona o modelo de entidade através do identificador. */
  void SelecionaModeloEntidade(const std::string& id_modelo);

  /** Acesso ao modelo de entidade selecionado. */
  const EntidadeProto* ModeloSelecionado() const { return modelo_selecionado_; }

  /** Acesso ao mapa de modelos. */
  const std::unordered_map<std::string, std::unique_ptr<EntidadeProto>>& MapaModelos() const {
    return mapa_modelos_;
  }

  /** Seleciona o modelo de entidade através do identificador. */
  void SelecionaAcao(const std::string& id_acao);

  /** Acesso ao modelo de entidade selecionado. */
  const AcaoProto* AcaoSelecionada() const { return acao_selecionada_; }

  /** Acesso ao mapa de modelos. */
  const std::unordered_map<std::string, std::unique_ptr<AcaoProto>>& MapaAcoes() const { return mapa_acoes_; }

  /** @return a entidade por id, ou nullptr se nao encontrá-la. */
  Entidade* BuscaEntidade(unsigned int id);

  /** Copia todas as entidades selecionadas para 'entidades_copiadas_'. */
  void CopiaEntidadesSelecionadas();

  /** Cola as 'entidades_copiadas_', gerando entidades com ids diferentes. */
  void ColaEntidadesSelecionadas();

 private:
  /** Poe o tabuleiro nas condicoes iniciais. */
  void EstadoInicial();

  /** Libera a textura do tabuleiro, se houver. */
  void LiberaTextura();

  /** funcao que desenha a cena independente do modo. */
  void DesenhaCena();

  /** Atualiza a posição do olho na direção do quadrado selecionado ou da entidade selecionada. */
  void AtualizaOlho();

  /** Encontra os hits de um clique em objetos. Desabilita iluminacao, texturas, grades, deixando apenas
  * as entidades e tabuleiros a serem pegos. Para desabilitar entidades, basta desliga-la antes da chamada
  * desta funcao.
  */
  void EncontraHits(int x, int y, unsigned int* numero_hits, unsigned int* buffer_hits);

  /** Busca informacoes sobre o hit mais proximo de uma coordenada de mouse X Y (openGL). */
  void BuscaHitMaisProximo(
      int x, int y, unsigned int* id, unsigned int* pos_pilha, float* profundidade = nullptr);

  /** Dada uma coordenada de mouse (x, y) retorna o valor (x, y, z) 3d do objeto projetado mais proximo.
  * Chama BuscaHitMaisProximo para obter a profundidade entao faz a projecao para aquele ponto, chamando 
  * MousePara3d.
  */
  bool MousePara3d(int x, int y, double* x3d, double* y3d, double* z3d);

  /** Dada uma profundidade, faz a projecao inversa (2d para 3d). */
  bool MousePara3d(int x, int y, float profundidade, double* x3d, double* y3d, double* z3d);

  /** trata o clique do botao esquerdo, preparando para movimento de arrastar entidades.
  * Seleciona a entidade o clique for nela. Se alterna_selecao for true, alterna o estado de selecao da entidade.
  */
  void TrataCliqueEsquerdo(int x, int y, bool alterna_selecao = false);

  /** trata o click do botao direito, preparando para movimento de deslizamento. */
  void TrataCliqueDireito(int x, int y);

  /** Trata o clique duplo do botao esquerdo. */
  void TrataDuploCliqueEsquerdo(int x, int y);

  /** Trata o duplo clique com botao direito. */
  void TrataDuploCliqueDireito(int x, int y);

  /** Retorna a entidade selecionada, se houver. Se houver mais de uma, retorna nullptr. */
  Entidade* EntidadeSelecionada();

  /** Retorna se uma entidade esta selecionada. */
  bool EntidadeEstaSelecionada(unsigned int id);

  /** seleciona a entidade pelo ID, deselecionando outras e colocando o tabuleiro no estado
  * ETAB_ENT_SELECIONADA.
  */ 
  void SelecionaEntidade(unsigned int id);

  /** Seleciona as entidades passadas por id, deselecionando outras e colocando o tabuleiro
  * no estado ETAB_ENTS_SELECIONADAS.
  */
  void SelecionaEntidades(const std::vector<unsigned int>& ids);

  /** seleciona as entidades em ids_adicionados_. */
  void SelecionaEntidadesAdicionadas() { SelecionaEntidades(ids_adicionados_); }

  /** Adiciona entidades as entidades selecionadas. O estado final depende do tamanho dos ids e do 
  * numero de entidades selecionadas corrente.
  */
  void AdicionaEntidadesSelecionadas(const std::vector<unsigned int>& ids);

  /** Alterna a selecao da entidade. */
  void AlternaSelecaoEntidade(unsigned int id);

  /** deseleciona todas as entidades selecionadas. */
  void DeselecionaEntidades();

  /** Deseleciona a entidade, se estiver selecionada. */
  void DeselecionaEntidade(unsigned int id);

  /** seleciona o quadrado pelo ID. */
  void SelecionaQuadrado(int id_quadrado);

  /** retorna as coordenadas do quadrado. */
  void CoordenadaQuadrado(int id_quadrado, double* x, double* y, double* z);

  /** @return uma notificacao do tipo TN_SERIALIZAR_TABULEIRO preenchida. */
  ntf::Notificacao* SerializaTabuleiro();

  /** @return uma notificacao do tipo TN_ABRIR_DIALOGO_ILUMINACAO_TEXTURA preenchida. */
  ntf::Notificacao* SerializaPropriedades() const;

  /** @return uma notificacao do tipo TN_ABRIR_DIALOGO_OPCOES preenchida. */
  ntf::Notificacao* SerializaOpcoes() const;

  /** Monta o tabuleiro de acordo com a notificacao TN_DESERIALIZAR_TABULEIRO. */
  void DeserializaTabuleiro(const ntf::Notificacao& notificacao);

  /** Deserializa apenas a parte de propriedades. */
  void DeserializaPropriedades(const ent::TabuleiroProto& novo_proto);

  /** Deserializa as opcoes. */
  void DeserializaOpcoes(const ent::OpcoesProto& novo_proto);

  /** Remove uma entidade pelo id.
  * @return true se a entidade removida for a selecionada.
  */
  bool RemoveEntidade(unsigned int id);

  /** @return um id unico de entidade para um cliente. Lanca excecao se nao houver mais id livre. */
  int GeraIdEntidade(int id_cliente);

  /** @return um id unico de cliente. Lanca excecao se chegar ao limite de clientes. */
  int GeraIdCliente();

  /** Libera e carrega texturas de acordo com novo_proto e o estado atual. */
  void AtualizaTexturas(const ent::TabuleiroProto& novo_proto);

  /** Desenha um quadrado do tabuleiro. */
  void DesenhaQuadrado(unsigned int id, int linha, int coluna, bool selecionado, bool usar_textura);

  /** Desenha a grade do tabuleiro. */
  void DesenhaGrade();

  /** Desenha a lista de pontos de vida a direita. */
  void DesenhaListaPontosVida();

  /** Retorna a razao de aspecto do viewport. */
  double Aspecto() const;

  /** Poe o tabuleiro no modo jogador. */
  void ModoJogador() { modo_mestre_ = false; }

 private:
  // Parametros de desenho, importante para operacoes de picking e manter estado durante renderizacao.
  ParametrosDesenho parametros_desenho_;
  // Parametros do tabuleiro (sem entidades).
  TabuleiroProto proto_;
  // Opcoes do usuario.
  OpcoesProto opcoes_;

  /** Cada cliente possui um identificador diferente. */
  int id_cliente_;

  /** mapa geral de entidades, por id. */
  MapaEntidades entidades_;

  /** Acoes de jogadores no tabuleiro. */
  std::vector<std::unique_ptr<Acao>> acoes_;

  /** um set com os id de clientes usados. */
  MapaClientes clientes_;

  /** as entidades selecionada. */
  std::unordered_set<unsigned int> ids_entidades_selecionadas_;

  /** Entidade detalhada: mouse parado sobre ela. */
  unsigned int id_entidade_detalhada_;

  /** quadrado selecionado (pelo id de desenho). */
  int quadrado_selecionado_;

  /** estado do tabuleiro. */
  etab_t estado_;
  /** usado para restaurar o estado apos rotacao. */
  etab_t estado_anterior_rotacao_;

  /** proximo id local de entidades. */
  int proximo_id_entidade_;

  /** proximo id de cliente. */
  int proximo_id_cliente_;

  /** dados (X) para calculo de mouse. */
  int ultimo_x_;
  /** dados (Y) para calculo de mouse. */
  int ultimo_y_;
  /** coordenadas 3d da ultima acao. */
  float ultimo_x_3d_;
  float ultimo_y_3d_;
  float ultimo_z_3d_;
  /** coordenadas 3d do inicio da acao. */
  float primeiro_x_3d_;
  float primeiro_y_3d_;
  float primeiro_z_3d_;

  /** Dimensoes do viewport. */
  int largura_;
  int altura_;

  // Para onde o olho olha.
  Olho olho_;

  /** O modelo selecionado para inserção de entidades. */
  const ent::EntidadeProto* modelo_selecionado_;
  std::unordered_map<std::string, std::unique_ptr<EntidadeProto>> mapa_modelos_;

  /** Ação selecionada (por id). */
  const AcaoProto* acao_selecionada_;
  std::unordered_map<std::string, std::unique_ptr<AcaoProto>> mapa_acoes_;

  Texturas* texturas_;
  ntf::CentralNotificacoes* central_;
  bool modo_mestre_;
  std::list<int> lista_pontos_vida_;  // Usado para as acoes.
  std::vector<EntidadeProto> entidades_copiadas_;

  // Para processamento de grupos de notificacoes. 
  bool processando_grupo_;
  std::vector<unsigned int> ids_adicionados_;

  // elimina copia
  Tabuleiro(const Tabuleiro& t);
  Tabuleiro& operator=(Tabuleiro&);
};

}

#endif
