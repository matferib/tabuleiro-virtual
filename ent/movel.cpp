#include <cmath>
#include <GL/gl.h>
#include <GL/glut.h>
#include "ent/movel.h"
#include "ent/parametrosdesenho.h"

using namespace ent;

const unsigned int NUM_FACES = 10;
const double ALTURA = 1.5;
const double RAIO_CONE = 0.5;
const double RAIO_ESFERA = 0.3;


class Movel::Dados {
public:

};

Movel::Movel(int id, double x, double y, double z) : Entidade(id, x, y, z), dm_(new Dados) {}
Movel::~Movel() { delete dm_; }

void Movel::desenha(const ParametrosDesenho& pd) {
	glPushMatrix();
	// desenha o cone com NUM_FACES faces com raio de RAIO e altura ALTURA
	glLoadName(id());
	glBegin(GL_TRIANGLE_FAN);
	glVertex3d(0, 0, ALTURA);
	double alfa = -M_PI / 2.0;
	for (unsigned int i = 0; i < NUM_FACES + 2; ++i) {
		glVertex3d( cos(alfa) * RAIO_CONE, sin(alfa) * RAIO_CONE, 0);
		alfa += (M_PI * 2.0 / NUM_FACES);
	}
	glEnd();
	glTranslated(0, 0, ALTURA);
	glutSolidSphere(RAIO_ESFERA, NUM_FACES, NUM_FACES);
	glPopMatrix();
}

