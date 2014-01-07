#ifndef ENT_TABULEIRO_H
#define ENT_TABULEIRO_H

#include <map>
#include <vector>
#include "ent/entidade.pb.h"
#include "ntf/notificacao.h"

namespace ntf {
  class Notificacao;
}

namespace ent {

class Entidade;

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
  ETAB_ENT_PRESSIONADA,
  ETAB_ENT_SELECIONADA,
  ETAB_QUAD_PRESSIONADO,
  ETAB_QUAD_SELECIONADO,
};

typedef std::map<unsigned int, Entidade*> MapaEntidades;

/** Responsavel pelo mundo do jogo. O sistema de coordenadas tera X Y como base e
* Z como altura (positivo pro alto).
*/
class Tabuleiro : public ntf::Receptor {
 public:
  /** cria o tabuleiro com o tamanho de grid passado (tamanho x tamanho). 
  * O identificador do tabuleiro sempre eh 0. 
  */
  Tabuleiro(int tamanho, ntf::CentralNotificacoes* central);

  /** libera os recursos do tabuleiro, inclusive entidades. */
  virtual ~Tabuleiro();

  /** @return tamanho do tabuleiro em X. */
  int TamanhoX() const;

  /** @return tamanho do tabuleiro em Y. */
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
  void TrataMovimento(int x, int y);

  /** trata o botao do mouse liberado. */
  void TrataBotaoLiberado();

  /** trata o botao pressionado, recebendo x, y (ja em coordenadas opengl) e
  * a razao de aspecto da janela.
  */
  void TrataBotaoPressionado(botao_e botao, int x, int y, double aspecto);

  /** trata a redimensao da janela. */
  void TrataRedimensionaJanela(int largura, int altura);

  /** inicializa os parametros do openGL. */
  static void InicializaGL();

 private:
  /** funcao que desenha a cena independente do modo.
  */
  void DesenhaCena();

  /** Encontra os hits de um clique em objetos. */
  void EncontraHits(int x, int y, double aspecto, unsigned int* numero_hits, unsigned int* buffer_hits);

  /** trata o clique, recebendo o numero de hits e o buffer de hits do opengl. */
  void TrataClique(unsigned int numero_hits, unsigned int* buffer_hits);

  /** seleciona a entidade pelo ID. */ 
  void SelecionaEntidade(unsigned int id);

  /** seleciona o quadrado pelo ID. */
  void SelecionaQuadrado(int id_quadrado);

  /** retorna as coordenadas do quadrado. */
  void CoordenadaQuadrado(int id_quadrado, double* x, double* y, double* z);

  /** Retorna uma notificacao do tipo TN_SERIALIZAR_TABULEIRO preenchida. */
  ntf::Notificacao* SerializaTabuleiro();

  /** Monta o tabuleiro de acordo com a notificacao TN_DESERIALIZAR_TABULEIRO. */
  void DeserializaTabuleiro(const ntf::Notificacao& notificacao);

 private:
  ParametrosDesenho parametros_desenho_;

  /** tamanho do tabuleiro (tamanho_ x tamanho_). */
  int tamanho_;

  /** Cada cliente possui um identificador diferente. */
  int id_cliente_;

  /** mapa geral de entidades, por id. */
  MapaEntidades entidades_;

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

  /** dados (X) para calculo de rotacao de mouse. */
  int rotacao_ultimo_x_; 
  /** dados (Y) para calculo de rotacao de mouse. */
  int rotacao_ultimo_y_; 

  // Para onde o olho olha.
  double olho_x_;
  double olho_y_;
  double olho_z_;
  // De onde o olho olha.
  double olho_delta_rotacao_;
  double olho_altura_;
  double olho_raio_;

  ntf::CentralNotificacoes* central_;

  // elimina copia
  Tabuleiro(const Tabuleiro& t);
  Tabuleiro& operator=(Tabuleiro&);
};

}

#endif
