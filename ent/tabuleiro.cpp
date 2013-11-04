#include <vector>
#include <map>
#include <stdexcept>
#include <cmath>
#include <GL/glu.h>
#include <GL/gl.h>
#include <iostream>
#include "ent/tabuleiro.h"
#include "ent/parametrosdesenho.h"
#include "ent/movel.h"
#include "ntf/notificacao.h"

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
	/** tamanho do lado do quadrado / 2. */
	const double TAMANHO_GL_2 = (TAMANHO_GL / 2.0);

	/** instancia unica do tabuleiro. */
	Tabuleiro* instancia_ = NULL;

	/** desenha o quadrado. */
	void desenhaQuadrado(const ParametrosDesenho& pd, GLuint id) {
		// desenha o quadrado
		glLoadName(id);
		glRectf(0.2, 0.2, (TAMANHO_GL-0.2), (TAMANHO_GL-0.2));
	}

}

bool Tabuleiro::haInstancia() {
	return (instancia_ != NULL);
}

Tabuleiro& Tabuleiro::instancia() {
	return *instancia_;
}

void Tabuleiro::cria(int tamanho) {
	delete instancia_;
	instancia_ = new Tabuleiro(tamanho);
}

Tabuleiro::Tabuleiro(int tamanho) : 
	tamanho_(tamanho), 
	entidadeSelecionada_(NULL), 
	quadradoSelecionado_(-1), 
	estado_(ETAB_NORMAL), proximoId_(0),
	olhoX_(0), olhoY_(0), olhoZ_(0), olhoDeltaRotacao_(0), olhoAltura_(OLHO_ALTURA_INICIAL), olhoRaio_(OLHO_RAIO_INICIAL)
{
	if (instancia_ != NULL) {
		throw logic_error("tabuleiro ja existe");
	}
	instancia_ = this;
}


Tabuleiro::~Tabuleiro() {
}

int Tabuleiro::tamanhoX() const { 
	return tamanho_; 
}

int Tabuleiro::tamanhoY() const { 
	return tamanho_; 
}

void Tabuleiro::desenha() {
	desenhaCena();
}

void Tabuleiro::adicionaEntidade(tipoent_e tipoEntidade, DadosCriacao* dc, int idQuadrado) {
	double x, y, z;
	coordenadaQuadrado(x, y, z, idQuadrado);
	Entidade* entidade;
	switch (tipoEntidade) {
		case TIPOENT_MOVEL:
			entidade = new Movel(proximoId_++, 0, x, y, z);
			break;
		default:
			throw logic_error("tipo invalido de entidade");
	}
	entidades_.insert(make_pair(entidade->id(), entidade));
}

void Tabuleiro::removeEntidade(int id) {
	MapaEntidades::iterator resFind = entidades_.find(id);
	if (resFind == entidades_.end()) {
		return;
	}
	entidades_.erase(resFind);
	Entidade* entidade = resFind->second;
	if (entidadeSelecionada_ == entidade) {
		entidadeSelecionada_ = NULL;
	}
	delete entidade;
}

void Tabuleiro::trataNotificacao(const ntf::Notificacao& notificacao) {
	switch (notificacao.tipo()) {
		case ntf::TN_ADICIONAR_ENTIDADE:
			estado_ = ETAB_ADICIONANDO_ENTIDADE;
		break;
		case ntf::TN_ENTIDADE_ADICIONADA:
		break;
		case ntf::TN_REMOVER_ENTIDADE:
			estado_ = ETAB_REMOVENDO_ENTIDADE;
		break;
		case ntf::TN_ENTIDADE_REMOVIDA:
		break;
		default:
			;
	}
}

void Tabuleiro::trataRodela(int delta) {
	// move o olho no eixo Z de acordo com o eixo Y do movimento
	olhoRaio_ -= (delta * SENSIBILIDADE_RODA); 
	if (olhoRaio_ < OLHO_RAIO_MINIMO) {
		olhoRaio_ = OLHO_RAIO_MINIMO;
	}
	else if (olhoRaio_ > OLHO_RAIO_MAXIMO) {
		olhoRaio_ = OLHO_RAIO_MAXIMO;
	}
}

void Tabuleiro::trataMovimento(int x, int y) {
	if (estado_ == ETAB_ROTACAO) {
		olhoDeltaRotacao_ -= (x - rotacaoUltimoX_) * SENSIBILIDADE_ROTACAO_X;
		if (olhoDeltaRotacao_ >= 2*M_PI) {
			olhoDeltaRotacao_ -= 2*M_PI;
		}
		else if (olhoDeltaRotacao_ <= -2*M_PI) {
			olhoDeltaRotacao_ += 2*M_PI;
		}
		// move o olho no eixo Z de acordo com o eixo Y do movimento
		olhoAltura_ -= (y - rotacaoUltimoY_) * SENSIBILIDADE_ROTACAO_Y; 
		if (olhoAltura_ < OLHO_ALTURA_MINIMA) {
			olhoAltura_ = OLHO_ALTURA_MINIMA;
		}
		else if (olhoAltura_ > OLHO_ALTURA_MAXIMA) {
			olhoAltura_ = OLHO_ALTURA_MAXIMA;
		}

		rotacaoUltimoX_ = x;
		rotacaoUltimoY_ = y;
	}
}

void Tabuleiro::trataBotaoLiberado() {
	estado_ = ETAB_NORMAL;
}

