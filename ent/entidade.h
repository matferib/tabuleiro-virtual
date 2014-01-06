#ifndef ENT_ENTIDADE_H
#define ENT_ENTIDADE_H

#include "ent/entidade.pb.h"

namespace ent {

class Entidade;

/** classe base para entidades.
* Toda entidade devera possuir um identificador unico. 
*/
class Entidade {
public:
  /** Identificador é composto pelo id_criador << 28 | id_local. Portanto, o maximo de jogadores é 16. 
  * É importante que o identificador da entidade caiba em um GLuint para permitir realização da operação
  * de picking no 3d.
  */
	Entidade(int id_criador, int id_local, int pontosVida, double x, double y, double z);

  /** Construtor para entidades criadas remotamente. */
  explicit Entidade(const EntidadeProto& proto);

  /** Destroi a entidade. */
	virtual ~Entidade();

public:
	/** @return o identificador da entidade que deve ser unico globalmente. */
	unsigned int Id() const;

	/** Move a entidade para o ponto especificado. Limpa destino. */
	void MovePara(double x, double y, double z = 0);

  /** Atribui um destino a entidade. */
  void Destino(const EntidadeProto& proto);

	/** Atualiza a posição da entidade em direção a seu destino. Ao alcançar o destino, o limpa. */
	void Atualiza();

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
	
	/** desenha o objeto. */
	virtual void Desenha();

  /** Retorna o proto da entidade. */
  const EntidadeProto& Proto() const;

 private:
  EntidadeProto proto_;
};

}

#endif

