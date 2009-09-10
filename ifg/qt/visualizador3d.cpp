#include <QMouseEvent>
#include <GL/gl.h>
#include "ifg/qt/visualizador3d.h"

using namespace ifg::qt;
using namespace std;

const double TAM_TABULEIRO = 64;

Visualizador3d::Visualizador3d(QWidget* pai) : 
	QGLWidget(pai), tabuleiro_(TAM_TABULEIRO), mouseUltimoY_(0), theta_(0)  
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
	gluPerspective(60, (double)width / height, 0.5, TAM_TABULEIRO*2);
}

void Visualizador3d::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		0, -50.0, 20.0 + theta_, // from
		0, 0, 0,       // to
		0, 0, 1.0      // up
	);

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
	mouseUltimoY_ = event->y();
}

void Visualizador3d::mouseMoveEvent(QMouseEvent* event) {
	// roda a tela no eixo Z de acordo com o eixo X do movimento
	int yEvento = event->y();
	theta_ -= (yEvento - mouseUltimoY_);
	mouseUltimoY_ = yEvento;
	glDraw();	
}

// cena
void Visualizador3d::desenhaCena() {
	//ceu_.desenha(parametrosDesenho_);
	tabuleiro_.desenha(parametrosDesenho_);
	for (list<ent::Entidade*>::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
		(*it)->desenha(parametrosDesenho_);
	}
}












