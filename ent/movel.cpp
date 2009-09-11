#include <cmath>
#include <GL/gl.h>
#include "ent/movel.h"
#include "ent/parametrosdesenho.h"

using namespace ent;

const unsigned int NUM_FACES = 10;
const double ALTURA = 1.8;
const double RAIO = 0.5;


class Movel::Dados {
public:

};

Movel::Movel(int id) : Entidade(id), dm_(new Dados) {}
Movel::~Movel() { delete dm_; }

void Movel::desenha(const ParametrosDesenho& pd) {
	// desenha o cone com NUM_FACES faces com raio de RAIO e altura ALTURA
	glLoadName(id());
	glColor3dv(selecionado() ? pd.corObjetoSelecionado() : pd.corObjetoNaoSelecionado());
	glLoadName(id());
	glBegin(GL_TRIANGLE_FAN);
	glVertex3d(0, 0, ALTURA);
	double alfa = -M_PI / 2.0;
	for (unsigned int i = 0; i < NUM_FACES + 2; ++i) {
		glVertex3d(cos(alfa)*RAIO, sin(alfa)*RAIO, 0);
		alfa += (M_PI * 2.0 / NUM_FACES);
	}
	glEnd();
}

