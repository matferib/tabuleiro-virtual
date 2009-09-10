#include <QMouseEvent>
#include <GL/gl.h>
#include "ifg/qt/visualizador3d.h"
#include "ifg/gl/tabuleiro.h"

using namespace ifg::qt;
using namespace std;

Visualizador3d::Visualizador3d(QWidget* pai) : 
	QGLWidget(pai), mouseUltimoX_(0), theta_(0)  
{}

Visualizador3d::~Visualizador3d() {
	for (list<ent::Entidade*>::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
		delete *it;
	}
}

// reimplementacoes
void Visualizador3d::initializeGL() {
	// cor da borracha
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

void Visualizador3d::resizeGL(int width, int height) {
	// projecao ortogonal
	glViewport(0, 0, (GLint)width, (GLint)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// @todo usar frustum aqui
	gluOrtho2D(-width/2, width/2, -height/2, height/2);
}

void Visualizador3d::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	// @todo aplicar camera aqui
	glRotatef(theta_, 0, 0, 1.0);

	desenhaCena();
}

// entidades

void Visualizador3d::adicionaEntidade(ent::Entidade* entidade) {
	entidades_.push_back(entidade);
}

// mouse

void Visualizador3d::mousePressEvent(QMouseEvent* event) {
	// roda a tela no eixo Z de acordo com o eixo X do movimento
	theta_ = 0;
	mouseUltimoX_ = event->x();
}

void Visualizador3d::mouseMoveEvent(QMouseEvent* event) {
	// roda a tela no eixo Z de acordo com o eixo X do movimento
	int xEvento = event->x();
	theta_ -= ((xEvento - mouseUltimoX_)*10.0 / width());
	mouseUltimoX_ = xEvento;
	glDraw();	
}

// cena
void Visualizador3d::desenhaCena() {
	ceu_.desenha(parametrosDesenho_);
	tabuleiro_.desenha(parametrosDesenho_);
	for (list<ent::Entidade*>::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
		(*it)->desenha(parametrosDesenho_);
	}
}












