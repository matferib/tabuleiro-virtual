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
const double VELOCIDADE_POR_EIXO = 0.1;  // deslocamento em cada eixo por chamada de atualizacao.

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
  MovePara(x, y, z);
}

Entidade::Entidade(const EntidadeProto& proto) { 
  proto_.CopyFrom(proto);
}

Entidade::~Entidade() {}

unsigned int Entidade::Id() const { return proto_.id(); }

void Entidade::MovePara(double x, double y, double z) { 
  auto* p = proto_.mutable_pos();
  p->set_x(x);
  p->set_y(y);
  p->set_z(z);
  proto_.clear_destino();
}

void Entidade::Destino(const EntidadeProto& proto) { 
  proto_.mutable_destino()->CopyFrom(proto.destino());
}

void Entidade::Atualiza() {
  if (!proto_.has_destino()) {
    return;
  }
  auto* po = proto_.mutable_pos();
  double origens[] = { po->x(), po->y(), po->z() };
  const auto& pd = proto_.destino();
  double destinos[] = { pd.x(), pd.y(), pd.z() };

  bool chegou = true;
  for (int i = 0; i < 3; ++i) {
    double delta = (origens[i] > destinos[i]) ? -VELOCIDADE_POR_EIXO : VELOCIDADE_POR_EIXO;
    if (fabs(origens[i] - destinos[i]) > VELOCIDADE_POR_EIXO) {
      origens[i] += delta;
      chegou = false;
    } else {
      origens[i] = destinos[i];
    }
  }
  po->set_x(origens[0]);
  po->set_y(origens[1]);
  po->set_z(origens[2]);
  if (chegou) {
    proto_.clear_destino();
  }
}

int Entidade::PontosVida() const {
	return 0;
}

void Entidade::DanoCura(int pontos_vida) {
}

double Entidade::X() const { 
	return proto_.pos().x(); 
}
double Entidade::Y() const { 
	return proto_.pos().y(); 
}
double Entidade::Z() const { 
	return proto_.pos().z(); 
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
