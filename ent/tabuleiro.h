#ifndef ENT_TABULEIRO_H
#define ENT_TABULEIRO_H

#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <vector>
#include "ent/acoes.pb.h"
#include "ent/entidade.h"
#include "ent/entidade.pb.h"
#include "ent/tabuleiro.pb.h"
#include "ent/watchdog.h"
#include "ntf/notificacao.h"

namespace ntf {
class Notificacao;
}  // namespace ntf

namespace ent {

class Acao;
class Entidade;
class InfoTextura;
class Texturas;
class Watchdog;

/** Estados possiveis do tabuleiro. */
enum etab_t {
  ETAB_OCIOSO,
  ETAB_ROTACAO,
  ETAB_DESLIZANDO,
  ETAB_ENTS_PRESSIONADAS,
  ETAB_ENTS_TRANSLACAO_ROTACAO,
  ETAB_ENTS_ESCALA,
  ETAB_ENTS_SELECIONADAS,
  ETAB_QUAD_PRESSIONADO,
  ETAB_QUAD_SELECIONADO,
  ETAB_SELECIONANDO_ENTIDADES,
  ETAB_DESENHANDO,
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
  /** Watchdog pode ser nullptr. */
  explicit Tabuleiro(const Texturas* texturas, ntf::CentralNotificacoes* central);

  /** libera os recursos do tabuleiro, inclusive entidades. */
  virtual ~Tabuleiro();

  /** @return numero de quadrados no eixo E-W. */
  int TamanhoX() const;

  /** @return numero de quadrados no eixo N-S. */
  int TamanhoY() const;

  /** adiciona a entidade ao tabuleiro, através de uma notificação. Notifica clientes se a notificacao
  * for local.
  * @throw logic_error se o limite de entidades for alcançado.
  */
  void AdicionaEntidadeNotificando(const ntf::Notificacao& notificacao);

  /** Trata a remocao de entidades do tabuleiro, pelo id da entidade passada ou a selecionada se nao houver
  * id de entidade. Se a notificacao for local, envia notificacao aos clientes.
  */
  void RemoveEntidadeNotificando(const ntf::Notificacao& notificacao);
  /** Remove a entidade id e notifica. */
  void RemoveEntidadeNotificando(unsigned int id_remocao);

  /** Remove uma entidade pelo id. Nao notifica.
  * @return true se foi removida com sucesso.
  */
  bool RemoveEntidade(unsigned int id);

  /** Move uma entidade notificando clientes. */
  void MoveEntidadeNotificando(const ntf::Notificacao& notificacao);

  /** Atualiza uma entidade, notificando clientes. */
  void AtualizaEntidadeNotificando(const ntf::Notificacao& notificacao);

  /** Inverte o bit da entidade. */
  enum bit_e {
    BIT_VISIBILIDADE     = 0x1,
    BIT_ILUMINACAO       = 0x2,
    BIT_VOO              = 0x4,
    BIT_MORTA            = 0x8,
    BIT_CAIDA            = 0x10,
  };
  /** Atualiza algum campo booleano da entidade selecionada, invertendo-o.
  * O valor eh uma mascara de OUs de bit_e. Notifica clientes.
  */
  void AtualizaBitsEntidadeNotificando(int bits);

  /** Adiciona delta_pontos_vida aos pontos de vida da entidade selecionada. */
  void TrataAcaoAtualizarPontosVidaEntidades(int delta_pontos_vida);

  /** Atualiza os pontos de vida de uma entidade, notificando clientes. */
  void AtualizaPontosVidaEntidadeNotificando(const ntf::Notificacao& notificacao);

  /** Atualiza os pontos de vida de uma entidade como consequencia de uma acao. Nao preocupa com desfazer, que ja foi feito no inicio da acao. */
  void AtualizaPontosVidaEntidadePorAcao(unsigned int id, int delta_pontos_vida);

