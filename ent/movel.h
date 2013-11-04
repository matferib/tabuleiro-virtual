#ifndef ENT_MOVEL_H
#define ENT_MOVEL_H

#include "ent/entidade.h"

namespace ent {

/** qualquer entidade movel. Representada por um cone. */
class Movel : public Entidade {
public:
	Movel(int id, int pontosVida, double x, double y, double z);
	~Movel();

	virtual void desenha(const ParametrosDesenho& pd);

private:
	class Dados;
	Dados* dm_;
};

}

#endif
