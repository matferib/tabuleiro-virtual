#ifndef ENT_ENTIDADE_H
#define ENT_ENTIDADE_H

#include "ent/entidade.pb.h"

namespace ntf {
class CentralNotificacoes;
}  // namespace ntf
namespace ent {

class Entidade;

/** Dados de renderizacao para texturas. O formato é sempre RGBA. */
struct InfoTextura {
  int altura;
  int largura;
  int formato;  // GLenum: ver glTexImage.
  int tipo;  // ditto.
  const void* dados;
};

/** Interface de texturas para entidades. */
class Texturas {
 public:
  virtual const InfoTextura* Textura(const std::string& id) const = 0;
};

/** Constroi uma entidade de acordo com o tipo, que deve pertencer a enum TipoEntidade. */
Entidade* NovaEntidade(TipoEntidade tipo, Texturas* texturas, ntf::CentralNotificacoes* central);

/** classe base para entidades.
* Toda entidade devera possuir um identificador unico.
*/
class Entidade {
 public:
  /** Inicializa a entidade, recebendo seu proto diretamente. */
  void Inicializa(const EntidadeProto& proto);

  /** Atualiza a entidade usando apenas alguns campos do proto passado. */
  void Atualiza(const EntidadeProto& novo_proto);

  /** Atualiza a posição da entidade em direção a seu destino. Ao alcançar o destino, o limpa. */
  void Atualiza();

  /** Destroi a entidade. */
  virtual ~Entidade();

  /** @return o identificador da entidade que deve ser unico globalmente. */
  unsigned int Id() const;

  /** Move a entidade para o ponto especificado. Limpa destino. */
  void MovePara(double x, double y, double z = 0);

  /** Atribui um destino a entidade. */
  void Destino(const EntidadeProto& proto);

  /** @return o HP da unidade. */
  int PontosVida() const;

  /** aplica dano ou cura na entidade. Dano eh negativo, cura eh positivo. */
  void DanoCura(int pontosVida);

  /** @return a coordenada (x). */
  double X() const;

  /** @return a coordenada (y). */
  double Y() const;

  /** @return a coordenada (z). */
  double Z() const;

  /** As luzes devem ser desenhadas primeiro, portanto há uma função separada para elas. */
  virtual void DesenhaLuz(ParametrosDesenho* pd);

  /** desenha o objeto. Pode alterar os parametros de desenho. */
  virtual void Desenha(ParametrosDesenho* pd);

  /** Retorna o proto da entidade. */
  const EntidadeProto& Proto() const;

 private:
  friend Entidade* NovaEntidade(TipoEntidade, Texturas*, ntf::CentralNotificacoes*);
  Entidade(Texturas* texturas, ntf::CentralNotificacoes* central);

  /** Realiza as chamadas de notificacao para as texturas. */
  void AtualizaTexturas(const ent::EntidadeProto& novo_proto);

 private:
  EntidadeProto proto_;
  // Como esse estado é local e não precisa ser salvo, fica aqui.
  double rotacao_disco_selecao_;
  Texturas* texturas_;
  // A central é usada apenas para enviar notificacoes de textura ja que as entidades nao sao receptoras.
  ntf::CentralNotificacoes* central_;
};

}  // namespace ent

#endif
