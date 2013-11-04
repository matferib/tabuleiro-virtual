#ifndef ENT_ENTIDADE_H
#define ENT_ENTIDADE_H

namespace ent {

class Entidade;

/** parametros de desenho dos objetos. */
class ParametrosDesenho;

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
	int id() const;

	/** move a entidade para o ponto especificado. */
	void movePara(double x, double y, double z = 0);

	/** move a entidade pelo vetor de movimento. */
	void move(double x, double y, double z = 0);

	/** @return o HP da unidade. */
	int pontosVida() const;

	/** aplica dano ou cura na entidade. Dano eh negativo, cura eh positivo. */
	void danoCura(int pontosVida);

	/** @return a coordenada (x). */
	double x() const;

	/** @return a coordenada (y). */
	double y() const;

	/** @return a coordenada (z). */
	double z() const;
	
	/** desenha o objeto, recebendo os parametros de desenho. */
	virtual void desenha(const ParametrosDesenho& pd) = 0;

private:
	/** identificador da entidade, deve ser unico. */
	int id_;

	/** maximo de pontos de vida. */
	int maximoPontosVida_;

	/** pontos de vida corrente da unidade. */
	int pontosVida_;

	/** coordenadas da entidade. */
	double x_, y_, z_;
};

}

#endif

