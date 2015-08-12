#ifndef ENT_TABULEIRO_H
#define ENT_TABULEIRO_H

#include <boost/timer/timer.hpp>
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
#if USAR_WATCHDOG
#include "ent/watchdog.h"
#endif
#include "gltab/gl_vbo.h"
#include "ntf/notificacao.h"
#include "tex/texturas.h"

namespace ntf {
class Notificacao;
}  // namespace ntf

namespace ent {

class Acao;
class Entidade;
class InfoTextura;
class Texturas;
#if USAR_WATCHDOG
class Watchdog;
#endif

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
// Mapa de id_tab-> id net.
typedef std::unordered_map<unsigned int, std::string> MapaClientes;

/** Responsavel pelo mundo do jogo. O sistema de coordenadas tera X apontando para o leste,
* Y para o norte e Z para alto. Cada unidade corresponde a um metro, portanto os quadrados
* sao de tamanho 1,5m.
*/
class Tabuleiro : public ntf::Receptor {
 public:
  explicit Tabuleiro(tex::Texturas* texturas, const m3d::Modelos3d* m3d, ntf::CentralNotificacoes* central);

  /** libera os recursos do tabuleiro, inclusive entidades. */
  virtual ~Tabuleiro();

  /** @return numero de quadrados no eixo E-W. */
  inline int TamanhoX() const { return proto_corrente_->largura(); }

  /** @return numero de quadrados no eixo N-S. */
  inline int TamanhoY() const { return proto_corrente_->altura(); }

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

  /** Seleciona todas as entidades do cenario corrente.
  * @param fixas se as entidades fixas devem ser selecionadas.
  */
  void SelecionaTudo(bool fixas);

  /** Atualiza uma entidade, notificando clientes. */
  void AtualizaEntidadeNotificando(const ntf::Notificacao& notificacao);

  /** Inverte o bit da entidade. */
  enum bit_e {
    BIT_VISIBILIDADE     = 0x1,
    BIT_ILUMINACAO       = 0x2,
    BIT_VOO              = 0x4,
    BIT_MORTA            = 0x8,
    BIT_CAIDA            = 0x10,
    BIT_SELECIONAVEL     = 0x20,
    BIT_FIXA             = 0x40,
  };
  /** Atualiza algum campo booleano da entidade selecionada, invertendo-o.
  * O valor eh uma mascara de OUs de bit_e. Notifica clientes.
  */
  void AlternaBitsEntidadeNotificando(int bits);
  void AtualizaBitsEntidadeNotificando(int bits, bool valor);

  /** Adiciona delta_pontos_vida aos pontos de vida da entidade selecionada. */
  void TrataAcaoAtualizarPontosVidaEntidades(int delta_pontos_vida);

  /** Atualiza os pontos de vida de uma entidade, notificando clientes. */
  void AtualizaPontosVidaEntidadeNotificando(const ntf::Notificacao& notificacao);

  /** Atualiza parcialmente entidade dentro da notificacao. Isso significa que apenas os campos presentes na entidade serao atualizados. */
  void AtualizaParcialEntidadeNotificando(const ntf::Notificacao& notificacao);

  /** Atualiza os pontos de vida de uma entidade como consequencia de uma acao.
  * Nao preocupa com desfazer, que ja foi feito no inicio da acao.
  */
  void AtualizaPontosVidaEntidadePorAcao(const Acao& acao, unsigned int id, int delta_pontos_vida);

  /** Atualiza a proxima salvacao para cada entidade selecionada. */
  void AtualizaSalvacaoEntidadesSelecionadas(ResultadoSalvacao rs);

  /** Adiciona a lista_pv no final da lista de pontos de vida acumulados. */
  void AcumulaPontosVida(const std::vector<int>& lista_pv);
  /** Limpa a lista de pontos de vida. */
  void LimpaListaPontosVida();
  /** Limpa a ultima entrada da lista de pontos de vida. */
  void LimpaUltimoListaPontosVida();
  /** Altera o ultimo valor da lista de pontos de vida. Se nao existir um ultimo valor, cria um novo. */
  void AlteraUltimoPontoVidaListaPontosVida(int delta);
  /** Alterna o ultimo valor da lista entre cura e dano. */
  void AlternaUltimoPontoVidaListaPontosVida();

  /** desenha o mundo. */
  void Desenha();

  /** Interface receptor. */
  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

  /** Trata teclado. */
  void TrataTeclaPressionada(int tecla);

  /** trata evento de escala por delta (rodela). */
  void TrataEscalaPorDelta(int delta);

