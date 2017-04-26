#ifndef ENT_TABULEIRO_H
#define ENT_TABULEIRO_H

#include <algorithm>
#include <boost/timer/timer.hpp>
#include <functional>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <vector>
#include "ent/acoes.pb.h"
#include "ent/constantes.h"
#include "ent/controle_virtual.pb.h"
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
class InterfaceGraficaOpengl;

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
  ETAB_RELEVO,
  ETAB_ESCALANDO_ROTACIONANDO_ENTIDADE_PINCA,
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
  explicit Tabuleiro(const OpcoesProto& opcoes,
                     tex::Texturas* texturas, const m3d::Modelos3d* m3d,
                     ntf::CentralNotificacoes* central);

  /** libera os recursos do tabuleiro, inclusive entidades. */
  virtual ~Tabuleiro();

  /** @return numero de quadrados no eixo E-W. */
  inline int TamanhoX() const { return proto_corrente_->largura(); }

  /** @return numero de quadrados no eixo N-S. */
  inline int TamanhoY() const { return proto_corrente_->altura(); }

  /** Uma cor personalizada foi escolhida. */
  void SelecionaCorPersonalizada(float r, float g, float b, float a);

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

  /** Atualiza a lista de iniciativas, caso alguma entidade nova tenha aparecido ou saido da lista. */
  void AtualizaIniciativas();

  /** Inverte o bit da entidade. */
  enum bit_e {
    BIT_VISIBILIDADE         = 0x1,
    BIT_ILUMINACAO           = 0x2,
    BIT_VOO                  = 0x4,
    BIT_MORTA                = 0x8,
    BIT_CAIDA                = 0x10,
    BIT_SELECIONAVEL         = 0x20,
    BIT_FIXA                 = 0x40,
    BIT_FURTIVO              = 0x80,
    BIT_SURPRESO             = 0x100,
    BIT_ATAQUE_MAIS_1        = 0x400,
    BIT_ATAQUE_MAIS_2        = 0x800,
    BIT_ATAQUE_MAIS_4        = 0x1000,
    BIT_ATAQUE_MAIS_8        = 0x2000,
    BIT_ATAQUE_MENOS_1       = 0x4000,
    BIT_ATAQUE_MENOS_2       = 0x8000,
    BIT_ATAQUE_MENOS_4       = 0x10000,
    BIT_ATAQUE_MENOS_8       = 0x20000,
    BIT_DANO_MAIS_1          = 0x40000,
    BIT_DANO_MAIS_2          = 0x80000,
    BIT_DANO_MAIS_4          = 0x100000,
    BIT_DANO_MAIS_8          = 0x200000,
    BIT_DANO_MAIS_16         = 0x400000,
    BIT_DANO_MAIS_32         = 0x800000,
    BIT_DANO_MENOS_1         = 0x1000000,
    BIT_DANO_MENOS_2         = 0x2000000,
    BIT_DANO_MENOS_4         = 0x4000000,
    BIT_DANO_MENOS_8         = 0x8000000,
  };
  /** Atualiza algum campo booleano da entidade selecionada, invertendo-o.
  * O valor eh uma mascara de OUs de bit_e. Notifica clientes.
  */
  void AlternaBitsEntidadeNotificando(int bits);
  /** Alguns bits sao locais. */
  void AtualizaBitsEntidadeNotificando(int bits, bool valor);

  /** Adiciona delta_pontos_vida aos pontos de vida da entidade selecionada. */
  void TrataAcaoAtualizarPontosVidaEntidades(int delta_pontos_vida);

  /** Atualiza os pontos de vida de uma entidade, notificando clientes. */
  void AtualizaPontosVidaEntidadeNotificando(const ntf::Notificacao& notificacao);

  /** Atualiza parcialmente entidade dentro da notificacao. Isso significa que apenas os campos presentes na entidade serao atualizados. */
  void AtualizaParcialEntidadeNotificando(const ntf::Notificacao& notificacao);

  /** Atualiza os pontos de vida de uma entidade como consequencia de uma acao.
  * Nao preocupa com desfazer, que ja foi feito no inicio da acao. Gera as ACAO_DELTA_PONTOS_VIDA.
  */
  void AtualizaPontosVidaEntidadePorAcao(const Acao& acao, unsigned int id, int delta_pontos_vida);

  /** Atualiza a proxima salvacao para cada entidade selecionada. */
  void AtualizaSalvacaoEntidadesSelecionadas(ResultadoSalvacao rs);

  /** Adiciona a lista_pv no final da lista de pontos de vida acumulados. */
  void AcumulaPontosVida(const std::vector<std::pair<int, std::string>>& lista_pv);
  /** Limpa a lista de pontos de vida. */
  void LimpaListaPontosVida();
  /** Limpa a ultima entrada da lista de pontos de vida. */
  void LimpaUltimoListaPontosVida();
  /** Altera o ultimo valor da lista de pontos de vida. Se nao existir um ultimo valor, cria um novo. */
  void AlteraUltimoPontoVidaListaPontosVida(int delta);
  /** Alterna o ultimo valor da lista entre cura e dano. */
  void AlternaUltimoPontoVidaListaPontosVida();
  /** Retorna true se houver valor na lista ou se for automatico e a entidade tiver os dados necessarios. */
  bool HaValorListaPontosVida();
  /** Retorna a frente da lista e a remove. Caso o dano seja automatico, le da entidade para o tipo de acao. */
  int LeValorListaPontosVida(const Entidade* entidade, const std::string& id_acao);
  /** Retorna o valor de ataque furtivo, se estiver ligado e houver. */
  int LeValorAtaqueFurtivo(const Entidade* entidade);

  /** desenha o mundo. Retorna o tempo em ms. */
  int Desenha();

  /** Desenha o mundo do ponto de vista da luz, gerando o framebuffer de sombra projetada. */
  void DesenhaMapaSombra();
  void DesenhaMapaOclusao();
  void DesenhaMapaLuz();

  /** Interface receptor. */
  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

  /** Trata teclado. */
  void TrataTeclaPressionada(int tecla);

  /** trata evento de escala por delta (rodela). */
  void TrataEscalaPorDelta(int delta);

  /** Trata evento de escala por fator (pinca). Quanto maior o fator, mais proximo o olho ficara do foco. */
  void TrataEscalaPorFator(float fator);

  /** Verifica se os dois toques da pinca sao em um objeto, alterando escala por fator e rotacao por delta. */
  void TrataInicioPinca(int x1, int y1, int x2, int y2);

  /** Altera o campo de visao, mantendo-o entre um minimo e maximo. */
  void AlteraAnguloVisao(float valor);

  /** Rola iniciativa das entidades selecionadas mais dos jogadores. */
  void RolaIniciativasNotificando();
  void LimpaIniciativasNotificando();
  void IniciaIniciativaParaCombate();
  void ProximaIniciativa();
  /** Realiza a atualizacao das iniciativas, notificando clientes. */
  void AtualizaIniciativaNotificando(const ntf::Notificacao& notificacao);
  /** Retorna o id da iniciativa corrente, ou IdInvalido. */
  unsigned int IniciativaCorrente() const {
    if (indice_iniciativa_ < 0 || indice_iniciativa_ >= (int)iniciativas_.size()) {
      return Entidade::IdInvalido;
    }
    return iniciativas_[indice_iniciativa_].id;
  }

  /** Trata evento de rotacao por delta (pinca). */
  void TrataRotacaoPorDelta(float delta_rad);

  /** Trata evento de inclinacao por delta radianos. */
  void TrataInclinacaoPorDelta(float delta_rad);

  /** Trata um evento de translacao do tabuleiro isoladamente. Parametros x, y sao as coordenadas originais,
  * e nx, ny indicam as coordenadas apos o movimento do cursor.
  */
  void TrataTranslacaoPorDelta(int x, int y, int nx, int ny);

  /** trata movimento do mouse (y ja em coordenadas opengl).
  * @return true se o mouse deve ser travado (para caso de rotacao).
  */
  bool TrataMovimentoMouse(int x, int y);

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

  /** inicializa os parametros do openGL. Chamado no IOS e ANDROID tambem para recuperar o contexto grafico. */
  void IniciaGL();

  /** Seleciona o modelo de entidade através do identificador. */
  void SelecionaModeloEntidade(const std::string& id_modelo);

  /** Busca um dos modelos pelo id. */
  const EntidadeProto* BuscaModelo(const std::string& id_modelo) const;

  /** Retorna true se o tabuleiro tiver nome e puder ser salvo. */
  bool TemNome() const { return !proto_.nome().empty(); }

  /** Muda para o modo de sinalizacao. */
  void SelecionaSinalizacao();
  /** Seleciona a acao para a entidade. */
  void SelecionaAcao(const std::string& id_acao, Entidade* entidade);
  /** Igual, mas seleciona para as entidades selecionadas ou primeira pessoa. */
  void SelecionaAcao(const std::string& id_acao);
  void ProximaAcao();
  void AcaoAnterior();
  /** Seleciona para a entidade selecionada umas das ultimas acoes executadas. Indice 0 eh a mais recente. */
  void SelecionaAcaoExecutada(int indice);

  /** Alterna o modo de dano automatico. */
  void AlternaDanoAutomatico();

  /** Acesso ao mapa de modelos. */
  typedef std::unordered_map<std::string, std::unique_ptr<AcaoProto>> MapaIdAcao;
  const MapaIdAcao& MapaAcoes() const { return mapa_acoes_; }

  /** Seleciona uma das formas de desenho como padrao. */
  void SelecionaFormaDesenho(TipoForma fd);

  /** Retorna a forma de desenho selecionada como padrao. */
  TipoForma FormaDesenhoSelecionada() const { return forma_selecionada_; }

  /** Seleciona a cor do desenho (em RGB). */
  void SelecionaCorDesenho(const Cor& cor) { forma_cor_ = cor; }

  /** Altera a cor das entidades selecionadas. Nao funciona em formas compostas. */
  void AlteraCorEntidadesSelecionadasNotificando(const Cor& cor);

  /** Altera a textura das entidades selecionadas, notificando. */
  void AlteraTexturaEntidadesSelecionadasNotificando(const std::string& id_textura);

  /** Retorna a cor de desenho. */
  const Cor& CorDesenho() const { return forma_cor_; }

  /** @return a entidade por id, ou nullptr se nao encontrá-la. */
  Entidade* BuscaEntidade(unsigned int id);
  const Entidade* BuscaEntidade(unsigned int id) const;

  /** Copia todas as entidades selecionadas para 'entidades_copiadas_'. */
  void CopiaEntidadesSelecionadas();

  /** Cola as 'entidades_copiadas_', gerando entidades com ids diferentes. Se ref_camera, as entidades serao 
  * coladas referentes a camera.
  */
  void ColaEntidadesSelecionadas(bool ref_camera);

  /** Agrupa as entidades selecionadas, criado uma so do tipo TE_FORMA, subtipo TF_COMPOSTA. */
  void AgrupaEntidadesSelecionadas();

  /** Desagrupa as entidades selecionadas, criando varias de acordo com os subtipos. */
  void DesagrupaEntidadesSelecionadas();

  /** Movimenta as entidades selecionadas em valor * quadrado. O movimento pode ser no eixo frente - atras ou no eixo lateral;
  * o valor deve sera multiplicado pelo tamanho do quadrado. A movimentacao sera referente a posicao da camera.
  */
  void TrataMovimentoEntidadesSelecionadas(bool frente_atras, float valor);

  /** Trata a espiada: movimento de pescoco lateral. */
  void TrataEspiada(int espiada);

  /** Trata o movimento de entidades no eixo Z de acordo com modo, notificando clientes. No modo terreno, trata a translacao do terreno. */
  void TrataTranslacaoZ(float delta);

  // Funcao auxiliar pra realizar algum hack qualquer em entidades selecionadas.
  void Hack();

  /** Adiciona a notificacao a lista de eventos que podem ser desfeitos. Caso a lista alcance tamanho
  * maximo, tira a cabeca.
  */
  void AdicionaNotificacaoListaEventos(const ntf::Notificacao& notificacao);

  /** Adiciona um evento ao log. */
  void AdicionaLogEvento(const std::string& evento);

  /** Desfaz a ultima acao local. */
  void TrataComandoDesfazer();

  /** refaz a ultima acao desfeita. */
  void TrataComandoRefazer();

  /** Altera o desenho entre os modos de debug (para OpenGL ES). */
  void AlternaModoDebug();

  /** No modo acao, cada clique gera uma acao. */
  void AlternaModoAcao();

  /** No modo transicao, cada clique causa uma transicao de cenario. */
  void AlternaModoTransicao();

  /** No modo regua, cada clique mede a distancia para a entidade selecionada. */
  void AlternaModoRegua();

  /** Modo d20, o clique rola um dado. */
  void AlternaModoD20();

  /** No modo terreno, cada clique seleciona um quadrado e a escala altera o relevo. */
  void AlternaModoTerreno();

  // Controle virtual.
  // O clique pode ter subtipos. Por exemplo, no MODO_ACAO, todo clique executa uma acao.
  // No MODO_TRANSICAO, o clique executara uma transicao de cenario.
  // No MODO_DESENHO, o clique desenhara.
  // No MODO_TERRENO, o clique selecionara quadrado mas os drags alterarao a altura dos pontos.
  enum modo_clique_e {
    MODO_NORMAL,
    MODO_ACAO,         // executa acoes no clique.
    MODO_SINALIZACAO,  // executa acao de sinalizacao.
    MODO_DESENHO,      // reservado.
    MODO_TRANSICAO,    // executa transicao no clique.
    MODO_SELECAO_TRANSICAO,    // escolhe o local de transicao durante clique.
    MODO_REGUA,        // o clique executara uma medicao.
    MODO_D20,          // o clique rolara um d20.
    MODO_AJUDA,        // o clique atuara como hover.
    MODO_ROTACAO,      // modo de rotacao da camera.
    MODO_TERRENO,      // modo de edicao de relevo do terreno.
  };
  void EntraModoClique(modo_clique_e modo);
  modo_clique_e ModoClique() const { return modo_clique_; }

  /** Retorna se o tabuleiro esta no modo mestre ou jogador. Parametro secundario para considerar 
  * mestres secundarios tambem.
  */
  bool EmModoMestre() const {
    return modo_mestre_;
  }
  bool EmModoMestreIncluindoSecundario() const {
    return EmModoMestre() || modo_mestre_secundario_;
  }
  // Debug.
  void AlternaModoMestre() { modo_mestre_ = !modo_mestre_; }
  void AlternaModoMestreSecundario() { modo_mestre_secundario_ = !modo_mestre_secundario_; }
  // debug.
  void AlternaListaObjetos() { opcoes_.set_mostra_lista_objetos(!opcoes_.mostra_lista_objetos()); }
  void AlternaListaJogadores() { opcoes_.set_mostra_lista_jogadores(!opcoes_.mostra_lista_jogadores()); }
  void AlternaMostraLogEventos() { opcoes_.set_mostra_log_eventos(!opcoes_.mostra_log_eventos()); }

  /** Permite ligar/desligar o detalhamento de todas as entidades. */
  void DetalharTodasEntidades(bool detalhar) { detalhar_todas_entidades_ = detalhar; }

  /** Adiciona evento de entidades as entidades selecionadas, para o numero de rodadas especificado. */
  void AdicionaEventoEntidadesSelecionadasNotificando(int rodadas);
  /** O contador de eventos de todas as entidades sera decrementado em 1. Nenhum ficara negativo.
  * Caso grupo nao seja null, a notificacao ira para ele e nao sera executada.
  */
  void PassaUmaRodadaNotificando(ntf::Notificacao* grupo = nullptr);
  /** Zera o contador de rodadas do tabuleiro. */
  void ZeraRodadasNotificando();
  /** Apaga os eventos que estao zerados para a entidade. */
  void ApagaEventosZeradosDeEntidadeNotificando(unsigned int id);

  /** Alterna o modo da camera entre isometrica e perspectiva. */
  void AlternaCameraIsometrica();
  /** Alterna entre a camera em primeira pessoa e a normal. */
  void AlternaCameraPrimeiraPessoa();

  /** Alterna entre visao do jogador e do mestre. */
  void AlternaVisaoJogador();

  /** Alterna a camera presa a entidade. */
  void AlternaCameraPresa();
  /** Se houver mais de uma entidade de camera presa, muda para a proxima. */
  void MudaEntidadeCameraPresa();

  /** Alterna a visao no escuro. Ainda depende da entidade selecionada possuir a visao. */
  void AlternaVisaoEscuro() { visao_escuro_ = !visao_escuro_; }

  /** Carrega um cenario do tabuleiro. O cenario deve existir.
  * @param id do cenario. Use CENARIO_PRINCIPAL para principal.
  * @param camera a posicao para onde a camera olha (alvo).
  */
  void CarregaSubCenario(int id, const Posicao& camera);

  /** Retorna o nivel do solo na coordenada ou zero se nao for valida. */
  float ZChao(float x, float y) const;

  /** Em algumas ocasioes eh interessante parar o watchdog (dialogos por exemplo). */
  void DesativaWatchdogSeMestre() { if (EmModoMestre()) DesativaWatchdog(); }

  /** Para reativar o watchdog. */
  void ReativaWatchdogSeMestre() { if (EmModoMestre()) ReativaWatchdog(); }

  // Ativa a interface opengl para dialogos de tipo abrir tabuleiro, janela etc.
  void AtivaInterfaceOpengl(InterfaceGraficaOpengl* gui) { gui_ = gui; }

  const OpcoesProto& Opcoes() const { return opcoes_; }

 private:
  struct DadosIniciativa {
    unsigned int id;
    int iniciativa;
    int modificador;
    bool presente;  // usado durante atualizacao de iniciativa.
  };

  /** Poe o tabuleiro nas condicoes iniciais. A parte grafica sera iniciada de acordo com o parametro. */
  void EstadoInicial(bool reiniciar_grafico);

  /** Libera a textura do tabuleiro, se houver. */
  void LiberaTextura();

  /** funcao que desenha a cena independente do modo. */
  void DesenhaCena();
  // Desenha a cena baseado em VBOs.
  void DesenhaCenaVbos();
  void GeraVbosCena();

  /** Desenha o alvo do olho. */
  void DesenhaOlho();

  /** Desenha as luzes do tabuleiro. */
  void DesenhaLuzes();
  void AtualizaLuzesPontuais();

  /** Desenha o skybox ao redor da camera. */
  void DesenhaCaixaCeu();

  /** Desenha o tabuleiro do sul pro norte. */
  void DesenhaTabuleiro();
  /** Desenha o quadrado selecionado do tabuleiro de forma mais escura, transparente. */
  void DesenhaQuadradoSelecionado();
  /** Desenha a grade do tabuleiro. */
  void DesenhaGrade();

  /** funcao para desenhar os rastros de movimento. */
  void DesenhaRastros();

  /** funcao para desenhar a rosa dos ventos. */
  void DesenhaRosaDosVentos();

  /** funcao para desenhar os pontos de rolagem do tabuleiro. */
  void DesenhaPontosRolagem();

  /** Desenha as entidades. */
  void DesenhaEntidadesBase(const std::function<void (Entidade*, ParametrosDesenho*)>& f);
  void DesenhaEntidades() { DesenhaEntidadesBase(&Entidade::Desenha); }
  void DesenhaEntidadesTranslucidas() { DesenhaEntidadesBase(&Entidade::DesenhaTranslucido); }
  void OrdenaEntidades();

  /** Detecta se havera colisao no movimento da entidade. */
  struct ResultadoColisao {
    float profundidade;  // quanto movimentou ate a colisao, length de movimento se nao houver.
    bool colisao;        // Houve colisao?
    Vector3 normal;      // normal do ponto de colisao.
  };
  ResultadoColisao DetectaColisao(const Entidade& entidade, const Vector3& movimento, bool ignora_espaco_entidade = false);
  ResultadoColisao DetectaColisao(
      float x, float y, float z_olho, float espaco_entidade, const Vector3& movimento, bool ignora_espaco_entidade = false);
  /** Retorna true se a entidade estiver apoiada. */
  bool Apoiado(float x, float y, float z_olho, float altura_olho);
  /** Retorna o nivel de apoio para a entidade. */
  struct ResultadoZApoio {
    bool apoiado;
    float z_apoio;
  };
  ResultadoZApoio ZApoio(float x, float y, float z_olho, float altura_olho);

  // Coleta os VBOs extraidos.
  void ColetaVbosEntidades();

  /** Desenha as acoes do tabuleiro (como misseis magicos). */
  void DesenhaAcoes();
  void DesenhaAcoesTranslucidas();

  void DesenhaAuras();

  /** Desenha a forma de desenho selecionada. */
  void DesenhaFormaSelecionada();

  /** Desenha os tempos de renderizacao, atualizacao, etc. */
  void DesenhaTempos();
  /** Funcao auxiliar usada por DesenhaTempos. */
  void DesenhaTempo(int linha, const std::string& prefixo, const std::list<uint64_t>& ultimos_tempos);

  void DesenhaLogEventos();

  /** Atualiza a posição do olho na direção do quadrado selecionado ou da entidade selecionada.
  * Se forcar for false, so atualiza se houver destino. Caso contrario, atualiza independente do destino.
  */
  void AtualizaOlho(int intervalo_ms, bool forcar = false);

  /** Atualiza o raio do olho (distancia horizontal para o ponto de foco), respeitando limites
  * maximos e minimos.
  */
  void AtualizaRaioOlho(float raio);

  /** Atualiza as aentidades do tabuleiro. */
  void AtualizaEntidades(int intervalo_ms);

  /** Atualiza as acoes do tabuleiro, removendo as finalizadas. */
  void AtualizaAcoes(int intervalo_ms);

  /** Similar a TrataBotaoAcaoPressionado, mas pos operacao de picking. */
  void TrataBotaoAcaoPressionadoPosPicking(bool acao_padrao, int x, int y, unsigned int id, unsigned int tipo_objeto, float profundidade);

  /** Trata o botao pressionado em modo de transicao de cenarios, recebendo x e y em coordenadas opengl.
  * O picking ja foi realizado pelo cliente, que devera prover as informacoes de id e tipo de objeto (pos_pilha). */
  void TrataBotaoTransicaoPressionadoPosPicking(int x, int y, unsigned int id, unsigned int tipo_objeto);

  void TrataBotaoTerrenoPressionadoPosPicking(float x3d, float y3d, float z3d);

  /** Trata o botao pressionado no modo de regua, recebendo o destino do clique em coordenadas de mundo. */
  void TrataBotaoReguaPressionadoPosPicking(float x3d, float y3d, float z3d);

  /** Rola um d20 para as entidades selecionadas e notifica. */
  void TrataBotaoD20PressionadoPosPicking(float x3d, float y3d, float z3d);

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
  bool MousePara3dParaleloZero(int x, int y, float* x3d, float* y3d, float* z3d);

  /** Dada uma profundidade, faz a projecao inversa (2d para 3d). Bem mais barato que MousePara3d acima. */
  bool MousePara3dComProfundidade(int x, int y, float profundidade, float* x3d, float* y3d, float* z3d);

  /** Dado um objeto retorna as coordenadas x y z. */
  bool MousePara3dComId(int x, int y, unsigned int id, unsigned int pos_pilha, float* x3d, float* y3d, float* z3d);

  /** Retorna a entidade de primeira pessoa se a camera for primeira pessoa ou a entidade selecionada, se houver apenas uma.
  * Caso contrario, retorna nullptr.
  */
  Entidade* EntidadePrimeiraPessoaOuSelecionada();
  const Entidade* EntidadePrimeiraPessoaOuSelecionada() const;
  /** Retorna a entidade camera presa ou selecionada, se houver apenas uma. */
  const Entidade* EntidadeCameraPresaOuSelecionada() const;
  /** Retorna o vetor ou com o id da entidade primeira pessoa, ou das entidades selecionadas se nao for primeira pessoa. */
  std::vector<unsigned int> IdsPrimeiraPessoaOuEntidadesSelecionadas() const;
  /** Retorna o vetor com o id da entidade de primeira pessoa mais as selecionadas. */
  std::vector<unsigned int> IdsPrimeiraPessoaIncluindoEntidadesSelecionadas() const;
  /** O contrario, se houver selecao retorna o que esta selecionado. Caso contrario, retorna primeira pessoa (se houver). */
  std::vector<unsigned int> IdsEntidadesSelecionadasOuPrimeiraPessoa() const;
  /** Retorna a entidade selecionada se houver apenas uma, ou a primeira pessoa. */
  Entidade* EntidadeSelecionadaOuPrimeiraPessoa();
  const Entidade* EntidadeSelecionadaOuPrimeiraPessoa() const;
  /** Retorna a entidade selecionada, se houver. Se houver mais de uma, retorna nullptr. */
  Entidade* EntidadeSelecionada();
  const Entidade* EntidadeSelecionada() const;
  /** Retorna as entidades selecionadas ou vazio se nao houver. */
  std::vector<const Entidade*> EntidadesSelecionadas() const;

  /** Retorna se uma entidade esta selecionada. */
  bool EntidadeEstaSelecionada(unsigned int id);

  /** seleciona a entidade pelo ID, deselecionando outras e colocando o tabuleiro no estado
  * ETAB_ENT_SELECIONADA em caso de sucesso. Pode falhar se a entidade nao for selecionavel, neste caso
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

  /** Envia atualizacoes de movimento para clientes. */
  void RefrescaMovimentosParciais();

  /** Envia atualizacoes de terreno para clientes. */
  void RefrescaTerrenoParaClientes();

  // O id eh sequencial, comecando em SW (0) indo para leste. As linhas sobem para o norte.
  /** seleciona o quadrado pelo ID. */
  void SelecionaQuadrado(int id_quadrado);
  /** retorna as coordenadas do centro do quadrado. */
  void CoordenadaQuadrado(unsigned int id_quadrado, float* x, float* y, float* z);
  /** Retorna o x3d e y3d do SW do quadrado. */
  void CoordenadaSwQuadrado(unsigned int id_quadrado, float* x, float* y, float* z = nullptr);
  void CoordenadaSwQuadrado(int x_quad, int y_quad, float* x, float* y, float* z = nullptr);
  /** Retorna o x e y do quadrado. O quadrado SW eh (0,0), a sua direita (1,0), acima (0,1) e por ai vai. */ 
  void XYQuadrado(unsigned int id_quadrado, int *x, int* y);
  /** retorna o id do quadrado em determinada coordenada ou -1 se for posicao invalida. */
  unsigned int IdQuadrado(float x, float y);

  /** @return uma notificacao do tipo TN_SERIALIZAR_TABULEIRO preenchida.
  * @param nome um tabuleiro com nome pode ser salvo diretamente, sem dialogo de nome
  **/
  ntf::Notificacao* SerializaTabuleiro(const std::string& nome = "");

  /** @return uma notificacao do tipo TN_SERIALIZAR_ENTIDADES_SELECIONAVEIS preenchida. */
  ntf::Notificacao* SerializaEntidadesSelecionaveis() const;

  /** @return uma notificacao do tipo TN_ABRIR_DIALOGO_ILUMINACAO_TEXTURA preenchida. */
  ntf::Notificacao* SerializaPropriedades() const;

  /** @return uma notificacao do tipo TN_ATUALIZAR_RELEVO_TABULEIRO preenchida. */
  ntf::Notificacao* SerializaRelevoCenario() const;

  /** @return uma notificacao do tipo TN_ABRIR_DIALOGO_OPCOES preenchida. */
  ntf::Notificacao* SerializaOpcoes() const;

  /** Monta o tabuleiro de acordo com a notificacao TN_DESERIALIZAR_TABULEIRO.
  * Se usar_id for true, muda o identificador de cliente caso ele seja zero.
  * Se manter_entidades for true, as entidades do tabuleiro corrente serao mantidas e as da notificacao serao ignoradas.
  */
  void DeserializaTabuleiro(const ntf::Notificacao& notificacao);

  /** Deserializa apenas a parte de propriedades. */
  void DeserializaPropriedades(const ent::TabuleiroProto& novo_proto);

  /** Deserializa o relevo de um cenario. */
  void DeserializaRelevoCenario(const ent::TabuleiroProto& novo_proto);

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

  /** Carrega o controle virtual. */
  void CarregaControleVirtual();
  /** Chamado quando contexto opengl eh perdido. */
  void IniciaGlControleVirtual();

  /** Libera o controle virtual. */
  void LiberaControleVirtual();

  /** Desenha a lista de pontos de vida a direita. */
  void DesenhaListaPontosVida();

  /** Desenha lista de jogadores. */
  void DesenhaListaJogadores();

  /** Para debugar, desenha uma lista de objetos. */
  void DesenhaListaObjetos();

  /** Desenha as iniciativas ordenadas. */
  void DesenhaIniciativas();

  // Auxiliares de DesenhaInfoGeral.
  void DesenhaIdAcaoEntidade();
  void DesenhaCoordenadas();
  /** Desenha informacoes gerais: zoom, id acao, coordenadas etc. */
  void DesenhaInfoGeral();

  /** Desenha o controle virtual. */
  void DesenhaControleVirtual();
  /** Dentro do controle virtual, informacoes de primeira pessoa ou camera presa. */
  void DesenhaInfoCameraPresa();

  /** Faz o picking do controle virtual, recebendo o id do objeto pressionado. */
  void PickingControleVirtual(int x, int y, bool alterna_selecao, bool duplo, int id);

  /** Retorna true se o botao estiver pressionado. O segundo argumento eh um mapa que retorna a funcao de estado de cada botao,
  * para botoes com estado. */
  bool AtualizaBotaoControleVirtual(
      DadosBotao* db,
      const std::unordered_map<int, std::function<bool(const Entidade*)>>& mapa_botoes, const Entidade* entidade);
  /** Alguns botoes ficam invisiveis em algumas situacoes, por exemplo, ataque automatico. */
  bool BotaoVisivel(const DadosBotao& db) const;
  /** Caso o botao tenha um estado associado (como uma variavel booleana), retorna. Caso contrario, retorna false. */
  bool EstadoBotao(IdBotao id_botao) const;

  /** Retorna o rotulo de um botao do controle virtual. */
  std::string RotuloBotaoControleVirtual(const DadosBotao& db) const;

  void DesenhaBotaoControleVirtual(
      const DadosBotao& db, const GLint* viewport, float padding, float largura_botao, float altura_botao, const Entidade* entidade);
  void DesenhaRotuloBotaoControleVirtual(
      const DadosBotao& db, const GLint* viewport, float fonte_x, float fonte_y, float padding,
      float largura_botao, float altura_botao, const Entidade* entidade);
  void DesenhaDicaBotaoControleVirtual(
      const DadosBotao& db, const GLint* viewport, float fonte_x, float fonte_y, float padding, float largura_botao, float altura_botao);
  /** Retorna a textura correspondente a um botao (para botoes com texturas variaveis). */
  unsigned int TexturaBotao(const DadosBotao& db, const Entidade* entidade) const;

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
  /** Igual a ReiniciaCamera(), mas para a posicao da notificacao ao inves de camera_inicial. */
  void ReiniciaCamera(const ntf::Notificacao& n);

  /** Ao limpar o proto, a iluminacao vai a zero. Esta funcao restaura os valores que dao visibilidade ao tabuleiro. */
  void ReiniciaIluminacao(TabuleiroProto* sub_cenario);

  /** Configura a matriz de projecao de acordo com o tipo de camera. */
  void ConfiguraProjecao();
  void ConfiguraProjecaoMapeamentoSombras();
  void ConfiguraProjecaoMapeamentoOclusaoLuzes();
  /** Configura o olho, de acordo com o tipo de camera. */
  void ConfiguraOlhar();
  void ConfiguraOlharMapeamentoSombras();
  void ConfiguraOlharMapeamentoOclusao();
  void ConfiguraOlharMapeamentoLuzes();

  /** Similar ao modo mestre, mas leva em consideracao se o mestre quer ver como jogador tambem. */
  bool VisaoMestre() const { return EmModoMestreIncluindoSecundario() && visao_jogador_ == 0; }
  /** Apenas a iluminacao do modo mestre. */
  bool IluminacaoMestre() const { return EmModoMestreIncluindoSecundario() && (visao_jogador_ == 0 || visao_jogador_ == 2); }

  /** Regera o Vertex Buffer Object do tabuleiro. Deve ser chamado sempre que houver uma alteracao de tamanho ou textura. */
  void RegeraVboTabuleiro();

  /** Ggera o Vbo da caixa do ceu, chamado apenas uma vez ja que o objeto da caixa nao muda (apenas a textura pode mudar). */
  void GeraVboCaixaCeu();

  /** Gera o vbo da rosa dos ventos, chamado apenas uma vez. */
  void GeraVboRosaDosVentos();

  /** Gera e configura o framebuffer. */
  void GeraFramebuffer();

  /** Gera um terreno com relevo aleatorio, respeitando os limites correntes. */
  void GeraTerrenoAleatorioNotificando();
  void GeraMontanhaNotificando();
  void TrataDeltaTerreno(float delta);
  void TrataNivelamentoTerreno(int x, int y);

  /** @return true se estiver executando o comando de desfazer/refazer. */
  bool Desfazendo() const { return ignorar_lista_eventos_; }

  /** Retorna a altura de um ponto de quadrado do tabuleiro (SW) ou zero se invalido. */
  float AlturaPonto(int x_quad, int y_quad) const;

  /** Retorna a acao padrao especificada ou proto vazio se nao houver indice. */
  const AcaoProto& AcaoPadrao(int indice) const;
  const std::vector<std::string>& AcoesPadroes() const;
  /** Retorna a acao ou vazio se nao houver indice. */
  const AcaoProto& AcaoDoMapa(const std::string& id_acao) const;

  bool MapeamentoOclusao() const { return opcoes_.mapeamento_oclusao() && camera_presa_ && camera_ != CAMERA_PRIMEIRA_PESSOA; }
  bool MapeamentoSombras() const { return opcoes_.mapeamento_sombras(); }
  bool MapeamentoLuzes() const { return opcoes_.mapeamento_luzes(); }

  void EscreveInfoGeral(const std::string& info_geral);

  void SerializaIniciativas(TabuleiroProto* tabuleiro) const;
  void SerializaIniciativaParaEntidade(const DadosIniciativa& di, EntidadeProto* e) const;

  unsigned int IdCameraPresa() const { return ids_camera_presa_.empty() ? Entidade::IdInvalido : ids_camera_presa_.front(); }
  bool IdPresoACamera(unsigned int id) const {
    return std::find(ids_camera_presa_.begin(), ids_camera_presa_.end(), id) != ids_camera_presa_.end();
  }

  struct DadosFramebuffer {
    ~DadosFramebuffer();
    GLuint framebuffer = 0;
    GLuint textura = 0;
    GLuint renderbuffer = 0;
  };

  // Gera um framebuffer.
  void GeraFramebufferLocal(int tamanho, bool textura_cubo, bool* usar_sampler_sombras, DadosFramebuffer* dfb);
  void GeraFramebufferColisao(int tamanho, DadosFramebuffer* dfb);

  void DesativaWatchdog();
  void ReativaWatchdog();

 private:
  // Parametros de desenho, importante para operacoes de picking e manter estado durante renderizacao.
  mutable ParametrosDesenho parametros_desenho_;
  // Parametros do tabuleiro (sem entidades).
  TabuleiroProto proto_;
  // Opcoes do usuario.
  OpcoesProto opcoes_;
  int pagina_lista_objetos_ = 0;

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
  /** No caso do duplo clique, a gente perde a selecao no primeiro clique. Essa variavel eh uma tentativa de reverter isso. */
  unsigned int ultima_entidade_selecionada_ = Entidade::IdInvalido;

  /** Entidade detalhada: mouse parado sobre ela. */
  unsigned int id_entidade_detalhada_;
  unsigned int tipo_entidade_detalhada_;
  // TODO sera que da pra usar ciclos_para_atualizar aqui?
  int temporizador_detalhamento_ms_;

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
  float angulo_visao_vertical_graus_ = CAMPO_VISAO_PADRAO;
  enum camera_e {
    CAMERA_PERSPECTIVA,
    CAMERA_ISOMETRICA,
    CAMERA_PRIMEIRA_PESSOA
  };
  camera_e camera_ = CAMERA_PERSPECTIVA;

  /** O modelo selecionado para inserção de entidades. */
  std::pair<std::string, const Modelo*> modelo_selecionado_com_parametros_;
  std::unordered_map<std::string, std::unique_ptr<EntidadeProto>> mapa_modelos_;
  std::unordered_map<std::string, std::unique_ptr<Modelo>> mapa_modelos_com_parametros_;

  /** Ação selecionada (por id). */
  MapaIdAcao mapa_acoes_;
  std::vector<std::string> id_acoes_;

  tex::Texturas* texturas_;
  const m3d::Modelos3d* m3d_;

