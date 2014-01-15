#ifndef ENT_TABULEIRO_H
#define ENT_TABULEIRO_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "ent/entidade.pb.h"
#include "ent/tabuleiro.pb.h"
#include "ntf/notificacao.h"

namespace ntf {
  class Notificacao;
}

namespace ent {

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

/** Estados possiveis do tabuleiro. */
enum etab_t {
  ETAB_OCIOSO,
  ETAB_ROTACAO,
  ETAB_DESLIZANDO,
  ETAB_ENT_PRESSIONADA,
  ETAB_ENT_SELECIONADA,
  ETAB_QUAD_PRESSIONADO,
  ETAB_QUAD_SELECIONADO,
};

typedef std::unordered_map<unsigned int, Entidade*> MapaEntidades;
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

  /** remove entidade do tabuleiro, pelo id da entidade passada ou a selecionada. 
  */
  void RemoveEntidade(const ntf::Notificacao& notificacao);

  /** desenha o mundo. */
  void Desenha();

  /** Interface receptor. */
  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

  /** trata evento de rodela de mouse. */
  void TrataRodela(int delta);

  /** trata movimento do mouse (y ja em coordenadas opengl). */
  void TrataMovimento(botao_e botao, int x, int y);

  /** trata o botao do mouse liberado. */
  void TrataBotaoLiberado(botao_e botao);

  /** trata o botao pressionado, recebendo x, y (ja em coordenadas opengl).
  */
  void TrataBotaoPressionado(botao_e botao, int x, int y);

  /** Trata o click duplo, recebendo x, y (coordenadas opengl). */
  void TrataDuploClique(botao_e botao, int x, int y);

  /** trata a redimensao da janela. */
  void TrataRedimensionaJanela(int largura, int altura);

  /** inicializa os parametros do openGL. */
  static void InicializaGL();

 private:
  /** funcao que desenha a cena independente do modo.
  */
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
  * Efetivamente chama BuscaHitMaisProximo para obter a profundidade entao faz a projecao para aquele ponto.
  */
  bool MousePara3d(int x, int y, double* x3d, double* y3d, double* z3d);

  /** trata o clique do botao esquerdo. */
  void TrataCliqueEsquerdo(int x, int y);

  /** Trata o clique duplo do botao esquerdo. */
  void TrataDuploCliqueEsquerdo(int x, int y);

  /** Trata o duplo clique com botao direito. */
  void TrataDuploCliqueDireito(int x, int y);

  /** seleciona a entidade pelo ID. */ 
  void SelecionaEntidade(unsigned int id);

  /** deseleciona a entidade selecionada. */
  void DeselecionaEntidade();

  /** seleciona o quadrado pelo ID. */
  void SelecionaQuadrado(int id_quadrado);

  /** retorna as coordenadas do quadrado. */
  void CoordenadaQuadrado(int id_quadrado, double* x, double* y, double* z);

  /** @return uma notificacao do tipo TN_SERIALIZAR_TABULEIRO preenchida. */
  ntf::Notificacao* SerializaTabuleiro();

  /** @return uma notificacao do tipo TN_ABRIR_DIALOGO_ILUMINACAO preenchida. */
  ntf::Notificacao* SerializaIluminacaoTextura() const;

  /** Monta o tabuleiro de acordo com a notificacao TN_DESERIALIZAR_TABULEIRO. */
  void DeserializaTabuleiro(const ntf::Notificacao& notificacao);

  /** Deserializa apenas a parte de iluminacao e textura do tabuleiro. */
  void DeserializaIluminacaoTextura(const ent::TabuleiroProto& novo_proto);

  /** @return a entidade por id, ou nullptr se nao encontrá-la. */
  Entidade* BuscaEntidade(unsigned int id);

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
  void DesenhaQuadrado(unsigned int id, int linha, int coluna, bool selecionado, const InfoTextura* info);

  /** Desenha a grade do tabuleiro. */
  void DesenhaGrade();

  /** Retorna a razao de aspecto do viewport. */
  double Aspecto() const { return static_cast<double>(largura_) / altura_; }

 private:
  // Parametros de desenho, importante para operacoes de picking e manter estado durante renderizacao.
  ParametrosDesenho parametros_desenho_;
  // Parametros do tabuleiro (sem entidades).
  TabuleiroProto proto_;

  /** Cada cliente possui um identificador diferente. */
  int id_cliente_;

  /** mapa geral de entidades, por id. */
  MapaEntidades entidades_;
  /** um set com os id de clientes usados. */
  MapaClientes clientes_;

  /** a entidade selecionada. */
  Entidade* entidade_selecionada_;

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

  /** Dimensoes do viewport. */
  int largura_;
  int altura_;

  // Para onde o olho olha.
  Olho olho_;

  Texturas* texturas_;
  ntf::CentralNotificacoes* central_;

  // elimina copia
  Tabuleiro(const Tabuleiro& t);
  Tabuleiro& operator=(Tabuleiro&);
};

}

#endif
