#include <cmath>
#include <stdexcept>
#include <GL/gl.h>
#include <GL/glut.h>
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ifg/qt/texturas.h"
#include "log/log.h"

namespace ent {
namespace {

const unsigned int NUM_FACES = 10;
const unsigned int NUM_LINHAS = 1;
const double ALTURA = 1.5;
// deslocamento em cada eixo (x, y, z) por chamada de atualizacao.
const double VELOCIDADE_POR_EIXO = 0.1;

void MudaCor(float r, float g, float b, float a) {
  GLfloat cor_gl[] = { r, g, b, a };
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cor_gl);
  glColor3fv(cor_gl);
}

/** Altera a cor corrente para cor. */
void MudaCor(const ent::Cor& cor) {
  MudaCor(cor.r(), cor.g(), cor.b(), cor.a());
}

/** Desenha um disco no eixo x-y, com um determinado numero de faces. */
void DesenhaDisco(GLfloat raio, int num_faces) {
  glNormal3f(0, 0, 1.0f);
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(-0.2f, -0.2f);
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(0.0, 0.0, 0.0);
  for (int i = 0; i <= num_faces; ++i) {
    float angulo = i * 2 * M_PI / num_faces;
    glVertex3f(cosf(angulo) * raio, sinf(angulo) * raio, 0.0f);
  }
  glEnd();
  glDisable(GL_POLYGON_OFFSET_FILL);
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
Entidade* NovaEntidade(TipoEntidade tipo, Texturas* texturas, ntf::CentralNotificacoes* central) {
  switch (tipo) {
    case TE_ENTIDADE:
      return new Entidade(texturas, central);
    default:
      LOG(ERROR) << "Tipo de entidade inválido: " << tipo;
      return nullptr;
  }
}

// Entidade
Entidade::Entidade(Texturas* texturas, ntf::CentralNotificacoes* central) {
  proto_.set_tipo(TE_ENTIDADE);
  rotacao_disco_selecao_ = 0;
  texturas_ = texturas;
  central_ = central;
}

Entidade::~Entidade() {
  if (proto_.has_textura()) {
    VLOG(1) << "Liberando textura: " << proto_.textura();
    auto* nl = ntf::NovaNotificacao(ntf::TN_LIBERAR_TEXTURA);
    nl->set_endereco(proto_.textura());
    central_->AdicionaNotificacao(nl);
  }
}

void Entidade::Inicializa(const EntidadeProto& novo_proto) { 
  // Atualiza texturas antes de tudo.
  AtualizaTexturas(novo_proto);
  // mantem o tipo.
  TipoEntidade tipo = proto_.tipo();
  proto_.CopyFrom(novo_proto);
  proto_.set_tipo(tipo);
}

void Entidade::AtualizaTexturas(const EntidadeProto& novo_proto) { 
  VLOG(2) << "Novo proto: " << novo_proto.ShortDebugString() << ", velho: " << proto_.ShortDebugString();
  // Libera textura anterior se houver e for diferente da corrente.
  if (proto_.has_textura() && proto_.textura() != novo_proto.textura()) {
    VLOG(1) << "Liberando textura: " << proto_.textura();
    auto* nl = ntf::NovaNotificacao(ntf::TN_LIBERAR_TEXTURA);
    nl->set_endereco(proto_.textura());
    central_->AdicionaNotificacao(nl);
  }
  // Carrega textura se houver e for diferente da antiga.
  if (novo_proto.has_textura() && novo_proto.textura() != proto_.textura()) {
    VLOG(1) << "Carregando textura: " << proto_.textura();
    auto* nc = ntf::NovaNotificacao(ntf::TN_CARREGAR_TEXTURA);
    nc->set_endereco(novo_proto.textura());
    central_->AdicionaNotificacao(nc);
  }
  if (novo_proto.has_textura()) {
    proto_.set_textura(novo_proto.textura());
  } else {
    proto_.clear_textura();
  }
}

void Entidade::Atualiza(const EntidadeProto& novo_proto) { 
  AtualizaTexturas(novo_proto);

  // mantem o tipo.
  ent::EntidadeProto copia_proto(proto_);
  proto_.CopyFrom(novo_proto);
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
	glLoadName(Id());
  // Tem que normalizar por causa das operacoes de escala, que afetam as normais.
  glEnable(GL_NORMALIZE);
  MudaCor(proto_.cor());
  float multiplicador = CalculaMultiplicador(proto_.tamanho());
  glScalef(multiplicador, multiplicador, multiplicador);
  // Aplica uma pequena diminuição para não ocupar o quadrado todo.
  if (proto_.has_textura()) {
    // Constroi a moldura e aplica a textura.
    // tijolo da base (altura TAMANHO_LADO_QUADRADO / 10).
    glPushMatrix();
    glScalef(0.8f, 0.8f, 0.1f);
    glutSolidCube(TAMANHO_LADO_QUADRADO);
    glPopMatrix();
    // Moldura da textura: achatado em Y.
    glPushMatrix();
    glTranslated(0, 0, TAMANHO_LADO_QUADRADO_2 + (TAMANHO_LADO_QUADRADO / 10.0f));
    glScalef(1.0f, 0.1f, 1.0f);
    glutSolidCube(TAMANHO_LADO_QUADRADO);
    glPopMatrix();
    // desenha a tela onde a textura será desenhada face para o sul.
    const InfoTextura* info = texturas_->Textura(proto_.textura());
    if (info != nullptr) {
      if (pd->desenha_texturas()) {
        glEnable(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,
                     0, GL_RGBA,
                     info->largura, info->altura,
                     0, GL_BGRA, GL_UNSIGNED_BYTE,
                     info->dados);
      }
      glNormal3f(0.0f, -1.0f, 0.0f);
      glPushMatrix();
      glTranslated(0, 0, TAMANHO_LADO_QUADRADO / 10.0f);
      MudaCor(1.0f, 1.0f, 1.0f, 1.0f);
      glBegin(GL_QUADS);
      // OpenGL espera a imagem vinda de baixo,esquerda para cima,direita. Como o carregador
      // carrega invertido, fazemos o desenho de cabeca para baixo.
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(
          -TAMANHO_LADO_QUADRADO_2, -TAMANHO_LADO_QUADRADO_2 / 10.0f - 0.1f, 0.0f);
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(
          TAMANHO_LADO_QUADRADO_2, -TAMANHO_LADO_QUADRADO_2 / 10.0f - 0.1f, 0.0f);
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f(
          TAMANHO_LADO_QUADRADO_2, -TAMANHO_LADO_QUADRADO_2 / 10.0f - 0.1f, TAMANHO_LADO_QUADRADO);
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(
          -TAMANHO_LADO_QUADRADO_2, -TAMANHO_LADO_QUADRADO_2 / 10.0f - 0.1f, TAMANHO_LADO_QUADRADO);
      glEnd();
      glPopMatrix();
      glDisable(GL_TEXTURE_2D);
    }
  } else {
    glutSolidCone(TAMANHO_LADO_QUADRADO_2 - 0.2, ALTURA, NUM_FACES, NUM_LINHAS);
    glPushMatrix();
	  glTranslated(0, 0, ALTURA);
	  glutSolidSphere(TAMANHO_LADO_QUADRADO_2 - 0.4, NUM_FACES, NUM_FACES);
    glPopMatrix();
  }

  if (pd->entidade_selecionada()) {
    MudaCor(proto_.cor());
    glRotatef(rotacao_disco_selecao_, 0, 0, 1.0f);
    DesenhaDisco(TAMANHO_LADO_QUADRADO_2, 6);
  }
	
	glPopMatrix();
  glDisable(GL_NORMALIZE);
}

void Entidade::DesenhaLuz(ParametrosDesenho* pd) {
  if (!pd->iluminacao() || !proto_.has_luz()) {
    return;
  }

  // Objeto de luz. O quarto componente indica que a luz é posicional.
  // Se for 0, a luz é direcional e os componentes indicam sua direção.
  GLfloat pos_luz[] = {
      0, 0, static_cast<GLfloat>(ALTURA * CalculaMultiplicador(proto_.tamanho())), 1.0f };
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
    glLightf(GL_LIGHT0 + id_luz, GL_CONSTANT_ATTENUATION, 0.5f);
    //glLightf(GL_LIGHT0 + id_luz, GL_LINEAR_ATTENUATION, -0.53f);
    glLightf(GL_LIGHT0 + id_luz, GL_QUADRATIC_ATTENUATION, 0.02f);
    glEnable(GL_LIGHT0 + id_luz);
    pd->set_luz_corrente(id_luz + 1);
  }
  glPopMatrix();
}

const EntidadeProto& Entidade::Proto() const {
  return proto_;
}

}  // namespace ent
