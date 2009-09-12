#include <stdexcept>
#include <QMouseEvent>
#include <GL/gl.h>
#include <GL/glut.h>
#include "ifg/qt/visualizador3d.h"
#include "ent/parametrosdesenho.h"
#include "ent/tabuleiro.h"
#include "ntf/notificacao.h"

using namespace ifg::qt;
using namespace std;

const double CAMPO_VERTICAL = 60.0; // campo de visao vertical
const double OLHO_ALTURA_INICIAL = 10.0;
const double OLHO_ALTURA_MAXIMA = 15;
const double OLHO_DELTA_MAXIMO = (OLHO_ALTURA_MAXIMA - OLHO_ALTURA_INICIAL);
const double OLHO_ALTURA_MINIMA = 1.5;
const double OLHO_DELTA_MINIMO = (OLHO_ALTURA_MINIMA - OLHO_ALTURA_INICIAL);
const double SENSIBILIDADE_RODA = 0.01;

namespace {
	enum modovis_t {
		MODOVIS_COMECO,
		MODOVIS_TABULEIRO,
	};
}

/** dados do visualizador 3d. */
class Visualizador3d::Dados {
public:
	Dados() : modoVis_(MODOVIS_COMECO), mouseUltimoY_(0), theta_(0) {}
	~Dados() {}

 	/** parametros de desenho da cena. */
 	ent::ParametrosDesenho parametrosDesenho_;

	/** modo do visualizador. Outros podem ser adicionados. */
	modovis_t modoVis_;

 	// ultimo X do mouse
 	int mouseUltimoY_;

 	// angulo de rotacao da camera
 	double theta_;
};

Visualizador3d::Visualizador3d(QWidget* pai) : 
	QGLWidget(QGLFormat(QGL::DepthBuffer | QGL::Rgba | QGL::DoubleBuffer), pai), dv3d_(new Dados){}

Visualizador3d::~Visualizador3d() {
}

// reimplementacoes
void Visualizador3d::initializeGL() {
	// cor da borracha
	char* bla = (char*)"bla";
	int argc = 1;
	glutInit(&argc, &bla);
	glClearColor(1.0, 1.0, 1.0, 1.0);

	// back face
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// zbuffer
	glEnable(GL_DEPTH_TEST);
}

void Visualizador3d::resizeGL(int width, int height) {
	if (dv3d_->modoVis_ == MODOVIS_TABULEIRO) {
		// projecao ortogonal
		glViewport(0, 0, (GLint)width, (GLint)height);
	
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(CAMPO_VERTICAL, (double)width / height, 0.5, ent::Tabuleiro::instancia().tamanhoX()*2.0);
	}
}

void Visualizador3d::paintGL() {
	if (dv3d_->modoVis_ == MODOVIS_TABULEIRO) {
		desenhaCena();
	}
}

// notificacao
void Visualizador3d::trataNotificacao(const ntf::Notificacao& notificacao) {
	switch (notificacao.tipo()) {
		case ntf::TN_INICIAR:
			dv3d_->modoVis_ = MODOVIS_TABULEIRO;
			// chama o resize pra iniciar a geometria e desenha a janela
			resizeGL(width(), height());
			glDraw();
		break;
		default:
			;
	}
}

// mouse

void Visualizador3d::mousePressEvent(QMouseEvent* event) {
	if (dv3d_->modoVis_ == MODOVIS_TABULEIRO) {
		// roda a tela no eixo Z de acordo com o eixo X do movimento
		dv3d_->mouseUltimoY_ = event->y();
		trataClique(event->x(), (height() - event->y()));
	}
	event->accept();
}

void Visualizador3d::mouseMoveEvent(QMouseEvent* event) {
	if (dv3d_->modoVis_ == MODOVIS_TABULEIRO) {
	}
	event->accept();
}

void Visualizador3d::wheelEvent(QWheelEvent* event) {
	if (dv3d_->modoVis_ == MODOVIS_TABULEIRO) {
		// move o olho no eixo Z de acordo com o eixo Y do movimento
		dv3d_->theta_ -= (event->delta() * SENSIBILIDADE_RODA); 
		if (dv3d_->theta_ < OLHO_DELTA_MINIMO) {
			dv3d_->theta_ = OLHO_DELTA_MINIMO;
		}
		else if (dv3d_->theta_ > OLHO_DELTA_MAXIMO) {
			dv3d_->theta_ = OLHO_DELTA_MAXIMO;
		}
		glDraw();	
	}
	event->accept();
}

// privadas auxiliareas
void Visualizador3d::desenhaCena() {
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		0, -ent::Tabuleiro::instancia().tamanhoX(), (OLHO_ALTURA_INICIAL + dv3d_->theta_), // from
		0, 0, 0,       // to
		0, 0, 1.0      // up
	);

	//ceu_.desenha(parametrosDesenho_);
	ent::Tabuleiro::instancia().desenha(dv3d_->parametrosDesenho_);
	glPopName();
}

void Visualizador3d::trataClique(int x, int y) {
	// essa operacao se chama PICKING. Mais informacoes podem ser encontradas no capitulo 11-6 do livro verde
	// ou entao aqui http://gpwiki.org/index.php/OpenGL:Tutorials:Picking
	// basicamente, entra-se em um modo de desenho onde o buffer apenas recebe o identificador de quem o acertou

	// informacao dos hits
	GLuint bufferHits[100] = {0};

	// inicia o buffer de picking (selecao)
	glSelectBuffer(100, bufferHits);
	// entra no modo de selecao e limpa a pilha de nomes e inicia com 0
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0); // inicia a pilha de nomes com 0 para sempre haver um nome

	// a pick matrix afeta a projecao, entao vamos salva-la antes de modifica-la
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	gluPickMatrix(x, y, 1.0, 1.0, viewport);
	gluPerspective(CAMPO_VERTICAL, (double)width() / height(), 0.5, ent::Tabuleiro::instancia().tamanhoX()*2);

	// desenha a cena
	desenhaCena();

	// volta a projecao
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// volta pro modo de desenho e processa os hits
	GLuint numeroHits = glRenderMode(GL_RENDER);
	ent::Tabuleiro::instancia().trataClique(numeroHits, bufferHits);
	glMatrixMode(GL_MODELVIEW);
	glDraw();
}













