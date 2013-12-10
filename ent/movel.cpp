#include <cmath>
#include <GL/gl.h>
#include <GL/glut.h>
#include "ent/movel.h"

using namespace ent;

const unsigned int NUM_FACES = 10;
const unsigned int NUM_LINHAS = 1;
const double ALTURA = 1.5;
const double RAIO_CONE = 0.5;
const double RAIO_ESFERA = 0.3;


class Movel::Dados {
public:

};

Movel::Movel(int id, int pontosVida, double x, double y, double z) : Entidade(id, pontosVida, x, y, z), dm_(new Dados) {}
Movel::~Movel() { delete dm_; }

void Movel::Desenha() {
	glPushMatrix();

	glTranslated(X(), Y(), Z());

	// desenha o cone com NUM_FACES faces com raio de RAIO e altura ALTURA
	glLoadName(Id());
  glutSolidCone(RAIO_CONE, ALTURA, NUM_FACES, NUM_LINHAS);
	glTranslated(0, 0, ALTURA);
	glutSolidSphere(RAIO_ESFERA, NUM_FACES, NUM_FACES);
	
	glPopMatrix();
}

