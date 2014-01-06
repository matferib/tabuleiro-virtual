#include <cmath>
#include <stdexcept>
#include <GL/gl.h>
#include <GL/glut.h>
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "log/log.h"

const unsigned int NUM_FACES = 10;
const unsigned int NUM_LINHAS = 1;
const double ALTURA = 1.5;
const double RAIO_CONE = 0.5;
const double RAIO_ESFERA = 0.3;

using namespace ent;
using namespace std;

Entidade::Entidade(int id_criador, int id_local, int pontos_vida, double x, double y, double z) { 
  if (id_criador > (1 << 4)) {
    LOG(FATAL) << "Id criador invalido: " << id_criador;
  }
  if (id_local >= (1 << 29)) {
    LOG(FATAL) << "Id entidade invalido: " << id_local;
  }
  proto_.set_id((id_criador << 28) | id_local);
  proto_.set_x(x);
  proto_.set_y(y);
  proto_.set_z(z);
}

Entidade::Entidade(const EntidadeProto& proto) { 
  proto_.CopyFrom(proto);
}

Entidade::~Entidade() {}

unsigned int Entidade::Id() const { return proto_.id(); }

void Entidade::MovePara(double x, double y, double z) { 
  proto_.set_x(x);
  proto_.set_y(y);
  proto_.set_z(z);
}

void Entidade::MovePara(const EntidadeProto& proto) { 
  MovePara(proto.x(), proto.y(), proto.z());
}

void Entidade::Move(double x, double y, double z) { 
  proto_.set_x(proto_.x() + x);
  proto_.set_y(proto_.y() + y);
  proto_.set_z(proto_.z() + z);
}

int Entidade::PontosVida() const {
	return 0;
}

void Entidade::DanoCura(int pontos_vida) {
}

double Entidade::X() const { 
	return proto_.x(); 
}
double Entidade::Y() const { 
	return proto_.y(); 
}
double Entidade::Z() const { 
	return proto_.z(); 
}

void Entidade::Desenha() {
	glPushMatrix();

	glTranslated(X(), Y(), Z());

	// desenha o cone com NUM_FACES faces com raio de RAIO e altura ALTURA
	glLoadName(Id());
  glutSolidCone(RAIO_CONE, ALTURA, NUM_FACES, NUM_LINHAS);
	glTranslated(0, 0, ALTURA);
	glutSolidSphere(RAIO_ESFERA, NUM_FACES, NUM_FACES);
	
	glPopMatrix();
}

const EntidadeProto& Entidade::Proto() const {
  return proto_;
}
