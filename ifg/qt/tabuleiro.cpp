#include <GL/gl.h>
#include "ifg/qt/tabuleiro.h"
#include <iostream>

using namespace ifg::qt;
using namespace std;

Tabuleiro::Tabuleiro(QWidget* pai) : QGLWidget(pai) {}

// reimplementacoes
void Tabuleiro::initializeGL() {
	// cor da borracha
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

void Tabuleiro::resizeGL(int width, int height) {
	// projecao ortogonal
	glViewport(0, 0, (GLint)width, (GLint)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-width/2, width/2, -height/2, height/2);
	cout <<"width: "<< width << ", height: " << height << endl;
}

void Tabuleiro::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT);
	// desenha um quadrado verde
	glColor3f(0, 1.0, 0);
	glRecti(50, 50, -50, -50);
}


