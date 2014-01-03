#include <assert.h>
#include <vector>
#include <map>
#include <stdexcept>
#include <cmath>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ent/tabuleiro.pb.h"
#include "ntf/notificacao.pb.h"

using namespace ent;
using namespace std;

namespace {
/** campo de visao vertical em graus. */
const double CAMPO_VERTICAL = 60.0;

/** altura inicial do olho. */
const double OLHO_ALTURA_INICIAL = 10.0;
/** altura maxima do olho. */
const double OLHO_ALTURA_MAXIMA = 15;
/** altura minima do olho. */
const double OLHO_ALTURA_MINIMA = 1.5;

/** raio (distancia) inicial do olho. */
const double OLHO_RAIO_INICIAL = 20.0;
/** raio maximo do olho. */
const double OLHO_RAIO_MAXIMO = 40.0;
/** raio minimo do olho. */
const double OLHO_RAIO_MINIMO = 1.5;

/** sensibilidade da rodela do mouse. */
const double SENSIBILIDADE_RODA = 0.01;
/** sensibilidade da rotacao lateral do olho. */
const double SENSIBILIDADE_ROTACAO_X = 0.01;
/** sensibilidade da altura do olho. */
const double SENSIBILIDADE_ROTACAO_Y = 0.08;

/** tamanho do lado do quadrado no 3D. */
const double TAMANHO_GL = 1.5;
/** expessura da linha do tabuleiro. */
const double EXPESSURA_LINHA = 0.1;
/** tamanho do lado do quadrado / 2. */
const double TAMANHO_GL_2 = (TAMANHO_GL / 2.0);

/** Altera a cor correnta para cor. */
void Cor(GLfloat* cor) {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cor);
  glColor3fv(cor);
}

/** desenha o quadrado, embaixo preto, acima cinza com pequeno offset para evitar z fight. */
void DesenhaQuadrado(GLuint id, bool selecionado) {
  // desenha o quadrado negro embaixo.
  glLoadName(id);
  GLfloat preto[] = { 0, 0, 0, 1.0 };
  Cor(preto);
  glRectf(0, 0, TAMANHO_GL, TAMANHO_GL);

  // Habilita a função pra acabar com zfight.
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(-0.1f, -0.1f);
  if (selecionado) {
    GLfloat cinza[] = { 0.5, 0.5, 0.5, 1.0 };
    Cor(cinza);
  } else {
    GLfloat cinza_claro[] = { 0.8, 0.8, 0.8, 1.0 };
    Cor(cinza_claro);
  }
  glRectf(0, 0, TAMANHO_GL - EXPESSURA_LINHA, TAMANHO_GL - EXPESSURA_LINHA);
  // Restaura os offset de zfight.
  glDisable(GL_POLYGON_OFFSET_FILL);
}

// Desenha uma bola amarela.
void DesenhaLuz() {
  glColor3ub(255, 255, 0);
	glutSolidSphere(0.3, 3, 3);
}

}  // namespace.

Tabuleiro::Tabuleiro(int tamanho, ntf::CentralNotificacoes* central) : 
    tamanho_(tamanho), 
    entidade_selecionada_(NULL), 
    quadrado_selecionado_(-1), 
    estado_(ETAB_OCIOSO), proximo_id_(0),
    olho_x_(0), olho_y_(0), olho_z_(0), olho_delta_rotacao_(0), olho_altura_(OLHO_ALTURA_INICIAL), olho_raio_(OLHO_RAIO_INICIAL),
    central_(central) {
  parametros_desenho_.desenha_entidades = true;
  parametros_desenho_.iluminacao = true;
  parametros_desenho_.desenha_luz = true;
  central_->RegistraReceptor(this);
}

Tabuleiro::~Tabuleiro() {
}

int Tabuleiro::TamanhoX() const { 
  return tamanho_; 
}

int Tabuleiro::TamanhoY() const { 
  return tamanho_; 
}

void Tabuleiro::Desenha() {
  DesenhaCena();
}

int Tabuleiro::AdicionaEntidade(int id_quadrado) {
  double x, y, z;
  CoordenadaQuadrado(id_quadrado, &x, &y, &z);
  auto* entidade = new Entidade(proximo_id_++, 0, x, y, z);
  entidades_.insert(make_pair(entidade->Id(), entidade));
  return entidade->Id();
}

void Tabuleiro::RemoveEntidade(int id) {
  MapaEntidades::iterator res_find = entidades_.find(id);
  if (res_find == entidades_.end()) {
    return;
  }
  entidades_.erase(res_find);
  Entidade* entidade = res_find->second;
  if (entidade_selecionada_ == entidade) {
    entidade_selecionada_ = NULL;
  }
  delete entidade;
}