void Tabuleiro::trataBotaoPressionado(botao_e botao, int x, int y, double aspecto) {
	if ((estado_ == ETAB_NORMAL) || (estado_ == ETAB_ADICIONANDO_ENTIDADE) || (estado_ == ETAB_REMOVENDO_ENTIDADE)) {
		// roda a tela no eixo Z de acordo com o eixo X do movimento
		if (botao == BOTAO_MEIO) {
			rotacaoUltimoX_ = x;
			rotacaoUltimoY_ = y;
			estado_ = ETAB_ROTACAO;
		}
		else if (botao == BOTAO_ESQUERDO) {
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
			glMatrixMode(GL_MODELVIEW);

			trataClique(numeroHits, bufferHits);
		}
	}
}

void Tabuleiro::trataRedimensionaJanela(int largura, int altura) {
	glViewport(0, 0, (GLint)largura, (GLint)altura);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(CAMPO_VERTICAL, (double)largura / altura, 0.5, tamanhoX()*2.0);
}

void Tabuleiro::inicializaGL() {
	glClearColor(1.0, 1.0, 1.0, 1.0);

	// back face
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// zbuffer
	glEnable(GL_DEPTH_TEST);
}


// privadas 

void Tabuleiro::desenhaCena() {
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		// from
		olhoX_ + cos(olhoDeltaRotacao_) * olhoRaio_, 
		olhoY_ + sin(olhoDeltaRotacao_) * olhoRaio_, 
		olhoAltura_,
		// to
		olhoX_, olhoY_, olhoZ_,
		// up
		0, 0, 1.0
	);

	//ceu_.desenha(parametrosDesenho_);

	// desenha tabuleiro de baixo pra cima
	glPushMatrix();
	double deltaX = -tamanhoX() * TAMANHO_GL;
	double deltaY = -tamanhoY() * TAMANHO_GL;
	glTranslated(deltaX / 2.0, deltaY / 2.0, 0);
	int id = 0;
	for (int y = 0; y < tamanhoY(); ++y) {
		for (int x = 0; x < tamanhoX(); ++x) {
			// desenha quadrado
			glColor3dv(id == quadradoSelecionado_ ? 
				parametrosDesenho_.corTabuleiroSelecionado() : parametrosDesenho_.corTabuleiroNaoSelecionado()
			);
			desenhaQuadrado(parametrosDesenho_, id);
			// anda 1 quadrado direita
			glTranslated(TAMANHO_GL, 0, 0);
			++id;
		}
		// volta tudo esquerda e sobe 1 quadrado
		glTranslated(deltaX, TAMANHO_GL, 0);
	}
	glPopMatrix();

	// desenha as entidades no segundo lugar da pilha
	glPushName(0);
	for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
		Entidade* entidade = it->second;
		glColor3dv(
			entidadeSelecionada_ == entidade ? 
			parametrosDesenho_.corEntidadeSelecionada() : parametrosDesenho_.corEntidadeNaoSelecionada()
		);
		entidade->desenha(parametrosDesenho_);
	}
	glPopName();
}

void Tabuleiro::trataClique(unsigned int numeroHits, unsigned int* bufferHits) {
	cout << "numero de hits: " << (unsigned int)numeroHits << endl << endl;
	GLuint* ptrHits = bufferHits;
	GLuint id = 0, posPilha = 0;
	GLuint menorZ = 0xFFFFFFFF;
	for (GLuint i = 0; i < numeroHits; ++i) {
		if (*(ptrHits+1) < menorZ) {
			posPilha = *ptrHits;
			cout << "posicao pilha: " << (unsigned int)(posPilha) << endl;
			menorZ = *(ptrHits+1); 
			// pula ele mesmo, profundidade e ids anteriores na pilha
			ptrHits += (posPilha + 2);
			id = *ptrHits;
			cout << "id: " << (unsigned int)(id) << endl << endl;
			++ptrHits;
		}
		else {
			cout << "pulando objeto mais longe..." << endl;
		}
	}

	if (posPilha == 1) {
		// tabuleiro
		if (estado_ == ETAB_NORMAL) {
			selecionaQuadrado(id);
		}
		else if (estado_ == ETAB_ADICIONANDO_ENTIDADE) {
			adicionaEntidade(TIPOENT_MOVEL, NULL, id);
			estado_ = ETAB_NORMAL;
		}
	}
	else if (posPilha > 1) {
		if (estado_ == ETAB_NORMAL) {
			selecionaEntidade(id);
		}
		else if (estado_ == ETAB_REMOVENDO_ENTIDADE) {
			removeEntidade(id);
			estado_ = ETAB_NORMAL;
		}
	}
	else {
		entidadeSelecionada_ = NULL;
		quadradoSelecionado_ = -1;
	}
}

void Tabuleiro::selecionaEntidade(int id) {
	cout << "selecionando entidade: " << id << endl;
	Entidade* e = entidades_.find(id)->second;
	entidadeSelecionada_ = e; 
	quadradoSelecionado_ = -1;
}

void Tabuleiro::selecionaQuadrado(int idQuadrado) {
	quadradoSelecionado_ = idQuadrado; 
	entidadeSelecionada_ = NULL;
}

void Tabuleiro::coordenadaQuadrado(double& x, double& y, double& z, int idQuadrado) {
	int quadX = idQuadrado % tamanhoX();
	int quadY = idQuadrado / tamanhoY();

	// centro do quadrado
	x = ((quadX * TAMANHO_GL) + TAMANHO_GL_2) - (tamanhoX() * TAMANHO_GL_2);
	y = ((quadY * TAMANHO_GL) + TAMANHO_GL_2) - (tamanhoY() * TAMANHO_GL_2); 
	z = 0;
}









