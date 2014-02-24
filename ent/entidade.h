#ifndef ENT_ENTIDADE_H
#define ENT_ENTIDADE_H

#include "ent/entidade.pb.h"

namespace ntf {
class CentralNotificacoes;
}  // namespace ntf
namespace ent {

class Entidade;
class DesenhoBase;
class IluminacaoDirecional;

/** Interface de texturas para entidades. */
class Texturas {
 public:
  /** Retorna o id da textura ou GL_INVALID_VALUE. */
  virtual unsigned int Textura(const std::string& id) const = 0;
};

/** Constroi uma entidade de acordo com o proto passando, inicializando-a. */
Entidade* NovaEntidade(const EntidadeProto& proto, Texturas* texturas, ntf::CentralNotificacoes* central);

/** classe base para entidades.
* Toda entidade devera possuir um identificador unico.
*/
class Entidade {
 public:
  /** Inicializa a entidade, recebendo seu proto diretamente. */
  void Inicializa(const EntidadeProto& proto);

  /** Atualiza a entidade usando apenas alguns campos do proto passado. Nao atualiza posicao. */
  void AtualizaProto(const EntidadeProto& novo_proto);

  /** Atualiza a posição da entidade em direção a seu destino. Ao alcançar o destino, o limpa. */
  void Atualiza();

  /** Destroi a entidade. */
  virtual ~Entidade();

  /** @return o identificador da entidade que deve ser unico globalmente. */
  unsigned int Id() const { return proto_.id(); }

  TipoEntidade Tipo() const { return proto_.tipo(); }

  /** Move a entidade para o ponto especificado. Limpa destino. */
  void MovePara(float x, float y, float z = 0);
  void MovePara(const Posicao& proto);

  /** Move a entidade uma quantidade em cada eixo. Limpa destino. */
  void MoveDelta(float dx, float dy, float dz);

  /** Atribui um destino a entidade. A cada atualizacao ela se movera em direcao ao destino. */
  void Destino(const Posicao& pos);

  /** @return o HP da unidade. */
  int PontosVida() const;

  /** @return a coordenada (x). */
  double X() const;

  /** @return a coordenada (y). */
  double Y() const;

  /** @return a coordenada (z). */
  double Z() const;

  /** Retorna as coordenadas do objeto como posicao. */
  const Posicao& Pos() const { return proto_.pos(); }

  /** Mata a entidade, ligando os bits de queda, morte e desligando voo e destino. */
  void MataEntidade();

  /** Atualiza pontos de vida do proto passado, matando ou revivendo os bits de acordo. */
  static void AtualizaPontosVidaProto(int delta_pontos_vida, EntidadeProto* proto);

  /** @return a posicao das acoes da entidade. */
  const Posicao PosicaoAcao() const;

  /** As luzes devem ser desenhadas primeiro, portanto há uma função separada para elas. */
  void DesenhaLuz(ParametrosDesenho* pd);

  /** desenha o objeto de forma solida. Pode alterar os parametros de desenho. */
  void Desenha(ParametrosDesenho* pd);

  /** Desenha a unidade de forma translucida. */
  void DesenhaTranslucido(ParametrosDesenho* pd);

  /** Desenha aura da entidade. */
  void DesenhaAura(ParametrosDesenho* pd);

  /** Monta a matriz de shear de acordo com posicao da luz, anula eixo Z e desenha o objeto com transparencia. */
  void DesenhaSombra(ParametrosDesenho* pd, const float* matriz_shear);

  /** Retorna o proto da entidade. */
  const EntidadeProto& Proto() const { return proto_; }

  /** Retorna se a entidade eh selecionavel para jogador. */
  bool SelecionavelParaJogador() const { return proto_.selecionavel_para_jogador(); }

  /** Retorna o multiplicador de tamanho para a entidade de acordo com seu tamanho. Por exemplo, retorna
  * 1.0f para entidades medias.
  */
  float MultiplicadorTamanho() const;

 private:
  friend Entidade* NovaEntidade(const EntidadeProto& proto, Texturas*, ntf::CentralNotificacoes*);
  Entidade(Texturas* texturas, ntf::CentralNotificacoes* central);

  /** Realiza as chamadas de notificacao para as texturas. */
  void AtualizaTexturas(const ent::EntidadeProto& novo_proto);

  /** A oscilacao de voo nao eh um movimento real (nao gera notificacoes). Esta funcao retorna o delta. */
  float DeltaVoo() const;

  /** Realiza o desenho do objeto com as decoracoes, como disco de selecao e barra de vida (de acordo com pd). */
  void DesenhaObjetoComDecoracoes(ParametrosDesenho* pd);

  /** desenha apenas o objeto, sem alterar cor nem matriz. Implementado em entidade_desenho.cpp. */
  void DesenhaObjeto(ParametrosDesenho* pd, const float* matriz_shear = nullptr);

  /** Desenha as decoracoes do objeto (pontos de vida, disco de selecao. */
  void DesenhaDecoracoes(ParametrosDesenho* pd);

  /** Auxiliar para montar a matriz de desenho do objeto.
  * @param usar_delta_voo se verdadeiro, posiciona matriz no ar, caso contrario no solo.
  */
  void MontaMatriz(bool usar_delta_voo, const float* matriz_shear = nullptr) const;

  // Funcoes de desenho.
  void DesenhaObjetoEntidade(ParametrosDesenho* pd, const float* matriz_shear);
  void DesenhaObjetoForma(ParametrosDesenho* pd, const float* matriz_shear);

 private:
  EntidadeProto proto_;
  // Como esse estado é local e não precisa ser salvo, fica aqui.
  double rotacao_disco_selecao_graus_;
  // Entidades em voo oscilam sobre a altura do voo. A oscilacao eh baseada no seno deste angulo.
  double angulo_disco_voo_rad_;
  // Entidades em queda caem progressivamente ate 90 graus.
  double angulo_disco_queda_graus_;

  Texturas* texturas_;
  // A central é usada apenas para enviar notificacoes de textura ja que as entidades nao sao receptoras.
  ntf::CentralNotificacoes* central_;
};

}  // namespace ent

#endif
