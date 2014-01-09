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
const double RAIO_CONE = 0.75;
const double RAIO_ESFERA = 0.3;
const double VELOCIDADE_POR_EIXO = 0.1;  // deslocamento em cada eixo (x, y, z) por chamada de atualizacao.

/** Altera a cor corrente para cor. */
void MudaCor(const ent::Cor& cor) {
  GLfloat cor_gl[] = { cor.r(), cor.g(), cor.b(), cor.a() };
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cor_gl);
  glColor3fv(cor_gl);
}

// Multiplicador de dimensão por tamanho de entidade.
float CalculaMultiplicador(TamanhoEntidade tamanho) {
  switch (tamanho) {
    case ent::TM_MINUSCULO: return 0.4f;
    case ent::TM_DIMINUTO: return 0.5f;
    case ent::TM_MIUDO: return 0.6f;
    case ent::TM_PEQUENO: return 0.7f;
    case ent::TM_MEDIO: return 1.0f;
    case ent::TM_GRANDE: return 2.0f;
    case ent::TM_ENORME: return 3.0f;
    case ent::TM_IMENSO: return 4.0f;
    case ent::TM_COLOSSAL: return 5.0f;
  }
  LOG(ERROR) << "Tamanho inválido: " << tamanho;
  return 1.0f;
}
}  // namespace

// Factory.
Entidade* NovaEntidade(TipoEntidade tipo) {
  switch (tipo) {
    case TE_ENTIDADE:
      return new Entidade;
    default:
      LOG(ERROR) << "Tipo de entidade inválido: " << tipo;
      return nullptr;
  }
}

// Entidade
Entidade::Entidade() {
  proto_.set_tipo(TE_ENTIDADE);
  rotacao_disco_selecao_ = 0;
}

void Entidade::Inicializa(const EntidadeProto& proto) { 
  // mantem o tipo.
  TipoEntidade tipo = proto_.tipo();
  proto_.CopyFrom(proto);
  proto_.set_tipo(tipo);
}

void Entidade::Atualiza(const EntidadeProto& proto) { 
  // mantem o tipo.
  ent::EntidadeProto copia_proto(proto_);
  proto_.CopyFrom(proto);
  proto_.set_id(copia_proto.id());
  proto_.mutable_pos()->Swap(copia_proto.mutable_pos());
  if (copia_proto.has_destino()) {
    proto_.mutable_destino()->Swap(copia_proto.mutable_destino());
  }
  LOG(INFO) << "Proto: " << proto_.ShortDebugString();
}

void Entidade::Atualiza() {
  rotacao_disco_selecao_ = fmod(rotacao_disco_selecao_ + 1.0, 360.0);

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
  float multiplicador = CalculaMultiplicador(proto_.tamanho());
  // Aplica uma pequena diminuição para não ocupar o quadrado todo.
  glutSolidCone(RAIO_CONE * multiplicador - 0.2f, ALTURA * multiplicador, NUM_FACES, NUM_LINHAS);
  glPushMatrix();
	glTranslated(0, 0, ALTURA * multiplicador);
	glutSolidSphere(RAIO_ESFERA * multiplicador, NUM_FACES, NUM_FACES);
  glPopMatrix();

  if (pd->entidade_selecionada()) {
    glRotatef(rotacao_disco_selecao_, 0, 0, 1.0f);
    glNormal3f(0, 0, 1.0f);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-0.2f, -0.2f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0, 0.0, 0.0);
    float raio_disco = RAIO_CONE * multiplicador * 1.2f;
    for (int i = 0; i <= 6; ++i) {
      float angulo = i * M_PI / 3.0f;
      glVertex3f(cosf(angulo) * raio_disco, sinf(angulo) * raio_disco, 0.0f);
    }
    glEnd();
    glDisable(GL_POLYGON_OFFSET_FILL);
  }
	
	glPopMatrix();
}

void Entidade::DesenhaLuz(ParametrosDesenho* pd) {
  if (!pd->iluminacao() || !proto_.has_luz()) {
    return;
  }

  // Objeto de luz. O quarto componente indica que a luz é posicional.
  // Se for 0, a luz é direcional e os componentes indicam sua direção.
  GLfloat pos_luz[] = { 0, 0, static_cast<GLfloat>(ALTURA * CalculaMultiplicador(proto_.tamanho())), 1.0f };
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
    glLightf(GL_LIGHT0 + id_luz, GL_LINEAR_ATTENUATION, -0.05);
    glLightf(GL_LIGHT0 + id_luz, GL_QUADRATIC_ATTENUATION, 0.05);
    glEnable(GL_LIGHT0 + id_luz);
    pd->set_luz_corrente(id_luz + 1);
  }
  glPopMatrix();
}

const EntidadeProto& Entidade::Proto() const {
  return proto_;
}

}  // namespace ent