  /** Adiciona a lista_pv no final da lista de pontos de vida acumulados. */
  void AcumulaPontosVida(const std::vector<int>& lista_pv);
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
  void TrataMovimentoMouse(int x, int y);

  /** Trata o movimento do mouse apos ficar em repouso. */
  void TrataMovimentoMouse();

  /** trata o botao do mouse liberado. */
  void TrataBotaoLiberado();

  /** trata o clique do botao esquerdo, preparando para movimento de arrastar entidades.
  * Seleciona a entidade o clique for nela. Se alterna_selecao for true, alterna o estado de selecao da entidade.
  */
  void TrataBotaoEsquerdoPressionado(int x, int y, bool alterna_selecao = false);

  /** trata o click do botao direito, preparando para movimento de deslizamento. */
  void TrataBotaoDireitoPressionado(int x, int y);

  /** Trata o clique do botao de rotacao pressionado. */
  void TrataBotaoRotacaoPressionado(int x, int y);

  /** Trata o botao de desenho pressionado. */
  void TrataBotaoDesenhoPressionado(int x, int y);

  /** Ocorre quando se clica com o control em uma entidade. */
  void TrataBotaoAlternarSelecaoEntidadePressionado(int x, int y);

  /** altera o estado da opcao de iluminacao do mestre igual a dos jogadores. */
  void TrataBotaoAlternarIluminacaoMestre();

  /** trata o botao pressionado em modo de acao, recebendo x, y (ja em coordenadas opengl).
  * Se acao_padrao == true, usa a acao de sinalizacao, caso contrario, usa a acao selecionada.
  */
  void TrataBotaoAcaoPressionado(bool acao_padrao, int x, int y);

  /** Trata o clique duplo do botao esquerdo. */
  void TrataDuploCliqueEsquerdo(int x, int y);

  /** Trata o duplo clique com botao direito. */
  void TrataDuploCliqueDireito(int x, int y);

  /** Trata o evento de mouse parado em um determinado local (coordenadas opengl). */
  void TrataMouseParadoEm(int x, int y);

  /** trata a redimensao da janela. */
  void TrataRedimensionaJanela(int largura, int altura);

  enum dir_rolagem_e {
    DIR_LESTE = 0,
    DIR_OESTE = 1,
    DIR_NORTE = 2,
    DIR_SUL = 3,
  };
  /** Move todos os objetos do tabuleiro em uma direcao. */
  void TrataRolagem(dir_rolagem_e dir);

  /** inicializa os parametros do openGL. */
  static void InicializaGL();

  /** Seleciona o modelo de entidade através do identificador. */
  void SelecionaModeloEntidade(const std::string& id_modelo);

  /** Busca um dos modelos pelo id. */
  const EntidadeProto* BuscaModelo(const std::string& id_modelo) const;

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

  /** Seleciona uma das formas de desenho como padrao. */
  void SelecionaFormaDesenho(TipoForma fd);

  /** Retorna a forma de desenho selecionada como padrao. */
  TipoForma FormaDesenhoSelecionada() const { return forma_selecionada_; }

  /** Seleciona a cor do desenho (em RGB). */
  void SelecionaCorDesenho(const Cor& cor) { forma_cor_ = cor; }

  /** Retorna a cor de desenho. */
  const Cor& CorDesenho() const { return forma_cor_; }

  /** @return a entidade por id, ou nullptr se nao encontrá-la. */
  Entidade* BuscaEntidade(unsigned int id);

  /** Copia todas as entidades selecionadas para 'entidades_copiadas_'. */
  void CopiaEntidadesSelecionadas();

  /** Cola as 'entidades_copiadas_', gerando entidades com ids diferentes. */
  void ColaEntidadesSelecionadas();

  /** Agrupa as entidades selecionadas, criado uma so do tipo TE_FORMA, subtipo TF_COMPOSTA. */
  void AgrupaEntidadesSelecionadas();

  /** Movimenta as entidades selecionadas 1 quadrado. O movimento pode ser vertical ou horizontal e o valor
  * deve ser 1 ou -1. A movimentacao sera referente a posicao da camera.
  */
  void TrataMovimentoEntidadesSelecionadas(bool vertical, int valor);

