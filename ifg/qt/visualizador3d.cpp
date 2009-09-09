#include <QMouseEvent>
#include <GL/gl.h>
#include "ifg/qt/visualizador3d.h"
#include "ifg/gl/tabuleiro.h"

using namespace ifg::qt;
using namespace std;

Visualizador3d::Visualizador3d(QWidget* pai) : 
	QGLWidget(pai), mouseUltimoX_(0), theta_(0)  {}

Visualizador3d::~Visualizador3d() {}

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
	gluOrtho2D(-width/2, width/2, -height/2, height/2);
}

void Visualizador3d::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glRotatef(theta_, 0, 0, 1.0);
	tabuleiro_.desenha(parametrosDesenho_);

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