  /** Trata evento de escala por fator (pinca). Quanto maior o fator, mais proximo o olho ficara do foco. */
  void TrataEscalaPorFator(float fator);

  /** Trata evento de rotacao por delta (pinca). */
  void TrataRotacaoPorDelta(float delta_rad);

  /** Trata evento de inclinacao por delta radianos. */
  void TrataInclinacaoPorDelta(float delta_rad);

  /** Trata um evento de translacao do tabuleiro isoladamente. Parametros x, y sao as coordenadas originais,
  * e nx, ny indicam as coordenadas apos o movimento do cursor.
  */
  void TrataTranslacaoPorDelta(int x, int y, int nx, int ny);

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
  void IniciaGL();

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

  /** Retorna true se o tabuleiro tiver nome e puder ser salvo. */
  bool TemNome() const { return !proto_.nome().empty(); }

  /** Seleciona a acao para as entidades selecionadas através do identificador. */
  void SelecionaAcao(const std::string& id_acao);
  void ProximaAcao();
  void AcaoAnterior();

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

  /** Desagrupa as entidades selecionadas, criando varias de acordo com os subtipos. */
  void DesagrupaEntidadesSelecionadas();

  /** Movimenta as entidades selecionadas 1 quadrado. O movimento pode ser vertical ou horizontal e o valor
  * deve ser 1 ou -1. A movimentacao sera referente a posicao da camera.
  */
  void TrataMovimentoEntidadesSelecionadas(bool vertical, float valor);

  /** Trata o movimento de entidades no eixo Z, notificando clientes. */
  void TrataTranslacaoZEntidadesSelecionadas(float delta);

  /** Adiciona a notificacao a lista de eventos que podem ser desfeitos. Caso a lista alcance tamanho
  * maximo, tira a cabeca.
  */
  void AdicionaNotificacaoListaEventos(const ntf::Notificacao& notificacao);

  /** Desfaz a ultima acao local. */
  void TrataComandoDesfazer();

  /** refaz a ultima acao desfeita. */
  void TrataComandoRefazer();

  /** Altera o desenho entre os modos de debug (para OpenGL ES). */
  void AlternaModoDebug();

  /** No modo acao, cada clique gera uma acao. Usado especialmente no tablet. */
  void AlternaModoAcao();

  /** No modo transicao, cada clique causa uma transicao de cenario. */
  void AlternaModoTransicao();

  /** Retorna se o tabuleiro esta no modo mestre ou jogador. */
  bool ModoMestre() const { return modo_mestre_; }
  // Debug.
  void AlternaModoMestre() { modo_mestre_ = !modo_mestre_; }

  /** Permite ligar/desligar o detalhamento de todas as entidades. */
  void DetalharTodasEntidades(bool detalhar) { detalhar_todas_entidades_ = detalhar; }

  /** Adiciona evento de entidades as entidades selecionadas, para o numero de rodadas especificado. */
  void AdicionaEventoEntidadesSelecionadasNotificando(int rodadas);
  /** O contador de eventos de todas as entidades sera decrementado em 1. Nenhum ficara negativo. */
  void PassaUmaRodadaNotificando();
  /** Zera o contador de rodadas do tabuleiro. */
  void ZeraRodadasNotificando();
  /** Apaga os eventos que estao zerados para a entidade. */
  void ApagaEventosZeradosDeEntidadeNotificando(unsigned int id);

  /** Alterna o modo da camera entre isometrica e perspectiva. */
  void AlternaCameraIsometrica();

  /** Alterna a visao de jogador para o mestre. */
  void AlternaVisaoJogador() { visao_jogador_ = !visao_jogador_; }

  /** Alterna a camera presa a entidade. */
  void AlternaCameraPresa();

  /** Carrega um cenario do tabuleiro. O cenario deve existir.
  * @param id do cenario. Use CENARIO_PRINCIPAL para principal.
  * @param camera a posicao para onde a camera olha (alvo).
  */
  void CarregaSubCenario(int id, const Posicao& camera);

  /** Em algumas ocasioes eh interessante parar o watchdog (dialogos por exemplo). */
  void DesativaWatchdog();

  /** Para reativar o watchdog. */
  void ReativaWatchdog();

 private:
  // Classe para computar o tempo de desenho da cena pelo escopo.
  class TimerEscopo {
   public:
    TimerEscopo(Tabuleiro* tabuleiro, bool valido) : tabuleiro_(tabuleiro), valido_(valido) {
      if (valido_) {
        tabuleiro_->timer_.start();
      }
    }