#if USAR_WATCHDOG
  Watchdog watchdog_;
#endif
  // Interface Grafica OpenGL.
  InterfaceGraficaOpengl* gui_ = nullptr;

  ntf::CentralNotificacoes* central_ = nullptr;
  bool modo_mestre_;
  // Mestre secundario funciona tipo mestre: pode ver, mexer pecas etc. Mas alguns controles sao
  // exclusivos do mestre, como quem sera mestre secundario ou a replicacao de algumas mensagens.
  bool modo_mestre_secundario_ = false;
  std::set<int> mestres_secundarios_;
  // Para o mestre poder ver na visao do jogador. 1 igual jogador, 2 igual jogador mas com iluminacao do mestre.
  int visao_jogador_ = 0;
  bool camera_presa_ = false;
  bool visao_escuro_ = false;  // Jogador ligou a visao no escuro (mas depende da entidade presa possuir).
  // Usado para acoes. Cada entrada indica se eh dano ou cura, e a string pode ter valor de dado (tipo 1d8+3).
  // O primeiro valor eh 1 para positivo, -1 para negativo.
  std::list<std::pair<int, std::string>> lista_pontos_vida_;
  // unsigned int id_camera_presa_ = Entidade::IdInvalido;
  // Lista de ids de camera presa. O corrente sempre é o front.
  std::list<unsigned int> ids_camera_presa_;  // A quais entidade a camera esta presa.

  std::list<std::string> log_eventos_;