  /** Adiciona a notificacao a lista de eventos que podem ser desfeitos. Caso a lista alcance tamanho
  * maximo, tira a cabeca.
  */
  void AdicionaNotificacaoListaEventos(const ntf::Notificacao& notificacao);

  /** Desfaz a ultima acao local. */
  void TrataComandoDesfazer();

  /** refaz a ultima acao desfeita. */
  void TrataComandoRefazer();

 private:
  /** Poe o tabuleiro nas condicoes iniciais. */
  void EstadoInicial();

  /** Libera a textura do tabuleiro, se houver. */
  void LiberaTextura();

  /** funcao que desenha a cena independente do modo. */
  void DesenhaCena();

  /** Desenha o tabuleiro do sul pro norte. */
  void DesenhaTabuleiro();

  /** funcao para desenhar os rastros de movimento. */
  void DesenhaRastros();

  /** funcao para desenhar a rosa dos ventos. */
  void DesenhaRosaDosVentos();

  /** funcao para desenhar os pontos de rolagem do tabuleiro. */
  void DesenhaPontosRolagem();

  /** Desenha as sombras dos objetos. */
  void DesenhaSombras();

  /** Desenha as entidades. */
  void DesenhaEntidadesBase(const std::function<void (Entidade*, ParametrosDesenho*)>& f);
  void DesenhaEntidades() { DesenhaEntidadesBase(&Entidade::Desenha); }
  void DesenhaEntidadesTranslucidas() { DesenhaEntidadesBase(&Entidade::DesenhaTranslucido); }

  /** Desenha as acoes do tabuleiro (como misseis magicos). */
  void DesenhaAcoes();
  void DesenhaAuras();

  /** Desenha a forma de desenho selecionada. */
  void DesenhaFormaSelecionada();

  /** Atualiza a posição do olho na direção do quadrado selecionado ou da entidade selecionada. Se forcar for false, so atualiza se houver
  * destino. Caso contrario, atualiza independente do destino.*/
  void AtualizaOlho(bool forcar = false);

  /** Atualiza as aentidades do tabuleiro. */
  void AtualizaEntidades();

  /** Atualiza as acoes do tabuleiro, removendo as finalizadas. */
  void AtualizaAcoes();

  /** Refresca o watchdog, se houver. */
  void RefrescaWatchdog();

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
  bool MousePara3d(int x, int y, float* x3d, float* y3d, float* z3d);

  /** Dada uma profundidade, faz a projecao inversa (2d para 3d). */
  bool MousePara3d(int x, int y, float profundidade, float* x3d, float* y3d, float* z3d);

  /** Retorna a entidade selecionada, se houver. Se houver mais de uma, retorna nullptr. */
  Entidade* EntidadeSelecionada();

  /** Retorna se uma entidade esta selecionada. */
  bool EntidadeEstaSelecionada(unsigned int id);

  /** seleciona a entidade pelo ID, deselecionando outras e colocando o tabuleiro no estado
  * ETAB_ENT_SELECIONADA em case de sucesso. Pode falhar se a entidade nao for selecionavel, neste caso
  * o tabuleiro fica ocioso e retorna false.
  */
  bool SelecionaEntidade(unsigned int id);

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

  /** Se a entidade estiver selecionada, verifica se ela pode continuar (por exemplo, apos atualizacao do bit
  * de selecao para jogador.
  */
  void AtualizaSelecaoEntidade(unsigned int id);

  /** Alterna a selecao da entidade. */
  void AlternaSelecaoEntidade(unsigned int id);

  /** deseleciona todas as entidades selecionadas. */
  void DeselecionaEntidades();

  /** Deseleciona a entidade, se estiver selecionada. */
  void DeselecionaEntidade(unsigned int id);