    ~TimerEscopo() {
      if (valido_) {
        tabuleiro_->DesenhaTempoRenderizacao();
      }
    }

   private:
    Tabuleiro* tabuleiro_;
    bool valido_;
  };

  /** Poe o tabuleiro nas condicoes iniciais. */
  void EstadoInicial();

  /** Libera a textura do tabuleiro, se houver. */
  void LiberaTextura();

  /** funcao que desenha a cena independente do modo. */
  void DesenhaCena();

  /** Desenha as luzes do tabuleiro. */
  void DesenhaLuzes();

  /** Desenha o skybox ao redor da camera. */
  void DesenhaCaixaCeu();

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

  /** Desenha as entidades. O parametro sombra indica que a entidade so sera desenha se estiver fora do fog. */
  void DesenhaEntidadesBase(const std::function<void (Entidade*, ParametrosDesenho*)>& f, bool sombra = false);
  void DesenhaEntidades() { DesenhaEntidadesBase(&Entidade::Desenha); }
  void DesenhaEntidadesTranslucidas() { DesenhaEntidadesBase(&Entidade::DesenhaTranslucido); }

  /** Desenha as acoes do tabuleiro (como misseis magicos). */
  void DesenhaAcoes();
  void DesenhaAuras();

  /** Desenha a forma de desenho selecionada. */
  void DesenhaFormaSelecionada();

  /** Desenha o tempo de renderizacao da cena. */
  void DesenhaTempoRenderizacao();

  /** Atualiza a posição do olho na direção do quadrado selecionado ou da entidade selecionada.
  * Se forcar for false, so atualiza se houver destino. Caso contrario, atualiza independente do destino.
  */
  void AtualizaOlho(bool forcar = false);

  /** Atualiza o raio do olho (distancia horizontal para o ponto de foco), respeitando limites
  * maximos e minimos.
  */
  void AtualizaRaioOlho(float raio);

  /** Atualiza as aentidades do tabuleiro. */
  void AtualizaEntidades();

  /** Atualiza as acoes do tabuleiro, removendo as finalizadas. */
  void AtualizaAcoes();

  /** Similar a TrataBotaoAcaoPressionado, mas pos operacao de picking. */
  void TrataBotaoAcaoPressionadoPosPicking(bool acao_padrao, int x, int y, unsigned int id, unsigned int tipo_objeto, float profundidade);

  /** Trata o botao pressionado em modo de transicao de cenarios, recebendo x e y em coordenadas opengl.
  * O picking ja foi realizado pelo cliente, que devera prover as informacoes de id e tipo de objeto (pos_pilha). */
  void TrataBotaoTransicaoPressionadoPosPicking(int x, int y, unsigned int id, unsigned int tipo_objeto);

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

  /** Dada uma coordenada de mouse (x, y) retorna o valor (x, y, z) do local onde o raio projetado intercepta o tabuleiro.
  * Retorna false caso nao haja.
  */
  bool MousePara3dTabuleiro(int x, int y, float* x3d, float* y3d, float* z3d);

#if !USAR_OPENGL_ES
  /** Dada uma profundidade, faz a projecao inversa (2d para 3d). Bem mais barato que MousePara3d acima. */
  bool MousePara3dComProfundidade(int x, int y, float profundidade, float* x3d, float* y3d, float* z3d);
#else
  /** Dado um objeto retorna as coordenadas x y z. */
  bool MousePara3dComId(int x, int y, unsigned int id, unsigned int pos_pilha, float* x3d, float* y3d, float* z3d);
#endif

  /** Retorna a entidade selecionada, se houver. Se houver mais de uma, retorna nullptr. */
  Entidade* EntidadeSelecionada();

  /** Retorna se uma entidade esta selecionada. */
  bool EntidadeEstaSelecionada(unsigned int id);

  /** seleciona a entidade pelo ID, deselecionando outras e colocando o tabuleiro no estado
  * ETAB_ENT_SELECIONADA em case de sucesso. Pode falhar se a entidade nao for selecionavel, neste caso
  * o tabuleiro fica ocioso e retorna false.
  * Se forcar_fixa for verdadeiro, entidades fixas sao aceitas.
  */
  bool SelecionaEntidade(unsigned int id, bool forcar_fixa = false);

  /** Seleciona as entidades passadas por id, deselecionando outras e colocando o tabuleiro
  * no estado ETAB_ENTS_SELECIONADAS.
  */
  void SelecionaEntidades(const std::vector<unsigned int>& ids);

  /** seleciona as entidades em ids_adicionados_. */
  void SelecionaEntidadesAdicionadas() { SelecionaEntidades(ids_adicionados_); }