bool Tabuleiro::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_ADICIONAR_ENTIDADE:
      if (estado_ == ETAB_QUAD_SELECIONADO) {
        // Adiciona entidade.
        SelecionaEntidade(AdicionaEntidade(quadrado_selecionado_));
        estado_ = ETAB_ENT_SELECIONADA;
        ntf::Notificacao* n = new ntf::Notificacao;
        n->set_tipo(ntf::TN_ENTIDADE_ADICIONADA);
        central_->AdicionaNotificacao(n);
      }
      return true;
    case ntf::TN_REMOVER_ENTIDADE:
      if (estado_ == ETAB_ENT_SELECIONADA) {
        // Remover entidade.
        RemoveEntidade(entidade_selecionada_->Id());
        estado_ = ETAB_OCIOSO;
        ntf::Notificacao* n = new ntf::Notificacao;
        n->set_tipo(ntf::TN_ENTIDADE_REMOVIDA);
        central_->AdicionaNotificacao(n);
      }
      return true;
    case ntf::TN_CLIENTE_PENDENTE:
      central_->AdicionaNotificacao(CriaNotificacaoTabuleiro());
      return true;
    case ntf::TN_TABULEIRO:
      RecebeNotificacaoTabuleiro(notificacao);
      return true;
    default:
      return false;
  }
}

void Tabuleiro::TrataRodela(int delta) {
  // move o olho no eixo Z de acordo com o eixo Y do movimento
  olho_raio_ -= (delta * SENSIBILIDADE_RODA); 
  if (olho_raio_ < OLHO_RAIO_MINIMO) {
    olho_raio_ = OLHO_RAIO_MINIMO;
  }
  else if (olho_raio_ > OLHO_RAIO_MAXIMO) {
    olho_raio_ = OLHO_RAIO_MAXIMO;
  }
}

// Coordenadas em OpenGL (origem canto inferior esquerdo).
void Tabuleiro::TrataMovimento(int x, int y) {
  if (estado_ == ETAB_ROTACAO) {
    // Realiza a rotacao da tela.
    olho_delta_rotacao_ -= (x - rotacao_ultimo_x_) * SENSIBILIDADE_ROTACAO_X;
    if (olho_delta_rotacao_ >= 2*M_PI) {
      olho_delta_rotacao_ -= 2*M_PI;
    }
    else if (olho_delta_rotacao_ <= -2*M_PI) {
      olho_delta_rotacao_ += 2*M_PI;
    }
    // move o olho no eixo Z de acordo com o eixo Y do movimento
    olho_altura_ -= (y - rotacao_ultimo_y_) * SENSIBILIDADE_ROTACAO_Y; 
    if (olho_altura_ < OLHO_ALTURA_MINIMA) {
      olho_altura_ = OLHO_ALTURA_MINIMA;
    }
    else if (olho_altura_ > OLHO_ALTURA_MAXIMA) {
      olho_altura_ = OLHO_ALTURA_MAXIMA;
    }

    rotacao_ultimo_x_ = x;
    rotacao_ultimo_y_ = y;
  } else if (estado_ == ETAB_ENT_PRESSIONADA) {
    // Realiza o movimento da entidade.
    // Transforma x e y em 3D, baseado no nivel do solo.
    bool desenha_entidades = parametros_desenho_.desenha_entidades;
    bool desenha_luz = parametros_desenho_.desenha_luz;
    parametros_desenho_.desenha_entidades = false;
    parametros_desenho_.desenha_luz = false;
    DesenhaCena();  // Sem as entidades pra pegar nivel solo.
    parametros_desenho_.desenha_entidades = desenha_entidades;
    parametros_desenho_.desenha_luz = desenha_luz;
    GLdouble modelview[16], projection[16];
    GLint viewport[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);
    GLdouble nx, ny, nz;
    GLfloat win_z;
    glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &win_z);
    //cout << "WX: " << x << ", WY: " << y << ", WZ: " << win_z 
    //     << ", viewport: " << viewport[0] << " " << viewport[1] << " " << viewport[2] << " " << viewport[3] << endl;
    if (!gluUnProject(x, y, win_z, modelview, projection, viewport, &nx, &ny, &nz)) {
      return;
    }
    //cout << "3x: " << nx << ", 3y: " << ny << ", 3z: " << nz << endl; 
    entidade_selecionada_->MovePara(nx, ny, 0);
  }
}