#if !USAR_QT
  std::vector<EntidadeProto> entidades_copiadas_;
#endif

  // Para processamento de grupos de notificacoes.
  bool processando_grupo_;
  std::vector<unsigned int> ids_adicionados_;

  // Para rastros de movimentos das unidades.
  std::unordered_map<unsigned int, std::vector<Posicao>> rastros_movimento_;
  int quadrados_movimentados_ = 0;

  // Para decidir entre translacao e rotacao.
  enum {
    TR_NENHUM, TR_TRANSLACAO, TR_ROTACAO
  } translacao_rotacao_;
  // Para desfazer translacao rotacao escalas.
  std::unordered_map<unsigned int, EntidadeProto> translacoes_rotacoes_escalas_antes_;

  // Usada para notificacoes de desfazer que comecam em um estado e terminam em outro.
  ntf::Notificacao notificacao_desfazer_;

  // Para desfazer e refazer. A lista tem tamanho maximo.
  bool ignorar_lista_eventos_;  // Quando verdadeiro, eventos inseridos na lista de eventos serao ignorados.
  std::list<ntf::Notificacao> lista_eventos_;  // Usar sempre as funcoes de evento para acessar.
  std::list<ntf::Notificacao>::iterator evento_corrente_;

  // Desenho corrente.
  TipoForma forma_selecionada_;  // Necessario para poder limpar o proto em paz.
  Cor forma_cor_;  // idem.
  Cor cor_personalizada_;  // Usada pelo controle virtual.
  EntidadeProto forma_proto_;

  // Timers.
  /** Cada vez que desenha cena eh chamado, este timer computa o tempo entre as chamadas. */
  boost::timer::cpu_timer timer_entre_cenas_;
  /** Cada vez que o temporizador eh chamado, este timer computa o tempo entre as chamadas. Eh importante para o tempo real,
  * pois computa o delta tempo a ser passado para as atualizacoes. */
  boost::timer::cpu_timer timer_entre_atualizacoes_;
  /** computa o tempo de renderizacao, debug apenas. */
  boost::timer::cpu_timer timer_uma_renderizacao_completa_;
  /** computa o tempo de uma atualizacao, debug apenas. */
  boost::timer::cpu_timer timer_uma_atualizacao_;
  /** computa tempo de desenho do controle virtual. */
  boost::timer::cpu_timer timer_uma_renderizacao_controle_virtual_;
  /** computa o tempo para renderizar os mapas de luz e oclusao. */
  boost::timer::cpu_timer timer_renderizacao_mapas_;
  
  // Listas que armazenam os ultimos tempos computados pelos timers.
  std::list<uint64_t> tempos_entre_cenas_;    // timer_entre_cenas_
  std::list<uint64_t> tempos_uma_renderizacao_completa_;  // timer_uma_renderizacao_completa_
  std::list<uint64_t> tempos_renderizacao_mapas_;   // timer_renderizacao_mapas_.
  std::list<uint64_t> tempos_uma_atualizacao_;   // timer_uma_atualizacao_
  std::list<uint64_t> tempos_uma_renderizacao_controle_virtual_;   // timer_uma_atualizacao_controle_virtual_.
  std::list<uint64_t> tempos_atualiza_parcial_;

  // Modo de depuracao do tabuleiro.
  bool modo_debug_ = false;

  // Se verdadeiro, todas entidades serao consideradas detalhadas durante o desenho. */
  bool detalhar_todas_entidades_ = false;

  modo_clique_e modo_clique_ = MODO_NORMAL;
  // Cada botao fica apertado por um numero de frames apos pressionado. Este mapa mantem o contador.
  std::map<IdBotao, int> contador_pressao_por_controle_;

  // Variaveis de estado de alguns botoes.
  bool modo_dano_automatico_ = false;
  bool bonus_dano_negativo_ = false;
  bool bonus_ataque_negativo_ = false;

  gl::VbosGravados vbos_tabuleiro_;
  gl::VbosGravados vbos_grade_;
  gl::VboGravado vbo_caixa_ceu_;
  gl::VboGravado vbo_cubo_;
  gl::VboGravado vbo_rosa_;
  DadosFramebuffer dfb_luz_direcional_;
  DadosFramebuffer dfb_oclusao_;
  DadosFramebuffer dfb_colisao_;
  std::vector<DadosFramebuffer> dfb_luzes_;

