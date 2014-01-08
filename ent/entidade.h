#ifndef ENT_ENTIDADE_H
#define ENT_ENTIDADE_H

#include "ent/entidade.pb.h"

namespace ent {

class Entidade;

/** Constroi uma entidade de acordo com o tipo, que deve pertencer a enum TipoEntidade. */
Entidade* NovaEntidade(TipoEntidade tipo);

/** classe base para entidades.
* Toda entidade devera possuir um identificador unico. 
*/
class Entidade {
 public:
  /** Inicializa a entidade, recebendo seu proto diretamente. */
  void Inicializa(const EntidadeProto& proto);

  /** Atualiza a entidade, usando so alguns campos de proto. */
  void Atualiza(const EntidadeProto& proto);

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

 protected:
  friend Entidade* NovaEntidade(TipoEntidade);
  Entidade();

 protected:
  EntidadeProto proto_;
  // Como esse estado é local e não precisa ser salvo, fica aqui.
  double rotacao_disco_selecao_;
};

/** Uma entidade de luz. */
class Luz : public Entidade {
 public:
	virtual void Desenha(ParametrosDesenho* pd) override;
	virtual void DesenhaLuz(ParametrosDesenho* pd) override;

 protected:
  friend Entidade* NovaEntidade(TipoEntidade);
  Luz();
};

}  // namespace ent

#endif