void Tabuleiro::TrataBotaoPressionado(botao_e botao, int x, int y, double aspecto) {
  if (botao == BOTAO_MEIO) {
    rotacao_ultimo_x_ = x;
    rotacao_ultimo_y_ = y;
    estado_anterior_rotacao_ = estado_;
    estado_ = ETAB_ROTACAO;
  } else if (botao == BOTAO_ESQUERDO) {
    // informacao dos hits. TODO ver esse limite aqui.
    GLuint buffer_hits[100] = {0};
    GLuint numero_hits = 0;
    EncontraHits(x, y, aspecto, &numero_hits, buffer_hits);
    TrataClique(numero_hits, buffer_hits);
  }
}

void Tabuleiro::TrataBotaoLiberado() {
  switch (estado_) {
    case ETAB_ROTACAO:
      estado_ = estado_anterior_rotacao_;
      return;
    case ETAB_ENT_PRESSIONADA: {
      auto* n = new ntf::Notificacao;
      n->set_local(false);
      n->set_remota(true);
      central_->AdicionaNotificacao(n);
      estado_ = ETAB_ENT_SELECIONADA;
      return;
    }
    case ETAB_QUAD_PRESSIONADO:
      estado_ = ETAB_QUAD_SELECIONADO;
      return;
    default:
      ;
  }
  estado_ = ETAB_OCIOSO;
}


void Tabuleiro::TrataRedimensionaJanela(int largura, int altura) {
  glViewport(0, 0, (GLint)largura, (GLint)altura);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(CAMPO_VERTICAL, (double)largura / altura, 0.5, 500.0);
}

void Tabuleiro::InicializaGL() {
  glClearColor(1.0, 1.0, 1.0, 1.0);

  // back face
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // zbuffer
  glEnable(GL_DEPTH_TEST);

  // Iluminação.
  glEnable(GL_LIGHTING);
}


// privadas 

void Tabuleiro::DesenhaCena() {
  glClear(GL_COLOR_BUFFER_BIT);
  glClear(GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(
    // from
    olho_x_ + cos(olho_delta_rotacao_) * olho_raio_, 
    olho_y_ + sin(olho_delta_rotacao_) * olho_raio_, 
    olho_altura_,
    // to
    olho_x_, olho_y_, olho_z_,
    // up
    0, 0, 1.0);

  //ceu_.desenha(parametros_desenho_);

  // Iluminação junto ao olho. O quarto componente indica que a luz é posicional. Se for 0, a luz é direcional e os componentes
  // indicam sua direção.
  GLfloat pos_luz[] = { 0.0f, 0.0f, 1.0f, 1.0f };
  if (parametros_desenho_.desenha_luz) {
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslated(pos_luz[0], pos_luz[1], pos_luz[2]);
    DesenhaLuz();
    glPopMatrix();
    if (parametros_desenho_.iluminacao) {
      glEnable(GL_LIGHTING);
    }
  }
  glLightfv(GL_LIGHT0, GL_POSITION, pos_luz);
  glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.1);
  //GLfloat cor_luz[] = { 1.0, 1.0, 1.0, 1.0 };
  if (parametros_desenho_.iluminacao) {
    glEnable(GL_LIGHT0);
  }

  //GLfloat ambient[] = { 1.0, 1.0, 1.0, 1.0 };
  //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

  // desenha tabuleiro de baixo pra cima
  glPushMatrix();
  double deltaX = -TamanhoX() * TAMANHO_GL;
  double deltaY = -TamanhoY() * TAMANHO_GL;
  glNormal3f(0, 0, 1.0f);
  glTranslated(deltaX / 2.0, deltaY / 2.0, 0);
  int id = 0;
  for (int y = 0; y < TamanhoY(); ++y) {
    for (int x = 0; x < TamanhoX(); ++x) {
      // desenha quadrado
      DesenhaQuadrado(id, id == quadrado_selecionado_);
      // anda 1 quadrado direita
      glTranslated(TAMANHO_GL, 0, 0);
      ++id;
    }
    // volta tudo esquerda e sobe 1 quadrado
    glTranslated(deltaX, TAMANHO_GL, 0);
  }
  glPopMatrix();
  if (!parametros_desenho_.desenha_entidades) {
    return;
  }

  // desenha as entidades no segundo lugar da pilha, importante para diferenciar entidades do tabuleiro
  // na hora do picking.
  glPushName(0);
  for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
    Entidade* entidade = it->second;
    GLfloat vermelho[] = { 1.0, 0, 0, 1.0 };
    GLfloat verde[] = { 0, 1.0, 0, 1.0 };
    Cor(entidade_selecionada_ == entidade ? verde : vermelho); 
    entidade->Desenha();
  }
  glPopName();
}

