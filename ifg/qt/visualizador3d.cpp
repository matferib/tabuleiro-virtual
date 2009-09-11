#include <stdexcept>
#include <QMouseEvent>
#include <GL/gl.h>
#include <iostream>
#include "ifg/qt/visualizador3d.h"
#include "ent/parametrosdesenho.h"
#include "ent/tabuleiro.h"

using namespace ifg::qt;
using namespace std;

const double TAM_TABULEIRO = 20;  // tamanho do lado do tabuleiro em quadrados
const double CAMPO_VERTICAL = 60; // camoo de visao vertical

/** dados do visualizador 3d. */
class Visualizador3d::Dados {
public:
	Dados() : tabuleiro_(TAM_TABULEIRO), mouseUltimoY_(0), theta_(0) {}
	~Dados() {
		for (list<ent::Entidade*>::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
			delete *it;
		}
	}

	ent::Entidade* achaEntidade(int id) const {
		Dados* dThis = const_cast<Dados*>(this);
		for (
			list<ent::Entidade*>::iterator it = dThis->entidades_.begin(); 
			it != dThis->entidades_.end(); 
			++it
		) {
			if ((*it)->id() == id) {
				return *it;
			}
		}
		throw logic_error("entidade nao encontrada");
	}

 	/** parametros de desenho da cena. */
 	ent::ParametrosDesenho parametrosDesenho_;

 	/** elementos da cena: terreno. */
 	ent::Tabuleiro tabuleiro_;

 	/** elementos da cena: ceu. */
 	//gl::Ceu ceu_;

 	/** elementos da cena: entidades. */
 	std::list<ent::Entidade*> entidades_;

 	// arrastando
 	bool arrastando_;

 	// ultimo X do mouse
 	int mouseUltimoY_;

 	// angulo de rotacao da camera
 	double theta_;
};

Visualizador3d::Visualizador3d(QWidget* pai) : 
	QGLWidget(pai), dv3d_(new Dados){}

Visualizador3d::~Visualizador3d() {
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
	gluPerspective(CAMPO_VERTICAL, (double)width / height, 0.5, TAM_TABULEIRO*4);
}

void Visualizador3d::paintGL() {
	desenhaCena();
}

// entidades

void Visualizador3d::adicionaEntidade(ent::Entidade* entidade, int id) {
	dv3d_->entidades_.push_back(entidade);
	dv3d_->tabuleiro_.adicionaEntidade(entidade, id);
}

// mouse

void Visualizador3d::mousePressEvent(QMouseEvent* event) {
	// verifica clique, Y deve ser invertido
	if (trataClique(event->x(), (height() - event->y()))) {
		dv3d_->arrastando_ = true;
	}
	else {
		dv3d_->arrastando_ = false;
		// roda a tela no eixo Z de acordo com o eixo X do movimento
		dv3d_->mouseUltimoY_ = event->y();
	}
	event->accept();
}

void Visualizador3d::mouseMoveEvent(QMouseEvent* event) {
	if (dv3d_->arrastando_) {
	}
	else {
		// roda a tela no eixo Z de acordo com o eixo Y do movimento
		int yEvento = event->y();
		dv3d_->theta_ -= (yEvento - dv3d_->mouseUltimoY_);
		dv3d_->mouseUltimoY_ = yEvento;
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
	GLuint id, posPilha;
	for (GLuint i = 0; i < numeroHits; ++i) {
		posPilha = *ptrHits;
		cout << "posicao pilha: " << (unsigned int)(posPilha) << endl;
		// pula ele mesmo, profundidade e ids anteriores na pilha
		ptrHits += posPilha + 2;
		id = *ptrHits;
		cout << "id: " << (unsigned int)(id) << endl << endl;
		++ptrHits;
	}
	if (posPilha == 1) {
		// tabuleiro
		ent::Entidade* movel = ent::novaEntidade(ent::TIPOENT_MOVEL);
		try {
			adicionaEntidade(movel, id);
		}
		catch (...) {
			delete movel;
		}
		//dv3d_->tabuleiro_.selecionaQuadrado(id);
	}
	else {
		// entidade
		ent::Entidade* e = dv3d_->achaEntidade(id);
		e->seleciona(true);
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
		0, -TAM_TABULEIRO, 20.0 + dv3d_->theta_, // from
		0, 0, 0,       // to
		0, 0, 1.0      // up
	);

	//ceu_.desenha(parametrosDesenho_);
	dv3d_->tabuleiro_.desenha(dv3d_->parametrosDesenho_);
	// so pra diferenciar do tabueleiro
	glPushName(0);
	dv3d_->tabuleiro_.desenhaEntidades(dv3d_->parametrosDesenho_);
	glPopName();
}