  /** Adiciona ids as entidades selecionadas. O estado final depende do tamanho dos ids e do
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

  /** Envia atualizacoes de movimento apos um intervalo de tempo. */
  void RefrescaMovimentosParciais();

  /** seleciona o quadrado pelo ID. */
  void SelecionaQuadrado(int id_quadrado);

  /** retorna as coordenadas do centro do quadrado. */
  void CoordenadaQuadrado(unsigned int id_quadrado, float* x, float* y, float* z);
  /** retorna o id do quadrado em determinada coordenada. */
  unsigned int IdQuadrado(float x, float y);

  /** @return uma notificacao do tipo TN_SERIALIZAR_TABULEIRO preenchida.
  * @param nome um tabuleiro com nome pode ser salvo diretamente, sem dialogo de nome
  **/
  ntf::Notificacao* SerializaTabuleiro(const std::string& nome = "");

  /** @return uma notificacao do tipo TN_SERIALIZAR_ENTIDADES_SELECIONAVEIS preenchida. */
  ntf::Notificacao* SerializaEntidadesSelecionaveis() const;

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

  /** Adiciona as entidades selecionaveis da notificacao ao tabuleiro. */
  void DeserializaEntidadesSelecionaveis(const ntf::Notificacao& notificacao);

  /** Cria um novo sub cenario no tabuleiro. O id deve ser unico caso contrario nao faz nada. */
  void CriaSubCenarioNotificando(const ntf::Notificacao& notificacao);

  /** Remove um sub cenario do tabuleiro. Nao eh possivel remover o cenario principal. */
  void RemoveSubCenarioNotificando(const ntf::Notificacao& notificacao);

  /** @return o proto do sub cenario, ou nullptr se nao houver. */
  TabuleiroProto* BuscaSubCenario(int id_cenario);

  /** @return um id unico de entidade para um cliente. Lanca excecao se nao houver mais id livre. */
  unsigned int GeraIdEntidade(int id_cliente);

  /** @return um id unico de tabuleiro para um cliente. Lanca excecao se chegar ao limite de clientes.
  * Os ids retornados serao de 1 a 15. O id zero eh reservado para o servidor.
  */
  int GeraIdTabuleiro();

  /** Recarrega todas as texturas, incluindo sub cenarios. */
  void AtualizaTexturasIncluindoSubCenarios(const ent::TabuleiroProto& proto_principal);

  /** Libera e carrega texturas de acordo com novo_proto e o estado atual. */
  void AtualizaTexturas(const ent::TabuleiroProto& novo_proto);

  /** Carrega as texturas do controle virtual. */
  void CarregaTexturasControleVirtual();

  /** Libera as texturas do controle virtual. */
  void LiberaTexturasControleVirtual();

  /** Desenha a grade do tabuleiro. */
  void DesenhaGrade();

  /** Desenha a lista de pontos de vida a direita. */
  void DesenhaListaPontosVida();

  /** Desenha o identificador de acao da entidade selecionada. */
  void DesenhaIdAcaoEntidade();

  /** Desenha as coordenadas na tela, abaixo das acoes. */
  void DesenhaCoordenadas();

  /** Desenha o controle virtual. */
  void DesenhaControleVirtual();

  /** Faz o picking do controle virtual, recebendo o id do objeto pressionado. */
  void PickingControleVirtual(bool alterna_selecao, int id);

  /** Retorna a razao de aspecto do viewport. */
  double Aspecto() const;

  /** Poe o tabuleiro no modo mestre se true, modo jogador se false. */
  void AlterarModoMestre(bool modo);

  /** Retorna quais unidades sao afetadas por determinada acao. */
  const std::vector<unsigned int> EntidadesAfetadasPorAcao(const AcaoProto& acao);

  /** Salva a camera inicial. */
  void SalvaCameraInicial();

  /** Reinicia a camera para a posicao especificada no proto_.camera_inicial(). Caso nao haja, usa a posicao inicial. Pode carregar um cenario. */
  void ReiniciaCamera();

  /** Ao limpar o proto, a iluminacao vai a zero. Esta funcao restaura os valores que dao visibilidade ao tabuleiro. */
  void ReiniciaIluminacao(TabuleiroProto* sub_cenario);

  /** Configura a matriz de projecao de acordo com o tipo de camera. */
  void ConfiguraProjecao();
  /** Configura o olho, de acordo com o tipo de camera. */
  void ConfiguraOlhar();

  /** Similar ao modo mestre, mas leva em consideracao se o mestre quer ver como jogador tambem. */
  bool VisaoMestre() const { return modo_mestre_ && !visao_jogador_; }