  /** Poe o tabuleiro no estado ETAB_OCIOSO ETAB_ENT_SELECIONADA ou ETAB_ENTS_SELECIONADAS de acordo
  * com o numero de entidades selecionadas.
  */
  void MudaEstadoAposSelecao();

  /** Alguns estados podem ser interrompidos por outros. Esta funcao finaliza o corrente antes de mudar para um novo. */
  void FinalizaEstadoCorrente();

  /** seleciona o quadrado pelo ID. */
  void SelecionaQuadrado(int id_quadrado);

  /** retorna as coordenadas do quadrado. */
  void CoordenadaQuadrado(unsigned int id_quadrado, float* x, float* y, float* z);
#if USAR_OPENGL_ES
  /** Retorna as coordenadas do sub quadrado dentro de um quadrado, para picking de maior resolucao. */
  void CoordenadaQuadradoDetalhado(unsigned int id_quadrado, unsigned int id_detalhado, float* x, float* y, float* z);
  void CoordenadaEntidadeDetalhada(unsigned int id, unsigned int id_detalhado, float* x, float* y, float* z);
#endif

  /** @return uma notificacao do tipo TN_SERIALIZAR_TABULEIRO preenchida. */
  ntf::Notificacao* SerializaTabuleiro();

  /** @return uma notificacao do tipo TN_ABRIR_DIALOGO_ILUMINACAO_TEXTURA preenchida. */
  ntf::Notificacao* SerializaPropriedades() const;

  /** @return uma notificacao do tipo TN_ABRIR_DIALOGO_OPCOES preenchida. */
  ntf::Notificacao* SerializaOpcoes() const;

  /** Monta o tabuleiro de acordo com a notificacao TN_DESERIALIZAR_TABULEIRO.
  * Se usar_id for true, muda o identificador de cliente caso ele seja zero.
  * Se manter_entidades for true, as entidades do tabuleiro corrente serao mantidas e as da notificacao serao ignoradas.
  */
  void DeserializaTabuleiro(const ntf::Notificacao& notificacao);

  /** Deserializa apenas a parte de propriedades. */
  void DeserializaPropriedades(const ent::TabuleiroProto& novo_proto);

  /** Deserializa as opcoes. */
  void DeserializaOpcoes(const ent::OpcoesProto& novo_proto);

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
  void ModoJogador();

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

  /** dados 2d para calculo de mouse. */
  int primeiro_x_;
  int primeiro_y_;
  int ultimo_x_;
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

  const Texturas* texturas_;
  Watchdog watchdog_;
  ntf::CentralNotificacoes* central_;
  bool modo_mestre_;
  std::list<int> lista_pontos_vida_;  // Usado para as acoes.
  std::vector<EntidadeProto> entidades_copiadas_;

  // Para processamento de grupos de notificacoes.
  bool processando_grupo_;
  std::vector<unsigned int> ids_adicionados_;

  // Para rastros de movimentos das unidades.
  std::unordered_map<unsigned int, std::vector<Posicao>> rastros_movimento_;

  // Para decidir entre translacao e rotacao.
  enum {
    TR_NENHUM, TR_TRANSLACAO, TR_ROTACAO
  } translacao_rotacao_;
  // Para desfazer translacao rotacao.
  std::unordered_map<unsigned int, std::pair<float, float>> translacoes_rotacoes_antes_;

  // Para desfazer e refazer. A lista tem tamanho maximo.
  bool ignorar_lista_eventos_;  // Quando verdadeiro, eventos inseridos na lista de eventos serao ignorados.
  std::list<ntf::Notificacao> lista_eventos_;  // Usar sempre as funcoes de evento para acessar.
  std::list<ntf::Notificacao>::iterator evento_corrente_;

  // Desenho corrente.
  TipoForma forma_selecionada_;  // Necessario para poder limpar o proto em paz.
  Cor forma_cor_;  // idem.
  EntidadeProto forma_proto_;

  // elimina copia
  Tabuleiro(const Tabuleiro& t);
  Tabuleiro& operator=(Tabuleiro&);
};

}

#endif
