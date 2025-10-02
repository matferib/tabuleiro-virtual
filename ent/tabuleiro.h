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
  Tabuleiro(const OpcoesProto& opcoes,
            const Tabelas& tabelas,
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

  /** adiciona entidades ao tabuleiro, através de uma notificação. Notifica clientes se a notificacao
  * for local.
  * @throw logic_error se o limite de entidades for alcançado.
  */
  void AdicionaEntidadesNotificando(const ntf::Notificacao& notificacao);

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

  /** Faz a entidade beber a pocao, recebendo seus efeitos. Indice da pocao indica qual pocao a entidade esta bebendo (na ordem do tesouro). */
  void BebePocaoNotificando(unsigned int id_entidade, int indice_pocao, unsigned int indice_efeito = 0);
  /** Faz a entidade usar o pergaminho, consumindo-o como se fosse uma magia. */
  void UsaPergaminhoNotificando(unsigned int id_entidade, TipoMagia tipo_pergaminho, int indice_pergaminho);

  /** Seleciona todas as entidades do cenario corrente.
  * @param fixas se as entidades fixas devem ser selecionadas.
  */
  void SelecionaTudo(bool fixas);

  /** Atualiza tudo dependente de timer apos o intervalo. */
  void AtualizaPorTemporizacao();

  /** Atualiza uma entidade, notificando clientes. */
  void AtualizaEntidadeNotificando(const ntf::Notificacao& notificacao);

  /** Atualiza a lista de iniciativas, caso alguma entidade nova tenha aparecido ou saido da lista.
  * Caso a iniciativa corrente seja removida, fara iniciativa_valida_ = false.
  */
  void AtualizaIniciativas();

  /** Trata a notificacao de atualizar iniciativas e notifica remotos se for local. */
  void TrataAtualizarIniciativaNotificando(const ntf::Notificacao& notificacao);

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
    BIT_ATAQUE_MAIS_16       = 0x4000,
    BIT_ATAQUE_MENOS_1       = 0x8000,
    BIT_ATAQUE_MENOS_2       = 0x10000,
    BIT_ATAQUE_MENOS_4       = 0x20000,
    BIT_ATAQUE_MENOS_8       = 0x40000,
    BIT_DANO_MAIS_1          = 0x80000,
    BIT_DANO_MAIS_2          = 0x100000,
    BIT_DANO_MAIS_4          = 0x200000,
    BIT_DANO_MAIS_8          = 0x400000,
    BIT_DANO_MAIS_16         = 0x800000,
    BIT_DANO_MAIS_32         = 0x1000000,
    BIT_DANO_MENOS_1         = 0x2000000,
    BIT_DANO_MENOS_2         = 0x4000000,
    BIT_DANO_MENOS_4         = 0x8000000,
    BIT_DANO_MENOS_8         = 0x10000000,
    BIT_FALHA_20             = 0x20000000,
    BIT_FALHA_50             = 0x40000000,
    BIT_FALHA_NEGATIVO       = 0x80000000,
  };
  /** Atualiza algum campo booleano da entidade selecionada, invertendo-o.
  * O valor eh uma mascara de OUs de bit_e. Notifica clientes.
  */
  void AlternaBitsEntidadeNotificando(int bits);
  /** Alguns bits sao locais. */
  void AtualizaBitsEntidadeNotificando(int bits, bool valor);

  /** Alterna o bit de flanquando para as entidades selecionadas. */
  void AlternaFlanqueandoEntidadesSelecionadasNotificando();

  /** Alterna estado em corpo a corpo para as entidades selecionadas. */
  void AlternaEmCorpoACorpoNotificando();

  /** Remove os efeitos de invisibilidade das entidades selecionadas. */
  void RemoveEfeitoInvisibilidadeEntidadesNotificando();

  /** Liga/desliga evento de investida do personagem. */
  void AlternaInvestida();

  /* Se estiver montado, desmonta. Caso contrario, entra no modo de clicar para montar. */
  void AlternaMontar();

  /** Alterna todos os modelos desligaveis de uma entidade (por exemplo, vulto na luz). */
  void AlternaModelosDesligaveisNotificando();

  /** Ao inves de notificar, apenas preenche grupo. */
  void PreencheAtualizacaoBitsEntidade(const Entidade& entidade, int bits, bool valor, ntf::Notificacao* grupo);

  /** Desagarra as entidades selecionadas. */
  void DesagarraEntidadesSelecionadasNotificando();

  /** Altera a forma da entidade selecionada, notificando. */
  void AlteraFormaEntidadeNotificando();

  /** Adiciona delta_pontos_vida aos pontos de vida da entidade selecionada. */
  void TrataAcaoAtualizarPontosVidaEntidades(int delta_pontos_vida);

  /** Atualiza os pontos de vida de uma entidade, notificando clientes. */
  void AtualizaPontosVidaEntidadeNotificando(const ntf::Notificacao& notificacao);

  /** Atualiza parcialmente entidade dentro da notificacao. Isso significa que apenas os campos presentes na entidade serao atualizados. */
  void AtualizaParcialEntidadeNotificando(const ntf::Notificacao& notificacao);

  /** Gera acao filha de acao para a entidade. Se afeta pontos de vida, ira causar dano de verdade.
  * Gera tambem localmente as acoes de texto.
  * Nao preocupa com desfazer, que ja foi feito no inicio da acao. Gera as ACAO_DELTA_PONTOS_VIDA.
  * Retorna o atraso atualizado.
  */
  float GeraAcaoFilha(const Acao& acao, const AcaoProto::PorEntidade& por_entidade, float atraso_s);

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
  /** Retorna a frente da lista e a remove. Caso o dano seja automatico, le da entidade para o tipo de acao.
  * Os dois valores se referem ao dano normal e o dano adicional.
  */
  std::pair<int, int> LeValorListaPontosVida(
      const Entidade* entidade, const EntidadeProto& alvo, const std::string& id_acao);
  /** Retorna o valor de ataque furtivo, se estiver ligado e houver. */
  int LeValorAtaqueFurtivo(const Entidade* entidade);

  /** desenha o mundo. Retorna o tempo em ms. */
  int Desenha();

  /** Desenha o mundo do ponto de vista da luz, gerando o framebuffer de sombra projetada. */
  void DesenhaMapaSombraLuzDirecional();
  void DesenhaMapaOclusao();
  void DesenhaMapaLuz(unsigned int indice_luz);
  void DesenhaFramebufferPrincipal();

  /** Desenha um screenshot na tela e nada mais. Retorna o tempo em ms. */
  int DesenhaModoMostrarImagem();

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
  void ProximaIniciativaModoMestre();
  /** Realiza a atualizacao das iniciativas, notificando clientes. */
  void AtualizaIniciativaNotificando(const ntf::Notificacao& notificacao);
  /** Retorna o id da iniciativa corrente, ou IdInvalido. */
  unsigned int IdIniciativaCorrente() const;

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
  * Se forca_selecao for true, ira selecionar até as entidades fixas.
  */
  void TrataBotaoEsquerdoPressionado(int x, int y, bool alterna_selecao = false, bool forca_selecao = false);

  /** trata o click do botao direito, preparando para movimento de deslizamento. */
  void TrataBotaoDireitoPressionado(int x, int y);

  /** Trata o clique do botao de rotacao pressionado. */
  void TrataBotaoRotacaoPressionado(int x, int y);

  /** Trata o botao de desenho pressionado. */
  void TrataBotaoDesenhoPressionado(int x, int y);

  /** Ocorre quando se clica com o control em uma entidade. */
  void TrataBotaoAlternarSelecaoEntidadePressionado(int x, int y, bool forca_selecao = false);

  /** altera o estado da opcao de iluminacao do mestre igual a dos jogadores. */
  void TrataBotaoAlternarIluminacaoMestre();

  /** trata o botao pressionado em modo de acao, recebendo x, y (ja em coordenadas opengl).
  * Se acao_padrao == true, usa a acao de sinalizacao, caso contrario, usa a acao selecionada.
  * Se forcar for true, ira fazer um click forçado (tipo selecionar ate entidades fixas.
  */
  void TrataBotaoAcaoPressionado(bool acao_padrao, int x, int y);

  /** Retorna id da entidade clicado (ou IdInvalido), a posicao do clique na entidade e a posicao do clique no tabuleiro. */
  std::tuple<unsigned int, Posicao, Posicao> IdPosicaoEntidadePosicaoTabuleiro(int x, int y, unsigned int id, unsigned int tipo_objeto, float profundidade);

  /** Trata o botao de esquiva. */
  void TrataBotaoEsquivaPressionadoPosPicking(unsigned int id, unsigned int tipo_objeto);

  /** Trata o botao do modo de pericia pressionado. */
  void TrataBotaoPericiaPressionadoPosPicking(unsigned int id, unsigned int tipo_objeto);

  /** Alterna a furia da entidade selecionada. */
  void AlternaFuria();

  /** Altera a defesa total para a entidade selecionada. */
  void AlternaDefesaTotal();
  /** Alterna a luta defensiva para a entidade selecionada. */
  void AlternaLutaDefensiva();
  /** Altera ataque poderoso para a entidade selecionada. */
  void AlternaAtaquePoderoso();

  /** Trata o clique duplo do botao esquerdo. */
  void TrataDuploCliqueEsquerdo(int x, int y, bool forcar = false);

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
  void IniciaGL(bool reinicio = false);

  /** Uma entrada pode conter varios modelos, por exemplo feiticeiro com familiar, entidade montada. */
  struct IdsModelosComPeso {
    IdsModelosComPeso(const std::string& id, int peso = 1, const std::string& quantidade = "1") : id_tudo(id), ids{id}, peso(peso), quantidade(quantidade) {}
    IdsModelosComPeso(const std::string& id_tudo, const std::vector<std::string>& ids, int peso = 1, const std::string& quantidade = "1")
        : id_tudo(id_tudo), ids(ids), peso(peso), quantidade(quantidade) {}
    std::string id_tudo;
    std::vector<std::string> ids;
    // Isso vale para a entrada inteira. Por exemplo, se ha 2 ids e quantidade for "4", gerara 8 entidades.
    int peso = 1;
    std::string quantidade = "1";
  };
  /** Pode representar uma entrada simples ou uma complexa, aleatoria, composta por varias outras. */
  struct ItemSelecionado {
    std::string id;
    std::vector<IdsModelosComPeso> ids_com_peso;
    std::string quantidade;
    bool aleatorio = false;
    void Reset();
  };
  /** Seleciona o modelo de entidade através do identificador. */
  void SelecionaModelosEntidades(const std::string& id_item_selecionado);

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

  /** Acesso ao mapa de modelos. O inteiro eh um TipoAcao. */
  typedef std::unordered_map<int, std::unique_ptr<AcaoProto>> MapaTipoAcao;
  typedef std::unordered_map<std::string, std::unique_ptr<AcaoProto>> MapaIdAcao;
  const MapaTipoAcao& MapaTipoAcoes() const { return mapa_acoes_por_tipo_; }

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
  inline const MapaEntidades& TodasEntidades() const { return entidades_; }

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
  void TrataMovimentoEntidadesSelecionadasOuCamera(bool frente_atras, float valor);

  /** Trata a espiada: movimento de pescoco lateral. */
  void TrataEspiada(int espiada);

  /** Trata o movimento de entidades no eixo Z de acordo com modo, notificando clientes. No modo terreno, trata a translacao do terreno. */
  void TrataTranslacaoZ(float delta);

  /** Rola a pericia do proto e mostra notifica clientes. Retorna o total e o modificador para desempate, ou nullptr caso nao tenha rolado. */
  std::optional<std::pair<int, int>> TrataRolarPericiaNotificando(const std::string& pericia, bool local_apenas, float atraso_s, const Bonus& outros_bonus, const EntidadeProto& proto);
  /** Para resistir arte da fuga. */
  void TrataRolarAgarrarNotificando(float atraso_s, const Bonus& outros_bonus, const Entidade& entidade);
  /** Para resistir intimidação. Retorna o total e o modificador para desempate. */
  void TrataRolarContraIntimidacaoNotificando(float atraso_s, const std::pair<int, int>& total_modificadores, const Entidade& entidade_origem, const Entidade& entidde_destino);

  // Funcao auxiliar pra realizar algum hack qualquer em entidades selecionadas.
  void Hack();

  /** Adiciona a notificacao a lista de eventos que podem ser desfeitos. Caso a lista alcance tamanho
  * maximo, tira a cabeca.
  */
  void AdicionaNotificacaoListaEventos(const ntf::Notificacao& notificacao);

  /** Adiciona um evento ao log. */
  void AdicionaLogEvento(const std::string& evento);
  /** Formata a entidade e concatena com texto antes de mandar pro log de eventos. */
  void AdicionaLogEvento(unsigned int id, const std::string& texto);
  /** Retorna o log de eventos. */
  const std::list<std::string>& LogEventos() const { return log_eventos_; }
  /** Incrementa para o proximo cliente e retorna tudo como log. */
  const std::unordered_map<std::string, std::string>& LogEventosClientes() const { return log_eventos_clientes_; } 

  /** Desfaz a ultima acao local. */
  void TrataComandoDesfazer();

  /** refaz a ultima acao desfeita. */
  void TrataComandoRefazer();

  /** Altera o desenho entre os modos de debug (para OpenGL ES). */
  void AlternaModoDebug();

  /** Entra no modo clique de pericia com as informações passadas. */
  void EntraModoPericia(const std::string& id_pericia, const ntf::Notificacao& notificacao);

  /** No modo acao, cada clique gera uma acao. */
  void AlternaModoAcao();

  /** No modo transicao, cada clique causa uma transicao de cenario. */
  void AlternaModoTransicao();

  /** No modo regua, cada clique mede a distancia para a entidade selecionada. */
  void AlternaModoRegua();

  /** Modo dado, o clique rola um dado. */
  void AlternaModoDado(int faces);

  /** No modo terreno, cada clique seleciona um quadrado e a escala altera o relevo. */
  void AlternaModoTerreno();

  /** No modo esquiva, o clique seleciona contra quem a entidade se esquivara. */
  void AlternaModoEsquiva();

  /** No modo remocao de grupo, o clique remove uma entidade de uma entidade
   * composta. */
  void AlternaModoRemocaoDeGrupo();

  /** Alterna para o modo minecraft, onde cada clique adiciona um cubo alinhado a grade. */
  void AlternaModoMinecraft();

  /** Alterna para o modo screenshot, mostrando uma imagem. O clique do mestre sai do modo e volta para o anterior. */
  void EntraModoMostrarImagem(const ntf::Notificacao& notificacao);

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
    MODO_ROLA_DADO,         // o clique rolara um dado.
    MODO_AJUDA,        // o clique atuara como hover.
    MODO_ROTACAO,      // modo de rotacao da camera.
    MODO_TERRENO,      // modo de edicao de relevo do terreno.
    MODO_ESQUIVA,      // Usado para escolher a entidade de esquiva.
    MODO_REMOCAO_DE_GRUPO,  // usado para remover entidades de grupos.
    MODO_MONTAR,            // usado para montar entidades em outras.
    MODO_DOACAO,            // usado para doar itens de um personagem para outro.
    MODO_AGUARDANDO,        // Quando entra nesse modo, os cliques ficam invalidos. So sai quando receber MODO_SAIR_AGUARDANDO.
    MODO_SAIR_AGUARDANDO,   // vide acima.
    MODO_PERICIA,           // O clique rolará a perícia do personagem.
    MODO_ADICAO_ENTIDADE,   // O clique adicionara as entidades escolhidas ao redor do ponto 3d do clique.
    MODO_MINECRAFT,         // O clique adicionara um cubo de 1 quadrado alinhado ao grid.
    MODO_MOSTRAR_IMAGEM,    // O clique saírá do modo.
  };
  void EntraModoClique(modo_clique_e modo);
  modo_clique_e ModoClique() const { return modo_clique_; }

  /** @return true se houver personagens selecionaveis. */
  bool HaEntidadesSelecionaveis() const;

  /** Retorna se o tabuleiro esta no modo mestre ou jogador. Parametro secundario para considerar
  * mestres secundarios tambem.
  */
  bool EmModoMestre() const {
    return modo_mestre_;
  }
  bool EmModoMestreIncluindoSecundario() const {
    return EmModoMestre() || modo_mestre_secundario_;
  }

  bool EmModoMostrarImagem() const {
    return modo_clique_ == MODO_MOSTRAR_IMAGEM;
  }

  // Debug.
  void AlternaModoMestre() { modo_mestre_ = !modo_mestre_; }
  void AlternaModoMestreSecundario() { modo_mestre_secundario_ = !modo_mestre_secundario_; }
  // debug.
  void AlternaListaObjetos() { opcoes_.set_mostra_lista_objetos(!opcoes_.mostra_lista_objetos()); SalvaOpcoes(); }
  void AlternaListaJogadores() { opcoes_.set_mostra_lista_jogadores(!opcoes_.mostra_lista_jogadores()); SalvaOpcoes(); }
  void AlternaMostraLogEventos() { opcoes_.set_mostra_log_eventos(!opcoes_.mostra_log_eventos()); SalvaOpcoes(); }

  /** Permite ligar/desligar o detalhamento de todas as entidades. */
  void DetalharTodasEntidades(bool detalhar) { detalhar_todas_entidades_ = detalhar; }

  /** O contador de eventos de todas as entidades sera decrementado em 1. Nenhum
   * ficara negativo.
   * Caso o parametro expira_eventos_zerados seja verdadeiro, eventos que estejam em zero serão removidos.
   */
  void PreenchePassaUmaRodada(bool passar_para_todos, ntf::Notificacao* grupo, ntf::Notificacao* grupo_desfazer, bool expira_eventos_zerados = false);
  /** Zera o contador de rodadas do tabuleiro. */
  void ZeraRodadasNotificando();
  /** Apaga os eventos que estao zerados para a entidade. */
  void ApagaEventosZeradosDeEntidadeNotificando(unsigned int id);

  /** Alterna o modo da camera entre isometrica e perspectiva. */
  void AlternaCameraIsometrica();
  /** Alterna entre a camera em primeira pessoa e a normal. */
  void AlternaCameraPrimeiraPessoa();
  /** Retorna a entidade de primeira pessoa ou null se nao tiver. */
  const Entidade* EntidadePrimeiraPessoa() const;

  /** Retorna a largura e a altura do viewport do tabuleiro. */
  std::pair<int, int> LarguraAlturaViewport() const {
    return std::make_pair(largura_, altura_);
  }

  /** Alterna entre visao do jogador e do mestre. */
  void AlternaVisaoJogador();

  /** Alterna entre camera presa a entidade e nao presa. */
  void AlternaCameraPresa();
  /** Se houver mais de uma entidade de camera presa, muda para a proxima. */
  void MudaEntidadeCameraPresa();
  /** Muda para uma entidade especifica da camera presa. */
  void MudaEntidadeCameraPresa(unsigned int id);

  /** Alterna a visao no escuro. Ainda depende da entidade selecionada possuir a visao. */
  void AlternaVisaoEscuro() { visao_escuro_ = !visao_escuro_; }

  /** Carrega um cenario do tabuleiro. O cenario deve existir.
  * @param id do cenario. Use CENARIO_PRINCIPAL para principal.
  * @param camera a posicao para onde a camera olha (alvo).
  */
  void CarregaSubCenario(int id, const Posicao& camera);

  int IdCenario() const { return proto_corrente_->id_cenario(); }

  /** Retorna o nivel do solo na coordenada ou zero se nao for valida. */
  float ZChao(float x, float y) const;

  /** Em algumas ocasioes eh interessante parar o watchdog (dialogos por exemplo). */
  void DesativaWatchdogSeMestre() { if (EmModoMestre()) DesativaWatchdog(); }

  /** Para reativar o watchdog. */
  void ReativaWatchdogSeMestre() { if (EmModoMestre()) ReativaWatchdog(); }

  // Ativa a interface opengl para dialogos de tipo abrir tabuleiro, janela etc.
  void AtivaInterfaceOpengl(InterfaceGraficaOpengl* gui) { gui_ = gui; }

  const OpcoesProto& Opcoes() const { return opcoes_; }

  /** Retorna o proto para acessos mais complexos. */
  const TabuleiroProto& Proto() const { return proto_; }

  /** @return o proto do sub cenario, ou nullptr se nao houver. Versao const. */
  const TabuleiroProto* BuscaSubCenario(int id_cenario) const;

  /** Remove as versoes passadas. Nao salva, nem nada. */
  void RemoveVersoes(const std::vector<int>& versao);

  /** Trata a acao de uma entidade especifica apos o picking. Retorna o atraso atualizado. */
  float TrataAcaoUmaEntidade(
      Entidade* entidade, const Posicao& pos_entidade, const Posicao& pos_tabuleiro,
      unsigned int id_entidade_destino, float atraso_s, const AcaoProto* acao_preenchida = nullptr);

  /** Restaura o contexto grafico. Deve ser chamado apenas por quem tem o contexto. */
  void ResetGrafico();

  void RemoverEntidadesDeTimeNotificando();
  void AdicionarEntidadesAoTimeNotificando();

 private:
  struct DadosIniciativa {
    unsigned int id;  // entidade.
    int id_unico_evento = -1;  // para eventos.
    int iniciativa;
    int modificador;
    bool presente;  // usado durante atualizacao de iniciativa.
  };

  /** Quando entidades sao adicionadas, isso pode ocorrer de formas diferentes:
  * - Varias entidades como notificacoes separadas de TN_ADICIONAR_ENTIDADE (colar, deserializar): neste caso, cada entidade vira com entidade preenchida na notificacao.
  * - Uma ou mais entidades em uma notificacao de TN_ADICIONAR_ENTIDADE (clique duplo): neste caso, a entidade vem vazia. Apesar de que por hora, nao ha relacao entre elas,
  * é provavel que no futuro isto mude (entidade com familiar, por exemplo, que teria um campo de familiar).
  * Ao ser adicionada, os ids que ligam as entidades tem que ser corrigidos para os ids que realmente foram usados.
  * Por exemplo, se a entidade 0 estava montada em 3 originalmente e ao deserializa-la elas viram 5 e 6, deve-se corrigir tanto o montada em de 5 quanto o entidades_montadas de 6.
  */
  ntf::Notificacao ArrumaIdsEntidadesAdicionadas() const;

  /** Descansa o personagem: cura 1 PV por nivel e restaura feiticos. */
  void DescansaPersonagemNotificando();

  /** Alterna o modo de ataque de derrubar da entidade selecionada. */
  void AlternaAtaqueDerrubar();
  /** Alterna o modo de ataque de desarmar da entidade selecionada. */
  void AlternaAtaqueDesarmar();

  /** Desliga a esquiva da primeira pessoa ou selecionado, notificando clientes. */
  void DesligaEsquivaNotificando();

  /** Botao de usar feitico clicado. */
  void TrataBotaoUsarFeitico(bool conversao_espontanea, int nivel);
  /** Botao de alterar a classe de feitico ativa clicado. */
  void TrataMudarClasseFeiticoAtiva();

  /** Adiciona uma acao de texto na entidade. */
  void AdicionaAcaoTexto(unsigned int id, const std::string& texto, float atraso_s = 0.0f, bool local_apenas = false);
  void AdicionaAcaoTextoComDuracaoAtraso(unsigned int id, const std::string& texto, float duracao_s, float atraso_s, bool local_apenas = false);
  // Junta AdicionaAcaoTexto e AdicionaLogEvento.
  void AdicionaAcaoTextoLogado(unsigned int id, const std::string& texto, float atraso_s = 0.0f, bool local_apenas = false);
  void AdicionaAcaoTextoLogadoComDuracaoAtraso(
      unsigned int id, const std::string& texto, float duracao_s, float atraso_s, bool local_apenas = false);
  /** Adiciona uma acao de delta pontos de vida sem afetar o destino (display apenas). */
  void AdicionaAcaoDeltaPontosVidaSemAfetar(unsigned int id, int delta, float atraso_s = 0.0f, bool local_apenas = false);
  void AdicionaAcaoDeltaPontosVidaSemAfetarComTexto(unsigned int id, int delta, const std::string& texto, float atraso_s = 0.0f, bool local_apenas = false);
  /** Adiciona uma entidade ao tabuleiro, de acordo com a notificacao e os modelos, notificando. */
  void AdicionaUmaEntidadeNotificando(
      std::unique_ptr<Entidade> entidade, const ntf::Notificacao& notificacao, ntf::Notificacao* n_desfazer);
  std::unique_ptr<Entidade> CriaUmaEntidadePorNotificacao(
      const ntf::Notificacao& notificacao, const Entidade* referencia, const Modelo& modelo_com_parametros,
      float x, float y, float z);

  /** Poe o tabuleiro nas condicoes iniciais. */
  void EstadoInicial();
  /** Libera a textura do tabuleiro, se houver. */
  void LiberaTextura();

  /** Salva o proto de opcoes em disco. */
  void SalvaOpcoes() const;

  /** funcao que desenha a cena independente do modo.
  * Parametro debug apenas para debugar e diferenciar as diferentes chamadas da funcao.
  */
  void DesenhaCena(bool debug = false);
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

  /** Desenha os elos das entidades agarradas. */
  void DesenhaElosAgarrar();

  /** Desenha as entidades. */
  void DesenhaEntidadesBase(const std::function<void (Entidade*, ParametrosDesenho*)>& f);
  void DesenhaEntidades() { DesenhaEntidadesBase(&Entidade::Desenha); }
  void DesenhaEntidadesTranslucidas() { DesenhaEntidadesBase(&Entidade::DesenhaTranslucido); }
  /** Retorna a posicao de referencia, id do cenario e a funcao de ordenacao. */
  std::pair<int, std::function<bool(const Entidade* lhs, const Entidade* rhs)>>
      IdCenarioComFuncaoOrdenacao(const ParametrosDesenho& pd) const;
  /** Ordena as entidades de acordo com os parametros de desenho. Preenche entidades_ordenadas_. */
  void OrdenaEntidades(const ParametrosDesenho& pd);

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
  void TrataAcaoSinalizacao(unsigned int id_entidade_destino, const Posicao& pos_tabuleiro);

  /** Move a camera na direcao passada. o valor deve sera multiplicado pelo tamanho do quadrado. */
  void TrataMovimentoCamera(bool frente_atras, float valor);

  /** Tudo que for comum as ações antes de sua execução deve ser tratado aqui. */
  float TrataPreAcaoComum(
      float atraso_s, const Posicao& pos_entidade, const Posicao& pos_tabuleiro, const Entidade& entidade_origem, unsigned int id_entidade_destino,
      AcaoProto* acao_proto, ntf::Notificacao* grupo_desfazer);
  float TrataAcaoEfeitoArea(
      unsigned int id_entidade_destino, float atraso_s, const Posicao& pos_entidade_destino, Entidade* entidade, AcaoProto* acao_proto,
      ntf::Notificacao* n, ntf::Notificacao* grupo_desfazer);
  float TrataAcaoExpulsarFascinarMortosVivos(
      float atraso_s, const Entidade* entidade, AcaoProto* acao_proto,
      ntf::Notificacao* n, ntf::Notificacao* grupo_desfazer);
  float TrataAcaoIndividual(
      unsigned int id_entidade_destino, float atraso_s, const Posicao& pos_entidade, Entidade* entidade, AcaoProto* acao_proto,
      ntf::Notificacao* n, ntf::Notificacao* grupo_desfazer);
  float TrataAcaoProjetilArea(
      unsigned int id_entidade_destino, float atraso_s, const Posicao& pos_entidade, Entidade* entidade, AcaoProto* acao_proto,
      ntf::Notificacao* n, ntf::Notificacao* grupo_desfazer);
  float TrataAcaoCriacao(
      float atraso_s, const Posicao& pos_entidade, Entidade* entidade, AcaoProto* acao_proto,
      ntf::Notificacao* n, ntf::Notificacao* grupo_desfazer);

  /** Trata o botao pressionado em modo de transicao de cenarios, recebendo x e y em coordenadas opengl.
  * O picking ja foi realizado pelo cliente, que devera prover as informacoes de id e tipo de objeto (pos_pilha). */
  void TrataBotaoTransicaoPressionadoPosPicking(int x, int y, bool forcar, unsigned int id, unsigned int tipo_objeto);

  void TrataBotaoTerrenoPressionadoPosPicking(float x3d, float y3d, float z3d);

  /** Trata o botao pressionado no modo de regua, recebendo o destino do clique em coordenadas de mundo. */
  void TrataBotaoReguaPressionadoPosPicking(float x3d, float y3d, float z3d);

  /** Rola um dado de faces_dado_ para as entidades selecionadas e notifica. */
  void TrataBotaoRolaDadoPressionadoPosPicking(float x3d, float y3d, float z3d);

  /** Remove um objeto de dentro de um composto. */
  void TrataBotaoRemocaoGrupoPressionadoPosPicking(int x, int y, unsigned int id, unsigned int tipo_objeto);

  /** Cria um cubo alinhado a grade. */
  void TrataBotaoAdicionarBlocoMinecraftPressionadoPosPicking(float x3d, float y3d, float z3d);

  /** Monta em um objeto. */
  void TrataBotaoMontariaPressionadoPosPicking(unsigned int id, unsigned int tipo_objeto);

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

  /** Retorna os ids das entidades selecionadas e montadas nelas. */
  std::vector<unsigned int> IdsEntidadesSelecionadasEMontadas() const;
  /** Retorna os ids das entidades selecionadas e tambem daquelas montadas nelas. */
  std::vector<unsigned int> IdsEntidadesSelecionadasEMontadasOuPrimeiraPessoa() const;
  /** Se estiver em primeira pessoa, retorna o id dela e das montadas, senao das entidades selecionadas e montadas nelas. */
  std::vector<unsigned int> IdsPrimeiraPessoaMontadasOuEntidadesSelecionadasMontadas() const;

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

  /** Muda a selecao para a entidade com a iniciativa. Se for jogador, apenas mudara se a entidade
  * estiver presa ao jogador.
  */
  void SelecionaEntidadeIniciativa();

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
  void AdicionaEntidadesSelecionadas(const std::vector<unsigned int>& ids, bool forca_selecao = false);

  /** Se a entidade estiver selecionada, verifica se ela pode continuar (por exemplo, apos atualizacao do bit
  * de selecao para jogador.
  */
  void AtualizaSelecaoEntidade(unsigned int id);

  /** Alterna a selecao da entidade. */
  void AlternaSelecaoEntidade(unsigned int id, bool forca_selecao = false);

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
  ntf::Notificacao* SerializaTabuleiro(bool salvar_versoes, const std::string& nome = "");

  /** @return uma notificacao do tipo TN_DESERIALIZAR_ENTIDADES_SELECIONAVEIS preenchida.
  * O motivo de ser TN_DESERIALIZAR_ENTIDADES_SELECIONAVEIS eh para os clientes poderem receber
  * a notificacao gerada pela funcao.
  */
  ntf::Notificacao* SerializaEntidadesSelecionaveis() const;
  /** Assim como acima, mas serializa apenas os personagens de camera presa. */
  ntf::Notificacao* SerializaEntidadesSelecionaveisJogador() const;

  /** @return uma notificacao do tipo TN_ABRIR_DIALOGO_ILUMINACAO_TEXTURA preenchida. */
  std::unique_ptr<ntf::Notificacao> SerializaPropriedades() const;

  /** @return uma notificacao do tipo TN_ATUALIZAR_RELEVO_TABULEIRO preenchida. */
  ntf::Notificacao* SerializaRelevoCenario() const;

  /** @return uma notificacao do tipo TN_ABRIR_DIALOGO_OPCOES preenchida. */
  ntf::Notificacao* CriaNotificacaoAbrirOpcoes() const;

  /** Monta o tabuleiro de acordo com a notificacao TN_DESERIALIZAR_TABULEIRO.
  * Se usar_id for true, muda o identificador de cliente caso ele seja zero.
  * Se manter_entidades for true, as entidades do tabuleiro corrente serao mantidas e as da notificacao serao ignoradas.
  */
  void DeserializaTabuleiro(const ntf::Notificacao& notificacao);

  /** Deserializa apenas a parte de propriedades. */
  void DeserializaPropriedades(const ent::TabuleiroProto& novo_proto);

  /** Deserializa o relevo de um cenario. */
  void DeserializaRelevoCenario(const ent::TabuleiroProto& novo_proto);

  /** Copia as opcoes de novo_proto e serializa as configuracoes, salvando-as. */
  void AtualizaSerializaOpcoes(const ent::OpcoesProto& novo_proto);

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

  /** Recarrega todas as texturas, incluindo sub cenarios. Usado ao abrir um tabuleiro. */
  void AtualizaPisoCeuIncluindoSubCenarios(const ent::TabuleiroProto& proto_principal);

  // Retorna o cenario que contem as informacoes de piso para o sub cenario.
  const TabuleiroProto& CenarioPiso(const TabuleiroProto& sub_cenario) const;

  // Retorna o cenario que contem as informacoes de ceu para o sub cenario.
  const TabuleiroProto& CenarioCeu(const TabuleiroProto& sub_cenario) const;

  // Retorna o cenario que contem as informacoes de iluminacao direcional e ambiente para o sub cenario.
  const TabuleiroProto& CenarioIluminacao(const TabuleiroProto& sub_cenario) const;

  // Retorna o cenario que contem as informacoes de nevoa para o sub cenario.
  const TabuleiroProto& CenarioNevoa(const TabuleiroProto& sub_cenario) const;

  // Retorna se a nevoa sera usada na renderizacao.
  bool UsaNevoa() const;

  /** Libera e carrega texturas de acordo com novo_proto e o estado atual. */
  void AtualizaPisoCeuCenario(const ent::TabuleiroProto& novo_proto);

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

  /** Desenha uma lista generica de strings de forma paginada. A funcao f_id recebe um indice da lista e retorna o id
  * de callback para controle virtual.
  * Os valores coluna e linha sao usados para posicionar o raster inicialmente. A origem eh esquerda embaixo.
  */
  void DesenhaListaGenerica(int coluna, int linha, int pagina_corrente, int pagina_corrente_horizontal, const char* titulo, const float* cor_titulo,
      int nome_cima, int nome_baixo, int nome_esquerda, int nome_direita, int tipo_lista,
      const std::vector<std::string>& lista, const float* cor_lista, const float* cor_lista_fundo,
      std::function<int(int)> f_id);

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
  std::string RotuloBotaoControleVirtual(const DadosBotao& db, const Entidade* entidade) const;

  void DesenhaBotaoControleVirtual(
      const DadosBotao& db, const GLint* viewport, float padding, float largura_botao, float altura_botao, const Entidade* entidade);
  void DesenhaRotuloBotaoControleVirtual(
      const DadosBotao& db, const GLint* viewport, float fonte_x, float fonte_y, float padding,
      float largura_botao, float altura_botao, const Entidade* entidade);
  void DesenhaDicaBotaoControleVirtual(
      const DadosBotao& db, const GLint* viewport, float fonte_x, float fonte_y, float padding, float largura_botao, float altura_botao,
      const Entidade* entidade);
  /** Retorna a textura correspondente a um botao (para botoes com texturas variaveis). */
  unsigned int TexturaBotao(const DadosBotao& db, const Entidade* entidade) const;
  /** Funcao para retornar o id de botao que melhor representa o estado do clique com a forma selecionada. */
  IdBotao ModoCliqueParaId(Tabuleiro::modo_clique_e mc, TipoForma tf) const;


  /** Retorna a razao de aspecto do viewport. */
  double Aspecto() const;

  /** Poe o tabuleiro no modo mestre se true, modo jogador se false. */
  void AlterarModoMestre(bool modo);

  enum aliado_e {
    TAL_DESCONHECIDO = 0,
    TAL_ALIADO = 1,
    TAL_INIMIGO = 2,
  };
  /** Retorna quais unidades sao afetadas por determinada acao e se são aliadas. */
  const std::vector<std::pair<unsigned int, aliado_e>> EntidadesAfetadasPorAcao(const AcaoProto& acao) const;
  /** Caso a acao deva ser preenchida, preenche e retorna a notificacao. Caso contrario, retorna a notificacao sem tipo. */
  std::unique_ptr<ntf::Notificacao> TalvezPreenchaAcaoNaoPreenchida(
      const Entidade& entidade_origem, const Entidade& entidade_destino,
      const AcaoProto& acao_proto, const Posicao& pos_tabuleiro, const Posicao& pos_entidade_destino) const;


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
  void ConfiguraOlharMapeamentoSombrasLuzDirecional();
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
  void GeraFramebuffer(bool reinicio);

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
  const AcaoProto& AcaoDoMapa(TipoAcao id_acao) const;

  bool MapeamentoOclusao() const { return opcoes_.mapeamento_oclusao() && camera_presa_ && camera_ != CAMERA_PRIMEIRA_PESSOA; }
  bool MapeamentoSombras() const { return opcoes_.mapeamento_sombras(); }
  bool MapeamentoLuzes() const { return opcoes_.mapeamento_luzes(); }

  void EscreveInfoGeral(const std::string& info_geral);

  void DeserializaIniciativas(const TabuleiroProto& tabuleiro);
  void SerializaIniciativas(TabuleiroProto* tabuleiro) const;
  void SerializaIniciativaParaEntidade(const DadosIniciativa& di, EntidadeProto* e) const;

  unsigned int IdCameraPresa() const { return ids_camera_presa_.empty() ? Entidade::IdInvalido : ids_camera_presa_.front(); }
  bool IdPresoACamera(unsigned int id) const {
    return std::find(ids_camera_presa_.begin(), ids_camera_presa_.end(), id) != ids_camera_presa_.end();
  }

  // Atualiza os eventos da entidade ao passar rodadas. As mensagens serao adicionadas ao grupo.
  void AtualizaEventosAoPassarRodada(const Entidade& entidade, std::vector<int>* ids_unicos, ntf::Notificacao* grupo, ntf::Notificacao* grupo_desfazer, bool expira_eventos_zerados);
  // Atualiza as resistencias da entidade ao passar rodada (zera contadores). As mensagens serao adicionadas ao grupo.
  void AtualizaEsquivaAoPassarRodada(const Entidade& entidade, ntf::Notificacao* grupo, ntf::Notificacao* grupo_desfazer);
  void AtualizaMovimentoAoPassarRodada(const Entidade& entidade, ntf::Notificacao* grupo, ntf::Notificacao* grupo_desfazer);
  void AtualizaCuraAceleradaAoPassarRodada(const Entidade& entidade, ntf::Notificacao* grupo, ntf::Notificacao* grupo_desfazer);
  void ConsomeAtaquesLivresRodada(const Entidade& entidade, ntf::Notificacao* grupo, ntf::Notificacao* grupo_desfazer);
  void ReiniciaAtaqueAoPassarRodada(const Entidade& entidade, ntf::Notificacao* grupo, ntf::Notificacao* grupo_desfazer);
  // Chamado ao atacar um alvo, possivelmente alterando a esquiva.
  void AtualizaEsquivaAoAtacar(const Entidade& entidade_origem, unsigned int id_destino, ntf::Notificacao* grupo_desfazer);

  struct DadosFramebuffer {
    ~DadosFramebuffer();
    void Apaga();
    GLuint framebuffer = 0;
    GLuint textura = 0;
    GLuint renderbuffer = 0;
  };

  // Gera um framebuffer.
  void GeraFramebufferLocal(int tamanho, bool textura_cubo, bool* usar_sampler_sombras, DadosFramebuffer* dfb);
  void GeraFramebufferColisao(int tamanho, DadosFramebuffer* dfb);
  void GeraFramebufferPrincipal(int tamanho, DadosFramebuffer* dfb);

  float DistanciaPlanoCorteDistante() const;

  void DesativaWatchdog();
  void ReativaWatchdog();

  // Funcoes de preenchimento que requerem o tabuleiro por causa de entidades dinamicas.
  // Configura as notificacoes para varias entidades montarem em montaria.
  void PreencheNotificacoesMontarEm(
      const std::vector<const Entidade*>& montandos, const Entidade* montaria, ntf::Notificacao* grupo) const;
  // Configura as notificacoes para varias entidades desmontarem de montaria.
  void PreencheNotificacoesDesmontar(
      const std::vector<const Entidade*>& desmontandos, ntf::Notificacao* grupo) const;

  void RequerAtualizacaoLuzesPontuais();

 protected:
  /** mapa geral de entidades, por id. */
  MapaEntidades entidades_;

 private:
  void ParaTimersPorEntidade();
  void DisparaTimerEntidadeCorrente();

  const Tabelas& tabelas_;
  // Parametros de desenho, importante para operacoes de picking e manter estado durante renderizacao.
  mutable ParametrosDesenho parametros_desenho_;
  // Parametros do tabuleiro (sem entidades).
  TabuleiroProto proto_;
  // Opcoes do usuario.
  OpcoesProto opcoes_;
  int pagina_lista_objetos_ = 0;

  /** Cada cliente possui um identificador diferente. */
  int id_cliente_;

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
  ItemSelecionado item_selecionado_;
  std::unordered_map<std::string, std::unique_ptr<Modelo>> mapa_modelos_com_parametros_;

  /** Ação selecionada (por id). */
  MapaIdAcao mapa_acoes_;
  MapaTipoAcao mapa_acoes_por_tipo_;
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
  std::map<unsigned int, Olho> camera_por_id_;  // A camera de cada identificador.

  std::list<std::string> log_eventos_;
  std::unordered_map<std::string, std::string> log_eventos_clientes_;
  int pagina_log_eventos_;
  int pagina_horizontal_log_eventos_;

