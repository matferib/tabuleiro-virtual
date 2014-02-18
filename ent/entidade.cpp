#include <cmath>
#include <stdexcept>
#if __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ent/tabuleiro.pb.h"
#include "ifg/qt/texturas.h"
#include "log/log.h"

namespace ent {
namespace {

const unsigned int NUM_FACES = 10;
const unsigned int NUM_LINHAS = 1;
const float ALTURA = TAMANHO_LADO_QUADRADO;
const float ALTURA_VOO = ALTURA;
// deslocamento em cada eixo (x, y, z) por chamada de atualizacao.
const double VELOCIDADE_POR_EIXO = 0.1;
// Tamanho da barra de vida.
const double TAMANHO_BARRA_VIDA = TAMANHO_LADO_QUADRADO_2;
const double TAMANHO_BARRA_VIDA_2 = TAMANHO_BARRA_VIDA / 2.0f;

// Placeholder para retornar a altura do chao em determinado ponto do tabuleiro.
float ZChao(float x3d, float y3d) {
  return 0;
}

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
  //glEnable(GL_POLYGON_OFFSET_FILL);
  //glPolygonOffset(1.0f, 0.0f);
  glNormal3f(0, 0, 1.0f);
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(0.0, 0.0, 0.0);
  for (int i = 0; i <= num_faces; ++i) {
    float angulo = i * 2 * M_PI / num_faces;
    glVertex3f(cosf(angulo) * raio, sinf(angulo) * raio, 0.0f);
  }
  glEnd();
  //glDisable(GL_POLYGON_OFFSET_FILL);
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

// Multiplica a matriz openGL matriz pelo vetor. A matriz OpenGL tem formato col x linha (column major), portanto,
// ao inves de multiplicar matriz (4x4) pelo vetor (4x1), fazemos a inversao: vetor (1x4) pela matriz (4x4).
void MultiplicaMatrizVetor(const GLfloat* matriz, GLfloat* vetor) {
  GLfloat res[4];
  for (int i = 0; i < 4; ++i) {
    res[i] = vetor[0] * matriz[i] + 
             vetor[1] * matriz[i + 4] + 
             vetor[2] * matriz[i + 8] + 
             vetor[3] * matriz[i + 12]; 
  }
  vetor[0] = res[0];
  vetor[1] = res[1];
  vetor[2] = res[2];
  vetor[3] = res[3];
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

void PreencheEntidadeProto(int id_cliente, int id_entidade, bool visivel, EntidadeProto* modelo) {
  modelo->set_id((id_cliente << 28) | id_entidade);
  modelo->set_visivel(visivel);
}


// Entidade
Entidade::Entidade(Texturas* texturas, ntf::CentralNotificacoes* central) {
  proto_.set_tipo(TE_ENTIDADE);
  rotacao_disco_selecao_graus_ = 0;
  angulo_disco_voo_rad_ = 0;
  angulo_disco_queda_graus_ = 0;
  texturas_ = texturas;
  central_ = central;
}

Entidade::~Entidade() {
  if (proto_.has_info_textura()) {
    VLOG(1) << "Liberando textura: " << proto_.info_textura().id();
    auto* nl = ntf::NovaNotificacao(ntf::TN_DESCARREGAR_TEXTURA);
    nl->mutable_info_textura()->set_id(proto_.info_textura().id());
    central_->AdicionaNotificacao(nl);
  }
}

void Entidade::Inicializa(const EntidadeProto& novo_proto) {
  // Atualiza texturas antes de tudo.
  AtualizaTexturas(novo_proto);
  // mantem o tipo.
  TipoEntidade tipo = proto_.tipo();
  proto_.CopyFrom(novo_proto);
  if (!proto_.has_pontos_vida() || proto_.pontos_vida() > proto_.max_pontos_vida()) {
    proto_.set_pontos_vida(proto_.max_pontos_vida());
  }
  proto_.set_tipo(tipo);
}

void Entidade::AtualizaTexturas(const EntidadeProto& novo_proto) {
  VLOG(2) << "Novo proto: " << novo_proto.ShortDebugString() << ", velho: " << proto_.ShortDebugString();
  // Libera textura anterior se houver e for diferente da corrente.
  if (proto_.has_info_textura() && proto_.info_textura().id() != novo_proto.info_textura().id()) {
    VLOG(1) << "Liberando textura: " << proto_.info_textura().id();
    auto* nl = ntf::NovaNotificacao(ntf::TN_DESCARREGAR_TEXTURA);
    nl->mutable_info_textura()->set_id(proto_.info_textura().id());
    central_->AdicionaNotificacao(nl);
  }
  // Carrega textura se houver e for diferente da antiga.
  if (novo_proto.has_info_textura() && novo_proto.info_textura().id() != proto_.info_textura().id()) {
    VLOG(1) << "Carregando textura: " << proto_.info_textura().id();
    auto* nc = ntf::NovaNotificacao(ntf::TN_CARREGAR_TEXTURA);
    nc->mutable_info_textura()->CopyFrom(novo_proto.info_textura());
    central_->AdicionaNotificacao(nc);
  }
  if (novo_proto.has_info_textura()) {
    proto_.mutable_info_textura()->CopyFrom(novo_proto.info_textura());
  } else {
    proto_.clear_info_textura();
  }
}

void Entidade::AtualizaProto(const EntidadeProto& novo_proto) {
  AtualizaTexturas(novo_proto);

  // mantem o tipo.
  ent::EntidadeProto copia_proto(proto_);
  proto_.CopyFrom(novo_proto);
  if (proto_.pontos_vida() > proto_.max_pontos_vida()) {
    proto_.set_pontos_vida(proto_.max_pontos_vida());
  }
  proto_.set_id(copia_proto.id());
  proto_.mutable_pos()->Swap(copia_proto.mutable_pos());
  if (copia_proto.has_destino()) {
    proto_.mutable_destino()->Swap(copia_proto.mutable_destino());
  }
  proto_.mutable_pos()->set_z(novo_proto.pos().z());
  VLOG(1) << "Proto: " << proto_.ShortDebugString();
}

void Entidade::Atualiza() {
  auto* po = proto_.mutable_pos();
  rotacao_disco_selecao_graus_ = fmod(rotacao_disco_selecao_graus_ + 1.0, 360.0);
  float z_chao = ZChao(X(), Y());
  if (proto_.voadora()) {
    if (Z() < z_chao + ALTURA_VOO) {
      po->set_z(po->z() + 0.03f);
    }
    angulo_disco_voo_rad_ = fmod(angulo_disco_voo_rad_ + 0.01, 2 * M_PI);
  } else {
    if (Z() > z_chao) {
      po->set_z(po->z() - 0.03f);
    }
    angulo_disco_voo_rad_ = 0.0f;
  }
  if (proto_.caida()) {
    if (angulo_disco_queda_graus_ < 90.0f) {
      angulo_disco_queda_graus_ += 1.0f;
    }
  } else {
    if (angulo_disco_queda_graus_ > 0) {
      angulo_disco_queda_graus_ -= 1.0f;
    }
  }
  // Nunca fica abaixo do solo.
  if (Z() < z_chao) {
    po->set_z(z_chao);
  }

  if (!proto_.has_destino()) {
    return;
  }
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

void Entidade::MoveDelta(double dx, double dy, double dz) {
  auto* p = proto_.mutable_pos();
  p->set_x(p->x() + dx);
  p->set_y(p->y() + dy);
  p->set_z(p->z() + dz);
  proto_.clear_destino();
}

void Entidade::Destino(const EntidadeProto& proto) {
  proto_.mutable_destino()->CopyFrom(proto.destino());
}

int Entidade::PontosVida() const {
  return proto_.pontos_vida();
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

void Entidade::MataEntidade() {
  proto_.set_morta(true);
  proto_.set_caida(true);
  proto_.set_voadora(false);
  proto_.set_aura(0);
}

const Posicao Entidade::PosicaoAcao() const {
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  MontaMatriz(true, nullptr);
  glTranslatef(0.0f, 0.0f, ALTURA);
  GLfloat matriz[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, matriz);
  VLOG(2) << "Matriz: " << matriz[0] << " " << matriz[1] << " " << matriz[2] << " " << matriz[3];
  VLOG(2) << "Matriz: " << matriz[4] << " " << matriz[5] << " " << matriz[6] << " " << matriz[7];
  VLOG(2) << "Matriz: " << matriz[8] << " " << matriz[9] << " " << matriz[10] << " " << matriz[11];
  VLOG(2) << "Matriz: " << matriz[12] << " " << matriz[13] << " " << matriz[14] << " " << matriz[15];
  GLfloat ponto[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  MultiplicaMatrizVetor(matriz, ponto);
  VLOG(2) << "Ponto: " << ponto[0] << " " << ponto[1] << " " << ponto[2] << " " << ponto[3];
  Posicao pos;
  pos.set_x(ponto[0]);
  pos.set_y(ponto[1]);
  pos.set_z(ponto[2]);
  glPopMatrix();
  return pos;
}

float Entidade::DeltaVoo() const {
  return angulo_disco_voo_rad_ > 0 ? sinf(angulo_disco_voo_rad_) * TAMANHO_LADO_QUADRADO_2 : 0.0f;
}

void Entidade::MontaMatriz(bool usar_delta_voo, const float* matriz_shear) const {
  if (matriz_shear == nullptr) {
    glTranslated(X(), Y(), usar_delta_voo ? Z() + DeltaVoo() : 0.0);
  } else {
    glTranslated(X(), Y(), 0);
    glMultMatrixf(matriz_shear);
    glTranslated(0, 0, usar_delta_voo ? Z() + DeltaVoo() : 0.0);
  }
  if (angulo_disco_queda_graus_ > 0) {
    //glTranslated(0, -TAMANHO_LADO_QUADRADO_2, 0);
    glRotatef(-angulo_disco_queda_graus_, 1.0, 0, 0);
  }
  float multiplicador = CalculaMultiplicador(proto_.tamanho());
  glScalef(multiplicador, multiplicador, multiplicador);
}

void Entidade::Desenha(ParametrosDesenho* pd) {
  if (!proto_.visivel()) {
    // Sera desenhado translucido para o mestre.
    return;
  }
  // desenha o cone com NUM_FACES faces com raio de RAIO e altura ALTURA
  MudaCor(proto_.cor());
  DesenhaObjetoComDecoracoes(pd);
}

void Entidade::DesenhaTranslucido(ParametrosDesenho* pd) {
  if (proto_.visivel() || !pd->modo_mestre()) {
    // Um pouco diferente, pois so desenha se for visivel.
    return;
  }
  // desenha o cone com NUM_FACES faces com raio de RAIO e altura ALTURA
  const auto& cor = proto_.cor();
  MudaCor(cor.r(), cor.g(), cor.b(), cor.a() * pd->alpha_translucidos());
  DesenhaObjetoComDecoracoes(pd);
}

void Entidade::DesenhaObjetoComDecoracoes(ParametrosDesenho* pd) {
  glLoadName(Id());
  // Tem que normalizar por causa das operacoes de escala, que afetam as normais.
  glEnable(GL_NORMALIZE);
  DesenhaObjeto(pd);
  if (!proto_.has_info_textura() && pd->entidade_selecionada()) {
    // Volta pro chao.
    glPushMatrix();
    MontaMatriz(false);
    MudaCor(proto_.cor());
    glRotatef(rotacao_disco_selecao_graus_, 0, 0, 1.0f);
    DesenhaDisco(TAMANHO_LADO_QUADRADO_2, 6);
    glPopMatrix();
  }
  // Desenha a barra de vida.
  if (pd->desenha_barra_vida()) {
    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT);
    // Luz no olho apontando para a barra.
    const Posicao& pos_olho = pd->pos_olho();
    glLightfv(GL_LIGHT0, GL_DIFFUSE, COR_BRANCA);
    const auto& pos = proto_.pos();
    GLfloat pos_luz[] = { pos_olho.x() - pos.x(), pos_olho.y() - pos.y(), pos_olho.z() - pos.z(), 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, pos_luz);

    glPushMatrix();
    MontaMatriz(true);
    glTranslatef(0.0f, 0.0f, ALTURA * 1.5f);
    glPushMatrix();
    glScalef(0.2, 0.2, 1.0f);
    MudaCor(COR_VERMELHA);
    glutSolidCube(TAMANHO_BARRA_VIDA);
    glPopMatrix();
    if (proto_.max_pontos_vida() > 0 && proto_.pontos_vida() > 0) {
      float porcentagem = static_cast<float>(proto_.pontos_vida()) / proto_.max_pontos_vida();
      float tamanho_barra = TAMANHO_BARRA_VIDA * porcentagem;
      float delta = -TAMANHO_BARRA_VIDA_2 + (tamanho_barra / 2.0f);
      glTranslatef(0, 0, delta);
      glScalef(0.3f, 0.3f, porcentagem);
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(0, -25.0);
      MudaCor(COR_VERDE);
      glutSolidCube(TAMANHO_BARRA_VIDA);
    }
    glPopMatrix();
    glPopAttrib();
  }
  glDisable(GL_NORMALIZE);
}

void Entidade::DesenhaObjeto(ParametrosDesenho* pd, const float* matriz_shear) {
  if (proto_.has_info_textura()) {
    // tijolo da base (altura TAMANHO_LADO_QUADRADO_10).
    {
      glPushMatrix();
      MontaMatriz(false, matriz_shear);
      glTranslated(0.0, 0.0, TAMANHO_LADO_QUADRADO_10 / 2);
      glScalef(0.8f, 0.8f, TAMANHO_LADO_QUADRADO_10 / 2);
      if (pd->entidade_selecionada()) {
        glRotatef(rotacao_disco_selecao_graus_, 0, 0, 1.0f);
      }
      glutSolidCube(TAMANHO_LADO_QUADRADO);
      glPopMatrix();
    }
    // Moldura da textura: achatado em Y.
    glPushMatrix();
    MontaMatriz(true, matriz_shear);
    glTranslated(0, 0, TAMANHO_LADO_QUADRADO_2 + (TAMANHO_LADO_QUADRADO / 10.0));
    double angulo = 0;
    // So desenha a textura de frente pra entidades nao caidas.
    if (pd->texturas_sempre_de_frente() && !proto_.caida()) {
      double dx = X() - pd->pos_olho().x();
      double dy = Y() - pd->pos_olho().y();
      double r = sqrt(pow(dx, 2) + pow(dy, 2));
      angulo = (acos(dx / r) * RAD_PARA_GRAUS);
      if (dy < 0) {
        // A funcao asin tem dois resultados mas sempre retorna o positivo [0, PI].
        // Se o vetor estiver nos quadrantes de baixo, inverte o angulo.
        angulo = -angulo;
      }
      glRotated(angulo - 90.0f, 0, 0, 1.0);
    }
    glPushMatrix();
    glScalef(1.0f, 0.1f, 1.0f);
    glutSolidCube(TAMANHO_LADO_QUADRADO);
    glPopMatrix();
    // desenha a tela onde a textura será desenhada face para o sul.
    GLuint id_textura = pd->desenha_texturas() && proto_.has_info_textura() ?
        texturas_->Textura(proto_.info_textura().id()) : GL_INVALID_VALUE;
    if (id_textura != GL_INVALID_VALUE) {
      if (matriz_shear != nullptr) {
        glMultMatrixf(matriz_shear);
      }
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, id_textura);
      glNormal3f(0.0f, -1.0f, 0.0f);
      MudaCor(1.0f, 1.0f, 1.0f, pd->has_alpha_translucidos() ? pd->alpha_translucidos() : 1.0f);
      glBegin(GL_QUADS);
      // O openGL assume que o (0.0, 0.0) da textura eh embaixo,esquerda. O QT retorna os dados da
      // imagem com origem em cima esquerda. Entao a gente mapeia a textura com o eixo vertical invertido.
      // O quadrado eh desenhado EB, DB, DC, EC. A textura eh aplicada: EC, DC, DB, EB.
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(
          -TAMANHO_LADO_QUADRADO_2, -TAMANHO_LADO_QUADRADO_2 / 10.0f - 0.01f, -TAMANHO_LADO_QUADRADO_2);
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f(
          TAMANHO_LADO_QUADRADO_2, -TAMANHO_LADO_QUADRADO_2 / 10.0f - 0.01f, -TAMANHO_LADO_QUADRADO_2);
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(
          TAMANHO_LADO_QUADRADO_2, -TAMANHO_LADO_QUADRADO_2 / 10.0f - 0.01f, TAMANHO_LADO_QUADRADO_2);
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(
          -TAMANHO_LADO_QUADRADO_2, -TAMANHO_LADO_QUADRADO_2 / 10.0f - 0.01f, TAMANHO_LADO_QUADRADO_2);
      glEnd();
      glDisable(GL_TEXTURE_2D);
    }
    glPopMatrix();
  } else {
    glPushMatrix();
    MontaMatriz(true, matriz_shear);
    glutSolidCone(TAMANHO_LADO_QUADRADO_2 - 0.2, ALTURA, NUM_FACES, NUM_LINHAS);
	  glTranslated(0, 0, ALTURA);
	  glutSolidSphere(TAMANHO_LADO_QUADRADO_2 - 0.4, NUM_FACES, NUM_FACES);
    glPopMatrix();
  }
}

void Entidade::DesenhaLuz(ParametrosDesenho* pd) {
  if (!pd->iluminacao() || !proto_.has_luz()) {
    return;
  }
  if (!proto_.visivel() && !pd->modo_mestre()) {
    return;
  }

  glPushMatrix();
  MontaMatriz(true  /*usar_delta_voo*/);
  // Um pouco acima do objeto e ao sul do objeto.
  glTranslated(0, -0.2f, ALTURA + TAMANHO_LADO_QUADRADO_2);
  int id_luz = pd->luz_corrente();
  if (id_luz == 0 || id_luz >= pd->max_num_luzes()) {
    LOG(ERROR) << "Limite de luzes alcançado: " << id_luz;
  } else {
    // Objeto de luz. O quarto componente indica que a luz é posicional.
    // Se for 0, a luz é direcional e os componentes indicam sua direção.
    GLfloat pos_luz[] = { 0, 0, 0, 1.0f };
    glLightfv(GL_LIGHT0 + id_luz, GL_POSITION, pos_luz);
    const ent::Cor& cor = proto_.luz().cor();
    GLfloat cor_luz[] = { cor.r(), cor.g(), cor.b(), cor.a() };
    glLightfv(GL_LIGHT0 + id_luz, GL_DIFFUSE, cor_luz);
    glLightf(GL_LIGHT0 + id_luz, GL_CONSTANT_ATTENUATION, 0.5f);
    //glLightf(GL_LIGHT0 + id_luz, GL_LINEAR_ATTENUATION, -0.53f);
    glLightf(GL_LIGHT0 + id_luz, GL_QUADRATIC_ATTENUATION, 0.02f);
    glEnable(GL_LIGHT0 + id_luz);
    pd->set_luz_corrente(id_luz + 1);
  }
  glPopMatrix();
}

void Entidade::DesenhaAura(ParametrosDesenho* pd) {
  if (!proto_.visivel() && !pd->modo_mestre()) {
    return;
  }
  if (!pd->desenha_aura() || !proto_.has_aura() || proto_.aura() == 0) {
    return;
  }
  glPushMatrix();
  glTranslated(X(), Y(), Z() + DeltaVoo());
  const auto& cor = proto_.cor();
  MudaCor(cor.r(), cor.g(), cor.b(), cor.a() * 0.2f);
  float ent_quadrados = CalculaMultiplicador(proto_.tamanho());
  if (ent_quadrados < 1.0f) {
    ent_quadrados = 1.0f;
  }
  // A aura estende alem do tamanho da entidade.
  glutSolidSphere(
      TAMANHO_LADO_QUADRADO_2 * ent_quadrados + TAMANHO_LADO_QUADRADO * proto_.aura(),
      NUM_FACES, NUM_FACES);
  glPopMatrix();
}

void Entidade::DesenhaSombra(ParametrosDesenho* pd, const float* matriz_shear) {
  if (!proto_.visivel() && !pd->modo_mestre()) {
    return;
  }
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(-1.0f, -10.0f);
  DesenhaObjeto(pd, matriz_shear);
  glDisable(GL_POLYGON_OFFSET_FILL);
}

const EntidadeProto& Entidade::Proto() const {
  return proto_;
}

}  // namespace ent
