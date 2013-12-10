#ifndef ENT_ENTIDADE_H
#define ENT_ENTIDADE_H

namespace ent {

class Entidade;

/** classe base para entidades.
* Toda entidade devera possuir um identificador unico. 
*/
class Entidade {
public:
	/** constroi a entidade com o identificador, pontos de vida e coordenadas passadas. */
	explicit Entidade(int id, int pontosVida, double x, double y, double z);
	virtual ~Entidade();

public:
	/** @return o identificador da entidade que deve ser unico. */
	int Id() const;

	/** move a entidade para o ponto especificado. */
	void MovePara(double x, double y, double z = 0);

	/** move a entidade pelo vetor de movimento. */
	void Move(double x, double y, double z = 0);

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
	
	/** desenha o objeto, recebendo os parametros de desenho. */
	virtual void Desenha() = 0;

private:
	/** identificador da entidade, deve ser unico. */
	int id_;

	/** maximo de pontos de vida. */
	int maximo_pontos_vida_;

	/** pontos de vida corrente da unidade. */
	int pontos_vida_;

	/** coordenadas da entidade. */
	double x_, y_, z_;
};

}

#endif

