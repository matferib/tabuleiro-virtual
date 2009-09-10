#include <QMouseEvent>
#include <GL/gl.h>
#include <iostream>
#include "ifg/qt/visualizador3d.h"

using namespace ifg::qt;
using namespace std;

const double TAM_TABULEIRO = 64;  // tamanho do lado do tabuleiro em quadrados
const double CAMPO_VERTICAL = 60; // camoo de visao vertical

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
	gluPerspective(CAMPO_VERTICAL, (double)width / height, 0.5, TAM_TABULEIRO*2);
}

void Visualizador3d::paintGL() {
	desenhaCena();
}

// entidades

void Visualizador3d::adicionaEntidade(ent::Entidade* entidade) {
	entidades_.push_back(entidade);
}

// mouse

void Visualizador3d::mousePressEvent(QMouseEvent* event) {
	// verifica clique, Y deve ser invertido
	if (trataClique(event->x(), (height() - event->y()))) {
		arrastando_ = true;
	}
	else {
		arrastando_ = false;
		// roda a tela no eixo Z de acordo com o eixo X do movimento
		mouseUltimoY_ = event->y();
	}
	event->accept();
}

void Visualizador3d::mouseMoveEvent(QMouseEvent* event) {
	if (arrastando_) {
	}
	else {
		// roda a tela no eixo Z de acordo com o eixo Y do movimento
		int yEvento = event->y();
		theta_ -= (yEvento - mouseUltimoY_);
		mouseUltimoY_ = yEvento;
		glDraw();	
	}
	event->accept();
}

// cena
bool Visualizador3d::trataClique(int x, int y) {
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
	gluPerspective(CAMPO_VERTICAL, (double)width() / height(), 0.5, TAM_TABULEIRO*2);

	// desenha a cena
	desenhaCena();

	// volta a projecao
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// volta pro modo de desenho e processa os hits
	GLuint numeroHits = glRenderMode(GL_RENDER);
	cout << "numero de hits: " << (unsigned int)numeroHits << endl << endl;
	GLuint* ptrHits = bufferHits;
	for (GLuint i = 0; i < numeroHits; ++i) {
		cout << "posicao pilha: " << (unsigned int)(*ptrHits) << endl;
		// pula profundidade por agora
		ptrHits += 3;
		cout << "id: " << (unsigned int)(*ptrHits) << endl << endl;
		tabuleiro_.clique(*ptrHits);
		++ptrHits;
	}
	glMatrixMode(GL_MODELVIEW);

	glDraw();

	return (numeroHits > 0);
}

void Visualizador3d::desenhaCena() {
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		0, -50.0, 20.0 + theta_, // from
		0, 0, 0,       // to
		0, 0, 1.0      // up
	);

	//ceu_.desenha(parametrosDesenho_);
	tabuleiro_.desenha(parametrosDesenho_);
	for (list<ent::Entidade*>::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
		(*it)->desenha(parametrosDesenho_);
	}
}












