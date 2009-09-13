#include <stdexcept>
#include <QMouseEvent>
#include <cmath>
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
const double OLHO_RAIO_INICIAL = 20.0;
const double OLHO_RAIO_MINIMO = 1.5;
const double OLHO_RAIO_MAXIMO = 40.0;
const double OLHO_ALTURA_MAXIMA = 15;
const double OLHO_DELTA_MAXIMO = (OLHO_ALTURA_MAXIMA - OLHO_ALTURA_INICIAL);
const double OLHO_ALTURA_MINIMA = 1.5;
const double OLHO_DELTA_MINIMO = (OLHO_ALTURA_MINIMA - OLHO_ALTURA_INICIAL);
const double SENSIBILIDADE_RODA = 0.01;
const double SENSIBILIDADE_ROTACAO_X = 0.01;
const double SENSIBILIDADE_ROTACAO_Y = 0.08;

namespace {
	enum modovis_t {
		MODOVIS_COMECO,
		MODOVIS_TABULEIRO,
		MODOVIS_TABULEIRO_ROTACAO
	};
}

/** dados do visualizador 3d. */
class Visualizador3d::Dados {
public:
	Dados() : modoVis_(MODOVIS_COMECO), olhoX_(0), olhoY_(0), olhoDeltaAltura_(0), olhoRaio_(OLHO_RAIO_INICIAL) {}
	~Dados() {}

	void trataNotificacao(const ntf::Notificacao& notificacao) {
		switch (notificacao.tipo()) {
			case ntf::TN_INICIAR:
				modoVis_ = MODOVIS_TABULEIRO;
				// chama o resize pra iniciar a geometria e desenha a janela
				break;
			default:
				;
		}
	}

	void redimensionaJanela(int largura, int altura) {
		if (modoVis_ == MODOVIS_TABULEIRO) {
			// projecao ortogonal
			glViewport(0, 0, (GLint)largura, (GLint)altura);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(CAMPO_VERTICAL, (double)largura / altura, 0.5, ent::Tabuleiro::instancia().tamanhoX()*2.0);
		}
	}

	void desenhaJanela() {
		if ((modoVis_ == MODOVIS_TABULEIRO) || (modoVis_ == MODOVIS_TABULEIRO_ROTACAO)) {
			desenhaCena();
		}
	}

	void trataBotaoPressionado(QMouseEvent* event, int altura, double aspecto) {
		if (modoVis_ == MODOVIS_TABULEIRO) {
			// roda a tela no eixo Z de acordo com o eixo X do movimento
			if (event->button() == Qt::MidButton) {
				trataInicioRotacao(event);
			}
			else if (event->button() == Qt::LeftButton) {
				trataClique(event->x(), (altura - event->y()), aspecto);
			}
		}
		event->accept();
	}

	void trataBotaoLiberado(QMouseEvent* event) {
		if (modoVis_ == MODOVIS_TABULEIRO_ROTACAO) {
			modoVis_ = MODOVIS_TABULEIRO;
		}
	}

	void trataMovimento(QMouseEvent* event) {
		if (modoVis_ == MODOVIS_TABULEIRO) {
		}
		else if (modoVis_ == MODOVIS_TABULEIRO_ROTACAO) {
			trataRotacao(event);
		}
		event->accept();
	}

	void trataRodela(QWheelEvent* event) {
		if (modoVis_ == MODOVIS_TABULEIRO) {
			// move o olho no eixo Z de acordo com o eixo Y do movimento
			olhoRaio_ -= (event->delta() * SENSIBILIDADE_RODA); 
			if (olhoRaio_ < OLHO_RAIO_MINIMO) {
				olhoRaio_ = OLHO_RAIO_MINIMO;
			}
			else if (olhoRaio_ > OLHO_RAIO_MAXIMO) {
				olhoRaio_ = OLHO_RAIO_MAXIMO;
			}
		}
		event->accept();
	}

	
private:
	void trataClique(int x, int y, double aspecto) {
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
		gluPerspective(CAMPO_VERTICAL, aspecto, 0.5, ent::Tabuleiro::instancia().tamanhoX()*2);

		// desenha a cena
		desenhaCena();

		// volta a projecao
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		// volta pro modo de desenho e processa os hits
		GLuint numeroHits = glRenderMode(GL_RENDER);
		ent::Tabuleiro::instancia().trataClique(numeroHits, bufferHits);
		glMatrixMode(GL_MODELVIEW);
	}