  /** Regera o Vertex Buffer Object do tabuleiro. Deve ser chamado sempre que houver uma alteracao de tamanho ou textura. */
  void RegeraVboTabuleiro();

  /** Regera o Vbo da caixa do ceu, chamado apenas uma vez ja que o objeto da caixa nao muda (apenas a textura pode mudar). */
  void GeraVboCaixaCeu();

  /** @return true se estiver executando o comando de desfazer/refazer. */
  bool Desfazendo() const { return ignorar_lista_eventos_; }

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
  /** usado para restaurar o estado apos algumas operacoes. */
  etab_t estado_anterior_;

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

  /** Quantos ciclos faltam para atualizar posicoes parciais. Atualiza em zero. -1 desliga. */
  int ciclos_para_atualizar_;

  /** Dimensoes do viewport. */
  int largura_;
  int altura_;

  // Para onde o olho olha.
  Olho olho_;
  bool camera_isometrica_ = false;

  /** O modelo selecionado para inserção de entidades. */
  const ent::EntidadeProto* modelo_selecionado_;
  std::unordered_map<std::string, std::unique_ptr<EntidadeProto>> mapa_modelos_;

  /** Ação selecionada (por id). */
  typedef std::unordered_map<std::string, std::unique_ptr<AcaoProto>> MapaIdAcao;
  MapaIdAcao mapa_acoes_;
  std::vector<std::string> id_acoes_;

  tex::Texturas* texturas_;
  const m3d::Modelos3d* m3d_;

#if USAR_WATCHDOG
  Watchdog watchdog_;
#endif
  ntf::CentralNotificacoes* central_;
  bool modo_mestre_;
  bool visao_jogador_ = false;  // Para o mestre poder ver na visao do jogador.
  bool camera_presa_ = false;
  unsigned int id_camera_presa_ = Entidade::IdInvalido;  // A qual entidade a camera esta presa.
  std::list<int> lista_pontos_vida_;  // Usado para as acoes.

#if !USAR_QT
  std::vector<EntidadeProto> entidades_copiadas_;
#endif

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

  // Armazena os ultimos tempos de renderizacao.
  boost::timer::cpu_timer timer_;
  std::list<uint64_t> tempos_renderizacao_;
  constexpr static unsigned int kMaximoTamTemposRenderizacao = 10;

  // Modo de depuracao do tabuleiro.
  bool modo_debug_ = false;

  // Se verdadeiro, todas entidades serao consideradas detalhadas durante o desenho. */
  bool detalhar_todas_entidades_ = false;

  // Controle virtual.
  // O clique pode ter subtipos. Por exemplo, no MODO_ACAO, todo clique executa uma acao.
  // No MODO_TRANSICAO, o clique executara uma transicao de cenario.
  // No MODO_DESENHO, o clique desenhara.
  enum modo_clique_e {
    MODO_NORMAL,
    MODO_ACAO,      // executa acoes no clique.
    MODO_DESENHO,   // reservado.
    MODO_TRANSICAO, // executa transicao no clique.
  };
  modo_clique_e modo_clique_ = MODO_NORMAL;
  bool modo_acao_cura_ = false;  // Indica se os incrementos de PV do controle vao adicionar ou subtrair valores.
  // Cada botao fica apertado por um numero de frames apos pressionado. Este mapa mantem o contador.
  std::map<int, int> contador_pressao_por_controle_;

  // Renderizacao por VBO.
  unsigned int nome_buffer_ = 0;
  unsigned int nome_buffer_indice_ = 0;
  std::vector<unsigned short> indices_tabuleiro_;
  struct InfoVerticeTabuleiro {
    float x, y;
    float s0, t0;
  };
  std::vector<InfoVerticeTabuleiro> vertices_tabuleiro_;
  unsigned int nome_buffer_grade_ = 0;
  unsigned int nome_buffer_indice_grade_ = 0;
  std::vector<float> vertices_grade_;
  std::vector<unsigned short> indices_grade_;
  // TODO VBO dessas coisas aqui em cima.
  gl::VboGravado vbo_caixa_ceu_;
  gl::VboGravado vbo_cubo_;

  // Sub cenarios. -1 para o principal.
  int cenario_corrente_ = -1;
  TabuleiroProto* proto_corrente_ = &proto_;

  bool gl_iniciado_ = false;

  // elimina copia
  Tabuleiro(const Tabuleiro& t);
  Tabuleiro& operator=(Tabuleiro&);
};

}

#endif
