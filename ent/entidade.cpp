#include <cmath>
#include <stdexcept>
#include <GL/gl.h>
#include <GL/glut.h>
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "log/log.h"

namespace {
const unsigned int NUM_FACES = 10;
const unsigned int NUM_LINHAS = 1;
const double ALTURA = 1.5;
const double RAIO_CONE = 0.5;
const double RAIO_ESFERA = 0.3;
const double VELOCIDADE_POR_EIXO = 0.1;  // deslocamento em cada eixo por chamada de atualizacao.

/** Altera a cor corrente para cor. */
void MudaCor(const ent::Cor& cor) {
  GLfloat cor_gl[] = { cor.r(), cor.g(), cor.b(), cor.a() };
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cor_gl);
  glColor3fv(cor_gl);
}

}  // namespace

namespace ent {

// Factory.
Entidade* NovaEntidade(int tipo) {
  switch (tipo) {
    case TE_ENTIDADE:
      return new Entidade;
    case TE_LUZ:
      return new Luz;
    default:
      return nullptr;
  }
}

// Entidade
Entidade::Entidade() {
  proto_.set_tipo(TE_ENTIDADE);
}

void Entidade::Inicializa(const EntidadeProto& proto) { 
  // mantem o tipo.
  int tipo = proto_.tipo();
  proto_.CopyFrom(proto);
  proto_.set_tipo(tipo);
}

void Entidade::Atualiza(const EntidadeProto& proto) { 
  // mantem o tipo.
  ent::EntidadeProto copia_proto(proto_);
  proto_.CopyFrom(proto);
  proto_.set_tipo(copia_proto.id());
  proto_.mutable_pos()->Swap(copia_proto.mutable_pos());
  if (copia_proto.has_destino()) {
    proto_.mutable_destino()->Swap(copia_proto.mutable_destino());
  }
  LOG(INFO) << "Proto: " << proto_.ShortDebugString();
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

void Entidade::Desenha(ParametrosDesenho* pd) {
	glPushMatrix();

	glTranslated(X(), Y(), Z());

	// desenha o cone com NUM_FACES faces com raio de RAIO e altura ALTURA
  MudaCor(proto_.cor());
	glLoadName(Id());
  glutSolidCone(RAIO_CONE, ALTURA, NUM_FACES, NUM_LINHAS);
	glTranslated(0, 0, ALTURA);
	glutSolidSphere(RAIO_ESFERA, NUM_FACES, NUM_FACES);
	
	glPopMatrix();
}

void Entidade::DesenhaLuz(ParametrosDesenho* pd) {
  if (!pd->iluminacao() || !proto_.has_luz()) {
    return;
  }

  // Objeto de luz. O quarto componente indica que a luz é posicional.
  // Se for 0, a luz é direcional e os componentes indicam sua direção.
  GLfloat pos_luz[] = { 0, 0, static_cast<GLfloat>(ALTURA), 1.0f };
  const ent::Cor& cor = proto_.luz().cor();
  GLfloat cor_luz[] = { cor.r(), cor.g(), cor.b(), cor.a() };
  glPushMatrix();
  glTranslated(X(), Y(), Z());
  int id_luz = pd->luz_corrente();
  if (id_luz == 0 || id_luz >= pd->max_num_luzes()) {
    LOG(ERROR) << "Limite de luzes alcançado: " << id_luz;
  } else {
    glLightfv(GL_LIGHT0 + id_luz, GL_POSITION, pos_luz);
    glLightfv(GL_LIGHT0 + id_luz, GL_DIFFUSE, cor_luz);
    glLightf(GL_LIGHT0 + id_luz, GL_CONSTANT_ATTENUATION, 1.0);
    glEnable(GL_LIGHT0 + id_luz);
    pd->set_luz_corrente(id_luz + 1);
  }
  glPopMatrix();
}

const EntidadeProto& Entidade::Proto() const {
  return proto_;
}

// LUZ

Luz::Luz() {
  proto_.set_tipo(TE_LUZ);
}

void Luz::DesenhaLuz(ParametrosDesenho* pd) {
	glPushMatrix();
	glTranslated(X(), Y(), Z());

  // Objeto de luz. O quarto componente indica que a luz é posicional.
  // Se for 0, a luz é direcional e os componentes indicam sua direção.
  GLfloat pos_luz[] = { 0, 0, static_cast<GLfloat>(ALTURA), 1.0f };
  GLfloat cor_luz[] = { 1.0, 1.0, 1.0, 1.0 };
  if (pd->iluminacao()) {
    int id_luz = pd->luz_corrente();
    if (id_luz == 0 || id_luz >= pd->max_num_luzes()) {
      LOG(ERROR) << "Limite de luzes alcançado: " << id_luz;
    } else {
      glLightfv(GL_LIGHT0 + id_luz, GL_POSITION, pos_luz);
      glLightfv(GL_LIGHT0 + id_luz, GL_DIFFUSE, cor_luz);
      glLightf(GL_LIGHT0 + id_luz, GL_CONSTANT_ATTENUATION, 1.0);
      glEnable(GL_LIGHT0 + id_luz);
      pd->set_luz_corrente(id_luz + 1);
    }
  }
	glPopMatrix();
}

void Luz::Desenha(ParametrosDesenho* pd) {
	glPushMatrix();

	glTranslated(X(), Y(), Z());

	// desenha o cone com NUM_FACES faces com raio de RAIO e altura ALTURA
	glLoadName(Id());
	glTranslated(0, 0, ALTURA);
	glutSolidSphere(RAIO_ESFERA, NUM_FACES, NUM_FACES);
	
	glPopMatrix();
}

}  // namespace ent