#if !USAR_QT
  std::vector<EntidadeProto> entidades_copiadas_;
#endif

  // Para processamento de grupos de notificacoes.
  bool processando_grupo_;
  std::vector<unsigned int> ids_adicionados_;
  // Esse mapa mapeia qual era o id da entidade na entrada e qual foi realmente adicionado.
  // Isso é usado para remapear campos dependentes do id (como montado em, por exemplo).
  std::unordered_map<unsigned int, unsigned int> mapa_ids_adicionados_;

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
  /** computa o tempo gasto em cada entidade com iniciativa ligada. */
  std::unordered_map<std::string, boost::timer::cpu_timer> timer_por_entidade_;
  /** Histograma de timer por entidade. */
  std::unordered_map<std::string, Histograma> histograma_por_entidade_;

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
  modo_clique_e modo_clique_anterior_ = MODO_NORMAL;
  // Cada botao fica apertado por um numero de frames apos pressionado. Este mapa mantem o contador.
  std::map<IdBotao, int> contador_pressao_por_controle_;
  // Para o modo dado, indica qual rolar.
  int faces_dado_ = 0;

  // Variaveis de estado de alguns botoes.
  bool modo_dano_automatico_ = true;
  bool bonus_dano_negativo_ = false;
  bool bonus_ataque_negativo_ = false;
  bool mostrar_dados_ = false;
  bool mostrar_dados_forcados_ = false;

  gl::VbosGravados vbos_tabuleiro_;
  gl::VbosGravados vbos_grade_;
  gl::VboGravado vbo_caixa_ceu_;
  gl::VboGravado vbo_cubo_;
  gl::VboGravado vbo_rosa_;
  DadosFramebuffer dfb_luz_direcional_;
  DadosFramebuffer dfb_oclusao_;
  DadosFramebuffer dfb_colisao_;
  DadosFramebuffer dfb_principal_;
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
  TabuleiroProto* proto_corrente_ = &proto_;

  // Controle virtual.
  ControleVirtualProto controle_virtual_;
  std::map<IdBotao, const DadosBotao*> mapa_botoes_controle_virtual_;
  std::set<std::string> texturas_entidades_;
  std::set<std::string> modelos_entidades_;
  std::set<std::string> itens_menu_;
  // Indica iniciativa valida. Pode acontecer apos remocoes de ficar invalidado.
  // Significa que o indice esta certo, mas apontava para outra entidade que foi removida.
  bool iniciativa_valida_ = false;
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
  ntf::Notificacao notificacao_doacao_;
  ntf::Notificacao notificacao_pericia_;

  // Posicao das luzes, para mapeamento de luzes. Apenas a primeira é usada por enquanto.
  struct LuzPontual {
    unsigned int id;
    Posicao pos;
  };
  std::vector<LuzPontual> luzes_pontuais_;

  // Usado para recuperacao de contexto IOS e android.
  bool regerar_vbos_entidades_ = false;

  // A imagem mostrada pelo mestre.
  InfoTextura imagem_mostrada_;

  // elimina copia
  Tabuleiro(const Tabuleiro& t);
  Tabuleiro& operator=(Tabuleiro&);
};

}

#endif