// Esta operacao se chama PICKING. Mais informacoes podem ser encontradas no capitulo 11-6 do livro verde
// ou entao aqui http://gpwiki.org/index.php/OpenGL:Tutorials:Picking
// basicamente, entra-se em um modo de desenho onde o buffer apenas recebe o identificador de quem o acertou
void Tabuleiro::EncontraHits(int x, int y, double aspecto, unsigned int* numero_hits, unsigned int* buffer_hits) {
  // inicia o buffer de picking (selecao)
  glSelectBuffer(100, buffer_hits);
  // entra no modo de selecao e limpa a pilha de nomes e inicia com 0
  glRenderMode(GL_SELECT);
  glInitNames();
  glPushName(0); // inicia a pilha de nomes com 0 para sempre haver um nome

  // a matriz de pick afeta a projecao, entao vamos salva-la antes de modifica-la
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  gluPickMatrix(x, y, 1.0, 1.0, viewport);
  gluPerspective(CAMPO_VERTICAL, aspecto, 0.5, 500.0);

  // desenha a cena
  DesenhaCena();

  // volta a projecao
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  // volta pro modo de desenho e processa os hits
  *numero_hits = glRenderMode(GL_RENDER);
  glMatrixMode(GL_MODELVIEW);
}

void Tabuleiro::TrataClique(unsigned int numero_hits, unsigned int* buffer_hits) {
  cout << "numero de hits: " << (unsigned int)numero_hits << endl << endl;
  GLuint* ptr_hits = buffer_hits;
  GLuint id = 0, pos_pilha = 0;
  GLuint menor_z = 0xFFFFFFFF;
  for (GLuint i = 0; i < numero_hits; ++i) {
    if (*(ptr_hits + 1) < menor_z) {
      pos_pilha = *ptr_hits;
      cout << "posicao pilha: " << (unsigned int)(pos_pilha) << endl;
      menor_z = *(ptr_hits+1); 
      // pula ele mesmo, profundidade e ids anteriores na pilha
      ptr_hits += (pos_pilha + 2);
      id = *ptr_hits;
      cout << "id: " << (unsigned int)(id) << endl << endl;
      ++ptr_hits;
    }
    else {
      cout << "pulando objeto mais longe..." << endl;
    }
  }

  if (pos_pilha == 1) {
    // Tabuleiro.
    SelecionaQuadrado(id);
    estado_ = ETAB_QUAD_PRESSIONADO; 
  } else if (pos_pilha > 1) {
    // Entidade.
    SelecionaEntidade(id);
    estado_ = ETAB_ENT_PRESSIONADA;
    cout << "Entidade x: " << entidade_selecionada_->X() << ", y: " << entidade_selecionada_->Y() << endl;
  } else {
    entidade_selecionada_ = NULL;
    quadrado_selecionado_ = -1; 
    estado_ = ETAB_OCIOSO;
  }
}

void Tabuleiro::SelecionaEntidade(int id) {
  cout << "selecionando entidade: " << id << endl;
  Entidade* e = entidades_.find(id)->second;
  entidade_selecionada_ = e; 
  quadrado_selecionado_ = -1;
}

void Tabuleiro::SelecionaQuadrado(int id_quadrado) {
  quadrado_selecionado_ = id_quadrado; 
  entidade_selecionada_ = NULL;
}

void Tabuleiro::CoordenadaQuadrado(int id_quadrado, double* x, double* y, double* z) {
  int quad_x = id_quadrado % TamanhoX();
  int quad_y = id_quadrado / TamanhoY();

  // centro do quadrado
  *x = ((quad_x * TAMANHO_GL) + TAMANHO_GL_2) - (TamanhoX() * TAMANHO_GL_2);
  *y = ((quad_y * TAMANHO_GL) + TAMANHO_GL_2) - (TamanhoY() * TAMANHO_GL_2); 
  *z = 0;
}

ntf::Notificacao* Tabuleiro::CriaNotificacaoTabuleiro() const {
  auto* notificacao = new ntf::Notificacao;
  notificacao->set_local(false);
  notificacao->set_remota(true);
  notificacao->set_tipo(ntf::TN_TABULEIRO);
  auto* t = notificacao->mutable_tabuleiro();
  for (const auto& id_ent : entidades_) {
    t->add_entidade()->CopyFrom(id_ent.second->Proto());
  }
  return notificacao;
}


void Tabuleiro::RecebeNotificacaoTabuleiro(const ntf::Notificacao& notificacao) {
  for (const auto& ep : entidades_) {
    delete ep.second;
  }
  entidades_.clear();
  for (const auto& ep : notificacao.tabuleiro().entidade()) {
    auto* e = new Entidade(ep);
    entidades_.insert({ e->Id(), e });
  }
}