	void desenhaCena() {
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(
			olhoX_ + cos(olhoDeltaRotacao_) * olhoRaio_, olhoY_ + sin(olhoDeltaRotacao_) * olhoRaio_, (OLHO_ALTURA_INICIAL + olhoDeltaAltura_), // from
			olhoX_, olhoY_, 0,       // to
			0, 0, 1.0      // up
		);

		//ceu_.desenha(parametrosDesenho_);
		ent::Tabuleiro::instancia().desenha(parametrosDesenho_);
		glPopName();
	}

	/** chamada no inicio de uma rotacao (press mouse do meio). */
	void trataInicioRotacao(QMouseEvent* event) {
		rotacaoUltimoX_ = event->x();
		rotacaoUltimoY_ = event->y();
		modoVis_ = MODOVIS_TABULEIRO_ROTACAO;
	}

	/** trata o movimento de rotacao. */
	void trataRotacao(QMouseEvent* event) {
		int x = event->x();
		int y = event->y();
		olhoDeltaRotacao_ -= (x - rotacaoUltimoX_) * SENSIBILIDADE_ROTACAO_X;
		if (olhoDeltaRotacao_ >= 2*M_PI) {
			olhoDeltaRotacao_ -= 2*M_PI;
		}
		else if (olhoDeltaRotacao_ <= -2*M_PI) {
			olhoDeltaRotacao_ += 2*M_PI;
		}
		// move o olho no eixo Z de acordo com o eixo Y do movimento
		olhoDeltaAltura_ += (y - rotacaoUltimoY_) * SENSIBILIDADE_ROTACAO_Y; 
		if (olhoDeltaAltura_ < OLHO_DELTA_MINIMO) {
			olhoDeltaAltura_ = OLHO_DELTA_MINIMO;
		}
		else if (olhoDeltaAltura_ > OLHO_DELTA_MAXIMO) {
			olhoDeltaAltura_ = OLHO_DELTA_MAXIMO;
		}

		rotacaoUltimoX_ = x;
		rotacaoUltimoY_ = y;
	}

	void trataFimRotacao() {
		modoVis_ = MODOVIS_TABULEIRO;
	}

private:
 	// parametros de desenho da cena.
 	ent::ParametrosDesenho parametrosDesenho_;

	// modo do visualizador. Outros podem ser adicionados.
	modovis_t modoVis_;

	int rotacaoUltimoX_; 
	int rotacaoUltimoY_; 

 	// altura do olho
	double olhoX_;
	double olhoY_;
 	double olhoDeltaAltura_;
	double olhoDeltaRotacao_;
	double olhoRaio_;
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
	dv3d_->redimensionaJanela(width, height);
}

void Visualizador3d::paintGL() {
	dv3d_->desenhaJanela();
}

// notificacao
void Visualizador3d::trataNotificacao(const ntf::Notificacao& notificacao) {
	dv3d_->trataNotificacao(notificacao);
	switch (notificacao.tipo()) {
		case ntf::TN_INICIAR:
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
	int altura = height();
	double aspecto = static_cast<double>(width()) / altura;
	dv3d_->trataBotaoPressionado(event, altura, aspecto);
	glDraw();
}

void Visualizador3d::mouseReleaseEvent(QMouseEvent* event) {
	dv3d_->trataBotaoLiberado(event);
	glDraw();
}

void Visualizador3d::mouseMoveEvent(QMouseEvent* event) {
	dv3d_->trataMovimento(event);
	glDraw();
}

void Visualizador3d::wheelEvent(QWheelEvent* event) {
	dv3d_->trataRodela(event);
	glDraw();
}
















