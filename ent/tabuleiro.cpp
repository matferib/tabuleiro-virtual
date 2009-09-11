#include <vector>
#include <stdexcept>
#include <GL/gl.h>
#include "ent/tabuleiro.h"
#include "ent/parametrosdesenho.h"

using namespace ent;
using namespace std;

namespace {

	/** representa um quadrado no tabuleiro. Cada quadrado tem lado 1,5m. */
	class Quadrado {
	public:
		/** tamanho do lado do quadrado no 3D. */
		static const double TAMANHO_GL = 1.5;

		/** desenha o quadrado. */
		void desenha(const ParametrosDesenho& pd, GLuint id) {
			glLoadName(id);
			glColor3dv(selecionado_ ? pd.corTabuleiroSelecionado() : pd.corTabuleiroNaoSelecionado());
			glRectf(0.2, 0.2, (TAMANHO_GL-0.2), (TAMANHO_GL-0.2));
		}

		/** desenha as entidades no quadrado. */
		void desenhaEntidades(const ParametrosDesenho& pd) {
			if (entidade_ != NULL) {
				entidade_->desenha(pd);
			}
		}

		/** seleciona ou deseleciona o quadrado. */
		void seleciona(bool val) { 
			selecionado_ = val; 
		}

		/** adiciona entidade ao quadrado. */
		void adicionaEntidade(Entidade* entidade) {
			if (entidade_ != NULL) {
				throw logic_error("quadrado ja tem entidade");
			}
			entidade_ = entidade;
		}	 

		/** @return a entidade do quadrado ou NULL se nao houver. */
		Entidade* entidade() const {
			return const_cast<Quadrado*>(this)->entidade_;
		}

	private:
		/** indica que o quadrado esta selecionado. */
		bool selecionado_;

		/** entidade no quadrado. */
		Entidade* entidade_;
	};

}

namespace ent {
	class DadosTabuleiro {
	public:
		explicit DadosTabuleiro(int tamanho) : tamanho_(tamanho) {
			quadrados_ = new vector<Quadrado>[tamanho];
			for (int i = 0; i < tamanho; ++i) {
				quadrados_[i].resize(tamanho);
			}
			quadradoSelecionado_ = NULL;
		}

		~DadosTabuleiro() {
			delete[] quadrados_;
		}

		inline int tamanhoX() const { return tamanho_; }
		inline int tamanhoY() const { return tamanho_; }

		// quadrados
		/** @return o quadrado identificado por id no desenho. */
		inline Quadrado& quadrado(int id) { return quadrados_[id / tamanhoY()][ id % tamanhoX()]; }

	private:
		/** tamanho do tabuleiro (tamanho_ x tamanho_). */
		int tamanho_;

	public:
		/** quadrados */
		vector<Quadrado>* quadrados_;

		/** quadrado selecionado. */
		Quadrado* quadradoSelecionado_;
	};
}

Tabuleiro::Tabuleiro(int tamanho) : dt_(new DadosTabuleiro(tamanho)) {
}

Tabuleiro::~Tabuleiro() {
	delete dt_;
}

void Tabuleiro::desenha(const ParametrosDesenho& pd) {
	glPushMatrix();
	// desenha de baixo pra cima
	double deltaX = -dt_->tamanhoX() * Quadrado::TAMANHO_GL;
	double deltaY = -dt_->tamanhoY() * Quadrado::TAMANHO_GL;
	glTranslatef(deltaX / 2.0, deltaY / 2.0, 0);

	GLuint id = 0;
	for (int y = 0; y < dt_->tamanhoY(); ++y) {
		vector<Quadrado>& vq = dt_->quadrados_[y];
		for (vector<Quadrado>::iterator itx = vq.begin(); itx != vq.end(); ++itx) {
			Quadrado& q = *itx;
			q.desenha(pd, id++);
			glTranslatef(Quadrado::TAMANHO_GL, 0, 0);
		}
		glTranslatef(deltaX, Quadrado::TAMANHO_GL, 0);
	}
	glPopMatrix();
}

void Tabuleiro::desenhaEntidades(const ParametrosDesenho& pd) {
	glPushMatrix();
	// desenha de baixo pra cima
	double deltaX = -dt_->tamanhoX() * Quadrado::TAMANHO_GL;
	double deltaY = -dt_->tamanhoY() * Quadrado::TAMANHO_GL;
	glTranslatef((deltaX +  Quadrado::TAMANHO_GL)/ 2.0, (deltaY + Quadrado::TAMANHO_GL) / 2.0, 0);

	for (int y = 0; y < dt_->tamanhoY(); ++y) {
		vector<Quadrado>& vq = dt_->quadrados_[y];
		for (vector<Quadrado>::iterator itx = vq.begin(); itx != vq.end(); ++itx) {
			Quadrado& q = *itx;
			q.desenhaEntidades(pd);
			glTranslatef(Quadrado::TAMANHO_GL, 0, 0);
		}
		glTranslatef(deltaX, Quadrado::TAMANHO_GL, 0);
	}
	glPopMatrix();
}


void Tabuleiro::selecionaQuadrado(int id) {
	if (dt_->quadradoSelecionado_ != NULL) { dt_->quadradoSelecionado_->seleciona(false); }
	Quadrado& q = dt_->quadrado(id);
	dt_->quadradoSelecionado_ = &q; 
	dt_->quadradoSelecionado_->seleciona(true);
}

void Tabuleiro::adicionaEntidade(Entidade* entidade, int id) {
	Quadrado& q = dt_->quadrado(id);
	q.adicionaEntidade(entidade);
}