#if 0
  GLuint framebuffer_ = 0;
  GLuint textura_framebuffer_ = 0;
  GLuint renderbuffer_framebuffer_ = 0;
  GLuint framebuffer_oclusao_ = 0;
  GLuint textura_framebuffer_oclusao_ =  0;
  GLuint renderbuffer_framebuffer_oclusao_ = 0;
  GLuint framebuffer_colisao_ = 0;
  GLuint textura_framebuffer_colisao_ = 0;
  GLuint renderbuffer_framebuffer_colisao_ = 0;
#endif
  constexpr static int TAM_BUFFER_COLISAO = 4;  // 4x4

  // Vbos gerados por renderizacao de cena.
  std::vector<const gl::VbosGravados*> vbos_selecionaveis_cena_;
  std::vector<const gl::VbosGravados*> vbos_nao_selecionaveis_cena_;
  std::vector<const gl::VbosGravados*> vbos_acoes_cena_;

  bool usar_sampler_sombras_ = true;

  // Sub cenarios. -1 para o principal.
  int cenario_corrente_ = CENARIO_PRINCIPAL;
  TabuleiroProto* proto_corrente_ = &proto_;

  // Controle virtual.
  ControleVirtualProto controle_virtual_;
  std::map<IdBotao, const DadosBotao*> mapa_botoes_controle_virtual_;
  std::set<std::string> texturas_entidades_;
  std::set<std::string> modelos_entidades_;
  // Qual iniciativa eh a corrente. -1 para nenhuma.
  int indice_iniciativa_;
  // Iniciativas ordenadas.
  std::vector<DadosIniciativa> iniciativas_;

  // String de informacao geral para display. Normalmente temporizada.
  // Nao escrever diretamente aqui. Ver funcao EscreveInfoGeral.
  std::string info_geral_;
  int temporizador_info_geral_ms_ = 0;

  std::vector<Entidade*> entidades_ordenadas_;

  ntf::Notificacao notificacao_selecao_transicao_;

  // Posicao das luzes, para mapeamento de luzes. Apenas a primeira é usada por enquanto.
  struct LuzPontual {
    unsigned int id;
    Posicao pos;
  };
  std::vector<LuzPontual> luzes_pontuais_;

  // Usado para recuperacao de contexto IOS e android.
  bool regerar_vbos_entidades_ = false;

  // elimina copia
  Tabuleiro(const Tabuleiro& t);
  Tabuleiro& operator=(Tabuleiro&);
};

}

#endif
